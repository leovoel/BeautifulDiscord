BeautifulDiscord
================

Simple Python script that adds CSS hot-reload to discord.

## Usage

```
$ beautifuldiscord
0: Found DiscordPTB.exe
1: Found DiscordCanary.exe
Discord executable to use (number): 1

Done!

You may now edit your C:\Users\leovoel\AppData\Local\DiscordCanary\app-0.0.146\discord-custom.css file,
which will be reloaded whenever it's saved.

Relaunching Discord now...
$
```

## Installing

```
python3 -m pip install -U https://github.com/leovoel/BeautifulDiscord/archive/master.zip
```

Usage of a virtual environment is recommended, to not pollute your global package space.

## Requirements

- Python 3.x (no interest in compatibility with 2.x, untested on Python 3.x versions below 3.4)
- `psutil` library: https://github.com/giampaolo/psutil

Normally, `pip` should install any required dependencies.
