/* hostio.c -
 * Copyright (C) 1987-2003,2018 J.G.Harston, 2004 David Daniels

 See the accompanying file LICENSE for authors and terms of use.

   28-Jan-1993 JGH: This is the re-written interface to UNIX
               'cos I went and copied the Arc version on top of it!
   14-Apr-1993 JGH: Started converting for Mark2 C Version
   20-May-1993 JGH: Started ANSI-ising it
   21-May-1993 JGH: removed look_esc(), add addr= to OSWORD,
               tidied up MOS_FF0D (SWI)
   28-May-1993 JGH: Still problems getting ioctl() to work.  Had
               to add system("stty...") commands to startup() and finish()
   28-May-1993 JGH: Added OSFILE/PUT/GET for bbc
   31-May-1993 JGH: MOS_QUIT moved to z80.c, PC made a pointer
               OSGBPB for __bbc written, but where is mem[HLreg+0] returned?
   26-Jun-1993 JGH: *Basic loads Z80Tube$Basic; *com checks Z80Tube$Flag
   12-Jun-1995 JGH: Note: could do following
               on startup, read ioctl, do "stty -cooked", read ioctl
               then use ioctl to change, and on shutdown, set back
   30-Aug-2003 JGH: Now called hostio.c
   23-Nov-2018 JGH: Added OSBYTE &86, cleaned up OSBYTE &81
   17-Dec-2018 JGH: Added escanable, some work on signals
   19-Dec-2018 JGH: tty settings working with new Linux calls
*/

/* ==================================================================== */
/* RISC OS kernel interface						*/
/* ==================================================================== */
#ifdef Z80IO_RO
#include <kernel.h>
#include <swis.h>
_kernel_oserror oserr;			/* Returned error information	*/
_kernel_oserror *oserr2;		/* Error from OSWORD 0		*/
_kernel_osfile_block ctrl;		/* OSFILE control block		*/
_kernel_osgbpb_block gbpb;		/* OSGBPB control block		*/
_kernel_swi_regs regs;			/* Used by OSWORD 0		*/
#endif

/* ==================================================================== */
/* UNIX kernel interface						*/
/* ==================================================================== */
#ifdef Z80IO_UNIX
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#ifdef TCSAFLUSH
  struct termios termhost;
  struct termios termz80;
#else
  #ifdef TIOCGETC
    struct tchars charshost;
    struct tchars charsz80;
    struct sgttyb termhost;
    struct sgttyb termz80;
  #endif
#endif
#include <time.h>
#include "console.c"
#endif

/* ==================================================================== */
/* Windows kernel interface						*/
/* ==================================================================== */
#ifdef Z80IO_WIN
#include <windows.h>
#include <time.h>
/* #include <winbase.h> */
#include "console.c"
#endif

/* ==================================================================== */
/* DOS kernel interface							*/
/* ==================================================================== */
#ifdef Z80IO_DOS
#include <time.h>
#include "console.c"
#endif


/* ==================================================================== */
/* Escape and Break handling						*/
/* ==================================================================== */
#include <signal.h>
int8 escenable=0;
int8 escchar=27;

void keyint_pressed(int sig)		/* Keypress interupts		*/
{
switch(sig) {
  case SIGINT:				/* Escape or Ctrl-C pressed	*/
#ifdef SIGBREAK
  case SIGBREAK:			/* Ctrl-Break pressed		*/
#endif
    mem[0xFF80]=128;			/* Set Z80's escape flag	*/
    signal(sig, keyint_pressed);	/* Re-attach signal		*/
  }
}

void keyint_claim(void)			/* Claim keypress interupts	*/
{
signal(SIGINT, keyint_pressed);		/* Escape or Ctrl-C pressed	*/
#ifdef SIGBREAK
signal(SIGBREAK, keyint_pressed);	/* Ctrl-Pause/Break pressed	*/
#endif
}

void keyint_release(void)		/* Release keypress interupts	*/
{
signal(SIGINT, SIG_DFL);
#ifdef SIGBREAK
signal(SIGBREAK, SIG_DFL);
#endif
}

void esc_on(void)			/* Enable SIGINT for Escape	*/
{
#ifdef Z80IO_UNIX
  #ifdef TCSAFLUSH
termz80.c_cc[VINTR]=escchar;		/* Interupt is enabled		*/
tcsetattr(STDIN_FILENO, TCSAFLUSH, &termz80);
  #endif
#endif
}

void esc_off(void)			/* Disable SIGINT for Escape	*/
{
#ifdef Z80IO_UNIX
  #ifdef TCSAFLUSH
termz80.c_cc[VINTR]=0;			/* Interupt is disabled		*/
tcsetattr(STDIN_FILENO, TCSAFLUSH, &termz80);
  #endif
#endif
}

#ifndef Z80IO_RO
int esc_rdch(int tmp)
{
if (tmp) esc_off();			/* Allow con_rdch() to read Esc	*/
tmp=con_rdch();
esc_on();				/* Restore Esc as bgnd interupt	*/
if (escenable == 0) {			/* If Escapes enabled		*/
  if (escchar == tmp) mem[0xFF80]=128;	/*  and EscChar, set ESCFLG	*/
  }
return tmp;
}
#endif


/* ==================================================================== */
/* Text input/output handling						*/
/* ==================================================================== */

void tty_init(void)
{
#ifdef Z80IO_WIN
  SetConsoleCtrlHandler(NULL, TRUE);	/* Ctrl-C is a normal character	*/
#endif

#ifdef Z80IO_UNIX
  #ifdef TCSAFLUSH
  /* Linux-style tty settings */
  tcgetattr(STDIN_FILENO, &termhost);	/* Get host's tty settings	*/
  #else
    #ifdef TIOCGETP
    /* Old-style unix ioctl() tty settings */
    ioctl(stdout, TIOCGETP, &termhost);	/* Get host's tty settings	*/
    ioctl(stdin, TIOCGETC, &charshost);	/* Get host's tty characters	*/
    #endif
  #endif
#endif
}

/* Set text i/o to raw */
void tty_raw(void)
{
#ifdef Z80IO_UNIX
  #ifdef TCSAFLUSH
  /* Linux-style tty settings */
  tcgetattr(STDIN_FILENO, &termz80);	/* Get tty settings		*/
  termz80.c_iflag &= ~ICRNL;		/* RETURN gives <cr>		*/
  termz80.c_lflag &= ~(ECHO | ICANON);	/* Turn off ECHO, EDITOR	*/
  termz80.c_cc[VINTR]=escchar;		/* Interupt is Escape		*/
  termz80.c_cc[VSUSP]=0;		/* Suspend is disabled		*/
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termz80);
  #else
    #ifdef TIOCGETP
    /* Old-style unix ioctl() tty settings */
    ioctl(stdout,TIOCGETP, &termz80);	/* Get again to modify them	*/
    termz80.sg_flags=((termz80.sg_flags & !RAW & !ECHO) | CBREAK);
					/* RAW, no ECHO, no EDITOR	*/
    ioctl(stdout,TIOCSETP, &termz80);
    ioctl(stdin,TIOCGETC, &charsz80);	/* Get tty characters		*/
    charsz80.t_intrc=27;		/* Interupt is Escape		*/
    charsz80.t_quitc=-1;		/* Suspend is disabled		*/
    ioctl(stdin,TIOCSETC,&charsz80);
    #endif
  #endif
#endif
}

/* Set text i/o for a call to host */
void tty_host(void) {
#ifdef Z80IO_UNIX
  #ifdef TCSAFLUSH
  /* Linux-style tty settings */
  memcpy(&termz80, &termhost, sizeof(termhost));
  termz80.c_cc[VINTR]=escchar;		/* Interupt is Escape		*/
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termz80);
  #else
    #ifdef TIOCGETP
    /* Old-style unix ioctrl() tty settings */
    ioctl(stdout,TIOCSETP, &termhost);	/* Set host's tty settings	*/
    memcpy(&charsz80, &charshost, sizeof(charshost));
    charsz80.t_intrc=escchar;		/* Interupt is Escape		*/
    ioctl(stdin,TIOCSETC,&charsz80);
    #endif
  #endif
#endif
}

/* Restore text i/o to host's settings */
void tty_quit(void)
{
#ifdef Z80IO_UNIX
  #ifdef TCSAFLUSH
  /* Linux-style tty settings */
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termhost);
  #else
    #ifdef TIOCGETP
    /* Old-style unix ioctrl() tty settings */
    ioctl(stdin, TIOCSETP, &termhost);
    ioctl(stdin, TIOCSETC, &charshost);
    #endif
  #endif
#endif
}


/* ==================================================================== */
/* Startup and finalisation						*/
/* ==================================================================== */
void io_start(void)
{
tty_init();				/* Initialise tty settings	*/
tty_raw();				/* Set tty to raw		*/
keyint_claim();				/* Trap keypress interupts	*/
#ifndef Z80IO_RO
con_init();				/* Initialise console		*/
#endif
disk_init();				/* Mount default images		*/
}

void io_finish(void)
{
disk_dismount(-1);			/* Close any open disk images	*/
#ifndef Z80IO_RO
con_quit();				/* Release console		*/
#endif
keyint_release();			/* Release keypress interupts	*/
tty_quit();				/* Restore to host's settings	*/
free(mem);				/* Release Z80 memory		*/
free(iomem);				/* Release I/O memory		*/
}


/* ==================================================================== */
/* Various bits								*/
/* ==================================================================== */
int get_hex(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  c = tolower(c);
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;
}


/* ============================================================ */
/* get_filetype - Returns the RISC OS file type of the named	*/
/* file or -1 if file cannot be found or the name passed is	*/
/* not that of a file						*/
/* ============================================================ */
int get_filetype(char *name) {
#ifdef Z80IO_RO
  _kernel_swi_regs regs;
  _kernel_oserror *error;
  regs.r[0] = 23;		/* Use OS_File call 23 to read file details */
  regs.r[1] = (int) name;
  error = _kernel_swi(OS_File, &regs, &regs);

/* R0 contains 1 if object is a file */

  if (error != NULL || regs.r[0] != 1) return -1;
  return regs.r[6];		/* R6 = file's filetype */
#endif
}


/* ==================================================================== */
/* Opcodes ED,F0 to ED,FF call host MOS routines.			*/
/* ==================================================================== */

/* ==================================================================== */
/* OSQUIT - Quit the system						*/
/* ==================================================================== */
void MOS_QUIT(void)
{
io_finish();				/* Disconnect any environment	*/
exit(0);				/* And exit			*/
}


/* ==================================================================== */
/* OSCLI - Execute an command						*/
/* ==================================================================== */
extern int lptr;
void MOS_CLI(void)
{
HLreg=Lreg | (Hreg << 8);		/* Point to string at HL	*/
addr=0;
for (addr=0; mem[HLreg]>31 && addr<250; HLreg++) iobuffer[addr++]=mem[HLreg];
iobuffer[addr]=0;			/* Copy HL string to iobuffer	*/
lptr=0;
while (iobuffer[lptr]==' ' || iobuffer[lptr]=='*') lptr++;
					/* Skip leading spaces and '*'s	*/
if (cli()) return;		        /* Internal command		*/

#ifdef Z80FILE_RO
/* Check for a CPM-type command (filetype &2xx) */
  tmp2 = get_filetype(&iobuffer[lptr]);
  if (tmp2 >= 0x200 || tmp2 < 0x300) {
    addr = (tmp2 & 0xFF) << 8;
    if (load(&iobuffer[lptr], addr) != -1) {
      PC = &mem[0] + addr;
      return;
    }
  }
_kernel_oscli("Unset Z80Tube$Flag");
if (chk_err(_kernel_system(&iobuffer[lptr],0)))
	return;				/* Error occured, return	*/
tmp=(int)_kernel_getenv("Z80Tube$Flag", iobuffer, 250);
if (tmp!=0 || iobuffer[0]==0) return;	/* Command completed externally */
addr=(get_hex(iobuffer[0]) << 12) | (get_hex(iobuffer[1]) << 8);
					/* Get address of file to load	*/
if (load(iobuffer+5,addr) != -1)
	PC=&mem[0]+addr;		/* Execute loaded file		*/
#else

if (iobuffer[lptr]=='/') {		/* /name becomes name		*/
  lptr++;
  while (iobuffer[lptr]==' ') lptr++;
  }
tty_host();				/* Use host's tty settings	*/
system(&iobuffer[lptr]);		/* Needs to check for errors	*/
tty_raw();				/* Restore back to raw tty	*/
#endif
}


/* ==================================================================== */
/* OSBYTE - Byte commands						*/
/* ==================================================================== */
void MOS_BYTE(void)
{
if (debug & 2) printf("OSBYTE %02X,%02X%02X\n",Areg,Hreg,Lreg);

Freg=Freg & 254;			/* Clear Carry			*/
tmp=(Hreg<<8)+Lreg;			/* 16-bit value of HL		*/
switch (Areg) {
#ifndef Z80IO_RO
  case 0:				/* Read host type		*/
    Lreg=Z80HOST;
    return;
#endif

  case 0x7C:
  case 0x7E:				/* Clear Escape			*/
    mem[0xFF80]=0;			/* then drop though to tell OS	*/
    break;

  case 0x7D:				/* Set Escape			*/
    mem[0xFF80]=128;			/* then drop though to tell OS	*/
    break;

#ifndef Z80IO_RO
  case 0x7F:				/* Read EOF#handle		*/
    if (Lreg) Lreg=file_eof(Lreg);	/* Call file.c			*/
    else      Lreg=(kbhit() ? 0 : 255);	/* IF NOT EOF#0 THEN keypending	*/
    return;

  case 0x80:				/* ADVAL(port)			*/
    switch (Lreg) {
      case 0xFF:			/* IF ADVAL(-1) THEN keypending	*/
        tmp=kbhit(); break;
      case 16:				/* ADVAL(16) - 16-bit GET	*/
        tmp=esc_rdch(1); break;
    }
    break;

  case 0x81:				/* INKEY			*/
    if (tmp == 0xFF00) {		/* INKEY-256			*/
        Lreg=Z80OS; Hreg=0;
        return;
      }    
    if (tmp >= 0xF000) {		/* INKEY-ve			*/
      Hreg=(Lreg=(con_keyscan(tmp) ? 0xFF : 0x00));
      return;
      }
    					/* Read timed			*/
    tmp=clock()+(tmp & 0x7fff)*(CLOCKS_PER_SEC/100);
    esc_off();				/* Allow kbhit() to detect Esc	*/
    for (;;) {				/* Wait for a key or timeout	*/
      if ((tmp2=kbhit()) || (clock()>tmp)) break;
      }
    if (tmp2) {
      tmp=esc_rdch(0);			/* Get keypress			*/
      } else {
      tmp=0xFFFF; esc_on();		/* No keypress			*/
#ifdef Z80IO_UNIX
      fflush(stdout);
#endif
      }
    if (Hreg < 0x80) tmp=tmp & 0x00FF;	/* Not INKEY(&8000+n)		*/
    Lreg=(int8)(tmp & 0xFF);		/* Character read		*/
    Hreg=(int8)(tmp >> 8);		/* High byte of character or -1	*/
    Freg=Freg | ((mem[0xFF80] & 128) >> 7);	/* Escape		*/
    return;
    
  case 0x86:				/* Read POS and VPOS		*/
    con_getxy(&tmp, &tmp2);
    Lreg=(int8)tmp; Hreg=(int8)tmp2;
    return;
    
  case 0xE5:				/* Enable/disable Escape	*/
    Lreg=escenable;			/* Return old state		*/
    escenable=(int8)tmp;		/* Set new state		*/
    return;    
#endif
  
  case 0x82:				/* Read Hi Order Address	*/
    Lreg=0; Hreg=0;
    return;

  case 0x83:				/* Read lowest PAGE		*/
    Lreg=0; Hreg=0x01;
    return;

  case 0x84:				/* Read HIMEM			*/
    Lreg=mem[6]; Hreg=mem[7];
    return;
  }

#ifdef Z80IO_RO
chk_err(tmp=_kernel_osbyte(Areg,Lreg,Hreg));
#endif

Lreg=tmp & 255;				/* Extract registers from tmp	*/
Hreg=(tmp & 0xFF00) >> 8;
Freg=Freg | ((tmp & 65536) >> 16);
}


/* ==================================================================== */
/* OSSWI - Do an SWI call - this has be here as MOS_WORD calls it	*/
/* ==================================================================== */

/* Format of control block:
 * HL=>0    number of in bytes, (number of regs)*4+8, usually 40
 *     1    number of out bytes, (number of regs)*4+8, usually 40
 *     2    outward translate flags, b0=r0, b1=r1, etc for Z80->Arm
 *     3    inward translate flags, b0=r0, b1=r1, etc for Arm-Z80
 *     4-7  SWI number
 *     8-11 r0
 *    12-15 r1
 *     etc
 *    36-39 r7
 */
void MOS_FF0D()
{
}


/* ==================================================================== */
/* OSWORD - Do an OSWORD						*/
/* Some calls need addresses translated:				*/
/*                             0123456789ABCDEF
   0: Read a line              01
   1: Read TIME
   2: Write TIME
   3: Read intervel timer
   4: Write interval timer
   5: Read memory
   6: Write memory
   7: SOUND
   8: ENVELOPE
   9: Read POINT(x,y)
  10: Read character definition
  11: Read palette
  12: Write palette
  13: Read graphics cursors
  14: Read TIME$
  15: Write TIME$
  16: Econet Transmit              456789AB
  17: Econet Receive                56789ABC
  18: Read RPC parameters
  19: Read/Set station info
  20: NFS Fileserver functions
  21: Mouse                    0     6789
  22: Screen address
  90: HADFS Read sectors       6 2345
 127: Read sectors              1234
 190: Disassemble code
 191: Assemble code
 200: SWI interface     (&ED0D)
 255: I/O processor memory access
*/
/* ==================================================================== */
void MOS_WORD(void)
{
int c;
int l;

HLreg=Lreg | (Hreg << 8);
addr32=mem[HLreg] | (mem[HLreg+1] << 8) | (mem[HLreg+2] << 16) | (mem[HLreg+3] << 24);
addr=addr32 & 0xFFFF;
if (debug & 2) printf("OSWORD %02X,%04X\n",Areg,HLreg);

switch (Areg) {
  case 0:				/* Read a line of input		*/
#ifdef Z80IO_RO
    regs.r[0]=addr+(int)&mem[0];	/* ARM address of input iobuffer */
    regs.r[1]=mem[HLreg+2];		/* iobuffer size		*/
    regs.r[2]=mem[HLreg+3];		/* min. ASCII			*/
    regs.r[3]=mem[HLreg+4];		/* max. ASCII			*/
    oserr2=_kernel_swi_c(0x0E, &regs, &regs, &tmp);
    Freg=(tmp & 1);
    Hreg=regs.r[1];			/* Returned length		*/
    if (oserr2 != NULL) z80error(oserr2->errnum, oserr2->errmess, "");
#else
    esc_off();				/* Escape processed by readln()	*/
    tmp=con_readln((char *)addr, mem[HLreg+2], mem[HLreg+3], mem[HLreg+4]);
    esc_on();				/* Escape is backgnd interupt	*/
    Freg=Freg & 254;
    if (tmp < 0) { Freg=Freg | 1; }
    else { Hreg=tmp; mem[addr+tmp]=13; }
#endif
    return;

#ifndef Z80IO_RO
  case 1:				/* Read TIME			*/
    tmp=(int)clock()/(CLOCKS_PER_SEC/100);    
    mem[HLreg]=tmp;
    mem[HLreg+1]=tmp >> 8;
    mem[HLreg+2]=tmp >> 16;
    mem[HLreg+3]=tmp >> 24;
    mem[HLreg+4]=0;
    return;
#endif

  case 5:				/* Read from memory		*/
    mem[HLreg+4]=*(char *)addr32;
    return;

  case 6:				/* Write to memory		*/
    *(char *)addr32=mem[HLreg+4];
    return;

  case 127:				/* FM direct disk I/O		*/
    osword_127();
    return;

#ifdef XxXx
  case 190:				/* Disassembly routine		*/
    if (mem[HLreg+2] == 80) {		/* Z80 disassem routine		*/
      mem[HLreg+3]=1; mem[HLreg+4]='?'; mem[HLreg+5]=13;
      }
    return;

  case 191:				/* Assembly routine		*/
    return;
#endif

  case 200:				/* SWI Interface		*/
    MOS_FF0D();				/* Pass on to ED0D handler	*/
    return;

  case 255:				/* I/O processor memory access */
    osword_255();
    return;

  default:				/* All other calls		*/
#ifdef Z80IO_RO
/* NB: Some calls need to have addresses relocated */
    for(l=0; l<250 & HLreg+l < 0xFFFF; l++) iobuffer[l]=mem[HLreg+l];
    chk_err(_kernel_osword(Areg, (int *)iobuffer));
    for(l=0; l<250 & HLreg+l < 0xFFFF; l++) mem[HLreg+l]=iobuffer[l];
#endif
    return;
  }
return;
}


/* ==================================================================== */
/* OSWRCH - Send a character to output stream				*/
/* ==================================================================== */
void MOS_WRCH(void)
{
#ifdef Z80IO_RO
chk_err(_kernel_oswrch(Areg));
#else
con_wrch(Areg);
#endif
}


/* ==================================================================== */
/* OSRDCH - Request an character from input stream			*/
/* ==================================================================== */
void MOS_RDCH(void)
{
#ifdef Z80IO_RO
chk_err(tmp=_kernel_osrdch());
#else
tmp=esc_rdch(1);			/* Read char, including Escape	*/
#endif
Areg=tmp & 0xFF;
Freg=(Freg & 254) | ((mem[0xFF80] & 128) >> 7);	/* Cy set from ESCFLG	*/
}


/* Adjust address of &0000xxxx to point to real memory */
/* To do: also support &FFFFxxxx for iomem CP/M buffer */
void adjustaddr(int *n, int *m)
{
#ifdef Z80IO_RO
if ((*n & 0xFFFF0000) == 0x00000000) {	/* Z80 main memory		*/
  *n=*n+(int)&mem[0];
  if (m != 0) *m=*m+(int)&mem[0];
  return;
  }

if ((*n & 0xFFFF0000) == 0xFFFF0000) {	/* CP/M buffer in I/O memory	*/
  *n=*n & 0x0FFF;
  *n=*n+(int)&iomem[0];
  if (m != 0) {
    *m=*m & 0x0FFF;
    *m=*m+(int)&iomem[0];
    }
  return;
  }
#endif
}


/* Check addresses before doing OSFILE */
void check_addresses()
{
#ifdef Z80IO_RO
/*  2345 6789 ABCD EF01
    load exec leng attr
R0   R2   R3   R4   R5
255 load flag             Load file with <File$Path>
0             start end   Save file
1    -
|    -                    Read/Write info with <File$Path>
v    -
9    -
10            start end   Save file with type 
11   -                    Create empty with type
12  load flag path        Load file with path
13            path        Read info with path
14  load flag path        Load file with <path>
15            path        Read info with <path>
16  load flag             Load with no path
17   -                    Read info with no path
18   -                    Set type and stamp
19                        Create error
*/
if (Areg > 11 && Areg < 16)  adjustaddr(&ctrl.start,0);
if (Areg == 0 || Areg == 10) adjustaddr(&ctrl.start,&ctrl.end);
if (Areg == 255 || Areg == 12 || Areg == 14 || Areg == 16)
  if ((ctrl.exec & 255) == 0) adjustaddr(&ctrl.load,0);
#endif
}


/* ==================================================================== */
/* OSFILE - Access whole files						*/
/* ==================================================================== */
void MOS_FILE(void)
{
#ifndef Z80FILE_RO
int loadL,execL,startL,endL;
int loadH,execH,startH,endH;
#endif

HLreg=Lreg | (Hreg << 8);
addr=mem[HLreg] | (mem[HLreg+1] << 8);
io_filename(addr);			/* Copy filename to iobuffer	*/

#ifdef Z80FILE_RO
ctrl.load =mem[HLreg+ 2]+(mem[HLreg+ 3] << 8)+(mem[HLreg+ 4] << 16)+(mem[HLreg+ 5] << 24);
ctrl.exec =mem[HLreg+ 6]+(mem[HLreg+ 7] << 8)+(mem[HLreg+ 8] << 16)+(mem[HLreg+ 9] << 24);
ctrl.start=mem[HLreg+10]+(mem[HLreg+11] << 8)+(mem[HLreg+12] << 16)+(mem[HLreg+13] << 24);
ctrl.end  =mem[HLreg+14]+(mem[HLreg+15] << 8)+(mem[HLreg+16] << 16)+(mem[HLreg+17] << 24);
check_addresses();			/* Check if addresses need altering */
chk_err(tmp=_kernel_osfile(Areg,iobuffer,&ctrl));
Areg=tmp;
mem[HLreg+2]=ctrl.load;
mem[HLreg+3]=ctrl.load >> 8;
mem[HLreg+4]=ctrl.load >> 16;
mem[HLreg+5]=ctrl.load >> 24;

mem[HLreg+6]=ctrl.exec;
mem[HLreg+7]=ctrl.exec >> 8;
mem[HLreg+8]=ctrl.exec >> 16;
mem[HLreg+9]=ctrl.exec >> 24;

mem[HLreg+10]=ctrl.start;
mem[HLreg+11]=ctrl.start >> 8;
mem[HLreg+12]=ctrl.start >> 16;
mem[HLreg+13]=ctrl.start >> 24;

mem[HLreg+14]=ctrl.end;
mem[HLreg+15]=ctrl.end >> 8;
mem[HLreg+16]=ctrl.end >> 16;
mem[HLreg+17]=ctrl.end >> 24;
#endif

#ifndef Z80FILE_RO
loadL =mem[HLreg+2] +(mem[HLreg+3]<<8);  loadH =mem[HLreg+4] +(mem[HLreg+5]<<8);
execL =mem[HLreg+6] +(mem[HLreg+7]<<8);  execH =mem[HLreg+8] +(mem[HLreg+9]<<8);
startL=mem[HLreg+10]+(mem[HLreg+11]<<8); startH=mem[HLreg+12]+(mem[HLreg+13]<<8);
endL  =mem[HLreg+14]+(mem[HLreg+15]<<8); endH  =mem[HLreg+16]+(mem[HLreg+17]<<8);

switch(Areg) {
  case 0xFF:				/* LOAD				*/
    if((execL & 0xFF) != 0) {
      z80error(252,"","LOAD address needed");  /* Generate an error	*/
      } else {
      tmp=load(iobuffer,loadL);		/* Try to load file		*/
      if (tmp == -1) {			/* Couldn't load file		*/
	z80error(214,iobuffer," not found");  /* Generate an error	*/
	} else {
	mem[HLreg+10]=tmp;		/* Return length in control block */
	mem[HLreg+11]=tmp >> 8;
	mem[HLreg+12]=0; mem[HLreg+13]=0;
	Areg=1;				/* Type=FILE			*/
	}
      }
    return;
    
  case 0x00:				/* SAVE				*/
    tmp=save(iobuffer,startL,endL,loadL,execL);
    if (tmp == -1) {			/* Couldn't save file		*/
      z80error(192,"Couldn't save ",iobuffer);  /* Generate an error	*/
      } else {
      mem[HLreg+10]=tmp;		/* Return length in control block */
      mem[HLreg+11]=tmp >> 8;
      mem[HLreg+12]=0; mem[HLreg+13]=0;
      Areg=1;				/* Type=FILE			*/
      }
    return;

  case 0x05:				/* Read file info		*/
    Areg=0;				/* Unsupported just now		*/
    return;
  }
#endif
}


/* ==================================================================== */
/* OSARGS - Information on open files and filing system			*/
/* ==================================================================== */
/* NOTE: A = function							*/
/*       HL=>data block							*/
/*       E = channel							*/
/*									*/
void MOS_ARGS(void)
{           
HLreg=Lreg | (Hreg << 8);
#ifdef Z80FILE_RO
chk_err(tmp=_kernel_osargs(Areg,Ereg,mem[HLreg]+(mem[HLreg+1] << 8)+(mem[HLreg+2] << 16)+(mem[HLreg+3] << 24)));
if ((Areg == 0) && (Ereg == 0)) {
  Areg=tmp;
  } else {
  mem[HLreg]=tmp;
  mem[HLreg+1]=tmp >> 8;
  mem[HLreg+2]=tmp >> 16;
  mem[HLreg+3]=tmp >> 24;
  Areg=0;
  }
#endif
if ((Areg == 0) && (Ereg == 0)) {
#ifdef Z80FILE_UNIX
  Areg=24;			/* UNIXFS				*/
#endif
#ifdef Z80FILE_DOS
  Areg=29;			/* DOSFS				*/
#endif
#ifdef Z80FILE_WIN
  Areg=29;			/* DOSFS				*/
#endif
  }
}


/* ==================================================================== */
/* OSBGET - Read a byte from files					*/
/* ==================================================================== */
void MOS_BGET(void)
{
#ifdef Z80FILE_RO
chk_err(Areg=_kernel_osbget(Hreg));
#endif
Freg=(Freg & 254) | ((Areg & 256) >> 8);
}


/* ==================================================================== */
/* OSBPUT - Write a byte to files					*/
/* ==================================================================== */
void MOS_BPUT(void)
{
#ifdef Z80FILE_RO
chk_err(_kernel_osbput(Areg,Hreg));
#endif
}


/* ==================================================================== */
/* OSGBPB - Read and Write multiple bytes and information		*/
/* ==================================================================== */
void MOS_GBPB(void)
{
int flag=0;

HLreg=Lreg | (Hreg << 8);

#ifdef Z80FILE_RO
  gbpb.dataptr =(void *)(mem[HLreg+1] | (mem[HLreg+2] << 8) | (mem[HLreg+3] << 16) | (mem[HLreg+4] << 24));
  gbpb.nbytes  =mem[HLreg+5] | (mem[HLreg+ 6] << 8) | (mem[HLreg+ 7] << 16) | (mem[HLreg+ 8] << 24);
  gbpb.fileptr =mem[HLreg+9] | (mem[HLreg+10] << 8) | (mem[HLreg+11] << 16) | (mem[HLreg+12] << 24);
  gbpb.wild_fld=(char *)(mem[HLreg+13] | (mem[HLreg+14] << 8) | (mem[HLreg+15] << 16) | (mem[HLreg+16] << 24));
  if (flag=(((int)gbpb.dataptr & 0xFFFF0000) == 0))
    gbpb.dataptr=(void *)((int)gbpb.dataptr + (int)&mem[0]);  /* Adjust */
  chk_err(tmp=_kernel_osgbpb(Areg, mem[HLreg], &gbpb));
  Areg=tmp;
  Freg=(Freg & 254) | ((tmp & 65536) >> 16);
/* Where is mem[HLreg+0] returned? */
  if (flag) gbpb.dataptr=(void *)((int)gbpb.dataptr - (int)&mem[0]); /* De-adjust */

  mem[HLreg+1]=(int)gbpb.dataptr;
  mem[HLreg+2]=(int)gbpb.dataptr >> 8;
  mem[HLreg+3]=(int)gbpb.dataptr >> 16;
  mem[HLreg+4]=(int)gbpb.dataptr >> 24;

  mem[HLreg+5]=gbpb.nbytes;
  mem[HLreg+6]=gbpb.nbytes >> 8;
  mem[HLreg+7]=gbpb.nbytes >> 16;
  mem[HLreg+8]=gbpb.nbytes >> 24;

  mem[HLreg+9]=gbpb.fileptr;
  mem[HLreg+10]=gbpb.fileptr >> 8;
  mem[HLreg+11]=gbpb.fileptr >> 16;
  mem[HLreg+12]=gbpb.fileptr >> 24;
#endif
}


/* ==================================================================== */
/* OSFIND - Open and close files					*/
/* ==================================================================== */
void MOS_FIND(void)
{
if (Areg != 0) {
  io_filename(Lreg | (Hreg << 8));	/* copy name to iobuffer	*/
#ifdef Z80FILE_RO
  chk_err(tmp=_kernel_osfind(Areg,iobuffer));		/* Open file	*/
#else
  tmp=0;
#endif
  Areg=tmp;
  } else {
#ifdef Z80FILE_RO
  chk_err(tmp=_kernel_osfind(Areg,(char *)Hreg));	/* Close file	*/
#endif
  }
}


/* ==================================================================== */
/* OSMISC - Miscellaneous emulator operations				*/
/* ==================================================================== */
void MOS_FF0C(void)
{
int flags;

switch(Areg) {

case 0:					/* Start Basic			*/
#ifdef Z80FILE_RO
  if(load("<Z80Tube$Basic>",0x0100) != -1) PC=&mem[0]+0x0100;
#endif
#ifdef Z80FILE_UNIX
  if(load("Z80Tube$BASIC",0x0100) != -1) PC=&mem[0]+0x0100;
#endif
#ifdef Z80FILE_DOS
/*  if (GetEnvironmentVariable("Z80TUBE$BASIC",iobuffer,254) != 0) {
    if(load(iobuffer,0x0100) != -1) PC=&mem[0]+0x0100;
    } */
#endif
#ifdef Z80FILE_WIN
  if (GetEnvironmentVariable("Z80TUBE$BASIC",iobuffer,254) != 0) {
    if(load(iobuffer,0x0100) != -1) PC=&mem[0]+0x0100;
    }
#endif
  break;

case 1:					/* Mount disk image 		*/
  HLreg=Lreg | (Hreg << 8);		/* Point at image name		*/
  Areg = disk_mount(Breg, (char *) &mem[0] + HLreg);
  break;

case 2:					/* Dismount disk		*/
  Areg = disk_dismount(Breg);
  break;

case 3:					/* Return disk information	*/
  HLreg=Lreg | (Hreg << 8);		/* Point at space for name	*/
  if (HLreg + Creg >= 0x10000) Creg = 0x10000 - HLreg;
  Areg = disk_info(Breg, Creg, (char *) &mem[0] + HLreg, &flags);
  Creg = (int8) flags;
  break;

default:
  break;
  }
}


/* ==================================================================== */
/* OSRDINF - Read emulator system information				*/
/* ==================================================================== */
void MOS_FF0E(void)
{
HLreg=Lreg | (Hreg << 8);
tmp=(int)&mem[0];
switch (Areg & 0x7F) {
  case 0:				/* Read start of Z80 memory	*/
    break;
  case 1:				/* Read error iobuffer address	*/
    tmp+=0xFF09;              
    break;
  case 2:				/* Read escape flag address	*/
    tmp+=0xFF80;
    break;
  case 3:				/* Read error vector		*/
    tmp+=0xFFFA;
    break;
  case 4:				/* Read event vector		*/
    tmp+=0xFFFC;
    break;
  case 5:				/* Read int vector		*/
    tmp+=0xFFFE;
    break;
  case 6:				/* Read NMI address		*/
    tmp+=0x0066;
    break;
  }
if(Areg < 0x80) {
  Freg=Freg & 254;
  tmp-=(int)&mem[0];
  if(tmp < 0 || tmp > 0xFFFF) Freg++;
  Lreg=tmp;
  Hreg=tmp >> 8;
  } else {
  mem[HLreg]=tmp;
  mem[HLreg+1]=tmp >> 8;
  mem[HLreg+2]=tmp >> 16;
  mem[HLreg+3]=tmp >> 24;
  }
}


/* ==================================================================== */
/* OSWRINF - Write emulator system information				*/
/* ==================================================================== */
void MOS_FF0F(void)
{
}


/* ==================================================================== */
/* String support routines						*/
/* ==================================================================== */

int skp_spc(p)				/* Skip spaces in iobuffer	*/
int p;
{
while(iobuffer[p] == 32) p++;
return(p);
}

/* Copies CR/SPC-terminated string into iobuffer with zero terminator	*/
int io_filename(int16 ptr)
{
int l;

while(mem[ptr]==32) ptr++;
for (l=0; mem[ptr]>32 && l<250; ptr++) iobuffer[l++]=mem[ptr];
iobuffer[l]=0;
return(l);				/* Returns length		*/
}


/* ==================================================================== */
/* Error handling							*/
/* ==================================================================== */

#ifdef Z80FILE_RO
/* Check if an error occured after calling RISC OS kernel		*/
int chk_err(int retval)
{
int l;

if (retval != -2) return FALSE;		/* No error			*/
oserr=*_kernel_last_oserror();		/* Find the error block		*/
mem[0xFF09]=0xFF;			/* RST &38 opcode		*/
mem[0xFF0A]=oserr.errnum;		/* Copy error message to iobuffer */
for(l=0; oserr.errmess[l] && l < 126; l++) mem[0xFF0B+l]=oserr.errmess[l];
mem[0xFF0B+l]=0;
mem[0xFF82]=9; mem[0xFF83]=0xFF;	/* Point FAULT to error block	*/
/* Lreg=0x0A; Hreg=0xFF; PC=mem[0xFFFA] | (mem[0xFFFB] << 8); */
PC=&mem[0]+0xFF09;			/* Force a jump to &FF09	*/
return TRUE;
}
#endif


/* Generate an error within the emulator				*/
z80error(int num, char *s1, char *s2)
{
int l,m;

for(l=0; s1[l]; l++) mem[0xff0b+l]=s1[l];
for(m=0; s2[m]; m++) mem[0xff0b+(l++)]=s2[m];
mem[0xff0b+l]=0;
mem[0xff09]=0xff;			/* RST &38 opcode		*/
mem[0xff0a]=num;			/* Error number			*/
mem[0xFF82]=0x0a; mem[0xFF83]=0xFF;	/* Point FAULT to error block	*/
PC=&mem[0]+0xFF09;			/* Force a jump to &FF09	*/
}


/* ==================================================================== */
/* I/O routines								*/
/* ==================================================================== */

void io_OUT(int port, int value)	/* Write a byte to output port	*/
{
printf("OUT (%04X),%d\x0a\x0d",port,value);
}

int io_IN(int port)			/* Read a byte from input port	*/
{
printf("IN (%04X)\x0a\x0d",port);
return(0);
}
