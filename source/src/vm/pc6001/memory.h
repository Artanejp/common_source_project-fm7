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

#ifndef _PC6001_MEMORY_H_
#define _PC6001_MEMORY_H_

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

class PC6001_MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu;
#ifndef _PC6001
	TIMER *d_timer;
#endif
	uint8_t MEMORY_BASE[MEMORY_SIZE];
//	uint8_t RAM[0x10000];
//	uint8_t EXTROM[0x4000];		// CURRENT EXTEND ROM
//	uint8_t BASICROM[0x8000];		// BASICROM
	uint8_t *CGROM;
//	uint8_t CGROM1[0x4000];		// CGROM1
	uint8_t *EXTROM1;				// EXTEND ROM 1
	uint8_t *EXTROM2;				// EXTEND ROM 2
	uint8_t *RdMem[8];			// READ  MEMORY MAPPING ADDRESS
	uint8_t *WrMem[8];			// WRITE MEMORY MAPPING ADDRESS
	uint8_t *VRAM;
//	uint8_t EmptyRAM[0x2000];
	uint8_t EnWrite[4];			// MEMORY MAPPING WRITE ENABLE [N60/N66]
	uint8_t CGSW93;
	bool inserted;
#ifndef _PC6001
	uint8_t CRTKILL;
//	uint8_t VOICEROM[0x4000];
//	uint8_t KANJIROM[0x8000];
	uint8_t *CurKANJIROM;
//	uint8_t CGROM5[0x2000];		// CGROM5
	uint8_t CRTMode1,CRTMode2,CRTMode3;
	uint8_t CSS1,CSS2,CSS3;
	uint8_t portF0;
	uint8_t portF1;
	uint8_t* dest;
	scrntype_t palette_pc[16];
	int BPal[16],BPal11[4],BPal12[8],BPal13[8],BPal14[4],BPal15[8],BPal53[32],BPal61[16],BPal62[32];
	uint8_t W;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	int bitmap;
	int cols;					// text cols
	int rows;					// text rows
	int lines;
	uint8_t *TEXTVRAM;
	uint8_t *SYSROM2;
//	uint8_t CGROM6[0x8000];		// CGROM6
//	uint8_t SYSTEMROM1[0x10000];	// SYSTEMROM1
//	uint8_t SYSTEMROM2[0x10000];	// SYSTEMROM2
//	uint8_t EXTRAM[0x10000];
	uint8_t port60[16];
	uint8_t portC1;					//I/O[C1]     CRT CONTROLLER MODE
	uint8_t portC8;					//I/O[C8]     CRT CONTROLLER TYPE
	uint8_t portCA;					//I/O[CA]     X GEOMETORY low  HARDWARE SCROLL
	uint8_t portCB;					//I/O[CB]     X GEOMETORY high HARDWARE SCROLL
	uint8_t portCC;					//I/O[CC]     Y GEOMETORY      HARDWARE SCROLL
	uint8_t portCE;					//I/O[CE]     LINE SETTING  BITMAP (low) */
	uint8_t portCF;					//I/O[CF]     LINE SETTING  BITMAP (High) */
	int palet[16];				// SR PALET
	uint8_t screen[204][640];
#else
	uint8_t screen[200][320];
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
	int chk_gvram(uint32_t A,int flag);
	uint8_t gvram_read(uint32_t A);
	void gvram_write(uint32_t A, uint32_t V);
#endif
#endif
	
	// Pointer Values
	int tmp_cgrom_ptr;
	int tmp_extrom1_ptr;
	int tmp_extrom2_ptr;
	int tmp_rdmem_ptr[8];
	int tmp_wrmem_ptr[8];
	int tmp_vram_ptr;
#ifndef _PC6001
	int tmp_kanjirom_ptr;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	int tmp_textvram_ptr;
	int tmp_sysrom2_ptr;
#endif
#endif

public:
	PC6001_MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		inserted = false;
		set_device_name(_T("Memory Bus"));
	}
	~PC6001_MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
#ifndef _PC6001
	uint32_t read_io8(uint32_t addr);
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
#endif
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
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
	void open_cart(const _TCHAR* file_path);
	void close_cart();
	bool is_cart_inserted()
	{
		return inserted;
	}
	uint8_t* get_vram()
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
