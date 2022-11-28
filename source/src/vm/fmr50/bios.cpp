/*
	FUJITSU FMR-30 Emulator 'eFMR-30'
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.10.06 -

	[ bios ]
*/

#include "bios.h"
#include "../disk.h"
#include "../harddisk.h"

// regs
#define AX	regs[0]
#define CX	regs[1]
#define DX	regs[2]
#define BX	regs[3]
#define SP	regs[4]
#define BP	regs[5]
#define SI	regs[6]
#define DI	regs[7]

#define AL	regs8[0]
#define AH	regs8[1]
#define CL	regs8[2]
#define CH	regs8[3]
#define DL	regs8[4]
#define DH	regs8[5]
#define BL	regs8[6]
#define BH	regs8[7]
#define SPL	regs8[8]
#define SPH	regs8[9]
#define BPL	regs8[10]
#define BPH	regs8[11]
#define SIL	regs8[12]
#define SIH	regs8[13]
#define DIL	regs8[14]
#define DIH	regs8[15]

// sregs
#define ES	sregs[0]
#define CS	sregs[1]
#define SS	sregs[2]
#define DS	sregs[3]

// error
#define ERR_FDD_NOTREADY	1
#define ERR_FDD_PROTECTED	2
#define ERR_FDD_DELETED		4
#define ERR_FDD_NOTFOUND	8
#define ERR_FDD_CRCERROR	0x10
#define ERR_SCSI_NOTREADY	1
#define ERR_SCSI_PARAMERROR	2
#define ERR_SCSI_NOTCONNECTED	4

#if defined(_FMR30)
// FMR-30
#define CMOS_SIZE	0x2000
#define VRAM_SIZE	0x20000
#define IPL_SIZE	0x10000
#define IPL_ID		'2'
#elif defined(_FMR50)
// FMR-50
#define CMOS_SIZE	0x800
#define VRAM_SIZE	0x40000
#define IPL_SIZE	0x4000
#define IPL_ID		'1'
#elif defined(_FMR60)
// FMR-60
#define CMOS_SIZE	0x800
#define VRAM_SIZE	0x80000
#define IPL_SIZE	0x4000
#define IPL_ID		'1'
#endif

#define BLOCK_SIZE	512

static const int iotable[][2] = {
#ifdef _FMR30
	{0x0100, 0x19},	// pic
	{0x0101, 0x40},
	{0x0101, 0x80},
	{0x0101, 0x01},
	{0x0101, 0xff},
	{0x0108, 0x19},
	{0x010a, 0x48},
	{0x010a, 0x07},
	{0x010a, 0x01},
	{0x010a, 0xff},
	{0x0042, 0x00},	// timer
	{0x0133, 0x30},
	{0x0130, 0xa0},
	{0x0130, 0x86},
	{0x000b, 0x02},	// sio
	{0x0009, 0x00},
	{0x0009, 0x50},
	{0x0009, 0x7f},
	{0x0009, 0x15},
	{0x0013, 0x02},
	{0x001d, 0x02},	// memory
	{0x001e, 0x00},
	{0x0040, 0x9f},	// psg
	{0x0040, 0xbf},
	{0x0040, 0xdf},
	{0x0040, 0xff},
	{0x0300, 0x01},	// lcdc
	{0x0302, 0x50},
	{0x0300, 0x09},
	{0x0302, 0x0f},
	{0x0300, 0x0a},
	{0x0302, 0x20},
	{0x0300, 0x0b},
	{0x0302, 0x0d},
	{0x0300, 0x0c},
	{0x0302, 0x00},
	{0x0300, 0x0d},
	{0x0302, 0x00},
	{0x0300, 0x0e},
	{0x0302, 0x00},
	{0x0300, 0x0f},
	{0x0302, 0x00},
	{0x0300, 0x11},
	{0x0302, 0xc7},
	{0x0300, 0x1d},
	{0x0302, 0x00},
	{0x0308, 0x63},
	{0x0309, 0x00},
	{0x030a, 0x00},
#else
	{0x0060, 0x00},	// timer
	{0x0604, 0x00},	// keyboard
	{0x0000, 0x19},	// pic
	{0x0002, 0x40},
	{0x0002, 0x80},
	{0x0002, 0x0d},
	{0x0002, 0xfe},
	{0x0010, 0x19},
	{0x0012, 0x48},
	{0x0012, 0x87},
	{0x0012, 0x09},
	{0x0012, 0xff},
	{0x0000, 0x20},
	{0x0046, 0x36},	// pit
	{0x0040, 0x00},
	{0x0040, 0x78},
	{0x0404, 0x00},	// memory
	{0x0500, 0x00},	// crtc
	{0x0502, 0x35},
	{0x0500, 0x01},
	{0x0502, 0x28},
	{0x0500, 0x02},
	{0x0502, 0x2c},
	{0x0500, 0x03},
	{0x0502, 0x04},
	{0x0500, 0x04},
	{0x0502, 0x1a},
	{0x0500, 0x05},
	{0x0502, 0x08},
	{0x0500, 0x06},
	{0x0502, 0x19},
	{0x0500, 0x07},
	{0x0502, 0x19},
	{0x0500, 0x08},
	{0x0502, 0x00},
	{0x0500, 0x09},
	{0x0502, 0x0f},
	{0x0500, 0x0a},
	{0x0502, 0x20},
	{0x0500, 0x0b},
	{0x0502, 0x1e},
	{0x0500, 0x0c},
	{0x0502, 0x00},
	{0x0500, 0x0d},
	{0x0502, 0x00},
	{0x0500, 0x0e},
	{0x0502, 0x00},
	{0x0500, 0x0f},
	{0x0502, 0x00},
	{0x0500, 0x10},
	{0x0502, 0x00},
	{0x0500, 0x11},
	{0x0502, 0x00},
	{0x0500, 0x1e},
	{0x0502, 0x00},
	{0x0500, 0x1f},
	{0x0502, 0x00},
	{0xfd98, 0x00},	// palette
	{0xfd99, 0x01},
	{0xfd9a, 0x02},
	{0xfd9b, 0x03},
	{0xfd9c, 0x04},
	{0xfd9d, 0x05},
	{0xfd9e, 0x06},
	{0xfd9f, 0x07},
	{0xfda0, 0x0f},	// video
#endif
	{-1, -1}
};

// cmos: $000-
static const uint8_t cmos_t[] = {
#ifdef _FMR30
	0x01,0xff,0x42,0x4f,0x4f,0x54,0xa8,0x00,0x40,0x00,0x01,0xfe,0x53,0x45,0x54,0x55,
	0xe8,0x00,0x00,0x01,0x01,0xfd,0x4c,0x4f,0x47,0x20,0xe8,0x01,0x10,0x03,0x01,0xfc,
	0x4f,0x41,0x53,0x59,0xf8,0x04,0x20,0x00,0x01,0xfb,0x44,0x45,0x42,0x20,0x18,0x05,
	0x00,0x01,0x01,0xfa,0x44,0x45,0x53,0x4b,0x18,0x06,0x32,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x4a,0x06,0x7b,0x19,0x97,0x62,0x79,0x41
#else
	0x01,0xff,0x42,0x4f,0x4f,0x54,0xa8,0x00,0x40,0x00,0x01,0xfe,0x53,0x45,0x54,0x55,
	0xe8,0x00,0x00,0x01,0x01,0xfd,0x4c,0x4f,0x47,0x20,0xe8,0x01,0x10,0x03,0x01,0xfc,
	0x4f,0x41,0x53,0x59,0xf8,0x04,0x20,0x00,0x01,0xfb,0x58,0x45,0x4e,0x49,0x18,0x05,
	0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//	0x28,0x05,0x99,0x02,0xe1,0xe1,0x79,0x41
	0x28,0x05,0x99,0x02,0x00,0x00,0x79,0x41
#endif
};
// FMR-30: cmos $1fd0-
// FMR-50: cmos $7d0-
static const uint8_t cmos_b[] = {
#ifdef _FMR30
	0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x7f,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
#else
	0x00,0x00,0x01,0x02,0x03,0x04,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
#endif
};

// boot message
static const uint8_t msg_c[] = {
	0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,
	0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,
	0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,
	0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07
};

// 'システムをセットしてください'
static const uint8_t msg_k[] = {
	0x25,0x37,0x00,0x00,0x25,0x39,0x00,0x00,0x25,0x46,0x00,0x00,0x25,0x60,0x00,0x00,
	0x24,0x72,0x00,0x00,0x25,0x3b,0x00,0x00,0x25,0x43,0x00,0x00,0x25,0x48,0x00,0x00,
	0x24,0x37,0x00,0x00,0x24,0x46,0x00,0x00,0x24,0x2f,0x00,0x00,0x24,0x40,0x00,0x00,
	0x24,0x35,0x00,0x00,0x24,0x24,0x00,0x00,0x21,0x21,0x00,0x00
};

void BIOS::initialize()
{
	// to increment timeout counter
	register_frame_event(this);
}

void BIOS::reset()
{
	for(int i = 0; i < MAX_DRIVE; i++) {
		access_fdd[i] = false;
		drive_mode1[i] = 0x03;	// MFM, 2HD, 1024B
		drive_mode2[i] = 0x208;	// 2 Heads, 8 sectors
	}
	for(int i = 0; i < USE_HARD_DISK; i++) {
		if(harddisk[i] != NULL && harddisk[i]->mounted()) {
			scsi_blocks[i] = harddisk[i]->sector_size * harddisk[i]->sector_num / BLOCK_SIZE;
		} else {
			scsi_blocks[i] = 0;
		}
	}
	secnum = 1;
	timeout = 0;
}

void BIOS::event_frame()
{
	timeout++;
}

bool BIOS::bios_call_far_i86(uint32_t PC, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	uint8_t *regs8 = (uint8_t *)regs;
	int drv = AL & 0xf;
	uint8_t buffer[BLOCK_SIZE * 4];
	
	if(PC == 0xfffc4) {
		// disk bios
#ifdef _DEBUG_LOG
		this->out_debug_log(_T("%6x\tDISK BIOS: AH=%2x,AL=%2x,CX=%4x,DX=%4x,BX=%4x,DS=%2x,DI=%2x\n"), get_cpu_pc(0), AH,AL,CX,DX,BX,DS,DI);
#endif
		if(AH == 0) {
			// set drive mode
			if(!(drv < MAX_DRIVE)) {
				AH = 2;
				*CarryFlag = 1;
				return true;
			}
			AH = 0;
			drive_mode1[drv] = DL;
			drive_mode2[drv] = BX;
			switch(DL & 0x30) {
			case 0x00: disk[drv]->drive_type = DRIVE_TYPE_2HD; break;
			case 0x10: disk[drv]->drive_type = DRIVE_TYPE_2DD; break;
			case 0x20: disk[drv]->drive_type = DRIVE_TYPE_2D ; break;
			}
			*CarryFlag = 0;
			return true;
		} else if(AH == 1) {
			// get drive mode
			if(!(drv < MAX_DRIVE)) {
				AH = 2;
				*CarryFlag = 1;
				return true;
			}
			AH = 0;
			DL = drive_mode1[drv];
			BX = drive_mode2[drv];
			*CarryFlag = 0;
			return true;
		} else if(AH == 2) {
			// drive status
			if((AL & 0xf0) == 0x20) {
				// floppy
				if(!(drv < MAX_DRIVE && disk[drv]->inserted)) {
					AH = 0x80;
					CX = ERR_FDD_NOTREADY;
					*CarryFlag = 1;
					return true;
				}
				AH = 0;
				if(disk[drv]->get_track(0, 0) && disk[drv]->get_sector(0, 0, 0)) {
					switch(disk[drv]->sector_size.sd) {
					case 128: AH = 0; break;
					case 256: AH = 1; break;
					case 512: AH = 2; break;
					default : AH = 3; break; // 1024
					}
				}
				DL = 0;
				if(disk[drv]->write_protected) {
					DL |= 2;
				}
				if(disk[drv]->two_side) {
					DL |= 4;
				}
//				if(disk[drv]->drive_type == DRIVE_TYPE_2D || disk[drv]->drive_type == DRIVE_TYPE_2DD) {
				if(disk[drv]->media_type == MEDIA_TYPE_2D || disk[drv]->media_type == MEDIA_TYPE_2DD) {
					DL |= 0x10;
				}
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			if((AL & 0xf0) == 0xb0) {
				// scsi
				if(!(drv < USE_HARD_DISK && scsi_blocks[drv])) {
					AH = 0x80;
					CX = ERR_SCSI_NOTCONNECTED;
					*CarryFlag = 1;
					return true;
				}
				AH = 0;
				AL = (BLOCK_SIZE == 128) ? 0 : (BLOCK_SIZE == 256) ? 1 : (BLOCK_SIZE == 512) ? 2 : 3;
				BX = scsi_blocks[drv] >> 16;
				DX = scsi_blocks[drv] & 0xffff;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			AH = 2;
			*CarryFlag = 1;
			return true;
		} else if(AH == 3 || AH == 4) {
			// restore/seek
			if((AL & 0xf0) == 0x20) {
				// floppy
				if(!(drv < MAX_DRIVE && disk[drv]->inserted)) {
					AH = 0x80;
					CX = ERR_FDD_NOTREADY;
					*CarryFlag = 1;
					return true;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			if((AL & 0xf0) == 0xb0) {
				// scsi
				if(!(drv < USE_HARD_DISK && scsi_blocks[drv])) {
					AH = 0x80;
					CX = ERR_SCSI_NOTCONNECTED;
					*CarryFlag = 1;
					return true;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			AH = 2;
			*CarryFlag = 1;
			return true;
		} else if(AH == 5) {
			// read sectors
			if((AL & 0xf0) == 0x20) {
				// floppy
				if(!(drv < MAX_DRIVE && disk[drv]->inserted)) {
					AH = 0x80;
					CX = ERR_FDD_NOTREADY;
					*CarryFlag = 1;
					return true;
				}
				// get initial c/h/r
				int ofs = DS * 16 + DI;
				int trk = CX;
				int hed = DH & 1;
				int sct = DL;
				while(BX > 0) {
					// search sector
					if(!disk[drv]->get_track(trk, hed)) {
						AH = 0x80;
						CX = ERR_FDD_NOTFOUND;
						*CarryFlag = 1;
						return true;
					}
					access_fdd[drv] = true;
					secnum = sct;
					if(!disk[drv]->get_sector(trk, hed, sct - 1)) {
						AH = 0x80;
						CX = ERR_FDD_NOTFOUND;
						*CarryFlag = 1;
						return true;
					}
					// check id crc error
					if(disk[drv]->addr_crc_error && !disk[drv]->ignore_crc()) {
						AH = 0x80;
						CX = ERR_FDD_NOTFOUND | ERR_FDD_CRCERROR;
						*CarryFlag = 1;
						return true;
					}
					// check deleted mark
					if(disk[drv]->deleted) {
						AH = 0x80;
						CX = ERR_FDD_DELETED;
						*CarryFlag = 1;
						return true;
					}
					// data transfer
					for(int i = 0; i < disk[drv]->sector_size.sd; i++) {
						d_mem->write_data8(ofs++, disk[drv]->sector[i]);
					}
					BX--;
					// check data crc error
					if(disk[drv]->data_crc_error && !disk[drv]->ignore_crc()) {
						AH = 0x80;
						CX = ERR_FDD_CRCERROR;
						*CarryFlag = 1;
						return true;
					}
					// update c/h/r
					if(++sct > disk[drv]->sector_num.sd) {
						sct = 1;
						if(++hed > 1) {
							hed = 0;
							++trk;
						}
					}
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			if((AL & 0xf0) == 0xb0) {
				// scsi
				if(!(drv < USE_HARD_DISK && scsi_blocks[drv])) {
					AH = 0x80;
					CX = ERR_SCSI_NOTCONNECTED;
					*CarryFlag = 1;
					return true;
				}
				if(!(harddisk[drv] != NULL && harddisk[drv]->mounted())) {
					AH = 0x80;
					CX = ERR_SCSI_NOTREADY;
					*CarryFlag = 1;
					return true;
				}
				// get params
				int ofs = DS * 16 + DI;
				int block = (CL << 16) | DX;
				long position = block * BLOCK_SIZE;
				while(BX > 0) {
					// check block
					if(!(block++ < scsi_blocks[drv])) {
						AH = 0x80;
						CX = ERR_SCSI_PARAMERROR;
						*CarryFlag = 1;
						return true;
					}
					// data transfer
					harddisk[drv]->read_buffer(position, BLOCK_SIZE, buffer);
					position += BLOCK_SIZE;
					for(int i = 0; i < BLOCK_SIZE; i++) {
						d_mem->write_data8(ofs++, buffer[i]);
					}
					BX--;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			AH = 2;
			*CarryFlag = 1;
			return true;
		} else if(AH == 6) {
			// write sectors
			if((AL & 0xf0) == 0x20) {
				// floppy
				if(!(drv < MAX_DRIVE && disk[drv]->inserted)) {
					AH = 0x80;
					CX = ERR_FDD_NOTREADY;
					*CarryFlag = 1;
					return true;
				}
				if(disk[drv]->write_protected) {
					AH = 0x80;
					CX = ERR_FDD_PROTECTED;
					*CarryFlag = 1;
					return true;
				}
				// get initial c/h/r
				int ofs = DS * 16 + DI;
				int trk = CX;
				int hed = DH & 1;
				int sct = DL;
				while(BX > 0) {
					// search sector
					if(!disk[drv]->get_track(trk, hed)) {
						AH = 0x80;
						CX = ERR_FDD_NOTFOUND;
						*CarryFlag = 1;
						return true;
					}
					access_fdd[drv] = true;
					secnum = sct;
					if(!disk[drv]->get_sector(trk, hed, sct - 1)) {
						AH = 0x80;
						CX = ERR_FDD_NOTFOUND;
						*CarryFlag = 1;
						return true;
					}
					// check id crc error
					if(disk[drv]->addr_crc_error && !disk[drv]->ignore_crc()) {
						AH = 0x80;
						CX = ERR_FDD_NOTFOUND | ERR_FDD_CRCERROR;
						*CarryFlag = 1;
						return true;
					}
					// data transfer
					for(int i = 0; i < disk[drv]->sector_size.sd; i++) {
						disk[drv]->sector[i] = d_mem->read_data8(ofs++);
					}
					BX--;
					// clear deleted mark and data crc error
					disk[drv]->set_deleted(false);
					disk[drv]->set_data_crc_error(false);
					// update c/h/r
					if(++sct > disk[drv]->sector_num.sd) {
						sct = 1;
						if(++hed > 1) {
							hed = 0;
							++trk;
						}
					}
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			if((AL & 0xf0) == 0xb0) {
				// scsi
				if(!(drv < USE_HARD_DISK && scsi_blocks[drv])) {
					AH = 0x80;
					CX = ERR_SCSI_NOTCONNECTED;
					*CarryFlag = 1;
					return true;
				}
				if(!(harddisk[drv] != NULL && harddisk[drv]->mounted())) {
					AH = 0x80;
					CX = ERR_SCSI_NOTREADY;
					*CarryFlag = 1;
					return true;
				}
				// get params
				int ofs = DS * 16 + DI;
				int block = (CL << 16) | DX;
				long position = block * BLOCK_SIZE;
				while(BX > 0) {
					// check block
					if(!(block++ < scsi_blocks[drv])) {
						AH = 0x80;
						CX = ERR_SCSI_PARAMERROR;
						*CarryFlag = 1;
						return true;
					}
					// data transfer
					for(int i = 0; i < BLOCK_SIZE; i++) {
						buffer[i] = d_mem->read_data8(ofs++);
					}
					harddisk[drv]->write_buffer(position, BLOCK_SIZE, buffer);
					position += BLOCK_SIZE;
					BX--;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			AH = 2;
			*CarryFlag = 1;
			return true;
		} else if(AH == 7) {
			// verify sectors
			if((AL & 0xf0) == 0x20) {
				// floppy
				if(!(drv < MAX_DRIVE && disk[drv]->inserted)) {
					AH = 0x80;
					CX = ERR_FDD_NOTREADY;
					*CarryFlag = 1;
					return true;
				}
				// get initial c/h/r
				int trk = CX;
				int hed = DH & 1;
				int sct = DL;
				while(BX > 0) {
					// search sector
					if(!disk[drv]->get_track(trk, hed)) {
						AH = 0x80;
						CX = ERR_FDD_NOTFOUND;
						*CarryFlag = 1;
						return true;
					}
					access_fdd[drv] = true;
					secnum = sct;
					if(!disk[drv]->get_sector(trk, hed, sct - 1)) {
						AH = 0x80;
						CX = ERR_FDD_NOTFOUND;
						*CarryFlag = 1;
						return true;
					}
					// check id crc error
					if(disk[drv]->addr_crc_error && !disk[drv]->ignore_crc()) {
						AH = 0x80;
						CX = ERR_FDD_NOTFOUND | ERR_FDD_CRCERROR;
						*CarryFlag = 1;
						return true;
					}
					// FIXME: verify
					BX--;
					// check data crc error
					if(disk[drv]->data_crc_error && !disk[drv]->ignore_crc()) {
						AH = 0x80;
						CX = ERR_FDD_CRCERROR;
						*CarryFlag = 1;
						return true;
					}
					// update c/h/r
					if(++sct > disk[drv]->sector_num.sd) {
						sct = 1;
						if(++hed > 1) {
							hed = 0;
							++trk;
						}
					}
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			if((AL & 0xf0) == 0xb0) {
				// scsi
				if(!(drv < USE_HARD_DISK && scsi_blocks[drv])) {
					AH = 0x80;
					CX = ERR_SCSI_NOTCONNECTED;
					*CarryFlag = 1;
					return true;
				}
				// get params
				int block = (CL << 16) | DX;
				while(BX > 0) {
					// check block
					if(!(block++ < scsi_blocks[drv])) {
						AH = 0x80;
						CX = ERR_SCSI_PARAMERROR;
						*CarryFlag = 1;
						return true;
					}
					BX--;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			AH = 2;
			*CarryFlag = 1;
			return true;
		} else if(AH == 8) {
			// reset hard drive controller
			AH = 0;
			CX = 0;
			*CarryFlag = 0;
			return true;
		} else if(AH == 9) {
			// read id
			if((AL & 0xf0) == 0x20) {
				// floppy
				if(!(drv < MAX_DRIVE && disk[drv]->inserted)) {
					AH = 0x80;
					CX = ERR_FDD_NOTREADY;
					*CarryFlag = 1;
					return true;
				}
				// get initial c/h
				int ofs = DS * 16 + DI;
				int trk = CX;
				int hed = DH & 1;
				// search sector
				if(!disk[drv]->get_track(trk, hed)) {
					AH = 0x80;
					CX = ERR_FDD_NOTFOUND;
					*CarryFlag = 1;
					return true;
				}
				access_fdd[drv] = true;
				if(++secnum > disk[drv]->sector_num.sd) {
					secnum = 1;
				}
				if(!disk[drv]->get_sector(trk, hed, secnum - 1)) {
					AH = 0x80;
					CX = ERR_FDD_NOTFOUND;
					*CarryFlag = 1;
					return true;
				}
				// data transfer
				for(int i = 0; i < 6; i++) {
					d_mem->write_data8(ofs++, disk[drv]->id[i]);
				}
				// check id crc error
				if(disk[drv]->addr_crc_error && !disk[drv]->ignore_crc()) {
					AH = 0x80;
					CX = ERR_FDD_CRCERROR;
					*CarryFlag = 1;
					return true;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			AH = 2;
			*CarryFlag = 1;
			return true;
		} else if(AH == 0xa) {
			// format track
			if((AL & 0xf0) == 0x20) {
				// floppy
				if(!(drv < MAX_DRIVE && disk[drv]->inserted)) {
					AH = 0x80;
					CX = ERR_FDD_NOTREADY;
					*CarryFlag = 1;
					return true;
				}
				// get initial c/h
				int ofs = DS * 16 + DI;
				int trk = CX;
				int hed = DH & 1;
				// format track
				disk[drv]->format_track(trk, hed);
				access_fdd[drv] = true;
				bool id_written = false;
				bool sector_found = false;
				int sector_length, sector_index;
				for(int index = 0; index < disk[drv]->get_track_size(); index++) {
					uint8_t datareg = d_mem->read_data8(ofs++);
					if(datareg == 0xf5) {
						// write a1h in missing clock
					} else if(datareg == 0xf6) {
						// write c2h in missing clock
					} else if(datareg == 0xf7) {
						// write crc
						if(!id_written) {
							// insert new sector with data crc error
write_id:
							id_written = true;
							sector_found = false;
							uint8_t c = disk[drv]->track[index - 4];
							uint8_t h = disk[drv]->track[index - 3];
							uint8_t r = disk[drv]->track[index - 2];
							uint8_t n = disk[drv]->track[index - 1];
							sector_length = 0x80 << (n & 3);
							sector_index = 0;
							disk[drv]->insert_sector(c, h, r, n, false, true, 0xe5, sector_length);
						} else if(sector_found) {
							// clear data crc error if all sector data are written
							disk[drv]->set_data_crc_error(false);
							id_written = false;
						} else {
							// data mark of current sector is not written
							disk[drv]->set_data_mark_missing();
							goto write_id;
						}
					} else if(id_written) {
						if(sector_found) {
							// sector data
							if(sector_index < sector_length) {
								disk[drv]->sector[sector_index] = datareg;
							}
							sector_index++;
						} else if(datareg == 0xf8 || datareg == 0xfb) {
							// data mark
							disk[drv]->set_deleted(datareg == 0xf8);
							sector_found = true;
						}
					}
					disk[drv]->track[index] = datareg;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			AH = 2;
			*CarryFlag = 1;
			return true;
		} else if(AH == 0xd) {
			// read error
			AH = 0;
			CX = 0;
			*CarryFlag = 0;
			return true;
		} else if(AH == 0xe) {
			// disk change ???
			if((AL & 0xf0) == 0x20) {
				// floppy
				if(!(drv < MAX_DRIVE && disk[drv]->inserted)) {
					AH = 0;
					CX = 0;
					DL = 1;
					*CarryFlag = 0;
					return true;
				}
				AH = 0;
				CX = 0;
				DL = disk[drv]->changed ? 1 : 0;
				disk[drv]->changed = false;
				*CarryFlag = 0;
				return true;
			}
			if((AL & 0xf0) == 0xb0) {
				// scsi
				if(!(drv < USE_HARD_DISK && scsi_blocks[drv])) {
					AH = 3;	// ???
					CX = 0;
					*CarryFlag = 1;
					return true;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			AH = 2;
			CX = 0;
			*CarryFlag = 1;
			return true;
		} else if(AH == 0xfa) {
			// unknown
			if((AL & 0xf0) == 0x20) {
				// floppy
				AH = 1;
				CX = 0;
				*CarryFlag = 1;
				return true;
			}
			if((AL & 0xf0) == 0xb0) {
				// scsi
				if(!(drv < USE_HARD_DISK && scsi_blocks[drv])) {
					AH = 0x80;
					CX = ERR_SCSI_NOTCONNECTED;
					*CarryFlag = 1;
					return true;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			AH = 2;
			*CarryFlag = 1;
			return true;
		} else if(AH == 0xfd) {
			// unknown
			if((AL & 0xf0) == 0x20) {
				// floppy
				AH = 1;
				CX = 0;
				*CarryFlag = 1;
				return true;
			}
			if((AL & 0xf0) == 0xb0) {
				// scsi
				if(!(drv < USE_HARD_DISK && scsi_blocks[drv])) {
					AH = 0;
					CX = 0x200;	// ???
					*CarryFlag = 0;
					return true;
				}
				AH = 2;
				CX = 0;
				*CarryFlag = 1;
				return true;
			}
			AH = 2;
			CX = 0;
			*CarryFlag = 1;
			return true;
		} else if(AH == 0x80) {
			// pseudo bios: init i/o
			for(int i = 0;; i++) {
				if(iotable[i][0] < 0) {
					break;
				}
				d_io->write_io8(iotable[i][0], iotable[i][1]);
			}
			// init cmos
			memset(cmos, 0, CMOS_SIZE);
			memcpy(cmos, cmos_t, sizeof(cmos_t));
			memcpy(cmos + CMOS_SIZE - sizeof(cmos_b), cmos_b, sizeof(cmos_b));
			// init int vector
			for(int i = 0, ofs = 0; i < 256; i++) {
				// int vector = ffff:0008
				d_mem->write_data16(ofs + 0, 0x0008);
				d_mem->write_data16(ofs + 2, 0xffff);
				ofs += 4;
			}
			// init screen
			memset(vram, 0, VRAM_SIZE);
#ifdef _FMR60
			memset(cvram, 0, 0x2000);
			memset(avram, 0, 0x2000);
#else
			memset(cvram, 0, 0x1000);
			memset(kvram, 0, 0x1000);
			memcpy(cvram + 0xf00, msg_c, sizeof(msg_c));
			memcpy(kvram + 0xf00, msg_k, sizeof(msg_k));
#endif
			*CarryFlag = 0;
			return true;
		} else if(AH == 0x81) {
			// pseudo bios: boot from fdd #0
			*ZeroFlag = (timeout > (int)(FRAMES_PER_SEC * 4));
			if(!disk[0]->inserted) {
				*CarryFlag = 1;
				return true;
			}
			// load ipl
			if(!disk[0]->get_track(0, 0)) {
				*CarryFlag = 1;
				return true;
			}
			access_fdd[0] = true;
			if(!disk[0]->get_sector(0, 0, 0)) {
				*CarryFlag = 1;
				return true;
			}
			for(int i = 0; i < disk[0]->sector_size.sd; i++) {
				buffer[i] = disk[0]->sector[i];
			}
			// check ipl
			if(!(buffer[0] == 'I' && buffer[1] == 'P' && buffer[2] == 'L' && buffer[3] == IPL_ID)) {
				*CarryFlag = 1;
				return true;
			}
			// data transfer
			for(int i = 0; i < disk[0]->sector_size.sd; i++) {
				d_mem->write_data8(0xb0000 + i, buffer[i]);
			}
			// clear screen
#ifdef _FMR60
			memset(cvram, 0, 0x2000);
			memset(avram, 0, 0x2000);
#else
			memset(cvram, 0, 0x1000);
			memset(kvram, 0, 0x1000);
#endif
			// set result
			AX = 0xff;
			CX = 0;
			BX = 2;
			*ZeroFlag = 1;
			*CarryFlag = 0;
			return true;
		} else if(AH == 0x82) {
			// pseudo bios: boot from scsi-hdd #0
			timeout = 0;
			if(!scsi_blocks[0]) {
				*CarryFlag = 1;
				return true;
			}
			if(!(harddisk[drv] != NULL && harddisk[drv]->mounted())) {
				*CarryFlag = 1;
				return true;
			}
			// load ipl
			harddisk[drv]->read_buffer(0, BLOCK_SIZE * 4, buffer);
			// check ipl
			if(!(buffer[0] == 'I' && buffer[1] == 'P' && buffer[2] == 'L' && buffer[3] == IPL_ID)) {
				*CarryFlag = 1;
				return true;
			}
			// data transfer
			for(int i = 0; i < BLOCK_SIZE * 4; i++) {
				d_mem->write_data8(0xb0000 + i, buffer[i]);
			}
			// clear screen
#ifdef _FMR60
			memset(cvram, 0, 0x2000);
			memset(avram, 0, 0x2000);
#else
			memset(cvram, 0, 0x1000);
			memset(kvram, 0, 0x1000);
#endif
			// set result
			AX = 0xffff;
			CX = 0;
			BX = 1;
			*ZeroFlag = 1;
			*CarryFlag = 0;
			return true;
		}
	} else if(PC == 0xfffc9) {
		// cmos
#ifdef _DEBUG_LOG
		this->out_debug_log(_T("%6x\tCMOS BIOS: AH=%2x,AL=%2x,CX=%4x,DX=%4x,BX=%4x,DS=%2x,DI=%2x\n"), get_cpu_pc(0), AH,AL,CX,DX,BX,DS,DI);
#endif
		if(AH == 0) {
			// init cmos
			memcpy(cmos, cmos_t, sizeof(cmos_t));
			memcpy(cmos + CMOS_SIZE - sizeof(cmos_b), cmos_b, sizeof(cmos_b));
		} else if(AH == 5) {
			// get $a2
			BX = cmos[0xa2] | (cmos[0xa3] << 8);
		} else if(AH == 10) {
			// memory to cmos
			int block = AL * 10;
			int len = cmos[block + 6] | (cmos[block + 7] << 8);
			int dst = cmos[block + 8] | (cmos[block + 9] << 8);
			int src = DS * 16 + DI;
			for(int i = 0; i < len; i++) {
				cmos[dst++] = d_mem->read_data8(src++);
			}
		} else if(AH == 11) {
			// cmos to memory
			int block = AL * 10;
			int len = cmos[block + 6] | (cmos[block + 7] << 8);
			int src = cmos[block + 8] | (cmos[block + 9] << 8);
			int dst = DS * 16 + DI;
			for(int i = 0; i < len; i++) {
				d_mem->write_data8(dst++, cmos[src++]);
			}
		} else if(AH == 20) {
			// check block header
			BX = 0;
		}
		AH = 0;
		*CarryFlag = 0;
		return true;
	} else if(PC == 0xfffd3) {
		// wait
#ifdef _DEBUG_LOG
		this->out_debug_log(_T("%6x\tWAIT BIOS: AH=%2x,AL=%2x,CX=%4x,DX=%4x,BX=%4x,DS=%2x,DI=%2x\n"), get_cpu_pc(0), AH,AL,CX,DX,BX,DS,DI);
#endif
		*CarryFlag = 0;
		return true;
	}
	return false;
}

bool BIOS::bios_int_i86(int intnum, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	uint8_t *regs8 = (uint8_t *)regs;
	
	if(intnum == 0x93) {
		// disk bios
		return bios_call_far_i86(0xfffc4, regs, sregs, ZeroFlag, CarryFlag);
	}
	return false;
}

uint32_t BIOS::read_signal(int ch)
{
	// get access status
	uint32_t stat = 0;
	if(ch == 0) {
		for(int i = 0; i < MAX_DRIVE; i++) {
			if(access_fdd[i]) {
				stat |= 1 << i;
			}
			access_fdd[i] = false;
		}
	}
	return stat;
}

#define STATE_VERSION	4

bool BIOS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(!disk[i]->process_state(state_fio, loading)) {
			return false;
		}
	}
	state_fio->StateValue(secnum);
	state_fio->StateValue(timeout);
	state_fio->StateArray(drive_mode1, sizeof(drive_mode1), 1);
	state_fio->StateArray(drive_mode2, sizeof(drive_mode2), 1);
	state_fio->StateArray(scsi_blocks, sizeof(scsi_blocks), 1);
	return true;
}

