#!/usr/bin/env python2.7

'''
A tool to install PE-COFF binaries into in installation prefix.
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
    perform its job. It tries to implement the behavior of the Windows runtime
    loader, but documentation is scarce.

    Basically, the approach is to rely on the location of loading libraries and
    the magic of the ``PATH`` environment variable.
    '''

    def __init__(self, path, parent=None, ignore=[], search_paths=None):
        # This is the actual path to a physical file
        self._path = os.path.normpath(path)

        if search_paths is None:
            self._search_paths = []
        else:
            self._search_paths = search_paths

        self._parent = parent
        self._ignore = ignore
        self._dependencies = None
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
    def loader_path(self):
        '''The path to use for ``@loader_path`` references from the library.'''
        return os.path.dirname(self.path)

    def _get_dependencies(self):
        '''
        Get the dependent libraries of the library.

        This only extracts the required libraries. Libraries which are marked
        as "delay load" libraries are ignored and will not be installed.
        '''
        pipe = Pipeline([
                'dumpbin',
                '/DEPENDENTS',
                self.path,
            ])

        deps = []
        for line in pipe().splitlines():
            line = line.strip()

            # Dependencies start here.
            if line == 'Image has the following dependencies:':
                deps = []
            elif line == 'Image has the following delay load dependencies:':
                return deps
            elif line.startswith('Dump of file'):
                continue
            elif line.endswith('.dll'):
                deps.append(line)

        return deps

    @property
    def dependencies(self):
        '''
        Dependent libraries of the library.

        Visual C runtime libraries are ignored.
        '''
        if self._dependencies is None:
            system_dlls = re.compile(r'[a-z]:\\windows\\system.*\.dll', re.IGNORECASE)
            if system_dlls.match(self.path):
                # if this is a system dll, stop traversing dependencies.
                self._dependencies = {}
                return self._dependencies
            collection = {}
            msvc_runtimes = re.compile('MSVC[A-Z][0-9]*\\.dll')
            vc_runtimes = re.compile('VC[A-Z][0-9]*\\.dll')
            win_core_runtimes = re.compile('api-ms-win-core-.*\\.dll')
            win_rt_runtimes = re.compile('api-ms-win-crt-.*\\.dll')
            for dep in self._get_dependencies():
                if msvc_runtimes.match(dep):
                    continue
                if vc_runtimes.match(dep):
                    continue
                if win_core_runtimes.match(dep):
                    continue
                if win_rt_runtimes.match(dep):
                    continue
                if dep in self._ignore:
                    continue
                deplib = Library.from_reference(dep, self, ignore=self._ignore)
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
        for loc in self._search_paths:
            path = os.path.join(loc, ref)
            if os.path.exists(path):
                return path
        return ref

    __search_cache = None

    @classmethod
    def from_reference(cls, ref, loader, **kwargs):
        '''Create a library representation given a name and a loading binary.'''
        paths = []

        # Use the loader's location.
        paths.append(loader.loader_path)

        # Use the loader's search paths.
        if loader._search_paths:
            paths += loader._search_paths

        # Use PATH.
        ld_library_path = os.environ.get('PATH')
        if ld_library_path is not None:
            paths.extend(ld_library_path.split(';'))

        for path in paths:
            libpath = os.path.join(path, ref)
            if os.path.exists(libpath):
                return cls.from_path(libpath, parent=loader, **kwargs)

        search_path = loader._find_library(ref)
        if os.path.exists(search_path):
            return cls.from_path(search_path, parent=loader, **kwargs)
        raise RuntimeError('Unable to find the %s library from %s: %s' % (ref, loader.path, ', '.join(paths)))

    __cache = {}

    @classmethod
    def from_path(cls, path, parent=None, **kwargs):
        '''Create a library representation from a path.'''
        if not os.path.exists(path):
            raise RuntimeError('%s does not exist' % path)

        path = os.path.normpath(path)
        if path not in cls.__cache:
            search_paths = None
            if parent is not None:
                search_paths = parent._search_paths

            cls.__cache[path] = Library(path, parent=parent,
                                        search_paths=search_paths, **kwargs)

        return cls.__cache[path]

    @classmethod
    def from_manifest(cls, path):
        '''Create a library representation from a cached manifest entry.'''
        if path in cls.__cache:
            raise RuntimeError('There is already a library for %s' % path)

        library = Library(path)
        library._dependencies = {}
        library._is_cached = True

        cls.__cache[path] = library
        return cls.__cache[path]


class Module(Library):
    '''
    A library loaded programmatically at runtime.

    Modules are loaded at runtime using ``LoadLibrary``.
    '''

    def __init__(self, path, bundle_location, **kwargs):
        super(Module, self).__init__(path, None, **kwargs)

        self._bundle_location = bundle_location

    @property
    def bundle_location(self):
        return self._bundle_location


class Executable(Module):
    '''
    An executable in the installation.
    '''

    def __init__(self, path, **kwargs):
        super(Executable, self).__init__(path, 'bin', **kwargs)


def copy_library(destination, bundle_dest, library, dry_run=False):
    '''Copy a library into the ``.app`` bundle.'''
    if library._is_cached:
        return

    print('Copying %s ==> %s' % (library.path, bundle_dest))

    app_dest = os.path.join(destination, bundle_dest)
    binary = os.path.join(app_dest, library.name)
    destination = app_dest

    if not dry_run:
        _os_makedirs(app_dest)
        shutil.copy(library.path, destination)

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

    parser = argparse.ArgumentParser(description='Install an ELF binary into a bundle')
    parser.add_argument('-d', '--destination', metavar='DEST', type=str, required=True,
                        help='the directory to create the bundle underneath')
    parser.add_argument('-i', '--include', metavar='REGEX', action='append',
                        default=[],
                        help='regular expression to include in the bundle (before exclusions)')
    parser.add_argument('-e', '--exclude', metavar='REGEX', action='append',
                        default=[],
                        help='regular expression to exclude from the bundle')
    parser.add_argument('-I', '--ignore', metavar='DLLNAME', action='append',
                        default=[],
                        help='DLL names to ignore in the dependency list')
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
    parser.add_argument('-B', '--binary-libdir', metavar='PATH', type=str, action='append', default=[],
                        help='location the binary will search for libraries')
    parser.add_argument('-L', '--libdir', metavar='PATH', type=str,
                        help='location to put dependent libraries')
    parser.add_argument('-m', '--manifest', metavar='PATH', type=str, required=True,
                        help='manifest for the application bundle')
    parser.add_argument('-t', '--type', metavar='TYPE', type=str, required=True,
                        choices=('executable', 'module'),
                        help='the type of binary to package')
    parser.add_argument('binary', metavar='BINARY', type=str,
                        help='the binary to package')

    return parser


def _install_binary(binary, is_excluded, bundle_dest, dep_libdir, installed, manifest, dry_run=False):
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
        installed[dep.path] = (dep, copy_library(bundle_dest, dep_libdir, dep, dry_run=dry_run))

    # Install the main executable itself.
    app_dest = os.path.join(bundle_dest, binary.bundle_location)
    binary_destination = os.path.join(app_dest, os.path.basename(binary.path))
    installed[binary.path] = (binary, binary_destination)
    print('Copying %s ==> %s' % (binary.path, binary.bundle_location))
    if not dry_run:
        _os_makedirs(app_dest)
        shutil.copy(binary.path, app_dest)


def _update_manifest(manifest, installed, path, location):
    '''Update the manifest file with a set of newly installed binaries.'''
    for input_path in installed.keys():
        manifest.setdefault(location, []).append(input_path)

    for k, v in manifest.items():
        manifest[k] = list(set(v))

    with open(path, 'w+') as fout:
        json.dump(manifest, fout)


def main(args):
    parser = _arg_parser()
    opts = parser.parse_args(args)

    if opts.type == 'executable':
        main_exe = Executable(opts.binary, ignore=opts.ignore, search_paths=opts.search)
        libdir = main_exe.bundle_location
    elif opts.type == 'module':
        if opts.location is None:
            raise RuntimeError('Modules require a location')

        main_exe = Module(opts.binary, opts.location, ignore=opts.ignore,
                          search_paths=opts.search)
        if opts.libdir is None:
            libdir = main_exe.bundle_location
        else:
            libdir = opts.libdir

    bundle_dest = opts.destination

    # Remove the old bundle.
    if not opts.dry_run and opts.clean and os.path.exists(bundle_dest):
        shutil.rmtree(bundle_dest)

    includes = list(map(re.compile, opts.include))
    excludes = list(map(re.compile, opts.exclude))
    system_dlls = re.compile(r'[a-z]:\\windows\\system.*\.dll', re.IGNORECASE)

    def is_excluded(path):
        # Filter by regex
        for include in includes:
            if include.match(path):
                return False
        for exclude in excludes:
            if exclude.match(path):
                return True
        # System libs
        if system_dlls.match(path):
            return True
        return False

    if opts.new:
        # A new bundle does not have a manifest.
        manifest = {}
    else:
        with open(opts.manifest, 'r') as fin:
            manifest = json.load(fin)

        manifest_dirs = set(opts.binary_libdir + [libdir,])
        for mdir in manifest_dirs:
            for path in manifest.get(mdir, []):
                Library.from_manifest(path)

    cur_manifest = manifest.setdefault(opts.libdir, [])

    installed = {}
    _install_binary(main_exe, is_excluded, bundle_dest, libdir, installed, manifest, dry_run=opts.dry_run)

    if not opts.dry_run:
        _update_manifest(manifest, installed, opts.manifest, libdir)


if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
