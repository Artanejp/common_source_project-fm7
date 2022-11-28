/*
	Yuasa Kyouiku System YALKY Emulator 'eYALKY'

	Author : Takeda.Toshiya
	Date   : 2016.03.28-

	[ i/o ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IO_PORT_B	0
#define SIG_IO_PORT_C	1
#define SIG_IO_DREC_EAR	2

class DATAREC;

class IO : public DEVICE
{
private:
	DATAREC *d_drec;
	DEVICE *d_cpu, *d_pio;
	const uint8_t* key_stat;
	uint8_t *vram;
	uint8_t font[0x2000];
	
	uint8_t pb, pc, div_counter, counter;
	int posi_counter, nega_counter;
	bool drec_in, drec_toggle;
	uint32_t prev_clock;
	int register_id;
	
	void adjust_event_timing();
	void update_counter();
	void update_key();
	
public:
	IO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("I/O Bus"));
	}
	~IO() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_drec(DATAREC* device)
	{
		d_drec = device;
	}
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_vram_ptr(uint8_t *ptr)
	{
		vram = ptr;
	}
	void open_tape()
	{
		adjust_event_timing();
		div_counter = 0;
		drec_in = drec_toggle = false;
		prev_clock = 0;
	}
	void draw_screen();
};

#endif

