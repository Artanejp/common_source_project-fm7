/*
	SHARP MZ-3500 Emulator 'EmuZ-3500'

	Author : Takeda.Toshiya
	Date   : 2010.08.31-

	[ main ]
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
	DEVICE *d_cpu, *d_subcpu, *d_fdc;
	
	uint8* rbank[32];	// 64KB / 2KB
	uint8* wbank[32];
	uint8 wdmy[0x800];
	uint8 rdmy[0x800];
	uint8 ipl[0x2000];
	uint8 ram[0x40000];
	uint8 common[0x800];
	uint8 basic[0x8000];
	uint8 ext[0x8000];
	
	uint8 ma, ms, mo;
	bool me1, me2;
	
	uint8 srqb, sres;
	bool sack, srdy;
	bool intfd, int0, int1, int2, int3, int4;
	bool me, e1;
	uint8 inp;
	bool motor, drq, index;
	
	void update_irq();
	void update_bank();
	
public:
	MAIN(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		intfd = int0 = int1 = int2 = int3 = int4 = false;
		me = e1 = false;
	}
	~MAIN() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_subcpu(DEVICE* device)
	{
		d_subcpu = device;
	}
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
	uint8 *get_ipl()
	{
		return ipl;
	}
	uint8 *get_common()
	{
		return common;
	}
};

#endif

