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
#include "../../fileio.h"

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
#define ERR_MEMCARD_NOTREADY	1
#define ERR_MEMCARD_PROTECTED	2
#define ERR_MEMCARD_PARAMERROR	0x200

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
static const uint8 cmos_t[] = {
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
static const uint8 cmos_b[] = {
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
static const uint8 msg_c[] = {
	0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,
	0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,
	0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,
	0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07,0xff,0x47,0xff,0x07
};

// 'システムをセットしてください'
static const uint8 msg_k[] = {
	0x25,0x37,0x00,0x00,0x25,0x39,0x00,0x00,0x25,0x46,0x00,0x00,0x25,0x60,0x00,0x00,
	0x24,0x72,0x00,0x00,0x25,0x3b,0x00,0x00,0x25,0x43,0x00,0x00,0x25,0x48,0x00,0x00,
	0x24,0x37,0x00,0x00,0x24,0x46,0x00,0x00,0x24,0x2f,0x00,0x00,0x24,0x40,0x00,0x00,
	0x24,0x35,0x00,0x00,0x24,0x24,0x00,0x00,0x21,0x21,0x00,0x00
};

void BIOS::initialize()
{
	// check ipl
	disk_pc1 = disk_pc2 = cmos_pc = wait_pc = -1;
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(buffer, sizeof(buffer), 1);
		fio->Fclose();
		
		uint32 addr = 0xfffc4;
		if(buffer[addr & (IPL_SIZE - 1)] == 0xea) {
			int ofs = buffer[++addr & (IPL_SIZE - 1)];
			ofs |= buffer[++addr & (IPL_SIZE - 1)] << 8;
			int seg = buffer[++addr & (IPL_SIZE - 1)];
			seg |= buffer[++addr & (IPL_SIZE - 1)] << 8;
			disk_pc1 = addr = ofs + (seg << 4);
		}
		if(buffer[addr & (IPL_SIZE - 1)] == 0xea) {
			int ofs = buffer[++addr & (IPL_SIZE - 1)];
			ofs |= buffer[++addr & (IPL_SIZE - 1)] << 8;
			int seg = buffer[++addr & (IPL_SIZE - 1)];
			seg |= buffer[++addr & (IPL_SIZE - 1)] << 8;
			disk_pc2 = ofs + (seg << 4);
		}
	} else {
		// use pseudo ipl
		cmos_pc = 0xfffc9;
		wait_pc = 0xfffd3;
		
		// register event
		register_frame_event(this);
	}
	
	// init scsi
	memset(scsi_blocks, 0, sizeof(scsi_blocks));
	for(int i = 0; i < MAX_SCSI; i++) {
		_stprintf(scsi_path[i], _T("%sSCSI%d.DAT"), emu->application_path(), i);
		if(fio->Fopen(scsi_path[i], FILEIO_READ_BINARY)) {
			fio->Fseek(0, FILEIO_SEEK_END);
			scsi_blocks[i] = fio->Ftell() / BLOCK_SIZE;
			fio->Fclose();
		}
	}
	
	// init memcard
	memset(memcard_blocks, 0, sizeof(memcard_blocks));
	for(int i = 0; i < MAX_MEMCARD; i++) {
		_stprintf(memcard_path[i], _T("%sMEMCARD%d.DAT"), emu->application_path(), i);
		if(fio->Fopen(memcard_path[i], FILEIO_READ_BINARY)) {
			fio->Fseek(0, FILEIO_SEEK_END);
			memcard_blocks[i] = fio->Ftell() / BLOCK_SIZE;
			memcard_protected[i] = fio->IsProtected(memcard_path[i]);
			fio->Fclose();
		}
	}
	delete fio;
}

void BIOS::reset()
{
	for(int i = 0; i < MAX_DRIVE; i++) {
		access_fdd[i] = false;
	}
	access_scsi = false;
	secnum = 1;
	powmode = 0;
	timeout = 0;
}

void BIOS::event_frame()
{
	timeout++;
}

bool BIOS::bios_call(uint32 PC, uint16 regs[], uint16 sregs[], int32* ZeroFlag, int32* CarryFlag)
{
	uint8 *regs8 = (uint8 *)regs;
	int drv = AL & 0xf;
	
	if(PC == 0xfffc4 || PC == disk_pc1 || PC == disk_pc2) {
		// disk bios
#ifdef _DEBUG_LOG
		emu->out_debug_log(_T("%6x\tDISK BIOS: AH=%2x,AL=%2x,CX=%4x,DX=%4x,BX=%4x,DS=%2x,DI=%2x\n"), get_cpu_pc(0), AH,AL,CX,DX,BX,DS,DI);
#endif
//		if(!((AL & 0xf0) == 0x20 || (AL & 0xf0) == 0x50 || (AL & 0xf0) == 0xb0)) {
			// target drive is not floppy, memcard and scsi hard drive
//			return false;
//		}
		if(AH == 2) {
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
				DL = 4;
				if(disk[drv]->write_protected) {
					DL |= 2;
				}
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			if((AL & 0xf0) == 0x50) {
				// memcard
				if(!(drv < MAX_MEMCARD && memcard_blocks[drv])) {
					AH = 0x80;
					CX = ERR_MEMCARD_NOTREADY;
					*CarryFlag = 1;
					return true;
				}
				AH = 0;
				AL = 2;
				DL = memcard_protected[drv] ? 2 : 0;
				CX = 0;
				*CarryFlag = 0;
				return true;
			}
			if((AL & 0xf0) == 0xb0) {
				// scsi
				if(!(drv < MAX_SCSI && scsi_blocks[drv])) {
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
			// resture/seek
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
				if(!(drv < MAX_SCSI && scsi_blocks[drv])) {
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
					disk[drv]->get_track(trk, hed);
					access_fdd[drv] = true;
					secnum = sct;
					if(!disk[drv]->get_sector(trk, hed, sct - 1)) {
						AH = 0x80;
						CX = ERR_FDD_NOTFOUND;
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
					for(int i = 0; i < disk[drv]->sector_size; i++) {
						d_mem->write_data8(ofs++, disk[drv]->sector[i]);
					}
					BX--;
					// check crc error
					if(disk[drv]->status) {
						AH = 0x80;
						CX = ERR_FDD_CRCERROR;
						*CarryFlag = 1;
						return true;
					}
					// update c/h/r
					if(++sct > disk[drv]->sector_num) {
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
			if((AL & 0xf0) == 0x50) {
				// memcard
				if(!(drv < MAX_MEMCARD && memcard_blocks[drv])) {
					AH = 0x80;
					CX = ERR_MEMCARD_NOTREADY;
					*CarryFlag = 1;
					return true;
				}
				FILEIO* fio = new FILEIO();
				if(!fio->Fopen(memcard_path[drv], FILEIO_READ_BINARY)) {
					AH = 0x80;
					CX = ERR_MEMCARD_NOTREADY;
					*CarryFlag = 1;
					delete fio;
					return true;
				}
				// get params
				int ofs = DS * 16 + DI;
				int block = (CL << 16) | DX;
				fio->Fseek(block * BLOCK_SIZE, FILEIO_SEEK_SET);
				while(BX > 0) {
					// check block
					if(!(block++ < memcard_blocks[drv])) {
						AH = 0x80;
						CX = ERR_MEMCARD_PARAMERROR;
						*CarryFlag = 1;
						fio->Fclose();
						delete fio;
						return true;
					}
					// data transfer
					fio->Fread(buffer, BLOCK_SIZE, 1);
					for(int i = 0; i < BLOCK_SIZE; i++) {
						d_mem->write_data8(ofs++, buffer[i]);
					}
					BX--;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				fio->Fclose();
				delete fio;
				return true;
			}
			if((AL & 0xf0) == 0xb0) {
				// scsi
				if(!(drv < MAX_SCSI && scsi_blocks[drv])) {
					AH = 0x80;
					CX = ERR_SCSI_NOTCONNECTED;
					*CarryFlag = 1;
					return true;
				}
				FILEIO* fio = new FILEIO();
				if(!fio->Fopen(scsi_path[drv], FILEIO_READ_BINARY)) {
					AH = 0x80;
					CX = ERR_SCSI_NOTREADY;
					*CarryFlag = 1;
					delete fio;
					return true;
				}
				// get params
				int ofs = DS * 16 + DI;
				int block = (CL << 16) | DX;
				fio->Fseek(block * BLOCK_SIZE, FILEIO_SEEK_SET);
				while(BX > 0) {
					// check block
					access_scsi = true;
					if(!(block++ < scsi_blocks[drv])) {
						AH = 0x80;
						CX = ERR_SCSI_PARAMERROR;
						*CarryFlag = 1;
						fio->Fclose();
						delete fio;
						return true;
					}
					// data transfer
					fio->Fread(buffer, BLOCK_SIZE, 1);
					for(int i = 0; i < BLOCK_SIZE; i++) {
						d_mem->write_data8(ofs++, buffer[i]);
					}
					BX--;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				fio->Fclose();
				delete fio;
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
					disk[drv]->get_track(trk, hed);
					access_fdd[drv] = true;
					secnum = sct;
					if(!disk[drv]->get_sector(trk, hed, sct - 1)) {
						AH = 0x80;
						CX = ERR_FDD_NOTFOUND;
						*CarryFlag = 1;
						return true;
					}
					// data transfer
					for(int i = 0; i < disk[drv]->sector_size; i++) {
						disk[drv]->sector[i] = d_mem->read_data8(ofs++);
					}
					BX--;
					// clear deleted mark and crc error
					disk[drv]->deleted = 0;
					disk[drv]->status = 0;
					// update c/h/r
					if(++sct > disk[drv]->sector_num) {
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
			if((AL & 0xf0) == 0x50) {
				// memcard
				if(!(drv < MAX_MEMCARD && memcard_blocks[drv])) {
					AH = 0x80;
					CX = ERR_MEMCARD_NOTREADY;
					*CarryFlag = 1;
					return true;
				}
				if(memcard_protected[drv]) {
					AH = 0x80;
					CX = ERR_MEMCARD_PROTECTED;
					*CarryFlag = 1;
					return true;
				}
				FILEIO* fio = new FILEIO();
				if(!fio->Fopen(memcard_path[drv], FILEIO_READ_WRITE_BINARY)) {
					AH = 0x80;
					CX = ERR_MEMCARD_NOTREADY;
					*CarryFlag = 1;
					delete fio;
					return true;
				}
				// get params
				int ofs = DS * 16 + DI;
				int block = (CL << 16) | DX;
				fio->Fseek(block * BLOCK_SIZE, FILEIO_SEEK_SET);
				while(BX > 0) {
					// check block
					access_scsi = true;
					if(!(block++ < scsi_blocks[drv])) {
						AH = 0x80;
						CX = ERR_MEMCARD_PARAMERROR;
						*CarryFlag = 1;
						fio->Fclose();
						delete fio;
						return true;
					}
					// data transfer
					for(int i = 0; i < BLOCK_SIZE; i++) {
						buffer[i] = d_mem->read_data8(ofs++);
					}
					fio->Fwrite(buffer, BLOCK_SIZE, 1);
					BX--;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				fio->Fclose();
				delete fio;
				return true;
			}
			if((AL & 0xf0) == 0xb0) {
				// scsi
				if(!(drv < MAX_SCSI && scsi_blocks[drv])) {
					AH = 0x80;
					CX = ERR_SCSI_NOTCONNECTED;
					*CarryFlag = 1;
					return true;
				}
				FILEIO* fio = new FILEIO();
				if(!fio->Fopen(scsi_path[drv], FILEIO_READ_WRITE_BINARY)) {
					AH = 0x80;
					CX = ERR_SCSI_NOTREADY;
					*CarryFlag = 1;
					delete fio;
					return true;
				}
				// get params
				int ofs = DS * 16 + DI;
				int block = (CL << 16) | DX;
				fio->Fseek(block * BLOCK_SIZE, FILEIO_SEEK_SET);
				while(BX > 0) {
					// check block
					access_scsi = true;
					if(!(block++ < scsi_blocks[drv])) {
						AH = 0x80;
						CX = ERR_SCSI_PARAMERROR;
						*CarryFlag = 1;
						fio->Fclose();
						delete fio;
						return true;
					}
					// data transfer
					for(int i = 0; i < BLOCK_SIZE; i++) {
						buffer[i] = d_mem->read_data8(ofs++);
					}
					fio->Fwrite(buffer, BLOCK_SIZE, 1);
					BX--;
				}
				AH = 0;
				CX = 0;
				*CarryFlag = 0;
				fio->Fclose();
				delete fio;
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
					disk[drv]->get_track(trk, hed);
					access_fdd[drv] = true;
					secnum = sct;
					if(!disk[drv]->get_sector(trk, hed, sct - 1)) {
						AH = 0x80;
						CX = ERR_FDD_NOTFOUND;
						*CarryFlag = 1;
						return true;
					}
					BX--;
					// check crc error
					if(disk[drv]->status) {
						AH = 0x80;
						CX = ERR_FDD_CRCERROR;
						*CarryFlag = 1;
						return true;
					}
					// update c/h/r
					if(++sct > disk[drv]->sector_num) {
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
				if(!(drv < MAX_SCSI && scsi_blocks[drv])) {
					AH = 0x80;
					CX = ERR_SCSI_NOTCONNECTED;
					*CarryFlag = 1;
					return true;
				}
				// get params
				int block = (CL << 16) | DX;
				while(BX > 0) {
					// check block
					access_scsi = true;
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
				disk[drv]->get_track(trk, hed);
				access_fdd[drv] = true;
				if(++secnum > disk[drv]->sector_num) {
					secnum = 1;
				}
				if(!disk[drv]->get_sector(trk, hed, secnum - 1)) {
					AH = 0x80;
					CX = ERR_FDD_NOTFOUND;
					*CarryFlag = 1;
					return true;
				}
				for(int i = 0; i < 6; i++) {
					d_mem->write_data8(ofs++, disk[drv]->id[i]);
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
				int trk = CX;
				int hed = DH & 1;
				// search sector
				disk[drv]->get_track(trk, hed);
				access_fdd[drv] = true;
				for(int i = 0; i < disk[drv]->sector_num; i++) {
					disk[drv]->get_sector(trk, hed, i);
					memset(disk[drv]->sector, 0xe5, disk[drv]->sector_size);
					disk[drv]->deleted = 0;
					disk[drv]->status = 0;
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
				if(!(drv < MAX_SCSI && scsi_blocks[drv])) {
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
				if(!(drv < MAX_SCSI && scsi_blocks[drv])) {
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
				if(!(drv < MAX_SCSI && scsi_blocks[drv])) {
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
			disk[0]->get_track(0, 0);
			access_fdd[0] = true;
			if(!disk[0]->get_sector(0, 0, 0)) {
				*CarryFlag = 1;
				return true;
			}
			for(int i = 0; i < disk[0]->sector_size; i++) {
				buffer[i] = disk[0]->sector[i];
			}
			// check ipl
			if(!(buffer[0] == 'I' && buffer[1] == 'P' && buffer[2] == 'L' && buffer[3] == IPL_ID)) {
				*CarryFlag = 1;
				return true;
			}
			// data transfer
			for(int i = 0; i < disk[0]->sector_size; i++) {
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
			FILEIO* fio = new FILEIO();
			if(!fio->Fopen(scsi_path[drv], FILEIO_READ_BINARY)) {
				*CarryFlag = 1;
				delete fio;
				return true;
			}
			// load ipl
			access_scsi = true;
			fio->Fread(buffer, BLOCK_SIZE * 4, 1);
			fio->Fclose();
			delete fio;
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
	} else if(PC == cmos_pc) {
		// cmos
#ifdef _DEBUG_LOG
		emu->out_debug_log(_T("%6x\tCMOS BIOS: AH=%2x,AL=%2x,CX=%4x,DX=%4x,BX=%4x,DS=%2x,DI=%2x\n"), get_cpu_pc(0), AH,AL,CX,DX,BX,DS,DI);
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
	} else if(PC == wait_pc) {
		// wait
#ifdef _DEBUG_LOG
		emu->out_debug_log(_T("%6x\tWAIT BIOS: AH=%2x,AL=%2x,CX=%4x,DX=%4x,BX=%4x,DS=%2x,DI=%2x\n"), get_cpu_pc(0), AH,AL,CX,DX,BX,DS,DI);
#endif
		*CarryFlag = 0;
		return true;
	}
	return false;
}

bool BIOS::bios_int(int intnum, uint16 regs[], uint16 sregs[], int32* ZeroFlag, int32* CarryFlag)
{
	uint8 *regs8 = (uint8 *)regs;
	
	if(intnum == 0x93) {
		// disk bios
		return bios_call(0xfffc4, regs, sregs, ZeroFlag, CarryFlag);
	} else if(intnum == 0xaa) {
		// power management bios
		if(AH == 0) {
			if(AL > 2) {
				AH = 2;
				*CarryFlag = 1;
				return true;
			}
			powmode = AL;
			AH = 0;
			*CarryFlag = 0;
			return true;
		} else if(AH == 1) {
			AH = 0;
			AL = BL = powmode;
			*CarryFlag = 0;
			return true;
		}
	}
	return false;
}

uint32 BIOS::read_signal(int ch)
{
	// get access status
	uint32 stat = 0;
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(access_fdd[i]) {
			stat |= 1 << i;
		}
		access_fdd[i] = false;
	}
	if(access_scsi) {
		stat |= 0x10;
	}
	access_scsi = false;
	return stat;
}

