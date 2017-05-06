BeautifulDiscord
================

Simple program that adds CSS hot-reload to Discord.

![demo gif](http://i.imgur.com/xq4HS5f.gif)

## Motivation

I wanted custom CSS injection for Discord, with no JavaScript add-ons or anything.
That's BeautifulDiscord.

If you want JS, you can either:
- Use [BetterDiscord](https://github.com/Jiiks/BetterDiscordApp)
- Make your own thing!

You could also fork this repo and add it, it's not that big of a stretch.
I just didn't add it because it's not what I want to do here.

## Installing

If you're on OS X or Windows the binaries are already provided for you in the [releases][ghrel] page. Just download the archive and extract it to get the program.

To compile it yourself, you can do the following:

```
$ python compile.py
```

This will create a `beautiful_discord` executable for you to use.

[ghrel]: https://github.com/leovoel/BeautifulDiscord/releases

## Requirements

- A C++14 capable compiler (GCC 6.1 or higher was tested).
- Boost.Filesystem (which depends on Boost.System)

## Usage

Just invoke the program when installed. If you don't pass the `--css` flag, the stylesheet
will be placed wherever the program is located.

**NOTE:** Discord has to be running for this to work in first place.
The program works by scanning the active processes and looking for the Discord ones.

(yes, this also means you can fool the program into trying to apply this to some random program named Discord)

```
$ beautiful_discord --css C:\mystuff\myown.css
0: Found DiscordPTB.exe
1: Found DiscordCanary.exe
Discord executable to use (number): 1

Done!

You may now edit your C:\mystuff\myown.css file,
which will be reloaded whenever it's saved.

Relaunching Discord now...
$
```

Pass the `--revert` flag to remove the extracted `app.asar` (it's the `resources/app` folder)
and rename `original_app.asar` to `app.asar`. You can also do this manually if your Discord
install gets screwed up.

```
$ beautiful_discord --revert
0: Found DiscordPTB.exe
1: Found DiscordCanary.exe
Discord executable to use (number): 1
Reverted changes, no more CSS hot-reload :(
$
```

## Themes

Some people have started a theming community over here:
https://github.com/beautiful-discord-community/resources/

They have a Discord server as well:
https://discord.gg/EDwd5wr

## More GIFs

![demo gif](http://i.imgur.com/w0bQOJ6.gif)
