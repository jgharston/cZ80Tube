/* cli.c
 * Copyright (C) 1987-2003 J.G.Harston

 Internal *commands

 Relevant acknowledgements, etc
 DaveDan: 31-Jul-2004 MOUNT/etc for disk images
 JGH:     04-Dec-2018 *CD <dir>, *SHOW changed to *MOUNTS
 JGH:     17-Dec-2018 *FX parses and passes to OSBYTE
 JGH:     21-Dec-2018 Generalised command parser called by *HELP

/* ==================================================================== */

extern char progtitle1[];

#ifdef Z80FILE_RO
char cli_cmd[]="BASIC\0DIR\0DISMOUNT\0FX\0HELP\0MOUNT\0MOUNTS\0QUIT\0CORE\0\0";
char cli_txt[]="\0  <dir>\0<drv>\0   <a>(,<x>(,<y>))\0 (<subject>)\0<drv> <afsp>\0\0\0\0";
#else
char cli_cmd[]="BASIC\0CD\0DISMOUNT\0FX\0HELP\0MOUNT\0MOUNTS\0QUIT\0CORE\0\0";
char cli_txt[]="\0   <dir>\0<drv>\0   <a>(,<x>(,<y>))\0 (<subject>)\0<drv> <afsp>\0\0\0\0";
#endif
int tptr;
int lptr;

int cli_chkerr(int test)
{
if (test<0) {
  z80error(254,"Bad command","");
  }
return test;
}


/* ==================================================================== */
/* cli_parsedec()
 * Parse 8-bit decimal number
 * On entry: address of pointer to command line
 * On exit:  pointer to command line updated
 *           returns decimal number
 *           leading and trailing spaces skipped
 *           returns <0 if no number or number>255
/* ==================================================================== */
int cli_parsedec(char** text)
{
int num;
char *command;

command=*text;
while (*command == ' ') command++;		/* Skip spaces		*/
if (*command < '0' || *command > '9') return -2; /* No number		*/
num = (*command++) - '0';
while (*command >= '0' && *command <='9') {
  num=num*10 + (*command++) - '0';
  if (num > 255) return -3;			/* num too big		*/
  }
while (*command == ' ') command++;		/* Skip spaces		*/
*text=command;
return num;
}

/* ==================================================================== */
/* cli_match(char *cmdtable)
 * Search a command table for a match
 * On entry: pointer to command table
 *           global lptr points to start of command string
 * On exit:  0 or command number
 *           global lptr updated to point to after command
/* ==================================================================== */
int cli_match(char *cmdtable)
{
int num=0;
int found=0;
int start=0;

start=lptr;				/* Start of command line	*/
tptr=0;					/* Index into table command	*/

while (cmdtable[tptr]) {		/* Stop when pointing to a zero	*/
  num++;				/* Entry number			*/
  for (lptr=start;
      ((iobuffer[lptr] == cmdtable[tptr]) ||
      ((iobuffer[lptr] ^ 0x20 )== cmdtable[tptr])) &&
      cmdtable[tptr];
      lptr++)
    tptr++;
  found=((cmdtable[tptr] == '\0' && iobuffer[lptr] < 'A')
       || iobuffer[lptr] == '.');	/* Remember if we match		*/
  if (iobuffer[lptr] == '.') lptr++;	/* Move past a '.'		*/
  while (cmdtable[tptr]) tptr++;	/* Move to next zero byte	*/
  if (!found) tptr++;			/* Move to next entry		*/
  }					/* Not at end zero, check more	*/
if (found) return num;
lptr=start;				/* Restore lptr			*/
return 0;
}


/* ==================================================================== */
/* Inbuilt command routines						*/
/* ==================================================================== */
int cli_cat()				/* *. (<dir>) (CAT passed to OS)*/
{
#ifdef Z80FILE_UNIX
tty_host(); system("ls");		/* Do an ls command		*/
tty_raw(); return(1);			/* Done it			*/
#endif
#if defined(Z80FILE_WIN) | defined(Z80FILE_DOS)
tty_host(); system("dir /w");		/* Do a dir command		*/
tty_raw(); return(1);			/* Done it			*/
#endif
return(0);				/* Don't claim it, pass to OS	*/
}

int cli_cd()				/* *CD <dir>			*/
{
#ifndef Z80FILE_RO
if (chdir(&iobuffer[0]+lptr))
  z80error(214,&iobuffer[0]+lptr," not found");
return(1);				/* Claim it			*/
#else
return(0);				/* Don't claim it, pass to OS	*/
#endif
}

int cli_basic()				/* *BASIC			*/
{
Areg=0;					/* Areg corrupted later		*/
MOS_FF0C();				/* Do *BASIC			*/
return(1);				/* Claim it			*/
}

int cli_fx()				/* *FX				*/
{
int areg=0, xreg=0, yreg=0;		/* Default parameters		*/
char *command;

command=&iobuffer[lptr];
if (*command == 0) {
  cli_chkerr(-2);			/* No parameters, Bad command	*/
  return;
  }

if (cli_chkerr(areg=cli_parsedec(&command))<0)
  return;					/* Get first parameter	*/
if (*command == ',') command++;			/* Step past any comma	*/
while (*command == ' ') command++;		/* Skip spaces		*/
if (*command) {
  if (cli_chkerr(xreg=cli_parsedec(&command))<0)
    return;					/* Get second parameter	*/
  if (*command == ',') command++;		/* Step past any comma	*/
  while (*command == ' ') command++;		/* Skip spaces		*/
  if (*command) {
    if (cli_chkerr(yreg=cli_parsedec(&command))<0);
      return;					/* Get third parameter	*/
    }
  }

Areg=(int8)areg; Lreg=(int8)xreg; Hreg=(int8)yreg;
MOS_BYTE();
return 1;					/* Claim command	*/
}

char help_cmd[]="Z80TUBE\0HOST\0\0";
int cli_help()				/* *HELP (<topic>)		*/
{
printf("\n\r%s\n\r",progtitle1);
if (cli_match(help_cmd)) {
  lptr=0; tptr=0;
  while(cli_cmd[tptr]) {
    putchar(32); putchar(32);
    while(cli_cmd[tptr]) putchar(cli_cmd[tptr++]);
    if(cli_txt[lptr]) {
      putchar(32);
      while(cli_txt[lptr]) putchar(cli_txt[lptr++]);
      }
    printf("\n\r");
    lptr++; tptr++;
    }
  return(1);				/* Internal HELP done		*/
  }
#ifdef Z80IO_RO
return 0;				/* Pass to OS			*/
#else
return 1;				/* HELP done			*/
#endif
}

int cli_quit()				/* *QUIT			*/
{
MOS_QUIT();
return 1;				/* Keep Norcroft happy		*/
}

int cli_core(void)			/* Dump system to disk		*/
{
FILE *fp;
fp = fopen("z80dump", "wb");
if (fp == 0) return 1;
fwrite(mem, 0x10000, 1, fp);
fclose(fp);
return(1);
}


/* ==================================================================== */
/* Try an internal command. Command is in iobuffer. Returns 0 if not	*/
/* an internal command. Returns <>0 if executed internally		*/
/* ==================================================================== */

/* Table of cli command functions (not actually used)			*/
/* static int (*cli_code[])(void) = {cli_basic,cli_help,cli_quit}; */

int cli()
{
int num=0;
int optr;

optr=lptr;
tptr=0;
if (iobuffer[lptr] == '.') {	/* *. - catalog				*/
  lptr++;
  lptr=skp_spc(lptr);
  cli_cat();
  return 1;
  }
num=cli_match(cli_cmd);		/* Match against command table		*/
if (num==0) return 0;		/* Speedily leave here if not for us	*/

/* We now are pointing to the character after the end of the command.
   IE: HELP     or H.  or LO.
           ^         ^       ^						*/

lptr=skp_spc(lptr);			/* Skip spaces			*/
switch (num) {				/* Execute internal command	*/
  case 1: tmp=cli_basic(); break;
  case 2: tmp=cli_cd(); break;
  case 3: tmp=cli_dismount(); break;	/* diskio.c			*/
  case 4: tmp=cli_fx(); break;
  case 5: tmp=cli_help(); break;
  case 6: tmp=cli_mount(); break;	/* diskio.c			*/
  case 7: tmp=cli_mounts(); break;	/* diskio.c			*/
  case 8: tmp=cli_quit(); break;
  case 9: tmp=cli_core(); break;
}

/* (*cli_code[num])();  Grrr - Can't get syntax working right */
lptr=optr;				/* Restore lptr			*/
return tmp;				/* Return unclaimed		*/
}

