import os
import sys

supported_modes_ = {
    'debug': 'Debug',
    'release': 'Release',
}

def build(mode):
    build_path = f'build/{mode}'
    os.makedirs(build_path, exist_ok=True)
    os.chdir(build_path)
    os.system(f'cmake -DPYTHON_EXECUTABLE={sys.executable} -DCMAKE_BUILD_TYPE={supported_modes_[mode]} -DCMAKE_INSTALL_PREFIX=../../pyvframe ../../cpp')
    os.system('make install')

if __name__ == '__main__':
    mode = sys.argv[1] # debug or release
    build(mode)
