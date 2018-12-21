/* diskio.c -
 * Copyright (C) 1987-2003 J.G.Harston, David Daniels

 See the accompanying file LICENSE for authors and terms of use.

   31-Jul-2004 DD:  OSWORD 127 disk image access

   Visible functions:
   disk_init()    - Initialise disk system
   osword_127()   - Disk sector access
   cli_mount()    - Mount disk image
   cli_dismount() - Dismount disk image
   cli_show()     - Show mounts

   Internal functions:
   disk_fdc()     - Do a disk access
*/

/* ==================================================================== */
/* Disk I/O routines by Dave Daniels					*/
/* ==================================================================== */

struct {
  char *diskfile;		/* Name of disk or directory. 0 = not mounted */
  FILE *handle;			/* File's handle			*/
  int tracks;			/* Number of tracks 0 to 255		*/
  int bytes_per_track;		/* Number of bytes per track		*/
  int sector_size;		/* Size of a sector in bytes		*/
  int flags;			/* Flags				*/
} disks [MAX_DRIVES];


/*
 * disk_dismount - Called to close the file associated with the
 * drive number passed, or all files if the drive number is -1
 */
int disk_dismount(int drive) {
  if (drive < -1 || drive > MAX_DRIVES) return DISK_BAD_DRIVE;
  if (drive == -1) {
    int n;
    for (n = 0; n < MAX_DRIVES; n++) {
      if (disks[n].diskfile != 0) {
        fclose(disks[n].handle);
        free(disks[n].diskfile);
        disks[n].diskfile = 0;
      }
    }
  } else if (disks[drive].diskfile != 0) {
    fclose(disks[drive].handle);
    free(disks[drive].diskfile);
    disks[drive].diskfile = 0;
  }
  else {
    return DISK_NOT_MOUNTED;
  }
  return DISK_OK;
}

/*
 * disk_mount - Mounts disk image 'name' on drive 'drive'. It returns
 * 0 if this worked or an error number if this failed
 */
int disk_mount(int drive, char *name) {
  FILE *f;
  char start[8], *p;
  int count, bytes_per_track, sector_size, n;

  if (drive < 0 || drive >= MAX_DRIVES) return DISK_BAD_DRIVE;
  f = fopen(name, "rb+");
  if (f == 0) return DISK_NOT_FOUND;
  count = fread(start, 8, 1, f);
  if (count != 1) {	/* 8 bytes read? */
    fclose(f);
    return DISK_BAD_FORMAT;
  }

/* Check that file starts with 'Acorn CP' */
/* JGH: Should be able to mount a blank disk for formatting */
  if (strncmp(start, "Acorn CP", 8) == 0) {
    bytes_per_track = 2560;
    sector_size = 256;
  }
  else {
    fclose(f);
    return DISK_BAD_FORMAT;
  }

/* Check size of disk image file */
  fseek(f, 0, SEEK_END);
  count = (int) ftell(f);
  fseek(f, 0, SEEK_SET);
  if (count % bytes_per_track != 0) {	/* Size not multiple of <track size> */
    fclose(f);
    return DISK_BAD_FORMAT;
  }

/* Already mounted? */
   for (n = 0; n < MAX_DRIVES; n++) {
     if (disks[n].diskfile != 0 && strcmp(disks[n].diskfile, name) == 0) return DISK_DUP_DISK;
   }

/* Disk looks okay. Mount it */
  if (disks[drive].diskfile != 0) disk_dismount(drive);
  p = (char *)malloc(strlen(name) + 1);
  strcpy(p, name);
  disks[drive].diskfile = p;
  disks[drive].handle = f;
  disks[drive].bytes_per_track = bytes_per_track;
  disks[drive].tracks = count / bytes_per_track / 2;	/* 2 sides */
  disks[drive].sector_size = sector_size;
  disks[drive].flags = 0;
  return DISK_OK;
}

/*
 * disk_info - Return information about the disk image mounted
 * on drive 'drive'. Returns 0 if call worked or error code
 */
int disk_info(int drive, int size, char *name, int *flags) {
  int len;
  *flags = 0;
  if (drive < 0 || drive >= MAX_DRIVES) return DISK_BAD_DRIVE;
  if (disks[drive].diskfile == 0) return DISK_NOT_MOUNTED;
  len = strlen(disks[drive].diskfile);
  if (len + 1 > size) len = size - 1;
  strncpy(name, disks[drive].diskfile, len + 1);
  name[len] = 0;
  *flags = disks[drive].flags;
  return DISK_OK;
}

/*
 * disk_show - List the disks mounted
 */
int disk_show(void) {
  int n, count;
  count = 0;
  for (n = 0; n < MAX_DRIVES; n++) {
    if (disks[n].diskfile != 0) {
      count++;
      printf("Drive %d (%c:) Disk image = '%s'\n", n, n + 'A', disks[n].diskfile);
    }
  }
  if (count == 0) {
    printf("No disks mounted\n");
    return DISK_NOT_MOUNTED;
  }
  return DISK_OK;
}

/*
 * disk_fdc - Access disk Reads the given sector at (track, sector) into
 * the buffer at 'addr'. This can be in the Z80 memory or the
 * I/O processor memory
 */
int disk_fdc(int drive, int track, int sector, int count, int size, int addr, int action) {
  int head, start, n, done;
  int8 *data;

  if (count == 0) return 0;

/*
 * Start by flushing buffers to disk if the last disk op was
 * a write
 */
  if ((action == DISK_WRITE) && (disks[drive].flags & DISK_RO))
    return DISK_READ_ONLY;
  if ((action == DISK_READ)  && (disks[drive].flags & DISK_WR)) {
    fflush(disks[drive].handle);
    disks[drive].flags -= DISK_WR;
  }

/*
 * Drive numbers: xxffddhd
 * xx   = reserved
 * ff   = density, 00=FM, 01=MFM
 * ddd  = drive
 * h    = head (side)
 */
  head = (drive & 2) >> 1;
  drive = ((drive & 0x1c) >> 1) | (drive & 1);
  start = head == 0 ? 0 : disks[drive].bytes_per_track * disks[drive].tracks;
  start += track * disks[drive].bytes_per_track + sector * disks[drive].sector_size;

/*
 * Data can be read into the I/O or Z80 memory depending on
 * the address. Data is read into the I/O processor if the
 * the address is greater than or equal to 0xffff0000. The
 * data is read into Z80 memory if the address is anything
 * else
 */
  if ((unsigned) addr >= 0xffff0000u)	/* I/O memory			*/
    data = (iomem + (addr & (IOMSIZE-1)));
  else
    data = (mem + addr);		/* Z80 memory			*/

  if (size == disks[drive].sector_size) {	/* Do in one operation	*/
    if (debug & 2) {
      switch (action) {
        case DISK_READ:  printf("Read"); break;
        case DISK_WRITE: printf("Read"); break;
        default:         printf("FDC %x",action); break;
      }
      printf(" - seek to %x\n", start);
    }
    (void) fseek(disks[drive].handle, start, SEEK_SET);
    switch (action) {
      case DISK_READ:
        done = fread(data, size, count, disks[drive].handle);
        break;
      case DISK_WRITE:
        done = fwrite(data, size, count, disks[drive].handle);
        disks[drive].flags |= (DISK_WR | DISK_WRITTEN);
        				/* Last op was a write		*/
        break;
    }
    if (debug & 2) printf("Completed: %d\n", done);
    if (done != count) return DISK_FAILED;	/* FDC fault		*/
  }
  else {
    for (n = 0; n < count; n++) {
      (void) fseek(disks[drive].handle, start, SEEK_SET);
      switch (action) {
        case DISK_READ:
          done = fread(data, size, 1, disks[drive].handle);
          break;
        case DISK_WRITE:
          done = fwrite(data, size, 1, disks[drive].handle);
          disks[drive].flags |= DISK_WR;	/* Last op was a write	*/
          break;
      }
      if (done != 1) return DISK_FAILED;	/* FDC fault		*/
      start += disks[drive].sector_size;
      addr += size;
    }
  }
  return 0;
}


/*
 * disk_init - Called to initialise the disk table and to
 * mount the disks from the 'Z80Tube$Drive#' environment
 * variables.
 */
void disk_init(void) {
  int n, rc;
  char drivevar[20], file[256];

#ifdef Z80FILE_RO
  for (n = 0; n < MAX_DRIVES; n++) disks[n].diskfile = 0;
  for (n = 0; n < MAX_DRIVES; n++) {
    sprintf(drivevar, "Z80Tube$Drive%d", n);
    file[0] = 0;	/* Clear return block */
    _kernel_getenv(drivevar, file, 250);
    if (file[0] != 0) {	/* Drive variable found */
      rc = disk_mount(n, file);
      switch (rc) {
      case DISK_BAD_DRIVE:
        printf("Drive number %d is out of the range 0..%d\n", n, MAX_DRIVES);
        break;
      case DISK_NOT_FOUND:
        printf("Disk image '%s' not found for drive %d\n", file, n);
        break;
      case DISK_BAD_FORMAT:
        printf("Disk image '%s' is not an Acorn CP/M disk\n", file);
        break;
      case DISK_DUP_DISK:
        printf("Disk image '%s' is already mounted\n", file);
        break;
      default:
        break;
      }
      if (debug & 2) {
        printf("File '%s' mounted as drive %c:\n", file, n + 'A');
        printf("%d tracks of %d bytes  sector size = %d\n",
        disks[n].tracks, disks[n].bytes_per_track, disks[n].sector_size);
      }
    }
  }
#endif
  (void) clock();	/* Start the clock running */
}


/*
 * osword_127 - Emulate OSWORD 127 to read/write from disk.
 * This only handles emulated CP/M disks.
 * On entry, HLreg contains the address within the Z80 memory
 * of the parameter block. Format is:
 * +0	   Drive number
 * +1..+4  Address of data in I/O processor or Z80 memory
 * +5	   Always 3
 * +6	   &53 to read
 *	   &4b to write
 * +7	   Track number
 * +8	   Sector number
 * +9	   Size of sector and number of sectors
 * +10	   Result
 *
 * All errors should return an error number in 'result'.
 */
void osword_127(void) {
  int drive, drive_no, addr, op, track, sector, sector_size, sector_count, res_offset, x;
  char text[50];

  drive = mem[HLreg];
  addr = mem[(HLreg + 1) & 0xFFFF] | mem[(HLreg + 2) & 0xFFFF] << 8 | 
   mem[(HLreg + 3) & 0xFFFF] << 16 | mem[(HLreg + 4) & 0xFFFF] << 24;
  op = mem[(HLreg + 6) & 0xFFFF];
  track = mem[(HLreg + 7) & 0xFFFF];
  sector = mem[(HLreg + 8) & 0xFFFF];
  x = mem[(HLreg + 9) & 0xFFFF];
  sector_size = x >> 5;
  sector_count = x & 0x1F;
  res_offset = (HLreg + 10) & 0xFFFF;		/* Where result goes */

  if (debug & 2) printf("OSWORD 127 drive=%d  addr=%x  op=%x  track=%d  sector=%d  size=%d  count=%d\n",
   drive, addr, op, track, sector, sector_size, sector_count);

/* Vet the parameters */

  if (addr >= 0x10000) {		/* Bad address in Z80 memory	*/
/*
 * I/O processor addresses are -ve numbers if treated as signed
 */
    mem[res_offset] = DISK_BAD_COMMAND;	/* Bad Osword 127 command	*/
    return;
  }
  if (op != DISK_READ && op != DISK_WRITE) {
    mem[res_offset] = DISK_BAD_COMMAND;	/* Bad Osword 127 command	*/
    return;
  }
  if (drive > MAX_DRIVES) {
    mem[res_offset] = DISK_BAD_DRIVE;	/* Drive out of range		*/
    return;
  }
  drive_no = ((drive & 0x1c) >> 1) | (drive & 1);
  if (disks[drive_no].diskfile == 0) {
    mem[res_offset] = DISK_CHANGED;
    return;
  }
  if (track > disks[drive_no].tracks) {
    mem[res_offset] = DISK_NOT_FOUND;	/* No such sector number	*/
    return;
  }
  if (sector_size > 2) {	/* 0 (128), 1 (256) and 2 (512) only */
    mem[res_offset] = DISK_FAILED;	/* Bad sector size		*/
    return;
  }
  if (sector_count > MAX_SECTORS) {
    mem[res_offset] = DISK_NOT_FOUND;	/* Past end of disk		*/
    return;
  }
  sector_size = 128 << sector_size;
  if (sector_size > disks[drive_no]. sector_size) {
    mem[res_offset] = DISK_FAILED;	/* Bad sector size		*/
    return;
  }

/*
 * Check that the end address of the data lies within
 * the Z80 memory. In real life the data would probably
 * wrap around and overlay low memory but we'll assume
 * it is an error to attempt to do this
 */
  if (addr >= 0 && addr + sector_size * sector_count >= 0x10000) {
    mem[res_offset] = DISK_BAD_COMMAND;	/* Bad data length		*/
    return;
  }
  mem[res_offset] = disk_fdc(drive, track, sector, sector_count, sector_size, addr, op);
  if (debug & 2) printf("OSWORD 127 - return code=%d\n", mem[res_offset]);
}


/* CLI commands for disk access */

/*
 * cli_mount - Called to handle the command to associate a disk
 * image file with a dirve number
 */
int cli_mount(void) {
  char *p;
  int drive, rc;

  p = &iobuffer[lptr];		/* p->command parameters		*/
  if (*p < ' ') return 0;	/* Bad syntax, pass to external		*/
  if (!isdigit(*p)) return 0;	/* Bad syntax, pass to external		*/
  drive = (int) strtol(p, &p, 10);
  if (*p != ' ') return 0;	/* Bad syntax, pass to external		*/
  if (drive > MAX_DRIVES -1 ) {
      z80error(254, "Bad drive", "");
      return 1;
    }
  while (isspace(*p)) p++;	/* Move to <filename>			*/
  if (*p < ' ') return 0;	/* Bad syntax, pass to external		*/
  rc = disk_mount(drive, p);	/* Associate this image with this drive	*/
  switch (rc) {
  case DISK_NOT_FOUND:
    z80error(214, p, " not found"); return 1;
  case DISK_BAD_FORMAT:
    z80error(214, p, " not Acorn CP/M disk"); return 1;
  case DISK_DUP_DISK:
    z80error(214, p, " already mounted"); return 1;
  default:
    break;
  }
  return 1;
}

/*
 * cli_dismount - Called to handle the 'dismount' command which
 * breaks the association between a drive and a disk image
 */
int cli_dismount(void) {
  char *p;
  int drive, rc;

  p = &iobuffer[lptr];		/* p->command parameters		*/
  if (*p < ' ') return 0;	/* Bad syntax, pass to external		*/
  if (!isdigit(*p)) return 0;	/* Bad syntax, pass to external		*/
  drive = (int) strtol(p, &p, 10);
  if (drive > MAX_DRIVES -1 ) {
      z80error(254, "Bad drive", "");
      return 1;
    }
  rc = disk_dismount(drive);
  if (rc == DISK_NOT_MOUNTED) {
    z80error(214, "No disk mounted", ""); return 1;
  }
  return 1;
}

/*
 * cli_show - Called to handle the 'show' command
 */
int cli_mounts(void) {
  (void) disk_show();
  return 1;
}
