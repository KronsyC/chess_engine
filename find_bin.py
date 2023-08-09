"""

This file is responsible for locating the libchess binary

If you want to override the function, set the OVERRIDE_BIN variable
to the path of the binary

"""


OVERRIDE_BIN = None


import os
import platform
import pathlib

arch = int(platform.architecture()[0][:-3])


def find_windows_binary():
    return pathlib.Path().absolute() / "binaries" / f"libchess_win{arch}.dll"

def find_linux_binary():
    return pathlib.Path(__file__).parent.absolute() / "binaries" / f"libchess_linux_x86_{arch}.so"
def find_binary():
    if OVERRIDE_BIN: return OVERRIDE_BIN
    OSNAME = platform.system()

    if OSNAME == "Windows": return find_windows_binary()
    elif OSNAME == "Linux": return find_linux_binary()
    else:
        raise Exception("The platform", OSNAME, "is not supported by default, please consult the readme.txt file for instructions")

