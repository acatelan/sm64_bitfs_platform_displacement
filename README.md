# sm64_bitfs_platform_displacement

A bruteforcer that looks for combinations of platform angle, Mario's speed, and Mario's position that will upwarp him past the pole.

# Building

Here are the dependencies, and how to install them.

Dependency | Windows | Linux (APT) |
:---------|:-------|:-----------|
Clang | Download from [here](https://github.com/llvm/llvm-project/releases/latest) | `sudo apt-get install clang`
Ninja | `pip install ninja` | `sudo apt-get install ninja-build`


Simply run `build.py` in some relatively recent version of Python 3.
If you want to change the flags, they are all available in `configure.py`

# Notes for contributors

If you want to change the build flags, do so in both `build/Makefile` and `configure.py`.