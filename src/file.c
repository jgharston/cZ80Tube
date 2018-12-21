/* file.c - file-specific routines
 * Copyright (C) 1987-2003 J.G.Harston

 30-Aug-2003 JGH: File routines separated into file.c

 If targetting RISC OS should really use native system calls
 */


/* ============================================================ */
/* Attempt to load a file					*
   *name - filename - tries name conversion
   addr  - =-1 load to top of memory, returns load address
           >=0 load to address, returns loaded file length
   If file not openable, returns -1
/* ============================================================ */
int load(char *name, int addr)
{
FILE *fd1;
int b,start,load;

start=addr; load=addr;
if ((fd1=fopen(name,"rb")) == NULL) {	/* Look for a file	*/
  for (b=0; name[b]; b++);		/* Find end marker	*/
  add_ext(&name[b],".bbc");
  if ((fd1=fopen(name,"rb")) == NULL) {
    add_ext(&name[b],",bbc");
    if ((fd1=fopen(name,"rb")) == NULL) {
      add_ext(&name[b],",ffb");
      if ((fd1=fopen(name,"rb")) == NULL) {
      name[b]=0;
      return(-1);			/* Couldn't find a file	*/      
      }
    }
  }
}
if (addr == -1) {
  fseek(fd1,0,SEEK_END);		/* Find file length		*/
  load=0x10000 - ftell(fd1);		/* Load to end of memory	*/
  fseek(fd1,0,SEEK_SET);		/* Move back to file start	*/
  }
addr=load;
while ((b=getc(fd1))!=EOF && addr<0x10000) mem[addr++]=b;
fclose(fd1);
if (start == -1) return(load);		/* Return load address from top	*/
return(addr-load);			/* Return length		*/
}


/* ============================================================ */
/* Attempt to save a file					*
   *name - filename
   start - first byte to save
   end   - byte after last byte to save
   exec  - 
   load  -
   Returns -1 if unable to save or length if saved
/* ============================================================ */
int save(char *name, int start, int end, int exec, int load)
{
FILE *fp1;
int oldstart;

oldstart=start;
if(start > end) return(0);
if((fp1=fopen(name,"wb"))==NULL) return(-1);

while(start < end && start < 0x10000) putc(mem[start++],fp1);
fclose(fp1);
return(end-oldstart);			/* Return length		*/
}

/* ============================================================ */
/* Add extension to a filename					*/
/* ============================================================ */
int add_ext(char *l, char *ext)
{
int m=0;

while(ext[m]) *l++=ext[m++];
return 0;			/* Avoids Norcroft warning	*/
}

/* ============================================================ */
/* Test end of file of open channel				*/
/* ============================================================ */
int file_eof(int a)
{
return(0);
}

