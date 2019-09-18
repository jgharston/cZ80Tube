@rem Make cZ80Tube for Windows with MinGW compiler
@rem Edit path to ming32-gcc to match your installation

@echo Make cZ80Tube for Windows with MinGW compiler
@set PATH=C:\Apps\Programming\TDM-GCC-32\bin;%PATH%
@
@cd %0\..\..\src
@gcc z80tube.c -o z80tube.exe -s -O -w -D__WIN32__ -DCONVDU_ANSI
@mkdir ..\binaries >NUL: 2>NUL:
@if exist z80tube.exe copy z80tube.exe ..\binaries\cZ80mgw.exe >NUL:
@if exist z80tube.exe del z80tube.exe
@if exist z80tube.obj del z80tube.obj
@cd ..\build
@pause

@rem To run with default files:
@rem cZ80mgw.exe -mos ..\files\MOS ..\files\BBCBasic
