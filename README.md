## Description
This is a fork of Russ Cox's drawterm and plan9front's drawterm that listens for incoming rcpu requests, instead of making one itself. It has rc with enough builtins that run inside the kernel for a typical session

## Installation
To build on Unix, run CONF=unix make.

To build on Solaris using Sun cc, run CONF=sun make.

To build on Windows, you can't use Visual C. Use Mingw on cygwin.

To build on Mac OS X, run CONF=osx-cocoa make.

TODO: plist

## Status

This is about 90% of the way there. See [TODO](https://github.com/halfwit/drawcpu/blob/main/TODO) for more information.
 - mount is erroring when attempting to mount a client fd
 - proc doesn't exist on all systems, devproc would be useful in at least a limited scope inside the kernel

## Future

 - devdraw/devkbd/devmouse/devaudio integration for various toolkits, translating the other direction compared to drawterm
 - rework path, attempt to get a proper userland with frontbase or related that can be compiled externally

## Limitations, etc

 - we're abusing rc builtins to provide a bit of utility here for sessions
 - mount will not be available outside of the namespace, unless we do something like 9pfuse
