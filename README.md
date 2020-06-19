# jetson-nano-simple-status

Show the status and IP address of a Jetson Nano

Licensed under the Creative Commons Zero un-license.

This little program shows the IP address of eth0 on your Jetson Nano. You can
make it start on bootup by installing the .desktop file in the
~/.config/autostart directory.

To build:

    make
    make install

It may ask for sudo password to install libfltk1.3-dev when you build, and to
copy the binary to /usr/local/bin when you install it.

You may wish to open up the .config/autostart/jetson-nano-simple-status.desktop
file and add a specific network interface to look for (default is eth0).
Options you can add include:

    -i --interface         (default "eth0")
                           which network interface to look for
    -x --xpos              (default 64)
                           horizontal coordinate to put the window
    -y --ypos              (default 32)
                           vertical coordinate to put the window


![Screenshot](https://watte.net/jetson-nano-simple-status-1.0.png "Screenshot")

[Bigger Screenshot!](screenshot.png "Bigger Screenshot")
