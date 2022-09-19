#!/usr/bin/env python2.7

'''
A tool to install Mach-O binaries into an ``.app`` bundle.

Other bundle types (particularly ``.framework`` and ``.plugin`` bundles) are
not supported yet.
'''
from __future__ import print_function

import json
import os
import os.path
import re
import shutil
import subprocess


class Pipeline(object):
    '''
    A simple class to handle a list of shell commands which need to pass input
    to each other.
    '''

    def __init__(self, *commands):
        if not commands:
            raise RuntimeError('Pipeline: at least one command must be given')

        self._commands = commands

    def __call__(self):
        # Use /dev/null as the input for the first command.
        last_input = open(os.devnull, 'r')

        for command_args in self._commands:
            command = subprocess.Popen(command_args, stdin=last_input, stdout=subprocess.PIPE)
            last_input.close()
            last_input = command.stdout

        stdout, stderr = command.communicate()
        if command.returncode:
            raise RuntimeError('failed to execute pipeline:\n%s' % stderr)
        return stdout.decode('utf-8')


class Library(object):
    '''
    A representation of a library.

    This class includes information that a runtime loader needs in order to
    perform its job. It tries to implement the behavior of ``dyld(1)`` as
    closely as possible.

    Known Issues
    ------------

    ``@rpath/`` and ``DYLD_LIBRARY_PATH``
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    When a library contains a reference to a library like
    ``@rpath/libname.dylib``, if ``DYLD_LIBRARY_PATH`` is set to contain a path
    which has a ``libname.dylib``, ``dyld(1)`` will find it even if no
    ``LC_RPATH`` commands are present. This behavior is not documented and it
    only seems to work if a library is directly underneath a
    ``DYLD_LIBRARY_PATH`` path. If the library reference is
    ``@rpath/dir/libname.dylib``, even if a ``dir/libname.dylib`` exists in a
    ``DYLD_LIBRARY_PATH`` path, it will still not be found. It is unknown
    whether this behavior is expected or not due to the lack of documentation.
    The logic in this script includes neither of these behaviors.
    '''

    def __init__(self, path, parent=None, search_paths=None, ignores=None):
        # This is the actual path to a physical file
        self._path = os.path.normpath(path)

        if search_paths is None:
            self._search_paths = []
        else:
            self._search_paths = search_paths

        if ignores is None:
            self._ignores = []
        else:
            self._ignores = ignores

        self._parent = parent
        self._symlinks = None
        self._framework_info = None
        self._executable_path = None
        self._dependencies = None
        self._rpaths = None
        self._installed_id = None

    def __hash__(self):
        return self._path.__hash__()

    def __eq__(self, other):
        return self._path == other._path

    def __repr__(self):
        return 'Library(%s : %s)' % (self.id, self.path)

    @property
    def path(self):
        '''The absolute path to the library.'''
        return self._path

    @property
    def parent(self):
        '''The binary which loaded the library.'''
        return self._parent

    @property
    def name(self):
        '''The name of the library.'''
        return os.path.basename(self.path)

    @property
    def ignores(self):
        '''Regular expressions of IDs to ignore from this library.'''
        if self.parent is None:
            return self._ignores
        return self.parent.ignores

    @property
    def installed_id(self):
        '''
        The ID of the library.

        This is the string by which the library will be referenced by other
        binaries in the installation.
        '''
        return self._installed_id

    def set_installed_id(self, installed_id):
        '''Set the ID of the library as it is installed as.'''
        self._installed_id = installed_id

    @property
    def dependent_reference(self):
        '''
        The prefix to use for a library loaded by the library.

        This is used as the prefix for IDs for libraries loaded by the library.
        It is based on the initial binary which loaded the library. For
        example, executables use ``@executable_path`` and plugins use
        ``@loader_path``. In a chain of loaded libraries, the loader (parent)
        of a library determines the prefix.
        '''
        # Refer to libraries the same way that the library which is loading it
        # references it.
        if self.parent is None:
            raise RuntimeError('Unable to get a reference')
        return self.parent.dependent_reference

    @property
    def symlinks(self):
        '''
        A list of symlinks to the library.

        Symlinks are looked for only beside the library and the names of these
        files are returned, not their full paths.
        '''
        if self._symlinks is None:
            realpath = os.path.realpath(self.path)
            dirname = os.path.dirname(realpath)
            symlinks = Pipeline([
                    'find',
                    '-L',
                    dirname,
                    '-depth', '1',
                    '-samefile', realpath,
                ])

            symlinks = set(symlinks().split())

            symlink_bases = []
            for symlink in symlinks:
                symlink_dir, symlink_base = os.path.split(symlink)
                if not symlink_dir == dirname:
                    continue
                symlink_bases.append(symlink_base)
            if self.name in symlink_bases:
                symlink_bases.remove(self.name)
            self._symlinks = symlink_bases

        return self._symlinks

    @property
    def executable_path(self):
        '''The path to the loading executable (if available).'''
        if self._parent is not None:
            return self._parent.executable_path
        return self._executable_path

    @property
    def loader_path(self):
        '''The path to use for ``@loader_path`` references from the library.'''
        return os.path.dirname(self.path)

    @property
    def is_framework(self):
        '''Whether the library is a framework or not.'''
        return self.path.count('.framework')

    @property
    def framework_info(self):
        '''
        Information for frameworks.

        The return value is a tuple of path (where the framework is located),
        name (the ``NAME.framework`` part of its path), and associated library
        (the path under the ``.framework`` directory which contains the actual
        library binary).

        See the ``framework_path``, ``framework_name``, and
        ``framework_library`` properties.
        '''
        if self._framework_info is None:
            if not self.is_framework:
                self._framework_info = (None, None, None)
            else:
                name = None
                library = []

                path = self.path
                while path:
                    path, component = os.path.split(path)
                    if component.endswith('.framework'):
                        name = component
                        break
                    library.append(component)

                if name is None:
                    raise RuntimeError('%s is not a framework?' % self.path)

                self._framework_info = (
                        os.path.join(path),
                        name,
                        os.path.join(*reversed(library)),
                    )
        return self._framework_info

    @property
    def framework_path(self):
        '''
        The path which contains the ``.framework`` for the library.

        ``None`` if the library is not a framework.
        '''
        return self.framework_info[0]

    @property
    def framework_name(self):
        '''
        The name of the framework containing the library.

        ``None`` if the library is not a framework.
        '''
        return self.framework_info[1]

    @property
    def framework_library(self):
        '''
        The path to the library under the ``.framework`` directory.

        ``None`` if the library is not a framework.
        '''
        return self.framework_info[2]

    @property
    def rpaths(self):
        '''
        The list of rpaths used when resolving ``@rpath/`` references in the
        library.

        In addition to the ``LC_RPATH`` load commands contained within the
        library, rpaths in the binaries which loaded the library are
        referenced. These are included in the property.
        '''
        if self._rpaths is None:
            rpaths = []
            if self._parent is not None:
                rpaths.extend(self._parent.rpaths)
            get_rpaths = Pipeline([
                    'otool',
                    '-l',
                    self.path,
                ], [
                    'awk',
                    '''
                    $1 == "cmd" {
                        cmd = $2
                    }

                    $1 == "path" {
                        if (cmd == "LC_RPATH") {
                            print $2
                        }
                    }
                    ''',
                ])
            rpaths.extend(get_rpaths().split('\n'))

            # rpaths may contain magic ``@`` references. This property only
            # contains full paths, so we resolve them now.
            resolved_rpaths = []
            for rpath in rpaths:
                if rpath.startswith('@executable_path'):
                    # If the loader does not have an executable path, it is a plugin or
                    # a framework and we trust the executable which loads the plugin to
                    # provide the library instead.
                    if self.executable_path is None:
                        continue
                    resolved_rpaths.append(rpath.replace('@executable_path', self.executable_path))
                elif rpath.startswith('@loader_path'):
                    resolved_rpaths.append(rpath.replace('@loader_path', self.loader_path))
                elif rpath:
                    resolved_rpaths.append(rpath)

            self._rpaths = resolved_rpaths
        return self._rpaths

    def _get_dependencies(self):
        '''Get the dependent library IDs of the library.'''
        pipe = Pipeline([
                'otool',
                '-L',
                self.path,
            ], [
                'sed',
                '-n',
                '-e', '/compatibility version/s/ (compatibility.*)//p',
            ])
        return pipe().split()

    @property
    def dependencies(self):
        '''Dependent libraries of the library.'''
        if self._dependencies is None:
            collection = {}
            for dep in self._get_dependencies():
                deplib = Library.from_reference(dep, self)
                if deplib is not None and \
                   not deplib.path == self.path:
                    collection[dep] = deplib
            self._dependencies = collection
        return self._dependencies

    def _find_library(self, ref):
        '''
        Find a library using search paths.

        Use of this method to find a dependent library indicates that the
        library depdencies are not properly specified. As such, it warns when
        it is used.
        '''
        print('WARNING: dependency from %s to %s requires a search path' % (self.path, ref))
        for loc in self._search_paths:
            path = os.path.join(loc, ref)
            if os.path.exists(path):
                return path
        return ref

    @classmethod
    def from_reference(cls, ref, loader):
        '''Create a library representation given an ID and a loading binary.'''
        paths = [ref]
        if ref.startswith('@executable_path/'):
            # If the loader does not have an executable path, it is a plugin or
            # a framework and we trust the executable which loads the plugin to
            # provide this library instead.
            if loader.executable_path is None:
                return None
            paths.append(ref.replace('@executable_path', loader.executable_path))
        elif ref.startswith('@loader_path/'):
            paths.append(ref.replace('@loader_path', loader.loader_path))
        elif ref.startswith('@rpath/'):
            for rpath in loader.rpaths:
                paths.append(ref.replace('@rpath', rpath))
        paths.append(os.path.join(os.path.dirname(loader.path), ref))
        for path in paths:
            if os.path.exists(path):
                return cls.from_path(os.path.realpath(path), parent=loader)
        if loader.ignores:
            for ignore in loader.ignores:
                if ignore.match(ref):
                    return None
        if ref.startswith('/System/Library/Frameworks/') or \
           ref.startswith('/usr/lib/'):
            # These files do not exist on-disk as of macOS 11. This is Apple
            # magic and assumed to be a system library.
            return None
        search_path = loader._find_library(ref)
        if os.path.exists(search_path):
            return cls.from_path(os.path.realpath(search_path), parent=loader)
        raise RuntimeError('Unable to find the %s library from %s' % (ref, loader.path))

    __cache = {}

    @classmethod
    def from_path(cls, path, parent=None):
        '''Create a library representation from a path.'''
        if not os.path.exists(path):
            raise RuntimeError('%s does not exist' % path)

        path = os.path.normpath(path)
        if path not in cls.__cache:
            search_paths = None
            if parent is not None:
                search_paths = parent._search_paths

            cls.__cache[path] = Library(path, parent=parent,
                                        search_paths=search_paths)

        return cls.__cache[path]

    @classmethod
    def from_manifest(cls, path, installed_id):
        '''Create a library representation from a cached manifest entry.'''
        if path in cls.__cache:
            raise RuntimeError('There is already a library for %s' % path)

        library = Library(path)
        library.set_installed_id(installed_id)

        cls.__cache[path] = library
        return cls.__cache[path]


class Executable(Library):
    '''
    The main executable of an ``.app`` bundle.

    A specialization of a library which lives in ``Contents/MacOS`` inside of a
    ``.app`` bundle.
    '''

    def __init__(self, path, **kwargs):
        super(Executable, self).__init__(path, None, **kwargs)

        self._executable_path = os.path.dirname(path)

    @property
    def bundle_location(self):
        return 'Contents/MacOS'

    @property
    def dependent_reference(self):
        return '@executable_path/..'


class Utility(Executable):
    '''
    A utility executable in an ``.app`` bundle.

    A specialization of a library which lives in ``Contents/bin`` inside of a
    ``.app`` bundle. These are not the executables which run when opening a
    ``.app``, but they are available for use by the main executable or via the
    command line.
    '''

    def __init__(self, path, **kwargs):
        super(Utility, self).__init__(path, **kwargs)

    @property
    def bundle_location(self):
        return 'Contents/bin'


class Plugin(Library):
    '''
    A plugin library in an ``.app`` bundle.

    These libraries live under the ``Contents/Plugins`` directory in an
    ``.app`` bundle. They are expected to be loaded by an executable and use
    ``@executable_path/`` references where possible, but for any libraries
    required by the plugin and not otherwise provided, ``@loader_path/`` is
    used instead.

    Some plugins may require to be considered as their own ``@executable_path``
    reference. This may indicate errors in the building of the plugin.
    '''

    def __init__(self, path, fake_exe_path=False, **kwargs):
        super(Plugin, self).__init__(path, None, **kwargs)

        if fake_exe_path:
            self._executable_path = os.path.dirname(path)

    @property
    def bundle_location(self):
        return 'Contents/Plugins'

    @property
    def dependent_reference(self):
        return '@loader_path/..'


class Module(Library):
    '''
    A library loaded programmatically at runtime.

    Modules are usually used by interpreted languages (as opposed to compiled
    languages) and loaded at runtime. They may live at any depth in a ``.app``
    bundle.

    Currently it is assumed that the only executables which will load these
    modules is a binary in the same ``.app`` bundle. It is unknown if this
    assumption is actually valid and documentation is scarce.

    Some modules may require to be considered as their own ``@executable_path``
    reference. This may indicate errors in the building of the module.
    '''
    def __init__(self, path, bundle_location, fake_exe_path=False, **kwargs):
        super(Module, self).__init__(path, None, **kwargs)

        self._bundle_location = bundle_location

        if fake_exe_path:
            self._executable_path = path
            for _ in range(self.bundle_location.count('/')):
                self._executable_path = os.path.dirname(self._executable_path)

        parent_parts = ['..'] * self.bundle_location.count('/')
        self._dependent_reference = os.path.join('@loader_path', *parent_parts)

    @property
    def bundle_location(self):
        return self._bundle_location

    @property
    def dependent_reference(self):
        return '@executable_path/..'
        # XXX(modules): is this right? should modules ever not be loaded by
        # their owning application?
        #return self._dependent_reference


class Framework(Library):
    '''
    A ``.framework`` library.

    Currently unsupported. Documentation for frameworks needs to be found and
    understood to find out what it really means to install a framework (as a
    usable framework).
    '''

    def __init__(self, path, **kwargs):
        super(Framework, self).__init__(path, None, **kwargs)

        raise RuntimeError('Framework support is unimplemented.')

    @property
    def bundle_location(self):
        return 'Contents/Frameworks'

    @property
    def dependent_reference(self):
        return '@loader_path/..' # FIXME: ???


def copy_library(destination, library, dry_run=False, library_dest='Libraries', framework_dest='Frameworks'):
    '''Copy a library into the ``.app`` bundle.'''
    if library.is_framework:
        # Frameworks go into Contents/<framework_dest>.
        print('Copying %s/%s ==> Contents/%s' % (library.framework_path, library.framework_name, framework_dest))

        app_dest = os.path.join(destination, 'Contents', framework_dest)
        binary = os.path.join(app_dest, library.framework_name, library.framework_library)
        library.set_installed_id(os.path.join('@executable_path', '..', framework_dest, library.framework_name, library.framework_library))
        destination = os.path.join(app_dest, library.framework_name)

        if not dry_run:
            # TODO: This could be optimized to only copy the particular
            # version.
            if os.path.exists(destination):
                shutil.rmtree(destination)
            _os_makedirs(app_dest)
            shutil.copytree(os.path.join(library.framework_path, library.framework_name), destination, symlinks=True)

            # We need to make sure the copied libraries are writable.
            chmod = Pipeline([
                'chmod',
                '-R',
                'u+w',
                destination,
            ])
            chmod()

    else:
        # Libraries go into Contents/<library_dest>.
        print('Copying %s ==> Contents/%s' % (library.path, library_dest))

        app_dest = os.path.join(destination, 'Contents', library_dest)
        binary = os.path.join(app_dest, library.name)
        # FIXME(plugins, frameworks): fix the installed id of the library based
        # on what drags it in.
        library.set_installed_id(os.path.join('@executable_path', '..', library_dest, library.name))
        destination = app_dest

        if not dry_run:
            _os_makedirs(app_dest)
            shutil.copy(library.path, destination)

            # We need to make the library after copying it.
            chmod = Pipeline([
                    'chmod',
                    'u+w',
                    os.path.join(destination, os.path.basename(library.path)),
                ])
            chmod()

        # Create any symlinks we found for the library as well.
        for symlink in library.symlinks:
            print('Creating symlink to Contents/%s/%s ==> %s' % (library_dest, library.name, symlink))
            if not dry_run:
                symlink_path = os.path.join(app_dest, symlink)
                if os.path.exists(symlink_path):
                    os.remove(symlink_path)
                ln = Pipeline([
                        'ln',
                        '-s',
                        library.name,
                        symlink_path,
                    ])
                ln()

    return binary


def _os_makedirs(path):
    '''
    A function to fix up the fact that os.makedirs chokes if the path already
    exists.
    '''
    if os.path.exists(path):
        return
    os.makedirs(path)


def _arg_parser():
    import argparse

    parser = argparse.ArgumentParser(description='Install an OS X binary into a bundle')
    parser.add_argument('-b', '--bundle', metavar='BUNDLE', type=str, required=True,
                        help='the name of the application (including .app extension)')
    parser.add_argument('-d', '--destination', metavar='DEST', type=str, required=True,
                        help='the directory to create the bundle underneath')
    parser.add_argument('-i', '--include', metavar='REGEX', action='append',
                        default=[],
                        help='regular expression to include in the bundle (before exclusions)')
    parser.add_argument('-I', '--ignore', metavar='REGEX', action='append',
                        default=[],
                        help='regular expression to ignore references')
    parser.add_argument('-e', '--exclude', metavar='REGEX', action='append',
                        default=[],
                        help='regular expression to exclude from the bundle')
    parser.add_argument('-p', '--plugin', metavar='PATH', action='append',
                        default=[], dest='plugins',
                        help='list of plugins to install with an executable')
    parser.add_argument('--library', metavar='PATH', action='append',
                        default=[], dest='libraries',
                        help='list of additional libraries to install with an executable')
    parser.add_argument('-s', '--search', metavar='PATH', action='append',
                        default=[],
                        help='add a directory to search for dependent libraries')
    parser.add_argument('-n', '--dry-run', action='store_true',
                        help='do not actually copy files')
    parser.add_argument('-c', '--clean', action='store_true',
                        help='clear out the bundle destination before starting')
    parser.add_argument('-N', '--new', action='store_true',
                        help='start a new manifest')
    parser.add_argument('-l', '--location', metavar='PATH', type=str,
                        help='where to place a module within the bundle')
    parser.add_argument('-m', '--manifest', metavar='PATH', type=str, required=True,
                        help='manifest for the application bundle')
    parser.add_argument('-t', '--type', metavar='TYPE', type=str, required=True,
                        choices=('executable', 'utility', 'plugin', 'module'),
                        help='the type of binary to package')
    parser.add_argument('-L', '--library-dest', metavar='PATH', type=str,
                        default='Libraries',
                        help='where to place libraries in the bundle')
    parser.add_argument('-F', '--framework-dest', metavar='PATH', type=str,
                        default='Frameworks',
                        help='where to place frameworks in the bundle')
    # This flag is here so that plugins which are built for one application can
    # bring in the required libraries for another application which doesn't
    # have a superset of the libraries required by the plugin provided by its
    # main application.
    parser.add_argument('--fake-plugin-paths', action='store_true',
                        help='if given, plugins will act as their own @executable_path')
    parser.add_argument('binary', metavar='BINARY', type=str,
                        help='the binary to package')

    return parser


def _install_binary(binary, is_excluded, bundle_dest, installed, manifest, dry_run=False, library_dest='Libraries', framework_dest='Frameworks'):
    '''Install the main binary into the package.'''
    # Start looking at our main executable's dependencies.
    deps = list(binary.dependencies.values())
    while deps:
        dep = deps.pop(0)

        # Ignore dependencies which the bundle already provides.
        if dep.path in manifest:
            continue

        # Ignore dependencies we don't care about.
        if is_excluded(dep.path):
            continue

        # If we've already installed this dependency for some other library,
        # skip it.
        if dep.path in installed:
            continue

        # Add this dependency's dependencies to the pile.
        deps.extend(dep.dependencies.values())
        # Remember what we installed and where.
        installed[dep.path] = (dep, copy_library(bundle_dest, dep, dry_run=dry_run, library_dest=library_dest, framework_dest=framework_dest))

    # Install the main executable itself.
    app_dest = os.path.join(bundle_dest, binary.bundle_location)
    binary_destination = os.path.join(app_dest, os.path.basename(binary.path))
    installed[binary.path] = (binary, binary_destination)
    binary.set_installed_id(binary_destination)
    print('Copying %s ==> %s' % (binary.path, binary.bundle_location))
    if not dry_run:
        _os_makedirs(app_dest)
        shutil.copy(binary.path, app_dest)


def _fix_installed_binaries(installed, dry_run=False):
    '''
    This function updates all of the installed binaries to use consistent
    library IDs when referring to each other.
    '''
    # Go through all of the binaries installed and fix up references to other things.
    for binary_info in installed.values():
        binary, installed_path = binary_info
        print('Fixing binary references in %s' % binary.path)

        if not dry_run and binary.installed_id:
            # Set the ID on the binary.
            install_name_tool = Pipeline([
                    'install_name_tool',
                    '-id', os.path.join(binary.installed_id, os.path.basename(installed_path)),
                    installed_path,
                ])
            install_name_tool()

        changes = []
        for old_name, library in binary.dependencies.items():
            if library.installed_id is not None and \
               not old_name == library.installed_id:
                changes.extend(['-change', old_name, library.installed_id])

        # Fix up the library names.
        if not dry_run and changes:
            install_name_tool = Pipeline([
                    'install_name_tool',
                ] + changes + [
                    installed_path,
                ])
            install_name_tool()


def _update_manifest(manifest, installed, path):
    '''Update the manifest file with a set of newly installed binaries.'''
    for input_path, binary_info in installed.items():
        binary, _ = binary_info
        manifest[input_path] = binary.installed_id

    with open(path, 'w+') as fout:
        json.dump(manifest, fout)


def main(args):
    parser = _arg_parser()
    opts = parser.parse_args(args)

    ignores = list(map(re.compile, opts.ignore))

    if opts.type == 'executable':
        main_exe = Executable(opts.binary, search_paths=opts.search, ignores=ignores)
    elif opts.type == 'utility':
        main_exe = Utility(opts.binary, search_paths=opts.search, ignores=ignores)
    elif opts.type == 'plugin':
        main_exe = Plugin(opts.binary, search_paths=opts.search, ignores=ignores)
    elif opts.type == 'module':
        if opts.location is None:
            raise RuntimeError('Modules require a location')

        main_exe = Module(opts.binary, opts.location,
                          fake_exe_path=opts.fake_plugin_paths,
                          search_paths=opts.search, ignores=ignores)
    elif opts.type == 'framework':
        main_exe = Framework(opts.binary, search_paths=opts.search, ignores=ignores)

    bundle_dest = os.path.join(opts.destination, opts.bundle)

    # Remove the old bundle.
    if not opts.dry_run and opts.clean and os.path.exists(bundle_dest):
        shutil.rmtree(bundle_dest)

    includes = list(map(re.compile, opts.include))
    excludes = list(map(re.compile, opts.exclude))

    def is_excluded(path):
        # Filter by regex
        for include in includes:
            if include.match(path):
                return False
        for exclude in excludes:
            if exclude.match(path):
                return True

        # Apple
        if path.startswith('/System/Library'):
            return True
        if path.startswith('/usr/lib'):
            return True

        # Homebrew
        if path.startswith('/usr/local/lib'):
            return False

        # Macports
        if path.startswith('/opt/local/lib'):
            return False

        return False

    if opts.new:
        # A new bundle does not have a manifest.
        manifest = {}
    else:
        with open(opts.manifest, 'r') as fin:
            manifest = json.load(fin)

        # Seed the cache with manifest entries.
        for path, installed_id in manifest.items():
            Library.from_manifest(path, installed_id)

    installed = {}
    _install_binary(main_exe, is_excluded, bundle_dest, installed, manifest, dry_run=opts.dry_run, library_dest=opts.library_dest, framework_dest=opts.framework_dest)

    for plugin in opts.plugins:
        plugin_bin = Plugin(plugin, fake_exe_path=opts.fake_plugin_paths, search_paths=opts.search)
        _install_binary(plugin_bin, is_excluded, bundle_dest, installed, manifest, dry_run=opts.dry_run, library_dest=opts.library_dest, framework_dest=opts.framework_dest)

    for library in opts.libraries:
        library_bin = Module(library, 'Contents/Libraries', search_paths=opts.search)
        _install_binary(library_bin, is_excluded, bundle_dest, installed, manifest, dry_run=opts.dry_run)
        library_bin.set_installed_id('@executable_path/../Libraries')

    _fix_installed_binaries(installed, dry_run=opts.dry_run)

    if not opts.dry_run:
        _update_manifest(manifest, installed, opts.manifest)


if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
