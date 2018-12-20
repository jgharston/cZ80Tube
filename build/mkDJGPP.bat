@rem Make cZ80Tube for DOS with DJGPP compiler
@rem Edit DJGPP and PATH paths to match your installation
@rem NOT WORKING

@set DJGPP=C:\Apps\Programming\djgpp\djgpp.env
@set PATH=C:\Apps\Programming\djgpp\bin;%PATH%
@cd %0\..\..\src
@gcc z80tube.c -w -D__dos__ -DUSECONIO
@mkdir ..\binaries >NUL: 2>NUL:
@if exist z80tube.exe copy z80tube.exe ..\binaries\cZ80djpp.exe >NUL:
@if exist z80tube.exe del z80tube.exe
@if exist z80tube.obj del z80tube.obj
@cd ..\build
@pause

@rem To run with default files:
@rem cZ80djpp.exe -mos ..\files\MOS ..\files\BBCBasic
