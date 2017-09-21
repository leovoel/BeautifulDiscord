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
        # terrible
        process.kill()

def discord_process_launch(self):
    with open(os.devnull, 'w') as f:
        subprocess.Popen([os.path.join(self.path, self.exe)], stdout=f, stderr=subprocess.STDOUT)

def discord_process_resources_path(self):
    if sys.platform == 'darwin':
        # OS X has a different resources path
        # Application directory is under <[EXE].app/Contents/MacOS/[EXE]>
        # where [EXE] is Discord Canary, Discord PTB, etc
        # Resources directory is under </Applications/[EXE].app/Contents/Resources/app.asar>
        # So we need to fetch the folder based on the executable path.
        # Go two directories up and then go to Resources directory.
        return os.path.abspath(os.path.join(self.path, '..', 'Resources'))
    return os.path.join(self.path, 'resources')

DiscordProcess.terminate = discord_process_terminate
DiscordProcess.launch = discord_process_launch
DiscordProcess.resources_path = property(discord_process_resources_path)

def parse_args():
    description = """\
Unpacks Discord and adds CSS hot-reloading.

Discord has to be open for this to work. When this tool is ran,
Discord will close and then be relaunched when the tool completes.
CSS files must have the ".css" extension.
"""
    parser = argparse.ArgumentParser(description=description.strip())
    parser.add_argument('--css', metavar='file_or_dir', help='Location of the file or directory to watch')
    parser.add_argument('--revert', action='store_true', help='Reverts any changes made to Discord (does not delete CSS)')
    args = parser.parse_args()
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

    if args.css:
        args.css = os.path.abspath(args.css)
    else:
        args.css = os.path.join(discord.resources_path, 'discord-custom.css')

    os.chdir(discord.resources_path)

    args.css = os.path.abspath(args.css)

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
                with open(args.css, 'w', encoding='utf-8') as f:
                    f.write('/* put your custom css here. */\n')

            css_injection_script = textwrap.dedent("""\
                window._fs = require("fs");
                window._path = require("path");
                window._fileWatcher = null;
                window._styleTag = {};

                window.applyCSS = function(path, name) {
                  var customCSS = window._fs.readFileSync(path, "utf-8");
                  if (!window._styleTag.hasOwnProperty(name)) {
                    window._styleTag[name] = document.createElement("style");
                    document.head.appendChild(window._styleTag[name]);
                  }
                  window._styleTag[name].innerHTML = customCSS;
                }

                window.clearCSS = function(name) {
                  if (window._styleTag.hasOwnProperty(name)) {
                    window._styleTag[name].innerHTML = "";
                    window._styleTag[name].parentElement.removeChild(window._styleTag[name]);
                    delete window._styleTag[name];
                  }
                }

                window.watchCSS = function(path) {
                  if (window._fs.lstatSync(path).isDirectory()) {
                    files = window._fs.readdirSync(path);
                    dirname = path;
                  } else {
                    files = [window._path.basename(path)];
                    dirname = window._path.dirname(path);
                  }

                  for (var i = 0; i < files.length; i++) {
                    var file = files[i];
                    if (file.endsWith(".css")) {
                      window.applyCSS(window._path.join(dirname, file), file)
                    }
                  }

                  if(window._fileWatcher === null) {
                    window._fileWatcher = window._fs.watch(path, { encoding: "utf-8" },
                      function(eventType, filename) {
                        if (!filename.endsWith(".css")) return;
                        path = window._path.join(dirname, filename);
                        if (eventType === "rename" && !window._fs.existsSync(path)) {
                          window.clearCSS(filename);
                        } else {
                          window.applyCSS(window._path.join(dirname, filename), filename);
                        }
                      }
                    );
                  }
                };

                window.tearDownCSS = function() {
                  for (var key in window._styleTag) {
                    if (window._styleTag.hasOwnProperty(key)) {
                      window.clearCSS(key)
                    }
                  }
                  if(window._fileWatcher !== null) { window._fileWatcher.close(); window._fileWatcher = null; }
                };

                window.applyAndWatchCSS = function(path) {
                  window.tearDownCSS();
                  window.watchCSS(path);
                };

                window.applyAndWatchCSS('%s');
            """ % args.css.replace('\\', '\\\\'))

            with open('./app/cssInjection.js', 'w', encoding='utf-8') as f:
                f.write(css_injection_script)

            css_injection_script_path = os.path.abspath('./app/cssInjection.js').replace('\\', '\\\\')

            css_reload_script = textwrap.dedent("""\
                mainWindow.webContents.on('dom-ready', function () {
                  mainWindow.webContents.executeJavaScript(
                    _fs2.default.readFileSync('%s', 'utf-8')
                  );
                });
            """ % css_injection_script_path)

            with open('./app/index.js', 'r', encoding='utf-8') as f:
                entire_thing = f.read()

            entire_thing = entire_thing.replace("mainWindow.webContents.on('dom-ready', function () {});", css_reload_script)

            with open('./app/index.js', 'w', encoding='utf-8') as f:
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
