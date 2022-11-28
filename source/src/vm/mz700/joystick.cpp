/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2006.11.24 -

	[ joystick ]
*/

#include "joystick.h"

void JOYSTICK::initialize()
{
	val_1x03 = 0x7e;
	joy_stat = emu->get_joy_buffer();

	// register event
	register_vline_event(this);
}

//		__           _______________________
//	JA2	  \_________/__y1___/__y0___/       \__	JA2は上下方向の状態をPWMする
//		__ _________ _______ _______ _______ __
//	JA1	__X____A____X___B___X___R___X___L___X__	JA1は左右とボタンの状態をそのまま送る
//
//                |←  38 →|← 30→|← 30→|← 30→|	(3.579545MHz)
//
//	y1y0	UD
//	 0 0	01
//	 0 1	10
//	 1 1	11
//
uint32_t JOYSTICK::read_AM7J(int jnum)
{
	uint32_t js;
	uint32_t val = ~0;
	uint32_t clk;
	bool y0, y1;

	clk = get_current_clock() & 0x007f;
	js = joy_stat[jnum];
	y0 = y1 = false;

	switch (js & 0xc0) {
		case 0x40: // RUN
			y1 = true;
			y0 = true;
			js |= 0x0c;  // LEFT+RIGHT
			break;
		case 0x80: // SELECT
			y0 = true;
			js |= 0x0c;  // LEFT+RIGHT
			break;
		case 0xc0: // RUN+SELECT
			js |= 0x0c;  // LEFT+RIGHT
			break;
		default:
			switch (js & 0x03) {
				case 0x01: // UP
					break;
				case 0x02: // DOWN
					y0 = true;
					break;
				default: // center
					y1 = true;
					y0 = true;
					break;
			}
			break;
	}

	if (clk < 38) {
		// JA2 = 0,  JA1 = TRG-A
		val &= ~0x10;
		if(js & 0x10) val &= ~0x08;  // trigger A
	} else if (clk < 68) {
		// JA2 = y1,  JA1 = TRG-B
		if(!y1) val &= ~0x10;
		if(js & 0x20) val &= ~0x08;  // trigger B
	} else if (clk < 98) {
		// JA2 = y0,  JA1 = RIGHT
		if(!y0) val &= ~0x10;
		if(js & 0x08) val &= ~0x08;  // right
	} else {
		// JA2 = 1,  JA1 = LEFT
		if(js & 0x04) val &= ~0x08;  // left
	}
	if (jnum == 0) {
		val >>= 2;
	}

	return val;
}

uint32_t JOYSTICK::read_io8(uint32_t addr)
{
	uint32_t val = 0x7e;

	switch (config.joystick_type) {

		case DEVICE_JOYSTICK_1X03:		// SHARP MZ-1X03
			val = val_1x03;
			break;

		case DEVICE_JOYSTICK_JOY700:		// TSUKUMO JOY-700
			if(joy_stat[0] & 0x01) val &= ~0x10;  // up    : JB2
			if(joy_stat[0] & 0x02) val &= ~0x08;  // down  : JB1
			if(joy_stat[0] & 0x04) val &= ~0x02;  // left  : JA1
			if(joy_stat[0] & 0x08) val &= ~0x04;  // right : JA2
			if(joy_stat[0] & 0x10) val &= ~0x1e;  // trigger A : ALL
			if(joy_stat[0] & 0x20) val &= ~0x1e;  // trigger B : ALL
			break;

		case DEVICE_JOYSTICK_AM7J:		// AM7J ATARI Joystick adaptor
			val &= read_AM7J(0);
			val &= read_AM7J(1);
			break;

		default:
			break;
        }

	return val;
}

//
// MZ-1X03 は /VBLK=H でボタンの状態(押されていればL)
//            /VBLK=L でスティックの傾き(PWM, 127.841kHz, (0～255)+10のLowパルス) を出力する
//            /VBLKが立ち下がってから 302 CPU clock待った後、28 CPU clock 単位のLowパルスを出力
//
uint64_t JOYSTICK::pulse_width_1x03(uint32_t js, uint32_t mmin, uint32_t mmax)
{
	if (js & mmin) return 192;
	if (js & mmax) return 10000;
	return 4192;
}

void JOYSTICK::event_vline(int v, int clock)
{
	if (config.joystick_type == DEVICE_JOYSTICK_1X03) {
		if (v == 0) {
			// trigger
			val_1x03 = 0x7e;
			if(joy_stat[0] & 0x10) val_1x03 &= ~0x02;
			if(joy_stat[0] & 0x20) val_1x03 &= ~0x04;
			if(joy_stat[1] & 0x10) val_1x03 &= ~0x08;
			if(joy_stat[1] & 0x20) val_1x03 &= ~0x10;
		} else if (v == 200) {
			// stick (PWM)
			val_1x03 &= ~(0x06 | 0x18);
			register_event_by_clock(this, EVENT_1X03_X1, pulse_width_1x03(joy_stat[0], 0x04, 0x08), false, NULL);
			register_event_by_clock(this, EVENT_1X03_Y1, pulse_width_1x03(joy_stat[0], 0x01, 0x02), false, NULL);
			register_event_by_clock(this, EVENT_1X03_X2, pulse_width_1x03(joy_stat[1], 0x04, 0x08), false, NULL);
			register_event_by_clock(this, EVENT_1X03_Y2, pulse_width_1x03(joy_stat[1], 0x01, 0x02), false, NULL);
		}
	}
}

void JOYSTICK::event_callback(int event_id, int err)
{
	switch (event_id) {
		case EVENT_1X03_X1:  val_1x03 |= 0x02; break;
		case EVENT_1X03_Y1:  val_1x03 |= 0x04; break;
		case EVENT_1X03_X2:  val_1x03 |= 0x08; break;
		case EVENT_1X03_Y2:  val_1x03 |= 0x10; break;
	}
}

#define STATE_VERSION	2

bool JOYSTICK::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(val_1x03);
	return true;
}

