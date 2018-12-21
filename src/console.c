/* console.c: Console keyboard and vdu library			*/
/* ------------------------------------------------------------	*/
/* Copyright (C) J.G.Harston, 1997-2003,2018			*/
/* version 0.11: con_wrch(9) uses gotoxy()			*/
/* version 0.12: added local versions of CONIO funcions		*/
/* added con_getxy(), filled in and corrected JP keys		*/
/* Ctrl/Shift handling of special keys rewritten		*/
/* version 0.13: added ANSI input and output			*/
/*								*/
/* ANSI output can be enabled with #define CONVDU_ANSI		*/
/* linux target defaults to using CONVDU_ANSI			*/
/* ANSI input can be enabled with #define CONKBD_ANSI		*/
/* linux target defaults to using CONKBD_ANSI			*/
/*								*/


#ifdef __WIN32__
#define CONKBD_PC
#define CONVDU_PC
#include <windows.h>
#include <wincon.h>
#include <conio.h>
// #include <user32.h>
#include <io.h>
#endif

#ifdef __DOS__
#define CONKBD_PC
#define CONVDU_PC
#include <stdio.h>
#include <conio.h>
#ifndef VK_SHIFT
#include <keysym.h>
#endif
#endif

#ifdef __linux__
#define CONKBD_ANSI
#define CONVDU_ANSI
#include <stdio.h>
#include <sys/types.h>
#endif


#ifdef CONVDU_PC
#ifndef USECONIO
/* If CONIO does not provide these functions, provide them ourselves */
int wherex()
{
CONSOLE_SCREEN_BUFFER_INFO  csbiInfo;
GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbiInfo);
return csbiInfo.dwCursorPosition.X+1;
}

int wherey()
{
CONSOLE_SCREEN_BUFFER_INFO  csbiInfo;
GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbiInfo);
return csbiInfo.dwCursorPosition.Y+1;
}

void gotoxy(int x, int y)
{
COORD coord;
coord.X = x-1;
coord.Y = y-1;
SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void clrscr()
{ system("cls"); }	/* MS-recommended bodge */
#endif /* USECONIO */
#else /* CONVDU_PC */

int wherex() { return 0; }
int wherey() { return 0; }

int kbhit()  {
#ifdef FIONREAD
int num=0;
ioctl(STDIN_FILENO, FIONREAD, &num);
return num;
#else
return 1;
#endif
} 

#ifdef CONVDU_ANSI
void clrscr() { fputs("\x1B[2J\x1B[1;1H",stdout); }
void gotoxy(int x, int y) { printf("\x1B[%d;%dH",y&255,x&255); }
#else /* CONVDU_ANSI */
void clrscr() { system("cls"); }
void gotoxy(int x, int y) { }
#endif /* CONVDU_ANSI */

#endif /* !CONKBD_PC */


/* VDU queue and variables */
char vduq[10];
int vduqlen=0;
#ifdef CONVDU_ANSI
char ansichars[]="\x1B[0;1;5;4;7;30;40m";
int ansifgd='7'; int ansibgd='0';
#endif


/* con_wrch(int c) - write a character to output			*/
/* -------------------------------------------------------------------- */
/* int c = character to output						*/
/*									*/
void con_wrch(int c)
{
char *seq;

if (isatty(fileno(stdout)) == 0) { putchar(c); return; }
if (vduqlen) {
  vduq[vduqlen--]=c;
  if (vduqlen != 0) return;
  switch(vduq[0]) {
#ifdef CONVDU_ANSI
    case 17:						/* COLOUR	*/
      seq=&ansichars[4];
      if (vduq[1] & 8)  { *seq++='1'; *seq++=';'; }	/* Bright	*/
      if (vduq[1] & 16) { *seq++='5'; *seq++=';'; }	/* Flash	*/
      if (vduq[1] & 32) { *seq++='4'; *seq++=';'; }	/* Underline	*/
      if (vduq[1] & 64) { *seq++='7'; *seq++=';'; }	/* Inverse	*/
      if (vduq[1] & 128)  ansibgd='0'+(vduq[1] & 7);	/* Background	*/
      else                ansifgd='0'+(vduq[1] & 7);	/* Foreground	*/
      if (ansibgd=='0') ansibgd='9';
      *seq++='3'; *seq++=ansifgd; *seq++=';';
      *seq++='4'; *seq++=ansibgd; *seq++='m';
      *seq++=0;
      fputs(ansichars,stdout);
    break;
#endif
    case 22:					/* MODE		*/
#ifdef CONVDU_ANSI
      fputs("\x1B[0m",stdout);			/* Reset colours*/
      ansifgd='7'; ansibgd='0';
#endif
      clrscr(); break;
    case 31: gotoxy(vduq[2]+1,vduq[1]+1); break; /* TAB()	*/
    }
  return;
  }
switch (c) {
#ifdef CONVDU_ANSI
  case 9:   fputs("\x1B[C",stdout); break;	/* Move right	*/
  case 11:  fputs("\x1B[A",stdout); break;	/* Move upwards	*/
#else
  case 9:   gotoxy(wherex()+1,wherey()); break;	/* Move right	*/
  case 11:  gotoxy(wherex(),wherey()-1); break;	/* Move upwards	*/
#endif
  case 12:  clrscr(); break;			/* CLS		*/
  case 17:  vduq[0]=c; vduqlen=1; break;	/* COLOUR	*/
#ifdef CONVDU_ANSI
  case 20:  fputs("\x1B[0m",stdout);		/* Reset colours*/
            ansifgd='7'; ansibgd='0';
            break;
#endif
  case 22:  vduq[0]=c; vduqlen=1; break;	/* MODE		*/
  case 23:  vduq[0]=c; vduqlen=9; break;	/* VDU 23	*/
  case 30:  gotoxy(1,1); break;			/* HOME		*/
  case 31:  vduq[0]=c; vduqlen=2; break;	/* TAB()	*/
  case 127: putchar(8); putchar(32); putchar(8); break;
  default:  putchar(c);
  }
}


/* con_readln(addr, max, lo, hi) - read a line of input			*/
/* con_readln(addr, max, lo, hi, flags) - read a line of input		*/
/* -------------------------------------------------------------------- */
/* char *addr = location to store entered line				*/
/* int max    = longest permissable line				*/
/* int lo     = lowest allowable character				*/
/* int hi     = highest allowable character				*/
/* int flags  = 							*/
/* Returns:								*/
/* int        = length of entered line (offset to terminator)		*/
/*		or <0 for Escape					*/
/*              memory at addr will hold entered line, terminated with	*/
/*		a zero byte. memory after terminator may be overwritten	*/
/*		up to maximum buffer size.				*/
/*									*/
/* Line can be edited with:						*/
/*		BS, DEL: delete preceding character			*/
/*		Ctrl-U:  delete whole line				*/
/*		LF, CR:  enter line					*/
/*		Escape:  abort entry					*/
/*									*/
int con_readln(char *addr, int max, int lo, int hi)
{
int c,l;

l=0; c=0;
for (;;) {
  c=con_rdch();
  if (c > 31 && c != 127 && c < 0x100 && l < max) {
    mem[(int)addr++]=c;
    con_wrch(c);
    l++;
    }
  if ((c == 127 || c == 8 || c == 0x1c7 || c == 21) && l != 0) {
    for (;l > 0;) {
      addr--; l--;
      con_wrch(127);
      if (c != 21) break;
      }				/* NB: WinConsole doesn't wrap past col 0 */
    }
  if (c == 27 || c == 13 || c == 10) break;
  }
mem[(int)addr]=0;
if (c == 27) return (-1);
con_wrch(10); con_wrch(13);
return l;
}


#ifdef CONKBD_PC
/* key <0xC0 is returned key, else base key to xor with Shift/Ctrl/Alt	*/
unsigned char winkey[]={
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, /* 00-07			*/
0xc4,0xc1,0xc5,0x0b,0x0c,0x0d,0x0e,0x0f, /* sBS,sTAB,sRET,0C-0F		*/
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17, /* 10-17			*/
0x18,0x19,0x1a,0xc3,0x1c,0x1d,0x1e,0x1f, /* 18-1A,sESC,1C-1F		*/
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27, /* 20-27			*/
0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f, /* 28-2F			*/
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37, /* 30-37			*/
0x38,0x39,0x3a,0x81,0x82,0x83,0x84,0x85, /* 38-3A,F1-F5			*/
0x86,0x87,0x88,0x89,0x8a,0x45,0x46,0xc8, /* F6-F10,45,46,Home		*/
0xcf,0xcb,0x4a,0xcc,0x4c,0xcd,0x4e,0xc9, /* Up,PgUp,4A,<-,4C,->,4E,End	*/
0xce,0xca,0xc6,0xc7,0x91,0x92,0x93,0x94, /* Down,PgDn,Ins,Del,sF1-sF4	*/
0x95,0x96,0x97,0x98,0x99,0x9a,0xa1,0xa2, /* sF5-sF10,cF1,cF2		*/
0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa, /* cF3-cF10			*/
0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8, /* aF1-aF8			*/
0xb9,0xba,0xa0,0xcc,0xcd,0xc9,0xca,0xc8, /* aF9-aF10,cPrint,c<-,c->,cEnd,cPgDn,cHome */
0xcb,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f, /* cPgUp,79-7F				*/
0x80,0x81,0x82,0x83,0x84,0x8b,0x8c,0x9b, /* 80-84,F11-12,sF11			*/
0x9c,0xab,0xac,0xbb,0xbc,0xcf,0x8e,0x8f, /* sF12,cF11-12,aF11-12,cUp,8E-8F	*/
0x90,0xce,0xc6,0xc7,0xc1,0x95,0x96,0xc8, /* 90,cDown,cIns,cDel,cTab,95,96,aHome	*/
0xcf,0xcb,0x9a,0xcc,0x9c,0xcd,0x9e,0xc9, /* aUp,aPgUp,9A,a<-,9C,a->,9E,aEnd	*/
0xce,0xca,0xc6,0xc7,0xa4,0xc1,0xa6,0xa7, /* aDn,aPgDn,aIns,aDel,A4,aTab,A6-A7	*/
0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf, /* A8-AF				*/
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,
0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,
0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,
0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,
0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,
0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff };
#endif

#ifdef CONKBD_ANSI
unsigned char ansikey[]={
0x00,0xC8,0xC6,0xC7,0xC9,0xCB,0xCA,0x00, /* 0,Home,Ins,Del,End,PgUp,PgDn,7 */
0x00,0x00,0x00,0x81,0x82,0x83,0x84,0x85, /* 8,9,10,F1,F2,F3,F4,F5          */
0x00,0x86,0x87,0x88,0x89,0x8A,0x00,0x8B, /* 16,F6,F7,F8,F9,F10,22,F11      */
0x8C,0x8D,0x8E,0x00,0x8F,0x00,0x00,0x00, /* F12,F13,F14,27,F15,F16,30,F17  */
0x00,0x00,0x00,0x00,0xCC,0xCD,0xCE,0xCF  /* F18,F19,F20,35,<-,->,Down,Up   */
};
#endif


/* con_rdch() - read a byte from input stream				*/
/* -------------------------------------------------------------------- */
/* Returns:								*/
/* int        = character from input stream, or character from keyboard	*/
/*		translated to give regular character numbers. 		*/
/*		Returns 0x000+x for regular characters and 0x100+x for	*/
/*		special keys: cursors, function keys, etc.		*/
/*									*/
int con_rdch()
{
int ch,s,c,a;
if (isatty(fileno(stdin)) == 0) if ((ch=getchar()) != EOF) return(ch);

#ifndef CONKBD_ANSI
#ifdef CONKBD_PC
ch=getch();				/* Read from console		*/
/* to do: if <0x20 and modifier pressed, do a translate */
if (ch != 0 && ch != 0xe0) return(ch);	/* Standard keyboard chars	*/
a=(GetAsyncKeyState(VK_MENU)<0);	/* Check for modifier keys	*/
c=(GetAsyncKeyState(VK_CONTROL)<0);
s=(GetAsyncKeyState(VK_SHIFT)<0);
ch=getch();
if (ch == 0x86)				/* Either F12 or cPgUp		*/
  if (c) ch=0x78;			/* Remap cPgUp			*/
if (((ch=winkey[ch] | 0x100) & 0xc0) == 0x80)
  return(ch);				/* Translate keys, return 80-bf	*/
if (s) ch=ch ^ 0x10;			/* SHIFT pressed		*/
if (c) ch=ch ^ 0x20;			/* CTRL pressed			*/
if (a) ch=ch ^ 0x30;			/* ALT pressed			*/
return(ch);				/* Other keys need extra help	*/
#else
return getchar();
#endif
#else

int key=0;
int mod=0;

fflush(stdout);
read(STDIN_FILENO, &ch, 1);		/* Read without flushing	*/
ch=ch & 0xFF;
if (ch != 27)     return ch;		/* Not <esc>			*/
if (kbhit() == 0) return ch;		/* Nothing pending		*/

/* ANSI key sequence is:
 *    <esc> [ (<num>) (;<num>) nondigit
 * or <esc> O nondigit			*/

ch=getchar();
if (ch=='O') mod=1;			/* Convert <esc>O to <esc>[1	*/
  else if(ch!='[') return (ch | 0x100);	/* Not opening <esc>[ or <esc>O	*/

while ((ch=getchar())<'@') {		/* Parse through non-alphas	*/
  if (ch>='0' && ch<='9') {		/* Digit - add to current num	*/
    mod=mod*10+(ch-'0');
    }
  if (ch==';') {			/* Semi - step to next number	*/
    key=mod; mod=0;
    }
  }
if (key==0) { key=mod; mod=1; }		/* Special cases		*/
if (ch>='A' && ch<='D') key=39-(ch-'A');
if (ch>='P' && ch<='S') key=11+(ch-'P');
if (ch=='F') key=4;
if (ch=='H') key=1;
mod=mod-1;				/* Convert modifiers to bitmap	*/
ch=ansikey[key];			/* Translate keypress		*/
if (mod & 1) ch=ch ^ 0x10;		/* SHIFT pressed		*/
if (mod & 4) ch=ch ^ 0x20;		/* CTRL pressed			*/
if (mod & 2) ch=ch ^ 0x30;		/* ALT pressed			*/
return (ch | 0x100);
#endif
}


#ifdef CONKBD_PC
unsigned char winmap[]={
VK_SHIFT,	/* -001  Shift        */
VK_CONTROL,	/* -002  Ctrl         */
VK_MENU,	/* -003  Alt          */
VK_LSHIFT,	/* -004  Left Shift   */
VK_LCONTROL,	/* -005  Left Ctrl    */
VK_LMENU,	/* -006  Left Alt     */
VK_RSHIFT,	/* -007  Right Shift  */
VK_RCONTROL,	/* -008  Right Ctrl   */
VK_RMENU,	/* -009  Right Alt    */
VK_LBUTTON,	/* -010  Mouse Select */
VK_RBUTTON,	/* -011  Mouse Menu   */
VK_MBUTTON,	/* -012  Mouse Adjust */
0,		/* -013  FN           */
0,		/* -014               */
0,		/* -015               */
0,		/* -016               */
'Q',		/* -017  Q            */
'3',		/* -018  3            */
'4',		/* -019  4            */
'5',		/* -020  5            */
VK_F4,		/* -021  F4           */
'8',		/* -022  8            */
VK_F7,		/* -023  F7           */
0xbd,		/* -024  -            */
0, // 0xde,	/* -025  ^            */
VK_LEFT,	/* -026  Left         */
VK_NUMPAD6,	/* -027  Keypad 6     */
VK_NUMPAD7,	/* -028  Keypad 7     */
VK_F11,		/* -029  F11          */
VK_F12,		/* -030  F12          */
VK_F10,		/* -031  F10          */
VK_SCROLL,	/* -032  Scroll Lock  */
VK_SNAPSHOT,	/* -033  F0/Print     */
'W',		/* -034  W            */
'E',		/* -035  E            */
'T',		/* -036  T            */
'7',		/* -037  7            */
'I',		/* -038  I            */
'9',		/* -039  9            */
'0',		/* -040  0            */
0xbd,		/* -041  _            */
VK_DOWN,	/* -042  Down         */
VK_NUMPAD8,	/* -043  Keypad 8     */
VK_NUMPAD9,	/* -044  Keypad 9     */
VK_PAUSE,	/* -045  Break        */
0xdf,		/* -046  `/~/?        */
0, // 0xdc,	/* -047  UKP/Yen      */
VK_BACK,	/* -048  Backspace    */
'1',		/* -049  1            */
'2',		/* -050  2            */
'D',		/* -051  D            */
'R',		/* -052  R            */
'6',		/* -053  6            */
'U',		/* -054  U            */
'O',		/* -055  O            */
'P',		/* -056  P            */
0xdb,		/* -057  [            */
VK_UP,		/* -058  Up           */
VK_ADD,		/* -059  Keypad +     */
VK_SUBTRACT,	/* -060  Keypad -     */
VK_RETURN,	/* -061  Keypad Enter - same as Return */
VK_INSERT,	/* -062  Insert       */
VK_HOME,	/* -063  Home         */
VK_PRIOR,	/* -064  PgUp         */
VK_CAPITAL,	/* -065  Caps Lock    */
'A',		/* -066  A            */
'X',		/* -067  X            */
'F',		/* -068  F            */
'Y',		/* -069  Y            */
'J',		/* -070  J            */
'K',		/* -071  K            */
0xc0,		/* -072  @            */
0, // 0xba,	/* -073  :            */
VK_RETURN,	/* -074  Return - Same as Keypad Enter */
VK_DIVIDE,	/* -075  Keypad /     */
VK_DECIMAL,	/* -076  Keypad Del   */
VK_DECIMAL,	/* -077  Keypad .     */
VK_NUMLOCK,	/* -078  Num Lock     */
VK_NEXT,	/* -079  PgDn         */
0xc0,		/* -080  '/"  '/@     */
0,		/* -081  Shift Lock   */
'S',		/* -082  S            */
'C',		/* -083  C            */
'G',		/* -084  G            */
'H',		/* -085  H            */
'N',		/* -086  N            */
'L',		/* -087  L            */
0xba,		/* -088  ;            */
0xdd,		/* -089  ]            */
VK_DELETE,	/* -090  Delete       */
0xde,		/* -091  Keypad # #/~ */
VK_MULTIPLY,	/* -092  Keypad *     */
VK_SEPARATOR,	/* -093  Keypad ,     */
0xbb,		/* -094  =/+          */
0xdc,		/* -095  Left \,|     */
0xe2,		/* -096  Right \,_    */
VK_TAB,		/* -097  TAB          */
'Z',		/* -098  Z            */
' ',		/* -099  Space        */
'V',		/* -100  V            */
'B',		/* -101  B            */
'M',		/* -102  M            */
0xbc,		/* -103  ,            */
0xbe,		/* -104  .            */
0xbf,		/* -105  /            */
VK_END,		/* -106  Copy/End     */
VK_NUMPAD0,	/* -107  Keypad 0     */
VK_NUMPAD1,	/* -108  Keypad 1     */
VK_NUMPAD3,	/* -109  Keypad 3     */
VK_NONCONVERT,	/* -110  NoConvert    */
VK_CONVERT,	/* -111  Convert      */
VK_KANA,	/* -112  Kana         */
VK_ESCAPE,	/* -113  Escape       */
VK_F1,		/* -114  F1           */
VK_F2,		/* -115  F2           */
VK_F3,		/* -116  F3           */
VK_F5,		/* -117  F5           */
VK_F6,		/* -118  F6           */
VK_F8,		/* -119  F8           */
VK_F9,		/* -120  F9           */
0xdc,		/* -121  \,|          */
VK_RIGHT,	/* -122  Right        */
VK_NUMPAD4,	/* -123  Keypad 4     */
VK_NUMPAD5,	/* -124  Keypad 5     */
VK_NUMPAD2,	/* -125  Keypad 2     */
VK_LWIN,	/* -126  WinLeft      */
VK_RWIN,	/* -127  WinRight     */
VK_APPS};	/* -128  WinMenu      */
#endif


/* con_keyscan(int key) - scan for keypress, negative INKEY		*/
/* -------------------------------------------------------------------- */
/* key = 0xffxx - Translated keyscan INKEY -1/&FFFF to -256/&FF00	*/
/* key = 0xfexx - Direct API call INKEY -257/&FEFF to -512/&FE00	*/
/* Returns:								*/
/* int = non-zero if key is pressed, 0 if key is not pressed		*/
/*									*/
int con_keyscan(int key)
{
#ifdef CONKBD_PC

key = (key & 0xffff) ^ 0xffff;		/* Convert to keyscan number	*/
if (key <0x080) {			/* Scan for single key		*/
#ifndef DJGPP			/* DJGPP doesn't support GetKbdLayout	*/
  if (((int)(GetKeyboardLayout(0)) & 0xFFFF)==0x0411) {
				/* BBC layout keyboard			*/
/* Note: if a console app, this is the ID from when the program started	*/
    switch (key) {
      case 24: return (GetAsyncKeyState(0xDE)<0 ? -1 : 0); /* ^~      */
      case 46: return (GetAsyncKeyState(0xDC)<0 ? -1 : 0); /* ?/Y/etc */
      case 72: return (GetAsyncKeyState(0xBA)<0 ? -1 : 0); /* :       */
      case 87: return (GetAsyncKeyState(0xBB)<0 ? -1 : 0); /* ;       */
      case 90:						   /* #/~     */
      case 93:						   /* =/+     */
      case 94:						   /* Left \| */
      case 120:						   /* \|      */
        return 0;
    }
  }
#endif /* !DJGPP */
  key=winmap[key];			/* Get translated keyscan code	*/
  if (key) return(GetAsyncKeyState(key)<0 ? -1 : 0);
    else return(0);			/* Return -1 if key pressed	*/
  }

if (key < 0x100) return(TRUE);		/* Scan range - unimplemented	*/

if (key < 0x200)			/* Direct API call		*/
  return(GetAsyncKeyState(key ^ 0x1ff)<0 ? -1 : 0);
#endif

return(0);				/* Everything else returns FALSE*/
}


/* con_getxy(int *x, int *y) - return cursor position			*/
/* -------------------------------------------------------------------- */
/*									*/
void con_getxy(int *x, int *y)
{
*x=wherex()-1;
*y=wherey()-1;
}


/* con_init() - initialise console					*/
/* -------------------------------------------------------------------- */
void con_init() { }


/* con_quit() - finalise console					*/
/* -------------------------------------------------------------------- */
void con_quit()
{
#ifdef CONVDU_ANSI
if (isatty(fileno(stdout)) == 0) con_wrch(20);	/* Reset colours	*/
#endif
}
