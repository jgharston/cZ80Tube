@ECHO OFF
CD %0\..\..\
SET Z80$BASIC=files\BBCBasic
binaries\cZ80win32 -debug 0 -mos files\MOS files\BBCBasic %1
