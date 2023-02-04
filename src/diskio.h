/* diskio.h by Dave Daniels

*/

#ifndef __diskio_h
#define __diskio_h

#define MAX_DRIVES 7			/* 8 drives, 0 to 7		*/
#define MAX_SECTORS 15			/* 16 sectors, 0 to 15		*/

/* Values for 'flags' in disks[] */
#define DISK_DIR 1			/*  1 = Disk is really a directory */
#define DISK_RO  2			/*  2 = Disk is read only	*/
#define DISK_WR  4			/*  4 = Last disk op was a write */
#define DISK_WRITTEN 8			/*  8 = Disk has been written to */                   

/* Status/Error values returned by Osword 127 code
 * Return values: 000ttsss
 */
#define DISK_OK          0		/* No error detected		*/
/* Data access errors */
#define DISK_BAD_FORMAT  0x08		/* Not recognised format	*/
#define DISK_ABORT       0x0A
#define DISK_BAD_ID      0x0C		/* Bad sector ID/size		*/
#define DISK_BAD_CRC     0x0E
/* Disk access errors */
#define DISK_NOT_READY   0x10		/* Should always cause a retry	*/
#define DISK_11          0x11
#define DISK_READ_ONLY   0x12		/* Disk read only		*/
#define DISK_13          0x13
#define DISK_TRACK_ZERO  0x14		/* Track 0 not found		*/
#define DISK_15          0x15
#define DISK_FAILED      0x16		/* Write fault (disk write fault) */
#define DISK_BAD_ADDR	 0x17		/* Bad Z80 address (memory access fault) */
/* Drive/media access error */
#define DISK_NOT_FOUND   0x18		/* Sector not found/out of range */
#define DISK_BAD_DRIVE   0x19		/* Bad drive number		*/
#define DISK_CHANGED     0x1A		/* Disk changed			*/
#define DISK_DUP_DISK    0x1B		/* Disk already mounted		*/
#define DISK_REQUEST     0x1C		/* Media change request		*/
#define DISK_BAD_NAME	 0x1D		/* Bad image file name		*/
#define DISK_DRIVE_EMPTY 0x1E		/* Drive empty/not present	*/
#define DISK_NOT_MOUNTED 0x1F		/* Disk not mounted on drive	*/
#define DISK_BAD_COMMAND 0xFE		/* Unsupported/bad disk command	*/

/* Osword 127 disk commands */
#define DISK_WRITE       0x4B		/* Write sectors		*/
#define DISK_READ        0x53		/* Read sectors			*/

/* DFS error numbers */
#define DFS_DUPLICATE    174		/* Error 174 - Same disk used	 */
#define DFS_DISK_FAULT   199		/* Error 199 - Disk fault	 */
#define DFS_DISK_CHANGED 200		/* Error 200 - Disk changed	 */
#define DFS_DISK_RO      201		/* Error 201 - Disk is read only */
#define DFS_BAD_DRIVE    205		/* Error 205 - Bad drive	 */
#define DFS_DRIVE_EMPTY  211		/* Error 211 - Disk/drive absent */

extern void disk_init(void);
extern void osword_127(void);
extern int cli_mount(void);
extern int cli_dismount(void);
extern int cli_show(void);

#endif
