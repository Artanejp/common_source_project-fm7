/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

	[ memory and crtc ]
*/

#include "membus.h"
#include "../i8237.h"
#if defined(HAS_I86)
#include "../i86.h"
#elif defined(HAS_I286)
#include "../i286.h"
#endif

static const uint8_t bios1[] = {
	0xFA,				// cli
	0xDB,0xE3,			// fninit
	0xB8,0x00,0x7F,			// mov	ax,7F00
	0x8E,0xD0,			// mov	ss,ax
	0xBC,0x64,0x0F,			// mov	sp,0F64
	// init i/o
	0xB4,0x80,			// mov	ah,80
	0x9A,0x14,0x00,0xFB,0xFF,	// call	far FFFB:0014
	// boot from fdd
	0xB4,0x81,			// mov	ah,81
	0x9A,0x14,0x00,0xFB,0xFF,	// call	far FFFB:0014
	0x73,0x0B,			// jnb	$+11
	0x74,0xF5,			// jz	$-11
	// boot from scsi-hdd
	0xB4,0x82,			// mov	ah,82
	0x9A,0x14,0x00,0xFB,0xFF,	// call	far FFFB:0014
	0x72,0xEC,			// jb	$-20
	// goto ipl
	0x9A,0x04,0x00,0x00,0xB0,	// call	far B000:0004
	0xEB,0xE7			// jmp $-25
};

static const uint8_t bios2[] = {
	0xEA,0x00,0x00,0x00,0xFC,	// jmp	FC00:0000
	0x00,0x00,0x00,
	0xcf				// iret
};

void MEMBUS::initialize()
{
	// init memory
	memset(ram, 0, sizeof(ram));
	memset(vram, 0, sizeof(vram));
	memset(cvram, 0, sizeof(cvram));
	memset(kvram, 0, sizeof(kvram));
	memset(ipl, 0xff, sizeof(ipl));
	memset(kanji16, 0xff, sizeof(kanji16));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	} else {
		// load pseudo ipl
		memcpy(ipl + 0xc000, bios1, sizeof(bios1));
		memcpy(ipl + 0xfff0, bios2, sizeof(bios2));
		
		// ank8/16
		if(fio->Fopen(create_local_path(_T("ANK8.ROM")), FILEIO_READ_BINARY)) {
			fio->Fread(ipl, 0x800, 1);
			fio->Fclose();
		}
		if(fio->Fopen(create_local_path(_T("ANK16.ROM")), FILEIO_READ_BINARY)) {
			fio->Fread(ipl + 0x800, 0x1000, 1);
			fio->Fclose();
		}
	}
	if(fio->Fopen(create_local_path(_T("KANJI16.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji16, sizeof(kanji16), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory
	set_memory_rw(0x000000, sizeof(ram) - 1, ram);
	set_memory_r (0x0f0000, 0x0fffff, ipl);
#if defined(HAS_I286)
	set_memory_r (0xff0000, 0xffffff, ipl);
#endif
	
	// register event
	register_frame_event(this);
}

void MEMBUS::reset()
{
	// reset crtc
	lcdadr = 0;
	memset(lcdreg, 0, sizeof(lcdreg));
	dcr1 = dcr2 = 0;
	kj_l = kj_h = kj_ofs = kj_row = 0;
	blinkcnt = 0;
	
	// reset memory
	mcr1 = 2;
	mcr2 = a20 = 0;
	update_bank();
}

void MEMBUS::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	// memory controller
	case 0x1d:
#ifdef HAS_I286
		// protect mode ???
//		d_cpu->write_signal(SIG_I286_A20, data, 0x10);
#endif
		mcr1 = data;
		update_bank();
		break;
	case 0x1e:
		mcr2 = data;
		update_bank();
		break;
	case 0x26:
#ifdef HAS_I286
		// protect mode ???
		d_cpu->write_signal(SIG_I286_A20, data, 0x80);
#endif
		a20 = data;
		break;
	// dma bank
	case 0x120:
	case 0x121:
	case 0x122:
	case 0x123:
		d_dma->write_signal(SIG_I8237_BANK0 + (addr & 3), data, 0x0f);
		break;
	// lcd controller
	case 0x300:
		lcdadr = data;
		break;
	case 0x302:
		lcdreg[lcdadr & 31] = data;
		break;
	case 0x308:
		dcr1 = (dcr1 & 0xff00) | data;
		break;
	case 0x309:
		dcr1 = (dcr1 & 0xff) | (data << 8);
		// bit11-10: vram bank
		update_bank();
		break;
	case 0x30a:
		dcr1 = (dcr1 & 0xff00) | data;
		break;
	case 0x30b:
		dcr1 = (dcr1 & 0xff) | (data << 8);
		break;
	// kanji rom
	case 0x30c:
		kj_h = data & 0x7f;
		break;
	case 0x30d:
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
	case 0x30e:
		kanji16[(kj_ofs | ((kj_row & 0x0f) << 1)) & 0x3ffff] = data;
		break;
	case 0x30f:
		kanji16[(kj_ofs | ((kj_row++ & 0x0f) << 1) | 1) & 0x3ffff] = data;
		break;
	}
}

uint32_t MEMBUS::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	
	switch(addr & 0xffff) {
	case 0x1d:
		return mcr1;
	case 0x1e:
		return mcr2;
	case 0x26:
		return a20;
	// lcd controller
	case 0x300:
		return lcdadr;
	case 0x302:
		return lcdreg[lcdadr & 31];
	case 0x308:
		return dcr1 & 0xff;
	case 0x309:
		return (dcr1 >> 8) & 0xff;
	case 0x30a:
		return dcr2 & 0xff;
	case 0x30b:
		return (dcr2 >> 8) & 0xff;
	// kanji rom
	case 0x30c:
		return kj_h;
	case 0x30d:
		return kj_l;
	case 0x30e:
		return kanji16[(kj_ofs | ((kj_row & 0x0f) << 1)) & 0x3ffff];
	case 0x30f:
		return kanji16[(kj_ofs | ((kj_row++ & 0x0f) << 1) | 1) & 0x3ffff];
	}
	return 0xff;
}

void MEMBUS::event_frame()
{
	blinkcnt++;
}

void MEMBUS::update_bank()
{
	if(!(mcr2 & 1)) {
		// $c0000-$cffff: vram
		unset_memory_rw(0xc0000, 0xcffff);
		int bank = 0x8000 * ((dcr1 >> 10) & 3);
		set_memory_rw(0xc0000, 0xc7fff, vram + bank);
		set_memory_rw(0xc8000, 0xc8fff, cvram);
		set_memory_rw(0xca000, 0xcafff, kvram);
	} else {
		set_memory_rw(0xc0000, 0xcffff, ram + 0xc0000);
	}
	if(!(mcr1 & 1)) {
		// $f000-$ffff: rom
		unset_memory_w(0xf0000, 0xfffff);
		set_memory_r (0xf0000, 0xfffff, ipl);
	} else {
		set_memory_rw(0xf0000, 0xfffff, ram + 0xf0000);
	}
}

void MEMBUS::draw_screen()
{
	// render screen
	memset(screen_txt, 0, sizeof(screen_txt));
	memset(screen_cg, 0, sizeof(screen_cg));
	if(dcr1 & 2) {
		if(dcr1 & 8) {
			draw_text40();
		} else {
			draw_text80();
		}
	}
	if(dcr1 & 1) {
		draw_cg();
	}
	
	scrntype_t cd = RGB_COLOR(48, 56, 16);
	scrntype_t cb = RGB_COLOR(160, 168, 160);
	for(int y = 0; y < 400; y++) {
		scrntype_t* dest = emu->get_screen_buffer(y);
		uint8_t* txt = screen_txt[y];
		uint8_t* cg = screen_cg[y];
		
		for(int x = 0; x < 640; x++) {
			dest[x] = (txt[x] || cg[x]) ? cd : cb;
		}
	}
}

void MEMBUS::draw_text40()
{
	uint8_t *ank8 = ipl;
	uint8_t *ank16 = ipl + 0x800;
	
	int src = 0;//((lcdreg[12] << 9) | (lcdreg[13] << 1)) & 0xfff;
	int caddr = (lcdreg[10] & 0x20) ? -1 : (((lcdreg[14] << 9) | (lcdreg[15] << 1)) & 0xfff);
	int yofs = lcdreg[9] + 1;
	int ymax = 400 / yofs;
	int freq = (dcr1 >> 4) & 3;
	bool blink = !((freq == 3) || (blinkcnt & (32 >> freq)));
	
	for(int y = 0; y < ymax; y++) {
		for(int x = 0; x < 40; x++) {
			bool cursor = (src == caddr);
			int cx = x;
			uint8_t code = cvram[src];
			uint8_t h = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8_t attr = cvram[src];
			uint8_t l = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8_t col = attr & 0x27;
			bool blnk = blink && (attr & 0x10);
			bool rev = ((attr & 8) != 0);
			
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
					uint8_t pat0 = kanji16[ofs + (l << 1) + 0];
					uint8_t pat1 = kanji16[ofs + (l << 1) + 1];
					pat0 = blnk ? 0 : rev ? ~pat0 : pat0;
					pat1 = blnk ? 0 : rev ? ~pat1 : pat1;
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
					uint8_t pat = ank16[(code << 4) + l];
					pat = blnk ? 0 : rev ? ~pat : pat;
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
			if(cursor && !blink) {
				int st = lcdreg[10] & 0x1f;
				int ed = lcdreg[11] & 0x1f;
				for(int i = st; i <= ed && i < yofs; i++) {
					memset(&screen_txt[y * yofs + i][cx << 4], 7, 8);
				}
			}
		}
	}
}

void MEMBUS::draw_text80()
{
	uint8_t *ank8 = ipl;
	uint8_t *ank16 = ipl + 0x800;
	
	int src = 0;//((lcdreg[12] << 9) | (lcdreg[13] << 1)) & 0xfff;
	int caddr = (lcdreg[10] & 0x20) ? -1 : (((lcdreg[14] << 9) | (lcdreg[15] << 1)) & 0xfff);
	int yofs = lcdreg[9] + 1;
	int ymax = 400 / yofs;
	int freq = (dcr1 >> 4) & 3;
	bool blink = !((freq == 3) || (blinkcnt & (32 >> freq)));
	
	for(int y = 0; y < ymax; y++) {
		for(int x = 0; x < 80; x++) {
			bool cursor = (src == caddr);
			int cx = x;
			uint8_t code = cvram[src];
			uint8_t h = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8_t attr = cvram[src];
			uint8_t l = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8_t col = attr & 0x27;
			bool blnk = blink && (attr & 0x10);
			bool rev = ((attr & 8) != 0);
			
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
					uint8_t pat0 = kanji16[ofs + (l << 1) + 0];
					uint8_t pat1 = kanji16[ofs + (l << 1) + 1];
					pat0 = blnk ? 0 : rev ? ~pat0 : pat0;
					pat1 = blnk ? 0 : rev ? ~pat1 : pat1;
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
					uint8_t pat = ank16[(code << 4) + l];
					pat = blnk ? 0 : rev ? ~pat : pat;
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
			if(cursor && !blink) {
				int st = lcdreg[10] & 0x1f;
				int ed = lcdreg[11] & 0x1f;
				for(int i = st; i <= ed && i < yofs; i++) {
					memset(&screen_txt[y * yofs + i][cx << 3], 7, 8);
				}
			}
		}
	}
}

void MEMBUS::draw_cg()
{
	uint8_t* plane = vram + ((dcr1 >> 8) & 3) * 0x8000;
	int ptr = 0;
	
	for(int y = 0; y < 400; y++) {
		for(int x = 0; x < 640; x += 8) {
			uint8_t pat = plane[ptr++];
			uint8_t* d = &screen_cg[y][x];
			
			d[0] = pat & 0x80;
			d[1] = pat & 0x40;
			d[2] = pat & 0x20;
			d[3] = pat & 0x10;
			d[4] = pat & 0x08;
			d[5] = pat & 0x04;
			d[6] = pat & 0x02;
			d[7] = pat & 0x01;
		}
	}
}

#define STATE_VERSION	3

bool MEMBUS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateArray(cvram, sizeof(cvram), 1);
	state_fio->StateArray(kvram, sizeof(kvram), 1);
	state_fio->StateValue(mcr1);
	state_fio->StateValue(mcr2);
	state_fio->StateValue(a20);
	state_fio->StateValue(lcdadr);
	state_fio->StateArray(lcdreg, sizeof(lcdreg), 1);
	state_fio->StateValue(dcr1);
	state_fio->StateValue(dcr2);
	state_fio->StateValue(kj_h);
	state_fio->StateValue(kj_l);
	state_fio->StateValue(kj_ofs);
	state_fio->StateValue(kj_row);
	state_fio->StateValue(blinkcnt);
	
	// post process
	if(loading) {
		update_bank();
	}
	return true;
}

