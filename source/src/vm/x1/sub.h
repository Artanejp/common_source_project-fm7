/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2013.05.01-

	[ sub system ]
*/

#ifndef _SUB_H_
#define _SUB_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SUB_PIO_PORT_C	0
#define SIG_SUB_TAPE_END	1
#define SIG_SUB_TAPE_APSS	2

class DATAREC;

namespace X1 {

class SUB : public DEVICE
{
private:
	DEVICE *d_pio, *d_rtc;
	DATAREC *d_drec;
	
	uint8_t p1_out, p1_in, p2_out, p2_in;
	uint8_t portc;
	bool tape_play, tape_rec, tape_eot, tape_apss;
	void update_tape();
	
	// interrupt
	bool intr, obf;
	
	// z80 daisy chain
	DEVICE *d_cpu;
	bool iei;
	uint32_t intr_bit;
	void update_intr() override;
	
public:
	SUB(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Sub System"));
	}
	~SUB() {}
	
	// common functions
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// interrupt common functions
	void set_context_intr(DEVICE* device, uint32_t bit) override
	{
		d_cpu = device;
		intr_bit = bit;
	}
	void __FASTCALL set_intr_iei(bool val) override;
	uint32_t get_intr_ack() override;
	void notify_intr_reti() override;
	
	// unique functions
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
	void set_context_drec(DATAREC* device)
	{
		d_drec = device;
	}
	void play_tape(bool value);
	void rec_tape(bool value);
	void close_tape();
	uint32_t rom_crc32;
};

}
#endif

