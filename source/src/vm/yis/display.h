/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2017.04.20-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FIFO;
class MEMORY;

class DISPLAY : public DEVICE
{
private:
	int monitor_type;
	
	FIFO* cmd_buffer;
	int active_cmd;
	uint8_t dpp_data, dpp_ctrl;
	
	int scroll_x0, scroll_y0, scroll_x1, scroll_y1;
	int cursor_x, cursor_y;
	int read_x, read_y;
	uint8_t mode1, mode2, mode3;
	uint16_t report;
	bool write_cr;
	
	struct {
		uint8_t code;
		uint8_t attr;
	} cvram[25][80];
	
	uint8_t screen[200][640];
	uint8_t font[0x1000];
	scrntype_t palette_pc[8];
	int blink;
	
	void process_cmd();
	void put_code(uint8_t data);
	uint8_t get_code();
	void scroll();
	void draw_text();
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void draw_screen();
};

#endif
