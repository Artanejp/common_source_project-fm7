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
#define SIG_IO_DREC_ROT	2

class IO : public DEVICE
{
private:
	DEVICE *d_drec, *d_pio;
	const uint8_t* key_stat;
	uint8_t *vram;
	uint8_t font[0x2000];
	
	uint8_t pb, pc, counter;
	
	void update_key();
	
public:
	IO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~IO() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_vram_ptr(uint8_t *ptr)
	{
		vram = ptr;
	}
	void draw_screen();
};

#endif

