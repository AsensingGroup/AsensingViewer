#!/usr/bin/env python2.7

'''
A tool to install ELF binaries into in installation prefix.
'''


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
    perform its job. It tries to implement the behavior of ``ld.so(8)`` as
    closely as possible.
    '''

    def __init__(self, path, parent=None, search_paths=None):
        # This is the actual path to a physical file
        self._path = os.path.normpath(path)

        if search_paths is None:
            self._search_paths = []
        else:
            self._search_paths = search_paths

        self._parent = parent
        self._symlinks = None
        self._dependencies = None
        self._rpaths = None
        self._runpaths = None
        self._is_cached = False

    def __hash__(self):
        return self._path.__hash__()

    def __eq__(self, other):
        return self._path == other._path

    def __repr__(self):
        return 'Library(%s : %s)' % (self.id, self.path)

    @property
    def is_cached(self):
        '''Whether the library has already been installed or not.'''
        return self._is_cached

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
                    '-maxdepth', '1',
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
    def loader_path(self):
        '''The path to use for ``@loader_path`` references from the library.'''
        return os.path.dirname(self.path)

    @property
    def extra_loader_paths(self):
        '''
        Extra paths which should be looked at because of a loader's search
        path.
        '''
        return []

    @property
    def loader_paths(self):
        '''
        A list of paths to look for libraries due to where the loading
        libraries look.
        '''
        loader_paths = [self.loader_path]
        loader_paths.extend(self.extra_loader_paths)
        if self.parent is not None:
            loader_paths.extend(self.parent.loader_paths)
        return loader_paths

    def _resolve_rpath(self, rpath):
        '''Resolve an rpath which may contain a special token reference.'''
        if rpath.startswith('$ORIGIN/'):
            rpath = rpath.replace('$ORIGIN', self.loader_path)
        elif rpath.startswith('${ORIGIN}/'):
            rpath = rpath.replace('${ORIGIN}', self.loader_path)

        # TODO: Handle $LIB and $PLATFORM. These really shouldn't be used in
        # superbuilds however; libraries should all be under a plain ``lib``
        # directory.

        return rpath.strip()

    @property
    def rpaths(self):
        '''
        The list of rpaths used when resolving required library references.

        In addition to the ``DT_RPATH`` entries contained within the library,
        rpaths in the binaries which loaded the library are referenced. These
        are included in the property.
        '''
        if self._rpaths is None:
            rpaths = []
            get_rpaths = Pipeline([
                    'objdump',
                    '-p',
                    self.path,
                ], [
                    'awk',
                    '''
                    $1 == "RUNPATH" {
                        exit
                    }

                    $1 == "RPATH" {
                        rpath_entries[LINE] = $2
                    }

                    END {
                        for (entry_idx in rpath_entries) {
                            split(rpath_entries[entry_idx], rpaths, /:/)
                            for (idx in rpaths) {
                                path = rpaths[idx]
                                if (path) {
                                    print path
                                }
                            }
                        }
                    }
                    ''',
                ])
            rpaths.extend(get_rpaths().split('\n'))

            self._rpaths = list(filter(lambda x: x, map(self._resolve_rpath, rpaths)))
        return self._rpaths

    @property
    def runpaths(self):
        '''
        The list of runpaths used when resolving required library references.

        These are specified by the library using ``DT_RUNPATH`` entries. Unlike
        ``DT_RPATH``, these entries are not inherited by loaded libraries.
        '''
        if self._runpaths is None:
            runpaths = []
            get_runpaths = Pipeline([
                    'objdump',
                    '-p',
                    self.path,
                ], [
                    'awk',
                    '''
                    $1 == "RUNPATH" {
                        print $2
                    }
                    ''',
                ])
            runpaths.extend(get_runpaths().split(':'))

            self._runpaths = list(filter(lambda x: x, map(self._resolve_rpath, runpaths)))
        return self._runpaths

    def _get_dependencies(self):
        '''
        Get the dependent libraries of the library.

        These are specified by ``DT_NEEDED`` entries and are specified as
        soname values.
        '''
        pipe = Pipeline([
                'objdump',
                '-p',
                self.path,
            ], [
                'awk',
                '''
                $1 == "NEEDED" {
                    print $2
                }
                ''',
            ])
        return pipe().split()

    @property
    def dependencies(self):
        '''Dependent libraries of the library.'''
        if self._dependencies is None:
            collection = {}
            for dep in self._get_dependencies():
                deplib = Library.from_reference(dep, self)
                if deplib is not None:
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

    __search_cache = None

    @classmethod
    def default_search_paths(cls):
        '''A list of search paths implicit to the system.'''
        if cls.__search_cache is None:
            pipe = Pipeline([
                    '/sbin/ldconfig',
                    '-v',
                    '-N', # Don't rebuild the cache.
                    '-X', # Don't update links.
                ], [
                    'grep',
                    '-v',
                    '^\t',
                ], [
                    'cut',
                    '-d:',
                    '-f1',
                ])
            cls.__search_cache = pipe().split('\n')

        return cls.__search_cache

    @classmethod
    def from_reference(cls, ref, loader):
        '''Create a library representation given a soname and a loading binary.'''
        paths = []

        if '/' in ref:
            if os.path.exists(ref):
                return cls.from_path(ref, parent=loader)
        else:
            # TODO: Automatically search in the libdir we are placing dependent libraries.

            paths.extend(loader.loader_paths)

            # Find the path via rpath.
            paths.extend(loader.rpaths)

            # Use LD_LIBRARY_PATH.
            ld_library_path = os.environ.get('LD_LIBRARY_PATH')
            if ld_library_path is not None:
                paths.extend(ld_library_path.split(':'))

            # Find the path via runpath.
            paths.extend(loader.runpaths)

            # Find the library in the default library search paths.
            paths.extend(cls.default_search_paths())

            for path in paths:
                libpath = os.path.join(path, ref)
                if os.path.exists(libpath):
                    return cls.from_path(libpath, parent=loader)

        search_path = loader._find_library(ref)
        if os.path.exists(search_path):
            return cls.from_path(search_path, parent=loader)
        raise RuntimeError('Unable to find the %s library from %s: %s' % (ref, loader.path, ', '.join(paths)))

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
    def from_manifest(cls, path):
        '''Create a library representation from a cached manifest entry.'''
        if path in cls.__cache:
            raise RuntimeError('There is already a library for %s' % path)

        library = Library(path)
        library._dependencies = {}
        library._symlinks = []
        library._is_cached = True

        cls.__cache[path] = library
        return cls.__cache[path]


class Module(Library):
    '''
    A library loaded programmatically at runtime.

    Modules are loaded at runtime using ``dlopen``. They may contain extra
    paths to search that are implicit to their loading executable.
    '''

    def __init__(self, path, bundle_location, loader_paths=None, **kwargs):
        super(Module, self).__init__(path, None, **kwargs)

        if loader_paths is None:
            self._extra_loader_paths = []
        else:
            self._extra_loader_paths = loader_paths
        self._bundle_location = bundle_location
        self._is_plugin = False

    @property
    def is_plugin(self):
        return self._is_plugin

    def set_is_plugin(self):
        self._is_plugin = True

    @property
    def extra_loader_paths(self):
        return self._extra_loader_paths

    @property
    def bundle_location(self):
        return self._bundle_location


class Executable(Module):
    '''
    An executable in the installation.
    '''

    def __init__(self, path, **kwargs):
        super(Executable, self).__init__(path, 'bin', **kwargs)


def copy_library(destination, libdir, library, sources, dry_run=False):
    '''Copy a library into the ``.app`` bundle.'''
    if library._is_cached:
        return

    print('Copying %s ==> %s' % (library.path, libdir))

    app_dest = os.path.join(destination, libdir)
    binary = os.path.join(app_dest, library.name)
    destination = app_dest

    if not dry_run:
        _os_makedirs(app_dest)
        shutil.copy(library.path, destination)

    for symlink in library.symlinks:
        print('Creating symlink to %s/%s ==> %s' % (libdir, library.name, symlink))
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

    if not dry_run:
        # We need to make the library writable first.
        chmod = Pipeline([
                'chmod',
                'u+w',
                binary,
            ])
        chmod()
        if library.rpaths or library.runpaths:
            remove_prefix_rpaths(binary, libdir, sources)

    return binary


# From https://stackoverflow.com/questions/3812849/how-to-check-whether-a-directory-is-a-sub-directory-of-another-directory#18115684
def is_subdir(path, directory):
    path = os.path.realpath(path)
    directory = os.path.realpath(directory)
    relative = os.path.relpath(path, directory)
    return not (relative == os.pardir or relative.startswith(os.pardir + os.sep))


HAVE_CHRPATH = None


def remove_prefix_rpaths(binary, location, sources):
    '''Remove rpaths which reference the build machine.'''
    if not sources:
        return

    global HAVE_CHRPATH
    if HAVE_CHRPATH is None:
        which = Pipeline([
            'which',
            'chrpath',
        ])
        try:
            which()
            HAVE_CHRPATH = True
        except RuntimeError:
            print('No chrpath found; superbuild rpaths may still exist in the package')
            HAVE_CHRPATH = False
    if not HAVE_CHRPATH:
        return

    chrpath = Pipeline([
        'chrpath',
        '--list',
        binary,
    ])
    old_path = chrpath().strip().split('=')[1]

    path_to_root = ''
    for part in location.split('/'):
        if part:
            path_to_root = os.path.join(path_to_root, '..')
    new_paths = []
    for path in old_path.split(':'):
        for source in sources:
            if is_subdir(path, source):
                path = path.replace(source, os.path.join('$ORIGIN', path_to_root))
            if path not in new_paths:
                new_paths.append(path)

    new_path = ':'.join(new_paths)
    if old_path == new_path:
        return

    if new_path:
        print('Updating the rpath in %s: %s -> %s' % (binary, old_path, new_path))
        chrpath = Pipeline([
            'chrpath',
            '--replace',
            new_path,
            binary,
        ])
    else:
        print('Removing the rpath in %s' % binary)
        chrpath = Pipeline([
            'chrpath',
            '--delete',
            binary,
        ])
    chrpath()


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

    parser = argparse.ArgumentParser(description='Install an ELF binary into a bundle')
    parser.add_argument('-d', '--destination', metavar='DEST', type=str, required=True,
                        help='the directory to create the bundle underneath')
    parser.add_argument('-i', '--include', metavar='REGEX', action='append',
                        default=[],
                        help='regular expression to include in the bundle (before exclusions)')
    parser.add_argument('-e', '--exclude', metavar='REGEX', action='append',
                        default=[],
                        help='regular expression to exclude from the bundle')
    parser.add_argument('-p', '--loader-path', metavar='PATH', action='append',
                        default=[],
                        help='additional location to look for libraries based on the expected loader')
    parser.add_argument('-s', '--search', metavar='PATH', action='append',
                        default=[],
                        help='add a directory to search for dependent libraries')
    parser.add_argument('-S', '--source', metavar='PATH', type=str, action='append',
                        default=[],
                        help='source directory for the binaries')
    parser.add_argument('-o', '--source-only', action='store_true',
                        default=False,
                        help='do not allow files outside of the source path')
    parser.add_argument('-n', '--dry-run', action='store_true',
                        help='do not actually copy files')
    parser.add_argument('-c', '--clean', action='store_true',
                        help='clear out the bundle destination before starting')
    parser.add_argument('-N', '--new', action='store_true',
                        help='start a new manifest')
    parser.add_argument('-l', '--location', metavar='PATH', type=str,
                        help='where to place a module within the bundle')
    parser.add_argument('-L', '--libdir', metavar='PATH', type=str, required=True,
                        help='location to put dependent libraries')
    parser.add_argument('-m', '--manifest', metavar='PATH', type=str, required=True,
                        help='manifest for the application bundle')
    parser.add_argument('-t', '--type', metavar='TYPE', type=str, required=True,
                        choices=('executable', 'module', 'plugin'),
                        help='the type of binary to package')
    parser.add_argument('-k', '--has-symlinks', action='store_true',
                        default=False,
                        help='Look for symlinks of the main binary')
    parser.add_argument('binary', metavar='BINARY', type=str,
                        help='the binary to package')

    return parser


def _install_binary(binary, is_excluded, bundle_dest, dep_libdir, installed, manifest, sources, dry_run=False, look_for_symlinks=False):
    '''Install the main binary into the package.'''
    # Start looking at our main executable's dependencies.
    deps = list(binary.dependencies.values())

    # If the main binary is a plugin, place new dependencies beside it. The
    # manifest contains the list of libraries already installed into `lib` and
    # will be skipped when copying dependencies.
    if isinstance(binary, Module) and binary.is_plugin:
        dep_libdir = binary.bundle_location

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
        installed[dep.path] = (dep, copy_library(bundle_dest, dep_libdir, dep, sources, dry_run=dry_run))

    # Install the main executable itself.
    app_dest = os.path.join(bundle_dest, binary.bundle_location)
    binary_destination = os.path.join(app_dest, os.path.basename(binary.path))
    installed[binary.path] = (binary, binary_destination)
    print('Copying %s ==> %s' % (binary.path, binary.bundle_location))
    if not dry_run:
        _os_makedirs(app_dest)
        shutil.copy(binary.path, app_dest)

        chmod = Pipeline([
                'chmod',
                'u+w',
                binary_destination,
            ])
        chmod()
        if binary.rpaths or binary.runpaths:
            remove_prefix_rpaths(binary_destination, binary.bundle_location, sources)

    if look_for_symlinks:
        for symlink in binary.symlinks:
            print('Creating symlink to %s ==> %s' % (binary_destination, symlink))
            if not dry_run:
                symlink_path = os.path.join(app_dest, symlink)
                if os.path.exists(symlink_path):
                    os.remove(symlink_path)
                ln = Pipeline([
                        'ln',
                        '-s',
                        binary.name,
                        symlink_path,
                    ])
                ln()


def _update_manifest(manifest, installed, path, libdir):
    '''Update the manifest file with a set of newly installed binaries.'''
    for input_path in installed.keys():
        manifest.setdefault(libdir, []).append(input_path)

    for k, v in manifest.items():
        manifest[k] = list(set(v))

    with open(path, 'w+') as fout:
        json.dump(manifest, fout)


def main(args):
    parser = _arg_parser()
    opts = parser.parse_args(args)

    if opts.type == 'executable':
        main_exe = Executable(opts.binary, search_paths=opts.search)
    elif opts.type in ('module', 'plugin'):
        if opts.location is None:
            raise RuntimeError('Modules require a location')

        main_exe = Module(opts.binary, opts.location,
                          loader_paths=opts.loader_path,
                          search_paths=opts.search)
        if opts.type == 'plugin':
            main_exe.set_is_plugin()

    bundle_dest = opts.destination

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

        # Filter by source path
        if opts.source_only:
            for src in opts.source:
                if not path.startswith(src):
                    return True

        # System libs
        if path.startswith('/lib'):
            return True
        if path.startswith('/usr/lib'):
            return True

        # Local installs
        if path.startswith('/usr/local/lib'):
            return True

        return False

    if opts.new:
        # A new bundle does not have a manifest.
        manifest = {}
    else:
        with open(opts.manifest, 'r') as fin:
            manifest = json.load(fin)

        # Seed the cache with manifest entries.
        for path in manifest.get(opts.libdir, []):
            Library.from_manifest(path)

    cur_manifest = manifest.setdefault(opts.libdir, [])

    installed = {}
    _install_binary(main_exe, is_excluded, bundle_dest, opts.libdir, installed, manifest, opts.source, dry_run=opts.dry_run, look_for_symlinks=opts.has_symlinks)

    if not opts.dry_run:
        _update_manifest(manifest, installed, opts.manifest, opts.libdir)


if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
