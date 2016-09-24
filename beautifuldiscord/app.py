#!/usr/bin/env python

import os
import shutil
import argparse
import textwrap
import subprocess
import psutil
import sys
from collections import namedtuple
from beautifuldiscord.asar import Asar


DiscordProcess = namedtuple('DiscordProcess', 'path exe processes')

def discord_process_terminate(self):
    for process in self.processes:
        process.terminate()

def discord_process_launch(self):
    subprocess.Popen([os.path.join(self.path, self.exe)])

def discord_process_resources_path(self):
    if sys.platform == 'darwin':
        # OS X has a different resources path
        # Application directory is under <[EXE].app/Contents/MacOS/[EXE]>
        # where [EXE] is Discord Canary, Discord PTB, etc
        # Resources directory is under </Applications/[EXE].app/Contents/Resources/app.asar>
        # So we need to fetch the folder based on the executable path.
        return os.path.join('/Applications/', '%s.app' % self.exe, 'Contents/Resources')
    return os.path.join(self.path, 'resources')

DiscordProcess.terminate = discord_process_terminate
DiscordProcess.launch = discord_process_launch
DiscordProcess.resources_path = property(discord_process_resources_path)

def parse_args():
    description = """\
Unpacks Discord and adds CSS hot-reloading.

Discord has to be open for this to work. When this tool is ran,
Discord will close and then be relaunched when the tool completes.
"""
    parser = argparse.ArgumentParser(description=description.strip())
    parser.add_argument('--css', metavar='file', help='Location of the CSS file to watch')
    parser.add_argument('--revert', action='store_true', help='Reverts any changes made to Discord (does not delete CSS)')
    args = parser.parse_args()

    if args.css:
        args.css = os.path.abspath(args.css)
    else:
        args.css = './discord-custom.css'

    return args

def discord_process():
    executables = {}
    for proc in psutil.process_iter():
        try:
            (path, exe) = os.path.split(proc.exe())
        except psutil.AccessDenied:
            pass
        else:
            if exe.startswith('Discord') and not exe.endswith('Helper'):
                entry = executables.get(exe)

                if entry is None:
                    entry = executables[exe] = DiscordProcess(path=path, exe=exe, processes=[])

                entry.processes.append(proc)

    if len(executables) == 0:
        raise RuntimeError('Could not find Discord executable.')

    if len(executables) == 1:
        r = executables.popitem()
        print('Found {0.exe} under {0.path}'.format(r[1]))
        return r[1]

    lookup = list(executables)
    for index, exe in enumerate(lookup):
        print('%s: Found %s' % (index, exe))

    while True:
        index = input("Discord executable to use (number): ")
        try:
            index = int(index)
        except ValueError as e:
            print('Invalid index passed')
        else:
            if index >= len(lookup) or index < 0:
                print('Index too big (or small)')
            else:
                key = lookup[index]
                return executables[key]

def extract_asar():
    try:
        with Asar.open('./app.asar') as a:
            try:
                a.extract('./app')
            except FileExistsError:
                answer = input('asar already extracted, overwrite? (Y/n): ')

                if answer.lower().startswith('n'):
                    print('Exiting.')
                    return False

                shutil.rmtree('./app')
                a.extract('./app')

        shutil.move('./app.asar', './original_app.asar')
    except FileNotFoundError as e:
        print('WARNING: app.asar not found')
    return True

def main():
    args = parse_args()
    try:
        discord = discord_process()
    except Exception as e:
        print(str(e))
        return

    os.chdir(discord.resources_path)
    discord.terminate()

    if args.revert:
        try:
            shutil.rmtree('./app')
            shutil.move('./original_app.asar', './app.asar')
        except FileNotFoundError as e:
            # assume things are fine for now i guess
            print('No changes to revert.')
        else:
            print('Reverted changes, no more CSS hot-reload :(')
    else:
        if extract_asar():
            if not os.path.exists(args.css):
                with open(args.css, 'w') as f:
                    f.write('/* put your custom css here. */\n')

            css_injection_script = textwrap.dedent("""\
                window._fs = require("fs");
                window._fileWatcher = null;
                window._styleTag = null;

                window.setupCSS = function(path) {
                  var customCSS = window._fs.readFileSync(path, "utf-8");
                  if(window._styleTag === null) {
                    window._styleTag = document.createElement("style");
                    document.head.appendChild(window._styleTag);
                  }
                  window._styleTag.innerHTML = customCSS;
                  if(window._fileWatcher === null) {
                    window._fileWatcher = window._fs.watch(path, { encoding: "utf-8" },
                      function(eventType, filename) {
                        if(eventType === "change") {
                          var changed = window._fs.readFileSync(path, "utf-8");
                          window._styleTag.innerHTML = changed;
                        }
                      }
                    );
                  }
                };

                window.tearDownCSS = function() {
                  if(window._styleTag !== null) { window._styleTag.innerHTML = ""; }
                  if(window._fileWatcher !== null) { window._fileWatcher.close(); window._fileWatcher = null; }
                };

                window.applyAndWatchCSS = function(path) {
                  window.tearDownCSS();
                  window.setupCSS(path);
                };

                window.applyAndWatchCSS('%s');
            """ % args.css.replace('\\', '\\\\'))

            with open('./app/cssInjection.js', 'w') as f:
                f.write(css_injection_script)

            css_injection_script_path = os.path.abspath('./app/cssInjection.js').replace('\\', '\\\\')

            css_reload_script = textwrap.dedent("""\
                mainWindow.webContents.on('dom-ready', function () {
                  mainWindow.webContents.executeJavaScript(
                    _fs2.default.readFileSync('%s', 'utf-8')
                  );
                });
            """ % css_injection_script_path)

            with open('./app/index.js', 'r') as f:
                entire_thing = f.read()

            entire_thing = entire_thing.replace("mainWindow.webContents.on('dom-ready', function () {});", css_reload_script)

            with open('./app/index.js', 'w') as f:
                f.write(entire_thing)

            print(
                '\nDone!\n' +
                '\nYou may now edit your %s file,\n' % os.path.abspath(args.css) +
                "which will be reloaded whenever it's saved.\n" +
                '\nRelaunching Discord now...'
            )

    discord.launch()


if __name__ == '__main__':
    main()
