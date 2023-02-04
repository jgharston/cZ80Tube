/* console.h: Console keyboard and vdu library			*/
/* Copyright (C) J.G.Harston, 1997-2003,2018			*/
/* version 0.15							*/

extern void con_init();			/* Initialise				*/
extern void con_quit();			/* Finalise				*/
extern void con_wrch(int c);		/* Write a character to output		*/
extern int  con_rdch();			/* Read a byte from input stream	*/
extern int  con_keyscan(int key); 	/* Scan for keypress			*/
extern int  con_readln(int addr, int max, int lo, int hi);/* Read line of input	*/
extern void con_getxy(int *x, int *y);	/* Return text cursor position		*/
