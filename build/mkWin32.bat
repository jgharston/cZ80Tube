@rem Make cZ80Tube for Windows with Visual Studio compiler
@rem Run from Visual Studio command prompt

@cd %0\..\..\src
@cl /w /EHsc z80tube.c user32.lib -D__win32__
@mkdir ..\binaries >NUL: 2>NUL:
@if exist z80tube.exe copy z80tube.exe ..\binaries\cZ80win32.exe >NUL:
@if exist z80tube.exe del z80tube.exe
@if exist z80tube.obj del z80tube.obj
@cd ..\build

@rem To run with default files:
@rem cZ80win32.exe -mos ..\files\MOS ..\files\BBCBasic
