/* main.c - C Z80 Emulator Mark 2
 * Copyright (C) 1987-2003 J.G.Harston

  See the accompanying file LICENSE for terms of use.
 */

/* include malloc.h ?*/

void proginfo(void)			/* Print program information	*/
{
/* Print startup message, something like:
   Z80Tube Emulator (C Code) Version 2.25
   Compiled on 25 Apr 2003 at 12:22:32
   Copyright (C)1987-2003 J.G.Harston
 */
printf("%s%sCopyright (C)1987-2018 J.G.Harston\n\n",progtitle1,progtitle2);
}

main(argc,argv)
int argc;
char *argv[];
{

int l,m;
FILE *fd1;

progtitle2[DATE_START+3]=progtitle2[DATE_START+0];	/* Fix date	*/
progtitle2[DATE_START+0]=progtitle2[DATE_START+4];
progtitle2[DATE_START+4]=progtitle2[DATE_START+1];
progtitle2[DATE_START+1]=progtitle2[DATE_START+5];
progtitle2[DATE_START+5]=progtitle2[DATE_START+2];
progtitle2[DATE_START+2]=' ';
if (progtitle2[DATE_START+0] == ' ') progtitle2[DATE_START+0]='0';

if((iomem=(int8 *)malloc(IOMSIZE)) == 0) exit(1);
					/* Claim I/O memory		*/
if((mem=(int8 *)malloc(65536+130)) == 0) exit(1);

for(l=0; l<266; l++) mem[l]=mem0[l];	/* Initialise main memory	*/
for(l=0; l<266; l++) {
  mem[0xFF00+l]=mem[l+8];		/* Copy mini-mos to high memory */
  mem[l+8]=0;				/* and clear zero page		*/
  }
mem[14]=0xE1; mem[15]=0xE9;		/* CALL 14 -> LD HL,PC		*/

SP=0xFF00;				/* Initial stack below mini-mos */
PC=&mem[0]+0xFF03;			/* Prepared if nothing loaded	*/

/* If started with no parameters, should really start silently with
   default MOS and default program, ie the equivalent of
   z80 -mos Z80TUBE$MOS Z80TUBE$BASIC
 */

/* if (argc == 1) proginfo();		/* No parameters, print intro	*/
l=1;					/* Loop though parameters	*/
while (l < argc) {
  if (argv[l][0] == '-') {
    m=argv[l][1];
    switch (m) {
      case 'q':				/* -quit			*/
      break;				/* Ignore, as always quits	*/
      case 'm':				/* -mos				*/
      l++;
      m=load(argv[l],-1);		/* Load to &10000-length	*/
      mem[1]=(m+6); mem[2]=(m+6) >> 8;	/* Make JP 0 point to MOS+6	*/
      mem[6]=m; mem[7]=m >> 8;		/* Make JP 5 point to MOS+0	*/
      PC=&mem[0]+m+3;			/* Start executing at MOS+3	*/
      SP=m-2;				/* Set stack to below MOS	*/
      mem[SP]=0; mem[SP+1]=0;		/* RET address jumps to 0000	*/
      break;
    case 'd':				/* -debug			*/
      l++;
      debug=argv[l][0];			/* Should really decode hex value */
      break;
    default:				/* All other options		*/
      if (argv[l][1] == 'h' || argv[l][1] == '?') {
      	proginfo();			/* -help, -?, print program info*/
	}
      printf("Usage: %s [-debug num] [-mos file] [file [params]]\n",argv[0]);
      exit(0);
      break;
    }
  l++;					/* Move to next parameter	*/
  }
  if (l < argc)
    if (argv[l][0] != '-') break;	/* No more options		*/
}

if (argv[l]) {				/* File specified, load it	*/
  load(argv[l],0x0100);
  PC=&mem[0]+0x0100;
  l++;
  }

if (l < argc) {				/* Copy any more parameters to	*/
					/* Parameter buffer at 0x0080	*/
  for (m=0; (argv[l][m] != 0) && (m < 120); m++) mem[0x81+m]=argv[l][m];
  mem[0x80]=m;				/* Only copies one param atm	*/
  } else {
  proginfo();				/* No parameters, print intro	*/
  }

/* Usage: z80 -mos <mos_file> program params				*
 *        any other params go to program				*
 * eg: z80 -mos <z80$dir>.mos <z80$dir>.bbcbasic Prog1 			*
 * needs pointer to CPM disk?						*
 */
 
/* Clear all registers, PC and SP have already been set			*/
Areg=0; Breg=0; Creg=0; Dreg=0; Ereg=0; Hreg=0; Lreg=0; Freg=0;
Aalt=0; Balt=0; Calt=0; Dalt=0; Ealt=0; Halt=0; Lalt=0; Falt=0;
XYreg[0]=0; XYreg[1]=0; Ireg=0; Rreg=0; IM=0;

execute();				/* Run the Z80 engine		*/
}

/* end of main.c */
