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
	uint32_t report;
	bool write_cr;
	
	struct {
		uint8_t code;
		uint8_t attr;
	} cvram[25][80];
	uint8_t gvram[480][640];
	
	int window_x0, window_y0, window_x1, window_y1;
	int view_x0, view_y0, view_x1, view_y1;
	
	double expand;
	int rotate;
	int translate_x, translate_y;
	int point_x, point_y;
	int fore_color, back_color;
	bool erase;
	int texture, texture_index;
	int pattern;
	
	uint8_t screen[480][640];
	uint8_t font[0x2000];
	scrntype_t palette_text[8];
	scrntype_t palette_graph[8];
	int blink;
	
	void process_cmd();
	void put_code(uint8_t data);
	uint8_t get_code();
	void scroll();
	void draw_text();
	
	void transform(double world_x, double world_y, double *x, double *y);
	void world_to_view(double world_x, double world_y, double *x, double *y);
	void view_to_vram(double view_x, double view_y, int *x, int *y);
	void transform_to_vram(double world_x, double world_y, int *x, int *y);
	
	void draw_solid_pixel(int x, int y);
	void draw_texture_pixel(int x, int y);
	void draw_pattern_pixel(int x, int y);
	void draw_solid_line(int x0, int y0, int x1, int y1);
	void draw_solid_cont_line(int x0, int y0, int x1, int y1);
	void draw_texture_line(int x0, int y0, int x1, int y1);
	void draw_texture_cont_line(int x0, int y0, int x1, int y1);
	void draw_char(int x, int y, int pow, int rot, int code);
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
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
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void draw_screen();
};

#endif
