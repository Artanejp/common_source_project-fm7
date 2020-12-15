<H2>** HowTo build/install Qt porting for Common Source Code Project **</H2>
<div align="right">
<H3>Oct 08, 2020<BR>
K.Ohta <whatisthis.sowhat _at_ gmail.com></H3>
</div>

At first
========
This article is for build HOWTO of CSP/Qt.

You need (at standard).
========

- Compiler Toolcain, i.e) G++ or CLANG++.MSVC or another toolchain still not be checked.

- CMake (Recommends later 3.9).

- Below **development** libraries.If you using any GNU/Linux or \*BSD OS,you can install via package manager (apt, dfn, etc). **Note: NOW ,OpenGL (2.1 or above) or OpenGL ES(2 or abobe) needs to display.**
  - QT5 (QtCore, QtOpenGL , QtNetwork etc). <https://www.qt.io/>
  - SDL2 <https://libsdl.org/>
  - libAV <https://ffmpeg.org/>
  - Zlib <https://zlib.net/>
    And, some others.
- For Windows build, you may need OpenGL ES implementation, [Angle Project](<https://github.com/Microsoft/angle>) to execute.Binaries of this are included at [Chromium Project](<http://www.chromium.org/>) Windows (x86 32bit) build, open source variant of Google Chrome.You need to extract libEGL.dll and libGLESv2.dll .

## In addition, for Windows build, useful inside of Docker Container.See,
## Docker repository: <https://hub.docker.com/r/artanejp/mingw-w64-llvm10-ubuntu20.04>
## Dockerfiles are: <https://github.com/Artanejp/llvm-mingw>

How to Build
============

If you build from already cloneed from GIT:

$ `cd ${SRCROOT}/source`
$ `mkdir build`
$ `cd build/`
$ `cmake ..`
$ `make`

but, **I offer shell scripts with standard setting parameters under `${SRCROOT}/source/sample-scripts/`**

With this sample:

$ `cd ${SRCROOT}/source`
$ `mkdir build`
$ `cd build/`
$ `cp ../sample-scripts/${SCRIPT_NAME}.sh .`

then, bootstrap setting with:
$ `sh ./${SCRIPT_NAME}.sh`

Finally,
$ `make {Some Options}`

How to Install
==============

Normally, `make install`.
For Windows build, this still ToDo.

                                      Last Update: Dec 16, 2020 00:07:53


