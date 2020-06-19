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

![Screenshot](https://watte.net/jetson-nano-simple-status-1.0.png "Screenshot")

[Bigger Screenshot!](screenshot.png "Bigger Screenshot")
