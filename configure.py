#!/usr/bin/env python3 
import sys
from pathlib import Path

from ninja import Writer


# This is the config section, all flags should be written here.
# cxx: the C++ compiler to use
# cxx_flags: the C++ flags to compile with
# ld_flags: the linker flags to use
#
# build_dir: the build directory
# inputs: list of input C++ files


cxx = "clang++"
cxx_flags = "-g -Wall -O3 -fopenmp=libomp -march=native -flto"
ld_flags = "-flto"

build_dir = "build"
inputs = [
    "Magic.cpp",
    "Mario.cpp",
    "Platform.cpp",
    "Surface.cpp",
    "Trig.cpp",
    "vmath.cpp",
    "main.cpp"
]

# END OF CONFIGURATION

# platform executable extensions
EXE_EXTS = {
    "linux": "",
    "aix": "",
    "darwin": ".app",
    "cygwin": ".exe",
    "win32": ".exe"
}


# link with libm on POSIX systems
if (sys.platform in ['linux', 'darwin', 'aix', 'cygwin']) and not ("-lm" in ld_flags):
    ld_flags += " -lm"

with open("build.ninja", "w") as f:
    file = Writer(f)

    # make rules

    file.variable("cxx", cxx)
    file.variable("cxx_flags", cxx_flags)
    file.variable("ld_flags", ld_flags)
    
    file.variable("build_dir", build_dir)

    file.newline()
    file.rule("regen", "python3 configure.py",
        description="Regenerating build script..."
    )
    
    file.rule("compile", "$cxx $cxx_flags -MMD -MF $out.d -c $in -o $out", 
        depfile="$out.d",
        description="Compiling $out"
    )
    file.rule("link", "$cxx $cxx_flags -o $out $in $ld_flags",
        description="Linking $out"
    )
    file.rule("run_shell", f"{'cmd /c ' if sys.platform == 'win32' else ''}\"$shellcmd\"",
        description="$desc",
        restat=True
    )
    
    file.build("always", "phony")
    
    file.newline()
    file.build("build.ninja", "regen", "configure.py")
    
    file.newline()
    for i in inputs:
        file.build(f"$build_dir/{i[:i.rfind('.')]}.o", "compile", i)
    
    file.newline()
    file.build(f"$build_dir/main{EXE_EXTS[sys.platform]}", "link", [f"$build_dir/{i[:i.rfind('.')]}.o" for i in inputs])
    
    file.newline()
    file.default(paths=f"$build_dir/main{EXE_EXTS[sys.platform]}")
    
    file.newline()
    file.comment("Run targets")
    
    file.newline()
    file.build("clean", "run_shell", "always",
        variables={
            "shellcmd": f"cd $build_dir && {'del' if sys.platform == 'win32' else 'rm'} *.o main{EXE_EXTS[sys.platform]}",
            "desc": "Deleting build files..."
        }
    )