/* oswordFF.c - Z80 memory transfer functions
 * 31-Aug-2004: Dave Daniels
*/

/* ==================================================================== */
/* OSWORD &FF - Transfer data between Z80 and I/O memory		*/
/* ==================================================================== */
void osword_255(void) {
  int ioaddr, z80addr, count, op;
  ioaddr  = mem[(HLreg +  2) & 0xFFFF] | mem[(HLreg +  3) & 0xFFFF] << 8;
  z80addr = mem[(HLreg +  6) & 0xFFFF] | mem[(HLreg +  7) & 0xFFFF] << 8;
  count   = mem[(HLreg + 10) & 0xFFFF] | mem[(HLreg + 11) & 0xFFFF] << 8;
  op      = mem[(HLreg + 12) & 0xFFFF];
  if (debug & 2) printf("OSWORD 255 - io:&%x z80:&%x count=%d dir=%d\n",
   ioaddr, z80addr, count, op);
  switch (op) {
    case 0:				/* write to I/O processor from Z80 */
      memmove(iomem + (ioaddr & 0xfff), mem + z80addr, count);
      break;
    case 1:				/* read from I/O processor to Z80 */
      memmove(mem + z80addr, iomem + (ioaddr & 0xfff), count);
      break;
  }
}
