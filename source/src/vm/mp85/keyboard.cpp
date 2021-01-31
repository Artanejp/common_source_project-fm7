/*
	MITEC MP-85 Emulator 'eMP-85'

	Author : Takeda.Toshiya
	Date   : 2021.01.19-

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8279.h"

void KEYBOARD::initialize()
{
	memset(key_stat, 0, sizeof(key_stat));
}

void KEYBOARD::reset()
{
	d_kdc->write_signal(SIG_I8279_RL, 0xff, 0xff);
	d_kdc->write_signal(SIG_I8279_SHIFT, 0, 1);
	d_kdc->write_signal(SIG_I8279_CTRL, 0, 1);
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_KEYBOARD_SEL) {
		uint32_t val = 0xff;
		
		switch(data & mask) {
		case 0:
			if(key_stat[0x30] || key_stat[0x60]) val &= ~0x01;	// 0
			if(key_stat[0x31] || key_stat[0x61]) val &= ~0x02;	// 1
			if(key_stat[0x32] || key_stat[0x62]) val &= ~0x04;	// 2
			if(key_stat[0x33] || key_stat[0x63]) val &= ~0x08;	// 3
			key_stat[0x30] = key_stat[0x60] = 0;
			key_stat[0x31] = key_stat[0x61] = 0;
			key_stat[0x32] = key_stat[0x62] = 0;
			key_stat[0x33] = key_stat[0x63] = 0;
			break;
		case 1:
			if(key_stat[0x34] || key_stat[0x64]) val &= ~0x01;	// 4
			if(key_stat[0x35] || key_stat[0x65]) val &= ~0x02;	// 5
			if(key_stat[0x36] || key_stat[0x66]) val &= ~0x04;	// 6
			if(key_stat[0x37] || key_stat[0x67]) val &= ~0x08;	// 7
			key_stat[0x34] = key_stat[0x64] = 0;
			key_stat[0x35] = key_stat[0x65] = 0;
			key_stat[0x36] = key_stat[0x66] = 0;
			key_stat[0x37] = key_stat[0x67] = 0;
			break;
		case 2:
			if(key_stat[0x38] || key_stat[0x68]) val &= ~0x01;	// 8
			if(key_stat[0x39] || key_stat[0x69]) val &= ~0x02;	// 9
			if(key_stat[0x41]                  ) val &= ~0x04;	// A
			if(key_stat[0x42]                  ) val &= ~0x08;	// B
			key_stat[0x38] = key_stat[0x68] = 0;
			key_stat[0x39] = key_stat[0x69] = 0;
			key_stat[0x41] = 0;
			key_stat[0x42] = 0;
			break;
		case 3:
			if(key_stat[0x43]                  ) val &= ~0x01;	// C
			if(key_stat[0x44]                  ) val &= ~0x02;	// D
			if(key_stat[0x45]                  ) val &= ~0x04;	// E
			if(key_stat[0x46]                  ) val &= ~0x08;	// F
			key_stat[0x43] = 0;
			key_stat[0x44] = 0;
			key_stat[0x45] = 0;
			key_stat[0x46] = 0;
			break;
		case 4:
			if(key_stat[0x73]                  ) val &= ~0x01;	// ADRS SET	F4
			if(key_stat[0x74] || key_stat[0x21]) val &= ~0x02;	// INCR READ	F5 or PgUp
			if(key_stat[0x75] || key_stat[0x22]) val &= ~0x04;	// DECR READ	F6 or PgDn
			if(key_stat[0x77] || key_stat[0x0d]) val &= ~0x08;	// INCR WRITE	F8 or Enter
			key_stat[0x73] = 0;
			key_stat[0x74] = key_stat[0x21] = 0;
			key_stat[0x75] = key_stat[0x22] = 0;
			key_stat[0x77] = key_stat[0x0d] = 0;
			break;
		case 5:
			if(key_stat[0x70]                  ) val &= ~0x01;	// RUN		F1
			if(key_stat[0x71]                  ) val &= ~0x02;	// STEP		F2
			if(key_stat[0x76]                  ) val &= ~0x08;	// FUNC		F7
			key_stat[0x70] = 0;
			key_stat[0x71] = 0;
			key_stat[0x76] = 0;
			break;
		}
		d_kdc->write_signal(SIG_I8279_RL, val, 0xff);
	}
}
