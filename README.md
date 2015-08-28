The open-source, plain & simple no-fluff nFusion PVR recorder for Windows, MacOS, Linux and XBMC.

#Console based version (nfpvr)
The console based version runs on Windows, Linux and MacOS. You can either download a pre-compiled binary or compile it yourself.

#Using the pre-compiled binaries (only Windows for now)
This version if for the pro-users who want more control over the server. Download the latest nfpvr binary package (only Windows binaries are provided for now). Extract and run nfpvr.

nfpvr 0.0.5, run "nfpvr --help" for help
Bound to UDP port 50000, waiting for data
Your local IP is 192.168.0.10
That's it; set the right PVR IP in the nFusion's Internet settings and you're good to go.

#Compiling it yourself (Linux and MacOS)
To get it up and running, you need to download & extract the latest source code archive nfpvrsrc and type make where you extracted the archive.

If you're lucky, what should happen next is something like this:

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
You can now run nfpvr/nfpvr to start the server.

#XBMC based version (nfpvrxbox)

You can also use your awesome Xbox-based XBMC media center as a storage device for your nFusion box; your Xbox and nFusion are already connected to your home network, so why not have them talk to each other?

#Here's how it works:

download & install T3CH XBMC 2008-02-24 SVN rev11787 off T3CH's website
replace the default.xbe with my custom-built version in nfpvrxbox
Now as simply as before, set the Xbox's IP on your nFusion in the PVR setting and hit record; no need for leaving a PC turned on anymore.
