/* z80.c - C Z80 Emulator Mark 2, Copyright (C)1987-2003 J.G.Harston
 * Source code generated Wed,02 Jul 2008.00:29:33
 * 20-May-1993 JGH: Got correct (*)f syntax (tortuous!)
 * 21-May-1993 JGH: Corrected for truth being a single bit
 * 22-May-1993 JGH: undef() moves PC back 2 temporarily
 *                  completed xy codes
 *                  IN/OUT, ED opcodes, SBC/ADC
 *                  INC/DEC dr single statement
 * 24-May-1993 JGH: XYreg[2], not [1]; BIT->&AD;
 *                  Sign/Parity check corrected
 * 25-May-1993 JGH: INC/DEC r corrected, Freg=... made one expression
 *                  Added default 256-byte MOS
 * 26-May-1993 JGH: POP/PUSH AF fixed; LD r,(XY+n) needed PC++
 *                  inc ccdo(), ccret: PV flag is 4 not 2
 *                  Parity table generated; sub/sbc/cp was doing '+'
 *                  Tidied up Freg=... in alu operations
 *                  LD I/R<->A, RLD, RRD, ADD XY,dr
 * 31-May-1993 JGH: don't use f=...; main loop is for(;;)...
 *                  Made PC a pointer; mos stored in mem[]
 * 03-Jun-1993 JGH: IM 0/1/2; RETN/RETI done
 * 25-May-1995 JGH: CALL BDOS just RETs
 * 13-Jul-1995 JGH: In-built small MOS 0.55
 *                  Stack and PC set up to run MOS
 * 29-Aug-2003 JGH: Some fixes for compiling on Win
 * 30-Aug-2003 JGH: Separated into easilier-maintained sections
 * 30-Aug-2004 JGH: CPI/CPD/LDI/LDD was setting P/V wrong
 */

/* z80 engine variables and memory defined in memory.c */

/* Title message */
char progtitle1[]="Z80Tube Emulator (C Code) Version 0.27\n\x00";
char progtitle2[]="Compiled on "__DATE__" at "__TIME__"\n\x00";
#define DATE_START 12

/* Now create the actual z80 interpreter */

/* Parity table */
static char parity[] = { 4, 0, 0, 4, 0, 4, 
4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 
4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 
4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 
0, 4, 0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 
4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 
4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 
4, 0, 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 
4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 
4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 
4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 
4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 
0, 4, 0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 
4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 
0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 
0, 4, 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 
0, 4, 0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 
4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 
4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 
4, 0, 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 
0, 4, 0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 
4, 0, 0, 4, 4, 0, 4, 0, 0, 4 };

void undef()
{
PC=PC-2; disp_regs(); PC=PC+2;
}

/* XYCBnnxx codes */

void xyCB_06() /* RLC (xy+n) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[addr];
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | (Cy & 1);
mem[addr]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void xyCB_0E() /* RRC (xy+n) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[addr];
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy) tmp=(tmp | 128);
mem[addr]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void xyCB_16() /* RL (xy+n) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[addr];
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | Cy2;
mem[addr]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void xyCB_1E() /* RR (xy+n) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[addr];
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy2) tmp=(tmp | 128);
mem[addr]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void xyCB_26() /* SLA (xy+n) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[addr];
tmp=tmp << 1;
Cy=(tmp > 255);
mem[addr]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void xyCB_2E() /* SRA (xy+n) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[addr];
Cy=(tmp & 1);
tmp=tmp >> 1;
if (tmp & 64) tmp=(tmp | 128);
mem[addr]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void xyCB_36() /* SLS (xy+n) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[addr];
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | 1;
mem[addr]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void xyCB_3E() /* SRL (xy+n) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[addr];
Cy=(tmp & 1);
tmp=tmp >> 1;
mem[addr]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void xyCB_46() /* BIT 0,(xy+n) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[addr] & 0x1)==0) Freg=Freg | 64;
}

void xyCB_4E() /* BIT 1,(xy+n) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[addr] & 0x2)==0) Freg=Freg | 64;
}

void xyCB_56() /* BIT 2,(xy+n) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[addr] & 0x4)==0) Freg=Freg | 64;
}

void xyCB_5E() /* BIT 3,(xy+n) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[addr] & 0x8)==0) Freg=Freg | 64;
}

void xyCB_66() /* BIT 4,(xy+n) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[addr] & 0x10)==0) Freg=Freg | 64;
}

void xyCB_6E() /* BIT 5,(xy+n) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[addr] & 0x20)==0) Freg=Freg | 64;
}

void xyCB_76() /* BIT 6,(xy+n) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[addr] & 0x40)==0) Freg=Freg | 64;
}

void xyCB_7E() /* BIT 7,(xy+n) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[addr] & 0x80)==0) Freg=Freg | 64;
}

void xyCB_86() /* RES 0,(xy+n) */
{
mem[addr]=(mem[addr] & 0xFE);
}

void xyCB_8E() /* RES 1,(xy+n) */
{
mem[addr]=(mem[addr] & 0xFD);
}

void xyCB_96() /* RES 2,(xy+n) */
{
mem[addr]=(mem[addr] & 0xFB);
}

void xyCB_9E() /* RES 3,(xy+n) */
{
mem[addr]=(mem[addr] & 0xF7);
}

void xyCB_A6() /* RES 4,(xy+n) */
{
mem[addr]=(mem[addr] & 0xEF);
}

void xyCB_AE() /* RES 5,(xy+n) */
{
mem[addr]=(mem[addr] & 0xDF);
}

void xyCB_B6() /* RES 6,(xy+n) */
{
mem[addr]=(mem[addr] & 0xBF);
}

void xyCB_BE() /* RES 7,(xy+n) */
{
mem[addr]=(mem[addr] & 0x7F);
}

void xyCB_C6() /* SET 0,(xy+n) */
{
mem[addr]=(mem[addr] | 0x1);
}

void xyCB_CE() /* SET 1,(xy+n) */
{
mem[addr]=(mem[addr] | 0x2);
}

void xyCB_D6() /* SET 2,(xy+n) */
{
mem[addr]=(mem[addr] | 0x4);
}

void xyCB_DE() /* SET 3,(xy+n) */
{
mem[addr]=(mem[addr] | 0x8);
}

void xyCB_E6() /* SET 4,(xy+n) */
{
mem[addr]=(mem[addr] | 0x10);
}

void xyCB_EE() /* SET 5,(xy+n) */
{
mem[addr]=(mem[addr] | 0x20);
}

void xyCB_F6() /* SET 6,(xy+n) */
{
mem[addr]=(mem[addr] | 0x40);
}

void xyCB_FE() /* SET 7,(xy+n) */
{
mem[addr]=(mem[addr] | 0x80);
}

/* Table of addresses for index registers CB codes */
static void (*codeXYcb[])(void) = {xyCB_06, xyCB_0E,
    xyCB_16, xyCB_1E, xyCB_26, xyCB_2E, xyCB_36, xyCB_3E, xyCB_46, xyCB_4E, 
    xyCB_56, xyCB_5E, xyCB_66, xyCB_6E, xyCB_76, xyCB_7E, xyCB_86, xyCB_8E, 
    xyCB_96, xyCB_9E, xyCB_A6, xyCB_AE, xyCB_B6, xyCB_BE, xyCB_C6, xyCB_CE, 
    xyCB_D6, xyCB_DE, xyCB_E6, xyCB_EE, xyCB_F6, xyCB_FE };




/* XYxx code (index register codes) */

void xy00() /* Index no-op */
{ undef(); }

void xy09() /* ADD  XY,BC */
{
tmp=XYreg[XY];
tmp2=Creg | (Breg << 8);
tmp=tmp+tmp2;
Freg=(Freg & 0xFC);  /* H should also get set */
if (tmp > 65535) Freg++;
XYreg[XY]=tmp;
}

void xy19() /* ADD  XY,DE */
{
tmp=XYreg[XY];
tmp2=Ereg | (Dreg << 8);
tmp=tmp+tmp2;
Freg=(Freg & 0xFC);  /* H should also get set */
if (tmp > 65535) Freg++;
XYreg[XY]=tmp;
}


void xy21() /* LD XY,nnnn */
{
XYreg[XY]=*(PC++);
XYreg[XY]=XYreg[XY] | (*(PC++) << 8);
}

void xy22() /* LD (nnnn),XY */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
mem[tmp++]=XYreg[XY];
mem[tmp]=(XYreg[XY] >> 8);
}

void xy23() /* INC XY */
{
XYreg[XY]++;
}

void xy24() /* INC YH */
{
tmp=((XYreg[XY] >> 8)+1) & 255;
Freg=(Freg & 0xE9);
if (tmp == 0x80) Freg=(Freg | 4);
if ((tmp & 0xF) == 0) Freg=(Freg | 16);
if (tmp == 0) Freg=(Freg | 64);
Freg=(Freg | (tmp & 128));
XYreg[XY]=(XYreg[XY] & 255) | (tmp << 8);
}

void xy25() /* DEC YH */
{
tmp=((XYreg[XY] >> 8)-1) & 255;
Freg=((Freg & 0xE9) | 2);
if (tmp == 0x7F) Freg=(Freg | 4);
if ((tmp & 0xF) == 15) Freg=(Freg | 16);
if (tmp == 0) Freg=(Freg | 64);
Freg=(Freg | (tmp & 128));
XYreg[XY]=(XYreg[XY] | 255) | (tmp << 8);
}

void xy26() /* LD YH,n */
{
XYreg[XY]=(XYreg[XY] & 0xFF) | (*(PC++) << 8);
}

void xy29() /* ADD  XY,HL */
{
tmp=XYreg[XY];
tmp2=XYreg[XY];
tmp=tmp+tmp2;
Freg=(Freg & 0xFC);  /* H should also get set */
if (tmp > 65535) Freg++;
XYreg[XY]=tmp;
}

void xy2A() /* LD XY,(nnnn) */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
XYreg[XY]=mem[tmp++];
XYreg[XY]=XYreg[XY] | (mem[tmp] << 8);
}

void xy2B() /* DEC XY */
{
XYreg[XY]--;
}

void xy2C() /* INC YL */
{
tmp=((XYreg[XY] & 255)+1) & 255;
Freg=(Freg & 0xE9);
if (tmp == 0x80) Freg=(Freg | 4);
if ((tmp & 0xF) == 0) Freg=(Freg | 16);
if (tmp == 0) Freg=(Freg | 64);
Freg=(Freg | (tmp & 128));
XYreg[XY]=(XYreg[XY] & 0xFF00) | tmp;
}

void xy2D() /* DEC YL */
{
tmp=((XYreg[XY] & 255)-1) & 255;
Freg=((Freg & 0xE9) | 2);
if (tmp == 0x7F) Freg=(Freg | 4);
if ((tmp & 0xF) == 15) Freg=(Freg | 16);
if (tmp == 0) Freg=(Freg | 64);
Freg=(Freg | (tmp & 128));
XYreg[XY]=(XYreg[XY] | 0xFF00) | tmp;
}

void xy2E() /* LD YL,n */
{
XYreg[XY]=(XYreg[XY] & 0xFF00) | *(PC++);
}

void xy34() /* INC (XY+n) */
{
PC++;
tmp=(++mem[addr] & 255);
Freg=(Freg & 0xE9);
if (tmp == 0x80) Freg=(Freg | 4);
if ((tmp & 0xF) == 0) Freg=(Freg | 16);
if (tmp == 0) Freg=(Freg | 64);
Freg=(Freg | (tmp & 128));
}

void xy35() /* DEC (XY+n) */
{
PC++;
tmp=(--mem[addr] & 255);
Freg=((Freg & 0xE9) | 2);
if (tmp == 0x7F) Freg=(Freg | 4);
if ((tmp & 0xF) == 15) Freg=(Freg | 16);
if (tmp == 0) Freg=(Freg | 64);
Freg=(Freg | (tmp & 128));
}

void xy36() /* LD (XY+n),m */
{
PC++;
mem[addr]=*(PC++);
}

void xy39() /* ADD  XY,SP */
{
tmp=XYreg[XY];
tmp2=SP;
tmp=tmp+tmp2;
Freg=(Freg & 0xFC);  /* H should also get set */
if (tmp > 65535) Freg++;
XYreg[XY]=tmp;
}

void xy44() /* LD  B,YH */
{
Breg=XYreg[XY] >> 8;
}

void xy45() /* LD  B,YL */
{
Breg=XYreg[XY];
}

void xy46() /* LD  B,(XY+n) */
{
PC++;
Breg=mem[addr];
}

void xy4C() /* LD  C,YH */
{
Creg=XYreg[XY] >> 8;
}

void xy4D() /* LD  C,YL */
{
Creg=XYreg[XY];
}

void xy4E() /* LD  C,(XY+n) */
{
PC++;
Creg=mem[addr];
}

void xy54() /* LD  D,YH */
{
Dreg=XYreg[XY] >> 8;
}

void xy55() /* LD  D,YL */
{
Dreg=XYreg[XY];
}

void xy56() /* LD  D,(XY+n) */
{
PC++;
Dreg=mem[addr];
}

void xy5C() /* LD  E,YH */
{
Ereg=XYreg[XY] >> 8;
}

void xy5D() /* LD  E,YL */
{
Ereg=XYreg[XY];
}

void xy5E() /* LD  E,(XY+n) */
{
PC++;
Ereg=mem[addr];
}

void xy60() /* LD  YH,B */
{
XYreg[XY]=(XYreg[XY] & 255) | (Breg << 8);
}

void xy61() /* LD  YH,C */
{
XYreg[XY]=(XYreg[XY] & 255) | (Creg << 8);
}

void xy62() /* LD  YH,D */
{
XYreg[XY]=(XYreg[XY] & 255) | (Dreg << 8);
}

void xy63() /* LD  YH,E */
{
XYreg[XY]=(XYreg[XY] & 255) | (Ereg << 8);
}

void xy64() /* LD YH,YH */
{ }

void xy65() /* LD YH,YL */
{
XYreg[XY]=(XYreg[XY] & 255) | ((XYreg[XY] & 255) << 8);
}

void xy66() /* LD H,(XY+n) */
{
PC++;
Hreg=mem[addr];
}

void xy67() /* LD YH,A */
{
XYreg[XY]=(XYreg[XY] & 255) | (Areg << 8);
}

void xy68() /* LD YL,B */
{
XYreg[XY]=(XYreg[XY] & 0xFF00) | Breg;
}

void xy69() /* LD YL,C */
{
XYreg[XY]=(XYreg[XY] & 0xFF00) | Creg;
}

void xy6A() /* LD YL,D */
{
XYreg[XY]=(XYreg[XY] & 0xFF00) | Dreg;
}

void xy6B() /* LD YL,E */
{
XYreg[XY]=(XYreg[XY] & 0xFF00) | Ereg;
}

void xy6C() /* LD YL,YH */
{
XYreg[XY]=(XYreg[XY] & 0xFF00) | (XYreg[XY] >> 8);
}

void xy6D() /* LD YL,YL */
{ }

void xy6E() /* LD L,(XY+n) */
{
PC++;
Lreg=mem[addr];
}

void xy6F() /* LD YL,A */
{
XYreg[XY]=(XYreg[XY] & 0xFF00) | Areg;
}

void xy70() /* LD (XY+n),B */
{
PC++;
mem[addr]=Breg;
}

void xy71() /* LD (XY+n),C */
{
PC++;
mem[addr]=Creg;
}

void xy72() /* LD (XY+n),D */
{
PC++;
mem[addr]=Dreg;
}

void xy73() /* LD (XY+n),E */
{
PC++;
mem[addr]=Ereg;
}

void xy74() /* LD (XY+n),H */
{
PC++;
mem[addr]=Hreg;
}

void xy75() /* LD (XY+n),L */
{
PC++;
mem[addr]=Lreg;
}

void xy76() /* LD (XY+n),(HL) */
{
}

void xy77() /* LD (XY+n),A */
{
PC++;
mem[addr]=Areg;
}

void xy7C() /* LD A,YH */
{
Areg=(XYreg[XY] >> 8);
}

void xy7D() /* LD A,YL */
{
Areg=XYreg[XY];
}

void xy7E() /* LD A,(XY+n) */
{
PC++;
Areg=mem[addr];
}

void xy86() /* ADD A,(XY+n) */
{
PC++;
tmp=mem[addr];
val=tmp;
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void xy8E() /* ADC A,(XY+n) */
{
PC++;
tmp=mem[addr];
val=tmp+(Freg & 1);
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void xy96() /* SUB A,(XY+n) */
{
PC++;
tmp=mem[addr];
val=tmp;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xy9E() /* SBC A,(XY+n) */
{
PC++;
tmp=mem[addr];
val=tmp+(Freg & 1);
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xyA6() /* AND A,(XY+n) */
{
PC++;
tmp=mem[addr];
Areg=(Areg & tmp);
Freg=(Areg & 128) | ((Areg == 0) << 6) | 16 | parity[Areg];
}

void xyAE() /* XOR A,(XY+n) */
{
PC++;
tmp=mem[addr];
Areg=(Areg ^ tmp);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xyB6() /* OR  A,(XY+n) */
{
PC++;
tmp=mem[addr];
Areg=(Areg | tmp);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xyBE() /* CP  A,(XY+n) */
{
PC++;
tmp=mem[addr];
val=tmp;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xyCB() /* Index rotate, etc, instructions */
{
PC++;
(*codeXYcb[*(PC++) >> 3])();     /* Call routine */
}

void xyDD() /* xyDD */
{ undef(); }

void xyE1() /* POP XY */
{
XYreg[XY]=mem[SP++];
XYreg[XY]=XYreg[XY] | (mem[SP++] << 8);
}

void xyE3() /* EX (SP),XY */
{
tmp=XYreg[XY];
XYreg[XY]=mem[SP] | (mem[SP+1] << 8);
mem[SP]=tmp & 255;
mem[SP+1]=tmp >> 8;
}

void xyE5() /* PUSH XY */
{
mem[--SP]=XYreg[XY] >> 8;
mem[--SP]=XYreg[XY] & 255;
}

void xyE9() /* JP (XY) */
{
PC=&mem[0]+XYreg[XY];
}

void xyF9() /* LD SP,XY */
{
SP=XYreg[XY];
}

void xyFD() /* xyFD */
{ undef(); }

/* Table of addresses for index register codes */
static void (*codeXY[])(void) = {xy00, xy00, xy00, xy00, xy00, xy00, 
     xy00, xy00, xy00, xy09, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy00, 
     xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy19, xy00, xy00, xy00, xy00, 
     xy00, xy00, xy00, xy21, xy22, xy23, xy24, xy25, xy26, xy00, xy00, xy29, 
     xy2A, xy2B, xy2C, xy2D, xy2E, xy00, xy00, xy00, xy00, xy00, xy34, xy35, 
     xy36, xy00, xy00, xy39, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy00, 
     xy00, xy00, xy44, xy45, xy46, xy00, xy00, xy00, xy00, xy00, xy4C, xy4D, 
     xy4E, xy00, xy00, xy00, xy00, xy00, xy54, xy55, xy56, xy00, xy00, xy00, 
     xy00, xy00, xy5C, xy5D, xy5E, xy00, xy60, xy61, xy62, xy63, xy64, xy65, 
     xy66, xy67, xy68, xy69, xy6A, xy6B, xy6C, xy6D, xy6E, xy6F, xy70, xy71, 
     xy72, xy73, xy74, xy75, xy00, xy77, xy00, xy00, xy00, xy00, xy7C, xy7D,
     xy7E, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy86, xy00, xy00, xy00, 
     xy00, xy00, xy00, xy00, xy8E, xy00, xy00, xy00, xy00, xy00, xy00, xy00, 
     xy96, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy9E, xy00, xy00, xy00, 
     xy00, xy00, xy00, xy00, xyA6, xy00, xy00, xy00, xy00, xy00, xy00, xy00, 
     xyAE, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xyB6, xy00, xy00, xy00, 
     xy00, xy00, xy00, xy00, xyBE, xy00, xy00, xy00, xy00, xy00, xy00, xy00, 
     xy00, xy00, xy00, xy00, xy00, xyCB, xy00, xy00, xy00, xy00, xy00, xy00, 
     xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xyDD,
     xy00, xy00, xy00, xyE1, xy00, xyE3, xy00, xyE5, xy00, xy00, xy00, xyE9, 
     xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy00, xy00, 
     xy00, xy00, xy00, xyF9, xy00, xy00, xy00, xyFD, xy00, xy00 };



/* CBxx codes */
void cb00() /* RLC B */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Breg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | (Cy & 1);
Breg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb01() /* RLC C */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Creg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | (Cy & 1);
Creg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb02() /* RLC D */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Dreg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | (Cy & 1);
Dreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb03() /* RLC E */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Ereg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | (Cy & 1);
Ereg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb04() /* RLC H */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Hreg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | (Cy & 1);
Hreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb05() /* RLC L */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Lreg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | (Cy & 1);
Lreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb06() /* RLC (HL) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[Lreg | (Hreg << 8)];
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | (Cy & 1);
mem[Lreg | (Hreg << 8)]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb07() /* RLC A */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Areg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | (Cy & 1);
Areg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb08() /* RRC B */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Breg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy) tmp=(tmp | 128);
Breg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb09() /* RRC C */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Creg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy) tmp=(tmp | 128);
Creg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb0A() /* RRC D */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Dreg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy) tmp=(tmp | 128);
Dreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb0B() /* RRC E */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Ereg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy) tmp=(tmp | 128);
Ereg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb0C() /* RRC H */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Hreg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy) tmp=(tmp | 128);
Hreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb0D() /* RRC L */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Lreg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy) tmp=(tmp | 128);
Lreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb0E() /* RRC (HL) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[Lreg | (Hreg << 8)];
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy) tmp=(tmp | 128);
mem[Lreg | (Hreg << 8)]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb0F() /* RRC A */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Areg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy) tmp=(tmp | 128);
Areg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb10() /* RL B */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Breg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | Cy2;
Breg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb11() /* RL C */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Creg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | Cy2;
Creg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb12() /* RL D */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Dreg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | Cy2;
Dreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb13() /* RL E */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Ereg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | Cy2;
Ereg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb14() /* RL H */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Hreg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | Cy2;
Hreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb15() /* RL L */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Lreg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | Cy2;
Lreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb16() /* RL (HL) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[Lreg | (Hreg << 8)];
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | Cy2;
mem[Lreg | (Hreg << 8)]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb17() /* RL A */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Areg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | Cy2;
Areg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb18() /* RR B */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Breg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy2) tmp=(tmp | 128);
Breg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb19() /* RR C */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Creg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy2) tmp=(tmp | 128);
Creg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb1A() /* RR D */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Dreg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy2) tmp=(tmp | 128);
Dreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb1B() /* RR E */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Ereg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy2) tmp=(tmp | 128);
Ereg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb1C() /* RR H */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Hreg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy2) tmp=(tmp | 128);
Hreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb1D() /* RR L */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Lreg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy2) tmp=(tmp | 128);
Lreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb1E() /* RR (HL) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[Lreg | (Hreg << 8)];
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy2) tmp=(tmp | 128);
mem[Lreg | (Hreg << 8)]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb1F() /* RR A */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Areg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy2) tmp=(tmp | 128);
Areg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb20() /* SLA B */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Breg;
tmp=tmp << 1;
Cy=(tmp > 255);
Breg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb21() /* SLA C */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Creg;
tmp=tmp << 1;
Cy=(tmp > 255);
Creg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb22() /* SLA D */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Dreg;
tmp=tmp << 1;
Cy=(tmp > 255);
Dreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb23() /* SLA E */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Ereg;
tmp=tmp << 1;
Cy=(tmp > 255);
Ereg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb24() /* SLA H */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Hreg;
tmp=tmp << 1;
Cy=(tmp > 255);
Hreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb25() /* SLA L */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Lreg;
tmp=tmp << 1;
Cy=(tmp > 255);
Lreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb26() /* SLA (HL) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[Lreg | (Hreg << 8)];
tmp=tmp << 1;
Cy=(tmp > 255);
mem[Lreg | (Hreg << 8)]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb27() /* SLA A */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Areg;
tmp=tmp << 1;
Cy=(tmp > 255);
Areg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb28() /* SRA B */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Breg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (tmp & 64) tmp=(tmp | 128);
Breg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb29() /* SRA C */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Creg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (tmp & 64) tmp=(tmp | 128);
Creg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb2A() /* SRA D */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Dreg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (tmp & 64) tmp=(tmp | 128);
Dreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb2B() /* SRA E */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Ereg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (tmp & 64) tmp=(tmp | 128);
Ereg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb2C() /* SRA H */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Hreg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (tmp & 64) tmp=(tmp | 128);
Hreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb2D() /* SRA L */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Lreg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (tmp & 64) tmp=(tmp | 128);
Lreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb2E() /* SRA (HL) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[Lreg | (Hreg << 8)];
Cy=(tmp & 1);
tmp=tmp >> 1;
if (tmp & 64) tmp=(tmp | 128);
mem[Lreg | (Hreg << 8)]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb2F() /* SRA A */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Areg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (tmp & 64) tmp=(tmp | 128);
Areg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb30() /* SLS B */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Breg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | 1;
Breg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb31() /* SLS C */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Creg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | 1;
Creg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb32() /* SLS D */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Dreg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | 1;
Dreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb33() /* SLS E */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Ereg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | 1;
Ereg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb34() /* SLS H */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Hreg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | 1;
Hreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb35() /* SLS L */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Lreg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | 1;
Lreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb36() /* SLS (HL) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[Lreg | (Hreg << 8)];
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | 1;
mem[Lreg | (Hreg << 8)]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb37() /* SLS A */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Areg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | 1;
Areg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb38() /* SRL B */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Breg;
Cy=(tmp & 1);
tmp=tmp >> 1;
Breg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb39() /* SRL C */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Creg;
Cy=(tmp & 1);
tmp=tmp >> 1;
Creg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb3A() /* SRL D */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Dreg;
Cy=(tmp & 1);
tmp=tmp >> 1;
Dreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb3B() /* SRL E */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Ereg;
Cy=(tmp & 1);
tmp=tmp >> 1;
Ereg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb3C() /* SRL H */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Hreg;
Cy=(tmp & 1);
tmp=tmp >> 1;
Hreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb3D() /* SRL L */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Lreg;
Cy=(tmp & 1);
tmp=tmp >> 1;
Lreg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb3E() /* SRL (HL) */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=mem[Lreg | (Hreg << 8)];
Cy=(tmp & 1);
tmp=tmp >> 1;
mem[Lreg | (Hreg << 8)]=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb3F() /* SRL A */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Areg;
Cy=(tmp & 1);
tmp=tmp >> 1;
Areg=tmp;
Freg=(tmp & 128) | (parity[tmp=(tmp & 255)]) | Cy;
if (tmp==0) Freg=(Freg | 64);
}

void cb40() /* BIT 0,B */
{
Freg=((Freg & 0xAD) | 16);
if ((Breg & 0x1)==0) Freg=Freg | 64;
}

void cb41() /* BIT 0,C */
{
Freg=((Freg & 0xAD) | 16);
if ((Creg & 0x1)==0) Freg=Freg | 64;
}

void cb42() /* BIT 0,D */
{
Freg=((Freg & 0xAD) | 16);
if ((Dreg & 0x1)==0) Freg=Freg | 64;
}

void cb43() /* BIT 0,E */
{
Freg=((Freg & 0xAD) | 16);
if ((Ereg & 0x1)==0) Freg=Freg | 64;
}

void cb44() /* BIT 0,H */
{
Freg=((Freg & 0xAD) | 16);
if ((Hreg & 0x1)==0) Freg=Freg | 64;
}

void cb45() /* BIT 0,L */
{
Freg=((Freg & 0xAD) | 16);
if ((Lreg & 0x1)==0) Freg=Freg | 64;
}

void cb46() /* BIT 0,(HL) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[Lreg | (Hreg << 8)] & 0x1)==0) Freg=Freg | 64;
}

void cb47() /* BIT 0,A */
{
Freg=((Freg & 0xAD) | 16);
if ((Areg & 0x1)==0) Freg=Freg | 64;
}

void cb48() /* BIT 1,B */
{
Freg=((Freg & 0xAD) | 16);
if ((Breg & 0x2)==0) Freg=Freg | 64;
}

void cb49() /* BIT 1,C */
{
Freg=((Freg & 0xAD) | 16);
if ((Creg & 0x2)==0) Freg=Freg | 64;
}

void cb4A() /* BIT 1,D */
{
Freg=((Freg & 0xAD) | 16);
if ((Dreg & 0x2)==0) Freg=Freg | 64;
}

void cb4B() /* BIT 1,E */
{
Freg=((Freg & 0xAD) | 16);
if ((Ereg & 0x2)==0) Freg=Freg | 64;
}

void cb4C() /* BIT 1,H */
{
Freg=((Freg & 0xAD) | 16);
if ((Hreg & 0x2)==0) Freg=Freg | 64;
}

void cb4D() /* BIT 1,L */
{
Freg=((Freg & 0xAD) | 16);
if ((Lreg & 0x2)==0) Freg=Freg | 64;
}

void cb4E() /* BIT 1,(HL) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[Lreg | (Hreg << 8)] & 0x2)==0) Freg=Freg | 64;
}

void cb4F() /* BIT 1,A */
{
Freg=((Freg & 0xAD) | 16);
if ((Areg & 0x2)==0) Freg=Freg | 64;
}

void cb50() /* BIT 2,B */
{
Freg=((Freg & 0xAD) | 16);
if ((Breg & 0x4)==0) Freg=Freg | 64;
}

void cb51() /* BIT 2,C */
{
Freg=((Freg & 0xAD) | 16);
if ((Creg & 0x4)==0) Freg=Freg | 64;
}

void cb52() /* BIT 2,D */
{
Freg=((Freg & 0xAD) | 16);
if ((Dreg & 0x4)==0) Freg=Freg | 64;
}

void cb53() /* BIT 2,E */
{
Freg=((Freg & 0xAD) | 16);
if ((Ereg & 0x4)==0) Freg=Freg | 64;
}

void cb54() /* BIT 2,H */
{
Freg=((Freg & 0xAD) | 16);
if ((Hreg & 0x4)==0) Freg=Freg | 64;
}

void cb55() /* BIT 2,L */
{
Freg=((Freg & 0xAD) | 16);
if ((Lreg & 0x4)==0) Freg=Freg | 64;
}

void cb56() /* BIT 2,(HL) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[Lreg | (Hreg << 8)] & 0x4)==0) Freg=Freg | 64;
}

void cb57() /* BIT 2,A */
{
Freg=((Freg & 0xAD) | 16);
if ((Areg & 0x4)==0) Freg=Freg | 64;
}

void cb58() /* BIT 3,B */
{
Freg=((Freg & 0xAD) | 16);
if ((Breg & 0x8)==0) Freg=Freg | 64;
}

void cb59() /* BIT 3,C */
{
Freg=((Freg & 0xAD) | 16);
if ((Creg & 0x8)==0) Freg=Freg | 64;
}

void cb5A() /* BIT 3,D */
{
Freg=((Freg & 0xAD) | 16);
if ((Dreg & 0x8)==0) Freg=Freg | 64;
}

void cb5B() /* BIT 3,E */
{
Freg=((Freg & 0xAD) | 16);
if ((Ereg & 0x8)==0) Freg=Freg | 64;
}

void cb5C() /* BIT 3,H */
{
Freg=((Freg & 0xAD) | 16);
if ((Hreg & 0x8)==0) Freg=Freg | 64;
}

void cb5D() /* BIT 3,L */
{
Freg=((Freg & 0xAD) | 16);
if ((Lreg & 0x8)==0) Freg=Freg | 64;
}

void cb5E() /* BIT 3,(HL) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[Lreg | (Hreg << 8)] & 0x8)==0) Freg=Freg | 64;
}

void cb5F() /* BIT 3,A */
{
Freg=((Freg & 0xAD) | 16);
if ((Areg & 0x8)==0) Freg=Freg | 64;
}

void cb60() /* BIT 4,B */
{
Freg=((Freg & 0xAD) | 16);
if ((Breg & 0x10)==0) Freg=Freg | 64;
}

void cb61() /* BIT 4,C */
{
Freg=((Freg & 0xAD) | 16);
if ((Creg & 0x10)==0) Freg=Freg | 64;
}

void cb62() /* BIT 4,D */
{
Freg=((Freg & 0xAD) | 16);
if ((Dreg & 0x10)==0) Freg=Freg | 64;
}

void cb63() /* BIT 4,E */
{
Freg=((Freg & 0xAD) | 16);
if ((Ereg & 0x10)==0) Freg=Freg | 64;
}

void cb64() /* BIT 4,H */
{
Freg=((Freg & 0xAD) | 16);
if ((Hreg & 0x10)==0) Freg=Freg | 64;
}

void cb65() /* BIT 4,L */
{
Freg=((Freg & 0xAD) | 16);
if ((Lreg & 0x10)==0) Freg=Freg | 64;
}

void cb66() /* BIT 4,(HL) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[Lreg | (Hreg << 8)] & 0x10)==0) Freg=Freg | 64;
}

void cb67() /* BIT 4,A */
{
Freg=((Freg & 0xAD) | 16);
if ((Areg & 0x10)==0) Freg=Freg | 64;
}

void cb68() /* BIT 5,B */
{
Freg=((Freg & 0xAD) | 16);
if ((Breg & 0x20)==0) Freg=Freg | 64;
}

void cb69() /* BIT 5,C */
{
Freg=((Freg & 0xAD) | 16);
if ((Creg & 0x20)==0) Freg=Freg | 64;
}

void cb6A() /* BIT 5,D */
{
Freg=((Freg & 0xAD) | 16);
if ((Dreg & 0x20)==0) Freg=Freg | 64;
}

void cb6B() /* BIT 5,E */
{
Freg=((Freg & 0xAD) | 16);
if ((Ereg & 0x20)==0) Freg=Freg | 64;
}

void cb6C() /* BIT 5,H */
{
Freg=((Freg & 0xAD) | 16);
if ((Hreg & 0x20)==0) Freg=Freg | 64;
}

void cb6D() /* BIT 5,L */
{
Freg=((Freg & 0xAD) | 16);
if ((Lreg & 0x20)==0) Freg=Freg | 64;
}

void cb6E() /* BIT 5,(HL) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[Lreg | (Hreg << 8)] & 0x20)==0) Freg=Freg | 64;
}

void cb6F() /* BIT 5,A */
{
Freg=((Freg & 0xAD) | 16);
if ((Areg & 0x20)==0) Freg=Freg | 64;
}

void cb70() /* BIT 6,B */
{
Freg=((Freg & 0xAD) | 16);
if ((Breg & 0x40)==0) Freg=Freg | 64;
}

void cb71() /* BIT 6,C */
{
Freg=((Freg & 0xAD) | 16);
if ((Creg & 0x40)==0) Freg=Freg | 64;
}

void cb72() /* BIT 6,D */
{
Freg=((Freg & 0xAD) | 16);
if ((Dreg & 0x40)==0) Freg=Freg | 64;
}

void cb73() /* BIT 6,E */
{
Freg=((Freg & 0xAD) | 16);
if ((Ereg & 0x40)==0) Freg=Freg | 64;
}

void cb74() /* BIT 6,H */
{
Freg=((Freg & 0xAD) | 16);
if ((Hreg & 0x40)==0) Freg=Freg | 64;
}

void cb75() /* BIT 6,L */
{
Freg=((Freg & 0xAD) | 16);
if ((Lreg & 0x40)==0) Freg=Freg | 64;
}

void cb76() /* BIT 6,(HL) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[Lreg | (Hreg << 8)] & 0x40)==0) Freg=Freg | 64;
}

void cb77() /* BIT 6,A */
{
Freg=((Freg & 0xAD) | 16);
if ((Areg & 0x40)==0) Freg=Freg | 64;
}

void cb78() /* BIT 7,B */
{
Freg=((Freg & 0xAD) | 16);
if ((Breg & 0x80)==0) Freg=Freg | 64;
}

void cb79() /* BIT 7,C */
{
Freg=((Freg & 0xAD) | 16);
if ((Creg & 0x80)==0) Freg=Freg | 64;
}

void cb7A() /* BIT 7,D */
{
Freg=((Freg & 0xAD) | 16);
if ((Dreg & 0x80)==0) Freg=Freg | 64;
}

void cb7B() /* BIT 7,E */
{
Freg=((Freg & 0xAD) | 16);
if ((Ereg & 0x80)==0) Freg=Freg | 64;
}

void cb7C() /* BIT 7,H */
{
Freg=((Freg & 0xAD) | 16);
if ((Hreg & 0x80)==0) Freg=Freg | 64;
}

void cb7D() /* BIT 7,L */
{
Freg=((Freg & 0xAD) | 16);
if ((Lreg & 0x80)==0) Freg=Freg | 64;
}

void cb7E() /* BIT 7,(HL) */
{
Freg=((Freg & 0xAD) | 16);
if ((mem[Lreg | (Hreg << 8)] & 0x80)==0) Freg=Freg | 64;
}

void cb7F() /* BIT 7,A */
{
Freg=((Freg & 0xAD) | 16);
if ((Areg & 0x80)==0) Freg=Freg | 64;
}

void cb80() /* RES 0,B */
{
Breg=(Breg & 0xFE);
}

void cb81() /* RES 0,C */
{
Creg=(Creg & 0xFE);
}

void cb82() /* RES 0,D */
{
Dreg=(Dreg & 0xFE);
}

void cb83() /* RES 0,E */
{
Ereg=(Ereg & 0xFE);
}

void cb84() /* RES 0,H */
{
Hreg=(Hreg & 0xFE);
}

void cb85() /* RES 0,L */
{
Lreg=(Lreg & 0xFE);
}

void cb86() /* RES 0,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] & 0xFE);
}

void cb87() /* RES 0,A */
{
Areg=(Areg & 0xFE);
}

void cb88() /* RES 1,B */
{
Breg=(Breg & 0xFD);
}

void cb89() /* RES 1,C */
{
Creg=(Creg & 0xFD);
}

void cb8A() /* RES 1,D */
{
Dreg=(Dreg & 0xFD);
}

void cb8B() /* RES 1,E */
{
Ereg=(Ereg & 0xFD);
}

void cb8C() /* RES 1,H */
{
Hreg=(Hreg & 0xFD);
}

void cb8D() /* RES 1,L */
{
Lreg=(Lreg & 0xFD);
}

void cb8E() /* RES 1,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] & 0xFD);
}

void cb8F() /* RES 1,A */
{
Areg=(Areg & 0xFD);
}

void cb90() /* RES 2,B */
{
Breg=(Breg & 0xFB);
}

void cb91() /* RES 2,C */
{
Creg=(Creg & 0xFB);
}

void cb92() /* RES 2,D */
{
Dreg=(Dreg & 0xFB);
}

void cb93() /* RES 2,E */
{
Ereg=(Ereg & 0xFB);
}

void cb94() /* RES 2,H */
{
Hreg=(Hreg & 0xFB);
}

void cb95() /* RES 2,L */
{
Lreg=(Lreg & 0xFB);
}

void cb96() /* RES 2,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] & 0xFB);
}

void cb97() /* RES 2,A */
{
Areg=(Areg & 0xFB);
}

void cb98() /* RES 3,B */
{
Breg=(Breg & 0xF7);
}

void cb99() /* RES 3,C */
{
Creg=(Creg & 0xF7);
}

void cb9A() /* RES 3,D */
{
Dreg=(Dreg & 0xF7);
}

void cb9B() /* RES 3,E */
{
Ereg=(Ereg & 0xF7);
}

void cb9C() /* RES 3,H */
{
Hreg=(Hreg & 0xF7);
}

void cb9D() /* RES 3,L */
{
Lreg=(Lreg & 0xF7);
}

void cb9E() /* RES 3,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] & 0xF7);
}

void cb9F() /* RES 3,A */
{
Areg=(Areg & 0xF7);
}

void cbA0() /* RES 4,B */
{
Breg=(Breg & 0xEF);
}

void cbA1() /* RES 4,C */
{
Creg=(Creg & 0xEF);
}

void cbA2() /* RES 4,D */
{
Dreg=(Dreg & 0xEF);
}

void cbA3() /* RES 4,E */
{
Ereg=(Ereg & 0xEF);
}

void cbA4() /* RES 4,H */
{
Hreg=(Hreg & 0xEF);
}

void cbA5() /* RES 4,L */
{
Lreg=(Lreg & 0xEF);
}

void cbA6() /* RES 4,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] & 0xEF);
}

void cbA7() /* RES 4,A */
{
Areg=(Areg & 0xEF);
}

void cbA8() /* RES 5,B */
{
Breg=(Breg & 0xDF);
}

void cbA9() /* RES 5,C */
{
Creg=(Creg & 0xDF);
}

void cbAA() /* RES 5,D */
{
Dreg=(Dreg & 0xDF);
}

void cbAB() /* RES 5,E */
{
Ereg=(Ereg & 0xDF);
}

void cbAC() /* RES 5,H */
{
Hreg=(Hreg & 0xDF);
}

void cbAD() /* RES 5,L */
{
Lreg=(Lreg & 0xDF);
}

void cbAE() /* RES 5,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] & 0xDF);
}

void cbAF() /* RES 5,A */
{
Areg=(Areg & 0xDF);
}

void cbB0() /* RES 6,B */
{
Breg=(Breg & 0xBF);
}

void cbB1() /* RES 6,C */
{
Creg=(Creg & 0xBF);
}

void cbB2() /* RES 6,D */
{
Dreg=(Dreg & 0xBF);
}

void cbB3() /* RES 6,E */
{
Ereg=(Ereg & 0xBF);
}

void cbB4() /* RES 6,H */
{
Hreg=(Hreg & 0xBF);
}

void cbB5() /* RES 6,L */
{
Lreg=(Lreg & 0xBF);
}

void cbB6() /* RES 6,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] & 0xBF);
}

void cbB7() /* RES 6,A */
{
Areg=(Areg & 0xBF);
}

void cbB8() /* RES 7,B */
{
Breg=(Breg & 0x7F);
}

void cbB9() /* RES 7,C */
{
Creg=(Creg & 0x7F);
}

void cbBA() /* RES 7,D */
{
Dreg=(Dreg & 0x7F);
}

void cbBB() /* RES 7,E */
{
Ereg=(Ereg & 0x7F);
}

void cbBC() /* RES 7,H */
{
Hreg=(Hreg & 0x7F);
}

void cbBD() /* RES 7,L */
{
Lreg=(Lreg & 0x7F);
}

void cbBE() /* RES 7,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] & 0x7F);
}

void cbBF() /* RES 7,A */
{
Areg=(Areg & 0x7F);
}

void cbC0() /* SET 0,B */
{
Breg=(Breg | 0x1);
}

void cbC1() /* SET 0,C */
{
Creg=(Creg | 0x1);
}

void cbC2() /* SET 0,D */
{
Dreg=(Dreg | 0x1);
}

void cbC3() /* SET 0,E */
{
Ereg=(Ereg | 0x1);
}

void cbC4() /* SET 0,H */
{
Hreg=(Hreg | 0x1);
}

void cbC5() /* SET 0,L */
{
Lreg=(Lreg | 0x1);
}

void cbC6() /* SET 0,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] | 0x1);
}

void cbC7() /* SET 0,A */
{
Areg=(Areg | 0x1);
}

void cbC8() /* SET 1,B */
{
Breg=(Breg | 0x2);
}

void cbC9() /* SET 1,C */
{
Creg=(Creg | 0x2);
}

void cbCA() /* SET 1,D */
{
Dreg=(Dreg | 0x2);
}

void cbCB() /* SET 1,E */
{
Ereg=(Ereg | 0x2);
}

void cbCC() /* SET 1,H */
{
Hreg=(Hreg | 0x2);
}

void cbCD() /* SET 1,L */
{
Lreg=(Lreg | 0x2);
}

void cbCE() /* SET 1,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] | 0x2);
}

void cbCF() /* SET 1,A */
{
Areg=(Areg | 0x2);
}

void cbD0() /* SET 2,B */
{
Breg=(Breg | 0x4);
}

void cbD1() /* SET 2,C */
{
Creg=(Creg | 0x4);
}

void cbD2() /* SET 2,D */
{
Dreg=(Dreg | 0x4);
}

void cbD3() /* SET 2,E */
{
Ereg=(Ereg | 0x4);
}

void cbD4() /* SET 2,H */
{
Hreg=(Hreg | 0x4);
}

void cbD5() /* SET 2,L */
{
Lreg=(Lreg | 0x4);
}

void cbD6() /* SET 2,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] | 0x4);
}

void cbD7() /* SET 2,A */
{
Areg=(Areg | 0x4);
}

void cbD8() /* SET 3,B */
{
Breg=(Breg | 0x8);
}

void cbD9() /* SET 3,C */
{
Creg=(Creg | 0x8);
}

void cbDA() /* SET 3,D */
{
Dreg=(Dreg | 0x8);
}

void cbDB() /* SET 3,E */
{
Ereg=(Ereg | 0x8);
}

void cbDC() /* SET 3,H */
{
Hreg=(Hreg | 0x8);
}

void cbDD() /* SET 3,L */
{
Lreg=(Lreg | 0x8);
}

void cbDE() /* SET 3,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] | 0x8);
}

void cbDF() /* SET 3,A */
{
Areg=(Areg | 0x8);
}

void cbE0() /* SET 4,B */
{
Breg=(Breg | 0x10);
}

void cbE1() /* SET 4,C */
{
Creg=(Creg | 0x10);
}

void cbE2() /* SET 4,D */
{
Dreg=(Dreg | 0x10);
}

void cbE3() /* SET 4,E */
{
Ereg=(Ereg | 0x10);
}

void cbE4() /* SET 4,H */
{
Hreg=(Hreg | 0x10);
}

void cbE5() /* SET 4,L */
{
Lreg=(Lreg | 0x10);
}

void cbE6() /* SET 4,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] | 0x10);
}

void cbE7() /* SET 4,A */
{
Areg=(Areg | 0x10);
}

void cbE8() /* SET 5,B */
{
Breg=(Breg | 0x20);
}

void cbE9() /* SET 5,C */
{
Creg=(Creg | 0x20);
}

void cbEA() /* SET 5,D */
{
Dreg=(Dreg | 0x20);
}

void cbEB() /* SET 5,E */
{
Ereg=(Ereg | 0x20);
}

void cbEC() /* SET 5,H */
{
Hreg=(Hreg | 0x20);
}

void cbED() /* SET 5,L */
{
Lreg=(Lreg | 0x20);
}

void cbEE() /* SET 5,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] | 0x20);
}

void cbEF() /* SET 5,A */
{
Areg=(Areg | 0x20);
}

void cbF0() /* SET 6,B */
{
Breg=(Breg | 0x40);
}

void cbF1() /* SET 6,C */
{
Creg=(Creg | 0x40);
}

void cbF2() /* SET 6,D */
{
Dreg=(Dreg | 0x40);
}

void cbF3() /* SET 6,E */
{
Ereg=(Ereg | 0x40);
}

void cbF4() /* SET 6,H */
{
Hreg=(Hreg | 0x40);
}

void cbF5() /* SET 6,L */
{
Lreg=(Lreg | 0x40);
}

void cbF6() /* SET 6,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] | 0x40);
}

void cbF7() /* SET 6,A */
{
Areg=(Areg | 0x40);
}

void cbF8() /* SET 7,B */
{
Breg=(Breg | 0x80);
}

void cbF9() /* SET 7,C */
{
Creg=(Creg | 0x80);
}

void cbFA() /* SET 7,D */
{
Dreg=(Dreg | 0x80);
}

void cbFB() /* SET 7,E */
{
Ereg=(Ereg | 0x80);
}

void cbFC() /* SET 7,H */
{
Hreg=(Hreg | 0x80);
}

void cbFD() /* SET 7,L */
{
Lreg=(Lreg | 0x80);
}

void cbFE() /* SET 7,(HL) */
{
mem[Lreg | (Hreg << 8)]=(mem[Lreg | (Hreg << 8)] | 0x80);
}

void cbFF() /* SET 7,A */
{
Areg=(Areg | 0x80);
}


/* Table of addresses for CB prefix codes */
static void (*codeCB[])(void) = {cb00, cb01, cb02, cb03, cb04, cb05, 
     cb06, cb07, cb08, cb09, cb0A, cb0B, cb0C, cb0D, cb0E, cb0F, cb10, cb11, 
     cb12, cb13, cb14, cb15, cb16, cb17, cb18, cb19, cb1A, cb1B, cb1C, cb1D, 
     cb1E, cb1F, cb20, cb21, cb22, cb23, cb24, cb25, cb26, cb27, cb28, cb29, 
     cb2A, cb2B, cb2C, cb2D, cb2E, cb2F, cb30, cb31, cb32, cb33, cb34, cb35, 
     cb36, cb37, cb38, cb39, cb3A, cb3B, cb3C, cb3D, cb3E, cb3F, cb40, cb41, 
     cb42, cb43, cb44, cb45, cb46, cb47, cb48, cb49, cb4A, cb4B, cb4C, cb4D, 
     cb4E, cb4F, cb50, cb51, cb52, cb53, cb54, cb55, cb56, cb57, cb58, cb59, 
     cb5A, cb5B, cb5C, cb5D, cb5E, cb5F, cb60, cb61, cb62, cb63, cb64, cb65, 
     cb66, cb67, cb68, cb69, cb6A, cb6B, cb6C, cb6D, cb6E, cb6F, cb70, cb71, 
     cb72, cb73, cb74, cb75, cb76, cb77, cb78, cb79, cb7A, cb7B, cb7C, cb7D, 
     cb7E, cb7F, cb80, cb81, cb82, cb83, cb84, cb85, cb86, cb87, cb88, cb89, 
     cb8A, cb8B, cb8C, cb8D, cb8E, cb8F, cb90, cb91, cb92, cb93, cb94, cb95, 
     cb96, cb97, cb98, cb99, cb9A, cb9B, cb9C, cb9D, cb9E, cb9F, cbA0, cbA1, 
     cbA2, cbA3, cbA4, cbA5, cbA6, cbA7, cbA8, cbA9, cbAA, cbAB, cbAC, cbAD, 
     cbAE, cbAF, cbB0, cbB1, cbB2, cbB3, cbB4, cbB5, cbB6, cbB7, cbB8, cbB9, 
     cbBA, cbBB, cbBC, cbBD, cbBE, cbBF, cbC0, cbC1, cbC2, cbC3, cbC4, cbC5, 
     cbC6, cbC7, cbC8, cbC9, cbCA, cbCB, cbCC, cbCD, cbCE, cbCF, cbD0, cbD1, 
     cbD2, cbD3, cbD4, cbD5, cbD6, cbD7, cbD8, cbD9, cbDA, cbDB, cbDC, cbDD, 
     cbDE, cbDF, cbE0, cbE1, cbE2, cbE3, cbE4, cbE5, cbE6, cbE7, cbE8, cbE9, 
     cbEA, cbEB, cbEC, cbED, cbEE, cbEF, cbF0, cbF1, cbF2, cbF3, cbF4, cbF5, 
     cbF6, cbF7, cbF8, cbF9, cbFA, cbFB, cbFC, cbFD, cbFE, cbFF };



/* EDxx codes */

/* ED00 to ED0F call MOS_00 to MOS_0F in Interface module */

void ED_null()
{
undef(); /* Undefined ED instruction */
}

void ED_00() { MOS_QUIT(); }
void ED_01() { MOS_CLI();  }
void ED_02() { MOS_BYTE(); }
void ED_03() { MOS_WORD(); }
void ED_04() { MOS_WRCH(); }
void ED_05() { MOS_RDCH(); }
void ED_06() { MOS_FILE(); }
void ED_07() { MOS_ARGS(); }
void ED_08() { MOS_BGET(); }
void ED_09() { MOS_BPUT(); }
void ED_0A() { MOS_GBPB(); }
void ED_0B() { MOS_FIND(); }
void ED_0C() { MOS_FF0C(); }
void ED_0D() { MOS_FF0D(); }
void ED_0E() { MOS_FF0E(); }
void ED_0F() { MOS_FF0F(); }

void ED_40() /* IN   B(C) */
{
Breg=io_IN(Creg | (Breg << 8));
}

void ED_41() /* OUT  (C),B */
{
io_OUT(Creg | (Breg << 8),Breg);
}

void ED_42() /* SBC  HL,BC */
{
tmp=Lreg | (Hreg << 8);
tmp2=(Creg | (Breg << 8)) + (Freg & 1);
PV=((tmp & 0x7FFF) - (tmp2 & 0x7FFF)) < 0;
tmp=tmp-tmp2;
Cy=(tmp < 0 || tmp > 0xFFFF);
PV=PV ^ Cy;
tmp=tmp & 0xFFFF;
Freg=((tmp & 0x8000) >> 8) | ((tmp == 0) << 6) | (PV << 2) | 2 | Cy;
Lreg=tmp;
Hreg=tmp >> 8;
}

void ED_43() /* LD   (nnnn),BC */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
mem[tmp++]=Creg;
mem[tmp]=Breg;
}

void ED_44() /* NEG */
{
Areg=256-Areg;
Freg=(Areg & 128) | ((Areg == 0)<< 6) | ((Areg == 0x7f) << 2) | (Areg == 0);
}

void ED_45() /* RETN */
{
IFF1=IFF2;
tmp=mem[SP++];
tmp=tmp + (mem[SP++] << 8);
SP=SP & 0xFFFF;     /* Careful about &FFFF->&0000 */
PC=&mem[0]+tmp;
}

void ED_46() /* IM 0 */
{
IM=0;
}

void ED_47() /* LD I,A */
{
Ireg=Areg;
}

void ED_48() /* IN   C(C) */
{
Creg=io_IN(Creg | (Breg << 8));
}

void ED_49() /* OUT  (C),C */
{
io_OUT(Creg | (Breg << 8),Creg);
}

void ED_4A() /* ADC  HL,BC */
{
tmp=Lreg | (Hreg << 8);
tmp2=(Creg | (Breg << 8)) + (Freg & 1);
PV=((tmp & 0x7FFF) + (tmp2 & 0x7FFF)) > 0x7FFF;
tmp=tmp+tmp2;
Cy=(tmp < 0 || tmp > 0xFFFF);
PV=PV ^ Cy;
tmp=tmp & 0xFFFF;
Freg=((tmp & 0x8000) >> 8) | ((tmp == 0) << 6) | (PV << 2) | 2 | Cy;
Lreg=tmp;
Hreg=tmp >> 8;
}

void ED_4B() /* LD   BC,(nnnn) */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
Creg=mem[tmp++];
Breg=mem[tmp];
}

void ED_4C() /* ??? */
{ undef(); }

void ED_4D() /* RETI */
{
tmp=mem[SP++];
tmp=tmp + (mem[SP++] << 8);
SP=SP & 0xFFFF;     /* Careful about &FFFF->&0000 */
PC=&mem[0]+tmp;
}

void ED_4E() /* ???? */
{
undef();
}

void ED_4F() /* LD R,A */
{
Rreg=Areg;
}

void ED_50() /* IN   D(C) */
{
Dreg=io_IN(Creg | (Breg << 8));
}

void ED_51() /* OUT  (C),D */
{
io_OUT(Creg | (Breg << 8),Dreg);
}

void ED_52() /* SBC  HL,DE */
{
tmp=Lreg | (Hreg << 8);
tmp2=(Ereg | (Dreg << 8)) + (Freg & 1);
PV=((tmp & 0x7FFF) - (tmp2 & 0x7FFF)) < 0;
tmp=tmp-tmp2;
Cy=(tmp < 0 || tmp > 0xFFFF);
PV=PV ^ Cy;
tmp=tmp & 0xFFFF;
Freg=((tmp & 0x8000) >> 8) | ((tmp == 0) << 6) | (PV << 2) | 2 | Cy;
Lreg=tmp;
Hreg=tmp >> 8;
}

void ED_53() /* LD   (nnnn),DE */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
mem[tmp++]=Ereg;
mem[tmp]=Dreg;
}

void ED_54() /* ??? */
{ undef(); }

void ED_55() /* ???? */
{
undef();
}

void ED_56() /* IM 1 */
{
IM=1;
}

void ED_57() /* LD A,I */
{
Areg=Ireg;
Freg=(Freg & 0x3B) | (Areg & 128) | ((Areg == 0) << 6) | (IFF2 << 2);
}

void ED_58() /* IN   E(C) */
{
Ereg=io_IN(Creg | (Breg << 8));
}

void ED_59() /* OUT  (C),E */
{
io_OUT(Creg | (Breg << 8),Ereg);
}

void ED_5A() /* ADC  HL,DE */
{
tmp=Lreg | (Hreg << 8);
tmp2=(Ereg | (Dreg << 8)) + (Freg & 1);
PV=((tmp & 0x7FFF) + (tmp2 & 0x7FFF)) > 0x7FFF;
tmp=tmp+tmp2;
Cy=(tmp < 0 || tmp > 0xFFFF);
PV=PV ^ Cy;
tmp=tmp & 0xFFFF;
Freg=((tmp & 0x8000) >> 8) | ((tmp == 0) << 6) | (PV << 2) | 2 | Cy;
Lreg=tmp;
Hreg=tmp >> 8;
}

void ED_5B() /* LD   DE,(nnnn) */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
Ereg=mem[tmp++];
Dreg=mem[tmp];
}

void ED_5C() /* ??? */
{ undef(); }

void ED_5D() /* ???? */
{
undef();
}

void ED_5E() /* IM 2 */
{
IM=2;
}

void ED_5F() /* LD A,R */
{
Areg=Rreg++;
Freg=(Freg & 0x3B) | (Areg & 128) | ((Areg == 0) << 6) | (IFF2 << 2);
}

void ED_60() /* IN   H(C) */
{
Hreg=io_IN(Creg | (Breg << 8));
}

void ED_61() /* OUT  (C),H */
{
io_OUT(Creg | (Breg << 8),Hreg);
}

void ED_62() /* SBC  HL,HL */
{
tmp=Lreg | (Hreg << 8);
tmp2=(Lreg | (Hreg << 8)) + (Freg & 1);
PV=((tmp & 0x7FFF) - (tmp2 & 0x7FFF)) < 0;
tmp=tmp-tmp2;
Cy=(tmp < 0 || tmp > 0xFFFF);
PV=PV ^ Cy;
tmp=tmp & 0xFFFF;
Freg=((tmp & 0x8000) >> 8) | ((tmp == 0) << 6) | (PV << 2) | 2 | Cy;
Lreg=tmp;
Hreg=tmp >> 8;
}

void ED_63() /* LD   (nnnn),HL */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
mem[tmp++]=Lreg;
mem[tmp]=Hreg;
}

void ED_64() /* ??? */
{ undef(); }

void ED_65() /* ???? */
{
undef();
}

void ED_66() /* ???? */
{
undef();
}

void ED_67() /* RRD    */
{
;
tmp=Lreg | (Hreg << 8);
tmp2=Areg;
Areg=(Areg & 0xF0) | (mem[tmp] &0x0F);
mem[tmp]=(mem[tmp] >> 4) | ((tmp2 & 0x0F) << 4);
Freg=(Freg & 0x29) | (Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void ED_68() /* IN   L(C) */
{
Lreg=io_IN(Creg | (Breg << 8));
}

void ED_69() /* OUT  (C),L */
{
io_OUT(Creg | (Breg << 8),Lreg);
}

void ED_6A() /* ADC  HL,HL */
{
tmp=Lreg | (Hreg << 8);
tmp2=(Lreg | (Hreg << 8)) + (Freg & 1);
PV=((tmp & 0x7FFF) + (tmp2 & 0x7FFF)) > 0x7FFF;
tmp=tmp+tmp2;
Cy=(tmp < 0 || tmp > 0xFFFF);
PV=PV ^ Cy;
tmp=tmp & 0xFFFF;
Freg=((tmp & 0x8000) >> 8) | ((tmp == 0) << 6) | (PV << 2) | 2 | Cy;
Lreg=tmp;
Hreg=tmp >> 8;
}

void ED_6B() /* LD   HL,(nnnn) */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
Lreg=mem[tmp++];
Hreg=mem[tmp];
}

void ED_6C() /* ??? */
{ undef(); }

void ED_6D() /* ???? */
{
undef();
}

void ED_6E() /* ???? */
{
undef();
}

void ED_6F() /* RLD    */
{
;
tmp=Lreg | (Hreg << 8);
tmp2=Areg;
Areg=(Areg & 0xF0) | (mem[tmp] >> 4);
mem[tmp]=((mem[tmp] & 0x0F) << 4) | (tmp2 & 0x0F);
Freg=(Freg & 0x29) | (Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void ED_70() /* IN   (HL)(C) */
{
mem[Lreg | (Hreg << 8)]=io_IN(Creg | (Breg << 8));
}

void ED_71() /* OUT  (C),(HL) */
{
io_OUT(Creg | (Breg << 8),mem[Lreg | (Hreg << 8)]);
}

void ED_72() /* SBC  HL,SP */
{
tmp=Lreg | (Hreg << 8);
tmp2=SP;
PV=((tmp & 0x7FFF) - (tmp2 & 0x7FFF)) < 0;
tmp=tmp-tmp2;
Cy=(tmp < 0 || tmp > 0xFFFF);
PV=PV ^ Cy;
tmp=tmp & 0xFFFF;
Freg=((tmp & 0x8000) >> 8) | ((tmp == 0) << 6) | (PV << 2) | 2 | Cy;
Lreg=tmp;
Hreg=tmp >> 8;
}

void ED_73() /* LD   (nnnn),SP */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
mem[tmp++]=SP & 255;
mem[tmp]=SP >> 8;
}

void ED_74() /* ??? */
{ undef(); }

void ED_75() /* ???? */
{
undef();
}

void ED_76() /* ???? */
{
undef();
}

void ED_77() /* ????   */
{ undef();
;
}

void ED_78() /* IN   A(C) */
{
Areg=io_IN(Creg | (Breg << 8));
}

void ED_79() /* OUT  (C),A */
{
io_OUT(Creg | (Breg << 8),Areg);
}

void ED_7A() /* ADC  HL,SP */
{
tmp=Lreg | (Hreg << 8);
tmp2=SP;
PV=((tmp & 0x7FFF) + (tmp2 & 0x7FFF)) > 0x7FFF;
tmp=tmp+tmp2;
Cy=(tmp < 0 || tmp > 0xFFFF);
PV=PV ^ Cy;
tmp=tmp & 0xFFFF;
Freg=((tmp & 0x8000) >> 8) | ((tmp == 0) << 6) | (PV << 2) | 2 | Cy;
Lreg=tmp;
Hreg=tmp >> 8;
}

void ED_7B() /* LD   SP,(nnnn) */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
SP=mem[tmp++];
SP=SP | (mem[tmp] << 8);
}

void ED_7C() /* ??? */
{ undef(); }

void ED_7D() /* ???? */
{
undef();
}

void ED_7E() /* ???? */
{
undef();
}

void ED_7F() /* ???? */
{ undef();
;
}

void ED_A0() /* LDI b=0 c=0 */
{
Freg=Freg & 0xBB;
if ((--Creg)==0xFF) Breg--;
if ((Breg | Creg) != 0) Freg=Freg | 4; /* V set if BC <> 0 */
tmp=Lreg | (Hreg << 8);
tmp2=Ereg | (Dreg << 8);
mem[tmp2]=mem[tmp];
tmp2++;
tmp++;
Lreg=tmp; Hreg=tmp >> 8;
Ereg=tmp2; Dreg=tmp2 >> 8;
}

void ED_A1() /* CPI b=1 c=0 */
{
Freg=Freg & 0xBB;
if ((--Creg)==0xFF) Breg--;
if ((Breg | Creg) != 0) Freg=Freg | 4; /* V set if BC <> 0 */
tmp=Lreg | (Hreg << 8);
if (Areg == mem[tmp]) Freg=Freg | 64;
tmp++;
Lreg=tmp; Hreg=tmp >> 8;
}

void ED_A2() /* INI b=2 c=0 */
{
Freg=Freg & 0xBB;
if ((--Breg) == 0) Freg=Freg | 64;     /* Z set if B = 0 */
Freg=Freg | 2; /* INI/D and OUTI/D set N */
tmp=Lreg | (Hreg << 8);
Areg=io_IN(Creg | (Breg << 8));
tmp++;
Lreg=tmp; Hreg=tmp >> 8;
}

void ED_A3() /* OUTI b=3 c=0 */
{
Freg=Freg & 0xBB;
if ((--Breg) == 0) Freg=Freg | 64;     /* Z set if B = 0 */
Freg=Freg | 2; /* INI/D and OUTI/D set N */
tmp=Lreg | (Hreg << 8);
io_OUT(Creg | (Breg << 8), Areg);
tmp++;
Lreg=tmp; Hreg=tmp >> 8;
}

void ED_A4() /* ???? */
{ undef(); }

void ED_A5() /* ???? */
{ undef(); }

void ED_A6() /* ???? */
{ undef(); }

void ED_A7() /* ???? */
{ undef(); }

void ED_A8() /* LDD b=0 c=1 */
{
Freg=Freg & 0xBB;
if ((--Creg)==0xFF) Breg--;
if ((Breg | Creg) != 0) Freg=Freg | 4; /* V set if BC <> 0 */
tmp=Lreg | (Hreg << 8);
tmp2=Ereg | (Dreg << 8);
mem[tmp2]=mem[tmp];
tmp2--;
tmp--;
Lreg=tmp; Hreg=tmp >> 8;
Ereg=tmp2; Dreg=tmp2 >> 8;
}

void ED_A9() /* CPD b=1 c=1 */
{
Freg=Freg & 0xBB;
if ((--Creg)==0xFF) Breg--;
if ((Breg | Creg) != 0) Freg=Freg | 4; /* V set if BC <> 0 */
tmp=Lreg | (Hreg << 8);
if (Areg == mem[tmp]) Freg=Freg | 64;
tmp--;
Lreg=tmp; Hreg=tmp >> 8;
}

void ED_AA() /* IND b=2 c=1 */
{
Freg=Freg & 0xBB;
if ((--Breg) == 0) Freg=Freg | 64;     /* Z set if B = 0 */
Freg=Freg | 2; /* INI/D and OUTI/D set N */
tmp=Lreg | (Hreg << 8);
Areg=io_IN(Creg | (Breg << 8));
tmp--;
Lreg=tmp; Hreg=tmp >> 8;
}

void ED_AB() /* OUTD b=3 c=1 */
{
Freg=Freg & 0xBB;
if ((--Breg) == 0) Freg=Freg | 64;     /* Z set if B = 0 */
Freg=Freg | 2; /* INI/D and OUTI/D set N */
tmp=Lreg | (Hreg << 8);
io_OUT(Creg | (Breg << 8), Areg);
tmp--;
Lreg=tmp; Hreg=tmp >> 8;
}

void ED_AC() /* ???? */
{ undef(); }

void ED_AD() /* ???? */
{ undef(); }

void ED_AE() /* ???? */
{ undef(); }

void ED_AF() /* ???? */
{ undef(); }

void ED_B0() /* LDIR b=0 c=2 */
{
Freg=Freg & 0xBB;
if ((--Creg)==0xFF) Breg--;
if ((Breg | Creg) != 0) Freg=Freg | 4; /* V set if BC <> 0 */
tmp=Lreg | (Hreg << 8);
tmp2=Ereg | (Dreg << 8);
mem[tmp2]=mem[tmp];
tmp2++;
tmp++;
Lreg=tmp; Hreg=tmp >> 8;
Ereg=tmp2; Dreg=tmp2 >> 8;
if ((Breg | Creg) && !(Freg & 64)) PC=PC-2; /* Repeat */
}

void ED_B1() /* CPIR b=1 c=2 */
{
Freg=Freg & 0xBB;
if ((--Creg)==0xFF) Breg--;
if ((Breg | Creg) != 0) Freg=Freg | 4; /* V set if BC <> 0 */
tmp=Lreg | (Hreg << 8);
if (Areg == mem[tmp]) Freg=Freg | 64;
tmp++;
Lreg=tmp; Hreg=tmp >> 8;
if ((Breg | Creg) && !(Freg & 64)) PC=PC-2; /* Repeat */
}

void ED_B2() /* INIR b=2 c=2 */
{
Freg=Freg & 0xBB;
if ((--Breg) == 0) Freg=Freg | 64;     /* Z set if B = 0 */
Freg=Freg | 2; /* INI/D and OUTI/D set N */
tmp=Lreg | (Hreg << 8);
Areg=io_IN(Creg | (Breg << 8));
tmp++;
Lreg=tmp; Hreg=tmp >> 8;
if ((Breg | Creg) && !(Freg & 64)) PC=PC-2; /* Repeat */
}

void ED_B3() /* OUTIR b=3 c=2 */
{
Freg=Freg & 0xBB;
if ((--Breg) == 0) Freg=Freg | 64;     /* Z set if B = 0 */
Freg=Freg | 2; /* INI/D and OUTI/D set N */
tmp=Lreg | (Hreg << 8);
io_OUT(Creg | (Breg << 8), Areg);
tmp++;
Lreg=tmp; Hreg=tmp >> 8;
if ((Breg | Creg) && !(Freg & 64)) PC=PC-2; /* Repeat */
}

void ED_B4() /* ???? */
{ undef(); }

void ED_B5() /* ???? */
{ undef(); }

void ED_B6() /* ???? */
{ undef(); }

void ED_B7() /* ???? */
{ undef(); }

void ED_B8() /* LDDR b=0 c=3 */
{
Freg=Freg & 0xBB;
if ((--Creg)==0xFF) Breg--;
if ((Breg | Creg) != 0) Freg=Freg | 4; /* V set if BC <> 0 */
tmp=Lreg | (Hreg << 8);
tmp2=Ereg | (Dreg << 8);
mem[tmp2]=mem[tmp];
tmp2--;
tmp--;
Lreg=tmp; Hreg=tmp >> 8;
Ereg=tmp2; Dreg=tmp2 >> 8;
if ((Breg | Creg) && !(Freg & 64)) PC=PC-2; /* Repeat */
}

void ED_B9() /* CPDR b=1 c=3 */
{
Freg=Freg & 0xBB;
if ((--Creg)==0xFF) Breg--;
if ((Breg | Creg) != 0) Freg=Freg | 4; /* V set if BC <> 0 */
tmp=Lreg | (Hreg << 8);
if (Areg == mem[tmp]) Freg=Freg | 64;
tmp--;
Lreg=tmp; Hreg=tmp >> 8;
if ((Breg | Creg) && !(Freg & 64)) PC=PC-2; /* Repeat */
}

void ED_BA() /* INDR b=2 c=3 */
{
Freg=Freg & 0xBB;
if ((--Breg) == 0) Freg=Freg | 64;     /* Z set if B = 0 */
Freg=Freg | 2; /* INI/D and OUTI/D set N */
tmp=Lreg | (Hreg << 8);
Areg=io_IN(Creg | (Breg << 8));
tmp--;
Lreg=tmp; Hreg=tmp >> 8;
if ((Breg | Creg) && !(Freg & 64)) PC=PC-2; /* Repeat */
}

void ED_BB() /* OUTDR b=3 c=3 */
{
Freg=Freg & 0xBB;
if ((--Breg) == 0) Freg=Freg | 64;     /* Z set if B = 0 */
Freg=Freg | 2; /* INI/D and OUTI/D set N */
tmp=Lreg | (Hreg << 8);
io_OUT(Creg | (Breg << 8), Areg);
tmp--;
Lreg=tmp; Hreg=tmp >> 8;
if ((Breg | Creg) && !(Freg & 64)) PC=PC-2; /* Repeat */
}

void ED_BC() /* ???? */
{ undef(); }

void ED_BD() /* ???? */
{ undef(); }

void ED_BE() /* ???? */
{ undef(); }

void ED_BF() /* ???? */
{ undef(); }


void ED_F0() { MOS_FF0F(); }
void ED_F1() { MOS_FF0E(); }
void ED_F2() { MOS_FF0D(); }
void ED_F3() { MOS_FF0C(); }
void ED_F4() { MOS_FIND(); }
void ED_F5() { MOS_GBPB(); }
void ED_F6() { MOS_BPUT(); }
void ED_F7() { MOS_BGET(); }
void ED_F8() { MOS_ARGS(); }
void ED_F9() { MOS_FILE(); }
void ED_FA() { MOS_RDCH(); }
void ED_FB() { MOS_WRCH(); }
void ED_FC() { MOS_WORD(); }
void ED_FD() { MOS_BYTE(); }
void ED_FE() { MOS_CLI();  }
void ED_FF() { MOS_QUIT(); }

/* Table of addresses for ED prefix codes */
static void (*codeED[])(void) = {ED_00, ED_01, ED_02, ED_03,
    ED_04, ED_05, ED_06, ED_07, ED_08, ED_09, ED_0A, ED_0B,
    ED_0C, ED_0D, ED_0E, ED_0F,
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_40, ED_41, ED_42, ED_43, ED_44, ED_45, ED_46, ED_47, 
    ED_48, ED_49, ED_4A, ED_4B, ED_4C, ED_4D, ED_4E, ED_4F, 
    ED_50, ED_51, ED_52, ED_53, ED_54, ED_55, ED_56, ED_57, 
    ED_58, ED_59, ED_5A, ED_5B, ED_5C, ED_5D, ED_5E, ED_5F, 
    ED_60, ED_61, ED_62, ED_63, ED_64, ED_65, ED_66, ED_67, 
    ED_68, ED_69, ED_6A, ED_6B, ED_6C, ED_6D, ED_6E, ED_6F, 
    ED_70, ED_71, ED_72, ED_73, ED_74, ED_75, ED_76, ED_77, 
    ED_78, ED_79, ED_7A, ED_7B, ED_7C, ED_7D, ED_7E, ED_7F, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_A0, ED_A1, ED_A2, ED_A3, ED_null, ED_null, ED_null, ED_null, 
    ED_A8, ED_A9, ED_AA, ED_AB, ED_null, ED_null, ED_null, ED_null, 
    ED_B0, ED_B1, ED_B2, ED_B3, ED_null, ED_null, ED_null, ED_null, 
    ED_B8, ED_B9, ED_BA, ED_BB, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, ED_null, 
    ED_F0, ED_F1, ED_F2, ED_F3, ED_F4, ED_F5, ED_F6, ED_F7,
    ED_F8, ED_F9, ED_FA, ED_FB, ED_FC, ED_FD, ED_FE, ED_FF };




/* Normal code */

/* Block 0 */
void x00() /* NOP */
{
}

void x01() /* LD   BC,nnnn */
{
Creg=*(PC++);
Breg=*(PC++);
}

void x02() /* LD   (BC),A */
{
mem[Creg | (Breg << 8)]=Areg;
}

void x03() /* INC  BC */
{
if((++Creg) == 0) Breg++;
}

void x04() /* INC  B */
{
Breg++;
Freg=(Freg & 0x2B) | (Breg & 128) | ((Breg == 0) << 6) | ((Breg == 0x80) << 2) | (((Breg & 0xF) == 0) << 4);
}

void x05() /* DEC  B */
{
Breg--;
Freg=(Freg & 0x2B) | (Breg & 128) | ((Breg == 0) << 6) | ((Breg == 0x7f) << 2) | (((Breg & 0xF) == 15) << 4) | 2;
}

void x06() /* LD   B,n */
{
Breg=*(PC++);
}

void x07() /* RLCA */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Areg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | (Cy & 1);
Areg=tmp;
Freg=(Freg & 0xEC) | Cy;
}

void x08() /* EX   AF,AF' */
{
tmp=Areg; Areg=Aalt; Aalt=tmp;
tmp=Freg; Freg=Falt; Falt=tmp;
}

void x09() /* ADD  HL,BC */
{
tmp=Lreg | (Hreg << 8);
tmp2=Creg | (Breg << 8);
tmp=tmp+tmp2;
Freg=(Freg & 0xFC);  /* H should also get set */
if (tmp > 65535) Freg++;
Lreg=tmp;
Hreg=tmp >> 8;
}

void x0A() /* LD   A,(BC) */
{
Areg=mem[Creg | (Breg << 8)];
}

void x0B() /* DEC  BC */
{
if((--Creg) == 0xFF) Breg--;
}

void x0C() /* INC  C */
{
Creg++;
Freg=(Freg & 0x2B) | (Creg & 128) | ((Creg == 0) << 6) | ((Creg == 0x80) << 2) | (((Creg & 0xF) == 0) << 4);
}

void x0D() /* DEC  C */
{
Creg--;
Freg=(Freg & 0x2B) | (Creg & 128) | ((Creg == 0) << 6) | ((Creg == 0x7f) << 2) | (((Creg & 0xF) == 15) << 4) | 2;
}

void x0E() /* LD   C,n */
{
Creg=*(PC++);
}

void x0F() /* RRCA */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Areg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy) tmp=(tmp | 128);
Areg=tmp;
Freg=(Freg & 0xEC) | Cy;
}

void x10() /* DJNZ dis */
{
tmp=*(PC++);
Breg=Breg-1;
if (Breg == 0) return;
if (tmp < 128) PC=PC+tmp;
else           PC=PC+tmp-256;
}

void x11() /* LD   DE,nnnn */
{
Ereg=*(PC++);
Dreg=*(PC++);
}

void x12() /* LD   (DE),A */
{
mem[Ereg | (Dreg << 8)]=Areg;
}

void x13() /* INC  DE */
{
if((++Ereg) == 0) Dreg++;
}

void x14() /* INC  D */
{
Dreg++;
Freg=(Freg & 0x2B) | (Dreg & 128) | ((Dreg == 0) << 6) | ((Dreg == 0x80) << 2) | (((Dreg & 0xF) == 0) << 4);
}

void x15() /* DEC  D */
{
Dreg--;
Freg=(Freg & 0x2B) | (Dreg & 128) | ((Dreg == 0) << 6) | ((Dreg == 0x7f) << 2) | (((Dreg & 0xF) == 15) << 4) | 2;
}

void x16() /* LD   D,n */
{
Dreg=*(PC++);
}

void x17() /* RLA */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Areg;
tmp=tmp << 1;
Cy=(tmp > 255);
tmp=tmp | Cy2;
Areg=tmp;
Freg=(Freg & 0xEC) | Cy;
}

void x18() /* JR    */
{
tmp=*(PC++);
if (tmp < 128) PC=PC+tmp;
else           PC=PC+tmp-256;
}

void x19() /* ADD  HL,DE */
{
tmp=Lreg | (Hreg << 8);
tmp2=Ereg | (Dreg << 8);
tmp=tmp+tmp2;
Freg=(Freg & 0xFC);  /* H should also get set */
if (tmp > 65535) Freg++;
Lreg=tmp;
Hreg=tmp >> 8;
}

void x1A() /* LD   A,(DE) */
{
Areg=mem[Ereg | (Dreg << 8)];
}

void x1B() /* DEC  DE */
{
if((--Ereg) == 0xFF) Dreg--;
}

void x1C() /* INC  E */
{
Ereg++;
Freg=(Freg & 0x2B) | (Ereg & 128) | ((Ereg == 0) << 6) | ((Ereg == 0x80) << 2) | (((Ereg & 0xF) == 0) << 4);
}

void x1D() /* DEC  E */
{
Ereg--;
Freg=(Freg & 0x2B) | (Ereg & 128) | ((Ereg == 0) << 6) | ((Ereg == 0x7f) << 2) | (((Ereg & 0xF) == 15) << 4) | 2;
}

void x1E() /* LD   E,n */
{
Ereg=*(PC++);
}

void x1F() /* RRA */
{
Cy=(Freg & 1); Cy2=Cy;
tmp=Areg;
Cy=(tmp & 1);
tmp=tmp >> 1;
if (Cy2) tmp=(tmp | 128);
Areg=tmp;
Freg=(Freg & 0xEC) | Cy;
}

void x20() /* JR NZ */
{
tmp=*(PC++);
if (Freg & 64 ) return;
if (tmp < 128) PC=PC+tmp;
else           PC=PC+tmp-256;
}

void x21() /* LD   HL,nnnn */
{
Lreg=*(PC++);
Hreg=*(PC++);
}

void x22() /* LD   (nnnn),HL */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
mem[tmp++]=Lreg;
mem[tmp]=Hreg;
}

void x23() /* INC  HL */
{
if((++Lreg) == 0) Hreg++;
}

void x24() /* INC  H */
{
Hreg++;
Freg=(Freg & 0x2B) | (Hreg & 128) | ((Hreg == 0) << 6) | ((Hreg == 0x80) << 2) | (((Hreg & 0xF) == 0) << 4);
}

void x25() /* DEC  H */
{
Hreg--;
Freg=(Freg & 0x2B) | (Hreg & 128) | ((Hreg == 0) << 6) | ((Hreg == 0x7f) << 2) | (((Hreg & 0xF) == 15) << 4) | 2;
}

void x26() /* LD   H,n */
{
Hreg=*(PC++);
}

void x27() /* DAA */
{
tmp=Areg;
if(Freg & 2) {
  if((tmp & 15) > 9 || (Freg & 16)) tmp=tmp-6;
  if((tmp & 0xF0) > 0x90 || (tmp > 255)) tmp=tmp-0x60;
  Freg=(Freg & 0x3E);
  } else {
  if((Areg & 15) > 9 || (Freg & 16)) tmp=Areg+6;
  if((tmp & 0xF0) > 0x90 || (tmp > 255)) tmp=tmp+0x60;
  Freg=(Freg & 0x3E);
  if (tmp > 255) Freg++;
  }
Areg=tmp;
Freg=(Freg  | (Areg & 128));
if (Areg==0) Freg=(Freg | 64);
}

void x28() /* JR Z  */
{
tmp=*(PC++);
if ((Freg & 64 )==0) return;
if (tmp < 128) PC=PC+tmp;
else           PC=PC+tmp-256;
}

void x29() /* ADD  HL,HL */
{
tmp=Lreg | (Hreg << 8);
tmp2=Lreg | (Hreg << 8);
tmp=tmp+tmp2;
Freg=(Freg & 0xFC);  /* H should also get set */
if (tmp > 65535) Freg++;
Lreg=tmp;
Hreg=tmp >> 8;
}

void x2A() /* LD HL,(nnnn) */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
Lreg=mem[tmp++];
Hreg=mem[tmp];
}

void x2B() /* DEC  HL */
{
if((--Lreg) == 0xFF) Hreg--;
}

void x2C() /* INC  L */
{
Lreg++;
Freg=(Freg & 0x2B) | (Lreg & 128) | ((Lreg == 0) << 6) | ((Lreg == 0x80) << 2) | (((Lreg & 0xF) == 0) << 4);
}

void x2D() /* DEC  L */
{
Lreg--;
Freg=(Freg & 0x2B) | (Lreg & 128) | ((Lreg == 0) << 6) | ((Lreg == 0x7f) << 2) | (((Lreg & 0xF) == 15) << 4) | 2;
}

void x2E() /* LD   L,n */
{
Lreg=*(PC++);
}

void x2F() /* CPL */
{
Areg=255-Areg;
Freg=(Freg | 0x12);
}

void x30() /* JR NC */
{
tmp=*(PC++);
if (Freg & 1  ) return;
if (tmp < 128) PC=PC+tmp;
else           PC=PC+tmp-256;
}

void x31() /* LD   SP,nnnn */
{
SP=*(PC++);
SP=SP | (*(PC++) << 8);
}

void x32() /* LD   (nnnn),A */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
mem[tmp]=Areg;
}

void x33() /* INC  SP */
{
SP++;
}

void x34() /* INC  (HL) */
{
tmp=++mem[Lreg | (Hreg << 8)];
Freg=(Freg & 0x2B) | (tmp & 128) | ((tmp == 0) << 6) | ((tmp == 0x80) << 2) | (((tmp & 0xF) == 0) << 4);
}

void x35() /* DEC  (HL) */
{
tmp=--mem[Lreg | (Hreg << 8)];
Freg=(Freg & 0x2B) | (tmp & 128) | ((tmp == 0) << 6) | ((tmp == 0x7f) << 2) | (((tmp & 0xF) == 15) << 4) | 2;
}

void x36() /* LD   (HL),n */
{
mem[Lreg | (Hreg << 8)]=*(PC++);
}

void x37() /* SCF */
{
Freg=(Freg | 1);
}

void x38() /* JR C  */
{
tmp=*(PC++);
if ((Freg & 1  )==0) return;
if (tmp < 128) PC=PC+tmp;
else           PC=PC+tmp-256;
}

void x39() /* ADD  HL,SP */
{
tmp=Lreg | (Hreg << 8);
tmp2=SP;
tmp=tmp+tmp2;
Freg=(Freg & 0xFC);  /* H should also get set */
if (tmp > 65535) Freg++;
Lreg=tmp;
Hreg=tmp >> 8;
}

void x3A() /* LD A,(nnnn) */
{
tmp=*(PC++);
tmp=tmp | (*(PC++) << 8);
Areg=mem[tmp];
}

void x3B() /* DEC  SP */
{
SP--;
}

void x3C() /* INC  A */
{
Areg++;
Freg=(Freg & 0x2B) | (Areg & 128) | ((Areg == 0) << 6) | ((Areg == 0x80) << 2) | (((Areg & 0xF) == 0) << 4);
}

void x3D() /* DEC  A */
{
Areg--;
Freg=(Freg & 0x2B) | (Areg & 128) | ((Areg == 0) << 6) | ((Areg == 0x7f) << 2) | (((Areg & 0xF) == 15) << 4) | 2;
}

void x3E() /* LD   A,n */
{
Areg=*(PC++);
}

void x3F() /* CCF */
{
Freg=(Freg ^ 1);
}


/* Block 2 */
/* LD r1,r2 - Move between registers (and HL=>mem) */
void x40() /* LD B,B */
{
Breg=Breg;
}

void x41() /* LD B,C */
{
Breg=Creg;
}

void x42() /* LD B,D */
{
Breg=Dreg;
}

void x43() /* LD B,E */
{
Breg=Ereg;
}

void x44() /* LD B,H */
{
Breg=Hreg;
}

void x45() /* LD B,L */
{
Breg=Lreg;
}

void x46() /* LD B,(HL) */
{
Breg=mem[Lreg | (Hreg << 8)];
}

void x47() /* LD B,A */
{
Breg=Areg;
}

void x48() /* LD C,B */
{
Creg=Breg;
}

void x49() /* LD C,C */
{
Creg=Creg;
}

void x4A() /* LD C,D */
{
Creg=Dreg;
}

void x4B() /* LD C,E */
{
Creg=Ereg;
}

void x4C() /* LD C,H */
{
Creg=Hreg;
}

void x4D() /* LD C,L */
{
Creg=Lreg;
}

void x4E() /* LD C,(HL) */
{
Creg=mem[Lreg | (Hreg << 8)];
}

void x4F() /* LD C,A */
{
Creg=Areg;
}

void x50() /* LD D,B */
{
Dreg=Breg;
}

void x51() /* LD D,C */
{
Dreg=Creg;
}

void x52() /* LD D,D */
{
Dreg=Dreg;
}

void x53() /* LD D,E */
{
Dreg=Ereg;
}

void x54() /* LD D,H */
{
Dreg=Hreg;
}

void x55() /* LD D,L */
{
Dreg=Lreg;
}

void x56() /* LD D,(HL) */
{
Dreg=mem[Lreg | (Hreg << 8)];
}

void x57() /* LD D,A */
{
Dreg=Areg;
}

void x58() /* LD E,B */
{
Ereg=Breg;
}

void x59() /* LD E,C */
{
Ereg=Creg;
}

void x5A() /* LD E,D */
{
Ereg=Dreg;
}

void x5B() /* LD E,E */
{
Ereg=Ereg;
}

void x5C() /* LD E,H */
{
Ereg=Hreg;
}

void x5D() /* LD E,L */
{
Ereg=Lreg;
}

void x5E() /* LD E,(HL) */
{
Ereg=mem[Lreg | (Hreg << 8)];
}

void x5F() /* LD E,A */
{
Ereg=Areg;
}

void x60() /* LD H,B */
{
Hreg=Breg;
}

void x61() /* LD H,C */
{
Hreg=Creg;
}

void x62() /* LD H,D */
{
Hreg=Dreg;
}

void x63() /* LD H,E */
{
Hreg=Ereg;
}

void x64() /* LD H,H */
{
Hreg=Hreg;
}

void x65() /* LD H,L */
{
Hreg=Lreg;
}

void x66() /* LD H,(HL) */
{
Hreg=mem[Lreg | (Hreg << 8)];
}

void x67() /* LD H,A */
{
Hreg=Areg;
}

void x68() /* LD L,B */
{
Lreg=Breg;
}

void x69() /* LD L,C */
{
Lreg=Creg;
}

void x6A() /* LD L,D */
{
Lreg=Dreg;
}

void x6B() /* LD L,E */
{
Lreg=Ereg;
}

void x6C() /* LD L,H */
{
Lreg=Hreg;
}

void x6D() /* LD L,L */
{
Lreg=Lreg;
}

void x6E() /* LD L,(HL) */
{
Lreg=mem[Lreg | (Hreg << 8)];
}

void x6F() /* LD L,A */
{
Lreg=Areg;
}

void x70() /* LD (HL),B */
{
mem[Lreg | (Hreg << 8)]=Breg;
}

void x71() /* LD (HL),C */
{
mem[Lreg | (Hreg << 8)]=Creg;
}

void x72() /* LD (HL),D */
{
mem[Lreg | (Hreg << 8)]=Dreg;
}

void x73() /* LD (HL),E */
{
mem[Lreg | (Hreg << 8)]=Ereg;
}

void x74() /* LD (HL),H */
{
mem[Lreg | (Hreg << 8)]=Hreg;
}

void x75() /* LD (HL),L */
{
mem[Lreg | (Hreg << 8)]=Lreg;
}

void x76() /* LD (HL),(HL) */
{
/* HALT */
}

void x77() /* LD (HL),A */
{
mem[Lreg | (Hreg << 8)]=Areg;
}

void x78() /* LD A,B */
{
Areg=Breg;
}

void x79() /* LD A,C */
{
Areg=Creg;
}

void x7A() /* LD A,D */
{
Areg=Dreg;
}

void x7B() /* LD A,E */
{
Areg=Ereg;
}

void x7C() /* LD A,H */
{
Areg=Hreg;
}

void x7D() /* LD A,L */
{
Areg=Lreg;
}

void x7E() /* LD A,(HL) */
{
Areg=mem[Lreg | (Hreg << 8)];
}

void x7F() /* LD A,A */
{
Areg=Areg;
}


/* ALU codes */
/* ADD A,r - Addition */
void x80() /* ADD A,B */
{
val=Breg;
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x81() /* ADD A,C */
{
val=Creg;
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x82() /* ADD A,D */
{
val=Dreg;
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x83() /* ADD A,E */
{
val=Ereg;
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x84() /* ADD A,H */
{
val=Hreg;
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x85() /* ADD A,L */
{
val=Lreg;
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x86() /* ADD A,(HL) */
{
val=mem[Lreg | (Hreg << 8)];
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x87() /* ADD A,A */
{
val=Areg;
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

/* ADC A,r - Addition */
void x88() /* ADC A,B */
{
val=Breg+(Freg & 1);
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x89() /* ADC A,C */
{
val=Creg+(Freg & 1);
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x8A() /* ADC A,D */
{
val=Dreg+(Freg & 1);
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x8B() /* ADC A,E */
{
val=Ereg+(Freg & 1);
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x8C() /* ADC A,H */
{
val=Hreg+(Freg & 1);
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x8D() /* ADC A,L */
{
val=Lreg+(Freg & 1);
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x8E() /* ADC A,(HL) */
{
val=mem[Lreg | (Hreg << 8)]+(Freg & 1);
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void x8F() /* ADC A,A */
{
val=Areg+(Freg & 1);
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

/* SUB r - Subtraction */
void x90() /* SBC B */
{
val=Breg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x91() /* SBC C */
{
val=Creg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x92() /* SBC D */
{
val=Dreg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x93() /* SBC E */
{
val=Ereg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x94() /* SBC H */
{
val=Hreg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x95() /* SBC L */
{
val=Lreg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x96() /* SBC (HL) */
{
val=mem[Lreg | (Hreg << 8)];
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x97() /* SBC A */
{
val=Areg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

/* SBC A,r - Subtraction */
void x98() /* SBC A,B */
{
val=Breg+(Freg & 1);
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x99() /* SBC A,C */
{
val=Creg+(Freg & 1);
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x9A() /* SBC A,D */
{
val=Dreg+(Freg & 1);
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x9B() /* SBC A,E */
{
val=Ereg+(Freg & 1);
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x9C() /* SBC A,H */
{
val=Hreg+(Freg & 1);
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x9D() /* SBC A,L */
{
val=Lreg+(Freg & 1);
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x9E() /* SBC A,(HL) */
{
val=mem[Lreg | (Hreg << 8)]+(Freg & 1);
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void x9F() /* SBC A,A */
{
val=Areg+(Freg & 1);
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

/* AND r - Logical AND */
void xA0() /* AND B */
{
Areg=(Areg & Breg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | 16 | parity[Areg];
}

void xA1() /* AND C */
{
Areg=(Areg & Creg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | 16 | parity[Areg];
}

void xA2() /* AND D */
{
Areg=(Areg & Dreg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | 16 | parity[Areg];
}

void xA3() /* AND E */
{
Areg=(Areg & Ereg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | 16 | parity[Areg];
}

void xA4() /* AND H */
{
Areg=(Areg & Hreg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | 16 | parity[Areg];
}

void xA5() /* AND L */
{
Areg=(Areg & Lreg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | 16 | parity[Areg];
}

void xA6() /* AND (HL) */
{
Areg=(Areg & mem[Lreg | (Hreg << 8)]);
Freg=(Areg & 128) | ((Areg == 0) << 6) | 16 | parity[Areg];
}

void xA7() /* AND A */
{
Areg=(Areg & Areg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | 16 | parity[Areg];
}

/* XOR r - Logical XOR */
void xA8() /* XOR B */
{
Areg=(Areg ^ Breg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xA9() /* XOR C */
{
Areg=(Areg ^ Creg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xAA() /* XOR D */
{
Areg=(Areg ^ Dreg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xAB() /* XOR E */
{
Areg=(Areg ^ Ereg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xAC() /* XOR H */
{
Areg=(Areg ^ Hreg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xAD() /* XOR L */
{
Areg=(Areg ^ Lreg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xAE() /* XOR (HL) */
{
Areg=(Areg ^ mem[Lreg | (Hreg << 8)]);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xAF() /* XOR A */
{
Areg=(Areg ^ Areg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

/* OR r - Logical OR */
void xB0() /* OR B */
{
Areg=(Areg | Breg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xB1() /* OR C */
{
Areg=(Areg | Creg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xB2() /* OR D */
{
Areg=(Areg | Dreg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xB3() /* OR E */
{
Areg=(Areg | Ereg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xB4() /* OR H */
{
Areg=(Areg | Hreg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xB5() /* OR L */
{
Areg=(Areg | Lreg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xB6() /* OR (HL) */
{
Areg=(Areg | mem[Lreg | (Hreg << 8)]);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xB7() /* OR A */
{
Areg=(Areg | Areg);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

/* CP r - Comparison */
void xB8() /* CP   B */
{
val=Breg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xB9() /* CP   C */
{
val=Creg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xBA() /* CP   D */
{
val=Dreg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xBB() /* CP   E */
{
val=Ereg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xBC() /* CP   H */
{
val=Hreg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xBD() /* CP   L */
{
val=Lreg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xBE() /* CP   (HL) */
{
val=mem[Lreg | (Hreg << 8)];
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xBF() /* CP   A */
{
val=Areg;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

/* Block 3 - Calls, jumps, misc, etc. */
void xC0() /* RET  NZ */
{
if (Freg & 64 ) return;
tmp=mem[SP++];
tmp=tmp + (mem[SP++] << 8);
SP=SP & 0xFFFF;     /* Careful about &FFFF->&0000 */
PC=&mem[0]+tmp;
}

void xC1() /* POP  BC */
{
Creg=mem[SP++];
Breg=mem[SP++];
SP=SP & 0xFFFF;
}

void xC2() /* JP   NZ,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if ((Freg & 64 )==0) PC=&mem[0]+tmp;
}

void xC3() /* JP nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
PC=&mem[0]+tmp;
}

void xC4() /* CALL NZ,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if (Freg & 64 ) return;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xC5() /* PUSH BC */
{
mem[(SP=(SP-1) & 0xFFFF)]=Breg;
mem[(SP=(SP-1) & 0xFFFF)]=Creg;
}

void xC6() /* ADD A,n */
{
tmp=*(PC++);
val=tmp;
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void xC7() /* RST &0 */
{
tmp=0x0;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xC8() /* RET  Z  */
{
if ((Freg & 64 )==0) return;
tmp=mem[SP++];
tmp=tmp + (mem[SP++] << 8);
SP=SP & 0xFFFF;     /* Careful about &FFFF->&0000 */
PC=&mem[0]+tmp;
}

void xC9() /* RET */
{
tmp=mem[SP++];
tmp=tmp + (mem[SP++] << 8);
SP=SP & 0xFFFF;     /* Careful about &FFFF->&0000 */
PC=&mem[0]+tmp;
}

void xCA() /* JP   Z ,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if (Freg & 64 ) PC=&mem[0]+tmp;
}

void xCB() /* CB codes - rotates, etc. */
{
/* opcode=*(PC++); */
(*codeCB[*(PC++)])();
}

void xCC() /* CALL Z ,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if ((Freg & 64 )==0) return;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xCD() /* CALL nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xCE() /* ADC A,n */
{
tmp=*(PC++);
val=tmp+(Freg & 1);
H=((Areg & 0xF) + (val & 0xF)) > 15;
PV=((Areg & 0x7F) + (val & 0x7F)) > 127;
tmp=Areg+val;
Cy=(tmp > 255);
PV=PV ^ Cy;
Areg=tmp;
Freg=(Areg & 128) | ((Areg == 0) << 6) | (H << 4) | (PV << 2) | Cy;
}

void xCF() /* RST &8 */
{
tmp=0x8;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xD0() /* RET  NC */
{
if (Freg & 1  ) return;
tmp=mem[SP++];
tmp=tmp + (mem[SP++] << 8);
SP=SP & 0xFFFF;     /* Careful about &FFFF->&0000 */
PC=&mem[0]+tmp;
}

void xD1() /* POP  DE */
{
Ereg=mem[SP++];
Dreg=mem[SP++];
SP=SP & 0xFFFF;
}

void xD2() /* JP   NC,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if ((Freg & 1  )==0) PC=&mem[0]+tmp;
}

void xD3() /* OUT  (n),A */
{
tmp=*(PC++);
io_OUT(tmp | (Areg << 8),Areg);
}

void xD4() /* CALL NC,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if (Freg & 1  ) return;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xD5() /* PUSH DE */
{
mem[(SP=(SP-1) & 0xFFFF)]=Dreg;
mem[(SP=(SP-1) & 0xFFFF)]=Ereg;
}

void xD6() /* SUB A,n */
{
tmp=*(PC++);
val=tmp;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xD7() /* RST &10 */
{
tmp=0x10;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xD8() /* RET  C  */
{
if ((Freg & 1  )==0) return;
tmp=mem[SP++];
tmp=tmp + (mem[SP++] << 8);
SP=SP & 0xFFFF;     /* Careful about &FFFF->&0000 */
PC=&mem[0]+tmp;
}

void xD9() /* EXX */
{
tmp=Balt; Balt=Breg; Breg=tmp;
tmp=Calt; Calt=Creg; Creg=tmp;
tmp=Dalt; Dalt=Dreg; Dreg=tmp;
tmp=Ealt; Ealt=Ereg; Ereg=tmp;
tmp=Halt; Halt=Hreg; Hreg=tmp;
tmp=Lalt; Lalt=Lreg; Lreg=tmp;
}

void xDA() /* JP   C ,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if (Freg & 1  ) PC=&mem[0]+tmp;
}

void xDB() /* IN   A,(n) */
{
tmp=*(PC++);
Areg=io_IN(tmp | (Areg << 8));
}

void xDC() /* CALL C ,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if ((Freg & 1  )==0) return;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xDD() /* IX register prefix */
{
opcode=*(PC++);
off=*(PC); /* Don't increment yet, might not be used */
if (off > 127) off=off-256;
addr=((XYreg[(XY=0)] + off) & 0xFFFF);
(*codeXY[opcode])();
}

void xDE() /* SBC A,n */
{
tmp=*(PC++);
val=tmp+(Freg & 1);
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Areg=tmp;
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xDF() /* RST &18 */
{
tmp=0x18;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xE0() /* RET  PO */
{
if (Freg & 4  ) return;
tmp=mem[SP++];
tmp=tmp + (mem[SP++] << 8);
SP=SP & 0xFFFF;     /* Careful about &FFFF->&0000 */
PC=&mem[0]+tmp;
}

void xE1() /* POP  HL */
{
Lreg=mem[SP++];
Hreg=mem[SP++];
SP=SP & 0xFFFF;
}

void xE2() /* JP   PO,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if ((Freg & 4  )==0) PC=&mem[0]+tmp;
}

void xE3() /* EX   (SP),HL */
{
tmp=Lreg; Lreg=mem[SP]; mem[SP]=tmp;
tmp=Hreg; Hreg=mem[SP+1]; mem[SP+1]=tmp;
}

void xE4() /* CALL PO,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if (Freg & 4  ) return;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xE5() /* PUSH HL */
{
mem[(SP=(SP-1) & 0xFFFF)]=Hreg;
mem[(SP=(SP-1) & 0xFFFF)]=Lreg;
}

void xE6() /* AND A,n */
{
tmp=*(PC++);
Areg=(Areg & tmp);
Freg=(Areg & 128) | ((Areg == 0) << 6) | 16 | parity[Areg];
}

void xE7() /* RST &20 */
{
tmp=0x20;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xE8() /* RET  PE */
{
if ((Freg & 4  )==0) return;
tmp=mem[SP++];
tmp=tmp + (mem[SP++] << 8);
SP=SP & 0xFFFF;     /* Careful about &FFFF->&0000 */
PC=&mem[0]+tmp;
}

void xE9() /* JP   (HL) */
{
PC=&mem[0]+(Lreg | (Hreg << 8));
}

void xEA() /* JP   PE,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if (Freg & 4  ) PC=&mem[0]+tmp;
}

void xEB() /* EX  DE,HL */
{
tmp=Dreg; Dreg=Hreg; Hreg=tmp;
tmp=Ereg; Ereg=Lreg; Lreg=tmp;
}

void xEC() /* CALL PE,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if ((Freg & 4  )==0) return;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xED() /* ED prefix */
{
/* opcode=*(PC++); */
(*codeED[*(PC++)])();
}

void xEE() /* XOR A,n */
{
tmp=*(PC++);
Areg=(Areg ^ tmp);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xEF() /* RST &28 */
{
tmp=0x28;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xF0() /* RET  P  */
{
if (Freg & 128) return;
tmp=mem[SP++];
tmp=tmp + (mem[SP++] << 8);
SP=SP & 0xFFFF;     /* Careful about &FFFF->&0000 */
PC=&mem[0]+tmp;
}

void xF1() /* POP  AF */
{
Freg=mem[SP++];
Areg=mem[SP++];
SP=SP & 0xFFFF;
}

void xF2() /* JP   P ,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if ((Freg & 128)==0) PC=&mem[0]+tmp;
}

void xF3() /* DI */
{
IFF1=1; IFF2=1;
}

void xF4() /* CALL P ,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if (Freg & 128) return;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xF5() /* PUSH AF */
{
mem[(SP=(SP-1) & 0xFFFF)]=Areg;
mem[(SP=(SP-1) & 0xFFFF)]=Freg;
}

void xF6() /* OR  A,n */
{
tmp=*(PC++);
Areg=(Areg | tmp);
Freg=(Areg & 128) | ((Areg == 0) << 6) | parity[Areg];
}

void xF7() /* RST &30 */
{
tmp=0x30;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xF8() /* RET  M  */
{
if ((Freg & 128)==0) return;
tmp=mem[SP++];
tmp=tmp + (mem[SP++] << 8);
SP=SP & 0xFFFF;     /* Careful about &FFFF->&0000 */
PC=&mem[0]+tmp;
}

void xF9() /* LD   SP,HL */
{
SP = Lreg | (Hreg << 8);
}

void xFA() /* JP   M ,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if (Freg & 128) PC=&mem[0]+tmp;
}

void xFB() /* EI */
{
IFF1=0; IFF2=0;
}

void xFC() /* CALL M ,nnnn */
{
tmp=*(PC++);
tmp=tmp + (*(PC++) << 8);
if ((Freg & 128)==0) return;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}

void xFD() /* IY register prefix */
{
opcode=*(PC++);
off=*(PC); /* Don't increment yet, might not be used */
if (off > 127) off=off-256;
addr=((XYreg[(XY=1)] + off) & 0xFFFF);
(*codeXY[opcode])();
}

void xFE() /* CP  A,n */
{
tmp=*(PC++);
val=tmp;
H=((Areg & 0xF) - (val & 0xF)) <0;
PV=((Areg & 0x7F) - (val & 0x7F)) <0;
tmp=Areg-val;
Cy=(tmp < 0);
PV=PV ^ Cy;
tmp=(tmp & 255);
Freg=(tmp & 128) | ((tmp == 0) << 6) | (H << 4) | (PV << 2) | 2 | Cy;
}

void xFF() /* RST &38 */
{
tmp=0x38;
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) >> 8); /* Careful about &0000->&FFFF */
mem[(SP=(SP-1) & 0xFFFF)]=((PC-&mem[0]) & 255);
PC=&mem[0]+tmp;
}


/* Table of addresses for normal codes */

static void (*code00[])(void) = {x00, x01, x02, x03, x04, x05, x06, 
      x07, x08, x09, x0A, x0B, x0C, x0D, x0E, x0F, x10, x11, x12, x13, 
      x14, x15, x16, x17, x18, x19, x1A, x1B, x1C, x1D, x1E, x1F, x20, 
      x21, x22, x23, x24, x25, x26, x27, x28, x29, x2A, x2B, x2C, x2D, 
      x2E, x2F, x30, x31, x32, x33, x34, x35, x36, x37, x38, x39, x3A, 
      x3B, x3C, x3D, x3E, x3F, x40, x41, x42, x43, x44, x45, x46, x47, 
      x48, x49, x4A, x4B, x4C, x4D, x4E, x4F, x50, x51, x52, x53, x54, 
      x55, x56, x57, x58, x59, x5A, x5B, x5C, x5D, x5E, x5F, x60, x61, 
      x62, x63, x64, x65, x66, x67, x68, x69, x6A, x6B, x6C, x6D, x6E, 
      x6F, x70, x71, x72, x73, x74, x75, x76, x77, x78, x79, x7A, x7B, 
      x7C, x7D, x7E, x7F, x80, x81, x82, x83, x84, x85, x86, x87, x88, 
      x89, x8A, x8B, x8C, x8D, x8E, x8F, x90, x91, x92, x93, x94, x95, 
      x96, x97, x98, x99, x9A, x9B, x9C, x9D, x9E, x9F, xA0, xA1, xA2, 
      xA3, xA4, xA5, xA6, xA7, xA8, xA9, xAA, xAB, xAC, xAD, xAE, xAF, 
      xB0, xB1, xB2, xB3, xB4, xB5, xB6, xB7, xB8, xB9, xBA, xBB, xBC, 
      xBD, xBE, xBF, xC0, xC1, xC2, xC3, xC4, xC5, xC6, xC7, xC8, xC9, 
      xCA, xCB, xCC, xCD, xCE, xCF, xD0, xD1, xD2, xD3, xD4, xD5, xD6, 
      xD7, xD8, xD9, xDA, xDB, xDC, xDD, xDE, xDF, xE0, xE1, xE2, xE3, 
      xE4, xE5, xE6, xE7, xE8, xE9, xEA, xEB, xEC, xED, xEE, xEF, xF0, 
      xF1, xF2, xF3, xF4, xF5, xF6, xF7, xF8, xF9, xFA, xFB, xFC, xFD, 
      xFE, xFF };

void execute()
{
io_start();                    /* Initialise I/O */
if (debug & 1) {               /* In debug mode, print info */
  for (;;) { disp_regs(); (*code00[*(PC++)])(); }
  }
for (;;) (*code00[*(PC++)])(); /* Interpret bytes */
}

/* end of z80.c */
