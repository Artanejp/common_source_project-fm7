/*
	FUJITSU FM16pi Emulator 'eFM16pi'

	Author : Takeda.Toshiya
	Date   : 2010.12.25-

	[ sub system ]
*/

#include "sub.h"
#include "../../fifo.h"
#include "../i8255.h"
#include "../i8259.h"
#include "../mb8877.h"
#include "../msm58321.h"
#include "../pcm1bit.h"

#define EVENT_KEYSCAN	0

static const int key_table[256] = {
	// EXT -> END, GRAPH -> ALT
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,0x0E,0x0F,  -1,  -1,  -1,0x1C,  -1,  -1,
	0x37,0x36,0x3C,0x3E,0x39,0x3D,  -1,  -1,  -1,  -1,  -1,0x01,0x3B,0x3A,  -1,  -1,
	0x34,  -1,  -1,0x44,0x47,0x48,0x49,0x4B,0x4A,  -1,  -1,  -1,  -1,0x45,0x46,  -1,
	0x0B,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,0x1D,0x2D,0x2B,0x1F,0x12,0x20,0x21,0x22,0x17,0x23,0x24,0x25,0x2F,0x2E,0x18,
	0x19,0x10,0x13,0x1E,0x14,0x16,0x2C,0x11,0x2A,0x15,0x29,  -1,  -1,  -1,  -1,  -1,
	0x0B,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,  -1,  -1,  -1,  -1,0x31,  -1,
	0x3F,0x40,0x41,0x42,0x43,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,0x27,0x26,0x30,0x0C,0x31,0x32,
	0x1A,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,0x1B,0x35,0x28,0x0D,  -1,
	  -1,  -1,0x33,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
};

void SUB::initialize()
{
	key_buffer = new FIFO(16);
	
	// key scan (15ms)
	register_event(this, EVENT_KEYSCAN, 15000, true, NULL);
}

void SUB::release()
{
	key_buffer->release();
	delete key_buffer;
}

void SUB::reset()
{
	key_buffer->clear();
	key_buffer->write(0xf0);	// self check result
	key_data = 0xf0;
	key_irq = false;
	fdc_drive = fdc_side = 0;	// 0xff ???
	rtc_data = 0;
}

void SUB::write_io8(uint32 addr, uint32 data)
{
	switch(addr) {
	case 0x40:
		d_rtc->write_signal(SIG_MSM58321_CS, data, 0x10);
		d_rtc->write_signal(SIG_MSM58321_ADDR_WRITE, data, 0x20);	// prev data is written
		d_rtc->write_signal(SIG_MSM58321_DATA, data, 0x0f);
		d_rtc->write_signal(SIG_MSM58321_READ, data, 0x40);
		d_rtc->write_signal(SIG_MSM58321_WRITE, data, 0x80);		// current data is written
		break;
	case 0xa0:
		if(!(data & 8)) {
			d_pic->write_signal(SIG_I8259_IR6, 0, 0);	// printer ack
		}
		if(!(data & 0x10)) {
			d_pic->write_signal(SIG_I8259_IR0, 0, 0);	// interval timer
			d_pio->write_signal(SIG_I8255_PORT_B, 0x20, 0x20);
		}
		if(data & 0x20) {
			// power off
			emu->power_off();
		}
		d_pcm->write_signal(SIG_PCM1BIT_ON, data, 0x40);
		rtc_data |= data & 0x80;
		break;
	case 0xc8:
		d_fdc->write_signal(SIG_MB8877_SIDEREG, data, 1);
		fdc_side = data;
		break;
	case 0xca:
		d_fdc->write_signal(SIG_MB8877_DRIVEREG, data, 3);
		// bit6: drive disable (1=disable)
		d_fdc->write_signal(SIG_MB8877_MOTOR, data, 0x80);
		fdc_drive = data;
		break;
	}
}

uint32 SUB::read_io8(uint32 addr)
{
	switch(addr) {
	case 0x40:
		return rtc_data;
	case 0x60:
		if(key_irq) {
			d_pic->write_signal(SIG_I8259_IR4, 0, 0);
			d_pio->write_signal(SIG_I8255_PORT_C, 8, 8);
			key_irq = false;
		}
		return key_data;
	case 0xc8:
		return fdc_side;
	case 0xca:
		return fdc_drive;
	}
	return 0xff;
}

void SUB::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_SUB_RTC) {
		// bit0-3: rtc data
		// bit4:   rtc busy
		rtc_data = (rtc_data & ~mask) | (data & mask);
	}
}

void SUB::event_callback(int event_id, int err)
{
	if(!key_buffer->empty() && !key_irq) {
		d_pic->write_signal(SIG_I8259_IR4, 1, 1);
		d_pio->write_signal(SIG_I8255_PORT_C, 0, 8);
		key_irq = true;
		key_data = key_buffer->read();
	}
}

void SUB::key_down(int code)
{
	if((code = key_table[code]) != -1) {
		key_buffer->write(code);
	}
}

void SUB::key_up(int code)
{
	if((code = key_table[code]) != -1) {
		key_buffer->write(code | 0x80);
	}
}

void SUB::notify_power_off()
{
	if(rtc_data & 0x80) {
		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
	}
}

void SUB::draw_screen()
{
	// 640x200, msb->lsb
	scrntype cd = RGB_COLOR(48, 56, 16);
	scrntype cb = RGB_COLOR(160, 168, 160);
	
	for(int y = 0, ptr = 0; y < 200; y++) {
		scrntype *dest = emu->screen_buffer(y);
		for(int x = 0; x < 640; x += 8) {
			uint8 pat = vram[ptr++];
			dest[0] = (pat & 0x80) ? cd : cb;
			dest[1] = (pat & 0x40) ? cd : cb;
			dest[2] = (pat & 0x20) ? cd : cb;
			dest[3] = (pat & 0x10) ? cd : cb;
			dest[4] = (pat & 0x08) ? cd : cb;
			dest[5] = (pat & 0x04) ? cd : cb;
			dest[6] = (pat & 0x02) ? cd : cb;
			dest[7] = (pat & 0x01) ? cd : cb;
			dest += 8;
		}
	}
}

