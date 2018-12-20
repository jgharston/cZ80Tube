@rem Make cZ80Tube for DOS with MinGW compiler
@rem Edit path to ming32-gcc to match your installation
@rem NOT WORKING

@cd %0\..\..\src
@set PATH=C:\Apps\Programming\TDM-GCC-32\bin;%PATH%
@mingw32-gcc z80tube.c -D__dos__ -DUSECONIO -w
@mkdir ..\binaries >NUL: 2>NUL:
@if exist z80tube.exe copy z80tube.exe ..\binaries\cZ80mingw.exe >NUL:
@if exist z80tube.exe del z80tube.exe
@if exist z80tube.obj del z80tube.obj
@cd ..\build
@pause

@rem To run with default files:
@rem cZ80mingw.exe -mos ..\files\MOS ..\files\BBCBasic
