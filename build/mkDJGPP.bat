@rem Make cZ80Tube for DOS with DJGPP compiler
@rem Edit DJGPP and PATH paths to match your installation
@rem NOT WORKING - gcc gives SIGSECV: Stack Overflow

@cd %0\..\..\src
@set DJGPP=C:\Apps\Programming\djgpp\djgpp.env
@set PATH=C:\Apps\Programming\djgpp\bin;%PATH%
@gcc z80tube.c -o z80tube.exe -w -s -O -D__DOS__ -DUSECONIO
@mkdir ..\binaries >NUL: 2>NUL:
@if exist z80tube.exe copy z80tube.exe ..\binaries\cZ80djp.exe >NUL:
@if exist z80tube.exe del z80tube.exe
@if exist z80tube.obj del z80tube.obj
@cd ..\build
@pause

@rem To run with default files:
@rem cZ80djpp.exe -mos ..\files\MOS ..\files\BBCBasic
