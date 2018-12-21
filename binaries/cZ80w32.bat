@ECHO OFF
CD %0\..\..\
SET Z80TUBE$BASIC=%0\..\files\BBCBasic
start binaries\cZ80w32 -debug 0 -mos files\MOS files\BBCBasic %1
