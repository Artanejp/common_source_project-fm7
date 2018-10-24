/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.08.03-

	[ mc6847 ]
*/

#include "mc6847.h"

#define LIGHTGREEN	0
#define YELLOW		1
#define BLUE		2
#define RED		3
#define WHITE		4
#define CYAN		5
#define MAGENTA		6
#define ORANGE		7
#define BLACK		8
// text
#define GREEN		9
#define BEIGE		10
// phc20
#define GRAY		11

void MC6847_BASE::initialize()
{
	DEVICE::initialize();
}

void MC6847_BASE::reset()
{
	vsync = hsync = disp = true;
}

void MC6847_BASE::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_MC6847_AG:
		ag = ((data & mask) != 0);
		break;
	case SIG_MC6847_AS:
		as = ((data & mask) != 0);
		break;
	case SIG_MC6847_INTEXT:
		intext = ((data & mask) != 0);
		break;
	case SIG_MC6847_GM:
		gm = (gm & ~mask) | (data & mask);
		break;
	case SIG_MC6847_CSS:
		css = ((data & mask) != 0);
		break;
	case SIG_MC6847_INV:
		inv = ((data & mask) != 0);
		break;
	case SIG_MC6847_ENABLE:
		disabled = ((data & mask) == 0);
		break;
	case SIG_MC6847_DISABLE:
		disabled = ((data & mask) != 0);
		break;
	}
}

void MC6847_BASE::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	// this should be called before vline event
	tWHS = (int)((double)new_clocks / new_frames_per_sec / (double)new_lines_per_frame * 16.5 / 227.5 + 0.5);
}

void MC6847_BASE::event_vline(int v, int clock)
{
	// vsync
	set_vsync(v > 32);	// 32/262
	
	// hsync
	set_hsync(false);
	register_event_by_clock(this, 0, tWHS, false, NULL);
}

void MC6847_BASE::event_callback(int event_id, int err)
{
	set_hsync(true);
}

void MC6847_BASE::set_vsync(bool val)
{
	if(vsync != val) {
		write_signals(&outputs_vsync, val ? 0xffffffff : 0);
		vsync = val;
		set_disp(vsync && hsync);
	}
}

void MC6847_BASE::set_hsync(bool val)
{
	if(hsync != val) {
		write_signals(&outputs_hsync, val ? 0xffffffff : 0);
		hsync = val;
		set_disp(vsync && hsync);
	}
}

void MC6847_BASE::set_disp(bool val)
{
	if(disp != val) {
		if(d_cpu != NULL && !disabled) {
			d_cpu->write_signal(SIG_CPU_BUSREQ, val ? 1 : 0, 1);
		}
		disp = val;
	}
}

void MC6847_BASE::load_font_image(const _TCHAR *file_path)
{
	// external font
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(extfont, sizeof(extfont), 1);
		fio->Fclose();
	}
	delete fio;
}

void MC6847_BASE::draw_screen()
{
	// render screen
	if(disabled) {
		memset(screen, 0, sizeof(screen));
	} else if(ag) {
		// graphics mode
		switch(gm) {
		case 0: draw_cg(4, 3); break;	//  64x 64
		case 1: draw_rg(2, 3); break;	// 128x 64
		case 2: draw_cg(2, 3); break;	// 128x 64
		case 3: draw_rg(2, 2); break;	// 128x 96
		case 4: draw_cg(2, 2); break;	// 128x 96
		case 5: draw_rg(2, 1); break;	// 128x192
		case 6: draw_cg(2, 1); break;	// 128x192
		case 7: draw_rg(1, 1); break;	// 256x192
		}
	} else {
		// alphanumerics / semigraphics
		draw_alpha();
	}
	
	// copy to screen
	for(int y = 0; y < 192; y++) {
		scrntype_t* dest = osd->get_vm_screen_buffer(y);
		for(int x = 0; x < 256; x++) {
			dest[x] = palette_pc[screen[y][x]];
		}
	}
}

void MC6847_BASE::draw_cg(int xofs, int yofs)
{
	uint8_t color = css ? 4 : 0;
	int ofs = 0;
	
	for(int y = 0; y < 192; y += yofs) {
		for(int x = 0; x < 256; x += xofs * 4) {
			uint8_t data = vram_ptr[ofs];
			if(++ofs >= vram_size) {
				ofs = 0;
			}
			uint8_t* dest = &screen[y][x];
			
			if(xofs == 4) {
				dest[ 0] = dest[ 1] = dest[ 2] = dest[ 3] = color | ((data >> 6) & 3);
				dest[ 4] = dest[ 5] = dest[ 6] = dest[ 7] = color | ((data >> 4) & 3);
				dest[ 8] = dest[ 9] = dest[10] = dest[11] = color | ((data >> 2) & 3);
				dest[12] = dest[13] = dest[14] = dest[15] = color | ((data >> 0) & 3);
			} else {
				dest[0] = dest[1] = color | ((data >> 6) & 3);
				dest[2] = dest[3] = color | ((data >> 4) & 3);
				dest[4] = dest[5] = color | ((data >> 2) & 3);
				dest[6] = dest[7] = color | ((data >> 0) & 3);
			}
		}
		if(yofs >= 2) {
			my_memcpy(screen[y + 1], screen[y], 256);
			if(yofs >= 3) {
				my_memcpy(screen[y + 2], screen[y], 256);
			}
		}
	}
}

void MC6847_BASE::draw_rg(int xofs, int yofs)
{
	static const uint8_t color_table[4] = {
		GREEN, LIGHTGREEN, BLACK, WHITE
	};
	static const uint8_t color_table2[4] = {
		BLACK, BLACK, CYAN, WHITE
	};
	static const uint8_t color_table3[4] = {
		BLACK, ORANGE, BLACK, WHITE
	};
	uint8_t color = css ? 2 : 0;
	int ofs = 0;
	
	for(int y = 0; y < 192; y += yofs) {
		for(int x = 0; x < 256; x += xofs * 8) {
			uint8_t data = vram_ptr[ofs];
			if(++ofs >= vram_size) {
				ofs = 0;
			}
			uint8_t* dest = &screen[y][x];
			
			if(xofs == 2) {
				dest[ 0] = dest[ 1] = color_table[color | ((data >> 7) & 1)];
				dest[ 2] = dest[ 3] = color_table[color | ((data >> 6) & 1)];
				dest[ 4] = dest[ 5] = color_table[color | ((data >> 5) & 1)];
				dest[ 6] = dest[ 7] = color_table[color | ((data >> 4) & 1)];
				dest[ 8] = dest[ 9] = color_table[color | ((data >> 3) & 1)];
				dest[10] = dest[11] = color_table[color | ((data >> 2) & 1)];
				dest[12] = dest[13] = color_table[color | ((data >> 1) & 1)];
				dest[14] = dest[15] = color_table[color | ((data >> 0) & 1)];
			} else if(css) {
				// color bleed in black/white pattern
				dest[0] = color_table2[(data >> 6) & 3];
				dest[1] = color_table3[(data >> 6) & 3];
				dest[2] = color_table2[(data >> 4) & 3];
				dest[3] = color_table3[(data >> 4) & 3];
				dest[4] = color_table2[(data >> 2) & 3];
				dest[5] = color_table3[(data >> 2) & 3];
				dest[6] = color_table2[(data >> 0) & 3];
				dest[7] = color_table3[(data >> 0) & 3];
			} else {
				dest[0] = color_table[(data >> 7) & 1];
				dest[1] = color_table[(data >> 6) & 1];
				dest[2] = color_table[(data >> 5) & 1];
				dest[3] = color_table[(data >> 4) & 1];
				dest[4] = color_table[(data >> 3) & 1];
				dest[5] = color_table[(data >> 2) & 1];
				dest[6] = color_table[(data >> 1) & 1];
				dest[7] = color_table[(data >> 0) & 1];
			}
		}
		if(yofs >= 2) {
			my_memcpy(screen[y + 1], screen[y], 256);
			if(yofs >= 3) {
				my_memcpy(screen[y + 2], screen[y], 256);
			}
		}
	}
}

void MC6847_BASE::draw_alpha()
{
}

#define STATE_VERSION	1

bool MC6847_BASE::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateArray(sg4, sizeof(sg4), 1);
	state_fio->StateArray(sg6, sizeof(sg6), 1);
	state_fio->StateValue(ag);
	state_fio->StateValue(as);
	state_fio->StateValue(intext);
	state_fio->StateValue(gm);
	state_fio->StateValue(css);
	state_fio->StateValue(inv);
	state_fio->StateValue(vsync);
	state_fio->StateValue(hsync);
	state_fio->StateValue(disp);
	state_fio->StateValue(tWHS);
	state_fio->StateValue(disabled);
 	return true;
}
 
