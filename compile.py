#!/usr/bin/env python

# Simple script to compile BD, which is a single file.
# Hence why this script is made.

# This is not a good build system.

import os
import re
import sys
import posixpath
import subprocess
import argparse

# gcc doesn't like \ paths
script_path = posixpath.normpath(posixpath.dirname(posixpath.realpath(__file__))).replace('\\', '/')

# backport of shlex.quote for <3.3
_find_unsafe = re.compile(r'[^\w@%+=:,./-]').search

def quote(s):
    """Return a shell-escaped version of the string *s*."""
    if not s:
        return "''"

    if _find_unsafe(s) is None:
        return s

    # use single quotes, and put single quotes into double quotes
    # the string $'b is then quoted as '$'"'"'b'
    return "'" + s.replace("'", "'\"'\"'") + "'"

def get_cli_args(is_debug):
    # looking at this function will show you just how bad of a build system this is
    # but this exists because Make is *really* bad at detecting OSes.
    # and I didn't want to enforce ninja for people while python is pretty much
    # installed by default,
    # now that you have a justification back to the code
    args = ['-std=c++14', '-Wall', '-pthread', '-Wextra', '-pedantic', '-I.', '-Iinclude']

    if is_debug:
        args.append('-DDEBUG')
        args.append('-g')
    else:
        args.append('-DNDEBUG')
        args.append('-O3')

    args.append(quote(posixpath.join(script_path, 'main.cpp')))
    args.append('-o')
    args.append(quote(posixpath.join(script_path, 'beautiful_discord')))
    if sys.platform == 'win32':
        args.append('-lpsapi')

    args.append('-lboost_system')
    args.append('-lboost_filesystem')
    return args

def main():
    parser = argparse.ArgumentParser()
    default_compiler = 'g++' if sys.platform != 'darwin' else 'clang++'
    parser.add_argument('--cxx', metavar='compiler', action='store', help='compiler to use', default=default_compiler)
    parser.add_argument('--debug', action='store_true', help='produce debug binary')
    args = parser.parse_args()

    cli = [args.cxx] + get_cli_args(args.debug)
    print('Running ' + ' '.join(cli))
    subprocess.check_call(cli)

if __name__ == '__main__':
    main()
