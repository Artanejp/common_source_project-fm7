/*
	Systems Formulate BUBCOM80 Emulator 'eBUBCOM80'

	Author : Takeda.Toshiya
	Date   : 2018.05.08-

	[ display ]
*/

#include "display.h"
#include "cmt.h"
#include "../pcm1bit.h"
#include "../z80.h"

#define EVENT_BUSREQ	0

void DISPLAY::initialize()
{
	for(int i = 0; i < 256; i++) {
		uint8_t *dest = sg_pattern + 8 * i;
		dest[0] = dest[1] = ((i & 0x01) ? 0xf0 : 0) | ((i & 0x02) ? 0x0f : 0);
		dest[2] = dest[3] = ((i & 0x04) ? 0xf0 : 0) | ((i & 0x08) ? 0x0f : 0);
		dest[4] = dest[5] = ((i & 0x10) ? 0xf0 : 0) | ((i & 0x20) ? 0x0f : 0);
		dest[6] = dest[7] = ((i & 0x40) ? 0xf0 : 0) | ((i & 0x80) ? 0x0f : 0);
	}
	memset(font, 0x00, sizeof(font));
	memset(vram, 0x00, sizeof(vram));
	
	for(int i = 0; i < 8; i++) {
		palette_text_pc [i] = RGBA_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0, 255); // A is a flag for crt filter
		palette_graph_pc[i] = RGBA_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0,   0);
	}
	color = true;
	width40 = false;
	mode = 0x80;
	
	register_frame_event(this);
	register_vline_event(this);
}

#define STORE_DMAC_CONTEXTS() \
	DEVICE *d_mem = dmac.mem; \
	DEVICE *d_ch0 = dmac.ch[0].io; \
	DEVICE *d_ch1 = dmac.ch[1].io; \
	DEVICE *d_ch2 = dmac.ch[2].io; \
	DEVICE *d_ch3 = dmac.ch[3].io

#define RESTORE_DMAC_CONTEXTS() \
	dmac.mem = d_mem; \
	dmac.ch[0].io = d_ch0; \
	dmac.ch[1].io = d_ch1; \
	dmac.ch[2].io = d_ch2; \
	dmac.ch[3].io = d_ch3;

void DISPLAY::reset()
{
	memset(&crtc, 0, sizeof(crtc));
	crtc.reset();
	update_timing();
	
	STORE_DMAC_CONTEXTS();
	memset(&dmac, 0, sizeof(dmac));
	RESTORE_DMAC_CONTEXTS();
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x10:
	case 0x12: // not full-decoded ?
	case 0x14:
	case 0x16:
	case 0x18:
	case 0x1a:
	case 0x1c:
	case 0x1e:
		crtc.write_param(data);
		if(crtc.timing_changed) {
			update_timing();
			crtc.timing_changed = false;
		}
		break;
	case 0x11:
	case 0x13: // not full-decoded ?
	case 0x15:
	case 0x17:
	case 0x19:
	case 0x1b:
	case 0x1d:
	case 0x1f:
		crtc.write_cmd(data);
		break;
	case 0x20:
		d_prn->write_signal(SIG_PRINTER_DATA, data, 0xff);
		break;
	case 0x50:
		d_cmt->write_signal(SIG_CMT_REMOTE, data, 0x01);
		color = ((data & 0x04) != 0);
		width40 = ((data & 0x08) != 0);
		d_pcm->write_signal(SIG_PCM1BIT_ON, data, 0x10);
		d_prn->write_signal(SIG_PRINTER_STROBE, data, 0x20);
		break;
	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
	case 0x68:
	case 0x69: // not full-decoded ?
	case 0x6a:
	case 0x6b:
	case 0x6c:
	case 0x6d:
	case 0x6e:
	case 0x6f:
		dmac.write_io8(addr, data);
		break;
	case 0x3ff0:
		mode = data;
		break;
	default:
		if(addr >= 0x800 && addr < 0x1000) {
			font[addr & 0x7ff] = data;
		} else if(addr >= 0x4000 && addr < 0x10000) {
			vram[addr] = data;
		}
		break;
	}
}

uint32_t DISPLAY::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x10:
	case 0x12: // not full-decoded ?
	case 0x14:
	case 0x16:
	case 0x18:
	case 0x1a:
	case 0x1c:
	case 0x1e:
		return crtc.read_param();
	case 0x11:
	case 0x13: // not full-decoded ?
	case 0x15:
	case 0x17:
	case 0x19:
	case 0x1b:
	case 0x1d:
	case 0x1f:
		return crtc.read_status();
	case 0x20:
		return d_prn->read_signal(SIG_PRINTER_BUSY) & 0x40;
	case 0x50:
		// bit1: 1=WIDTH80, 0=WIDTH40
		return 0xff; // dipswitch???
	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
	case 0x68:
	case 0x69: // not full-decoded ?
	case 0x6a:
	case 0x6b:
	case 0x6c:
	case 0x6d:
	case 0x6e:
	case 0x6f:
		return dmac.read_io8(addr);
	case 0x3ff0:
		return (crtc.vblank ? 0x40 : 0);
	default:
		if(addr >= 0x800 && addr < 0x1000) {
			return font[addr & 0x7ff];
		} else if(addr >= 0x4000 && addr < 0x10000) {
			return vram[addr];
		}
		break;
	}
	return 0x0f;
}

void DISPLAY::write_dma_io8(uint32_t addr, uint32_t data)
{
	// to crtc
	crtc.write_buffer(data);
}

void DISPLAY::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_DISPLAY_DMAC_CH0:
	case SIG_DISPLAY_DMAC_CH1:
	case SIG_DISPLAY_DMAC_CH2:
	case SIG_DISPLAY_DMAC_CH3:
		if(data & mask) {
			if(!dmac.ch[id].running) {
				dmac.start(id);
			}
			dmac.run(id);
		}
		break;
	}
}

void DISPLAY::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_BUSREQ:
		d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
		break;
	}
}

void DISPLAY::event_frame()
{
	crtc.update_blink();
}

void DISPLAY::event_vline(int v, int clock)
{
	int disp_line = crtc.height * crtc.char_height;
	
	if(v == 0) {
		if(crtc.status & 0x10) {
			// start dma transfer to crtc
			dmac.start(2);
			if(!dmac.ch[2].running) {
				// dma underrun occurs !!!
				crtc.status |= 8;
//				crtc.status &= ~0x10;
			} else {
				crtc.status &= ~8;
			}
			// dma wait cycles
			// from memory access test on PC-8801MA2 (XM8 version 1.20)
			busreq_clocks = (int)((double)(dmac.ch[2].count.sd + 1) * 5.95 / (double)disp_line + 0.5);
		}
		crtc.start();
	}
	if(v < disp_line) {
		if(/*(crtc.status & 0x10) && */dmac.ch[2].running) {
			// bus request
			d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
			register_event_by_clock(this, EVENT_BUSREQ, busreq_clocks, false, NULL);
			// run dma transfer to crtc
			if((v % crtc.char_height) == 0) {
				for(int i = 0; i < 80 + crtc.attrib.num * 2; i++) {
					dmac.run(2);
				}
			}
		}
	} else if(v == disp_line) {
		if(/*(crtc.status & 0x10) && */dmac.ch[2].running) {
			dmac.finish(2);
		}
		crtc.expand_buffer();
		crtc.finish();
	}
}

void DISPLAY::update_timing()
{
	int lines_per_frame = (crtc.height + crtc.vretrace) * crtc.char_height;
	double frames_per_sec = 15980.0 / (double)lines_per_frame;
	
	set_frames_per_sec(frames_per_sec);
	set_lines_per_frame(lines_per_frame);
}

void DISPLAY::draw_screen()
{
	draw_text();
	draw_graph();
	
	for(int y = 0; y < 200; y++) {
		scrntype_t* dest0 = emu->get_screen_buffer(y * 2);
		scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1);
		uint8_t* src_t = text[y];
		uint8_t* src_g = graph[y];
		
		for(int x = 0; x < 640; x++) {
			uint32_t t = src_t[x];
			uint32_t g = src_g[x];
			dest0[x] = t ? palette_text_pc[t & 7] : palette_graph_pc[g ? g : (mode & 7)];
		}
		if(config.scan_line) {
			memset(dest1, 0, 640 * sizeof(scrntype_t));
		} else {
			for(int x = 0; x < 640; x++) {
				dest1[x] = dest0[x];
			}
		}
	}
	emu->screen_skip_line(true);
}

/*
	attributes:
	
	bit7: graph=1/character=0
	bit6: green
	bit5: red
	bit4: blue
	bit3: under line
	bit2: upper line
	bit1: secret
	bit0: reverse
*/

void DISPLAY::draw_text()
{
	if(emu->now_waiting_in_debugger) {
		// dmac.run
		uint8_t buffer[120 * 200];
		memset(buffer, 0, sizeof(buffer));
		
		for(int i = 0; i < dmac.ch[3].count.sd + 1; i++) {
			buffer[i] = dmac.mem->read_dma_data8(dmac.ch[3].addr.w.l + i);
		}
		
		// crtc.expand_buffer
		for(int cy = 0, ofs = 0; cy < crtc.height; cy++, ofs += 80 + crtc.attrib.num * 2) {
			for(int cx = 0; cx < crtc.width; cx++) {
				crtc.text.expand[cy][cx] = buffer[ofs + cx];
			}
		}
		if(crtc.mode & 4) {
			// non transparent
			for(int cy = 0, ofs = 0; cy < crtc.height; cy++, ofs += 80 + crtc.attrib.num * 2) {
				for(int cx = 0; cx < crtc.width; cx += 2) {
					crtc.set_attrib(buffer[ofs + cx + 1]);
					crtc.attrib.expand[cy][cx] = crtc.attrib.expand[cy][cx + 1] = crtc.attrib.data;
				}
			}
		} else {
			// transparent
			if(crtc.mode & 1) {
				memset(crtc.attrib.expand, 0xe0, sizeof(crtc.attrib.expand));
			} else {
				for(int cy = 0, ofs = 0; cy < crtc.height; cy++, ofs += 80 + crtc.attrib.num * 2) {
					uint8_t flags[128];
					memset(flags, 0, sizeof(flags));
					for(int i = 2 * (crtc.attrib.num - 1); i >= 0; i -= 2) {
						flags[buffer[ofs + i + 80] & 0x7f] = 1;
					}
					crtc.attrib.data &= 0xf3; // for PC-8801mkIIFR 付属デモ
					
					for(int cx = 0, pos = 0; cx < crtc.width; cx++) {
						if(flags[cx]) {
							crtc.set_attrib(buffer[ofs + pos + 81]);
							pos += 2;
						}
						crtc.attrib.expand[cy][cx] = crtc.attrib.data;
					}
				}
			}
		}
		if(crtc.cursor.x < 80 && crtc.cursor.y < 200) {
			if((crtc.cursor.type & 1) && crtc.blink.cursor) {
				// no cursor
			} else {
				static const uint8_t ctype[5] = {0, 8, 8, 1, 1};
				crtc.attrib.expand[crtc.cursor.y][crtc.cursor.x] ^= ctype[crtc.cursor.type + 1];
			}
		}
	} else {
		if(crtc.status & 0x88) {
			// dma underrun
			crtc.status &= ~0x80;
			memset(crtc.text.expand, 0, 200 * 80);
			memset(crtc.attrib.expand, crtc.reverse ? 3 : 2, 200 * 80);
		}
		// for Advanced Fantasian Opening (20line) (XM8 version 1.00)
		if(!(crtc.status & 0x10)) {
//		if(!(crtc.status & 0x10) || (crtc.status & 8)) {
			memset(crtc.text.expand, 0, 200 * 80);
			for(int y = 0; y < 200; y++) {
				for(int x = 0; x < 80; x++) {
					crtc.attrib.expand[y][x] &= 0x70;
					crtc.attrib.expand[y][x] |= 0x02;
				}
			}
//			memset(crtc.attrib.expand, 2, 200 * 80);
		}
	}
	
	// for Xak2 opening
	memset(text, 8, sizeof(text));
	
	int char_height = crtc.char_height;
	
	if(crtc.skip_line) {
		char_height <<= 1;
	}
	for(int cy = 0, ytop = 0; cy < crtc.height && ytop < 200; cy++, ytop += char_height) {
		for(int x = 0, cx = 0; cx < crtc.width; x += 8, cx++) {
			if(width40 && (cx & 1)) {
				continue;
			}
			uint8_t attrib = crtc.attrib.expand[cy][cx];
			uint8_t color = (attrib & 0x70) ? (attrib >> 4) : 8;
			bool under_line = ((attrib & 8) != 0);
			bool upper_line = ((attrib & 4) != 0);
			bool secret = ((attrib & 2) != 0);
			bool reverse = ((attrib & 1) != 0);
			
			uint8_t code = secret ? 0 : crtc.text.expand[cy][cx];
			uint8_t *pattern = ((attrib & 0x80) ? sg_pattern : font) + code * 8;
			
			for(int l = 0, y = ytop; l < char_height && y < 200; l++, y++) {
				uint8_t pat = (l < 8) ? pattern[l] : 0;
				if((upper_line && l == 0) || (under_line && l >= 7)) {
					pat = 0xff;
				}
				if(reverse) {
					pat ^= 0xff;
				}
				
				uint8_t *dest = &text[y][x];
				if(width40) {
					dest[ 0] = dest[ 1] = (pat & 0x80) ? color : 0;
					dest[ 2] = dest[ 3] = (pat & 0x40) ? color : 0;
					dest[ 4] = dest[ 5] = (pat & 0x20) ? color : 0;
					dest[ 6] = dest[ 7] = (pat & 0x10) ? color : 0;
					dest[ 8] = dest[ 9] = (pat & 0x08) ? color : 0;
					dest[10] = dest[11] = (pat & 0x04) ? color : 0;
					dest[12] = dest[13] = (pat & 0x02) ? color : 0;
					dest[14] = dest[15] = (pat & 0x01) ? color : 0;
				} else {
					dest[0] = (pat & 0x80) ? color : 0;
					dest[1] = (pat & 0x40) ? color : 0;
					dest[2] = (pat & 0x20) ? color : 0;
					dest[3] = (pat & 0x10) ? color : 0;
					dest[4] = (pat & 0x08) ? color : 0;
					dest[5] = (pat & 0x04) ? color : 0;
					dest[6] = (pat & 0x02) ? color : 0;
					dest[7] = (pat & 0x01) ? color : 0;
				}
			}
		}
	}
}

void DISPLAY::draw_graph()
{
	uint8_t *vram_b, *vram_r, *vram_g;
	
	switch(mode & 0xe0) {
	case 0x00:
		vram_b = vram_r = vram_g = vram + 0x0000; // null
		break;
	case 0xc0:
		vram_b = vram_r = vram_g = vram + 0x8000; // blue
		break;
	case 0xe0:
		vram_b = vram_r = vram_g = vram + 0xc000; // red
		break;
	case 0xa0:
		vram_b = vram_r = vram_g = vram + 0x4000; // green
		break;
	default:
		vram_b = vram + 0x8000; // blue
		vram_r = vram + 0xc000; // red
		vram_g = vram + 0x4000; // green
		break;
	}
	for(int y = 0, src = 0; y < 200; y ++) {
		for(int x = 0; x < 80; x++) {
			uint8_t b = vram_b[src];
			uint8_t r = vram_r[src];
			uint8_t g = vram_g[src];
			uint8_t* d = &graph[y][x << 3];
			
			d[0] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
			d[1] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
			d[2] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
			d[3] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
			d[4] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
			d[5] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) >> 0);
			d[6] = ((b & 0x02) >> 1) | ((r & 0x02) >> 0) | ((g & 0x02) << 1);
			d[7] = ((b & 0x01) >> 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
			
			src = (src + 1) & 0x3fff;
		}
	}
}

/* ----------------------------------------------------------------------------
	CRTC (uPD3301)
---------------------------------------------------------------------------- */

void crtc_t::reset()
{
	blink.rate = 24;
	cursor.type = cursor.mode = -1;
	cursor.x = cursor.y = -1;
	attrib.data = 0x70;
	attrib.num = 20;
	width = 80;
	height = 25;
	char_height = 8;
	skip_line = false;
	vretrace = 7;
	timing_changed = false;
	reverse = 0;
	intr_mask = 3;
}

void crtc_t::write_cmd(uint8_t data)
{
	cmd = (data >> 5) & 7;
	cmd_ptr = 0;
	switch(cmd) {
	case 0:	// reset
		status &= ~0x16;
		status |= 0x80;	// fix
		cursor.x = cursor.y = -1;
		break;
	case 1:	// start display
		reverse = data & 1;
//		status |= 0x10;
		status |= 0x90;	// fix
		status &= ~8;
		break;
	case 2:	// set interrupt mask
		if(!(data & 1)) {
//			status = 0; // from M88
			status = 0x80; // fix
		}
		intr_mask = data & 3;
		break;
	case 3:	// read light pen
		status &= ~1;
		break;
	case 4:	// load cursor position ON/OFF
		cursor.type = (data & 1) ? cursor.mode : -1;
		break;
	case 5:	// reset interrupt
		status &= ~6;
		break;
	case 6:	// reset counters
		status &= ~6;
		break;
	}
}

void crtc_t::write_param(uint8_t data)
{
	switch(cmd) {
	case 0:
		switch(cmd_ptr) {
		case 0:
			width = min((data & 0x7f) + 2, 80);
			break;
		case 1:
			if(height != (data & 0x3f) + 1) {
				height = (data & 0x3f) + 1;
				timing_changed = true;
			}
			blink.rate = 32 * ((data >> 6) + 1);
			break;
		case 2:
			if(char_height != (data & 0x1f) + 1) {
				char_height = (data & 0x1f) + 1;
				timing_changed = true;
			}
			cursor.mode = (data >> 5) & 3;
			skip_line = ((data & 0x80) != 0);
			break;
		case 3:
			if(vretrace != ((data >> 5) & 7) + 1) {
				vretrace = ((data >> 5) & 7) + 1;
				timing_changed = true;
			}
			break;
		case 4:
			mode = (data >> 5) & 7;
			attrib.num = (mode & 1) ? 0 : min((data & 0x1f) + 1, 20);
			break;
		}
		break;
	case 4:
		switch(cmd_ptr) {
		case 0:
			cursor.x = data;
			break;
		case 1:
			cursor.y = data;
			break;
		}
		break;
	case 6:
		status = 0;
		break;
	}
	cmd_ptr++;
}

uint32_t crtc_t::read_param()
{
	uint32_t val = 0xff;
	
	switch(cmd) {
	case 3:	// read light pen
		switch(cmd_ptr) {
		case 0:
			val = 0; // fix me
			break;
		case 1:
			val = 0; // fix me
			break;
		}
		break;
	default:
		// XM8 version 1.10
		val = read_status();
		break;
	}
	cmd_ptr++;
	return val;
}

uint32_t crtc_t::read_status()
{
	if(status & 8) {
		return status & ~0x10;
	} else {
		return status;
	}
}

void crtc_t::start()
{
	memset(buffer, 0, sizeof(buffer));
	buffer_ptr = 0;
	vblank = false;
}

void crtc_t::finish()
{
	if((status & 0x10) && !(intr_mask & 1)) {
		status |= 2;
	}
	vblank = true;
}

void crtc_t::write_buffer(uint8_t data)
{
	buffer[(buffer_ptr++) & 0x3fff] = data;
}

uint8_t crtc_t::read_buffer(int ofs)
{
	if(ofs < buffer_ptr) {
		return buffer[ofs];
	}
	// dma underrun occurs !!!
	status |= 8;
//	status &= ~0x10;
	return 0;
}

void crtc_t::update_blink()
{
	// from m88
	if(++blink.counter > blink.rate) {
		blink.counter = 0;
	}
	blink.attrib = (blink.counter < blink.rate / 4) ? 2 : 0;
	blink.cursor = (blink.counter <= blink.rate / 4) || (blink.rate / 2 <= blink.counter && blink.counter <= 3 * blink.rate / 4);
}

void crtc_t::expand_buffer()
{
	int char_height_tmp = char_height;
	int exitline = -1;
	
	if(skip_line) {
		char_height_tmp <<= 1;
	}
	if(!(status & 0x10)) {
		exitline = 0;
		goto underrun;
	}
	for(int cy = 0, ytop = 0, ofs = 0; cy < height && ytop < 200; cy++, ytop += char_height_tmp, ofs += 80 + attrib.num * 2) {
		for(int cx = 0; cx < width; cx++) {
			text.expand[cy][cx] = read_buffer(ofs + cx);
		}
		if((status & 8) && exitline == -1) {
			exitline = cy;
//			goto underrun;
		}
	}
	if(mode & 4) {
		// non transparent
		for(int cy = 0, ytop = 0, ofs = 0; cy < height && ytop < 200; cy++, ytop += char_height_tmp, ofs += 80 + attrib.num * 2) {
			for(int cx = 0; cx < width; cx += 2) {
				set_attrib(read_buffer(ofs + cx + 1));
				attrib.expand[cy][cx] = attrib.expand[cy][cx + 1] = attrib.data;
			}
			if((status & 8) && exitline == -1) {
				exitline = cy;
//				goto underrun;
			}
		}
	} else {
		// transparent
		if(mode & 1) {
			memset(attrib.expand, 0x70, sizeof(attrib.expand));
		} else {
			for(int cy = 0, ytop = 0, ofs = 0; cy < height && ytop < 200; cy++, ytop += char_height_tmp, ofs += 80 + attrib.num * 2) {
				uint8_t flags[128];
				memset(flags, 0, sizeof(flags));
				for(int i = 2 * (attrib.num - 1); i >= 0; i -= 2) {
					flags[read_buffer(ofs + i + 80) & 0x7f] = 1;
				}
				attrib.data &= 0xf3; // for PC-8801mkIIFR 付属デモ
				
				for(int cx = 0, pos = 0; cx < width; cx++) {
					if(flags[cx]) {
						set_attrib(read_buffer(ofs + pos + 81));
						pos += 2;
					}
					attrib.expand[cy][cx] = attrib.data;
				}
				if((status & 8) && exitline == -1) {
					exitline = cy;
//					goto underrun;
				}
			}
		}
	}
	if(cursor.x < 80 && cursor.y < 200) {
		if((cursor.type & 1) && blink.cursor) {
			// no cursor
		} else {
			static const uint8_t ctype[5] = {0, 8, 8, 1, 1};
			attrib.expand[cursor.y][cursor.x] ^= ctype[cursor.type + 1];
		}
	}
	// only burst mode
underrun:
	if(exitline != -1) {
		for(int cy = exitline; cy < 200; cy++) {
			memset(&text.expand[cy][0], 0, width);
			memset(&attrib.expand[cy][0], 0x70, width); // color=7
		}
	}
}

void crtc_t::set_attrib(uint8_t code)
{
	if(mode & 2) {
		// color
		if(code & 8) {
			attrib.data = (attrib.data & 0x0f) | (code & 0xf0);
		} else {
			attrib.data = (attrib.data & 0xf0) | ((code >> 2) & 0x0d) | ((code << 1) & 2);
			attrib.data ^= reverse;
			attrib.data ^= ((code & 2) && !(code & 1)) ? blink.attrib : 0;
		}
	} else {
		attrib.data = 0x70 | (code & 0x80) | ((code >> 2) & 0x0d) | ((code << 1) & 2);
		attrib.data ^= reverse;
		attrib.data ^= ((code & 2) && !(code & 1)) ? blink.attrib : 0;
	}
}

/* ----------------------------------------------------------------------------
	DMAC (uPD8257)
---------------------------------------------------------------------------- */

void dmac_t::write_io8(uint32_t addr, uint32_t data)
{
	int c = (addr >> 1) & 3;
	
	switch(addr & 0x0f) {
	case 0x00:
	case 0x02: case 0x0a:
	case 0x04: case 0x0c:
	case 0x06: case 0x0e:
		if(!high_low) {
			if((mode & 0x80) && c == 2) {
				ch[3].addr.b.l = data;
			}
			ch[c].addr.b.l = data;
		} else {
			if((mode & 0x80) && c == 2) {
				ch[3].addr.b.h = data;
				ch[3].addr.b.h2 = ch[3].addr.b.h3 = 0;
			}
			ch[c].addr.b.h = data;
			ch[c].addr.b.h2 = ch[c].addr.b.h3 = 0;
		}
		high_low = !high_low;
		break;
	case 0x01: case 0x09:
	case 0x03: case 0x0b:
	case 0x05: case 0x0d:
	case 0x07: case 0x0f:
		if(!high_low) {
			if((mode & 0x80) && c == 2) {
				ch[3].count.b.l = data;
			}
			ch[c].count.b.l = data;
		} else {
			if((mode & 0x80) && c == 2) {
				ch[3].count.b.h = data & 0x3f;
				ch[3].count.b.h2 = ch[3].count.b.h3 = 0;
				ch[3].mode = data & 0xc0;
			}
			ch[c].count.b.h = data & 0x3f;
			ch[c].count.b.h2 = ch[c].count.b.h3 = 0;
			ch[c].mode = data & 0xc0;
		}
		high_low = !high_low;
		break;
	case 0x08:
		if(!(data & 0x80)) {
			status &= ~0x10;
		}
		mode = data;
		high_low = false;
		break;
	}
}

uint32_t dmac_t::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	int c = (addr >> 1) & 3;
	
	switch(addr & 0x0f) {
	case 0x00:
	case 0x02: case 0x0a:
	case 0x04: case 0x0c:
	case 0x06: case 0x0e:
		if(!high_low) {
			val = ch[c].addr.b.l;
		} else {
			val = ch[c].addr.b.h;
		}
		high_low = !high_low;
		break;
	case 0x01: case 0x09:
	case 0x03: case 0x0b:
	case 0x05: case 0x0d:
	case 0x07: case 0x0f:
		if(!high_low) {
			val = ch[c].count.b.l;
		} else {
			val = (ch[c].count.b.h & 0x3f) | ch[c].mode;
		}
		high_low = !high_low;
		break;
	case 0x08:
		val = status;
		status &= 0xf0;
//		high_low = false;
		break;
	}
	return val;
}

void dmac_t::start(int c)
{
	if(mode & (1 << c)) {
		status &= ~(1 << c);
		ch[c].running = true;
	} else {
		ch[c].running = false;
	}
}

void dmac_t::run(int c)
{
	if(ch[c].running) {
		if(ch[c].count.sd >= 0) {
			if(ch[c].mode == 0x80) {
				ch[c].io->write_dma_io8(0, mem->read_dma_data8(ch[c].addr.w.l));
			} else if(ch[c].mode == 0x40) {
				mem->write_dma_data8(ch[c].addr.w.l, ch[c].io->read_dma_io8(0));
			} else if(ch[c].mode == 0x00) {
				ch[c].io->read_dma_io8(0); // verify
			}
			ch[c].addr.sd++;
			ch[c].count.sd--;
			status &= ~0x10;
		}
		if(ch[c].count.sd < 0) {
			finish(c);
		}
	}
}

void dmac_t::finish(int c)
{
	if(ch[c].running) {
		while(ch[c].count.sd >= 0) {
			if(ch[c].mode == 0x80) {
				ch[c].io->write_dma_io8(0, mem->read_dma_data8(ch[c].addr.w.l));
			} else if(ch[c].mode == 0x40) {
				mem->write_dma_data8(ch[c].addr.w.l, ch[c].io->read_dma_io8(0));
			} else if(ch[c].mode == 0x00) {
				ch[c].io->read_dma_io8(0); // verify
			}
			ch[c].addr.sd++;
			ch[c].count.sd--;
			status &= ~0x10;
		}
		if((mode & 0x80) && c == 2) {
			ch[2].addr.sd = ch[3].addr.sd;
			ch[2].count.sd = ch[3].count.sd;
			ch[2].mode = ch[3].mode;
			status |= 0x10;
		} else if(mode & 0x40) {
			mode &= ~(1 << c);
		}
		status |= (1 << c);
		ch[c].running = false;
	}
}

#define STATE_VERSION	2

bool DISPLAY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(font, sizeof(font), 1);
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateValue(busreq_clocks);
	state_fio->StateValue(color);
	state_fio->StateValue(width40);
	state_fio->StateValue(mode);
	state_fio->StateValue(crtc.blink.rate);
	state_fio->StateValue(crtc.blink.counter);
	state_fio->StateValue(crtc.blink.cursor);
	state_fio->StateValue(crtc.blink.attrib);
	state_fio->StateValue(crtc.cursor.type);
	state_fio->StateValue(crtc.cursor.mode);
	state_fio->StateValue(crtc.cursor.x);
	state_fio->StateValue(crtc.cursor.y);
	state_fio->StateValue(crtc.attrib.data);
	state_fio->StateValue(crtc.attrib.num);
	state_fio->StateArray(&crtc.attrib.expand[0][0], sizeof(crtc.attrib.expand), 1);
	state_fio->StateArray(&crtc.text.expand[0][0], sizeof(crtc.text.expand), 1);
	state_fio->StateValue(crtc.width);
	state_fio->StateValue(crtc.height);
	state_fio->StateValue(crtc.char_height);
	state_fio->StateValue(crtc.skip_line);
	state_fio->StateValue(crtc.vretrace);
	state_fio->StateValue(crtc.timing_changed);
	state_fio->StateArray(crtc.buffer, sizeof(crtc.buffer), 1);
	state_fio->StateValue(crtc.buffer_ptr);
	state_fio->StateValue(crtc.cmd);
	state_fio->StateValue(crtc.cmd_ptr);
	state_fio->StateValue(crtc.mode);
	state_fio->StateValue(crtc.reverse);
	state_fio->StateValue(crtc.intr_mask);
	state_fio->StateValue(crtc.status);
	state_fio->StateValue(crtc.vblank);
	for(int i = 0; i < array_length(dmac.ch); i++) {
		state_fio->StateValue(dmac.ch[i].addr);
		state_fio->StateValue(dmac.ch[i].count);
		state_fio->StateValue(dmac.ch[i].mode);
		state_fio->StateValue(dmac.ch[i].nbytes);
		state_fio->StateValue(dmac.ch[i].running);
	}
	state_fio->StateValue(dmac.mode);
	state_fio->StateValue(dmac.status);
	state_fio->StateValue(dmac.high_low);
	return true;
}

