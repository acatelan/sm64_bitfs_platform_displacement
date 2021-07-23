#!/usr/bin/env python3
from pathlib import Path
import subprocess
import sys
import platform
import os
from textwrap import dedent

proj_dir = Path(__file__).parent

os.chdir(proj_dir)

# https://stackoverflow.com/questions/377017/test-if-executable-exists-in-python/377028#377028
# This is necessary.


if not proj_dir.joinpath("build.ninja").exists():
    subprocess.run(["python3", "configure.py"])
    
try:
    subprocess.run("clang++", stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
except FileNotFoundError:
    print("You need to install clang.")
    print("To install clang:")
    if platform.system() == "Linux":
        # check the system's package manager
        proc = subprocess.run(
            ["lsb_release", "-d"], 
            encoding="utf-8",
            capture_output=True
        )
        desc = proc.stdout;
        desc = desc[desc.find(":") + 1:].strip()
        print(desc)
        # Taken from https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages
        if "Ubuntu" in desc or "Debian" in desc:
            print("Run \"sudo apt-get install clang\"")
        elif "Fedora" in desc.lower():
            print("Run \"sudo dnf install clang\"")
        elif "Arch" in desc:
            print("Run \"pacman -S clang\"")
        else:
            print(dedent("""\
            1. Search for \"clang\" or \"llvm\" on your system's package manager.
            2. Install it.
            """))
    else:
        print("""\
        Go to this link -> https://github.com/llvm/llvm-project/releases/latest
        And find the version of Clang for your system.
        """)
    sys.exit(12)
        

try:
    subprocess.run(["ninja"])
except FileNotFoundError:
    print("You need to install Ninja.")
    print("To install Ninja:")
    if platform.system() == "Windows":
        print(dedent("""
        1. Make sure you have access to pip.
        2. Run "pip install ninja", without the quotes.
        """))
    if platform.system() == "Linux":
        # check the system's package manager
        proc = subprocess.run(
            ["lsb_release", "-d"], 
            encoding="utf-8",
            capture_output=True
        )
        desc = proc.stdout;
        desc = desc[desc.find(":") + 1:].strip()
        # Taken from https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages
        if "Ubuntu" in desc or "Debian" in desc:
            print("Run \"sudo apt-get install ninja-build\" (without the quotes)")
        elif "Fedora" in desc:
            print("Run \"sudo dnf install ninja-build\" (without the quotes)")
        elif "Arch" in desc:
            print("Run \"pacman -S ninja\" (without the quotes)")
        else:
            print(
                "Go to https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages and " +
                "see how you're supposed to install Ninja on your Linux distro. \n" +
                "I was too lazy to put them all in."
            )
    else:
        print(
            "Go to https://github.com/ninja-build/ninja and " +
            "figure out how you can install Ninja. \n" +
            "Note that building outside of Windows and Linux is untested."
        )
    sys.exit(12)