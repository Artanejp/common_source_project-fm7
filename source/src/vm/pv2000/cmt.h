/*
	CASIO PV-2000 Emulator 'EmuGaki'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ cmt ]
*/

#ifndef _CMT_H_
#define _CMT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FILEIO;

#define BUFFER_SIZE 0x10000

class CMT : public DEVICE
{
private:
	FILEIO* fio;
	int bufcnt;
	uint8_t buffer[BUFFER_SIZE];
	bool play, rec;
	uint8_t start, bit;
	
public:
	CMT(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("CMT I/F"));
	}
	~CMT() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted()
	{
		return (play || rec);
	}
	bool skip() { return (start & 1) && (play || rec); }
};

#endif

