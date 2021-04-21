@echo off
del /S *.o
mingw32-make mode=release 2>&1 | unix2dos