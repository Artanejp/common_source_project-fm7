/*
	FUJITSU FM16beta Emulator 'eFM16beta'

	Author : Takeda.Toshiya
	Date   : 2017.12.28-

	[ sub system ]
*/

#include "sub.h"
#include "main.h"
#include "../pcm1bit.h"

void SUB::initialize()
{
	MEMORY::initialize();

	// init memory
	memset(wram, 0, sizeof(wram));
	memset(sram, 0, sizeof(sram));
	memset(gvram, 0, sizeof(gvram));
	memset(dummy, 0, sizeof(dummy));
	memset(cvram, 0, sizeof(cvram));
	memset(kvram, 0, sizeof(kvram));
	memset(rom, 0xff, sizeof(rom));
	memset(ank8, 0xff, sizeof(ank8));
	memset(ank16, 0xff, sizeof(ank16));
	memset(kanji16, 0xff, sizeof(kanji16));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("SUBSYS.ROM")), FILEIO_READ_BINARY) ||
	   fio->Fopen(create_local_path(_T("SUB.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
		if(rom[0x4ff2] == 0xff && rom[0x4ff3] == 0xff) { // SWI3
			rom[0x4ff2] = 0x9f;
			rom[0x4ff3] = 0x6e;
		}
		if(rom[0x4ff4] == 0xff && rom[0x4ff5] == 0xff) { // SWI2
			rom[0x4ff4] = 0x9f;
			rom[0x4ff5] = 0x71;
		}
		if(rom[0x4ff6] == 0xff && rom[0x4ff7] == 0xff) { // FIRQ
			rom[0x4ff6] = 0x9f;
			rom[0x4ff7] = 0x74;
		}
		if(rom[0x4ff8] == 0xff && rom[0x4ff9] == 0xff) { // IRQ
			rom[0x4ff8] = 0x9f;
			rom[0x4ff9] = 0x77;
		}
		if(rom[0x4ffa] == 0xff && rom[0x4ffb] == 0xff) { // SWI
			rom[0x4ffa] = 0x9f;
			rom[0x4ffb] = 0x7a;
		}
		if(rom[0x4ffc] == 0xff && rom[0x4ffd] == 0xff) { // NMI
			rom[0x4ffc] = 0x9f;
			rom[0x4ffd] = 0x7d;
		}
		if(rom[0x4ffe] == 0xff && rom[0x4fff] == 0xff) { // RESET
			for(int i = 0; i < sizeof(rom) - 4; i++) {
				static const uint8_t boot[4] = {0x86, 0x90, 0x1f, 0x8b};
				if(memcmp(rom + i, boot, sizeof(boot)) == 0) {
					i += 0xb000;
					rom[0x4ffe] = (i >> 8) & 0xff;
					rom[0x4fff] = (i >> 0) & 0xff;
					break;
				}
			}
		}
	}
	if(fio->Fopen(create_local_path(_T("ANK8.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ank8, sizeof(ank8), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("ANK16.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ank16, sizeof(ank16), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("KANJI16.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji16, sizeof(kanji16), 1);
		fio->Fclose();
	}
	delete fio;
	
	set_memory_rw(0x9000, 0x9f7f, wram + 0x1000);
	set_memory_rw(0x9f80, 0x9fff, sram);
	set_memory_r (0xb000, 0xffff, rom);
	
	for(int i = 0; i < 8; i++) {
		palette_txt[i] = palette_cg[i] = RGB_COLOR(i & 2 ? 255 : 0, i & 4 ? 255 : 0, i & 1 ? 255 : 0);
	}


}

void SUB::release()
{
	MEMORY::release();
}

void SUB::reset()
{
	MEMORY::reset();
	
	// reset crtc
	blink = 0;
	outctrl = 0xf;
	
	d_main->write_signal(SIG_MAIN_SUB_BUSY, 1, 1);
	mix = 0x00;
	
	update = dispctrl = pagesel = 0;
	ankcg = 0;
	accaddr = dispaddr = 0;
	
	update_cvram_bank();
	update_kvram_bank();
	
	memset(attention, 0, sizeof(attention));
	mainack = 0x80;
	
	irq_cancel = irq_vsync = firq_key = firq_pen = false;
	
	kj_l = kj_h = kj_ofs = kj_row = 0;
	
	cmdreg = maskreg = compbit = bankdis = 0;
	memset(compreg, 0xff, sizeof(compreg));
}

void SUB::write_data8(uint32_t addr, uint32_t data)
{
	// sub cpu memory bus
	write_memory(addr, data);
#ifdef _IO_DEBUG_LOG
	if((addr >= 0x9f80 && addr < 0xa000) || (addr >= 0xff80 && addr < 0xffe0)) {
		this->out_debug_log(_T("SUB %06x\tOUT8\t%04x,%02x\n"), get_cpu_pc(1), addr, data);
	}
#endif
}

uint32_t SUB::read_data8(uint32_t addr)
{
	// sub cpu memory bus
	uint32_t val = read_memory(addr);
#ifdef _IO_DEBUG_LOG
	if((addr >= 0x9f80 && addr < 0xa000) || (addr >= 0xff80 && addr < 0xffe0)) {
		this->out_debug_log(_T("SUB %06x\tIN8\t%04x,%02x\n"), get_cpu_pc(1), addr, val);
	}
#endif
	return val;
}

void SUB::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	// main cpu direct access
	addr &= 0xffff;
	write_memory(addr, data);
#ifdef _IO_DEBUG_LOG
	if((addr >= 0x9f80 && addr < 0xa000) || (addr >= 0xff80 && addr < 0xffe0)) {
		this->out_debug_log(_T("MAIN %06x\tOUT8\t%04x,%02x\n"), get_cpu_pc(0), addr, data);
	}
#endif
}

uint32_t SUB::read_memory_mapped_io8(uint32_t addr)
{
	// main cpu direct access
	addr &= 0xffff;
	uint32_t val = read_memory(addr);
#ifdef _IO_DEBUG_LOG
	if((addr >= 0x9f80 && addr < 0xa000) || (addr >= 0xff80 && addr < 0xffe0)) {
		this->out_debug_log(_T("MAIN %06x\tIN8\t%04x,%02x\n"), get_cpu_pc(1), addr, val);
	}
#endif
	return val;
}

void SUB::write_memory(uint32_t addr, uint32_t data)
{
	if(addr < 0x8000) {
		if(dispctrl & 0x40) {
			uint32_t bank = (pagesel >> 4) & 1;
			addr &= 0x7fff;
			addr |= bank << 15;
		} else {
			uint32_t bank = (pagesel >> 3) & 3;
			addr &= 0x3fff;
			addr |= bank << 14;
		}
		if(update & 1) gvram[addr | 0x00000] = data;
		if(update & 2) gvram[addr | 0x10000] = data;
		if(update & 4) gvram[addr | 0x20000] = data;
	} else if(addr >= 0xff80 && addr < 0xffe0) {
		uint8_t change;
		
		switch(addr) {
		case 0xff80:
			d_main->write_signal(SIG_MAIN_SUB_BUSY, ~data, 1);
			mix = data;
			break;
		case 0xff81:
			update = data;
			break;
		case 0xff82:
			dispctrl = data;
			break;
		case 0xff83:
			change = pagesel ^ data;
			pagesel = data;
			if(change & 0x20) {
				update_cvram_bank();
			}
			break;




		case 0xff87:
			d_main->write_signal(SIG_MAIN_FIRQ0, 1, 1);
			mainack &= ~0x80;
			break;

		case 0xff88:
			accaddr = (accaddr & 0x00ff) | (data << 8);
			break;
		case 0xff89:
			accaddr = (accaddr & 0xff00) | (data << 0);
			break;
		case 0xff8a:
			dispaddr = (dispaddr & 0x00ff) | (data << 8);
			break;
		case 0xff8b:
			dispaddr = (dispaddr & 0xff00) | (data << 0);
			break;

		case 0xff8e:
		case 0xff8f:
			d_crtc->write_io8(addr, data);
			break;

		case 0xff90:
		case 0xff91:
		case 0xff92:
			attention[addr & 3] = data;
			break;
		case 0xff94:
			kj_h = data & 0x7f;
			break;
		case 0xff95:
			kj_l = data & 0x7f;
			kj_row = 0;
			if(kj_h < 0x30) {
				kj_ofs = (((kj_l - 0x00) & 0x1f) <<  5) | (((kj_l - 0x20) & 0x20) <<  9) | (((kj_l - 0x20) & 0x40) <<  7) | (((kj_h - 0x00) & 0x07) << 10);
			} else if(kj_h < 0x70) {
				kj_ofs = (((kj_l - 0x00) & 0x1f) <<  5) + (((kj_l - 0x20) & 0x60) <<  9) + (((kj_h - 0x00) & 0x0f) << 10) + (((kj_h - 0x30) & 0x70) * 0xc00) + 0x08000;
			} else {
				kj_ofs = (((kj_l - 0x00) & 0x1f) <<  5) | (((kj_l - 0x20) & 0x20) <<  9) | (((kj_l - 0x20) & 0x40) <<  7) | (((kj_h - 0x00) & 0x07) << 10) | 0x38000;
			}
			break;
		case 0xff96:
			kanji16[(kj_ofs | ((kj_row & 0xf) << 1)) & 0x3ffff] = data;
			break;
		case 0xff97:
			kanji16[(kj_ofs | ((kj_row++ & 0xf) << 1) | 1) & 0x3ffff] = data;
			break;

		case 0xff98:
			d_pcm->write_signal(SIG_PCM1BIT_ON, 0, 0);
			break;

		case 0xff99:
			change = ankcg ^ data;
			ankcg = data;
			if(change & 0x01) {
				update_kvram_bank();
			}
			if(change & 0x80) {
				update_kvram_bank();
			}
			break;

		case 0xffa0:
			cmdreg = data;
			break;
		case 0xffa1:
			imgcol = data;
			break;
		case 0xffa2:
			maskreg = data;
			break;
		case 0xffa3:
		case 0xffa4:
		case 0xffa5:
		case 0xffa6:
		case 0xffa7:
		case 0xffa8:
		case 0xffa9:
		case 0xffaa:
			compreg[addr & 7] = data;
			break;
		case 0xffab:
			bankdis = data;
			break;
		case 0xffac:
		case 0xffad:
		case 0xffae:
		case 0xffaf:
			tilereg[addr & 3] = data;
			break;
		case 0xffb0:
			lofs = (lofs & 0xff) | (data << 8);
			break;
		case 0xffb1:
			lofs = (lofs & 0xff00) | data;
			break;
		case 0xffb2:
			lsty = (lsty & 0xff) | (data << 8);
			break;
		case 0xffb3:
			lsty = (lsty & 0xff00) | data;
			break;
		case 0xffb4:
			lsx = (lsx & 0xff) | (data << 8);
			break;
		case 0xffb5:
			lsx = (lsx & 0xff00) | data;
			break;
		case 0xffb6:
			lsy = (lsy & 0xff) | (data << 8);
			break;
		case 0xffb7:
			lsy = (lsy & 0xff00) | data;
			break;
		case 0xffb8:
			lex = (lex & 0xff) | (data << 8);
			break;
		case 0xffb9:
			lex = (lex & 0xff00) | data;
			break;
		case 0xffba:
			ley = (ley & 0xff) | (data << 8);
			break;
		case 0xffbb:
			ley = (ley & 0xff00) | data;
			// start drawing line
			line();
			break;
#ifdef _IO_DEBUG_LOG
		default:
			this->out_debug_log(_T("UNKNOWN:\t"));
			break;
#endif
		}
	} else {
		MEMORY::write_data8(addr, data);
	}
}

uint32_t SUB::read_memory(uint32_t addr)
{
	if(addr < 0x8000) {
		if(dispctrl & 0x40) {
			uint32_t bank = (pagesel >> 4) & 1;
			addr &= 0x7fff;
			addr |= bank << 15;
		} else {
			uint32_t bank = (pagesel >> 3) & 3;
			addr &= 0x3fff;
			addr |= bank << 14;
		}
		switch(update & 0xc0) {
		case 0x00: return gvram[addr | 0x00000];
		case 0x40: return gvram[addr | 0x10000];
		case 0x80: return gvram[addr | 0x20000];
		}
		return 0xff;
	} else if(addr >= 0xff80 && addr < 0xffe0) {
		switch(addr) {
		case 0xff80:
			// bit5: Cursor LSB
			// bit4: Light Pen FIRQ 0:Disabled 1:Enabled
			// bit3: WIDTH 0:40 1:80
			// bit2: FLASH 0:OFF 1:ON
			// bit1: INSLED 0:OFF 1:ON
			// bit0: SUB BUSY 0:BUSY 1:READY
			return mix | 0xc0;
		case 0xff81:
			// bit7: Read Out Control RC2
			// bit6: Read Out Control RC1
			// bit2: RAM Select Bit RAM3
			// bit1: RAM Select Bit RAM2
			// bit0: RAM Select Bit RAM1
			return update | 0x38;
		case 0xff83:
			return pagesel | 0xc7;

		case 0xff84:
			{
				uint8_t val = (firq_key ? 0x01 : 0) | (firq_pen ? 0x80 : 0);
				firq_key = firq_pen = false;
				update_firq();
				return val;
			}

		case 0xff85:
			{
				uint8_t val = (irq_cancel ? 0x01 : 0) | (irq_vsync ? 0x02 : 0) | 0x7e;
				irq_cancel = irq_vsync = false;
				update_irq();
				return val;
			}



		case 0xff86:
			return (disp ? 0x80 : 0) | (vsync ? 0x04 : 0) | 0x70;

		case 0xff87:
			d_main->write_signal(SIG_MAIN_FIRQ0, 1, 1);
			mainack &= ~0x80;
			break;

		case 0xff8c:
		case 0xff8d:
			return d_keyboard->read_io8(addr);
		case 0xff8e:
		case 0xff8f:
			return d_crtc->read_io8(addr);
		case 0xff90:
		case 0xff91:
		case 0xff92:
			return attention[addr & 3];
		case 0xff93:
			return mainack;
		case 0xff94:
			return 0x80; // ëÊìÒêÖèÄÇ†ÇË
		case 0xff96:
			return kanji16[(kj_ofs | ((kj_row & 0xf) << 1)) & 0x3ffff];
		case 0xff97:
			return kanji16[(kj_ofs | ((kj_row++ & 0xf) << 1) | 1) & 0x3ffff];
		case 0xff98:
			d_pcm->write_signal(SIG_PCM1BIT_ON, 1, 1);
			break;

		case 0xff99:
			return ankcg;
		case 0xffa0:
			return cmdreg;
		case 0xffa1:
			return imgcol | 0xf0;
		case 0xffa2:
			return maskreg;
		case 0xffa3:
			return compbit;
		case 0xffab:
			return bankdis & 0x0f;

		case 0xffb0:
			return (lofs >> 8) & 0xff;
		case 0xffb1:
			return (lofs >> 0) & 0xff;
#ifdef _IO_DEBUG_LOG
		default:
			this->out_debug_log(_T("UNKNOWN:\t"));
			break;
#endif
		}
		return 0xff;
	} else {
		return MEMORY::read_data8(addr);
	}
}

void SUB::write_io8(uint32_t addr, uint32_t data)
{
	// main cpu i/o bus
	switch(addr) {
	case 0xfd98:
	case 0xfd99:
	case 0xfd9a:
	case 0xfd9b:
	case 0xfd9c:
	case 0xfd9d:
	case 0xfd9e:
	case 0xfd9f:
		// digital palette
		if(data & 8) {
			palette_cg[addr & 7] = RGB_COLOR(data & 2 ? 255 : 0, data & 4 ? 255 : 0, data & 1 ? 255 : 0);
		} else {
			palette_cg[addr & 7] = RGB_COLOR(data & 2 ? 127 : 0, data & 4 ? 127 : 0, data & 1 ? 127 : 0);
		}
		dpal[addr & 7] = data;
		break;
	case 0xfda0:
		outctrl = data;
		break;
	// 0xfc80 - 0xfcff
	default:
		sram[addr & 0x7f] = data;
		break;
	}
}

uint32_t SUB::read_io8(uint32_t addr)
{
	// main cpu i/o bus
	switch(addr) {
	case 0xfd20:
	case 0xfd21:
	case 0xfd22:
		return attention[addr & 3];

	case 0xfd98:
	case 0xfd99:
	case 0xfd9a:
	case 0xfd9b:
	case 0xfd9c:
	case 0xfd9d:
	case 0xfd9e:
	case 0xfd9f:
		// digital palette
		return dpal[addr & 7] | 0xf0;
	// 0xfc80 - 0xfcff
	default:
		return sram[addr & 0x7f];
	}
	return 0xff;
}

void SUB::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_SUB_DISP) {
		disp = ((data & mask) != 0);
	} else if(id == SIG_SUB_VSYNC) {
		irq_vsync = vsync = ((data & mask) != 0);
		update_irq();
	} else if(id == SIG_SUB_CANCEL) {
		irq_cancel = ((data & mask) != 0);
		update_irq();
this->out_debug_log(_T("MAIN -> SUB: CANCEL = %d\n"), irq_cancel);

	} else if(id == SIG_SUB_KEY) {
		firq_key = ((data & mask) != 0);
		update_firq();
	} else if(id == SIG_SUB_HALT) {
this->out_debug_log(_T("MAIN -> SUB: HALT = %d\n"), data & mask ?1 : 0);
		if(data & mask) {
			d_main->write_signal(SIG_MAIN_SUB_BUSY, 1, 1);
			mix &= ~0x01;
		}
		d_subcpu->write_signal(SIG_CPU_BUSREQ, data, mask);
	} else if(id == SIG_SUB_MAINACK) {
this->out_debug_log(_T("MAIN -> SUB: MAINACK = %d\n"), data & mask ? 1 : 0);
		if(data & mask) {
			d_main->write_signal(SIG_MAIN_FIRQ0, 0, 0);
			mainack |= 0x80;
		}
	}
}

void SUB::update_irq()
{
	d_subcpu->write_signal(SIG_CPU_IRQ, irq_cancel || (irq_vsync && (ankcg & 0x80)) ? 1 : 0, 1);
}

void SUB::update_firq()
{
	d_subcpu->write_signal(SIG_CPU_FIRQ, firq_key || (firq_pen &&  (mix & 0x10)) ? 1 : 0, 1);
}

void SUB::update_cvram_bank()
{
	if(pagesel & 0x20) {
		set_memory_rw(0x8000, 0x8fff, wram);
	} else {
		set_memory_rw(0x8000, 0x8fff, cvram);
	}
}

void SUB::update_kvram_bank()
{
	if(ankcg & 0x01) {
		set_memory_r(0xa000, 0xa7ff, ank8);
		set_memory_r(0xa800, 0xafff, ank8);
	} else {
		set_memory_rw(0xa000, 0xafff, kvram);
	}
}

void SUB::key_down(int code)
{
	
}

void SUB::key_up(int code)
{
	
}

void SUB::point(int x, int y, int col)
{
	if(x < 640 && y < 400) {
		int ofs = ((lofs & 0x3fff) + (x >> 3) + y * 80) & 0x7fff;
		uint8_t bit = 0x80 >> (x & 7);
		for(int pl = 0; pl < 3; pl++) {
			uint8_t pbit = 1 << pl;
			if(!(bankdis & pbit)) {
				if(col & pbit) {
					gvram[0x8000 * pl + ofs] |= bit;
				} else {
					gvram[0x8000 * pl + ofs] &= ~bit;
				}
			}
		}
	}
}

void SUB::line()
{
	int nx = lsx, ny = lsy;
	int dx = abs(lex - lsx) * 2;
	int dy = abs(ley - lsy) * 2;
	int sx = (lex < lsx) ? -1 : 1;
	int sy = (ley < lsy) ? -1 : 1;
	int c = 0;
	
	point(lsx, lsy, (lsty & (0x8000 >> (c++ & 15))) ? imgcol : 0);
	if(dx > dy) {
		int frac = dy - dx / 2;
		while(nx != lex) {
			if(frac >= 0) {
				ny += sy;
				frac -= dx;
			}
			nx += sx;
			frac += dy;
			point(nx, ny, (lsty & (0x8000 >> (c++ & 15))) ? imgcol : 0);
		}
	} else {
		int frac = dx - dy / 2;
		while(ny != ley) {
			if(frac >= 0) {
				nx += sx;
				frac -= dy;
			}
			ny += sy;
			frac += dx;
			point(nx, ny, (lsty & (0x8000 >> (c++ & 15))) ? imgcol : 0);
		}
	}
//	point(lex, ley, (lsty & (0x8000 >> (c++ & 15))) ? imgcol : 0);
}

void SUB::draw_screen()
{
	// render screen
	memset(screen_txt, 0, sizeof(screen_txt));
	memset(screen_cg, 0, sizeof(screen_cg));
	
	if(outctrl & 1) {
		if(mix & 8) {
			draw_text80();
		} else {
			draw_text40();
		}
	} else if(outctrl & 2) {
		// greem text
	}
	if(outctrl & 4) {
		draw_cg();
	} else if(outctrl & 8) {
		// green graphics
	}
	
	for(int y = 0; y < SCREEN_HEIGHT; y++) {
		scrntype_t* dest = emu->get_screen_buffer(y);
		uint8_t* txt = screen_txt[y];
		uint8_t* cg = screen_cg[y];
		
		for(int x = 0; x < SCREEN_WIDTH; x++) {
			dest[x] = txt[x] ? palette_txt[txt[x] & 15] : palette_cg[cg[x]];
		}
	}
	emu->screen_skip_line(false);
}

void SUB::draw_text40()
{
	int src = ((chreg[12] << 9) | (chreg[13] << 1)) & 0xfff;
	int caddr = ((chreg[8] & 0xc0) == 0xc0) ? -1 : (((chreg[14] << 9) | (chreg[15] << 1) | (mix & 0x20 ? 1 : 0)) & 0x7ff);
	int ymax = (chreg[6] > 0) ? chreg[6] : 25;
	int yofs = 400 / ymax;
	
	for(int y = 0; y < ymax; y++) {
		for(int x = 0; x < 40; x++) {
			bool cursor = ((src >> 1) == caddr);
			int cx = x;
			uint8_t code = cvram[src];
			uint8_t h = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8_t attr = cvram[src];
			uint8_t l = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8_t col = ((attr & 0x20) >> 2) | (attr & 7) | 16;
			bool blnk = (blink & 32) && (attr & 0x10);
			bool rev = ((attr & 8) != 0);
			uint8_t xor_mask = (rev != blnk) ? 0xff : 0;
			
			if(attr & 0x40) {
				// kanji
				int ofs;
				if(h < 0x30) {
					ofs = (((l - 0x00) & 0x1f) <<  5) | (((l - 0x20) & 0x20) <<  9) | (((l - 0x20) & 0x40) <<  7) | (((h - 0x00) & 0x07) << 10);
				} else if(h < 0x70) {
					ofs = (((l - 0x00) & 0x1f) <<  5) + (((l - 0x20) & 0x60) <<  9) + (((h - 0x00) & 0x0f) << 10) + (((h - 0x30) & 0x70) * 0xc00) + 0x08000;
				} else {
					ofs = (((l - 0x00) & 0x1f) <<  5) | (((l - 0x20) & 0x20) <<  9) | (((l - 0x20) & 0x40) <<  7) | (((h - 0x00) & 0x07) << 10) | 0x38000;
				}
				
				for(int l = 0; l < 16 && l < yofs; l++) {
					uint8_t pat0 = kanji16[ofs + (l << 1) + 0] ^ xor_mask;
					uint8_t pat1 = kanji16[ofs + (l << 1) + 1] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 400) {
						break;
					}
					uint8_t* d = &screen_txt[yy][x << 4];
					
					d[ 0] = d[ 1] = (pat0 & 0x80) ? col : 0;
					d[ 2] = d[ 3] = (pat0 & 0x40) ? col : 0;
					d[ 4] = d[ 5] = (pat0 & 0x20) ? col : 0;
					d[ 6] = d[ 7] = (pat0 & 0x10) ? col : 0;
					d[ 8] = d[ 9] = (pat0 & 0x08) ? col : 0;
					d[10] = d[11] = (pat0 & 0x04) ? col : 0;
					d[12] = d[13] = (pat0 & 0x02) ? col : 0;
					d[14] = d[15] = (pat0 & 0x01) ? col : 0;
					d[16] = d[17] = (pat1 & 0x80) ? col : 0;
					d[18] = d[19] = (pat1 & 0x40) ? col : 0;
					d[20] = d[21] = (pat1 & 0x20) ? col : 0;
					d[22] = d[23] = (pat1 & 0x10) ? col : 0;
					d[24] = d[25] = (pat1 & 0x08) ? col : 0;
					d[26] = d[27] = (pat1 & 0x04) ? col : 0;
					d[28] = d[29] = (pat1 & 0x02) ? col : 0;
					d[30] = d[31] = (pat1 & 0x01) ? col : 0;
				}
				src = (src + 2) & 0xfff;
				x++;
			} else {
				for(int l = 0; l < 16 && l < yofs; l++) {
					uint8_t pat = ank16[(code << 4) + l] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 400) {
						break;
					}
					uint8_t* d = &screen_txt[yy][x << 4];
					
					d[ 0] = d[ 1] = (pat & 0x80) ? col : 0;
					d[ 2] = d[ 3] = (pat & 0x40) ? col : 0;
					d[ 4] = d[ 5] = (pat & 0x20) ? col : 0;
					d[ 6] = d[ 7] = (pat & 0x10) ? col : 0;
					d[ 8] = d[ 9] = (pat & 0x08) ? col : 0;
					d[10] = d[11] = (pat & 0x04) ? col : 0;
					d[12] = d[13] = (pat & 0x02) ? col : 0;
					d[14] = d[15] = (pat & 0x01) ? col : 0;
				}
			}
			if(cursor) {
				int bp = chreg[10] & 0x60;
				if(bp == 0 || (bp == 0x40 && (blink & 8)) || (bp == 0x60 && (blink & 0x10))) {
					int st = chreg[10] & 15;
					int ed = chreg[11] & 15;
					for(int i = st; i < ed && i < yofs; i++) {
						memset(&screen_txt[y * yofs + i][cx << 3], 7, 8);
					}
				}
			}
		}
	}
}

void SUB::draw_text80()
{
	int src = ((chreg[12] << 9) | (chreg[13] << 1)) & 0xfff;
	int caddr = ((chreg[8] & 0xc0) == 0xc0) ? -1 : (((chreg[14] << 9) | (chreg[15] << 1) | (mix & 0x20 ? 1 : 0)) & 0x7ff);
	int ymax = (chreg[6] > 0) ? chreg[6] : 25;
	int yofs = 400 / ymax;
	
	for(int y = 0; y < 25; y++) {
		for(int x = 0; x < 80; x++) {
			bool cursor = ((src >> 1) == caddr);
			int cx = x;
			uint8_t code = cvram[src];
			uint8_t h = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8_t attr = cvram[src];
			uint8_t l = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8_t col = ((attr & 0x20) >> 2) | (attr & 7) | 16;
			bool blnk = (blink & 32) && (attr & 0x10);
			bool rev = ((attr & 8) != 0);
			uint8_t xor_mask = (rev != blnk) ? 0xff : 0;
			
			if(attr & 0x40) {
				// kanji
				int ofs;
				if(h < 0x30) {
					ofs = (((l - 0x00) & 0x1f) <<  5) | (((l - 0x20) & 0x20) <<  9) | (((l - 0x20) & 0x40) <<  7) | (((h - 0x00) & 0x07) << 10);
				} else if(h < 0x70) {
					ofs = (((l - 0x00) & 0x1f) <<  5) + (((l - 0x20) & 0x60) <<  9) + (((h - 0x00) & 0x0f) << 10) + (((h - 0x30) & 0x70) * 0xc00) + 0x08000;
				} else {
					ofs = (((l - 0x00) & 0x1f) <<  5) | (((l - 0x20) & 0x20) <<  9) | (((l - 0x20) & 0x40) <<  7) | (((h - 0x00) & 0x07) << 10) | 0x38000;
				}
				
				for(int l = 0; l < 16 && l < yofs; l++) {
					uint8_t pat0 = kanji16[ofs + (l << 1) + 0] ^ xor_mask;
					uint8_t pat1 = kanji16[ofs + (l << 1) + 1] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 400) {
						break;
					}
					uint8_t* d = &screen_txt[yy][x << 3];
					
					d[ 0] = (pat0 & 0x80) ? col : 0;
					d[ 1] = (pat0 & 0x40) ? col : 0;
					d[ 2] = (pat0 & 0x20) ? col : 0;
					d[ 3] = (pat0 & 0x10) ? col : 0;
					d[ 4] = (pat0 & 0x08) ? col : 0;
					d[ 5] = (pat0 & 0x04) ? col : 0;
					d[ 6] = (pat0 & 0x02) ? col : 0;
					d[ 7] = (pat0 & 0x01) ? col : 0;
					d[ 8] = (pat1 & 0x80) ? col : 0;
					d[ 9] = (pat1 & 0x40) ? col : 0;
					d[10] = (pat1 & 0x20) ? col : 0;
					d[11] = (pat1 & 0x10) ? col : 0;
					d[12] = (pat1 & 0x08) ? col : 0;
					d[13] = (pat1 & 0x04) ? col : 0;
					d[14] = (pat1 & 0x02) ? col : 0;
					d[15] = (pat1 & 0x01) ? col : 0;
				}
				src = (src + 2) & 0xfff;
				x++;
			} else {
				for(int l = 0; l < 16 && l < yofs; l++) {
					uint8_t pat = ank16[(code << 4) + l] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 400) {
						break;
					}
					uint8_t* d = &screen_txt[yy][x << 3];
					
					d[0] = (pat & 0x80) ? col : 0;
					d[1] = (pat & 0x40) ? col : 0;
					d[2] = (pat & 0x20) ? col : 0;
					d[3] = (pat & 0x10) ? col : 0;
					d[4] = (pat & 0x08) ? col : 0;
					d[5] = (pat & 0x04) ? col : 0;
					d[6] = (pat & 0x02) ? col : 0;
					d[7] = (pat & 0x01) ? col : 0;
				}
			}
			if(cursor) {
				int bp = chreg[10] & 0x60;
				if(bp == 0 || (bp == 0x40 && (blink & 8)) || (bp == 0x60 && (blink & 0x10))) {
					int st = chreg[10] & 15;
					int ed = chreg[11] & 15;
					for(int i = st; i < ed && i < yofs; i++) {
						memset(&screen_txt[y * yofs + i][cx << 3], 7, 8);
					}
				}
			}
		}
	}
}

void SUB::draw_cg()
{
	if(dispctrl & 0x40) {
		// 400line
		int pofs = ((dispctrl >> 3) & 1) * 0x20000;
		uint8_t* p0 = (dispctrl & 0x01) ? &gvram[pofs | 0x00000] : dummy;
		uint8_t* p1 = (dispctrl & 0x02) ? &gvram[pofs | 0x08000] : dummy;
		uint8_t* p2 = (dispctrl & 0x04) ? &gvram[pofs | 0x10000] : dummy;
		int ptr = dispaddr & 0x7ffe;
		
		for(int y = 0; y < 400; y++) {
			for(int x = 0; x < 640; x += 8) {
				uint8_t r = p0[ptr];
				uint8_t g = p1[ptr];
				uint8_t b = p2[ptr++];
				ptr &= 0x7fff;
				uint8_t* d = &screen_cg[y][x];
				
				d[0] = ((r & 0x80) >> 7) | ((g & 0x80) >> 6) | ((b & 0x80) >> 5);
				d[1] = ((r & 0x40) >> 6) | ((g & 0x40) >> 5) | ((b & 0x40) >> 4);
				d[2] = ((r & 0x20) >> 5) | ((g & 0x20) >> 4) | ((b & 0x20) >> 3);
				d[3] = ((r & 0x10) >> 4) | ((g & 0x10) >> 3) | ((b & 0x10) >> 2);
				d[4] = ((r & 0x08) >> 3) | ((g & 0x08) >> 2) | ((b & 0x08) >> 1);
				d[5] = ((r & 0x04) >> 2) | ((g & 0x04) >> 1) | ((b & 0x04) >> 0);
				d[6] = ((r & 0x02) >> 1) | ((g & 0x02) >> 0) | ((b & 0x02) << 1);
				d[7] = ((r & 0x01) >> 0) | ((g & 0x01) << 1) | ((b & 0x01) << 2);
			}
		}
	} else {
		// 200line
		int pofs = ((dispctrl >> 3) & 3) * 0x10000;
		uint8_t* p0 = (dispctrl & 0x01) ? &gvram[pofs | 0x0000] : dummy;
		uint8_t* p1 = (dispctrl & 0x02) ? &gvram[pofs | 0x4000] : dummy;
		uint8_t* p2 = (dispctrl & 0x04) ? &gvram[pofs | 0x8000] : dummy;
		int ptr = dispaddr & 0x3ffe;
		
		for(int y = 0; y < 400; y += 2) {
			for(int x = 0; x < 640; x += 8) {
				uint8_t r = p0[ptr];
				uint8_t g = p1[ptr];
				uint8_t b = p2[ptr++];
				ptr &= 0x3fff;
				uint8_t* d = &screen_cg[y][x];
				
				d[0] = ((r & 0x80) >> 7) | ((g & 0x80) >> 6) | ((b & 0x80) >> 5);
				d[1] = ((r & 0x40) >> 6) | ((g & 0x40) >> 5) | ((b & 0x40) >> 4);
				d[2] = ((r & 0x20) >> 5) | ((g & 0x20) >> 4) | ((b & 0x20) >> 3);
				d[3] = ((r & 0x10) >> 4) | ((g & 0x10) >> 3) | ((b & 0x10) >> 2);
				d[4] = ((r & 0x08) >> 3) | ((g & 0x08) >> 2) | ((b & 0x08) >> 1);
				d[5] = ((r & 0x04) >> 2) | ((g & 0x04) >> 1) | ((b & 0x04) >> 0);
				d[6] = ((r & 0x02) >> 1) | ((g & 0x02) >> 0) | ((b & 0x02) << 1);
				d[7] = ((r & 0x01) >> 0) | ((g & 0x01) << 1) | ((b & 0x01) << 2);
			}
			memcpy(screen_cg[y + 1], screen_cg[y], 640);
		}
	}
}

#define STATE_VERSION	1

bool SUB::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return MEMORY::process_state(state_fio, loading);
}

