//
// refresh screen by Koichi Nishida 2006
// based on Marat Fayzullin's fMSX
// and Hiroshi Ishioka's iP6
//

/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.07.15-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_PIO_PORT_C	0

#ifndef _PC6001
class TIMER;
#endif

// memory offset
#define RAM_BASE		0
#define RAM_SIZE		0x10000
#define BASICROM_BASE		(RAM_BASE + RAM_SIZE)
#define BASICROM_SIZE		0x8000
#define EXTROM_BASE		(BASICROM_BASE + BASICROM_SIZE)
#define EXTROM_SIZE		0x4000
#define CGROM1_BASE		(EXTROM_BASE + EXTROM_SIZE)
#define CGROM1_SIZE		0x4000
#define EmptyRAM_BASE		(CGROM1_BASE + CGROM1_SIZE)
#define EmptyRAM_SIZE		0x2000
#if defined(_PC6001)
#define MEMORY_SIZE		(EmptyRAM_BASE + EmptyRAM_SIZE)
#endif
#define VOICEROM_BASE		(EmptyRAM_BASE + EmptyRAM_SIZE)
#define VOICEROM_SIZE		0x4000
#define KANJIROM_BASE		(VOICEROM_BASE + VOICEROM_SIZE)
#define KANJIROM_SIZE		0x8000
#define CGROM5_BASE		(KANJIROM_BASE + KANJIROM_SIZE)
#define CGROM5_SIZE		0x2000
#if defined(_PC6601) || defined(_PC6001MK2)
#define MEMORY_SIZE		(CGROM5_BASE + CGROM5_SIZE)
#endif
#define EXTRAM_BASE		(CGROM5_BASE + CGROM5_SIZE)
#define EXTRAM_SIZE		0x10000
#define SYSTEMROM1_BASE		(EXTRAM_BASE + EXTRAM_SIZE)
#define SYSTEMROM1_SIZE		0x10000
#define SYSTEMROM2_BASE		(SYSTEMROM1_BASE + SYSTEMROM1_SIZE)
#define SYSTEMROM2_SIZE		0x10000
#define CGROM6_BASE		(SYSTEMROM2_BASE + SYSTEMROM2_SIZE)
#define CGROM6_SIZE		0x8000
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
#define MEMORY_SIZE		(CGROM6_BASE + CGROM6_SIZE)
#endif

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu;
#ifndef _PC6001
	TIMER *d_timer;
#endif
	uint8 MEMORY_BASE[MEMORY_SIZE];
//	uint8 RAM[0x10000];
//	uint8 EXTROM[0x4000];		// CURRENT EXTEND ROM
//	uint8 BASICROM[0x8000];		// BASICROM
	uint8 *CGROM;
//	uint8 CGROM1[0x4000];		// CGROM1
	uint8 *EXTROM1;				// EXTEND ROM 1
	uint8 *EXTROM2;				// EXTEND ROM 2
	uint8 *RdMem[8];			// READ  MEMORY MAPPING ADDRESS
	uint8 *WrMem[8];			// WRITE MEMORY MAPPING ADDRESS
	uint8 *VRAM;
//	uint8 EmptyRAM[0x2000];
	uint8 EnWrite[4];			// MEMORY MAPPING WRITE ENABLE [N60/N66]
	byte CGSW93;
	bool inserted;
#ifndef _PC6001
	byte CRTKILL;
//	uint8 VOICEROM[0x4000];
//	uint8 KANJIROM[0x8000];
	uint8 *CurKANJIROM;
//	uint8 CGROM5[0x2000];		// CGROM5
	byte CRTMode1,CRTMode2,CRTMode3;
	byte CSS1,CSS2,CSS3;
	byte portF0;
	byte portF1;
	uint8* dest;
	scrntype palette_pc[16];
	int BPal[16],BPal11[4],BPal12[8],BPal13[8],BPal14[4],BPal15[8],BPal53[32],BPal61[16],BPal62[32];
	uint8 W;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	int bitmap;
	int cols;					// text cols
	int rows;					// text rows
	int lines;
	uint8 *TEXTVRAM;
	uint8 *SYSROM2;
//	uint8 CGROM6[0x8000];		// CGROM6
//	uint8 SYSTEMROM1[0x10000];	// SYSTEMROM1
//	uint8 SYSTEMROM2[0x10000];	// SYSTEMROM2
//	uint8 EXTRAM[0x10000];
	byte port60[16];
	byte portC1;					//I/O[C1]     CRT CONTROLLER MODE
	byte portC8;					//I/O[C8]     CRT CONTROLLER TYPE
	byte portCA;					//I/O[CA]     X GEOMETORY low  HARDWARE SCROLL
	byte portCB;					//I/O[CB]     X GEOMETORY high HARDWARE SCROLL
	byte portCC;					//I/O[CC]     Y GEOMETORY      HARDWARE SCROLL
	byte portCE;					//I/O[CE]     LINE SETTING  BITMAP (low) */
	byte portCF;					//I/O[CF]     LINE SETTING  BITMAP (High) */
	int palet[16];				// SR PALET
	uint8 screen[204][640];
#else
	uint8 screen[200][320];
#endif
	
	void RefreshScr10();
	void RefreshScr11();
	void RefreshScr13();
	void RefreshScr13a();
	void RefreshScr13b();
	void RefreshScr13c();
	void RefreshScr13d();
	void RefreshScr13e();
	void RefreshScr51();
	void RefreshScr53();
	void RefreshScr54();
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	void RefreshScr61();
	void RefreshScr62();
	void RefreshScr63();
	void do_palet(int dest,int src);
	void make_semigraph(void);
	int chk_gvram(uint32 A,int flag);
	byte gvram_read(uint32 A);
	void gvram_write(uint32 A, uint32 V);
#endif
#endif
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {inserted=false;}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
#ifndef _PC6001
	uint32 read_io8(uint32 addr);
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
#endif
	void write_signal(int id, uint32 data, uint32 mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
#ifndef _PC6001
	void draw_screen();
#endif
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
#ifndef _PC6001
	void set_context_timer(TIMER* device)
	{
		d_timer = device;
	}
#endif
	void open_cart(_TCHAR* file_path);
	void close_cart();
	bool cart_inserted()
	{
		return inserted;
	}
	uint8* get_vram()
	{
		return MEMORY_BASE + RAM_BASE;
	}
#ifndef _PC6001
	int get_CRTMode2()
	{
		return CRTMode2;
	}
#endif
};
#endif
