/*
	SHARP MZ-3500 Emulator 'EmuZ-3500'

	Author : Takeda.Toshiya
	Date   : 2010.08.31-

	[ main pcb ]
*/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MAIN_SACK	0
#define SIG_MAIN_SRDY	1
#define SIG_MAIN_INTFD	2
#define SIG_MAIN_INT0	3
#define SIG_MAIN_INT1	4
#define SIG_MAIN_INT2	5
#define SIG_MAIN_INT3	6
#define SIG_MAIN_INT4	7
#define SIG_MAIN_DRQ	8
#define SIG_MAIN_INDEX	9

class MAIN : public DEVICE
{
private:
	DEVICE *d_maincpu, *d_subcpu, *d_fdc;
	
	uint8_t* rbank[32];	// 64KB / 2KB
	uint8_t* wbank[32];
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	uint8_t ipl[0x2000];
	uint8_t ram[0x40000];
	uint8_t common[0x800];
	uint8_t basic[0x8000];
	uint8_t ext[0x8000];
	
	uint8_t ma, ms, mo;
	bool me1, me2;
	
	uint8_t srqb, sres;
	bool sack, srdy;
	bool intfd, int0, int1, int2, int3, int4;
	bool me, e1;
	uint8_t inp;
	bool motor, drq, index;
	
	bool crt_400line;
	
	void update_irq();
	void update_bank();
	
public:
	MAIN(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		intfd = int0 = int1 = int2 = int3 = int4 = false;
		me = e1 = false;
		set_device_name(_T("Memory Bus (Main)"));
	}
	~MAIN() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	uint32_t fetch_op(uint32_t addr, int *wait);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_maincpu(DEVICE* device)
	{
		d_maincpu = device;
	}
	void set_context_subcpu(DEVICE* device)
	{
		d_subcpu = device;
	}
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
	uint8_t *get_ipl()
	{
		return ipl;
	}
	uint8_t *get_common()
	{
		return common;
	}
};

#endif

