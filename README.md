# Visual Voicemail Player (VVM Player)

Copyright (C) 2021-2022, Chris Talbot

## About
VVM Player is a front end GUI to display, play, and delete visual voicemails (VVMs).

VVM Player depends on vvmd for VVM functionality
( https://gitlab.com/kop316/vvmd )

<div align="center">
  <img src="https://gitlab.com/kop316/vvmplayer/-/raw/main/data/metainfo/screenshot.png?inline=false" width="300" >
  <img src="https://gitlab.com/kop316/vvmplayer/-/raw/main/data/metainfo/screenshot2.png?inline=false" width="300" >
</div>

## License
Unless otherwise specified, all code is licensed under the GPL Version 3 or later
 https://www.gnu.org/licenses/gpl-3.0.en.html

All other images/data is licensed under  Creative Commons
Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)
https://creativecommons.org/licenses/by-sa/4.0/

## Notes
VVM Player is configured in the "Settings" page. VVM Player has the same
limitations as vvmd (e.g. carrier limitations, backend limitations)

## Settings Notes
VVM player will give you feedback on whether vvmd is functional. Both
"VVMD Running" and "Mailbox Active" must be checked for VVM to work!

Please look here for a databse of known good settings:
https://gitlab.com/kop316/vvmd/-/wikis/Carrier-Database

Note: Default Modem Number can be left blank (whereas in vvmd, it must be
      set to NULL) if a default modem number is not needed

## Compiling VVM Player
In order to compile VVM Player you need following software packages:

- GCC compiler
- GTK4 library
- libadwaita
- GStreamer
- GStreamer-pbutils

## Running VVM Player
VVM sound files are the generally of the Adaptive Multi-Rate (amr) audio codec.
gstreamer supports this in the "GStreamer Ugly Plug-ins":
https://gstreamer.freedesktop.org/modules/gst-plugins-ugly.html

This plugin is not needed to build VVM player, but if the Ugly plugins are not
installed, you will get a runtime error in attempting to playback your voicemail.

Depending on your distribution, you may have to additionally manually check that
support for amr is built into your distributions Ugly Plug-ins.


## Installing VVM Player
Build tools requirements

When building and testing directly from the repository it is important to
have at least meson version 0.56 or later installed. In addition, some
supported Meson backend is required (e.g. ninja).

To prepare/configure the project for compilation:

    # meson _build

However, if you want to install it, run

    # meson _build -Dprefix=/usr --sysconfdir=/etc

`--sysconfdir=/etc` will install the autostart file in the correct place, and
`-Dprefix=/usr` installs `vvmplayer` in `/usr/bin/` rather than `/usr/local/bin`

To build the project:

    # meson compile -C _build

Install:

    # meson install -C _build

    (Install may require sudo or root privileges).

Uninstall:

    # ninja uninstall -C _build/

    (Uninstall may require sudo or root privileges).

## Running from within the source code repository
Meson will build the binary in builddir (_build, using the examples above), and
can be invoked directly.

  Run with debugging
    # G_MESSAGES_DEBUG=all ./_build/src/vvmplayer
