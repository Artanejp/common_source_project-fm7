/*
	Skelton for retropc emulator

	Origin : MAME 0.148
	Author : Takeda.Toshiya
	Date   : 2013.05.01-

	[ MCS48 ]
*/

#ifndef _LIBNEWDEV_MCS84_BASE_H_ 
#define _LIBNEWDEV_MCS84_BASE_H_ 

#include "./device.h"

#define MCS48_PORT_P0	0x100	/* Not used */
#define MCS48_PORT_P1	0x101	/* P10-P17 */
#define MCS48_PORT_P2	0x102	/* P20-P28 */
#define MCS48_PORT_T0	0x110
#define MCS48_PORT_T1	0x111
#define MCS48_PORT_BUS	0x120	/* DB0-DB7 */
#define MCS48_PORT_PROG	0x121	/* PROG line to 8243 expander */

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* live processor state */
struct mcs48_state
{
	UINT16      prevpc;             /* 16-bit previous program counter */
	UINT16      pc;                 /* 16-bit program counter */

	UINT8       a;                  /* 8-bit accumulator */
	int         regptr;             /* offset of r0-r7 */
	UINT8       psw;                /* 8-bit cpustate->psw */
	UINT8       p1;                 /* 8-bit latched port 1 */
	UINT8       p2;                 /* 8-bit latched port 2 */
	UINT8       timer;              /* 8-bit timer */
	UINT8       prescaler;          /* 5-bit timer prescaler */
	UINT8       t1_history;         /* 8-bit history of the T1 input */
	UINT8       sts;                /* 8-bit status register */

	UINT8       int_state;          /* INT signal status */
	UINT8       irq_state;          /* TRUE if an IRQ is pending */
	UINT8       irq_in_progress;    /* TRUE if an IRQ is in progress */
	UINT8       timer_overflow;     /* TRUE on a timer overflow; cleared by taking interrupt */
	UINT8       timer_flag;         /* TRUE on a timer overflow; cleared on JTF */
	UINT8       tirq_enabled;       /* TRUE if the timer IRQ is enabled */
	UINT8       xirq_enabled;       /* TRUE if the external IRQ is enabled */
	UINT8       t0_clk_enabled;     /* TRUE if ent0_clk is called */
	UINT8       timecount_enabled;  /* bitmask of timer/counter enabled */

	UINT16      a11;                /* A11 value, either 0x000 or 0x800 */

	DEVICE *    mem;
	DEVICE *    io;
	DEVICE *    intr;

	int         icount;

	UINT8       rom[0x1000];
//	UINT8       ram[0x100];
};

/* opcode table entry */
typedef int (*mcs48_ophandler)(mcs48_state *state);
extern const mcs48_ophandler _mcs48_opcode_table[256];

class VM;
class EMU;
class MCS48MEM : public DEVICE
{
private:
	uint8_t ram[0x100];
public:
	MCS48MEM(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(ram, 0, sizeof(ram));
		set_device_name(_T("MCS48 MEMORY BUS"));
	}
	~MCS48MEM() {}
	
	uint32_t read_data8(uint32_t addr)
	{
		return ram[addr & 0xff];
	}
	void write_data8(uint32_t addr, uint32_t data)
	{
		ram[addr & 0xff] = data;
	}
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

class MCS48_BASE : public DEVICE
{
protected:
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */
	const mcs48_ophandler *opcode_table;
	
	DEVICE *d_mem, *d_io, *d_intr;
	DEVICE *d_mem_stored, *d_io_stored;
	void *opaque;
public:
	MCS48_BASE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_mem = d_io = d_intr = NULL;
		opcode_table =  _mcs48_opcode_table;
		set_device_name(_T("MCS48 MCU"));
	}
	~MCS48_BASE() {}
	
	// common functions
	virtual void initialize();
	virtual void release();
	virtual void reset();
	virtual int run(int icount);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t get_pc();
	uint32_t get_next_pc();
	void write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t read_debug_data8(uint32_t addr);
	void write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t read_debug_io8(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	void get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);

	void save_state(FILEIO* state_state_fio);
	bool load_state(FILEIO* state_state_fio);
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_context_intr(DEVICE* device)
	{
		d_intr = device;
	}
	void load_rom_image(const _TCHAR *file_path);
	uint8_t *get_rom_ptr();
};

#endif

