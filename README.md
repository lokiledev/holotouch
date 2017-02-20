Holotouch
=========

A 3D file explorer based on cheap computer peripherals (a usb webcam and a leapmotion) that simulates a 3D world where you can drag and drop files with natural hand gestures.

Here you can see it in action: https://www.youtube.com/watch?v=8b99VDvN64M

How to install dev environment for Linux (Ubuntu)

Install opencv (V2.4.8):
http://miloq.blogspot.fr/2012/12/install-opencv-ubuntu-linux.html

to uninstall opencv go to the build dir and type "sudo make uninstall"
(sudo needed to remove from /usr/bin etc.)

Install Qt 5.2:
http://qt-project.org/downloads

Open Qt project:
open holotouch.pro with qtcreator. It will create holotouch.pro.user,
don't add it on git, it's generated by Qt

Leapmotion software for linux: 
go to https://developer.leapmotion.com/downloads and download the latest
sdk for linux.
You also need to install leapmotion software (drivers) on the same page.
Install Leap-1.0.9+8411-x64.deb (or x86 depending on architecture).
Then when your computer starts it will automatically launch leapd service.
To verify it works just launch Visualizer in a terminal

The sdk is for developping applications, useful files are already included in this depot.
