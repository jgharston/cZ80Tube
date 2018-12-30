/* host.h - Investigate and define target environment
   Copyright (C) 1987-2003 J.G.Harston

  See the accompanying file LICENSE for terms of use.
 */


/* Host (osbyte 0,<>0) and OS (inkey-256) values */
#define HOST_UNDEFINED		-1
#define HOST_BBC_100		0
#define HOST_BBC		1
#define HOST_BBC_ACW		2
#define HOST_MASTER		3
#define HOST_MASTER_ET		4
#define HOST_MASTER_COMPACT	5
#define HOST_RISCOS		6
#define HOST_SPRINGBOARD	7
#define HOST_UNIX		8
#define HOST_CPC		30
#define HOST_SPECTRUM		31
#define HOST_DOS		32
#define HOST_WINDOWS		32

#define OS_ELECTRON		0x01
#define OS_BBC			0xFF
#define OS_REUTERSUK		0xFF
#define OS_BBCUS		0xFE
#define OS_REUTERSUS		0xFE
#define OS_NETBSD		0xFE
#define OS_MASTER320		0xFD
#define OS_BBCGERMANY		0xFC
#define OS_WINDOWS		0xFC
#define OS_BBCPLUS		0xFB
#define OS_BEOS			0xFB
#define OS_ABC			0xFA
#define OS_DOS			0xFA
#define OS_LINUX		0xF9
#define OS_UNIX			0xF9
#define OS_F8			0xF8
#define OS_MASTERET		0xF7
#define OS_F6			0xF6
#define OS_COMPACT		0xF5
#define OS_SINCLAIR		0xE0
#define OS_AMSTRAD		0xD0
#define OS_PDP11UNIX7		0xB7
#define OS_PDP11UNIX6		0xB6
#define OS_RISCOS1		0xA0
#define OS_RISCOS2		0xA1
#define OS_RISCOS201		0xA2
#define OS_RISCOS3		0xA3
#define OS_RISCOS31		0xA4
#define OS_RISCOS35		0xA5
#define OS_RISCOS36		0xA6
#define OS_RISCOS37		0xA7
#define OS_RISCOS4		0xA8
#define OS_RISCOS41		0xA9
#define OS_RISCOS5		0xAA
#define OS_BRAZIL		0xAF


/* Determine the target environment */
#ifdef __riscos__
#define Z80IO_RO
#define Z80FILE_RO
#define Z80HOST	  HOST_RISCOS
#ifdef __CC_NORCROFT
#define Z80TARGET "RISC OS/NCC"
#else
#define Z80TARGET "RISC OS"
#endif
#endif


#if defined(__unix__) || defined(__linux__)
#ifndef __DOS__
#define Z80IO_UNIX
#define Z80FILE_UNIX
#define Z80HOST	  HOST_UNIX
#define Z80OS	  OS_UNIX
#ifdef __linux__
#define Z80UNIX "LINUX"
#else
#define Z80UNIX "UNIX"
#endif
#ifdef __GNUC__
#define Z80TARGET Z80UNIX "/GCC"
#else
#define Z80UNIX Z80UNIX
#endif
#endif
#endif


#ifdef __WIN32__
#define Z80IO_WIN
#define Z80FILE_WIN
#define Z80HOST	  HOST_WINDOWS
#define Z80OS	  OS_WINDOWS
#ifdef __MINGW32__
#define Z80TARGET "Win32/MinGW"
#else
#ifdef _MSC_VER
#define Z80TARGET "Win32/MSC"
#else
#define Z80TARGET "Win32"
#endif
#endif
#endif


#ifdef __DOS__
#define Z80IO_DOS
#define Z80FILE_DOS
#define Z80HOST	  HOST_DOS
#define Z80OS	  OS_DOS
#ifdef __DJGPP__
#define Z80TARGET "DOS/DJGPP"
#else
#define Z80TARGET "DOS"
#endif
#endif

