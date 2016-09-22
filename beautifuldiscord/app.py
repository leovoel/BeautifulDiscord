#!/usr/bin/env python

import os
import shutil
import argparse
import subprocess
import psutil
from collections import namedtuple
from beautifuldiscord.asar import Asar


DiscordProcess = namedtuple('DiscordProcess', 'path exe processes')

def discord_process_terminate(self):
    for process in self.processes:
        process.terminate()

def discord_process_launch(self):
    subprocess.Popen([os.path.join(self.path, self.exe)])

DiscordProcess.terminate = discord_process_terminate
DiscordProcess.launch = discord_process_launch

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
            if exe.startswith('Discord'):
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
        with Asar.open('./resources/app.asar') as a:
            try:
                a.extract('./resources/app')
            except FileExistsError:
                answer = input('asar already extracted, overwrite? (Y/n): ')

                if answer.lower().startswith('n'):
                    print('Exiting.')
                    return

                shutil.rmtree('./resources/app')
                a.extract('./resources/app')

        shutil.move('./resources/app.asar', './resources/original_app.asar')
    except FileNotFoundError as e:
        print('WARNING: app.asar not found')

def main():
    args = parse_args()
    try:
        discord = discord_process()
    except Exception as e:
        print(str(e))
        return

    os.chdir(discord.path)
    discord.terminate()

    if args.revert:
        try:
            shutil.rmtree('./resources/app')
            shutil.move('./resources/original_app.asar', './resources/app.asar')
        except FileNotFoundError as e:
            # assume things are fine for now i guess
            print('No changes to revert.')
        else:
            print('Reverted changes, no more CSS hot-reload :(')
    else:
        extract_asar()

        if not os.path.exists(args.css):
            with open(args.css, 'w') as f:
                f.write('/* put your custom css here. */')

        with open('./resources/app/index.js', 'r') as f:
            entire_thing = f.read()

        css_reload_script = """var customCSSPath = '%s';
        var customCSS = _fs2.default.readFileSync(customCSSPath, 'utf-8');
        mainWindow.webContents.on('dom-ready', function () {
          mainWindow.webContents.executeJavaScript(
            'window.myStyles = document.createElement("style");' +
            'window.myStyles.innerHTML = `' + customCSS + '`;' +
            'document.head.appendChild(window.myStyles);'
          );

          _fs2.default.watch(customCSSPath, { encoding: 'utf-8' }, function(eventType, filename) {
            if(eventType === 'change') {
              var changed = _fs2.default.readFileSync(customCSSPath, 'utf-8');
              mainWindow.webContents.executeJavaScript(
                "window.myStyles.innerHTML = `" + changed + "`;"
              );
            }
          });
        });""" % args.css

        entire_thing = entire_thing.replace("mainWindow.webContents.on('dom-ready', function () {});", css_reload_script)

        with open('./resources/app/index.js', 'w') as f:
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
