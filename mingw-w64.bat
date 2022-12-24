echo off
set GCCVER=8.1.0
set MINGWDIR=C:\mingw-w64\x86_64-%GCCVER%
set SDLDIR=D:\src\SDL
set PATH=%MINGWDIR%\mingw64\bin;%SDLDIR%\lib\x64;%PATH%
rem echo %PATH%
rem cd "C:\mingw-w64\x86_64-8.1.0\mingw64\bin"
D:
cd "D:\src\githubwrk\vm6502"
"C:\Windows\system32\cmd.exe"
