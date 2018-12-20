/* cli.c
 * Copyright (C) 1987-2003 J.G.Harston

 Internal *commands

 Relevant acknowledgements, etc
 DaveDan: 31-Jul-2004 MOUNT/etc for disk images
 JGH:     04-Dec-2018 *CD <dir>, *SHOW changed to *MOUNTS

/* ==================================================================== */

extern char progtitle1[];

#ifdef Z80_FILE_RO
char cli_cmd[]="CAT\0BASIC\0DIR\0DISMOUNT\0FX\0HELP\0MOUNT\0MOUNTS\0QUIT\0CORE\0\0";
#else
char cli_cmd[]="CAT\0BASIC\0CD\0DISMOUNT\0FX\0HELP\0MOUNT\0MOUNTS\0QUIT\0CORE\0\0";
#endif
char cli_txt[]="(<dir>)\0\0<dir>\0<drv>\0<a>(,<x>(,<y>))\0(<subject>)\0<drv> <afsp>\0\0\0\0";
int t_ptr;
int l_ptr;


/* ==================================================================== */
/* Inbuilt command routines						*/
/* ==================================================================== */
int cli_cat()				/* *CAT (<dir>)			*/
{
#ifdef Z80_FILE_UNIX
set_cooked();
system("ls");				/* Do an ls command		*/
set_cbreak();
return(1);				/* Done it			*/
#endif
#ifdef Z80_FILE_WIN
set_cooked();
system("dir /w");			/* Do a dir command		*/
set_cbreak();
return(1);				/* Done it			*/
#endif
#ifdef Z80_FILE_RO
return(0);				/* Don't claim it, pass to OS	*/
#endif
}

int cli_cd()				/* *CD <dir>			*/
{
#ifndef Z80_FILE_RO
if (chdir(&iobuffer[0]+l_ptr))
  z80error(214,&iobuffer[0]+l_ptr," not found");
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

int cli_fx()				/* *FX - null for the moment	*/
{
return(1);				/* Claim it			*/
}

int cli_help()				/* *HELP (<topic>)		*/
{
printf("\n%s",progtitle1);
if(   (iobuffer[l_ptr] == '.') ||
     ((iobuffer[l_ptr] & 0xDF) == 'Z') ||
    (((iobuffer[l_ptr] & 0xDF) == 'H')  &&  ((iobuffer[l_ptr+1] & 0xDF) == 'O'))
  ) {
  l_ptr=0; t_ptr=0;
  while(cli_cmd[t_ptr]) {
    putchar(32); putchar(32);
    while(cli_cmd[t_ptr]) putchar(cli_cmd[t_ptr++]);
    if(cli_txt[l_ptr]) {
      putchar(32);
      while(cli_txt[l_ptr]) putchar(cli_txt[l_ptr++]);
      }
    printf("\n");
    l_ptr++; t_ptr++;
    }
  return(1);				/* Internal HELP done		*/
  }
#ifdef Z80_IO_RO
return(0);				/* Pass to OS			*/
#else
return(1);				/* HELP done			*/
#endif
}

int cli_quit()				/* *QUIT			*/
{
(void)MOS_QUIT();
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

/* Table of cli command functions (not actually used!)			*/
/* static int (*cli_code[])(void) = {cli_basic,cli_help,cli_quit}; */

int cli()
{
int l_start=0;
int num=0;
int found=0;

t_ptr=0;
l_ptr=0;
tmp=0;					/* Initialise to 'claimed'	*/
					/* Skip spaces and '*'s		*/
while(iobuffer[l_start] == 32 || iobuffer[l_start] == '*') l_start++;

while(cli_cmd[t_ptr]) {			/* Stop when pointing to a zero	*/
  num++;				/* Entry number			*/
  for(l_ptr=l_start;
      (iobuffer[l_ptr] & 0xDF) == cli_cmd[t_ptr] && cli_cmd[t_ptr];
      l_ptr++)
    t_ptr++;
  found=((cli_cmd[t_ptr] == '\0' && iobuffer[l_ptr] < 'A')
       || iobuffer[l_ptr] == '.');	/* Remember if we match		*/
  if(iobuffer[l_ptr] == '.') l_ptr++;	/* Move past a '.'		*/
  while(cli_cmd[t_ptr]) t_ptr++;	/* Move to next zero byte	*/
  if(!found) t_ptr++;			/* Move to next entry		*/
  }				/* If not at end zero, go round for more */
if(!found) return(0);		/* Speedily leave here if not for us	*/
/* We now are pointing to the character after the end of the command.
   IE: HELP     or H.  or LO.
           ^         ^       ^						*/
l_ptr=skp_spc(l_ptr);			/* Skip spaces			*/

switch (num) {				/* Execute internal command	*/
  case 1:  tmp=cli_cat();	break;
  case 2:  tmp=cli_basic();	break;
  case 3:  tmp=cli_cd();	break;
  case 4:  tmp=cli_dismount();	break;	/* diskio.c			*/
  case 5:  tmp=cli_fx();	break;
  case 6:  tmp=cli_help();	break;
  case 7:  tmp=cli_mount();	break;	/* diskio.c			*/
  case 8:  tmp=cli_mounts();	break;	/* diskio.c			*/
  case 9:  tmp=cli_quit();	break;
  case 10: tmp=cli_core();	break;
}

/* (*cli_code[num])();  Grrr - Can't get syntax working right */
return(tmp);				/* Return claimed/unclaimed	*/
}

