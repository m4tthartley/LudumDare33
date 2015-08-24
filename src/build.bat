@echo off
pushd ..\build
cl -Zi -Od -MT ../src/ld33.c opengl32.lib SDL2.lib -link -SUBSYSTEM:WINDOWS
