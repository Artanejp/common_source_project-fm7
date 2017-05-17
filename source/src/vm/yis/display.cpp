/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2017.04.20-

	[ display ]
*/

#include "display.h"
#include "../../fifo.h"

//		Char	Graph10	Graph20	English
// PU-1-10	O	X	X	X
// PU-1-20	O	X	O	O
// PU-10	O	O	X	X

void DISPLAY::initialize()
{
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font + 0x000, 0x800, 1);
		memcpy(font + 0x800, font, 0x800);
		fio->Fread(font + 0x800, 0x800, 1);
		fio->Fclose();
	}
	delete fio;
	
	// create pc palette
	for(int i = 0; i < 8; i++) {
		palette_pc[i] = RGB_COLOR((i & 1) ? 255 : 0, (i & 2) ? 255 : 0, (i & 4) ? 255 : 0);
	}
	
	cmd_buffer = new FIFO(32);
	dpp_ctrl = 0xff;
	
	// register event
	register_frame_event(this);
}

void DISPLAY::release()
{
	cmd_buffer->release();
	delete cmd_buffer;
}

void DISPLAY::reset()
{
	switch(monitor_type = config.monitor_type) {
	case 0: dpp_data = 0x43; break; // PU-1-10
	case 1: dpp_data = 0x47; break; // PU-1-20
	case 2: dpp_data = 0x5a; break; // PU-10 ???
	}
	for(int y = 0; y < 25; y++) {
		for(int x = 0; x < 80; x++) {
			cvram[y][x].code = 0x20;
			cvram[y][x].attr = 0x0e;
		}
	}
	cmd_buffer->clear();
	active_cmd = -1;
	
	scroll_x0 = 0;
	scroll_y0 = 0;
	scroll_x1 = 79;
	scroll_y1 = 24;
	cursor_x = cursor_y = 0;
	read_x = read_y = 0;
	mode1 = 0x0e;
	mode2 = 0x30;
	mode3 = 0x01;
	write_cr = false;
}

void DISPLAY::event_frame()
{
	blink = (blink + 1) & 0x1f;
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0xf040: // CAP: WRITE DATA
	case 0xf048: // GRP: WRITE DATA
		if(active_cmd == 0x18) {
			emu->out_debug_log(_T("Send GPP: %02X\n"), data);
			if(data == 0x00) {
				cmd_buffer->clear();
				active_cmd = -1;
				emu->out_debug_log(_T("\n"));
			} else {
				put_code(data);
			}
		} else {
			if(active_cmd != -1) {
				cmd_buffer->clear();
				active_cmd = -1;
				emu->out_debug_log(_T("\n"));
			}
			emu->out_debug_log(_T("Send GPP: %02X\n"), data);
			cmd_buffer->write(data);
			process_cmd();
		}
		break;
	case 0xf041: // CAP: CONTROL
	case 0xf049: // GRP: CONTROL
		if(!(dpp_ctrl & 1) && (data & 1)) {
			// bit0: L->H RESET CAP/GRP
			reset();
		}
		dpp_ctrl = data;
		break;
	}
}

uint32_t DISPLAY::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0xf041: // CAP: WRITE DATA READY
	case 0xf049: // GRP: WRITE DATA READY
		// bit0: H=READY
		return 0x01;
	case 0xf042: // CAP: READ DATA
	case 0xf04a: // GRP: READ DATA
		if(active_cmd == 0x1a) {
			dpp_data = get_code();
		} else if(active_cmd == 0x1e) {
			dpp_data = report & 0xff;
			report >>= 8;
		}
		emu->out_debug_log(_T("Recv GPP: %02X\n"), dpp_data);
		return dpp_data;
	case 0xf043: // CAP: READ DATA READY
	case 0xf04b: // GRP: READ DATA READY
		// bit0: H=READY
		return 0x01;
	}
	return 0xff;
}

void DISPLAY::process_cmd()
{
	switch(cmd_buffer->read_not_remove(0)) {
	case 0x18:
		if(cmd_buffer->count() == 1) {
			active_cmd = cmd_buffer->read_not_remove(0);
		}
		break;
	case 0x19:
		if(cmd_buffer->count() == 3) {
			cursor_x = cmd_buffer->read_not_remove(1) & 0x7f;
			cursor_y = cmd_buffer->read_not_remove(2) & 0x1f;
			active_cmd = cmd_buffer->read_not_remove(0);
		}
		break;
	case 0x1a:
		if(cmd_buffer->count() == 1) {
			read_x = cursor_x;
			read_y = cursor_y;
			active_cmd = cmd_buffer->read_not_remove(0);
		}
		break;
	case 0x1b:
		if(cmd_buffer->count() == 5) {
			scroll_x0 = cmd_buffer->read_not_remove(1) & 0x7f;
			scroll_y0 = cmd_buffer->read_not_remove(2) & 0x1f;
			scroll_x1 = cmd_buffer->read_not_remove(3) & 0x7f;
			scroll_y1 = cmd_buffer->read_not_remove(4) & 0x1f;
			active_cmd = cmd_buffer->read_not_remove(0);
		}
		break;
	case 0x1c:
		if(cmd_buffer->count() == 5) {
			int x = cmd_buffer->read_not_remove(1) & 0x7f;
			int y = cmd_buffer->read_not_remove(2) & 0x1f;
			cvram[y % 25][x % 80].code = cmd_buffer->read_not_remove(3);
			cvram[y % 25][x % 80].attr = cmd_buffer->read_not_remove(4);
			active_cmd = cmd_buffer->read_not_remove(0);
		}
		break;
	case 0x1d:
		if(cmd_buffer->count() == 7) {
			int x0 = cmd_buffer->read_not_remove(1) & 0x7f;
			int y0 = cmd_buffer->read_not_remove(2) & 0x1f;
			int x1 = cmd_buffer->read_not_remove(3) & 0x7f;
			int y1 = cmd_buffer->read_not_remove(4) & 0x1f;
			for(int y = y0; y <= y1 && y <= 24; y++) {
				for(int x = x0; x <= x1 && x <= 79; x++) {
					cvram[y % 25][x % 80].code = cmd_buffer->read_not_remove(5);
					cvram[y % 25][x % 80].attr = cmd_buffer->read_not_remove(6);
				}
			}
			cursor_x = cursor_y = 0;
			write_cr = false;
			active_cmd = cmd_buffer->read_not_remove(0);
		}
		break;
	case 0x1e:
		if(cmd_buffer->count() == 3) {
			int x = cmd_buffer->read_not_remove(1) & 0x7f;
			int y = cmd_buffer->read_not_remove(2) & 0x1f;
			report  = cvram[y % 25][x % 80].code << 0;
			report |= cvram[y % 25][x % 80].attr << 8;
			active_cmd = cmd_buffer->read_not_remove(0);
		}
		break;
	case 0x1f:
		if(cmd_buffer->count() == 4) {
			mode1 = cmd_buffer->read_not_remove(1);
			mode2 = cmd_buffer->read_not_remove(2);
			mode3 = cmd_buffer->read_not_remove(3);
			active_cmd = cmd_buffer->read_not_remove(0);
		}
		break;
	default:
		emu->out_debug_log(_T("Unknown GPP: %02X\n\n"), cmd_buffer->read_not_remove(0));
		cmd_buffer->clear();
		break;
	}
}

void DISPLAY::put_code(uint8_t data)
{
	switch(data) {
	case 0x00:
		break;
	case 0x08: // BS
	case 0x1d:
		if(--cursor_x < scroll_x0) {
			if(--cursor_y < scroll_y0) {
				cursor_x = scroll_x0;
				cursor_y = scroll_y0;
			} else {
				cursor_x = scroll_x1;
			}
		}
		break;
	case 0x0a: // LF
		if(++cursor_y > scroll_y1) {
			cursor_y = scroll_y1;
			scroll();
		}
		break;
	case 0x0d: // CR
		cvram[cursor_y % 25][cursor_x % 80].code = data;
		cvram[cursor_y % 25][cursor_x % 80].attr = mode1;
		cursor_x = scroll_x0;
		write_cr = true;
		break;
	case 0x10: // FS
	case 0x1c: // FS
		if(++cursor_x > scroll_x1) {
			if(++cursor_y > scroll_y1) {
				cursor_y = scroll_y1;
				scroll();
			}
			cursor_x = scroll_x0;
		}
		break;
	case 0x1e: // UP
		if(--cursor_y < scroll_y0) {
			cursor_y = scroll_y0;
		}
		break;
	case 0x1f: // DOWN
		if(++cursor_y > scroll_y1) {
			cursor_y = scroll_y1;
		}
		break;
	default:
		cvram[cursor_y % 25][cursor_x % 80].code = data;
		cvram[cursor_y % 25][cursor_x % 80].attr = mode1;
		if(++cursor_x > scroll_x1) {
			if(++cursor_y > scroll_y1) {
				cursor_y = scroll_y1;
				scroll();
			}
			cursor_x = scroll_x0;
		}
		break;
	}
}

uint8_t DISPLAY::get_code()
{
	uint8_t data = 0x0d;
	
	if(write_cr) {
		if((data = cvram[read_y % 25][read_x % 80].code) != 0x0d) {
			if(++read_x > scroll_x1) {
				if(++read_y > scroll_y1) {
					write_cr = false; // ???
				}
				read_x = scroll_x0;
			}
		} else {
			write_cr = false;
		}
	}
	return data;
}

void DISPLAY::scroll()
{
	for(int y = scroll_y0; y <= scroll_y1 - 1; y++) {
		for(int x = scroll_x0; x <= scroll_x1; x++) {
			cvram[y % 25][x % 80].code = cvram[(y + 1) % 25][x % 80].code;
			cvram[y % 25][x % 80].attr = cvram[(y + 1) % 25][x % 80].attr;
		}
	}
	for(int x = scroll_x0; x <= scroll_x1; x++) {
		cvram[scroll_y1 % 25][x % 80].code = 0x20;
		cvram[scroll_y1 % 25][x % 80].attr = mode1;
	}
}

void DISPLAY::draw_screen()
{
	// render screen
	memset(screen, 0, sizeof(screen));
	draw_text();
	
	// copy to real screen
	for(int y = 0; y < 200; y++) {
		scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0);
		scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1);
		uint8_t* src = screen[y];
		
		for(int x = 0; x < 640; x++) {
			dest0[x] = palette_pc[src[x] & 7];
		}
		if(config.scan_line) {
//			for(int x = 0; x < 640; x++) {
//				dest1[x] = palette_pc[0];
//			}
			memset(dest1, 0, 640 * sizeof(scrntype_t));
		} else {
			memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
		}
	}
	emu->screen_skip_line(true);
}

void DISPLAY::draw_text()
{
	for(int y = 0; y < 25; y++) {
		for(int x = 0; x < 80; x++) {
			uint8_t code = cvram[y][x].code;
			uint8_t fore = (mode3 & 1) ? (cvram[y][x].attr >> 1) & 7 : 7;
			uint8_t back = (mode3 & 1) ? (cvram[y][x].attr >> 5) & 7 : 0;
			
			if(fore == back) {
				fore = 7 - back;
			}
			for(int l = 0; l < 8; l++) {
				uint8_t pat = (mode3 & 4) ? 0 : font[((mode3 & 2) ? 0x800 : 0) + (code << 3) + l];
				uint8_t* d = &screen[(y << 3) + l][x << 3];
				
				d[0] = (pat & 0x80) ? fore : back;
				d[1] = (pat & 0x40) ? fore : back;
				d[2] = (pat & 0x20) ? fore : back;
				d[3] = (pat & 0x10) ? fore : back;
				d[4] = (pat & 0x08) ? fore : back;
				d[5] = (pat & 0x04) ? fore : back;
				d[6] = (pat & 0x02) ? fore : back;
				d[7] = (pat & 0x01) ? fore : back;
			}
			if(x == cursor_x && y == cursor_y && (mode2 & 0x10) && (blink & 0x10)) {
				if(mode2 & 0x80) {
					for(int l = 0; l < 8; l++) {
						uint8_t* d = &screen[(y << 3) + l][x << 3];
						for(int c = 0; c < 8; c++) {
							d[c] = 7 - d[c];
						}
					}
				} else {
					for(int l = 7; l < 8; l++) {
						uint8_t* d = &screen[(y << 3) + l][x << 3];
						memset(d, 7, 8);
					}
				}
			}
		}
	}
}

#define STATE_VERSION	1

void DISPLAY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	cmd_buffer->save_state((void *)state_fio);
	state_fio->FputInt32(active_cmd);
	state_fio->FputUint8(dpp_data);
	state_fio->FputUint8(dpp_ctrl);
	state_fio->FputInt32(scroll_x0);
	state_fio->FputInt32(scroll_y0);
	state_fio->FputInt32(scroll_x1);
	state_fio->FputInt32(scroll_y1);
	state_fio->FputInt32(cursor_x);
	state_fio->FputInt32(cursor_y);
	state_fio->FputInt32(read_x);
	state_fio->FputInt32(read_y);
	state_fio->FputUint8(mode1);
	state_fio->FputUint8(mode2);
	state_fio->FputUint8(mode3);
	state_fio->FputUint16(report);
	state_fio->FputBool(write_cr);
	state_fio->Fwrite(cvram, sizeof(cvram), 1);
	state_fio->FputInt32(blink);
}

bool DISPLAY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	if(!cmd_buffer->load_state((void *)state_fio)) {
		return false;
	}
	active_cmd = state_fio->FgetInt32();
	dpp_data = state_fio->FgetUint8();
	dpp_ctrl = state_fio->FgetUint8();
	scroll_x0 = state_fio->FgetInt32();
	scroll_y0 = state_fio->FgetInt32();
	scroll_x1 = state_fio->FgetInt32();
	scroll_y1 = state_fio->FgetInt32();
	cursor_x = state_fio->FgetInt32();
	cursor_y = state_fio->FgetInt32();
	read_x = state_fio->FgetInt32();
	read_y = state_fio->FgetInt32();
	mode1 = state_fio->FgetUint8();
	mode2 = state_fio->FgetUint8();
	mode3 = state_fio->FgetUint8();
	report = state_fio->FgetUint16();
	write_cr = state_fio->FgetBool();
	state_fio->Fread(cvram, sizeof(cvram), 1);
	blink = state_fio->FgetInt32();
	return true;
}

