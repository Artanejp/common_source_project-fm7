/*
	NEC PC-98DO Emulator 'ePC-98DO'
	NEC PC-8801MA Emulator 'ePC-8801MA'
	NEC PC-8001mkIISR Emulator 'ePC-8001mkIISR'

	Author : Takeda.Toshiya
	Date   : 2011.12.29-

	[ PC-8801 ]
*/

#include "pc88.h"
#include "../event.h"
#include "../i8251.h"
#include "../pcm1bit.h"
#include "../upd1990a.h"
#include "../ym2203.h"
#include "../z80.h"

#define DEVICE_JOYSTICK	0
#define DEVICE_MOUSE	1
#define DEVICE_JOYMOUSE	2	// not supported yet

#define EVENT_TIMER	0
#define EVENT_BUSREQ	1
#define EVENT_CMT_SEND	2
#define EVENT_CMT_DCD	3
#define EVENT_BEEP	4

#define IRQ_USART	0
#define IRQ_VRTC	1
#define IRQ_TIMER	2
#define IRQ_SOUND	4

#define Port30_40	!(port[0x30] & 0x01)
#define Port30_COLOR	!(port[0x30] & 0x02)
#define Port30_MTON	(port[0x30] & 0x08)
#define Port30_CMT	!(port[0x30] & 0x20)
#define Port30_RS232C	(port[0x30] & 0x20)

#define Port31_MMODE	(port[0x31] & 0x02)
#define Port31_RMODE	(port[0x31] & 0x04)
#define Port31_GRAPH	(port[0x31] & 0x08)
#define Port31_HCOLOR	(port[0x31] & 0x10)
#if !defined(_PC8001SR)
#define Port31_400LINE	!(port[0x31] & 0x11)
#else
#define Port31_400LINE	false
#endif

#define Port31_V1_320x200	(port[0x31] & 0x10)	// PC-8001 (V1)
#define Port31_V1_MONO		(port[0x31] & 0x04)	// PC-8001 (V1)

#define Port31_320x200	(port[0x31] & 0x04)	// PC-8001

#define Port32_EROMSL	(port[0x32] & 0x03)
#define Port32_TMODE	(port[0x32] & 0x10)
#if !defined(_PC8001SR)
#define Port32_PMODE	(port[0x32] & 0x20)
#else
#define Port32_PMODE	false
#endif
#define Port32_GVAM	(port[0x32] & 0x40)
#define Port32_SINTM	(port[0x32] & 0x80)

#define Port33_SINTM	(port[0x33] & 0x02)	// PC-8001
#define Port33_GVAM	(port[0x33] & 0x40)	// PC-8001
#define Port33_N80SR	(port[0x33] & 0x80)	// PC-8001

#define Port34_ALU	port[0x34]

#define Port35_PLN0	(port[0x35] & 0x01)
#define Port35_PLN1	(port[0x35] & 0x02)
#define Port35_PLN2	(port[0x35] & 0x04)
#define Port35_GDM	(port[0x35] & 0x30)
#define Port35_GAM	(port[0x35] & 0x80)

#define Port40_GHSM	(port[0x40] & 0x10)
#define Port40_JOP1	(port[0x40] & 0x40)

#define Port44_OPNCH	port[0x44]

#define Port53_TEXTDS	(port[0x53] & 0x01)
#define Port53_G0DS	(port[0x53] & 0x02)
#define Port53_G1DS	(port[0x53] & 0x04)
#define Port53_G2DS	(port[0x53] & 0x08)
#define Port53_G3DS	(port[0x53] & 0x10)	// PC-8001
#define Port53_G4DS	(port[0x53] & 0x20)	// PC-8001
#define Port53_G5DS	(port[0x53] & 0x40)	// PC-8001

#define Port70_TEXTWND	port[0x70]

#define Port71_EROM	port[0x71]

// XM8 version 1.20
#define PortA8_OPNCH	port[0xa8]
#define PortAA_S2INTM	(port[0xaa] & 0x80)

#define PortE2_RDEN	(port[0xe2] & 0x01)
#define PortE2_WREN	(port[0xe2] & 0x10)

#ifdef PC88_IODATA_EXRAM
#define PortE3_ERAMSL	port[0xe3]
#else
#define PortE3_ERAMSL	(port[0xe3] & 0x0f)
#endif

#define PortE8E9_KANJI1	(port[0xe8] | (port[0xe9] << 8))
#define PortECED_KANJI2	(port[0xec] | (port[0xed] << 8))

#define PortF0_DICROMSL	(port[0xf0] & 0x1f)
#define PortF1_DICROM	!(port[0xf1] & 0x01)

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x1000 * (i - sb); \
		} \
	} \
}

#define SET_BANK_W(s, e, w) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		} \
	} \
}

#define SET_BANK_R(s, e, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x1000 * (i - sb); \
		} \
	} \
}

static const int key_table[15][8] = {
	{ 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67 },
	{ 0x68, 0x69, 0x6a, 0x6b, 0x92, 0x6c, 0x6e, 0x0d },	// 0x92 = VK_OEM_NEC_EQUAL
	{ 0xc0, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 },
	{ 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f },
	{ 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57 },
	{ 0x58, 0x59, 0x5a, 0xdb, 0xdc, 0xdd, 0xde, 0xbd },
	{ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
	{ 0x38, 0x39, 0xba, 0xbb, 0xbc, 0xbe, 0xbf, 0xe2 },
//	{ 0x24, 0x26, 0x27, 0x2e, 0x12, 0x15, 0x10, 0x11 },
	{ 0x24, 0x26, 0x27, 0x08, 0x12, 0x15, 0x10, 0x11 },
	{ 0x13, 0x70, 0x71, 0x72, 0x73, 0x74, 0x20, 0x1b },
	{ 0x09, 0x28, 0x25, 0x23, 0x7b, 0x6d, 0x6f, 0x14 },
	{ 0x21, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x75, 0x76, 0x77, 0x78, 0x79, 0x08, 0x2d, 0x2e },
	{ 0x1c, 0x1d, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00 },
	{ 0x0d, 0x00, 0xa0, 0xa1, 0x00, 0x00, 0x00, 0x00 }
};

static const int key_conv_table[9][3] = {
	{0x2d, 0x2e, 1}, // INS	-> SHIFT + DEL
	{0x75, 0x70, 1}, // F6	-> SHIFT + F1
	{0x76, 0x71, 1}, // F7	-> SHIFT + F2
	{0x77, 0x72, 1}, // F8	-> SHIFT + F3
	{0x78, 0x73, 1}, // F9	-> SHIFT + F4
	{0x79, 0x74, 1}, // F10	-> SHIFT + F5
//	{0x08, 0x2e, 0}, // BS	-> DEL
	{0x2e, 0x08, 0}, // DEL	-> BS
	{0x1c, 0x20, 0}, // •ÏŠ·-> SPACE
	{0x1d, 0x20, 0}, // Œˆ’è-> SPACE
};

static const int16_t intr_mask2_table[8] = {
	~7, ~3, ~5, ~1, ~6, ~2, ~4, ~0
};

void PC88::initialize()
{
	memset(rdmy, 0xff, sizeof(rdmy));
//	memset(ram, 0, sizeof(ram));
#ifdef PC88_EXRAM_BANKS
	memset(exram, 0, sizeof(exram));
#endif
	memset(gvram, 0, sizeof(gvram));
	memset(gvram_null, 0, sizeof(gvram_null));
	memset(tvram, 0, sizeof(tvram));
#if defined(_PC8001SR)
	memset(n80mk2rom, 0xff, sizeof(n80mk2rom));
	memset(n80mk2srrom, 0xff, sizeof(n80mk2srrom));
#else
	memset(n88rom, 0xff, sizeof(n88rom));
	memset(n88exrom, 0xff, sizeof(n88exrom));
	memset(n80rom, 0xff, sizeof(n80rom));
#endif
	memset(kanji1, 0xff, sizeof(kanji1));
	memset(kanji2, 0xff, sizeof(kanji2));
#ifdef SUPPORT_PC88_DICTIONARY
	memset(dicrom, 0xff, sizeof(dicrom));
#endif
	
	// load rom images
	FILEIO* fio = new FILEIO();
#if defined(_PC8001SR)
	if(fio->Fopen(create_local_path(_T("N80_2.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(n80mk2rom, 0x8000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("N80_3.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(n80mk2srrom, 0xa000, 1);
		fio->Fclose();
	}
#else
	if(fio->Fopen(create_local_path(_T("PC88.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(n88rom, 0x8000, 1);
		fio->Fread(n80rom + 0x6000, 0x2000, 1);
		fio->Fseek(0x2000, FILEIO_SEEK_CUR);
		fio->Fread(n88exrom, 0x8000, 1);
		fio->Fseek(0x2000, FILEIO_SEEK_CUR);
		fio->Fread(n80rom, 0x6000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("N88.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(n88rom, 0x8000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("N88_0.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(n88exrom + 0x0000, 0x2000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("N88_1.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(n88exrom + 0x2000, 0x2000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("N88_2.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(n88exrom + 0x4000, 0x2000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("N88_3.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(n88exrom + 0x6000, 0x2000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("N80.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(n80rom, 0x8000, 1);
		fio->Fclose();
	}
#endif
	if(fio->Fopen(create_local_path(_T("KANJI1.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji1, 0x20000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("KANJI2.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji2, 0x20000, 1);
		fio->Fclose();
	}
#ifdef SUPPORT_PC88_DICTIONARY
	if(fio->Fopen(create_local_path(_T("JISYO.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(dicrom, 0x80000, 1);
		fio->Fclose();
	}
#endif
	delete fio;
	
	// memory pattern
	for(int i = 0, ofs = 0; i < 256; i++) {
		for(int j = 0; j < 16; j++) {
			static const uint8_t p0[256] = {
				0,1,0,1,0,1,0,0,0,0,0,0,1,0,1,0, // 0000
				0,1,0,1,0,1,0,0,0,0,1,0,1,0,1,0, // 1000
				0,1,0,1,0,0,0,0,0,0,1,0,1,0,1,0, // 2000
				0,1,0,1,0,0,0,0,0,0,1,0,1,0,1,0, // 3000
				1,0,1,0,1,0,1,1,1,1,1,1,0,1,0,1, // 4000
				1,0,1,0,1,0,1,1,1,1,0,1,0,1,0,1, // 5000
				1,0,1,0,1,1,1,1,1,1,0,1,0,1,0,1, // 6000
				1,0,1,0,1,1,1,1,1,1,0,1,0,1,0,1, // 7000
				1,0,1,0,1,0,1,1,1,1,1,1,0,1,0,1, // 8000
				1,0,1,0,1,0,1,1,1,1,1,1,0,1,0,1, // 9000
				1,0,1,0,1,0,1,1,1,1,0,1,0,1,0,1, // a000
				1,0,1,0,1,1,1,1,1,1,0,1,0,1,0,1, // b000
				0,1,0,1,0,1,0,0,0,0,0,0,1,0,1,0, // c000
				0,1,0,1,0,1,0,0,0,0,0,0,1,0,1,0, // d000
				0,1,0,1,0,1,0,0,0,0,1,0,1,0,1,0, // e000
				0,1,0,1,0,0,0,0,0,0,1,0,1,0,1,0, // f000
			};
			static const uint8_t p1[16] = {
				0x00,0xff,0x00,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0x00,0xff,0xff,0x00,0xff,0x00,
			};
			memset(ram + ofs, (p0[i] == 0) ? p1[j] : ~p1[j], 16);
			ofs += 16;
		}
	}
	
	// create semi graphics pattern
	for(int i = 0; i < 256; i++) {
		uint8_t *dest = sg_pattern + 8 * i;
		dest[0] = dest[1] = ((i & 1) ? 0xf0 : 0) | ((i & 0x10) ? 0x0f : 0);
		dest[2] = dest[3] = ((i & 2) ? 0xf0 : 0) | ((i & 0x20) ? 0x0f : 0);
		dest[4] = dest[5] = ((i & 4) ? 0xf0 : 0) | ((i & 0x40) ? 0x0f : 0);
		dest[6] = dest[7] = ((i & 8) ? 0xf0 : 0) | ((i & 0x80) ? 0x0f : 0);
	}
	
	// initialize text/graph palette
	for(int i = 0; i < 8; i++) {
		palette_text_pc[i] = RGBA_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0, 255); // A is a flag for crt filter
	}
	for(int i = 0; i < 8; i++) {
		palette_graph_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}
	palette_text_pc[8] = palette_graph_pc[8] = 0;
	
#ifdef SUPPORT_PC88_HIGH_CLOCK
	cpu_clock_low = (config.cpu_type == 1);		// 4MHz
	cpu_clock_high_fe2 = (config.cpu_type == 2);	// 8MHz (FE2/MC)
#else
	cpu_clock_low = true;
#endif
	
#ifdef SUPPORT_PC88_JOYSTICK
	joystick_status = emu->get_joy_buffer();
	mouse_status = emu->get_mouse_buffer();
	mouse_strobe_clock_lim = (int)((cpu_clock_low ? 720 : 1440) * 1.25);
#endif
	
	// initialize cmt
	cmt_fio = new FILEIO();
	cmt_play = cmt_rec = false;
	
	register_frame_event(this);
	register_vline_event(this);
	register_event(this, EVENT_TIMER, 1000000.0 / 600.0, true, NULL);
	register_event(this, EVENT_BEEP, 1000000.0 / 4800.0, true, NULL);
}

void PC88::release()
{
	release_tape();
	delete cmt_fio;
}

void PC88::reset()
{
#if defined(_PC8001SR)
	hireso = false;
#else
	hireso = (config.monitor_type == 0);
#endif
	
	// memory
	memset(port, 0, sizeof(port));
	port[0x31] = 0x01;
	port[0x32] = 0x98;
	for(int i = 0; i < 8; i++) {
		port[0x54 + i] = i;
	}
//	port[0x70] = 0x80;	// XM8 version 1.10
	port[0x71] = port[0xf1] = 0xff;
	
	memset(alu_reg, 0, sizeof(alu_reg));
	gvram_plane = gvram_sel = 0;
	
#if defined(_PC8001SR)
	if(config.boot_mode == MODE_PC80_V2) {
		SET_BANK(0x0000, 0x7fff, wdmy, n80mk2srrom);
		port[0x33] = 0x80;
	} else {
		SET_BANK(0x0000, 0x7fff, wdmy, n80mk2rom);
	}
	SET_BANK(0x8000, 0xffff, ram + 0x8000, ram + 0x8000);
#else
	SET_BANK(0x0000, 0x7fff, ram, n88rom);
	SET_BANK(0x8000, 0xefff, ram + 0x8000, ram + 0x8000);
	update_tvram_memmap();	// XM8 version 1.10
#endif
	
	// misc
	usart_dcd = false;
	opn_busy = true;
	
	// memory wait
	mem_wait_on = ((config.dipswitch & 1) != 0);
	
	m1_wait_clocks = get_m1_wait(false);
	f000_m1_wait_clocks = get_m1_wait(true);
	mem_wait_clocks_r = get_main_wait(true);
	mem_wait_clocks_w = get_main_wait(false);
	tvram_wait_clocks_r = get_tvram_wait(true);
	tvram_wait_clocks_w = get_tvram_wait(false);
	
	// crtc
	memset(&crtc, 0, sizeof(crtc));
	crtc.reset(hireso);
	update_timing();
	
	for(int i = 0; i < 9; i++) {
		palette[i].b = (i & 1) ? 7 : 0;
		palette[i].r = (i & 2) ? 7 : 0;
		palette[i].g = (i & 4) ? 7 : 0;
	}
	update_palette = true;
	
	// dma
	memset(&dmac, 0, sizeof(dmac));
	dmac.mem = dmac.ch[2].io = this;
	dmac.ch[0].io = dmac.ch[1].io = dmac.ch[3].io = vm->dummy;
	dmac.ch[0].addr.b.l = 0x56;	// XM8 version 1.10
	dmac.ch[0].addr.b.h = 0x56;
	dmac.ch[1].addr.b.l = 0x7a;
	dmac.ch[1].addr.b.h = 0x7a;
	
	// keyboard
	key_kana = key_caps = 0;
	
	// mouse
#ifdef SUPPORT_PC88_JOYSTICK
	mouse_strobe_clock = get_current_clock();
	mouse_phase = -1;
	mouse_dx = mouse_dy = mouse_lx = mouse_ly = 0;
#endif
	
	// interrupt
	intr_req = intr_mask1 = intr_mask2 = 0;
	intr_req_sound = false;
#ifdef SUPPORT_PC88_SB2
	intr_req_sb2 = false;
#endif
	
	// fdd i/f
	d_pio->write_io8(1, 0);
	d_pio->write_io8(2, 0);
	
	// data recorder
	close_tape();
	cmt_play = cmt_rec = false;
	cmt_register_id = -1;
	
	// beep/sing
	beep_on = beep_signal = sing_signal = false;
	
#ifdef SUPPORT_PC88_PCG8100
	// pcg
	memcpy(pcg_pattern, kanji1 + 0x1000, sizeof(pcg_pattern));
	write_io8(1, 0);
	write_io8(2, 0);
	write_io8(3, 0);
#endif
#ifdef NIPPY_PATCH
	// dirty patch for NIPPY
	nippy_patch = false;
#endif
}

void PC88::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	addr &= 0xffff;
	*wait = mem_wait_clocks_w;
	
#if !defined(_PC8001SR)
	if((addr & 0xfc00) == 0x8000) {
		// text window
		if(!Port31_MMODE && !Port31_RMODE) {
			addr = (Port70_TEXTWND << 8) + (addr & 0x3ff);
		}
		ram[addr & 0xffff] = data;
		return;
	} else if((addr & 0xc000) == 0xc000) {
#else
	if((addr & 0xc000) == 0x8000) {
#endif
		switch(gvram_sel) {
		case 1:
			*wait = gvram_wait_clocks_w;
			gvram[(addr & 0x3fff) | 0x0000] = data;
			return;
		case 2:
			*wait = gvram_wait_clocks_w;
			gvram[(addr & 0x3fff) | 0x4000] = data;
			return;
		case 4:
			*wait = gvram_wait_clocks_w;
			gvram[(addr & 0x3fff) | 0x8000] = data;
			return;
		case 8:
			*wait = gvram_wait_clocks_w;
			addr &= 0x3fff;
			switch(Port35_GDM) {
			case 0x00:
				for(int i = 0; i < 3; i++) {
					switch((Port34_ALU >> i) & 0x11) {
					case 0x00:	// reset
						gvram[addr | (0x4000 * i)] &= ~data;
						break;
					case 0x01:	// set
						gvram[addr | (0x4000 * i)] |= data;
						break;
					case 0x10:	// reverse
						gvram[addr | (0x4000 * i)] ^= data;
						break;
					}
				}
				break;
			case 0x10:
				gvram[addr | 0x0000] = alu_reg[0];
				gvram[addr | 0x4000] = alu_reg[1];
				gvram[addr | 0x8000] = alu_reg[2];
				break;
			case 0x20:
				gvram[addr | 0x0000] = alu_reg[1];
				break;
			case 0x30:
				gvram[addr | 0x4000] = alu_reg[0];
				break;
			}
			return;
		}
	}
#if !defined(_PC8001SR)
	if((addr & 0xf000) == 0xf000) {
		// high speed ram
		*wait += tvram_wait_clocks_w;
	}
#endif
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32_t PC88::read_data8w(uint32_t addr, int* wait)
{
	addr &= 0xffff;
	*wait = mem_wait_clocks_r;
	
#if !defined(_PC8001SR)
	if((addr & 0xfc00) == 0x8000) {
		// text window
		if(!Port31_MMODE && !Port31_RMODE) {
			addr = (Port70_TEXTWND << 8) + (addr & 0x3ff);
		}
		return ram[addr & 0xffff];
	} else if((addr & 0xc000) == 0xc000) {
#else
	if((addr & 0xc000) == 0x8000) {
#endif
		uint8_t b, r, g;
		switch(gvram_sel) {
		case 1:
			*wait = gvram_wait_clocks_r;
			return gvram[(addr & 0x3fff) | 0x0000];
		case 2:
			*wait = gvram_wait_clocks_r;
			return gvram[(addr & 0x3fff) | 0x4000];
		case 4:
			*wait = gvram_wait_clocks_r;
			return gvram[(addr & 0x3fff) | 0x8000];
		case 8:
			*wait = gvram_wait_clocks_r;
			addr &= 0x3fff;
			alu_reg[0] = gvram[addr | 0x0000];
			alu_reg[1] = gvram[addr | 0x4000];
			alu_reg[2] = gvram[addr | 0x8000];
			b = alu_reg[0]; if(!Port35_PLN0) b ^= 0xff;
			r = alu_reg[1]; if(!Port35_PLN1) r ^= 0xff;
			g = alu_reg[2]; if(!Port35_PLN2) g ^= 0xff;
			return b & r & g;
		}
#ifdef SUPPORT_PC88_DICTIONARY
		if(PortF1_DICROM) {
			return dicrom[(addr & 0x3fff) | (0x4000 * PortF0_DICROMSL)];
		}
#endif
	}
#if !defined(_PC8001SR)
	if((addr & 0xf000) == 0xf000) {
		// high speed ram
		*wait += tvram_wait_clocks_r;
	}
#endif
	return rbank[addr >> 12][addr & 0xfff];
}

uint32_t PC88::fetch_op(uint32_t addr, int *wait)
{
	uint32_t data = read_data8w(addr, wait);
	if((addr & 0xf000) == 0xf000) {
		*wait += f000_m1_wait_clocks;
	} else {
		*wait += m1_wait_clocks;
	}
	return data;
}

void PC88::write_io8(uint32_t addr, uint32_t data)
{
	addr &= 0xff;
#ifdef _IO_DEBUG_LOG
	this->out_debug_log(_T("%6x\tOUT8\t%02x,%02x\n"), d_cpu->get_pc(), addr, data);
#endif
#ifdef NIPPY_PATCH
	// dirty patch for NIPPY
	if(addr == 0x31 && data == 0x3f && d_cpu->get_pc() == 0xaa4f && nippy_patch) {
		data = 0x39; // select n88rom
	}
	// poke &haa4e, &h39
#endif
	uint8_t mod = port[addr] ^ data;
	port[addr] = data;
	
	switch(addr) {
	case 0x00:
#ifdef SUPPORT_PC88_PCG8100
		pcg_data = data;
#endif
		// load tape image ??? (from QUASI88)
		if(cmt_play) {
			while(cmt_buffer[cmt_bufptr++] != 0x3a) {
				if(!(cmt_bufptr <= cmt_bufcnt)) return;
			}
			int val, sum, ptr, len, wait;
			sum = (val = cmt_buffer[cmt_bufptr++]);
			ptr = val << 8;
			sum += (val = cmt_buffer[cmt_bufptr++]);
			ptr |= val;
			sum += (val = cmt_buffer[cmt_bufptr++]);
			if((sum & 0xff) != 0) return;
			
			while(1) {
				while(cmt_buffer[cmt_bufptr++] != 0x3a) {
					if(!(cmt_bufptr <= cmt_bufcnt)) return;
				}
				sum = (len = cmt_buffer[cmt_bufptr++]);
				if(len == 0) break;
				for(; len; len--) {
					sum += (val = cmt_buffer[cmt_bufptr++]);
					write_data8w(ptr++, val, &wait);
				}
				sum += cmt_buffer[cmt_bufptr++];
				if((sum & 0xff) != 0) return;
			}
		}
		break;
#ifdef SUPPORT_PC88_PCG8100
	case 0x01:
		pcg_addr = (pcg_addr & 0x300) | data;
		break;
	case 0x02:
		if((pcg_ctrl & 0x10) && !(data & 0x10)) {
			if(pcg_ctrl & 0x20) {
				pcg_pattern[0x400 | pcg_addr] = kanji1[0x1400 | pcg_addr];
			} else {
				pcg_pattern[0x400 | pcg_addr] = pcg_data;
			}
		}
		pcg_addr = (pcg_addr & 0x0ff) | ((data & 3) << 8);
		pcg_ctrl = data;
		d_pcg_pcm0->write_signal(SIG_PCM1BIT_ON, data, 0x08);
		d_pcg_pcm1->write_signal(SIG_PCM1BIT_ON, data, 0x40);
		d_pcg_pcm2->write_signal(SIG_PCM1BIT_ON, data, 0x80);
		break;
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
		d_pcg_pit->write_io8(addr & 3, data);
#endif
		break;
	case 0x10:
		d_prn->write_signal(SIG_PRINTER_DATA, data, 0xff);
		d_rtc->write_signal(SIG_UPD1990A_C0, data, 1);
		d_rtc->write_signal(SIG_UPD1990A_C1, data, 2);
		d_rtc->write_signal(SIG_UPD1990A_C2, data, 4);
		d_rtc->write_signal(SIG_UPD1990A_DIN, data, 8);
		break;
	case 0x20:
	case 0x21:
		d_sio->write_io8(addr, data);
		break;
	case 0x30:
		if(mod & 0x08) {
			if(Port30_MTON) {
				// start motor
				if(cmt_play && cmt_bufptr < cmt_bufcnt) {
#if 0
					// skip to the top of next block
					int tmp = cmt_bufptr;
					while(cmt_bufptr < cmt_bufcnt) {
						if(check_data_carrier()) {
							break;
						}
						cmt_bufptr++;
					}
					if(cmt_bufptr == cmt_bufcnt) {
						cmt_bufptr = tmp;
					}
#endif
					if(cmt_register_id != -1) {
						cancel_event(this, cmt_register_id);
					}
					register_event(this, EVENT_CMT_SEND, 5000, false, &cmt_register_id);
				}
			} else {
				// stop motor
				if(cmt_register_id != -1) {
					cancel_event(this, cmt_register_id);
					cmt_register_id = -1;
				}
				usart_dcd = true; // for Jackie Chan no Spartan X
			}
		}
		break;
#if defined(_PC8001SR)
	case 0x31:
		if(mod & 0x03) {
			update_n80_write();
			update_n80_read();
		}
		if(mod & 0xfc) {
			palette[8].b = (data & 0x20) ? 7 : 0;
			palette[8].r = (data & 0x40) ? 7 : 0;
			palette[8].g = (data & 0x80) ? 7 : 0;
			update_palette = true;
		}
		break;
	case 0x33:
		if(mod & 0x80) {
			update_n80_read();
			update_gvram_wait();
			update_palette = true;
		}
		if(mod & 0xc0) {
			update_gvram_sel();
		}
		if(mod & 0x02) {
			if(intr_req_sound && !Port33_SINTM) {
				request_intr(IRQ_SOUND, true);
			}
		}
		break;
#else
	case 0x31:
		if(mod & 0x06) {
			update_low_memmap();
		}
		if(mod & 0x08) {
			update_gvram_wait();
			update_palette = true;
		}
		if(mod & 0x11) {
			update_timing();
			update_palette = true;
		}
#ifdef NIPPY_PATCH
		// dirty patch for NIPPY
		nippy_patch = (data == 0x37 && d_cpu->get_pc() == 0xaa32);
#endif
		break;
	case 0x32:
		if(mod & 0x03) {
			if(!(Port71_EROM & 1)) {
				update_low_memmap();
			}
		}
		if(mod & 0x10) {
			// XM8 version 1.10
//			if(config.boot_mode == MODE_PC88_V1H || config.boot_mode == MODE_PC88_V2) {
				update_tvram_memmap();
				f000_m1_wait_clocks = get_m1_wait(true);
//			}
		}
		if(mod & 0x40) {
			update_gvram_sel();
		}
		if(mod & 0x80) {
			if(intr_req_sound && !Port32_SINTM) {
				request_intr(IRQ_SOUND, true);
			}
		}
		break;
#endif
	case 0x35:
		if(mod & 0x80) {
			update_gvram_sel();
		}
		break;
	case 0x40:
		d_prn->write_signal(SIG_PRINTER_STROBE, data, 1);
		d_rtc->write_signal(SIG_UPD1990A_STB, ~data, 2);
		d_rtc->write_signal(SIG_UPD1990A_CLK, data, 4);
		// bit3: crtc i/f sync mode
		if(mod & 0x10) {
			update_gvram_wait();
		}
		beep_on = ((data & 0x20) != 0);
#ifdef SUPPORT_PC88_JOYSTICK
		if(mod & 0x40) {
			if(Port40_JOP1 && (mouse_phase == -1 || get_passed_clock(mouse_strobe_clock) > mouse_strobe_clock_lim)) {
				mouse_phase = 0;//mouse_dx = mouse_dy = 0;
			} else {
				mouse_phase = (mouse_phase + 1) & 3;
			}
			if(mouse_phase == 0) {
				// latch position
				mouse_lx = -((mouse_dx > 127) ? 127 : (mouse_dx < -127) ? -127 : mouse_dx);
				mouse_ly = -((mouse_dy > 127) ? 127 : (mouse_dy < -127) ? -127 : mouse_dy);
				mouse_dx = mouse_dy = 0;
			}
			mouse_strobe_clock = get_current_clock();
		}
#endif
		sing_signal = ((data & 0x80) != 0);
		d_pcm->write_signal(SIG_PCM1BIT_SIGNAL, ((beep_on && beep_signal) || sing_signal) ? 1 : 0, 1);
		break;
	case 0x44:
	case 0x45:
		d_opn->write_io8(addr, data);
		break;
#ifdef SUPPORT_PC88_OPNA
	case 0x46:
	case 0x47:
		if(d_opn->is_ym2608) {
			d_opn->write_io8(addr, data);
		}
		break;
#endif
	case 0x50:
		crtc.write_param(data);
		if(crtc.timing_changed) {
			update_timing();
			crtc.timing_changed = false;
		}
		break;
	case 0x51:
		crtc.write_cmd(data);
		break;
#if !defined(_PC8001SR)
	case 0x52:
		palette[8].b = (data & 0x10) ? 7 : 0;
		palette[8].r = (data & 0x20) ? 7 : 0;
		palette[8].g = (data & 0x40) ? 7 : 0;
		update_palette = true;
		break;
#endif
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
	case 0x58:
	case 0x59:
	case 0x5a:
	case 0x5b:
#if !defined(_PC8001SR)
		if(Port32_PMODE) {
			int n = (data & 0x80) ? 8 : (addr - 0x54);
			if(data & 0x40) {
				palette[n].g = data & 7;
			} else {
				palette[n].b = data & 7;
				palette[n].r = (data >> 3) & 7;
			}
		} else {
			int n = addr - 0x54;
			palette[n].b = (data & 1) ? 7 : 0;
			palette[n].r = (data & 2) ? 7 : 0;
			palette[n].g = (data & 4) ? 7 : 0;
		}
#endif
		update_palette = true;
		break;
	case 0x5c:
		if(gvram_plane != 1) {
			gvram_plane = 1;
			update_gvram_sel();
		}
		break;
	case 0x5d:
		if(gvram_plane != 2) {
			gvram_plane = 2;
			update_gvram_sel();
		}
		break;
	case 0x5e:
		if(gvram_plane != 4) {
			gvram_plane = 4;
			update_gvram_sel();
		}
		break;
	case 0x5f:
		if(gvram_plane != 0) {
			gvram_plane = 0;
			update_gvram_sel();
		}
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
		dmac.write_io8(addr, data);
		break;
#if defined(_PC8001SR)
	case 0x71:
		if((mod & 1) && Port33_N80SR) {
			update_n80_read();
		}
		break;
	case 0xe2:
		if(mod & 0x01) {
			update_n80_read();
		}
		if(mod & 0x10) {
			update_n80_write();
		}
		break;
	case 0xe3:
		if(mod) {
			update_n80_write();
			update_n80_read();
		}
		break;
#else
	case 0x71:
		if(mod) {
			update_low_memmap();
		}
		break;
	case 0x78:
		Port70_TEXTWND++;
		break;
#ifdef SUPPORT_PC88_SB2
	case 0xa8:
	case 0xa9:
		if(d_sb2 != NULL) {
			d_sb2->write_io8(addr, data);
		}
		break;
	case 0xaa:
		if(mod & 0x80) {
			if(intr_req_sb2 && !PortAA_S2INTM) {
				request_intr(IRQ_SOUND, true);
			}
		}
		break;
#ifdef SUPPORT_PC88_OPNA
	case 0xac:
	case 0xad:
		if(d_sb2 != NULL && d_sb2->is_ym2608) {
			d_sb2->write_io8(addr | 2, data);
		}
		break;
#endif
#endif
	case 0xe2:
		if(mod & 0x11) {
			update_low_memmap();
		}
		break;
	case 0xe3:
		if(mod) {
			update_low_memmap();
		}
		break;
#endif
	case 0xe4:
		intr_mask1 = ~(0xff << (data < 8 ? data : 8));
		update_intr();
		break;
	case 0xe6:
		// for Romancia (XM8 version 1.00)
		if((uint8_t)intr_mask2_table[data & 7] != intr_mask2) {
			intr_req &= ((uint8_t)intr_mask2_table[data & 7] & intr_mask2);
		}
		intr_mask2 = (uint8_t)intr_mask2_table[data & 7];
		intr_req &= intr_mask2;
		update_intr();
		break;
#ifdef SUPPORT_PC88_DICTIONARY
	case 0xf0:
		// XM8 version 1.20
		if(port[0xf0] >= 0x20) {
			// no effect if data >= 0x20
			port[0xf0] ^= mod;
		}
		break;
	case 0xf1:
		// XM8 version 1.20
		if(port[0xf1] != 0x00 && port[0xf1] != 0x01) {
			// effect only 0x00 or 0x01
			port[0xf1] ^= mod;
		}
		break;
#endif
	case 0xfc:
	case 0xfd:
	case 0xfe:
	case 0xff:
		d_pio->write_io8(addr, data);
		break;
	}
}

uint32_t PC88::read_io8(uint32_t addr)
#ifdef _IO_DEBUG_LOG
{
	uint32_t val = read_io8_debug(addr);
	this->out_debug_log(_T("%06x\tIN8\t%02x = %02x\n"), d_cpu->get_pc(), addr & 0xff, val);
	return val;
}

uint32_t PC88::read_io8_debug(uint32_t addr)
#endif
{
	uint32_t val = 0xff;
	
	addr &= 0xff;
	switch(addr) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
		for(int i = 0; i < 8; i++) {
			if(key_status[key_table[addr & 0x0f][i]]) {
				val &= ~(1 << i);
			}
		}
		if(addr == 0x0e) {
			val &= ~0x80; // http://www.maroon.dti.ne.jp/youkan/pc88/iomap.html
		}
		return val;
	case 0x20:
	case 0x21:
		return d_sio->read_io8(addr);
#if defined(_PC8001SR)
	case 0x30:
		return (config.boot_mode == MODE_PC80_N ? 0 : 1) | (config.boot_mode == MODE_PC80_V2 ? 0 : 2) | 0xfc;
	case 0x31:
		return (config.boot_mode == MODE_PC80_V2 ? 0 : 0x80) | 0x39;
	case 0x33:
		return port[0x33];
#else
	case 0x30:
//		return (config.boot_mode == MODE_PC88_N ? 0 : 1) | 0xca; // 80x20 (XM8 version 1.00)
		return (config.boot_mode == MODE_PC88_N ? 0 : 1) | 0xc2; // 80x25
	case 0x31:
		// XM8 version 1.10
		return (config.boot_mode == MODE_PC88_V2 ? 0 : 0x80) | (config.boot_mode == MODE_PC88_V1S || config.boot_mode == MODE_PC88_N ? 0 : 0x40) | 0x39;
//		return (config.boot_mode == MODE_PC88_V2 ? 0 : 0x80) | (config.boot_mode == MODE_PC88_V1S || config.boot_mode == MODE_PC88_N ? 0 : 0x40);
	case 0x32:
		return port[0x32];
#endif
	case 0x40:
		// XM8 version 1.10
//		return (crtc.vblank ? 0x20 : 0) | (d_rtc->read_signal(0) ? 0x10 : 0) | (usart_dcd ? 4 : 0) | (hireso ? 0 : 2) | 0xc1;
		return (crtc.vblank ? 0x20 : 0) | (d_rtc->read_signal(0) ? 0x10 : 0) | (usart_dcd ? 4 : 0) | (hireso ? 0 : 2) | (d_prn->read_signal(SIG_PRINTER_BUSY) ? 1 : 0) | 0xc0;
	case 0x44:
		val = d_opn->read_io8(addr);
		if(opn_busy) {
			// show busy flag for first access (for ALPHA)
			if(d_cpu->get_pc() == 0xe615) {
				val |= 0x80;
			}
			opn_busy = false;
		}
		return val;
	case 0x45:
		if(Port44_OPNCH == 14) {
#ifdef SUPPORT_PC88_JOYSTICK
			if(config.joystick_type == DEVICE_JOYSTICK) {
				return (~(joystick_status[0] >> 0) & 0x0f) | 0xf0;
			} else if(config.joystick_type == DEVICE_MOUSE) {
				switch(mouse_phase) {
				case 0:
					return ((mouse_lx >> 4) & 0x0f) | 0xf0;
				case 1:
					return ((mouse_lx >> 0) & 0x0f) | 0xf0;
				case 2:
					return ((mouse_ly >> 4) & 0x0f) | 0xf0;
				case 3:
					return ((mouse_ly >> 0) & 0x0f) | 0xf0;
				}
				return 0xf0; // ???
			}
#endif
			return 0xff;
		} else if(Port44_OPNCH == 15) {
#ifdef SUPPORT_PC88_JOYSTICK
			if(config.joystick_type == DEVICE_JOYSTICK) {
				return (~(joystick_status[0] >> 4) & 0x03) | 0xfc;
			} else if(config.joystick_type == DEVICE_MOUSE) {
				return (~mouse_status[2] & 0x03) | 0xfc;
			}
#endif
			return 0xff;
		}
		return d_opn->read_io8(addr);
#ifdef SUPPORT_PC88_OPNA
	case 0x46:
	case 0x47:
		if(d_opn->is_ym2608) {
			return d_opn->read_io8(addr);
		}
		break;
#endif
	case 0x50:
		return crtc.read_param();
	case 0x51:
		return crtc.read_status();
	case 0x5c:
		return gvram_plane | 0xf8;
	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
	case 0x68:
		return dmac.read_io8(addr);
	case 0x6e:
		// XM8 version 1.20
		return (cpu_clock_low ? 0x80 : 0) | (is_sr_mr() ? 0x7f : 0x10);
	case 0x6f:
		// XM8 version 1.20
		return is_sr_mr() ? 0xff : (port[0x6f] | 0xf0);
#if !defined(_PC8001SR)
	case 0x70:
		// PC-8001mkIISR returns the constant value
		// this port is used to detect PC-8001mkIISR or 8801mkIISR
		return port[0x70];
#endif
	case 0x71:
		return port[0x71];
#ifdef SUPPORT_PC88_SB2
	case 0xa8:
		if(d_sb2 != NULL) {
			return d_sb2->read_io8(addr);
		}
		break;
	case 0xa9:
		if(d_sb2 != NULL) {
			if(PortA8_OPNCH == 14) {
#ifdef SUPPORT_PC88_JOYSTICK
				if(config.joystick_type == DEVICE_JOYSTICK) {
					return (~(joystick_status[0] >> 0) & 0x0f) | 0xf0;
				} else if(config.joystick_type == DEVICE_MOUSE) {
					switch(mouse_phase) {
					case 0:
						return ((mouse_lx >> 4) & 0x0f) | 0xf0;
					case 1:
						return ((mouse_lx >> 0) & 0x0f) | 0xf0;
					case 2:
						return ((mouse_ly >> 4) & 0x0f) | 0xf0;
					case 3:
						return ((mouse_ly >> 0) & 0x0f) | 0xf0;
					}
					return 0xf0; // ???
				}
#endif
				return 0xff;
			} else if(PortA8_OPNCH == 15) {
#ifdef SUPPORT_PC88_JOYSTICK
				if(config.joystick_type == DEVICE_JOYSTICK) {
					return (~(joystick_status[0] >> 4) & 0x03) | 0xfc;
				} else if(config.joystick_type == DEVICE_MOUSE) {
					return (~mouse_status[2] & 0x03) | 0xfc;
				}
#endif
				return 0xff;
			}
			return d_sb2->read_io8(addr);
		}
		break;
	case 0xaa:
		return (PortAA_S2INTM) | 0x7f;
#ifdef SUPPORT_PC88_OPNA
	case 0xac:
	case 0xad:
		if(d_sb2 != NULL && d_sb2->is_ym2608) {
			d_sb2->read_io8(addr | 2);
		}
		break;
#endif
#endif
	case 0xe2:
		return (~port[0xe2]) | 0xee;
	case 0xe3:
#ifdef PC88_IODATA_EXRAM
		return port[0xe3];
#else
		return port[0xe3] | 0xf0;
#endif
	case 0xe8:
		return kanji1[PortE8E9_KANJI1 * 2 + 1];
	case 0xe9:
		return kanji1[PortE8E9_KANJI1 * 2];
	case 0xec:
		return kanji2[PortECED_KANJI2 * 2 + 1];
	case 0xed:
		return kanji2[PortECED_KANJI2 * 2];
	case 0xfc:
	case 0xfd:
	case 0xfe:
		return d_pio->read_io8(addr);
	}
	return 0xff;
}

uint32_t PC88::read_dma_data8(uint32_t addr)
{
#if defined(_PC8001SR)
	return ram[addr & 0xffff];
#else
	if((addr & 0xf000) == 0xf000 && (config.boot_mode == MODE_PC88_V1H || config.boot_mode == MODE_PC88_V2)) {
		return tvram[addr & 0xfff];
	} else {
		return ram[addr & 0xffff];
	}
#endif
}

void PC88::write_dma_io8(uint32_t addr, uint32_t data)
{
	// to crtc
	crtc.write_buffer(data);
}

void PC88::update_timing()
{
	int lines_per_frame = (crtc.height + crtc.vretrace) * crtc.char_height;
	// 56.4229Hz (25line) on PC-8801MA2 (XM8 version 1.00)
	double frames_per_sec = (hireso ? 24860.0 * 56.423 / 56.5 : 15980.0) / (double)lines_per_frame;
//	double frames_per_sec = (hireso ? 24860.0 * 56.424 / 56.5 : 15980.0) / (double)lines_per_frame;
	
	set_frames_per_sec(frames_per_sec);
	set_lines_per_frame(lines_per_frame);
}

int PC88::get_m1_wait(bool addr_f000)
{
	// XM8 version 1.20
	int wait = 0;
	
#if defined(_PC8001SR)
	if(config.boot_mode == MODE_PC80_V1 || config.boot_mode == MODE_PC80_N) {
#else
	if(config.boot_mode == MODE_PC88_V1S || config.boot_mode == MODE_PC88_N) {
#endif
		// V1S or N
		if(!mem_wait_on) {
			// memory wait = off
			if(cpu_clock_low) {
				// 4MHz
				wait += 1;
			}
		}
	} else {
		// V1H or V2
		if(!mem_wait_on) {
			// no memory wait
			if(addr_f000) {
				// TVRAM
				if(!gvram_sel && !Port32_TMODE) {
					if(cpu_clock_low) {
						// TVRAM, 4MHz only
						wait += 1;
					}
				}
			}
		}
	}
	return wait;
}

int  PC88::get_main_wait(bool read)
{
	// XM8 version 1.20
	int wait = 0;
	
	if(cpu_clock_low) {
		// 4MHz
		if(mem_wait_on) {
			if(read) {
				// memory wait + read
				wait += 1;
			}
		}
	} else {
		// 8MHz
#if !defined(_PC8001SR)
		if(!cpu_clock_high_fe2) {
			// not 8MHzH
			wait += 1;
		}
#endif
		if(mem_wait_on) {
			// memory wait (read and write)
			wait += 1;
		}
	}
	return wait;
}

int PC88::get_tvram_wait(bool read)
{
	int wait = 0;
	
	if(cpu_clock_low) {
		// 4MHz
		if(read) {
			if(mem_wait_on) {
				// memory wait + read
				wait += 1;
			}
		}
	} else {
		// 8MHz -> memory wait do not effect
		if(read) {
			// read
			wait += 2;
		} else {
			// write
			wait += 1;
		}
	}
	return wait;
}

int  PC88::get_gvram_wait(bool read)
{
	// XM8 version 1.20
	int wait = 0;
	
	if(Port31_GRAPH) {
		// graphic on
		if(cpu_clock_low) {
			// 4MHz
#if defined(_PC8001SR)
			if(config.boot_mode == MODE_PC80_V1 || config.boot_mode == MODE_PC80_N) {
#else
			if(config.boot_mode == MODE_PC88_V1S || config.boot_mode == MODE_PC88_N) {
#endif
				// V1S
				if(!Port40_GHSM && !crtc.vblank) {
					// V1S + not GHSM, V-DISP
					if(hireso) {
						wait += 114;
					} else {
						wait += 68;
					}
				} else {
					if(crtc.vblank) {
						wait += 0;
					} else {
						wait += 2;
					}
				}
			} else {
				// V1H or V2
				if(crtc.vblank) {
					wait += 0;
				} else {
					wait += 2;
				}
			}
		}
		else {
			// 8MHz
#if defined(_PC8001SR)
			if(config.boot_mode == MODE_PC80_V1 || config.boot_mode == MODE_PC80_N) {
#else
			if(config.boot_mode == MODE_PC88_V1S || config.boot_mode == MODE_PC88_N) {
#endif
				// V1S
				if(!Port40_GHSM && !crtc.vblank) {
					// V1S + not GHSM, V-DISP
					if(hireso) {
						wait += 141;
					} else {
						wait += 90;
					}
				} else {
					if(crtc.vblank) {
						wait += 3;
					} else {
						wait += 5;
					}
				}
			} else {
				// V1H or V2
				if(crtc.vblank) {
					wait += 3;
				} else {
					wait += 5;
				}
			}
		}
	} else {
		// graphic off
		if(cpu_clock_low) {
			// 4MHz
			if(mem_wait_on) {
				if(read) {
					// memory wait + read
					wait += 1;
				}
			}
		} else {
			// 8MHz -> memory wait do not effect
			wait += 3;
		}
	}
	return wait;
}

void PC88::update_gvram_wait()
{
	gvram_wait_clocks_r = get_gvram_wait(true);
	gvram_wait_clocks_w = get_gvram_wait(false);
}

void PC88::update_gvram_sel()
{
#if defined(_PC8001SR)
	if(Port33_GVAM) {
#else
	if(Port32_GVAM) {
#endif
		if(Port35_GAM) {
			gvram_sel = 8;
		} else {
			gvram_sel = 0;
		}
		gvram_plane = 0; // from M88
	} else {
		gvram_sel = gvram_plane;
	}
	f000_m1_wait_clocks = get_m1_wait(true);
}

#if defined(_PC8001SR)
void PC88::update_n80_write()
{
	if(PortE2_WREN || Port31_MMODE) {
		if(PortE3_ERAMSL < PC88_EXRAM_BANKS) {
			SET_BANK_W(0x0000, 0x7fff, exram + 0x8000 * PortE3_ERAMSL);
		} else {
			SET_BANK_W(0x0000, 0x7fff, exram);
		}
	} else {
		SET_BANK_W(0x0000, 0x7fff, wdmy);
	}
}

void PC88::update_n80_read()
{
	if(PortE2_RDEN || Port31_MMODE) {
		if(PortE3_ERAMSL < PC88_EXRAM_BANKS) {
			SET_BANK_R(0x0000, 0x7fff, exram + 0x8000 * PortE3_ERAMSL);
		} else {
			SET_BANK_R(0x0000, 0x7fff, exram);
		}
	} else if(Port33_N80SR) {
		if(port[0x71] & 1) {
			SET_BANK_R(0x0000, 0x7fff, n80mk2srrom);
		} else {
			SET_BANK_R(0x0000, 0x5fff, n80mk2srrom);
			SET_BANK_R(0x6000, 0x7fff, n80mk2srrom + 0x8000);
		}
	} else {
		if(port[0x31] & 1) {
			SET_BANK_R(0x0000, 0x7fff, n80mk2rom);
		} else {
			SET_BANK_R(0x0000, 0x5fff, n80mk2rom);
			SET_BANK_R(0x6000, 0x7fff, rdmy);
		}
	}
}
#else
void PC88::update_low_memmap()
{
	// read
	if(PortE2_RDEN) {
#ifdef PC88_EXRAM_BANKS
		if(PortE3_ERAMSL < PC88_EXRAM_BANKS) {
			SET_BANK_R(0x0000, 0x7fff, exram + 0x8000 * PortE3_ERAMSL);
		} else {
#endif
//			SET_BANK_R(0x0000, 0x7fff, rdmy);
#ifdef PC88_EXRAM_BANKS
		}
#endif
	} else if(Port31_MMODE) {
		// 64K RAM
		SET_BANK_R(0x0000, 0x7fff, ram);
	} else if(Port31_RMODE) {
		// N-BASIC
		SET_BANK_R(0x0000, 0x7fff, n80rom);
	} else {
		// N-88BASIC
		SET_BANK_R(0x0000, 0x5fff, n88rom);
		if(Port71_EROM & 1) {
			SET_BANK_R(0x6000, 0x7fff, n88rom + 0x6000);
		} else {
			SET_BANK_R(0x6000, 0x7fff, n88exrom + 0x2000 * Port32_EROMSL);
		}
	}
	
	// write
	if(PortE2_WREN) {
#ifdef PC88_EXRAM_BANKS
		if(PortE3_ERAMSL < PC88_EXRAM_BANKS) {
			SET_BANK_W(0x0000, 0x7fff, exram + 0x8000 * PortE3_ERAMSL);
		} else {
#endif
//			SET_BANK_W(0x0000, 0x7fff, wdmy);
			SET_BANK_W(0x0000, 0x7fff, ram);
#ifdef PC88_EXRAM_BANKS
		}
#endif
	} else {
		SET_BANK_W(0x0000, 0x7fff, ram);
	}
}

void PC88::update_tvram_memmap()
{
	// XM8 version 1.10
	if(config.boot_mode == MODE_PC88_V1S || config.boot_mode == MODE_PC88_N || Port32_TMODE) {
		SET_BANK(0xf000, 0xffff, ram + 0xf000, ram + 0xf000);
	} else {
		SET_BANK(0xf000, 0xffff, tvram, tvram);
	}
}
#endif

void PC88::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_PC88_USART_IRQ) {
		request_intr(IRQ_USART, ((data & mask) != 0));
	} else if(id == SIG_PC88_SOUND_IRQ) {
		intr_req_sound = ((data & mask) != 0);
#if defined(_PC8001SR)
		if(intr_req_sound && !Port33_SINTM) {
#else
		if(intr_req_sound && !Port32_SINTM) {
#endif
			request_intr(IRQ_SOUND, true);
		}
#ifdef SUPPORT_PC88_SB2
	} else if(id == SIG_PC88_SB2_IRQ) {
		intr_req_sb2 = ((data & mask) != 0);
		if(intr_req_sb2 && !PortAA_S2INTM) {
			request_intr(IRQ_SOUND, true);
		}
#endif
	} else if(id == SIG_PC88_USART_OUT) {
		// recv from sio
		if(Port30_CMT) {
			// send to cmt
			if(cmt_rec && Port30_MTON) {
				cmt_buffer[cmt_bufptr++] = data & mask;
				if(cmt_bufptr >= CMT_BUFFER_SIZE) {
					cmt_fio->Fwrite(cmt_buffer, cmt_bufptr, 1);
					cmt_bufptr = 0;
				}
			}
		} else {
			// send to rs-232c
		}
	}
}

void PC88::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_TIMER:
		request_intr(IRQ_TIMER, true);
		break;
	case EVENT_BUSREQ:
		d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
		break;
	case EVENT_CMT_SEND:
		// check data carrier
		if(cmt_play && cmt_bufptr < cmt_bufcnt && Port30_MTON) {
			// detect the data carrier at the top of next block
			if(check_data_carrier()) {
				register_event(this, EVENT_CMT_DCD, 1000000, false, &cmt_register_id);
				usart_dcd = true;
				break;
			}
		}
	case EVENT_CMT_DCD:
		// send data to sio
		usart_dcd = false;
		if(cmt_play && cmt_bufptr < cmt_bufcnt && Port30_MTON) {
			d_sio->write_signal(SIG_I8251_RECV, cmt_buffer[cmt_bufptr++], 0xff);
			if(cmt_bufptr < cmt_bufcnt) {
				register_event(this, EVENT_CMT_SEND, 5000, false, &cmt_register_id);
				break;
			}
		}
		usart_dcd = true; // Jackie Chan no Spartan X
		cmt_register_id = -1;
		break;
	case EVENT_BEEP:
		beep_signal = !beep_signal;
		d_pcm->write_signal(SIG_PCM1BIT_SIGNAL, ((beep_on && beep_signal) || sing_signal) ? 1 : 0, 1);
		break;
	}
}

void PC88::event_frame()
{
	// update key status
	memcpy(key_status, emu->get_key_buffer(), sizeof(key_status));
	
	for(int i = 0; i < 9; i++) {
		// INS or F6-F10 -> SHIFT + DEL or F1-F5
		if(key_status[key_conv_table[i][0]]) {
			key_status[key_conv_table[i][1]] = 1;
			key_status[0x10] |= key_conv_table[i][2];
		}
	}
	if(key_status[0x11] && (key_status[0xbc] || key_status[0xbe])) {
		// CTRL + "," or "." -> NumPad "," or "."
		key_status[0x6c] = key_status[0xbc];
		key_status[0x6e] = key_status[0xbe];
		key_status[0x11] = key_status[0xbc] = key_status[0xbe] = 0;
	}
	key_status[0x14] = key_caps;
	key_status[0x15] = key_kana;
	
	crtc.update_blink();
	
#ifdef SUPPORT_PC88_JOYSTICK
	mouse_dx += mouse_status[0];
	mouse_dy += mouse_status[1];
#endif
}

void PC88::event_vline(int v, int clock)
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
			busreq_clocks = (int)((double)(dmac.ch[2].count.sd + 1) * (cpu_clock_low ? 5.95 : 10.58) / (double)disp_line + 0.5);
//			busreq_clocks = (int)((double)(dmac.ch[2].count.sd + 1) * (cpu_clock_low ? 7.0 : 16.0) / (double)disp_line + 0.5);
		}
		crtc.start();
		// for Nobunaga Fuunroku Opening (XM8 version 1.00)
//		request_intr(IRQ_VRTC, false);
		update_gvram_wait();
	}
	if(v < disp_line) {
		if(/*(crtc.status & 0x10) && */dmac.ch[2].running) {
			// bus request
#if defined(_PC8001SR)
			if(config.boot_mode == MODE_PC80_V1 || config.boot_mode == MODE_PC80_N) {
#else
			if(config.boot_mode == MODE_PC88_V1S || config.boot_mode == MODE_PC88_N) {
#endif
				d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				register_event_by_clock(this, EVENT_BUSREQ, busreq_clocks, false, NULL);
			}
			// run dma transfer to crtc
			if((v % crtc.char_height) == 0) {
				dmac.run(2, 80 + crtc.attrib.num * 2);
			}
		}
	} else if(v == disp_line) {
		if(/*(crtc.status & 0x10) && */dmac.ch[2].running) {
			dmac.finish(2);
			// for Romancia (XM8 version 1.00)
//			crtc.expand_buffer(hireso, Port31_400LINE);
		}
		// for Romancia (XM8 version 1.00)
		crtc.expand_buffer(hireso, Port31_400LINE);
		
		crtc.finish();
		request_intr(IRQ_VRTC, true);
		update_gvram_wait();
	}
}

void PC88::key_down(int code, bool repeat)
{
	if(!repeat) {
		if(code == 0x14) {
			key_caps ^= 1;
		} else if(code == 0x15) {
			key_kana ^= 1;
		}
	}
}

void PC88::play_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(cmt_fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		if(check_file_extension(file_path, _T(".n80"))) {
			cmt_fio->Fread(ram + 0x8000, 0x7f40, 1);
			cmt_fio->Fclose();
			d_cpu->set_sp(ram[0xff3e] | (ram[0xff3f] << 8));
			d_cpu->set_pc(0xff3d);
			return;
		}
		
		cmt_fio->Fseek(0, FILEIO_SEEK_END);
		cmt_bufcnt = cmt_fio->Ftell();
		cmt_bufptr = 0;
		cmt_data_carrier_cnt = 0;
		cmt_fio->Fseek(0, FILEIO_SEEK_SET);
		memset(cmt_buffer, 0, sizeof(cmt_buffer));
		cmt_fio->Fread(cmt_buffer, sizeof(cmt_buffer), 1);
		cmt_fio->Fclose();
		
		if(strncmp((char *)cmt_buffer, "PC-8801 Tape Image(T88)", 23) == 0) {
			// this is t88 format
			int ptr = 24, tag = -1, len = 0, prev_bufptr = 0;
			while(!(tag == 0 && len == 0)) {
				tag = cmt_buffer[ptr + 0] | (cmt_buffer[ptr + 1] << 8);
				len = cmt_buffer[ptr + 2] | (cmt_buffer[ptr + 3] << 8);
				ptr += 4;
				
				if(tag == 0x0101) {
					// data tag
					for(int i = 12; i < len; i++) {
						cmt_buffer[cmt_bufptr++] = cmt_buffer[ptr + i];
					}
				} else if(tag == 0x0102 || tag == 0x0103) {
					// data carrier
					if(prev_bufptr != cmt_bufptr) {
						cmt_data_carrier[cmt_data_carrier_cnt++] = prev_bufptr = cmt_bufptr;
					}
				}
				ptr += len;
			}
			cmt_bufcnt = cmt_bufptr;
			cmt_bufptr = 0;
		}
		cmt_play = (cmt_bufcnt != 0);
		
		if(cmt_play && Port30_MTON) {
			// start motor and detect the data carrier at the top of tape
			if(cmt_register_id != -1) {
				cancel_event(this, cmt_register_id);
			}
			register_event(this, EVENT_CMT_SEND, 5000, false, &cmt_register_id);
		}
	}
}

void PC88::rec_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(cmt_fio->Fopen(file_path, FILEIO_READ_WRITE_NEW_BINARY)) {
		my_tcscpy_s(rec_file_path, _MAX_PATH, file_path);
		cmt_bufptr = 0;
		cmt_rec = true;
	}
}

void PC88::close_tape()
{
	// close file
	release_tape();
	
	// clear sio buffer
	d_sio->write_signal(SIG_I8251_CLEAR, 0, 0);
}

void PC88::release_tape()
{
	// close file
	if(cmt_fio->IsOpened()) {
		if(cmt_rec && cmt_bufptr) {
			cmt_fio->Fwrite(cmt_buffer, cmt_bufptr, 1);
		}
		cmt_fio->Fclose();
	}
	cmt_play = cmt_rec = false;
}

bool PC88::is_frame_skippable()
{
	return (cmt_play && cmt_bufptr < cmt_bufcnt && Port30_MTON);
}

bool PC88::check_data_carrier()
{
	if(cmt_bufptr == 0) {
		return true;
	} else if(cmt_data_carrier_cnt) {
		for(int i = 0; i < cmt_data_carrier_cnt; i++) {
			if(cmt_data_carrier[i] == cmt_bufptr) {
				return true;
			}
		}
	} else if(cmt_buffer[cmt_bufptr] == 0xd3) {
		for(int i = 1; i < 10; i++) {
			if(cmt_buffer[cmt_bufptr + i] != cmt_buffer[cmt_bufptr]) {
				return false;
			}
		}
		return true;
	} else if(cmt_buffer[cmt_bufptr] == 0x9c) {
		for(int i = 1; i < 6; i++) {
			if(cmt_buffer[cmt_bufptr + i] != cmt_buffer[cmt_bufptr]) {
				return false;
			}
		}
		return true;
	}
	return false;
}

void PC88::draw_screen()
{
	// render text screen
	draw_text();
	
	// render graph screen
	bool disp_color_graph = true;
#if defined(_PC8001SR)
	if(config.boot_mode != MODE_PC80_V2) {
		if(Port31_V1_320x200) {
			disp_color_graph = draw_320x200_4color_graph();
		} else if(Port31_V1_MONO) {
			draw_640x200_mono_graph();
		} else {
			draw_640x200_attrib_graph();
		}
		emu->set_vm_screen_lines(200);
	} else {
		if(Port31_HCOLOR) {
			if(Port31_320x200) {
				disp_color_graph = draw_320x200_color_graph();
			} else {
				disp_color_graph = draw_640x200_color_graph();
			}
			emu->set_vm_screen_lines(200);
		} else {
			if(Port31_320x200) {
				draw_320x200_attrib_graph();
			} else {
				draw_640x200_attrib_graph();
			}
			emu->set_vm_screen_lines(200);
		}
	}
#else
	if(Port31_HCOLOR) {
		disp_color_graph = draw_640x200_color_graph();
		emu->set_vm_screen_lines(200);
	} else if(!Port31_400LINE) {
		draw_640x200_attrib_graph();
//		draw_640x200_mono_graph();
		emu->set_vm_screen_lines(200);
	} else {
		draw_640x400_attrib_graph();
		emu->set_vm_screen_lines(400);
//		draw_640x400_mono_graph();
	}
#endif
	
	// update palette
	if(update_palette) {
		static const int pex[8] = {
			0,  36,  73, 109, 146, 182, 219, 255 // from m88
		};
#if defined(_PC8001SR)
		if(config.boot_mode != MODE_PC80_V2) {
			if(Port31_V1_320x200) {
				for(int i = 0; i < 3; i++) {
					uint8_t b = (port[0x31] & 4) ? 7 : 0;
					uint8_t r = (i          & 1) ? 7 : 0;
					uint8_t g = (i          & 2) ? 7 : 0;
					palette_graph_pc[i] = RGB_COLOR(pex[r], pex[g], pex[b]);
				}
				palette_graph_pc[3] = RGB_COLOR(pex[palette[8].r], pex[palette[8].g], pex[palette[8].b]);
			} else if(Port31_V1_MONO) {
				palette_graph_pc[0] = 0;
				palette_graph_pc[1] = RGB_COLOR(pex[palette[8].r], pex[palette[8].g], pex[palette[8].b]);
			} else {
				for(int i = 1; i < 8; i++) {
					palette_graph_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
				}
				palette_graph_pc[0] = RGB_COLOR(pex[palette[8].r], pex[palette[8].g], pex[palette[8].b]);
			}
			if(Port31_V1_320x200) {
				palette_text_pc[0] = 0;
			} else {
				palette_text_pc[0] = palette_graph_pc[0];
			}
		} else {
			for(int i = 0; i < 8; i++) {
				uint8_t b = (port[0x54 + i] & 1) ? 7 : 0;
				uint8_t r = (port[0x54 + i] & 2) ? 7 : 0;
				uint8_t g = (port[0x54 + i] & 4) ? 7 : 0;
				palette_graph_pc[i] = RGB_COLOR(pex[r], pex[g], pex[b]);
			}
			if(!Port31_HCOLOR) {
				palette_graph_pc[0] = RGB_COLOR(pex[palette[8].r], pex[palette[8].g], pex[palette[8].b]);
			}
			palette_text_pc[0] = palette_graph_pc[0];
		}
#else
		for(int i = 0; i < 8; i++) {
			palette_graph_pc[i] = RGB_COLOR(pex[palette[i].r], pex[palette[i].g], pex[palette[i].b]);
		}
		if(!Port31_HCOLOR && !Port32_PMODE) {
			palette_graph_pc[0] = RGB_COLOR(pex[palette[8].r], pex[palette[8].g], pex[palette[8].b]);
		}
		palette_text_pc[0] = palette_graph_pc[0];
#endif
		update_palette = false;
	}
	
	// set back color to black if cg screen is off in color mode
	scrntype_t palette_text_back = palette_text_pc[0];
	scrntype_t palette_graph_back = palette_graph_pc[0];
	
	if(!disp_color_graph) {
		palette_text_pc[0] = palette_graph_pc[0] = 0;
	}
	palette_graph_pc[8] = /*palette_text_pc[8] = */palette_text_pc[0];
	
	// copy to screen buffer
#if !defined(_PC8001SR)
	if(!Port31_400LINE) {
#endif
		for(int y = 0; y < 200; y++) {
			scrntype_t* dest0 = emu->get_screen_buffer(y * 2);
			scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1);
			uint8_t* src_t = text[y];
			uint8_t* src_g = graph[y];
			
#if defined(_PC8001SR)
			if(port[0x33] & 8) {
				for(int x = 0; x < 640; x++) {
					uint32_t g = src_g[x];
					dest0[x] = g ? palette_graph_pc[g] : palette_text_pc[src_t[x]];
				}
			} else {
				for(int x = 0; x < 640; x++) {
					uint32_t t = src_t[x];
					dest0[x] = t ? palette_text_pc[t] : palette_graph_pc[src_g[x]];
				}
			}
#else
			if(Port31_HCOLOR) {
				for(int x = 0; x < 640; x++) {
					uint32_t t = src_t[x];
					dest0[x] = t ? palette_text_pc[t] : palette_graph_pc[src_g[x]];
				}
			} else if(Port32_PMODE) {
				for(int x = 0; x < 640; x++) {
					uint32_t t = src_t[x];
					dest0[x] = palette_graph_pc[t ? t : src_g[x]];
				}
			} else {
				for(int x = 0; x < 640; x++) {
					uint32_t t = src_t[x];
					dest0[x] = palette_text_pc[t ? t : src_g[x]];
				}
			}
#endif
			if(config.scan_line) {
				memset(dest1, 0, 640 * sizeof(scrntype_t));
			} else {
				for(int x = 0; x < 640; x++) {
					dest1[x] = dest0[x];
				}
			}
		}
		emu->screen_skip_line(true);
#if !defined(_PC8001SR)
	} else {
		for(int y = 0; y < 400; y++) {
			scrntype_t* dest = emu->get_screen_buffer(y);
			uint8_t* src_t = text[y >> 1];
			uint8_t* src_g = graph[y];
			
//			if(Port31_HCOLOR) {
//				for(int x = 0; x < 640; x++) {
//					uint32_t t = src_t[x];
//					dest[x] = t ? palette_text_pc[t] : palette_graph_pc[src_g[x]];
//				}
//			} else
			if(Port32_PMODE) {
				for(int x = 0; x < 640; x++) {
					uint32_t t = src_t[x];
					dest[x] = palette_graph_pc[t ? t : src_g[x]];
				}
			} else {
				for(int x = 0; x < 640; x++) {
					uint32_t t = src_t[x];
					dest[x] = palette_text_pc[t ? t : src_g[x]];
				}
			}
		}
		emu->screen_skip_line(false);
	}
#endif
	
	// restore back color palette
	palette_text_pc[0] = palette_text_back;
	palette_graph_pc[0] = palette_graph_back;
}

/*
	attributes:
	
	bit7: green
	bit6: red
	bit5: blue
	bit4: graph=1/character=0
	bit3: under line
	bit2: upper line
	bit1: secret
	bit0: reverse
*/

void PC88::draw_text()
{
	if(crtc.status & 0x88) {
		// dma underrun
		crtc.status &= ~0x80;
		memset(crtc.text.expand, 0, 200 * 80);
		memset(crtc.attrib.expand, crtc.reverse ? 3 : 2, 200 * 80);
	}
	// for Advanced Fantasian Opening (20line) (XM8 version 1.00)
	if(!(crtc.status & 0x10) || Port53_TEXTDS) {
//	if(!(crtc.status & 0x10) || (crtc.status & 8) || Port53_TEXTDS) {
		memset(crtc.text.expand, 0, 200 * 80);
		for(int y = 0; y < 200; y++) {
			for(int x = 0; x < 80; x++) {
				crtc.attrib.expand[y][x] &= 0xe0;
				crtc.attrib.expand[y][x] |= 0x02;
			}
		}
//		memset(crtc.attrib.expand, 2, 200 * 80);
	}
	
	// for Xak2 opening
	memset(text, 8, sizeof(text));
	memset(text_color, 7, sizeof(text_color));
	memset(text_reverse, 0, sizeof(text_reverse));
	
	int char_height = crtc.char_height;
	uint8_t color_mask = Port30_COLOR ? 0 : 7;
	
	if(!hireso) {
		char_height <<= 1;
	}
//	if(Port31_400LINE || !crtc.skip_line) {
//		char_height >>= 1;
//	}
	if(crtc.skip_line) {
		char_height <<= 1;
	}
//	for(int cy = 0, ytop = 0; cy < 64 && ytop < 400; cy++, ytop += char_height) {
	for(int cy = 0, ytop = 0; cy < crtc.height && ytop < 400; cy++, ytop += char_height) {
		for(int x = 0, cx = 0; cx < crtc.width; x += 8, cx++) {
			if(Port30_40 && (cx & 1)) {
				continue;
			}
			uint8_t attrib = crtc.attrib.expand[cy][cx];
//			uint8_t color = !(Port30_COLOR && (attrib & 8)) ? 7 : (attrib & 0xe0) ? (attrib >> 5) : 8;
			uint8_t color = (attrib & 0xe0) ? ((attrib >> 5) | color_mask) : 8;
			bool under_line = ((attrib & 8) != 0);
			bool upper_line = ((attrib & 4) != 0);
			bool secret = ((attrib & 2) != 0);
			bool reverse = ((attrib & 1) != 0);
			
			uint8_t code = secret ? 0 : crtc.text.expand[cy][cx];
#ifdef SUPPORT_PC88_PCG8100
			uint8_t *pattern = ((attrib & 0x10) ? sg_pattern : pcg_pattern) + code * 8;
#else
			uint8_t *pattern = ((attrib & 0x10) ? sg_pattern : kanji1 + 0x1000) + code * 8;
#endif
			
			for(int l = 0, y = ytop; l < char_height / 2 && y < 400; l++, y += 2) {
				uint8_t pat = (l < 8) ? pattern[l] : 0;
				if((upper_line && l == 0) || (under_line && l >= 7)) {
					pat = 0xff;
				}
				if(reverse) {
					pat ^= 0xff;
				}
				
				uint8_t *dest = &text[y >> 1][x];
				if(Port30_40) {
					dest[ 0] = dest[ 1] = (pat & 0x80) ? color : 0;
					dest[ 2] = dest[ 3] = (pat & 0x40) ? color : 0;
					dest[ 4] = dest[ 5] = (pat & 0x20) ? color : 0;
					dest[ 6] = dest[ 7] = (pat & 0x10) ? color : 0;
					dest[ 8] = dest[ 9] = (pat & 0x08) ? color : 0;
					dest[10] = dest[11] = (pat & 0x04) ? color : 0;
					dest[12] = dest[13] = (pat & 0x02) ? color : 0;
					dest[14] = dest[15] = (pat & 0x01) ? color : 0;
					
					// store text color for monocolor graph screen
					text_color[y >> 1][cx + 0] = 
					text_color[y >> 1][cx + 1] = color;
					text_reverse[y >> 1][cx + 0] = 
					text_reverse[y >> 1][cx + 1] = reverse;
				} else {
					dest[0] = (pat & 0x80) ? color : 0;
					dest[1] = (pat & 0x40) ? color : 0;
					dest[2] = (pat & 0x20) ? color : 0;
					dest[3] = (pat & 0x10) ? color : 0;
					dest[4] = (pat & 0x08) ? color : 0;
					dest[5] = (pat & 0x04) ? color : 0;
					dest[6] = (pat & 0x02) ? color : 0;
					dest[7] = (pat & 0x01) ? color : 0;
					
					// store text color for monocolor graph screen
					text_color[y >> 1][cx] = color;
					text_reverse[y >> 1][cx] = reverse;
				}
			}
		}
	}
}

#if defined(_PC8001SR)
bool PC88::draw_320x200_color_graph()
{
	if(!Port31_GRAPH || (Port53_G0DS && Port53_G1DS)) {
		memset(graph, 0, sizeof(graph));
		return false;
	}
	uint8_t *gvram_b0 = Port53_G0DS ? gvram_null : (gvram + 0x0000);
	uint8_t *gvram_r0 = Port53_G0DS ? gvram_null : (gvram + 0x4000);
	uint8_t *gvram_g0 = Port53_G0DS ? gvram_null : (gvram + 0x8000);
	uint8_t *gvram_b1 = Port53_G1DS ? gvram_null : (gvram + 0x2000);
	uint8_t *gvram_r1 = Port53_G1DS ? gvram_null : (gvram + 0x6000);
	uint8_t *gvram_g1 = Port53_G1DS ? gvram_null : (gvram + 0xa000);
	
	if(port[0x33] & 4) {
		// G1>G0
		uint8_t *tmp;
		tmp = gvram_b0; gvram_b0 = gvram_b1; gvram_b1 = tmp;
		tmp = gvram_r0; gvram_r0 = gvram_r1; gvram_r1 = tmp;
		tmp = gvram_g0; gvram_g0 = gvram_g1; gvram_g1 = tmp;
	}
	
	for(int y = 0, addr = 0; y < 200; y++) {
		for(int x = 0; x < 640; x += 16) {
			uint8_t b0 = gvram_b0[addr];
			uint8_t r0 = gvram_r0[addr];
			uint8_t g0 = gvram_g0[addr];
			uint8_t b1 = gvram_b1[addr];
			uint8_t r1 = gvram_r1[addr];
			uint8_t g1 = gvram_g1[addr];
			addr++;
			uint8_t *dest = &graph[y][x];
			uint8_t brg0, brg1;
			brg0 = ((b0 & 0x80) >> 7) | ((r0 & 0x80) >> 6) | ((g0 & 0x80) >> 5);
			brg1 = ((b1 & 0x80) >> 7) | ((r1 & 0x80) >> 6) | ((g1 & 0x80) >> 5);
			dest[ 0] = dest[ 1] = brg0 ? brg0 : brg1;
			brg0 = ((b0 & 0x40) >> 6) | ((r0 & 0x40) >> 5) | ((g0 & 0x40) >> 4);
			brg1 = ((b1 & 0x40) >> 6) | ((r1 & 0x40) >> 5) | ((g1 & 0x40) >> 4);
			dest[ 2] = dest[ 3] = brg0 ? brg0 : brg1;
			brg0 = ((b0 & 0x20) >> 5) | ((r0 & 0x20) >> 4) | ((g0 & 0x20) >> 3);
			brg1 = ((b1 & 0x20) >> 5) | ((r1 & 0x20) >> 4) | ((g1 & 0x20) >> 3);
			dest[ 4] = dest[ 5] = brg0 ? brg0 : brg1;
			brg0 = ((b0 & 0x10) >> 4) | ((r0 & 0x10) >> 3) | ((g0 & 0x10) >> 2);
			brg1 = ((b1 & 0x10) >> 4) | ((r1 & 0x10) >> 3) | ((g1 & 0x10) >> 2);
			dest[ 6] = dest[ 7] = brg0 ? brg0 : brg1;
			brg0 = ((b0 & 0x08) >> 3) | ((r0 & 0x08) >> 2) | ((g0 & 0x08) >> 1);
			brg1 = ((b1 & 0x08) >> 3) | ((r1 & 0x08) >> 2) | ((g1 & 0x08) >> 1);
			dest[ 8] = dest[ 9] = brg0 ? brg0 : brg1;
			brg0 = ((b0 & 0x04) >> 2) | ((r0 & 0x04) >> 1) | ((g0 & 0x04)     );
			brg1 = ((b1 & 0x04) >> 2) | ((r1 & 0x04) >> 1) | ((g1 & 0x04)     );
			dest[10] = dest[11] = brg0 ? brg0 : brg1;
			brg0 = ((b0 & 0x02) >> 1) | ((r0 & 0x02)     ) | ((g0 & 0x02) << 1);
			brg1 = ((b1 & 0x02) >> 1) | ((r1 & 0x02)     ) | ((g1 & 0x02) << 1);
			dest[12] = dest[13] = brg0 ? brg0 : brg1;
			brg0 = ((b0 & 0x01)     ) | ((r0 & 0x01) << 1) | ((g0 & 0x01) << 2);
			brg1 = ((b1 & 0x01)     ) | ((r1 & 0x01) << 1) | ((g1 & 0x01) << 2);
			dest[14] = dest[15] = brg0 ? brg0 : brg1;
		}
	}
	return true;
}

bool PC88::draw_320x200_4color_graph()
{
	if(!Port31_GRAPH || (Port53_G0DS && Port53_G1DS && Port53_G2DS)) {
		memset(graph, 0, sizeof(graph));
		return false;
	}
	uint8_t *gvram_b = Port53_G0DS ? gvram_null : (gvram + 0x0000);
	uint8_t *gvram_r = Port53_G1DS ? gvram_null : (gvram + 0x4000);
	uint8_t *gvram_g = Port53_G2DS ? gvram_null : (gvram + 0x8000);
	
	for(int y = 0, addr = 0; y < 200; y++) {
		for(int x = 0; x < 640; x += 8) {
			uint8_t brg = gvram_b[addr] | gvram_r[addr] | gvram_g[addr];
			addr++;
			uint8_t *dest = &graph[y][x];
			dest[0] = dest[1] = (brg >> 6) & 3;
			dest[2] = dest[3] = (brg >> 4) & 3;
			dest[4] = dest[5] = (brg >> 2) & 3;
			dest[6] = dest[7] = (brg     ) & 3;
		}
	}
	return true;
}

void PC88::draw_320x200_attrib_graph()
{
	if(!Port31_GRAPH || (Port53_G0DS && Port53_G1DS && Port53_G2DS && Port53_G3DS && Port53_G4DS && Port53_G5DS)) {
		memset(graph, 0, sizeof(graph));
		return;
	}
	uint8_t *gvram_b0 = Port53_G0DS ? gvram_null : (gvram + 0x0000);
	uint8_t *gvram_r0 = Port53_G1DS ? gvram_null : (gvram + 0x4000);
	uint8_t *gvram_g0 = Port53_G2DS ? gvram_null : (gvram + 0x8000);
	uint8_t *gvram_b1 = Port53_G3DS ? gvram_null : (gvram + 0x2000);
	uint8_t *gvram_r1 = Port53_G4DS ? gvram_null : (gvram + 0x6000);
	uint8_t *gvram_g1 = Port53_G5DS ? gvram_null : (gvram + 0xa000);
	
	if(Port30_40) {
		for(int y = 0, addr = 0; y < 200; y++) {
			for(int x = 0, cx = 0; x < 640; x += 16, cx += 2) {
				uint8_t color = text_color[y][cx];
				uint8_t brg = gvram_b0[addr] | gvram_r0[addr] | gvram_g0[addr] |
				              gvram_b1[addr] | gvram_r1[addr] | gvram_g1[addr];
				if(text_reverse[y][cx]) {
					brg ^= 0xff;
				}
				addr++;
				uint8_t *dest = &graph[y][x];
				dest[ 0] = dest[ 1] = (brg & 0x80) ? color : 0;
				dest[ 2] = dest[ 3] = (brg & 0x40) ? color : 0;
				dest[ 4] = dest[ 5] = (brg & 0x20) ? color : 0;
				dest[ 6] = dest[ 7] = (brg & 0x10) ? color : 0;
				dest[ 8] = dest[ 9] = (brg & 0x08) ? color : 0;
				dest[10] = dest[11] = (brg & 0x04) ? color : 0;
				dest[12] = dest[13] = (brg & 0x02) ? color : 0;
				dest[14] = dest[15] = (brg & 0x01) ? color : 0;
			}
		}
	} else {
		for(int y = 0, addr = 0; y < 200; y++) {
			for(int x = 0, cx = 0; x < 640; x += 16, cx += 2) {
				uint8_t color_l = text_color[y][cx + 0];
				uint8_t color_r = text_color[y][cx + 1];
				uint8_t brg = gvram_b0[addr] | gvram_r0[addr] | gvram_g0[addr] |
				              gvram_b1[addr] | gvram_r1[addr] | gvram_g1[addr];
				if(text_reverse[y][cx + 0]) {
					brg ^= 0xf0;
				}
				if(text_reverse[y][cx + 1]) {
					brg ^= 0x0f;
				}
				addr++;
				uint8_t *dest = &graph[y][x];
				dest[ 0] = dest[ 1] = (brg & 0x80) ? color_l : 0;
				dest[ 2] = dest[ 3] = (brg & 0x40) ? color_l : 0;
				dest[ 4] = dest[ 5] = (brg & 0x20) ? color_l : 0;
				dest[ 6] = dest[ 7] = (brg & 0x10) ? color_l : 0;
				dest[ 8] = dest[ 9] = (brg & 0x08) ? color_r : 0;
				dest[10] = dest[11] = (brg & 0x04) ? color_r : 0;
				dest[12] = dest[13] = (brg & 0x02) ? color_r : 0;
				dest[14] = dest[15] = (brg & 0x01) ? color_r : 0;
			}
		}
	}
}
#endif

bool PC88::draw_640x200_color_graph()
{
#if defined(_PC8001SR)
	if(!Port31_GRAPH || Port53_G0DS) {
#else
	if(!Port31_GRAPH/* || (Port53_G0DS && Port53_G1DS && Port53_G2DS)*/) {
#endif
		memset(graph, 0, sizeof(graph));
		return false;
	}
	uint8_t *gvram_b = /*Port53_G0DS ? gvram_null : */(gvram + 0x0000);
	uint8_t *gvram_r = /*Port53_G1DS ? gvram_null : */(gvram + 0x4000);
	uint8_t *gvram_g = /*Port53_G2DS ? gvram_null : */(gvram + 0x8000);
	
	for(int y = 0, addr = 0; y < 200; y++) {
		for(int x = 0; x < 640; x += 8) {
			uint8_t b = gvram_b[addr];
			uint8_t r = gvram_r[addr];
			uint8_t g = gvram_g[addr];
			addr++;
			uint8_t *dest = &graph[y][x];
			dest[0] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
			dest[1] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
			dest[2] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
			dest[3] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
			dest[4] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
			dest[5] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04)     );
			dest[6] = ((b & 0x02) >> 1) | ((r & 0x02)     ) | ((g & 0x02) << 1);
			dest[7] = ((b & 0x01)     ) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
		}
	}
	return true;
}

void PC88::draw_640x200_mono_graph()
{
	if(!Port31_GRAPH || (Port53_G0DS && Port53_G1DS && Port53_G2DS)) {
		memset(graph, 0, sizeof(graph));
		return;
	}
	uint8_t *gvram_b = Port53_G0DS ? gvram_null : (gvram + 0x0000);
	uint8_t *gvram_r = Port53_G1DS ? gvram_null : (gvram + 0x4000);
	uint8_t *gvram_g = Port53_G2DS ? gvram_null : (gvram + 0x8000);
	
	for(int y = 0, addr = 0; y < 200; y++) {
		for(int x = 0; x < 640; x += 8) {
			uint8_t brg = gvram_b[addr] | gvram_r[addr] | gvram_g[addr];
			addr++;
			uint8_t *dest = &graph[y][x];
			dest[0] = (brg & 0x80) >> 7;
			dest[1] = (brg & 0x40) >> 6;
			dest[2] = (brg & 0x20) >> 5;
			dest[3] = (brg & 0x10) >> 4;
			dest[4] = (brg & 0x08) >> 3;
			dest[5] = (brg & 0x04) >> 2;
			dest[6] = (brg & 0x02) >> 1;
			dest[7] = (brg & 0x01)     ;
		}
	}
}

void PC88::draw_640x200_attrib_graph()
{
	if(!Port31_GRAPH || (Port53_G0DS && Port53_G1DS && Port53_G2DS)) {
		memset(graph, 0, sizeof(graph));
		return;
	}
	uint8_t *gvram_b = Port53_G0DS ? gvram_null : (gvram + 0x0000);
	uint8_t *gvram_r = Port53_G1DS ? gvram_null : (gvram + 0x4000);
	uint8_t *gvram_g = Port53_G2DS ? gvram_null : (gvram + 0x8000);
	
	for(int y = 0, addr = 0; y < 200; y++) {
		for(int x = 0, cx = 0; x < 640; x += 8, cx++) {
			uint8_t color = text_color[y][cx];
			uint8_t brg = gvram_b[addr] | gvram_r[addr] | gvram_g[addr];
			if(text_reverse[y][cx]) {
				brg ^= 0xff;
			}
			addr++;
			uint8_t *dest = &graph[y][x];
			dest[0] = (brg & 0x80) ? color : 0;
			dest[1] = (brg & 0x40) ? color : 0;
			dest[2] = (brg & 0x20) ? color : 0;
			dest[3] = (brg & 0x10) ? color : 0;
			dest[4] = (brg & 0x08) ? color : 0;
			dest[5] = (brg & 0x04) ? color : 0;
			dest[6] = (brg & 0x02) ? color : 0;
			dest[7] = (brg & 0x01) ? color : 0;
		}
	}
}

#if !defined(_PC8001SR)
void PC88::draw_640x400_mono_graph()
{
	if(!Port31_GRAPH || (Port53_G0DS && Port53_G1DS)) {
		memset(graph, 0, sizeof(graph));
		return;
	}
	uint8_t *gvram_b = Port53_G0DS ? gvram_null : (gvram + 0x0000);
	uint8_t *gvram_r = Port53_G1DS ? gvram_null : (gvram + 0x4000);
	
	for(int y = 0, addr = 0; y < 200; y++) {
		for(int x = 0; x < 640; x += 8) {
			uint8_t b = gvram_b[addr];
			addr++;
			uint8_t *dest = &graph[y][x];
			dest[0] = (b & 0x80) >> 7;
			dest[1] = (b & 0x40) >> 6;
			dest[2] = (b & 0x20) >> 5;
			dest[3] = (b & 0x10) >> 4;
			dest[4] = (b & 0x08) >> 3;
			dest[5] = (b & 0x04) >> 2;
			dest[6] = (b & 0x02) >> 1;
			dest[7] = (b & 0x01)     ;
		}
	}
	for(int y = 200, addr = 0; y < 400; y++) {
		for(int x = 0; x < 640; x += 8) {
			uint8_t r = gvram_r[addr];
			addr++;
			uint8_t *dest = &graph[y][x];
			dest[0] = (r & 0x80) >> 7;
			dest[1] = (r & 0x40) >> 6;
			dest[2] = (r & 0x20) >> 5;
			dest[3] = (r & 0x10) >> 4;
			dest[4] = (r & 0x08) >> 3;
			dest[5] = (r & 0x04) >> 2;
			dest[6] = (r & 0x02) >> 1;
			dest[7] = (r & 0x01)     ;
		}
	}
}

void PC88::draw_640x400_attrib_graph()
{
	if(!Port31_GRAPH || (Port53_G0DS && Port53_G1DS)) {
		memset(graph, 0, sizeof(graph));
		return;
	}
	uint8_t *gvram_b = Port53_G0DS ? gvram_null : (gvram + 0x0000);
	uint8_t *gvram_r = Port53_G1DS ? gvram_null : (gvram + 0x4000);
	
	for(int y = 0, addr = 0; y < 200; y++) {
		for(int x = 0, cx = 0; x < 640; x += 8, cx++) {
			uint8_t color = text_color[y >> 1][cx];
			uint8_t b = gvram_b[addr];
			if(text_reverse[y >> 1][cx]) {
				b ^= 0xff;
			}
			addr++;
			uint8_t *dest = &graph[y][x];
			dest[0] = (b & 0x80) ? color : 0;
			dest[1] = (b & 0x40) ? color : 0;
			dest[2] = (b & 0x20) ? color : 0;
			dest[3] = (b & 0x10) ? color : 0;
			dest[4] = (b & 0x08) ? color : 0;
			dest[5] = (b & 0x04) ? color : 0;
			dest[6] = (b & 0x02) ? color : 0;
			dest[7] = (b & 0x01) ? color : 0;
		}
	}
	for(int y = 200, addr = 0; y < 400; y++) {
		for(int x = 0, cx = 0; x < 640; x += 8, cx++) {
			uint8_t color = text_color[y >> 1][cx];
			uint8_t r = gvram_r[addr];
			if(text_reverse[y >> 1][cx]) {
				r ^= 0xff;
			}
			addr++;
			uint8_t *dest = &graph[y][x];
			dest[0] = (r & 0x80) ? color : 0;
			dest[1] = (r & 0x40) ? color : 0;
			dest[2] = (r & 0x20) ? color : 0;
			dest[3] = (r & 0x10) ? color : 0;
			dest[4] = (r & 0x08) ? color : 0;
			dest[5] = (r & 0x04) ? color : 0;
			dest[6] = (r & 0x02) ? color : 0;
			dest[7] = (r & 0x01) ? color : 0;
		}
	}
}
#endif

void PC88::request_intr(int level, bool status)
{
	uint8_t bit = 1 << level;
	
	if(status) {
		// for Nobunaga Fuunroku Opening & MID-GARTS Opening (XM8 version 1.00)
//		bit &= intr_mask2;
		if(!(intr_req & bit)) {
			intr_req |= bit;
			update_intr();
		}
	} else {
		if(intr_req & bit) {
			intr_req &= ~bit;
			update_intr();
		}
	}
}

void PC88::update_intr()
{
	d_cpu->set_intr_line(((intr_req & intr_mask1 & intr_mask2) != 0), true, 0);
}

uint32_t PC88::get_intr_ack()
{
	uint8_t ai = intr_req & intr_mask1 & intr_mask2;
	
	for(int i = 0; i < 8; i++, ai >>= 1) {
		if(ai & 1) {
			intr_req &= ~(1 << i);
			intr_mask1 = 0;
			return i * 2;
		}
	}
	return 0;
}

void PC88::notify_intr_ei()
{
	update_intr();
}

/* ----------------------------------------------------------------------------
	CRTC (uPD3301)
---------------------------------------------------------------------------- */

void pc88_crtc_t::reset(bool hireso)
{
	blink.rate = 24;
	cursor.type = cursor.mode = -1;
	cursor.x = cursor.y = -1;
	attrib.data = 0xe0;
	attrib.num = 20;
	width = 80;
	height = 25;
	char_height = hireso ? 16 : 8;
	skip_line = false;
	vretrace = hireso ? 3 : 7;
	timing_changed = false;
	reverse = 0;
	intr_mask = 3;
}

void pc88_crtc_t::write_cmd(uint8_t data)
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

void pc88_crtc_t::write_param(uint8_t data)
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

uint32_t pc88_crtc_t::read_param()
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

uint32_t pc88_crtc_t::read_status()
{
	if(status & 8) {
		return status & ~0x10;
	} else {
		return status;
	}
}

void pc88_crtc_t::start()
{
	memset(buffer, 0, sizeof(buffer));
	buffer_ptr = 0;
	vblank = false;
}

void pc88_crtc_t::finish()
{
	if((status & 0x10) && !(intr_mask & 1)) {
		status |= 2;
	}
	vblank = true;
}

void pc88_crtc_t::write_buffer(uint8_t data)
{
	buffer[(buffer_ptr++) & 0x3fff] = data;
}

uint8_t pc88_crtc_t::read_buffer(int ofs)
{
	if(ofs < buffer_ptr) {
		return buffer[ofs];
	}
	// dma underrun occurs !!!
	status |= 8;
//	status &= ~0x10;
	return 0;
}

void pc88_crtc_t::update_blink()
{
	// from m88
	if(++blink.counter > blink.rate) {
		blink.counter = 0;
	}
	blink.attrib = (blink.counter < blink.rate / 4) ? 2 : 0;
	blink.cursor = (blink.counter <= blink.rate / 4) || (blink.rate / 2 <= blink.counter && blink.counter <= 3 * blink.rate / 4);
}

void pc88_crtc_t::expand_buffer(bool hireso, bool line400)
{
	int char_height_tmp = char_height;
	int exitline = -1;
	
	if(!hireso) {
		char_height_tmp <<= 1;
	}
	if(line400 || !skip_line) {
		char_height_tmp >>= 1;
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
			memset(attrib.expand, 0xe0, sizeof(attrib.expand));
		} else {
			
			for(int cy = 0, ytop = 0, ofs = 0; cy < height && ytop < 200; cy++, ytop += char_height_tmp, ofs += 80 + attrib.num * 2) {
				uint8_t flags[128];
				memset(flags, 0, sizeof(flags));
				for(int i = 2 * (attrib.num - 1); i >= 0; i -= 2) {
					flags[read_buffer(ofs + i + 80) & 0x7f] = 1;
				}
				attrib.data &= 0xf3; // for PC-8801mkIIFR •t‘®ƒfƒ‚
				
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
			memset(&attrib.expand[cy][0], 0xe0, width); // color=7
		}
	}
}

void pc88_crtc_t::set_attrib(uint8_t code)
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
		attrib.data = 0xe0 | ((code >> 3) & 0x10) | ((code >> 2) & 0x0d) | ((code << 1) & 2);
		attrib.data ^= reverse;
		attrib.data ^= ((code & 2) && !(code & 1)) ? blink.attrib : 0;
	}
}

/* ----------------------------------------------------------------------------
	DMAC (uPD8257)
---------------------------------------------------------------------------- */

void pc88_dmac_t::write_io8(uint32_t addr, uint32_t data)
{
	int c = (addr >> 1) & 3;
	
	switch(addr & 0x0f) {
	case 0:
	case 2:
	case 4:
	case 6:
		if(!high_low) {
			if((mode & 0x80) && c == 2) {
				ch[3].addr.b.l = data;
			}
			ch[c].addr.b.l = data;
		} else {
			if((mode & 0x80) && c == 2) {
				ch[3].addr.b.h = data;
			}
			ch[c].addr.b.h = data;
		}
		high_low = !high_low;
		break;
	case 1:
	case 3:
	case 5:
	case 7:
		if(!high_low) {
			if((mode & 0x80) && c == 2) {
				ch[3].count.b.l = data;
			}
			ch[c].count.b.l = data;
		} else {
			if((mode & 0x80) && c == 2) {
				ch[3].count.b.h = data & 0x3f;
				ch[3].mode = data & 0xc0;
			}
			ch[c].count.b.h = data & 0x3f;
			ch[c].mode = data & 0xc0;
		}
		high_low = !high_low;
		break;
	case 8:
		mode = data;
		high_low = false;
		break;
	}
}

uint32_t pc88_dmac_t::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	int c = (addr >> 1) & 3;
	
	switch(addr & 0x0f) {
	case 0:
	case 2:
	case 4:
	case 6:
		if(!high_low) {
			val = ch[c].addr.b.l;
		} else {
			val = ch[c].addr.b.h;
		}
		high_low = !high_low;
		break;
	case 1:
	case 3:
	case 5:
	case 7:
		if(!high_low) {
			val = ch[c].count.b.l;
		} else {
			val = (ch[c].count.b.h & 0x3f) | ch[c].mode;
		}
		high_low = !high_low;
		break;
	case 8:
		val = status;
		status &= 0xf0;
//		high_low = false;
		break;
	}
	return val;
}

void pc88_dmac_t::start(int c)
{
	if(mode & (1 << c)) {
		status &= ~(1 << c);
		ch[c].running = true;
	} else {
		ch[c].running = false;
	}
}

void pc88_dmac_t::run(int c, int nbytes)
{
	if(ch[c].running) {
		while(nbytes > 0 && ch[c].count.sd >= 0) {
//			if(ch[c].mode == 0x80) {
				ch[c].io->write_dma_io8(0, mem->read_dma_data8(ch[c].addr.w.l));
//			} else if(ch[c].mode == 0x40) {
//				mem->write_dma_data8(ch[c].addr.w.l, ch[c].io->read_dma_io8(0));
//			}
			ch[c].addr.sd++;
			ch[c].count.sd--;
			nbytes--;
		}
	}
}

void pc88_dmac_t::finish(int c)
{
	if(ch[c].running) {
		while(ch[c].count.sd >= 0) {
//			if(ch[c].mode == 0x80) {
				ch[c].io->write_dma_io8(0, mem->read_dma_data8(ch[c].addr.w.l));
//			} else if(ch[c].mode == 0x40) {
//				mem->write_dma_data8(ch[c].addr.w.l, ch[c].io->read_dma_io8(0));
//			}
			ch[c].addr.sd++;
			ch[c].count.sd--;
		}
		if((mode & 0x80) && c == 2) {
			ch[2].addr.sd = ch[3].addr.sd;
			ch[2].count.sd = ch[3].count.sd;
			ch[2].mode = ch[3].mode;
		} else if(mode & 0x40) {
			mode &= ~(1 << c);
		}
		status |= (1 << c);
		ch[c].running = false;
	}
}

#define STATE_VERSION	8

bool PC88::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateBuffer(ram, sizeof(ram), 1);
#if defined(PC88_EXRAM_BANKS)
	state_fio->StateBuffer(exram, sizeof(exram), 1);
#endif
	state_fio->StateBuffer(gvram, sizeof(gvram), 1);
	state_fio->StateBuffer(tvram, sizeof(tvram), 1);
	state_fio->StateBuffer(port, sizeof(port), 1);
	//state_fio->StateBuffer(&crtc, sizeof(crtc), 1);
	{
		{
			state_fio->StateInt32(crtc.blink.rate);
			state_fio->StateInt32(crtc.blink.counter);
			state_fio->StateUint8(crtc.blink.cursor);
			state_fio->StateUint8(crtc.blink.attrib);
		}
		{
			state_fio->StateInt32(crtc.cursor.type);
			state_fio->StateInt32(crtc.cursor.mode);
			state_fio->StateInt32(crtc.cursor.x);
			state_fio->StateInt32(crtc.cursor.y);
		}
		{
			state_fio->StateUint8(crtc.attrib.data);
			state_fio->StateInt32(crtc.attrib.num);
			for(int i = 0; i < 200; i++) {
				for(int j = 0; j < 80; j++) {
					state_fio->StateUint8(crtc.attrib.expand[i][j]);
				}
			}
		}
		{
			for(int i = 0; i < 200; i++) {
				for(int j = 0; j < 80; j++) {
					state_fio->StateUint8(crtc.text.expand[i][j]);
				}
			}
		}
		state_fio->StateInt32(crtc.width);
		state_fio->StateInt32(crtc.height);
		state_fio->StateInt32(crtc.char_height);
		state_fio->StateBool(crtc.skip_line);
		state_fio->StateInt32(crtc.vretrace);
		state_fio->StateBool(crtc.timing_changed);
		state_fio->StateBuffer(crtc.buffer, sizeof(crtc.buffer), 1);
		state_fio->StateInt32(crtc.buffer_ptr);
		state_fio->StateUint8(crtc.cmd);
		state_fio->StateInt32(crtc.cmd_ptr);
		state_fio->StateUint8(crtc.mode);
		state_fio->StateUint8(crtc.reverse);
		state_fio->StateUint8(crtc.intr_mask);
		state_fio->StateUint8(crtc.status);
		state_fio->StateBool(crtc.vblank);
	}
	//state_fio->StateBuffer(&dmac, sizeof(dmac), 1);
	{
		for(int i = 0; i < 4; i++) {
			state_fio->StateUint32(dmac.ch[i].addr.d);
			state_fio->StateUint32(dmac.ch[i].count.d);
			state_fio->StateUint8(dmac.ch[i].mode);
			state_fio->StateInt32(dmac.ch[i].nbytes);
			state_fio->StateBool(dmac.ch[i].running);
		}
		state_fio->StateUint8(dmac.mode);
		state_fio->StateUint8(dmac.status);
		state_fio->StateBool(dmac.high_low);

	}
	state_fio->StateBuffer(alu_reg, sizeof(alu_reg), 1);
	state_fio->StateUint8(gvram_plane);
	state_fio->StateUint8(gvram_sel);
	state_fio->StateBool(cpu_clock_low);
#if defined(SUPPORT_PC88_HIGH_CLOCK)
	state_fio->StateBool(cpu_clock_high_fe2);
#endif
	state_fio->StateBool(mem_wait_on);
	state_fio->StateInt32(m1_wait_clocks);
	state_fio->StateInt32(f000_m1_wait_clocks);
	state_fio->StateInt32(mem_wait_clocks_r);
	state_fio->StateInt32(mem_wait_clocks_w);
	state_fio->StateInt32(tvram_wait_clocks_r);
	state_fio->StateInt32(tvram_wait_clocks_w);
	state_fio->StateInt32(gvram_wait_clocks_r);
	state_fio->StateInt32(gvram_wait_clocks_w);
	state_fio->StateInt32(busreq_clocks);
	//state_fio->StateBuffer(palette, sizeof(palette), 1);
	for(int i = 0; i < 9; i++) {
		state_fio->StateUint8(palette[i].b);
		state_fio->StateUint8(palette[i].r);
		state_fio->StateUint8(palette[i].g);
	}
	state_fio->StateBool(update_palette);
	state_fio->StateBool(hireso);
	state_fio->StateBuffer(text, sizeof(text), 1);
	state_fio->StateBuffer(graph, sizeof(graph), 1);

	//state_fio->StateBuffer(palette_text_pc, sizeof(palette_text_pc), 1);
	//state_fio->StateBuffer(palette_graph_pc, sizeof(palette_graph_pc), 1);
	if(loading) {
		for(int i = 0; i < (sizeof(palette_text_pc) / sizeof(scrntype_t)); i++) {
			uint8_t r, g, b;
			r = state_fio->FgetUint8();
			g = state_fio->FgetUint8();
			b = state_fio->FgetUint8();
			palette_text_pc[i] = RGB_COLOR(r, g, b);
		}
	} else {
		for(int i = 0; i < (sizeof(palette_text_pc) / sizeof(scrntype_t)); i++) {
			uint8_t r, g, b;
			r = R_OF_COLOR(palette_text_pc[i]);
			g = G_OF_COLOR(palette_text_pc[i]);
			b = B_OF_COLOR(palette_text_pc[i]);
			state_fio->FputUint8(r);
			state_fio->FputUint8(g);
			state_fio->FputUint8(b);
		}
	}
	if(loading) {
		for(int i = 0; i < (sizeof(palette_graph_pc) / sizeof(scrntype_t)); i++) {
			uint8_t r, g, b;
			r = state_fio->FgetUint8();
			g = state_fio->FgetUint8();
			b = state_fio->FgetUint8();
			palette_graph_pc[i] = RGB_COLOR(r, g, b);
		}
	} else {
		for(int i = 0; i < (sizeof(palette_graph_pc) / sizeof(scrntype_t)); i++) {
			uint8_t r, g, b;
			r = R_OF_COLOR(palette_graph_pc[i]);
			g = G_OF_COLOR(palette_graph_pc[i]);
			b = B_OF_COLOR(palette_graph_pc[i]);
			state_fio->FputUint8(r);
			state_fio->FputUint8(g);
			state_fio->FputUint8(b);
		}
	}
	state_fio->StateBool(usart_dcd);
	state_fio->StateBool(opn_busy);
	state_fio->StateUint8(key_caps);
	state_fio->StateUint8(key_kana);
#ifdef SUPPORT_PC88_JOYSTICK
	state_fio->StateUint32(mouse_strobe_clock);
	state_fio->StateUint32(mouse_strobe_clock_lim);
	state_fio->StateInt32(mouse_phase);
	state_fio->StateInt32(mouse_dx);
	state_fio->StateInt32(mouse_dy);
	state_fio->StateInt32(mouse_lx);
	state_fio->StateInt32(mouse_ly);
#endif
	state_fio->StateUint8(intr_req);
	state_fio->StateBool(intr_req_sound);
#ifdef SUPPORT_PC88_SB2
	state_fio->StateBool(intr_req_sb2);
#endif
	state_fio->StateUint8(intr_mask1);
	state_fio->StateUint8(intr_mask2);
	state_fio->StateBool(cmt_play);
	state_fio->StateBool(cmt_rec);
	state_fio->StateBuffer(rec_file_path, sizeof(rec_file_path), 1);
	if(loading) {
		int length_tmp = state_fio->FgetInt32_LE();
		if(cmt_rec) {
			cmt_fio->Fopen(rec_file_path, FILEIO_READ_WRITE_NEW_BINARY);
			while(length_tmp != 0) {
				uint8_t buffer[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer));
				state_fio->Fread(buffer, length_rw, 1);
				if(cmt_fio->IsOpened()) {
					cmt_fio->Fwrite(buffer, length_rw, 1);
				}
				length_tmp -= length_rw;
			}
		}
	} else {
		if(cmt_rec && cmt_fio->IsOpened()) {
			int length_tmp = (int)cmt_fio->Ftell();
			cmt_fio->Fseek(0, FILEIO_SEEK_SET);
			state_fio->FputInt32_LE(length_tmp);
			while(length_tmp != 0) {
				uint8_t buffer[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer));
				cmt_fio->Fread(buffer, length_rw, 1);
				state_fio->Fwrite(buffer, length_rw, 1);
				length_tmp -= length_rw;
 			}
		} else {
			state_fio->FputInt32_LE(0);
 		}
 	}
	state_fio->StateInt32(cmt_bufptr);
	state_fio->StateInt32(cmt_bufcnt);
	state_fio->StateBuffer(cmt_buffer, sizeof(cmt_buffer), 1);
	//state_fio->StateBuffer(cmt_data_carrier, sizeof(cmt_data_carrier), 1);
	for(int i = 0; i < (sizeof(cmt_data_carrier) / sizeof(int)); i++) {
		state_fio->StateInt32(cmt_data_carrier[i]);
	}
	state_fio->StateInt32(cmt_data_carrier_cnt);
	state_fio->StateInt32(cmt_register_id);
	state_fio->StateBool(beep_on);
	state_fio->StateBool(beep_signal);
	state_fio->StateBool(sing_signal);
#ifdef SUPPORT_PC88_PCG8100
	state_fio->StateUint16(pcg_addr);
	state_fio->StateUint8(pcg_data);
	state_fio->StateUint8(pcg_ctrl);
	state_fio->StateBuffer(pcg_pattern, sizeof(pcg_pattern), 1);
#endif
#ifdef NIPPY_PATCH
	state_fio->StateBool(nippy_patch);
#endif
 	
 	// post process
	if(loading) {
		dmac.mem = dmac.ch[2].io = this;
		dmac.ch[0].io = dmac.ch[1].io = dmac.ch[3].io = vm->dummy;
#if defined(_PC8001SR)
		update_n80_write();
		update_n80_read();
#else
		update_low_memmap();
		update_tvram_memmap();
#endif
	}
 	return true;
}

