@echo off
pushd ..\build
cl -O2 -MT -DBUILD_RELEASE=1 ../src/ld33.c opengl32.lib SDL2.lib -link -SUBSYSTEM:WINDOWS
