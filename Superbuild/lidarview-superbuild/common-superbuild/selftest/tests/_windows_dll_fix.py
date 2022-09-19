import os
for p in filter(lambda x: x, os.environ.get('PYTHON_WINDOWS_DLL_DIRECTORIES', '').split(';')):
    os.add_dll_directory(p)
