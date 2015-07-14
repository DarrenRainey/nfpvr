# What is this? #
The open-source, plain & simple no-fluff **nFusion PVR recorder** for Windows, MacOS, Linux and XBMC.

Comments, questions, love letters? [email me](mailto:jonathan.fillion@gmail.com)!

---


# Windows version (nfpvrwin) #

This is the version most of you **Windows** users will probably be using. Simply download the latest **[nfpvrwin](http://nfpvr.googlecode.com/files/nfpvrwin-0.0.5.zip)** package. It's as simple as this:
![http://img228.imageshack.us/img228/4386/nfpromocg3.png](http://img228.imageshack.us/img228/4386/nfpromocg3.png)

---


# Console based version (nfpvr) #
The console based version runs on **Windows**, **Linux** and **MacOS**.
You can either download a pre-compiled binary or compile it yourself.

## Using the pre-compiled binaries (only Windows for now) ##
This version if for the pro-users who want more control over the server. Download the latest **[nfpvr](http://nfpvr.googlecode.com/files/nfpvr-0.0.5.zip)** binary package (only Windows binaries are provided for now). Extract and run `nfpvr`.
```
nfpvr 0.0.5, run "nfpvr --help" for help
Bound to UDP port 50000, waiting for data
Your local IP is 192.168.0.10
```

That's it; set the right PVR IP in the nFusion's Internet settings and you're good to go.

## Compiling it yourself (Linux and MacOS) ##
To get it up and running, you need to download & extract the latest source code archive **[nfpvrsrc](http://nfpvr.googlecode.com/files/nfpvrsrc-0.0.5.zip)** and type `make` where you extracted the archive.

If you're lucky, what should happen next is something like this:
```
make -C nfpvr
make[1]: Entering directory `/cygdrive/c/Code/nfpvr/nfpvr'
make -C ../nfpvrlib
make[2]: Entering directory `/cygdrive/c/Code/nfpvr/nfpvrlib'
make[2]: Nothing to be done for `all'.
make[2]: Leaving directory `/cygdrive/c/Code/nfpvr/nfpvrlib'
g++ -c ProgramOptions.cpp -Wall -I../nfpvrlib
g++ -c nfpvr.cpp -Wall -I../nfpvrlib
g++ -o nfpvr ProgramOptions.o nfpvr.o -L../nfpvrlib -lnfpvr
make[1]: Leaving directory `/cygdrive/c/Code/nfpvr/nfpvr'
```

You can now run `nfpvr/nfpvr` to start the server.

---


# XBMC based version (nfpvrxbox) #

You can also use your awesome **Xbox**-based [XBMC](http://en.wikipedia.org/wiki/Xbox_Media_Center) media center as a storage device for your nFusion box; your Xbox and nFusion are already connected to your home network, so why not have them talk to each other?

Here's how it works:
  1. download & install _T3CH XBMC 2008-02-24 SVN rev11787_ off [T3CH's website](http://t3ch.yi.se)
  1. replace the _default.xbe_ with my custom-built version in **[nfpvrxbox](http://nfpvr.googlecode.com/files/nfpvrxbox-0.0.5.zip)**

Now as simply as before, set the Xbox's IP on your nFusion in the PVR setting and hit record; no need for leaving a PC turned on anymore.