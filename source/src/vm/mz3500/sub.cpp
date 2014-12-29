/*
	SHARP MZ-3500 Emulator 'EmuZ-3500'

	Author : Takeda.Toshiya
	Date   : 2010.08.31-

	[ sub ]
*/

#include "sub.h"
#include "main.h"
#include "../../fileio.h"

#define EVENT_KEY		0

#define PHASE_KEYBOARD_INIT	-1

#define PHASE_SEND_START_H	1
#define PHASE_SEND_START_L	2
#define PHASE_SEND_BIT9_H	3
#define PHASE_SEND_BIT9_L	4
#define PHASE_SEND_BIT8_H	5
#define PHASE_SEND_BIT8_L	6
#define PHASE_SEND_BIT7_H	7
#define PHASE_SEND_BIT7_L	8
#define PHASE_SEND_BIT6_H	9
#define PHASE_SEND_BIT6_L	10
#define PHASE_SEND_BIT5_H	11
#define PHASE_SEND_BIT5_L	12
#define PHASE_SEND_BIT4_H	13
#define PHASE_SEND_BIT4_L	14
#define PHASE_SEND_BIT3_H	15
#define PHASE_SEND_BIT3_L	16
#define PHASE_SEND_BIT2_H	17
#define PHASE_SEND_BIT2_L	18
#define PHASE_SEND_BIT1_H	19
#define PHASE_SEND_BIT1_L	20
#define PHASE_SEND_BIT0_H	21
#define PHASE_SEND_BIT0_L	22
#define PHASE_SEND_ACK		23

#define PHASE_RECV_BIT3		31
#define PHASE_RECV_BIT2		32
#define PHASE_RECV_BIT1		33
#define PHASE_RECV_BIT0		34
#define PHASE_RECV_ACK_H	35
#define PHASE_RECV_ACK_L	36

#define WAIT_USEC(usec) { \
	register_event(this, EVENT_KEY, usec, false, NULL); \
}

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x800 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x800 * (i - sb); \
		} \
	} \
}

void SUB::initialize()
{
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	// create pc palette
	for(int i = 0; i < 8; i++) {
		palette_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}
	
	// init memory
	memset(ram, 0, sizeof(ram));
	memset(vram_chr, 0, sizeof(vram_chr));
	memset(vram_gfx, 0, sizeof(vram_gfx));
	
	SET_BANK(0x0000, 0x1fff, wdmy, ipl);
	SET_BANK(0x2000, 0x27ff, common, common);
	SET_BANK(0x2800, 0x3fff, wdmy, rdmy);
	SET_BANK(0x4000, 0x5fff, ram, ram);
	SET_BANK(0x6000, 0xffff, wdmy, rdmy);
	
	register_frame_event(this);
}

void SUB::reset()
{
	memset(disp, 0, sizeof(disp));
	blink = 0;
	
	// keyboard
	dk = stk = hlt = ackc = false;
	stc = true;
	key_phase = PHASE_KEYBOARD_INIT;
}

void SUB::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32 SUB::read_data8(uint32 addr)
{
	addr &= 0xffff;
	return rbank[addr >> 11][addr & 0x7ff];
}

void SUB::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xf0) {
	case 0x00:	// mz3500sm p.18,77
//		emu->out_debug_log("SUB->MAIN\tINT0=1\n");
		d_main->write_signal(SIG_MAIN_INT0, 1, 1);
		break;
	case 0x50:	// mz3500sm p.28
		disp[addr & 0x0f] = data;
		break;
	}
}

uint32 SUB::read_io8(uint32 addr)
{
	switch(addr & 0xf0) {
	case 0x00:	// mz3500sm p.18,77
//		emu->out_debug_log("SUB->MAIN\tINT0=1\n");
		d_main->write_signal(SIG_MAIN_INT0, 1, 1);
		break;
	case 0x40:	// mz3500sm p.80
		return (dout ? 1 : 0) | (obf ? 2 : 0) | (dk ? 0x20 : 0) | (stk ? 0x40 : 0) | (hlt ? 0 : 0x80);
	}
	return 0xff;
}

void SUB::event_frame()
{
	blink++;
}

void SUB::event_callback(int event_id, int err)
{
	key_drive();
}

void SUB::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_SUB_RTC_DOUT) {
		dout = ((data & mask) != 0);
	} else if(id == SIG_SUB_PIO_OBF) {
		obf = ((data & mask) != 0);
	} else if(id == SIG_SUB_PIO_PM) {
		pm = ((data & mask) != 0);
	} else if(id == SIG_SUB_KEYBOARD_DC) {
		dc = ((data & mask) != 0);
	} else if(id == SIG_SUB_KEYBOARD_STC) {
		bool next = ((data & mask) != 0);
		if(!stc && next) {
			// L->H
//			emu->out_debug_log("STC L->H\n");
			if(key_phase == PHASE_KEYBOARD_INIT) {
				key_phase = 0;
			} else if(key_phase == 0) {
				key_phase = PHASE_RECV_BIT3;
				key_recv_data = 0;
				key_recv_bit = 8;
			} else if(PHASE_RECV_BIT3 <= key_phase && key_phase <= PHASE_RECV_BIT0) {
				key_drive();
			}
		}
		stc = next;
	} else if(id == SIG_SUB_KEYBOARD_ACKC) {
		bool next = ((data & mask) != 0);
		if(ackc && !next) {
			// H->L
//			emu->out_debug_log("ACKC H->L\n");
			if(key_phase == PHASE_SEND_ACK) {
				key_drive();
			}
		}
		ackc = next;
	}
}

// keyboard

void SUB::key_down(int code)
{
	
}

void SUB::key_up(int code)
{
	
}

void SUB::key_send(int data, bool command)
{
	key_send_data = (data << 2) | (command ? 2 : 0);
	key_send_bit = 0x200;
	int parity = 0;
	for(int i = 1; i < 10; i++) {
		if(key_send_data & (1 << i)) {
			parity++;
		}
	}
	if(!(parity & 1)) {
		key_send_data |= 1;
	}
	key_phase = PHASE_SEND_START_H;
	key_drive();
}

void SUB::key_recv(int data)
{
//	emu->out_debug_log("RECV %2x\n", data);
}

void SUB::key_drive()
{
	switch(key_phase) {
	case PHASE_SEND_START_H:
		stk = true;
		WAIT_USEC(12.5);
		key_phase++;
		break;
	case PHASE_SEND_START_L:
		stk = false;
		WAIT_USEC(32.5);
		key_phase++;
		break;
	case PHASE_SEND_BIT9_H:
	case PHASE_SEND_BIT8_H:
	case PHASE_SEND_BIT7_H:
	case PHASE_SEND_BIT6_H:
	case PHASE_SEND_BIT5_H:
	case PHASE_SEND_BIT4_H:
	case PHASE_SEND_BIT3_H:
	case PHASE_SEND_BIT2_H:
	case PHASE_SEND_BIT1_H:
	case PHASE_SEND_BIT0_H:
		dk = ((key_send_data & key_send_bit) != 0);
		stk = true;
		key_send_bit >>= 1;
		WAIT_USEC(17.5);
		key_phase++;
	case PHASE_SEND_BIT9_L:
	case PHASE_SEND_BIT8_L:
	case PHASE_SEND_BIT7_L:
	case PHASE_SEND_BIT6_L:
	case PHASE_SEND_BIT5_L:
	case PHASE_SEND_BIT4_L:
	case PHASE_SEND_BIT3_L:
	case PHASE_SEND_BIT2_L:
	case PHASE_SEND_BIT1_L:
	case PHASE_SEND_BIT0_L:
		stk = false;
		WAIT_USEC(50.0);
		key_phase++;
		break;
	case PHASE_SEND_ACK:
		key_phase = 0;
		break;
	case PHASE_RECV_BIT3:
	case PHASE_RECV_BIT2:
	case PHASE_RECV_BIT1:
	case PHASE_RECV_BIT0:
		if(dc) {
			key_recv_data |= key_recv_bit;
		}
		key_recv_bit >>= 1;
		if(key_phase == PHASE_RECV_BIT0) {
			WAIT_USEC(60);
		}
		key_phase++;
		break;
	case PHASE_RECV_ACK_H:
//		emu->out_debug_log("DK=STK=H\n");
		dk = stk = true;
		WAIT_USEC(17.5);
		key_phase++;
		break;
	case PHASE_RECV_ACK_L:
//		emu->out_debug_log("DK=STK=L\n");
		dk = stk = false;
		key_phase = 0;
		key_recv(key_recv_data >> 1);
		break;
	}
}

// display

void SUB::draw_screen()
{
	memset(screen_chr, 0, sizeof(screen_chr));
	memset(screen_gfx, 0, sizeof(screen_gfx));
	
	// mz3500sm p.28
	if(disp[0] & 1) {
		draw_chr();
	}
	if(disp[1] & 7) {
		draw_gfx();
	}
	uint8 back = disp[3] & 7;
	
	// copy to pc screen
	for(int y = 0; y <400; y++) {
		scrntype* dest = emu->screen_buffer(y);
		uint8* src_chr = screen_chr[y];
		uint8* src_gfx = screen_gfx[y];
		
		for(int x = 0; x < 640; x++) {
			dest[x] = palette_pc[(src_chr[x] ? src_chr[x] : src_gfx[x] ? src_gfx[x] : back)];
		}
	}
	emu->screen_skip_line = false;
}

void SUB::draw_chr()
{
	// mz3500sm p.28
	int width = 1;//(disp[5] & 2) ? 1 : 2;	// 80/40 columns
	int height = 16;//(disp[7] & 1) ? 20 : 16;	// 20/16 dots/ine
	int ymax = 25;//(disp[7] & 1) ? 20 : 25;	// 20/25 lines
	
	for(int i = 0, ytop = 0; i < 4; i++) {
		uint32 ra = ra_chr[4 * i];
		ra |= ra_chr[4 * i + 1] << 8;
		ra |= ra_chr[4 * i + 2] << 16;
		ra |= ra_chr[4 * i + 3] << 24;
		int src = ra << 1;
		int len = (ra >> 20) & 0x3ff;
		int caddr = ((cs_chr[0] & 0x80) && ((cs_chr[1] & 0x20) || !(blink & 0x10))) ? ((*ead_chr << 1) & 0xffe) : -1;
		
		for(int y = ytop; y < (ytop + len) && y < ymax; y++) {
			for(int x = 0; x < 80; x += width) {
				src &= 0xffe;
				bool cursor = (src == caddr);
				uint8 code = vram_chr[src++];	// low byte  : code
				uint8 attr = vram_chr[src++];	// high byte : attr
				
				// bit0: blink
				// bit1: reverse or green
				// bit2: vertical line or red
				// bit3: horizontal line or blue
				
//				uint8 color = 7;
				uint8 color = ((attr & 1) && (blink & 0x10)) ? 0 : 7;
				uint8* pattern = &font[0x1000 | (code << 4)];
				
				// NOTE: need to consider 200 line mode
				
				for(int l = 0; l < 16; l++) {
					int yy = y * height + l;
					if(yy >= 400) {
						break;
					}
					uint8 pat = (attr & 2) ? ~pattern[l] : pattern[l];
					uint8 *dest = &screen_chr[yy][x << 3];
					
					if(width == 1) {
						// 8dots (80columns)
						dest[0] = (pat & 0x80) ? color : 0;
						dest[1] = (pat & 0x40) ? color : 0;
						dest[2] = (pat & 0x20) ? color : 0;
						dest[3] = (pat & 0x10) ? color : 0;
						dest[4] = (pat & 0x08) ? color : 0;
						dest[5] = (pat & 0x04) ? color : 0;
						dest[6] = (pat & 0x02) ? color : 0;
						dest[7] = (pat & 0x01) ? color : 0;
					} else {
						// 16dots (40columns)
						dest[ 0] = dest[ 1] = (pat & 0x80) ? color : 0;
						dest[ 2] = dest[ 3] = (pat & 0x40) ? color : 0;
						dest[ 4] = dest[ 5] = (pat & 0x20) ? color : 0;
						dest[ 6] = dest[ 7] = (pat & 0x10) ? color : 0;
						dest[ 8] = dest[ 9] = (pat & 0x08) ? color : 0;
						dest[10] = dest[11] = (pat & 0x04) ? color : 0;
						dest[12] = dest[13] = (pat & 0x02) ? color : 0;
						dest[14] = dest[15] = (pat & 0x01) ? color : 0;
					}
				}
				if(cursor) {
					int top = cs_chr[1] & 0x1f, bottom = cs_chr[2] >> 3;
					for(int l = top; l < bottom && l < height; l++) {
						int yy = y * height + l;
						if(yy >= 400) {
							break;
						}
						memset(&screen_chr[yy][x << 3], 7, width * 8);	// always white ???
					}
				}
			}
		}
		ytop += len;
	}
}

void SUB::draw_gfx()
{
}
