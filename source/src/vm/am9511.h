/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Origin : Xep128
	Date   : 2017.05.09-

	[ AM9511 ]
*/

#ifndef _AM9511_H_
#define _AM9511_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

class  DLL_PREFIX AM9511 : public DEVICE
{
private:
	uint8_t _apu_stack[16];
	int _apu_tos;
	uint8_t _apu_status;
	int register_id;

	double _APU_CLOCK;
	
	void apu_reset ( void );
	uint8_t apu_read_status( void );
	void _apu_move( int n);
	uint8_t _apu_look8(int depth);
	uint8_t _apu_pop8();
	uint8_t apu_read_data();
	void _apu_push8(uint8_t data);
	void apu_write_data(uint8_t data);
	int  _apu_pop_fix16(void);
	void _apu_push_fix16(int data);
	int64_t _apu_pop_fix32(void);
	void _apu_push_fix32(int64_t data);
	double _apu_pop_float();
	void _apu_push_float(double data);
	void  _apu_sz_fix16(void);
	void _apu_sz_fix32(void);
	void _apu_sz_float(void);
	void _apu_xchg(int d1, int d2);
	void _apu_copy(int from, int to);
	void _apu_carry ( int64_t val, int64_t limit );
	void apu_write_command ( uint8_t cmd );
	
public:
	AM9511(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		_APU_CLOCK = 1.0e6;
		set_device_name(_T("AM9511"));
	}
	~AM9511() {}
	
	// common functions
	void initialize() override;
	void reset() override;
	
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	
	void __FASTCALL event_callback(int event_id, int err) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
};

#endif

