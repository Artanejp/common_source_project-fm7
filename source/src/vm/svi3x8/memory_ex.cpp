/*
	Common Source Code Project
	SVI-3x8

	Origin : src/vm/msx/memory_ex.cpp

	modified by tanam
	Date   : 2018.12.09-

	[ memory ]
*/

#include "memory_ex.h"
#if defined(FDD_PATCH_SLOT)
#include "../disk.h"
#define MSX_SECTOR_SIZE 512
char dskPath[64]="";		/* Disk image path                 	*/
/** Floppy drive *********/
unsigned char fdc_drive = 0;
unsigned char fdc_head = 0;
unsigned char fdc_density = 0;
unsigned char fdc_status = 0;
unsigned char svi_disk_heads[2];
unsigned char svi_disk_tracks=40;
unsigned char svi_UseDisk = 0;

#ifndef UINT16
#define UINT16  unsigned short int
#endif

#ifndef UINT8
#define UINT8   unsigned char
#endif

#define MAX_DRIVES	2	/* we support 'only' four drives in MESS */

#define WD179X_IRQ_CLR  0
#define WD179X_IRQ_SET  1
#define WD179X_DRQ_CLR  2
#define WD179X_DRQ_SET  3

#define DEN_FM_LO	0	/* this is used by TRS-80 (but not working) */
#define DEN_FM_HI	1
#define DEN_MFM_LO	2	/* and this one is the one that works */
#define DEN_MFM_HI	3	/* There were no HD disks back then ;) */

#define REAL_FDD		((void*)-1)

#define FDC_STEP_RATE   0x03    /* Type I additional flags */
#define FDC_STEP_VERIFY 0x04    /* verify track number */
#define FDC_STEP_HDLOAD 0x08    /* load head */
#define FDC_STEP_UPDATE 0x10    /* update track register */

#define FDC_RESTORE     0x00    /* Type I commands */
#define FDC_SEEK        0x10
#define FDC_STEP        0x20
#define FDC_STEP_IN     0x40
#define FDC_STEP_OUT    0x60

#define FDC_MASK_TYPE_I         (FDC_STEP_HDLOAD|FDC_STEP_VERIFY|FDC_STEP_RATE)

/* Type I commands status */
#define STA_1_BUSY      0x01    /* controller is busy */
#define STA_1_IPL       0x02    /* index pulse */
#define STA_1_TRACK0    0x04    /* track 0 detected */
#define STA_1_CRC_ERR   0x08    /* CRC error */
#define STA_1_SEEK_ERR  0x10    /* seek error */
#define STA_1_HD_LOADED 0x20    /* head loaded */
#define STA_1_WRITE_PRO 0x40    /* floppy is write protected */
#define STA_1_NOT_READY 0x80    /* controller not ready */

/* Type II and III additional flags */
#define FDC_DELETED_AM  0x01    /* read/write deleted address mark */
#define FDC_SIDE_CMP_T  0x02    /* side compare track data */
#define FDC_15MS_DELAY  0x04    /* delay 15ms before command */
#define FDC_SIDE_CMP_S  0x08    /* side compare sector data */
#define FDC_MULTI_REC   0x10    /* only for type II commands */

/* Type II commands */
#define FDC_READ_SEC    0x80    /* read sector */
#define FDC_WRITE_SEC   0xA0    /* write sector */

#define FDC_MASK_TYPE_II        (FDC_MULTI_REC|FDC_SIDE_CMP_S|FDC_15MS_DELAY|FDC_SIDE_CMP_T|FDC_DELETED_AM)

/* Type II commands status */
#define STA_2_BUSY      0x01
#define STA_2_DRQ       0x02
#define STA_2_LOST_DAT  0x04
#define STA_2_CRC_ERR   0x08
#define STA_2_REC_N_FND 0x10
#define STA_2_REC_TYPE  0x20
#define STA_2_WRITE_PRO 0x40
#define STA_2_NOT_READY 0x80

#define FDC_MASK_TYPE_III       (FDC_SIDE_CMP_S|FDC_15MS_DELAY|FDC_SIDE_CMP_T|FDC_DELETED_AM)

/* Type III commands */
#define FDC_READ_DAM    0xc0    /* read data address mark */
#define FDC_READ_TRK    0xe0    /* read track */
#define FDC_WRITE_TRK   0xf0    /* write track (format) */

/* Type IV additional flags */
#define FDC_IM0         0x01    /* interrupt mode 0 */
#define FDC_IM1         0x02    /* interrupt mode 1 */
#define FDC_IM2         0x04    /* interrupt mode 2 */
#define FDC_IM3         0x08    /* interrupt mode 3 */

#define FDC_MASK_TYPE_IV        (FDC_IM3|FDC_IM2|FDC_IM1|FDC_IM0)

/* Type IV commands */
#define FDC_FORCE_INT   0xd0    /* force interrupt */

typedef struct {
	UINT8	 track;
	UINT8	 sector;
	UINT8	 status;
}	SECMAP;

typedef struct {
	void   (* callback)(int event);   /* callback for IRQ status */
	UINT8	unit;				/* unit number if image_file == REAL_FDD */
	UINT8	tracks; 			/* maximum # of tracks */
	UINT8	heads;				/* maximum # of heads */
	UINT8	density;			/* FM/MFM, single / double density */
	UINT16	offset; 			/* track 0 offset */
	UINT8	first_sector_id;		/* id of first sector */
	UINT8	sec_per_track;			/* sectors per track */
	UINT16	sector_length;			/* sector length (byte) */

	UINT8	head;				/* current head # */
	UINT8	track;				/* current track # */
	UINT8	track_reg;			/* value of track register */
	UINT8	direction;			/* last step direction */
	UINT8	sector; 			/* current sector # */
	UINT8	sector_dam; 			/* current sector # to fake read DAM command */
	UINT8	data;				/* value of data register */
	UINT8	command;			/* last command written */

	UINT8	read_cmd;			/* last read command issued */
	UINT8	write_cmd;			/* last write command issued */

	UINT8	status; 			/* status register */
	UINT8	status_drq; 			/* status register data request bit */
	UINT8	status_ipl; 			/* status register toggle index pulse bit */
	UINT8	busy_count; 			/* how long to keep busy bit set */

	UINT8	buffer[6144];			/* I/O buffer (holds up to a whole track) */
	UINT8	dam_list[256][4];		/* list of data address marks while formatting */
	int 	dam_data[256];			/* offset to data inside buffer while formatting */
	int 	dam_cnt;			/* valid number of entries in the dam_list */
	UINT8	*fmt_sector_data[256];	/* pointer to data after formatting a track */
    	int     data_offset;            /* offset into I/O buffer */
	int 	data_count; 			/* transfer count from/into I/O buffer */

	const char *image_name; 		/* file name for disc image */
	void	*image_file;			/* file handle for disc image */
	int 	mode;					/* open mode == 0 read only, != 0 read/write */
	unsigned long image_size;		/* size of image file */

	UINT16	dir_sector; 			/* directory track for deleted DAM */
	UINT16	dir_length; 			/* directory length for deleted DAM */

	SECMAP	*secmap;

}	WD179X;

/* structure describing a double density track */
#define TRKSIZE_DD      6144
static UINT8 track_DD[][2] = {
	{16, 0x4e}, 	/* 16 * 4E (track lead in)				 */
	{ 8, 0x00}, 	/*	8 * 00 (pre DAM)					 */
	{ 3, 0xf5}, 	/*	3 * F5 (clear CRC)					 */

	{ 1, 0xfe}, 	/* *** sector *** FE (DAM)				 */
	{ 1, 0x80}, 	/*	4 bytes track,head,sector,seclen	 */
	{ 1, 0xf7}, 	/*	1 * F7 (CRC)						 */
	{22, 0x4e}, 	/* 22 * 4E (sector lead in) 			 */
	{12, 0x00}, 	/* 12 * 00 (pre AM) 					 */
	{ 3, 0xf5}, 	/*	3 * F5 (clear CRC)					 */
	{ 1, 0xfb}, 	/*	1 * FB (AM) 						 */
	{ 1, 0x81}, 	/*	x bytes sector data 				 */
	{ 1, 0xf7}, 	/*	1 * F7 (CRC)						 */
	{16, 0x4e}, 	/* 16 * 4E (sector lead out)			 */
	{ 8, 0x00}, 	/*	8 * 00 (post sector)				 */
	{ 0, 0x00}, 	/* end of data							 */
};

/* structure describing a single density track */
#define TRKSIZE_SD      3172
static UINT8 track_SD[][2] = {
	{16, 0xff}, 	/* 16 * FF (track lead in)				 */
	{ 8, 0x00}, 	/*	8 * 00 (pre DAM)					 */
	{ 1, 0xfc}, 	/*	1 * FC (clear CRC)					 */

	{11, 0xff}, 	/* *** sector *** 11 * FF				 */
	{ 6, 0x00}, 	/*	6 * 00 (pre DAM)					 */
	{ 1, 0xfe}, 	/*	1 * FE (DAM)						 */
	{ 1, 0x80}, 	/*	4 bytes track,head,sector,seclen	 */
	{ 1, 0xf7}, 	/*	1 * F7 (CRC)						 */
	{10, 0xff}, 	/* 10 * FF (sector lead in) 			 */
	{ 4, 0x00}, 	/*	4 * 00 (pre AM) 					 */
	{ 1, 0xfb}, 	/*	1 * FB (AM) 						 */
	{ 1, 0x81}, 	/*	x bytes sector data 				 */
	{ 1, 0xf7}, 	/*	1 * F7 (CRC)						 */
	{ 0, 0x00}, 	/* end of data							 */
};

WD179X *wd[MAX_DRIVES];
static UINT8 drv = 0;

void wd179x_CloseDiskImage(unsigned char DriveNumber)
{
	WD179X *w = wd[DriveNumber];
	
	fclose((FILE *)w->image_file);
	w->image_file = NULL;
	w->image_name = NULL;
}
void wd179x_InitDiskImage(unsigned char DriveNumber, const char *DriveImageFileName)
{
	
	WD179X *w = wd[DriveNumber];

	w->image_name = DriveImageFileName;

	// Open file images
	w->mode = 1;
				
	// Open Read/Write
	w->image_file = fopen(w->image_name, "r+b");
	if( !w->image_file )
	{
		w->mode = 0;
		// Open Read Only
		w->image_file = fopen(w->image_name,"rb");
	}

	w->track = 0;
	w->head = 0;
	w->sector = 0;
}

void wd179x_init(int active)
{
	int i;

    	for (i = 0; i < MAX_DRIVES; i++)
	{
		wd[i] = (WD179X *)malloc(sizeof(WD179X));
		if (!wd[i])
		{
			while (--i >= 0)
			{
				free(wd[i]);
				wd[i] = 0;
			}
			return;
		}
		memset(wd[i], 0, sizeof(WD179X));
		wd[i]->unit = 0;
        	wd[i]->tracks = 40;
		wd[i]->heads = 1;
		wd[i]->density = DEN_MFM_LO;
		wd[i]->offset = 0;
                wd[i]->first_sector_id = 0;
		wd[i]->sec_per_track = 17;
		wd[i]->sector_length = 256;
		wd[i]->head = 0;
		wd[i]->track = 0;
		wd[i]->track_reg = 0;
		wd[i]->direction = 1;
		wd[i]->sector = 0;
		wd[i]->data = 0;
		wd[i]->status = (active) ? STA_1_TRACK0 : 0;
		wd[i]->status_drq = 0;
		wd[i]->status_ipl = 0;
		wd[i]->busy_count = 0;
		wd[i]->data_offset = 0;
		wd[i]->data_count = 0;
		wd[i]->image_name = 0;
		wd[i]->image_size = 0;
		wd[i]->dir_sector = 0;
		wd[i]->dir_length = 0;
		wd[i]->secmap = 0;
	}
}

void wd179x_select_drive(UINT8 drive, UINT8 head, void (*callback) (int))
{
WD179X *w = wd[drive];

	if (drive < MAX_DRIVES)
	{
		drv = drive;
		w->head = head;
		w->status_ipl = STA_1_IPL;
		w->callback = callback;

		if (w->image_file)
		{
			return;
		}
	}
	w->status = STA_1_NOT_READY;
}

void wd179x_stop_drive(void)
{
	int i;

	for (i = 0; i < MAX_DRIVES; i++)
	{
		WD179X *w = wd[i];
		w->busy_count = 0;
		w->status = 0;
		w->status_drq = 0;
		if (w->callback)
			(*w->callback) (WD179X_DRQ_CLR);
		w->status_ipl = 0;
	}
}

void wd179x_read_sectormap(UINT8 drive, UINT8 * tracks, UINT8 * heads, UINT8 * sec_per_track)
{
WD179X *w = wd[drive];
SECMAP *p;
UINT8 head;

    if (!w->secmap)
		w->secmap = (SECMAP *)malloc(0x2200);
	if (!w->secmap) return;
	fseek((FILE *)w->image_file, 0, SEEK_SET);
	fread(w->secmap, 1, 0x2200, (FILE *)w->image_file);
	w->offset = 0x2200;
	w->tracks = 0;
	w->heads = 0;
	w->sec_per_track = 0;
        w->first_sector_id = 0x0ff;
	for (p = w->secmap; p->track != 0xff; p++)
	{
		if (p->track > w->tracks)
			w->tracks = p->track;

                if (p->sector < w->first_sector_id)
                        w->first_sector_id = p->sector;

		if (p->sector > w->sec_per_track)
			w->sec_per_track = p->sector;
		head = (p->status >> 4) & 1;
		if (head > w->heads)
			w->heads = head;
	}
	*tracks = w->tracks++;
	*heads = w->heads++;
	*sec_per_track = w->sec_per_track++;
}

void wd179x_set_geometry(UINT8 density, UINT8 drive, UINT8 tracks, UINT8 heads, UINT8 sec_per_track, UINT16 sector_length, UINT16 dir_sector, UINT16 dir_length, UINT8 first_sector_id)
{
WD179X *w = wd[drive];

	if (drive >= MAX_DRIVES)
	{
		return;
	}

	w->density = density;
    	w->tracks = tracks;
	w->heads = heads;
        w->first_sector_id = first_sector_id;
	w->sec_per_track = sec_per_track;
	w->sector_length = sector_length;
	w->dir_sector = dir_sector;
	w->dir_length = dir_length;

	w->image_size = w->tracks * w->heads * w->sec_per_track * w->sector_length;

        /* calculate greatest power of 2 */
        if (w->image_file == REAL_FDD)
        {
                unsigned long N = 0;
                unsigned long ShiftCount = 0;

                if (N==0)
                {
                        N = (w->sector_length);

                        while ((N & 0x080000000)==0)
                        {
                                N = N<<1;
                                ShiftCount++;
                        }

                        /* get left-shift required to shift 1 to this
                        power of 2 */

                        /* N = 0 for 128, N = 1 for 256, N = 2 for 512 ... */
                        N = (31 - ShiftCount)-7;
                 }
                 else
                 {
                       N = 1;
                 }
        }
}

/* seek to track/head/sector relative position in image file */
static int seek(WD179X * w, UINT8 t, UINT8 h, UINT8 s)
{
unsigned long offset;
SECMAP *p;
UINT8 head;

    if (w->secmap)
	{
		offset = 0x2200;
		for (p = w->secmap; p->track != 0xff; p++)
		{
			if (p->track == t && p->sector == s)
			{
				head = (p->status & 0x10) >> 4;
				if (head == h)
				{
					if (fseek((FILE *)w->image_file, offset, SEEK_SET) < 0)
					{
						return STA_1_SEEK_ERR;
					}
					return 0;
				}
			}
			//offset += 0x100;
			if (p->track==0 && head==0)
				offset += 0x80;
			else
				offset += 0x100;
		}
		return STA_1_SEEK_ERR;
	}

	/* allow two additional tracks */
    if (t >= w->tracks + 2)
	{
		return STA_1_SEEK_ERR;
	}

    if (h >= w->heads)
    {
		return STA_1_SEEK_ERR;
	}

    if (s >= (w->first_sector_id + w->sec_per_track))
	{
		return STA_2_REC_N_FND;
	}

	if ((t==0) && (h==0)) 
     		offset = (s-w->first_sector_id)*128; 
   	else
     		offset = ((t*w->heads+h)*17+s-w->first_sector_id)*256-2048; // (17*256)-(18*128)=2048

	if (offset > w->image_size)
	{
		return STA_1_SEEK_ERR;
	}

	if (fseek((FILE *)w->image_file, offset, SEEK_SET) < 0)
	{
		return STA_1_SEEK_ERR;
	}

	return 0;
}

/* return STA_2_REC_TYPE depending on relative sector */
static int deleted_dam(WD179X * w)
{
unsigned rel_sector = (w->track * w->heads + w->head) * w->sec_per_track + (w->sector-w->first_sector_id);
SECMAP *p;
UINT8 head;

	if (w->secmap)
	{
		for (p = w->secmap; p->track != 0xff; p++)
		{
			if (p->track == w->track && p->sector == w->sector)
			{
				head = (p->status >> 4) & 1;
				if (w->head == head)
					return p->status & STA_2_REC_TYPE;
			}
		}
		return STA_2_REC_N_FND;
	}
	if (rel_sector >= w->dir_sector && rel_sector < w->dir_sector + w->dir_length)
	{
		return STA_2_REC_TYPE;
	}
	return 0;
}

/* calculate CRC for data address marks or sector data */
static void calc_crc(UINT16 * crc, UINT8 value)
{
UINT8 l, h;

	l = value ^ (*crc >> 8);
	*crc = (*crc & 0xff) | (l << 8);
	l >>= 4;
	l ^= (*crc >> 8);
	*crc <<= 8;
	*crc = (*crc & 0xff00) | l;
	l = (l << 4) | (l >> 4);
	h = l;
	l = (l << 2) | (l >> 6);
	l &= 0x1f;
	*crc = *crc ^ (l << 8);
	l = h & 0xf0;
	*crc = *crc ^ (l << 8);
	l = (h << 1) | (h >> 7);
	l &= 0xe0;
	*crc = *crc ^ l;
}

/* read the next data address mark */
static void read_dam(WD179X * w)
{
UINT16 crc = 0xffff;

	w->data_offset = 0;
	w->data_count = 6;
	w->buffer[0] = w->track;
	w->buffer[1] = w->head;
	w->buffer[2] = w->sector_dam;
	w->buffer[3] = w->sector_length >> 8;
	calc_crc(&crc, w->buffer[0]);
	calc_crc(&crc, w->buffer[1]);
	calc_crc(&crc, w->buffer[2]);
	calc_crc(&crc, w->buffer[3]);
	w->buffer[4] = crc & 255;
	w->buffer[5] = crc / 256;
	if (++w->sector_dam == w->sec_per_track)
                w->sector_dam = w->first_sector_id;
	w->status_drq = STA_2_DRQ;
	if (w->callback)
		(*w->callback) (WD179X_DRQ_SET);
	w->status = STA_2_DRQ | STA_2_BUSY;
	w->busy_count = 50;
}

/* read a sector */
static void read_sector(WD179X * w)
{
    w->data_offset = 0;
	w->data_count = w->sector_length;

	/* if a track was just formatted */
	if (w->dam_cnt)
	{
		int i;
		for (i = 0; i < w->dam_cnt; i++)
		{
			if (w->track == w->dam_list[i][0] &&
				w->head == w->dam_list[i][1] &&
				w->sector == w->dam_list[i][2])
			{
				w->data_offset = w->dam_data[i];
				return;
			}
		}
		/* sector not found, now the track buffer is invalid */
		w->dam_cnt = 0;
	}

    /* if this is the real thing */
    if (w->image_file == REAL_FDD)
    {
	int tries = 3;
		do {
			//w->status = osd_fdc_get_sector(w->track, w->head, w->head, w->sector, w->buffer);
			tries--;
		} while (tries && (w->status & (STA_2_REC_N_FND | STA_2_CRC_ERR | STA_2_LOST_DAT)));
		/* no error bits set ? */
		if ((w->status & (STA_2_REC_N_FND | STA_2_CRC_ERR | STA_2_LOST_DAT)) == 0)
		{
			/* start transferring data to the emulation now */
			w->status_drq = STA_2_DRQ;
			if (w->callback)
				(*w->callback) (WD179X_DRQ_SET);
			w->status |= STA_2_DRQ | STA_2_BUSY;
        }
        return;
    }
	else
	if (fread(w->buffer, 1, w->sector_length, (FILE *)w->image_file) != w->sector_length)
	{
		w->status = STA_2_LOST_DAT;
		return;
	}

	w->status_drq = STA_2_DRQ;
	if (w->callback)
		(*w->callback) (WD179X_DRQ_SET);
	w->status = STA_2_DRQ | STA_2_BUSY;
	w->busy_count = 0;
}

/* read an entire track */
static void read_track(WD179X * w)
{
UINT8 *psrc;					   /* pointer to track format structure */
UINT8 *pdst;					   /* pointer to track buffer */
int cnt;					   /* number of bytes to fill in */
UINT16 crc;					   /* id or data CRC */
UINT8 d;						   /* data */
UINT8 t = w->track;			   /* track of DAM */
UINT8 h = w->head;			   /* head of DAM */
UINT8 s = w->sector_dam;		   /* sector of DAM */
UINT16 l = w->sector_length;	   /* sector length of DAM */
int i;

	for (i = 0; i < w->sec_per_track; i++)
	{
		w->dam_list[i][0] = t;
		w->dam_list[i][1] = h;
		w->dam_list[i][2] = i;
		w->dam_list[i][3] = l >> 8;
	}

	pdst = w->buffer;

	if (w->density)
	{
		psrc = track_DD[0];	   /* double density track format */
		cnt = TRKSIZE_DD;
	}
	else
	{
		psrc = track_SD[0];	   /* single density track format */
		cnt = TRKSIZE_SD;
	}

	while (cnt > 0)
	{
		if (psrc[0] == 0)	   /* no more track format info ? */
		{
			if (w->dam_cnt < w->sec_per_track) /* but more DAM info ? */
			{
				if (w->density)/* DD track ? */
					psrc = track_DD[3];
				else
					psrc = track_SD[3];
			}
		}

		if (psrc[0] != 0)	   /* more track format info ? */
		{
			cnt -= psrc[0];	   /* subtract size */
			d = psrc[1];

			if (d == 0xf5)	   /* clear CRC ? */
			{
				crc = 0xffff;
				d = 0xa1;	   /* store A1 */
			}

			for (i = 0; i < *psrc; i++)
				*pdst++ = d;   /* fill data */

			if (d == 0xf7)	   /* store CRC ? */
			{
				pdst--;		   /* go back one byte */
				*pdst++ = crc & 255;	/* put CRC low */
				*pdst++ = crc / 256;	/* put CRC high */
				cnt -= 1;	   /* count one more byte */
			}
			else if (d == 0xfe)/* address mark ? */
			{
				crc = 0xffff;   /* reset CRC */
			}
			else if (d == 0x80)/* sector ID ? */
			{
				pdst--;		   /* go back one byte */
				t = *pdst++ = w->dam_list[w->dam_cnt][0]; /* track number */
				h = *pdst++ = w->dam_list[w->dam_cnt][1]; /* head number */
				s = *pdst++ = w->dam_list[w->dam_cnt][2]; /* sector number */
				l = *pdst++ = w->dam_list[w->dam_cnt][3]; /* sector length code */
				w->dam_cnt++;
				calc_crc(&crc, t);	/* build CRC */
				calc_crc(&crc, h);	/* build CRC */
				calc_crc(&crc, s);	/* build CRC */
				calc_crc(&crc, l);	/* build CRC */
				l = (l == 0) ? 128 : l << 8;
			}
			else if (d == 0xfb)// data address mark ?
			{
				crc = 0xffff;   // reset CRC
			}
			else if (d == 0x81)// sector DATA ?
			{
				pdst--;		   /* go back one byte */
				if (seek(w, t, h, s) == 0)
				{
					if (fread(pdst, 1, l, (FILE *)w->image_file) != l)
					{
						w->status = STA_2_CRC_ERR;
						return;
					}
				}
				else
				{
					w->status = STA_2_REC_N_FND;
					return;
				}
				for (i = 0; i < l; i++)	// build CRC of all data
					calc_crc(&crc, *pdst++);
				cnt -= l;
			}
			psrc += 2;
		}
		else
		{
			*pdst++ = 0xff;	   /* fill track */
			cnt--;			   /* until end */
		}
	}

	w->data_offset = 0;
	w->data_count = (w->density) ? TRKSIZE_DD : TRKSIZE_SD;

	w->status_drq = STA_2_DRQ;
	if (w->callback)
		(*w->callback) (WD179X_DRQ_SET);
	w->status |= STA_2_DRQ | STA_2_BUSY;
	w->busy_count = 0;
}

/* write a sector */
static void write_sector(WD179X * w)
{
	if (w->image_file == REAL_FDD)
	{
		return;
	}
    if (w->mode == 0)
	{
		w->status = STA_2_WRITE_PRO;
	}
	else
	{
		w->status = seek(w, w->track, w->head, w->sector);
		if (w->status == 0)
		{
			if (fwrite(w->buffer, 1, w->data_offset, (FILE *)w->image_file) != w->data_offset)
				w->status = STA_2_LOST_DAT;
		}
	}
}

/* write an entire track by extracting the sectors */
static void write_track(WD179X * w)
{
UINT8 *f;
int cnt;

	w->dam_cnt = 0;
    if (w->image_file != REAL_FDD && w->mode == 0)
    {
        w->status = STA_2_WRITE_PRO;
        return;
    }

	memset(w->dam_list, 0xff, sizeof(w->dam_list));
	memset(w->dam_data, 0x00, sizeof(w->dam_data));

	f = w->buffer;
    cnt = (w->density) ? TRKSIZE_DD : TRKSIZE_SD;

	do
	{
		while ((--cnt > 0) && (*f != 0xfe))	/* start of DAM ?? */
			f++;

		if (cnt > 4)
		{
		int seclen;
			cnt -= 5;
			f++;			   /* skip FE */
			w->dam_list[w->dam_cnt][0] = *f++;	  /* copy track number */
			w->dam_list[w->dam_cnt][1] = *f++;	  /* copy head number */
			w->dam_list[w->dam_cnt][2] = *f++;	  /* copy sector number */
			w->dam_list[w->dam_cnt][3] = *f++;	  /* copy sector length */
			/* sector length in bytes */
			seclen = 128 << w->dam_list[w->dam_cnt][3];
			/* search start of DATA */
			while ((--cnt > 0) && (*f != 0xf9) && (*f != 0xfa) && (*f != 0xfb))
				f++;
			if (cnt > seclen)
			{
				cnt--;
				/* skip data address mark */
                f++;
                /* set pointer to DATA to later write the sectors contents */
				w->dam_data[w->dam_cnt] = (int)(f - w->buffer);
				w->dam_cnt++;
				f += seclen;
				cnt -= seclen;
			}
        }
	} while (cnt > 0);

	if (w->image_file == REAL_FDD)
	{
		w->status = 0;
    }
	else
	{
		/* now put all sectors contained in the format buffer */
		for (cnt = 0; cnt < w->dam_cnt; cnt++)
		{
			w->status = seek(w, w->track, w->head, w->dam_list[cnt][2]);
			if (w->status == 0)
			{
				if (fwrite(&w->buffer[w->dam_data[cnt]],1, w->sector_length, (FILE *)w->image_file) != w->sector_length)
				{
					w->status = STA_2_LOST_DAT;
					return;
				}
			}
		}
	}
}


/* read the FDC status register. This clears IRQ line too */
UINT8 wd179x_status_r(void)
{
WD179X *w = wd[drv];
int result = w->status;

	if (w->callback)
		(*w->callback) (WD179X_IRQ_CLR);
	if (w->busy_count)
	{
		if (!--w->busy_count)
			w->status &= ~STA_1_BUSY;
	}
/* eventually toggle index pulse bit */
	w->status ^= w->status_ipl;
/* eventually set data request bit */
	w->status |= w->status_drq;

	return result;
}

/* read the FDC track register */
UINT8 wd179x_track_r(void)
{
WD179X *w = wd[drv];

	return w->track_reg;
}

/* read the FDC sector register */
UINT8 wd179x_sector_r(void)
{
WD179X *w = wd[drv];

	return w->sector;
}

/* read the FDC data register */
UINT8 wd179x_data_r(void)
{
WD179X *w = wd[drv];

	if (w->data_count > 0)
	{
		w->status &= ~STA_2_DRQ;
		if (--w->data_count <= 0)
		{
			/* clear busy bit */
			w->status &= ~STA_2_BUSY;
			/* no more setting of data request bit */
			w->status_drq = 0;
			if (w->callback)
				(*w->callback) (WD179X_DRQ_CLR);
			if (w->image_file != REAL_FDD)
			{
				/* read normal or deleted data address mark ? */
				w->status |= deleted_dam(w);
			}
			/* generate an IRQ */
			if (w->callback)
				(*w->callback) (WD179X_IRQ_SET);
		}
		w->data = w->buffer[w->data_offset++];
	}
	return w->data;
}

/* write the FDC command register */
void wd179x_command_w(UINT8 data)
{
WD179X *w = wd[drv];

	if ((data | 1) == 0xff)	   /* change single/double density ? */
	{
		/* only supports FM/LO and MFM/LO */
		w->density = (data & 1) ? DEN_MFM_LO : DEN_FM_LO;
		return;
	}

	if ((data & ~FDC_MASK_TYPE_IV) == FDC_FORCE_INT)
	{
		w->data_count = 0;
		w->data_offset = 0;
		w->status &= ~(STA_2_DRQ | STA_2_BUSY);
		w->status_drq = 0;
		if (w->callback)
			(*w->callback) (WD179X_DRQ_CLR);
		w->status_ipl = 0;
		if (w->callback)
			(*w->callback) (WD179X_IRQ_CLR);
		w->busy_count = 0;
		return;
	}

	if (data & 0x80)
	{
		w->status_ipl = 0;

		if ((data & ~FDC_MASK_TYPE_II) == FDC_READ_SEC)
		{
			w->read_cmd = data;
            		w->command = data & ~FDC_MASK_TYPE_II;
			w->status = seek(w, w->track, w->head, w->sector);
			if (w->status == 0)
				read_sector(w);
			return;
		}

		if ((data & ~FDC_MASK_TYPE_II) == FDC_WRITE_SEC)
		{
			w->write_cmd = data;
			w->command = data & ~FDC_MASK_TYPE_II;
			w->data_offset = 0;
			w->data_count = w->sector_length;
			w->status_drq = STA_2_DRQ;
			if (w->callback)
				(*w->callback) (WD179X_DRQ_SET);
			w->status = STA_2_DRQ | STA_2_BUSY;
			w->busy_count = 0;
			return;
		}

		if ((data & ~FDC_MASK_TYPE_III) == FDC_READ_TRK)
		{
			w->command = data & ~FDC_MASK_TYPE_III;
			w->status = seek(w, w->track, w->head, w->sector);
			if (w->status == 0)
				read_track(w);
			return;
		}

		if ((data & ~FDC_MASK_TYPE_III) == FDC_WRITE_TRK)
		{
			w->command = data & ~FDC_MASK_TYPE_III;
			w->data_offset = 0;
			w->data_count = (w->density) ? TRKSIZE_DD : TRKSIZE_SD;
			w->status_drq = STA_2_DRQ;
			if (w->callback)
				(*w->callback) (WD179X_DRQ_SET);
			w->status = STA_2_DRQ | STA_2_BUSY;
			w->busy_count = 0;
			return;
		}

		if ((data & ~FDC_MASK_TYPE_III) == FDC_READ_DAM)
		{
			w->status = seek(w, w->track, w->head, w->sector);
			if (w->status == 0)
				read_dam(w);
			return;
		}

        return;
	}


	if ((data & ~FDC_MASK_TYPE_I) == FDC_RESTORE)
	{
		/* simulate seek time busy signal */
		w->busy_count = w->track * ((data & FDC_STEP_RATE) + 1);
		/* if it is a real floppy, issue a recal command */
        if (w->image_file == REAL_FDD)
		{
			w->track = 0;//osd_fdc_recal(&w->track);
		}
		else
		{
			w->track = 0;	 /* set track number 0 */
		}
		w->track_reg = w->track;
    }

	if ((data & ~FDC_MASK_TYPE_I) == FDC_SEEK)
	{
	UINT8 newtrack = w->data;
		/* if it is a real floppy, issue a seek command */
        /* simulate seek time busy signal */
		w->busy_count = abs(newtrack - w->track) * ((data & FDC_STEP_RATE) + 1);
        if (w->image_file == REAL_FDD)
			w->track = newtrack;//osd_fdc_seek(newtrack, &w->track);
		else
			w->track = newtrack;	/* get track number from data register */
		w->track_reg = w->track;
	}

	if ((data & ~(FDC_STEP_UPDATE | FDC_MASK_TYPE_I)) == FDC_STEP)
	{
		/* if it is a real floppy, issue a step command */
        /* simulate seek time busy signal */
		w->busy_count = ((data & FDC_STEP_RATE) + 1);
		if (w->image_file == REAL_FDD)
            		w->track += w->direction;//osd_fdc_step(w->direction, &w->track);
		else
			w->track += w->direction;	/* adjust track number */
	}

	if ((data & ~(FDC_STEP_UPDATE | FDC_MASK_TYPE_I)) == FDC_STEP_IN)
	{
        w->direction = +1;
		/* simulate seek time busy signal */
		w->busy_count = ((data & FDC_STEP_RATE) + 1);
		/* if it is a real floppy, issue a step command */
        if (w->image_file == REAL_FDD)
			w->track += w->direction;//osd_fdc_step(w->direction, &w->track);
		else
			w->track += w->direction;	/* adjust track number */
	}

	if ((data & ~(FDC_STEP_UPDATE | FDC_MASK_TYPE_I)) == FDC_STEP_OUT)
	{
        w->direction = -1;
		/* simulate seek time busy signal */
		w->busy_count = ((data & FDC_STEP_RATE) + 1);
		/* if it is a real floppy, issue a step command */
        if (w->image_file == REAL_FDD)
			w->track += w->direction;//osd_fdc_step(w->direction, &w->track);
		else
			w->track += w->direction;	/* adjust track number */
	}

	if (w->busy_count)
		w->status = STA_1_BUSY;

/* toggle index pulse at read */
	w->status_ipl = STA_1_IPL;

	if (w->track >= w->tracks)
		w->status |= STA_1_SEEK_ERR;

	if (w->track == 0)
		w->status |= STA_1_TRACK0;

    if (w->mode == 0)
        w->status |= STA_1_WRITE_PRO;

    if (data & FDC_STEP_UPDATE)
		w->track_reg = w->track;

	if (data & FDC_STEP_HDLOAD)
		w->status |= STA_1_HD_LOADED;

	if (data & FDC_STEP_VERIFY)
		if (w->track_reg != w->track)
			w->status |= STA_1_SEEK_ERR;
}

/* write the FDC track register */
void wd179x_track_w(UINT8 data)
{
WD179X *w = wd[drv];
	w->track = w->track_reg = data;
}

/* write the FDC sector register */
void wd179x_sector_w(UINT8 data)
{
WD179X *w = wd[drv];

	w->sector = data;
}

/* write the FDC data register */
void wd179x_data_w(UINT8 data)
{
WD179X *w = wd[drv];

	if (w->data_count > 0)
	{
		w->buffer[w->data_offset++] = data;
		if (--w->data_count <= 0)
		{
			w->status_drq = 0;
			if (w->callback)
				(*w->callback) (WD179X_DRQ_CLR);
			if (w->command == FDC_WRITE_TRK)
				write_track(w);
			else
				write_sector(w);
			w->data_offset = 0;
			if (w->callback)
				(*w->callback) (WD179X_IRQ_SET);
		}
	}
	w->data = data;
}

/*************************************************************/
/* Function: svi_LoadDisk                                    */
/* Purpose:  Try to load a disk image                        */
/*************************************************************/
unsigned char svi_LoadDisk(unsigned char disk, char *filename)
{
	unsigned char status = 0;
	char s[256];
	FILE *f;
	int fsize;
	
	if (strstr(filename,".\\")!=filename)
	{
		strcpy(s,dskPath);
		strcat(s,filename);
		if (f=fopen(s,"rb"))
		{
			strcpy(filename,s);
			fclose(f);
		}
	}
	if (!(f=fopen(filename,"rb")))
		return 0;
	fseek(f,0,SEEK_END);
	fsize=ftell(f);
	switch (fsize)
	{
		case 346112 : svi_disk_heads[disk] = 2;
			      status=1;
			      break;
		case 172032 : svi_disk_heads[disk] = 1;
			      status=1;
			      break;
	}
	fclose(f);
	return status;
}

/*************************************************************/
/* Function: svi_fdc_callback                                */
/* Purpose:  Callback routine for the FDC.                   */
/*************************************************************/
void svi_fdc_callback(int param)
{
	switch( param )
	{
		case WD179X_IRQ_CLR:
			fdc_status &= ~0x80;
	        	break;
		case WD179X_IRQ_SET:
			fdc_status |= 0x80;
        		break;
		case WD179X_DRQ_CLR:
			fdc_status &= ~0x40;
			break;
		case WD179X_DRQ_SET:
			fdc_status |= 0x40;
	        	break;
	}
}

/*************************************************************/
/* Function: svi_fdc_disk_motor                              */
/* Purpose:  Floppy disk and motor select.                   */
/*************************************************************/
void svi_fdc_disk_motor(unsigned char data)
{
	unsigned char seldrive = 255;
	
	if (data == 0)
	{
		wd179x_stop_drive();
		return;
	}
	if (data & 2)
	{
		seldrive=1;
	}
	if (data & 1)
	{
		seldrive=0;
	}
	if (seldrive > 3) return;
	fdc_drive = seldrive;
	wd179x_select_drive(fdc_drive, fdc_head, svi_fdc_callback);
}


/*************************************************************/
/* Function: svi_fdc_density_side                            */
/* Purpose:  Floppy density and head select.                 */
/*************************************************************/
void svi_fdc_density_side(unsigned char data)
{
	unsigned char sectors_track;
	unsigned short int sector_size;

	if (data & 2)
		fdc_head = 1;
	else
		fdc_head = 0;

	if (data & 1)
	{
		fdc_density = DEN_FM_LO;
		sectors_track =  18;
		sector_size = 128;
	}
	else
	{
		fdc_density = DEN_MFM_LO;
		sectors_track =  17;
		sector_size = 256;
	}
	wd179x_set_geometry(fdc_density, fdc_drive, svi_disk_tracks, svi_disk_heads[fdc_drive], sectors_track, sector_size, 0, 0, 1);
}

#endif

#define EVENT_CLOCK	0

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 13, eb = (e) >> 13; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x2000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x2000 * (i - sb); \
		} \
	} \
}

bool MEMORY_EX::load_cart(const _TCHAR *file_path)
{
	bool result = false;
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(rom, 0xff, sizeof(rom));
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fread(r12, sizeof(r12), 1);
		SET_BANK(0x0000, 0x7fff, wdmy, rom);
		SET_BANK(0x8000, 0xffff, ram, ram);
		fio->Fclose();
		result = true;
	}
	delete fio;
	return result;
}

void MEMORY_EX::open_cart(const _TCHAR *file_path)
{
	if(load_cart(file_path)) {
		inserted = true;
	}
}

void MEMORY_EX::close_cart()
{
	SET_BANK(0x0000, 0x7fff, wdmy, bio);
	SET_BANK(0x8000, 0xffff, ram, ram);
	inserted = false;
}

// memory bus
void MEMORY_EX::initialize()
{
	svi_UseDisk = 0;
	memset(bio, 0xff, sizeof(bio));
	memset(rdmy, 0xff, sizeof(rdmy));
	memset(wdmy, 0xff, sizeof(wdmy));
	FILEIO* fio = new FILEIO();
	if((fio->Fopen(create_local_path(_T("SVI318.ROM")), FILEIO_READ_BINARY)) ||
	   (fio->Fopen(create_local_path(_T("SVI328.ROM")), FILEIO_READ_BINARY)) ||
	   (fio->Fopen(create_local_path(_T("SVI328a.ROM")), FILEIO_READ_BINARY))) {
		fio->Fread(bio, sizeof(bio), 1);
		fio->Fclose();
	}
	delete fio;
#if defined(FDD_PATCH_SLOT)
	wd179x_init(1);
	for(int i = 0; i < MAX_DRIVE; i++) {
		disk[i] = new DISK(emu);
		disk[i]->set_device_name(_T("%s/Disk #%d"), this_device_name, i + 1);
		disk[i]->drive_type = DRIVE_TYPE_2DD;
	}
#endif
	close_cart();
	close_tape();
}

#if defined(FDD_PATCH_SLOT)
void MEMORY_EX::release()
{
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(disk[i]) {
			disk[i]->close();
			delete disk[i];
		}
	}
}
#endif

void MEMORY_EX::reset()
{
	if (!inserted) {
		memset(rom, 0xff, sizeof(rom));
		memset(r12, 0xff, sizeof(r12));
	}
	memset(ram, 0, sizeof(ram));
	memset(r21, 0, sizeof(r21));
	memset(r22, 0, sizeof(r22));
	memset(r31, 0, sizeof(r31));
	memset(r32, 0, sizeof(r32));
	memset(tapedata, 0, sizeof(tapedata));
}

void MEMORY_EX::write_data8(uint32_t addr, uint32_t data)
{
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t MEMORY_EX::read_io8(uint32_t addr)
{
	unsigned char port = addr;
	uint32_t ret=0xff;
	switch (port)
	{
		case 0x30 : if (svi_UseDisk == 1)
				ret=wd179x_status_r();
			    break;

		case 0x31 : if (svi_UseDisk == 1)
				ret=wd179x_track_r();
			    break;

		case 0x32 : if (svi_UseDisk == 1)
				ret=wd179x_sector_r();
			    break;

		case 0x33 : if (svi_UseDisk == 1)
				ret=wd179x_data_r();
			    break;

		case 0x34 : if (svi_UseDisk == 1)
				ret=fdc_status;
			    break;
		default:
				ret=strig;
	}

	return ret;
}

void MEMORY_EX::write_io8(uint32_t addr, uint32_t data)
{
	// Write the value to the appropriate port
	unsigned char port = addr;
	unsigned char value = data;
	switch (port)
	{
		case 0x30 : if (svi_UseDisk == 1)
			    {
				wd179x_command_w(value);
			    	if ((value & ~FDC_MASK_TYPE_I) == FDC_RESTORE)
				    	fdc_status |= 0x80;
			    }
			    break;
	
		case 0x31 : if (svi_UseDisk == 1)
				wd179x_track_w(value);
			    break;

		case 0x32 : if (svi_UseDisk == 1)
				wd179x_sector_w(value);
			    break;

		case 0x33 : if (svi_UseDisk == 1)
				wd179x_data_w(value);
			    break;

		case 0x34 : if (svi_UseDisk == 1)
				svi_fdc_disk_motor(value);
			    break;

		case 0x38 : if (svi_UseDisk == 1)
				svi_fdc_density_side(value);
			    break;
		default:
				strig &= 0xE0;
				data &= 0x1F;
				strig |= data;
	}
}

uint32_t MEMORY_EX::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	if (!play)
		return rbank[addr >> 13][addr & 0x1fff];
	if (addr==0x210a) {
		count=0;
		done=0;
		return 0xAF;
	}
	if (addr==0x210b) {
		strig &= 0x7F;
		return 0x00;
	}
	if (addr==0x210c)
		return 0xc9;
	if (addr==0x21a9)
		return 0x3e;
	if (addr==0x21aa) {
		while ((tapePos<tapeLen)&&(!done))
		{
			switch (tapedata[tapePos++])
			{
				case 0x7F : done=count>=10; break;
				case 0x55 : count++; break;
				default		: count=0; break;
			}
		}
		return tapedata[tapePos++];
	}
	if (addr==0x21ab)
			return 0xc9;
	return rbank[addr >> 13][addr & 0x1fff];
}

void MEMORY_EX::write_signal(int id, uint32_t data, uint32_t mask) {
	if (data & 16) {
		if (data & 4) {
			SET_BANK(0x8000, 0xffff, ram, ram);
		} else {
			if (data % 2 == 1) {
				SET_BANK(0x8000, 0xffff, r22, r22);
			} else {
				SET_BANK(0x8000, 0xffff, wdmy,r12);
			}
		}
	} else {
		SET_BANK(0x8000, 0xffff, r32, r32);
	}
	data &= 0x0f;
	if (data==3 || data==7) {
		SET_BANK(0x0000, 0x7fff, r31, r31);
	} else if (data==13) {
		SET_BANK(0x0000, 0x7fff, r21, r21);
	} else {
		if (data % 2 == 1) {
			SET_BANK(0x0000, 0x7fff, wdmy,bio);
		} else {
			SET_BANK(0x0000, 0x7fff, wdmy,rom);
		}
	}
	return;
}

uint32_t MEMORY_EX::fetch_op(uint32_t addr, int* wait)
{
	*wait = 1;
	return read_data8(addr);
}

bool MEMORY_EX::play_tape(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(tapedata, sizeof(tapedata), 1);
		tapeLen = fio->Ftell();
		fio->Fclose();
		play = true;
	}
	delete fio;
	strig = 0xBF;
	tapePos=0;
	return true;
}

void MEMORY_EX::close_tape()
{
	memset(tapedata, 0, sizeof(tapedata));
	strig = 0xFF;
	tapePos=0;
	play = false;
}


#if defined(FDD_PATCH_SLOT)

void MEMORY_EX::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < MAX_DRIVE) {
//		disk[drv]->open(file_path, bank);
		if (svi_LoadDisk(drv, (char *)file_path) > 0)
		{
			wd179x_InitDiskImage(drv, tchar_to_char(file_path));
			disk[drv]->inserted = true;
			svi_UseDisk = 1;
		}
	}
}

void MEMORY_EX::close_disk(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]->inserted) {
//		disk[drv]->close();
		wd179x_CloseDiskImage(drv);
		disk[drv]->inserted = false;
		svi_UseDisk = 0; 
	}
}

bool MEMORY_EX::is_disk_inserted(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->inserted;
	}
	return false;
}

void MEMORY_EX::is_disk_protected(int drv, bool value)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->write_protected = value;
	}
}

bool MEMORY_EX::is_disk_protected(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->write_protected;
	}
	return false;
}

#endif

#define STATE_VERSION	1

bool MEMORY_EX::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(inserted);
	// post process
	if(loading) {
		if(inserted) {
			SET_BANK(0x0000, 0x7fff, wdmy, rom);
			SET_BANK(0x8000, 0xffff, ram, ram);
		} else {
			SET_BANK(0x0000, 0x7fff, wdmy, bio);
			SET_BANK(0x8000, 0xffff, ram, ram);
		}
	}
	return true;
}
