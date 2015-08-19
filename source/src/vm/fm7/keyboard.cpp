/*
 * Common Source code Project -> VM -> FM-7/77AV -> Keyboard
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Licence: GPLv2
 * History : 
 *  Feb 12, 2015 : Initial 
 */

#include "../../fifo.h"
#include "../device.h"
#include "fm7_keyboard.h"

#include "keyboard_tables.h"
#if defined(_FM77AV_VARIANTS)
#include "fm77av_hidden_message_keyboard.h"
#endif
enum {
	ID_KEYBOARD_RXRDY_OK = 1,
	ID_KEYBOARD_ACK,
	ID_KEYBOARD_RXRDY_BUSY,
	ID_KEYBOARD_RTC_COUNTUP,
	ID_KEYBOARD_INT,
	ID_KEYBOARD_AUTOREPEAT_FIRST,
	ID_KEYBOARD_AUTOREPEAT,
	ID_KEYBOARD_HIDDENMESSAGE_AV
};

//
/*
 * I/O API (subio)
 */
// 0xd400(SUB) or 0xfd00(MAIN)
uint8 KEYBOARD::get_keycode_high(void)
{
	uint8 data = 0x00;
	if((keycode_7 & 0x0100) != 0) data = 0x80;
	return data;
}

// 0xd401(SUB) or 0xfd01(MAIN)
uint8 KEYBOARD::get_keycode_low(void)
{
	uint8 data = keycode_7 & 0xff;
	this->write_signals(&int_line, 0x00000000);
	return data;
}

// 0xd40d : R
void KEYBOARD::turn_on_ins_led(void)
{
	this->write_signals(&ins_led, 0xff);
}

// 0xd40d : W
void KEYBOARD::turn_off_ins_led(void)
{
	this->write_signals(&ins_led, 0x00);
}

// UI Handler. 
uint16 KEYBOARD::vk2scancode(uint32 vk)
{
	uint16 i;
	i = 0;
	if(vk == VK_PAUSE) vk = VK_KANJI; // Workaround some desktop environments for [ESC].
	do {
		if(vk_matrix_106[i] == vk) return i;
		i++;
	} while(vk_matrix_106[i] != 0xffff);
	return 0x0000;
}

bool KEYBOARD::isModifier(uint16 sc)
{
	if(((sc >= 0x52) && (sc <= 0x56)) || // CTRL LSHIFT RSHIFT CAPS GRPH
     		(sc == 0x5a) || (sc == 0x5c)) { // KANA BREAK
		return true;
	}
	return false;
}

void KEYBOARD::set_modifiers(uint16 sc, bool flag)
{
	if(sc == 0x52) { // CTRL
		ctrl_pressed = flag; 
	} else if(sc == 0x53) { // LSHIFT
		lshift_pressed = flag;
		shift_pressed = lshift_pressed | rshift_pressed;
		//printf("LSHIFT : %d\n", flag ? 1 : 0);
	} else if(sc == 0x54) { // RSHIFT
		rshift_pressed = flag;
		shift_pressed = lshift_pressed | rshift_pressed;
		//printf("RSHIFT : %d\n", flag ? 1 : 0);
	} else if(sc == 0x56) { // GRPH
		graph_pressed = flag;
	} else if(sc == 0x55) { // CAPS
		// Toggle on press.
		if(flag) {
			if(caps_pressed) {
				caps_pressed = false;
			} else {
				caps_pressed = true;
			}
			if(keymode == KEYMODE_STANDARD) this->write_signals(&caps_led, caps_pressed ? 0xff : 0x00);
			//this->write_signals(&caps_led, caps_pressed ? 0xff : 0x00);
		}
	} else if(sc == 0x5a) { // KANA
		// Toggle on press.
		if(flag) {
			if(kana_pressed) {
				kana_pressed = false;
			} else {
				kana_pressed = true;
			}
			if(keymode == KEYMODE_STANDARD) this->write_signals(&kana_led, kana_pressed ? 0xff : 0x00);
		}
	} else if(sc == 0x5c) { // Break
		break_pressed = flag;
	}
}

uint16 KEYBOARD::scan2fmkeycode(uint16 sc)
{
	const struct key_tbl_t *keyptr = NULL;
	bool stdkey = false;
	int i;
	uint16 retval;
	
	if((sc == 0) || (sc >= 0x67)) return 0xffff;
	// Set repeat flag(s)
	if(shift_pressed && ctrl_pressed) {
		switch(sc) {
			case 0x02: // 1
			case 0x42: // 1
				repeat_mode = true;
				return 0xffff;
				break;
			case 0x0b: // 0
			case 0x46: // 0
				repeat_mode = false;
				return 0xffff;
				break;
		}
	}
	if(keymode == KEYMODE_STANDARD) {
		if(ctrl_pressed) {
			if(shift_pressed) {
				keyptr = ctrl_shift_key;
			} else {
				keyptr = ctrl_key;
			}
		} else if(graph_pressed) {
			if(shift_pressed) {
				keyptr = graph_shift_key;
			} else {
				keyptr = graph_key;
			}
		} else if(kana_pressed) {
			if(shift_pressed) {
				keyptr = kana_shift_key;
			} else {
				keyptr = kana_key;
			}
		} else { // Standard
			stdkey = true;
			if(shift_pressed) {
				keyptr = standard_shift_key;
			} else {
				keyptr = standard_key;
			}
		}
	}
#if defined(_FM77AV_VARIANTS)
	else 	if(shift_pressed) {
	  // DO super-impose mode:
	  // F7 : PC
	  // F8 : IMPOSE (High brightness)
	  // F9 : IMPOSE (Low brightness)
	  // F10: TV
	}
	if(keymode == KEYMODE_SCAN) {
		retval = sc;
		return retval;
	} else if(keymode == KEYMODE_16BETA) { // Will Implement
		if(ctrl_pressed) {
			if(shift_pressed) {
				keyptr = ctrl_shift_key_16beta;
			} else {
				keyptr = ctrl_key_16beta;
			}
		} else if(graph_pressed) {
			if(shift_pressed) {
				keyptr = graph_shift_key_16beta;
			} else {
				keyptr = graph_key_16beta;
			}
		} else if(kana_pressed) {
			if(shift_pressed) {
				keyptr = kana_shift_key_16beta;
			} else {
				keyptr = kana_key_16beta;
			}
		} else { // Standard
			stdkey = true;
			if(shift_pressed) {
				keyptr = standard_shift_key_16beta;
			} else {
				keyptr = standard_key_16beta;
			}
		}
	}
#endif //_FM77AV_VARIANTS	
	i = 0;
	retval = 0xffff;
	if (keyptr == NULL) return 0xffff;
	do {
		if(keyptr[i].phy == sc) {
			retval = keyptr[i].code;
			break;
		}
		i++;
	} while(keyptr[i].phy != 0xffff);
	if(keyptr[i].phy == 0xffff) return 0x00;
	if(stdkey) {
		if((retval >= 'A') && (retval <= 'Z')) {
			if(caps_pressed) {
				retval += 0x20;
			}
		} else if((retval >= 'a') && (retval <= 'z')) {
			if(caps_pressed) {
				retval -= 0x20;
			}
		}
	}
	return retval;
}

void KEYBOARD::key_up(uint32 vk)
{
	uint16 bak_scancode = vk2scancode(vk);
	bool stat_break = break_pressed;
	older_vk = 0;
	if(bak_scancode == 0) return;
	if((event_keyrepeat >= 0) && (repeat_keycode == bak_scancode)) { // Not Break
		cancel_event(this, event_keyrepeat);
		event_keyrepeat = -1;
		repeat_keycode = 0;
	}
	//printf("Key: up: %04x\n", bak_scancode);
	if(this->isModifier(bak_scancode)) {
		set_modifiers(bak_scancode, false);
		if(break_pressed != stat_break) { // Break key UP.
			this->write_signals(&break_line, 0x00);
		}
	}
	scancode = 0;
	if((keymode == KEYMODE_SCAN) && (bak_scancode != 0)) { // Notify even key-up, when using SCAN mode.
		uint32 code = (bak_scancode & 0x7f) | 0x80;
		key_fifo->write(code);
	}
}

void KEYBOARD::key_down(uint32 vk)
{
	if(older_vk == vk) return;
	older_vk = vk;
	
	scancode = vk2scancode(vk);
#if defined(_FM77AV_VARIANTS)
	// Below are FM-77AV's hidden message , see :
	// https://twitter.com/maruan/status/499558392092831745
	//if(caps_pressed && kana_pressed) {
	//	if(ctrl_pressed && lshift_pressed && rshift_pressed && graph_pressed) {
	if(caps_pressed && kana_pressed && graph_pressed && shift_pressed && ctrl_pressed) { // IT's deprecated key pressing
		if(scancode == 0x15) { // "T"
			if(event_hidden1_av < 0) {
				hidden1_ptr = 0;
				register_event(this,
						ID_KEYBOARD_HIDDENMESSAGE_AV,
						100.0 * 1000, true, &event_hidden1_av);
			}
			return;
		}
	}
#endif 
	key_down_main();
}

void KEYBOARD::key_down_main(void)
{
	bool stat_break = break_pressed;
	uint32 code;
	if(scancode == 0) return;
	if(this->isModifier(scancode)) {  // modifiers
		set_modifiers(scancode, true);
		if(break_pressed != stat_break) { // Break key Down.
			this->write_signals(&break_line, 0xff);
		}
		//printf("DOWN SCAN=%04x break=%d\n", scancode, break_pressed);
		if(keymode != KEYMODE_SCAN) return;
	}
	if(keymode == KEYMODE_SCAN) {
		code = scancode & 0x7f;
	} else {
		code = scan2fmkeycode(scancode);
	}
	if(code != 0) {
		key_fifo->write(code);
		if((scancode < 0x5c) && (code != 0xffff) && (repeat_keycode == 0)) {
			double usec = (double)repeat_time_long * 1000.0;
			if(event_keyrepeat >= 0) cancel_event(this, event_keyrepeat);
			event_keyrepeat = -1;
			repeat_keycode = (uint8)scancode;
			if(repeat_mode) register_event(this,
						       ID_KEYBOARD_AUTOREPEAT_FIRST,
						       usec, false, &event_keyrepeat);
		}
	}
   
}

#if defined(_FM77AV_VARIANTS)
void KEYBOARD::adjust_rtc(void)
{
	p_emu->get_host_time(&cur_time);
	rtc_yy = cur_time.year % 100;
	rtc_mm = cur_time.month;
	rtc_dd = cur_time.day;

	rtc_dayofweek = cur_time.day_of_week;
	if(rtc_count24h) {
		rtc_ispm = (cur_time.hour >= 12) ? true : false;
		rtc_hour = cur_time.hour % 12;
	} else {
		rtc_ispm = false;
		rtc_hour = cur_time.hour;
	}
	rtc_minute = cur_time.minute;
	rtc_sec = cur_time.second;
	if(event_key_rtc >= 0) {
		cancel_event(this, event_key_rtc);
	}
	register_event(this, ID_KEYBOARD_RTC_COUNTUP, 1000.0 * 1000.0, true, &event_key_rtc);
}
#endif

void KEYBOARD::do_repeatkey(uint16 sc)
{
	uint16 code_7;
	if((sc == 0) || (sc >= 0x67)) return; // scancode overrun.
	if(!repeat_mode) {
		if(event_keyrepeat >= 0) {
			cancel_event(this, event_keyrepeat);
			event_keyrepeat = -1;
		}
		return;
	}
	code_7 = scan2fmkeycode(sc);
	if(keymode == KEYMODE_SCAN) {
		code_7 = sc;
	}
	if(code_7 < 0x200) {
		key_fifo->write((uint32)code_7);
	}
}

void KEYBOARD::event_callback(int event_id, int err)
{
#if defined(_FM77AV_VARIANTS)
	if(event_id == ID_KEYBOARD_RXRDY_OK) {
		rxrdy_status = true;
		write_signals(&rxrdy, 0xff);
	} else if(event_id == ID_KEYBOARD_RXRDY_BUSY) {
		rxrdy_status = false;
		write_signals(&rxrdy, 0x00);
	} else if(event_id == ID_KEYBOARD_ACK) {
		key_ack_status = true;
		write_signals(&key_ack, 0xff);
	} else if(event_id == ID_KEYBOARD_RTC_COUNTUP) {
		rtc_count();
	} else if(event_id == ID_KEYBOARD_HIDDENMESSAGE_AV) {
		if(hidden_message_77av_1[hidden1_ptr] == 0x00) {
			cancel_event(this, event_hidden1_av);
			event_hidden1_av = -1;
			hidden1_ptr = 0;
		} else {
			key_fifo->write(hidden_message_77av_1[hidden1_ptr++]);
		}
	} else
#endif
	if(event_id == ID_KEYBOARD_AUTOREPEAT_FIRST) {
		uint32 sc = (uint32)repeat_keycode;
		double usec = (double)repeat_time_short * 1000.0;

		if((sc >= 0x67) || (sc == 0) && (sc == 0x5c)) return; 
		do_repeatkey((uint16)sc);
		register_event(this,
			       ID_KEYBOARD_AUTOREPEAT,
			       usec, true, &event_keyrepeat);
		// Key repeat.
	} else if(event_id == ID_KEYBOARD_AUTOREPEAT){
		if(repeat_keycode != 0) {
			do_repeatkey((uint16)repeat_keycode);
		} else {
			cancel_event(this, event_keyrepeat);
			event_keyrepeat = -1;
		}
	} else if(event_id == ID_KEYBOARD_INT) {
		if(!(key_fifo->empty())) {
			keycode_7 = key_fifo->read();
			this->write_signals(&int_line, 0xffffffff);
		}
	}
}

// Commands
void KEYBOARD::reset_unchange_mode(void)
{
	int i;
	repeat_time_short = 70; // mS
	repeat_time_long = 700; // mS
	repeat_mode = true;
	older_vk = 0;

	lshift_pressed = false;
	rshift_pressed = false;
	shift_pressed = false;
	ctrl_pressed   = false;
	graph_pressed = false;
	kana_pressed = false;
	caps_pressed = false;
	//	ins_pressed = false;
	datareg = 0x00;

#if defined(_FM77AV_VARIANTS)
	cmd_fifo->clear();
	data_fifo->clear();
	if(event_key_rtc >= 0) {
		cancel_event(this, event_key_rtc);
	}
	register_event(this,ID_KEYBOARD_RTC_COUNTUP, 1000.0 * 1000.0, true, &event_key_rtc);

	cmd_phase = 0;
	if(event_keyrepeat >= 0) cancel_event(this, event_keyrepeat);
	event_keyrepeat = -1;
	repeat_keycode = 0x00;
   
	if(event_hidden1_av >= 0) cancel_event(this, event_hidden1_av);
   	event_hidden1_av = -1;
	hidden1_ptr = 0;
#endif
	// Bus
	this->write_signals(&break_line, 0x00);
#if defined(_FM77AV_VARIANTS)
	rxrdy_status = false;
	key_ack_status = true;
	this->write_signals(&rxrdy, 0x00);		  
	this->write_signals(&key_ack, 0xff);		  
#endif
	this->write_signals(&kana_led, 0x00);		  
	this->write_signals(&caps_led, 0x00);		  
	this->write_signals(&ins_led, 0x00);
}


void KEYBOARD::reset(void)
{
	keymode = KEYMODE_STANDARD;
	scancode = 0x00;
	//keycode_7 = 0x00; 
	keycode_7 = 0xffffffff; 
	reset_unchange_mode();
#if defined(_FM77AV_VARIANTS)  
	adjust_rtc();
#endif
	key_fifo->clear();
	if(event_int >= 0) cancel_event(this, event_int);
	register_event(this,
		       ID_KEYBOARD_INT,
		       20000.0, true, &event_int);
	write_signals(&int_line, 0x00000000);
	
	write_signals(&kana_led, 0x00000000);
	write_signals(&caps_led, 0x00000000);
	write_signals(&ins_led,  0x00000000);
#if defined(_FM77AV_VARIANTS)  
	write_signals(&rxrdy,    0xffffffff);
	write_signals(&key_ack,  0x00000000);
#endif
}


#if defined(_FM77AV_VARIANTS)  
// 0xd431 : Read
uint8 KEYBOARD::read_data_reg(void)
{
	if(!data_fifo->empty()) {
		datareg = data_fifo->read() & 0xff;
	}
	if(!data_fifo->empty()) {
		rxrdy_status = true;
		write_signals(&rxrdy, 0xff);
	} else {
		rxrdy_status = false;
		write_signals(&rxrdy, 0x00);
	}
	return datareg;
}

// 0xd432
uint8 KEYBOARD::read_stat_reg(void)
{
	uint8 data = 0xff;
	if(rxrdy_status) {
		data &= 0x7f;
	}
	if(!key_ack_status) {
		data &= 0xfe;
	}
	// Digityze : bit0 = '0' when waiting,
	return data;
}

void KEYBOARD::set_mode(void)
{
	int count = cmd_fifo->count();
	int cmd;
	int mode;
	if(count < 2) return;
	cmd = cmd_fifo->read();
	mode = cmd_fifo->read();
	if(mode <= KEYMODE_SCAN) {
		keymode = mode;
		//printf("Keymode : %d\n", keymode);
		//reset_unchange_mode();
		if(scancode != 0) key_down_main(); 
	}
	cmd_fifo->clear();
	data_fifo->clear(); // right?
	rxrdy_status = false;
	write_signals(&rxrdy, 0x00);
}

void KEYBOARD::get_mode(void)
{
	int cmd;
	int dummy;
	cmd = cmd_fifo->read();
	if(data_fifo->full()) {
		dummy = data_fifo->read();
	}
	data_fifo->write(keymode);
	rxrdy_status = true;
	write_signals(&rxrdy, 0xff);
}

void KEYBOARD::set_leds(void)
{
	int count = cmd_fifo->count();
	int cmd;
	int ledvar;
	if(count < 2) return;
	cmd = cmd_fifo->read();
	ledvar = cmd_fifo->read();
	if(ledvar < 4) {
		if((ledvar & 0x02) != 0) {
			// Kana
			kana_pressed = ((ledvar & 0x01) == 0);
			write_signals(&kana_led, kana_pressed);
		} else {
			// Caps
			caps_pressed = ((ledvar & 0x01) == 0);
			write_signals(&caps_led, caps_pressed);
		}
	}
	cmd_fifo->clear();
	data_fifo->clear(); // right?
	rxrdy_status = false;
	write_signals(&rxrdy, 0x00);
}

void KEYBOARD::get_leds(void)
{
	uint8 ledvar = 0x00;
	data_fifo->clear();
	ledvar |= caps_pressed ? 0x01 : 0x00;
	ledvar |= kana_pressed ? 0x02 : 0x00;
	data_fifo->write(ledvar);
	cmd_fifo->clear();
	rxrdy_status = true;
	write_signals(&rxrdy, 0xff);
}

void KEYBOARD::set_repeat_type(void)
{
	int cmd;
	int modeval;

	cmd = cmd_fifo->read();
	if(!cmd_fifo->empty()) {
		modeval = cmd_fifo->read();
		if((modeval < 2) && (modeval >= 0)) {
			repeat_mode = (modeval == 0);
			if(repeat_mode) {
				//scancode = 0x00;
				//keycode_7 = 0x00;
				key_fifo->clear();
			}
		}
	}
	data_fifo->clear();
	cmd_fifo->clear();
	rxrdy_status = false;
	write_signals(&rxrdy, 0x00);
}

void KEYBOARD::set_repeat_time(void)
{
	int cmd;
	uint32 time_high = 0;
	uint32 time_low = 0;
	cmd = cmd_fifo->read();
	if(cmd_fifo->empty()) goto _end;
	time_high = cmd_fifo->read();
	if(cmd_fifo->empty()) goto _end;
	time_low = cmd_fifo->read();
//	if(cmd_fifo->empty()) goto _end;
_end:
	if((time_high == 0) || (time_low == 0)) {
		repeat_time_long = 700;
		repeat_time_short = 70;
	} else {
		repeat_time_long = (int)time_high * 10;
		repeat_time_short = (int)time_low * 10;
	}
	data_fifo->clear();
	cmd_fifo->clear();
	rxrdy_status = false;
	write_signals(&rxrdy, 0x00);
}

void KEYBOARD::set_rtc(void)
{
	int cmd;
	int tmp;
	int localcmd;
	if(cmd_fifo->count() < 9) return;
	cmd = cmd_fifo->read();
	localcmd = cmd_fifo->read();
	// YY
	tmp = cmd_fifo->read();
	rtc_yy = ((tmp >> 4) * 10) | (tmp & 0x0f);
	// MM
	tmp = cmd_fifo->read();
	rtc_mm = ((tmp >> 4) * 10) | (tmp & 0x0f);
	// DD
	tmp = cmd_fifo->read();
	rtc_dd = (((tmp & 0x30) >> 4) * 10) | (tmp & 0x0f);
	// DayOfWeek + Hour
	tmp = cmd_fifo->read();
	rtc_count24h = ((tmp & 0x08) != 0);
	if(!rtc_count24h) {
		rtc_ispm = ((tmp & 0x04) != 0);
	}
	rtc_dayofweek = (tmp >> 4) % 0x07;
	rtc_hour = ((tmp & 0x03) * 10);
	// Low
	tmp = cmd_fifo->read();
	rtc_hour = rtc_hour | (tmp >> 4);
	if(rtc_count24h) {
	  rtc_ispm = (rtc_hour >= 12);
	}
	rtc_minute = (tmp & 0x0f) * 10;
	
	tmp = cmd_fifo->read();
	rtc_minute = rtc_minute | (tmp >> 4);
	rtc_sec = (tmp & 0x0f) * 10;
	
	tmp = cmd_fifo->read();
	rtc_sec = rtc_sec | (tmp >> 4);
	
	data_fifo->clear();
	cmd_fifo->clear();
	if(event_key_rtc >= 0) {
		cancel_event(this, event_key_rtc);
	}
	register_event(this, ID_KEYBOARD_RTC_COUNTUP, 1000.0 * 1000.0, true, &event_key_rtc);
	rxrdy_status = false;
	write_signals(&rxrdy, 0x00);
}

void KEYBOARD::get_rtc(void)
{
	int tmp;
	data_fifo->clear();
	// YY
	tmp = ((rtc_yy / 10) << 4) | (rtc_yy % 10);
	data_fifo->write(tmp);
	// MM
	tmp = ((rtc_mm / 10) << 4) | (rtc_mm % 10);
	data_fifo->write(tmp);
	// DD
	tmp = ((rtc_dd / 10) << 4) | (rtc_dd % 10);
	tmp = tmp | (0 << 6); // leap
	data_fifo->write(tmp);
	// DayOfWeek + Hour
	tmp = rtc_dayofweek << 4;
	tmp = tmp | (rtc_hour / 10);
	if(rtc_count24h) {
	  tmp = tmp | 0x08;
	} else {
	  if(rtc_ispm) {
	    tmp = tmp | 0x04;
	  }
	}
	data_fifo->write(tmp);
	// Low
	tmp = (rtc_hour % 10) << 4;
	tmp = tmp | (rtc_minute / 10);
	data_fifo->write(tmp);
	
	tmp = (rtc_minute % 10) << 4;
	tmp = tmp | (rtc_sec / 10);
	data_fifo->write(tmp);
	
	tmp = (rtc_sec % 10) << 4;
	data_fifo->write(tmp);
	
	cmd_fifo->clear();
	rxrdy_status = true;
	write_signals(&rxrdy, 0xff);
}

const int rtc_month_days[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

void KEYBOARD::rtc_count(void)
{
	// count per 1sec
	rtc_sec++;
	if(rtc_sec >= 60) {
		rtc_sec = 0;
		rtc_minute++;
		if(rtc_minute >= 60) {
			rtc_minute = 0;
			rtc_hour++;
			if(rtc_count24h) {
				rtc_ispm = (rtc_hour >= 12);
				if(rtc_hour < 24) return;
			} else {
				if(rtc_ispm) {
					if(rtc_hour < 12) return;
				} else {
					if(rtc_hour < 12) return;
					rtc_ispm = true;
					rtc_hour = 0;
					return;
				}
			}
			// Day count up
			rtc_hour = 0;
			rtc_dd++;
			rtc_dayofweek++;
			if(rtc_dayofweek >= 7) rtc_dayofweek = 0;
			if(rtc_dd > rtc_month_days[rtc_mm]){
				if((rtc_mm ==1) && (rtc_dd == 29)) {
					if((rtc_yy % 4) == 0) return;
				}
				rtc_dd = 1;
				rtc_mm++;
				if(rtc_mm >= 12) {
					rtc_yy++;
					rtc_mm = 0;
					if(rtc_yy >= 100) rtc_yy = 0;
				}
			}
		}
	}
}
#endif // FM77AV_VARIANTS

uint32 KEYBOARD::read_signal(int id)
{
	if(id == SIG_FM7KEY_BREAK_KEY) {
		return break_pressed ? 0xfffffff : 0x00000000;
	}
	return 0;
}


void KEYBOARD::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_FM7KEY_SET_INSLED) {
		write_signals(&ins_led, data & mask);
	}
#if defined(_FM77AV_VARIANTS)  
	 else if(id == SIG_FM7KEY_PUSH_TO_ENCODER) {
		/*
		 * I refered XM7's sourcecode : VM/keyboard.c act of key-encoder.
		 * Thanks to Ryu.Takegami and PI.
		 */
		int count;
		if(!key_ack_status) return; // If (not(ACK)) noop.

		if(cmd_fifo->full()) {
			cmd_fifo->clear();
		}
		if(cmd_fifo->empty()) {
			cmd_phase = data & 0xff;
		}
		
		cmd_fifo->write(data & 0xff);
		count = cmd_fifo->count();
		
		rxrdy_status = false;
		key_ack_status = false;
		write_signals(&key_ack, 0x00);
		write_signals(&rxrdy, 0x00);
	    
		switch(cmd_phase) {
			case 0: // Set mode
				if(count >= 2) {
					set_mode();
					if(keymode == KEYMODE_SCAN) key_down_main();
					cmd_phase = -1;
					cmd_fifo->clear();
				}
				break;
			case 1: // Get mode
				get_mode();
				cmd_fifo->clear();
				cmd_phase = -1;
				break;
			case 2: // Set LED Phase
				if(count >= 2) {
					set_leds();
					if(keymode == KEYMODE_SCAN) key_down_main();
					cmd_phase = -1;
					cmd_fifo->clear();
				}
				break;
			case 3: // Get LED Phase
				get_leds();
				cmd_phase = -1;
				cmd_fifo->clear();
				break;
			case 4:
				if(count >= 2) {
					set_repeat_type();
					cmd_phase = -1;
					cmd_fifo->clear();
				}
				break;
			case 5:
				if(count >= 3) {
					set_repeat_time();
					cmd_phase = -1;
					cmd_fifo->clear();
				}
				break;
			case 0x80: // Communicate to/from RTC.
				if(count == 1) {
					rtc_set = false;
				}
				if(count == 2) {
					if((data & 0xff) == 0) { // Get
						get_rtc();
						cmd_phase = -1;
						cmd_fifo->clear();
					} else if((data & 0xff) == 1) { // Set
						rtc_set_flag = true;
					} else { // Illegal
						cmd_fifo->clear();
						cmd_phase = -1;
					}
				}
				if(rtc_set_flag) {
					if(count >= 9) {
						set_rtc();
						cmd_fifo->clear();
						cmd_phase = -1;
					}
				}
				break;
			case 0x81: // Digitize.
				if(count >= 2) {
					do_digitize(); // WILL Implement?
					cmd_fifo->clear();
					cmd_phase = -1;
				}
				break;
			case 0x82:
				if(count >= 2) {
					set_screen_mode(); // WILL Implement?
					cmd_fifo->clear();
					cmd_phase = -1;
				}
				break;
			case 0x83:
				get_screen_mode(); // WILL Implement?
				cmd_fifo->clear();
				cmd_phase = -1;
				break;
			case 0x84:
				if(count >= 2) {
					set_brightness(); // WILL Implement?
					cmd_fifo->clear();
					cmd_phase = -1;
				}
				break;
			default:
				cmd_fifo->clear();
				cmd_phase = -1;
				break;
		}
		register_event(this, ID_KEYBOARD_ACK, 5, false, NULL); // Delay 5us until ACK is up.
	} else if(id == SIG_FM7KEY_RXRDY) {
		rxrdy_status = ((data & mask) != 0);
		//write_signals(&rxrdy, (rxrdy_status) ? 0xffffffff : 0x00000000);
	} else if(id == SIG_FM7KEY_ACK) {
		key_ack_status = ((data & mask) != 0);
		//write_signals(&key_ack, (key_ack_status) ? 0xffffffff : 0x00000000);
	}

#endif
}

uint32 KEYBOARD::read_data8(uint32 addr)
{
	uint32 retval = 0xff;
	switch(addr) {
		case 0x00:
			retval = get_keycode_high();
			break;
		case 0x01:
			retval = get_keycode_low();
			break;
#if defined(_FM77AV_VARIANTS)			
		case 0x31:
			retval = read_data_reg();
			break;
		case 0x32:
			retval = read_stat_reg();
			break;
#endif
		default:
			break;
	}
	return retval;
}

void KEYBOARD::write_data8(uint32 addr, uint32 data)
{
	switch(addr) {
#if defined(_FM77AV_VARIANTS)			
		case 0x31:
			this->write_signal(SIG_FM7KEY_PUSH_TO_ENCODER, data, 0x000000ff);
			break;
#endif
	}
}

KEYBOARD::KEYBOARD(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	int i;
	p_vm = parent_vm;
	p_emu = parent_emu;
  
	keycode_7 = 0xffffffff;
   
	ctrl_pressed = false; 
	lshift_pressed = false; 
	rshift_pressed = false; 
	shift_pressed = false; 
	graph_pressed = false;
	caps_pressed = false;
	kana_pressed = false;
	break_pressed = false;
	event_keyrepeat = -1;
   
	keymode = KEYMODE_STANDARD;
#if defined(_FM77AV_VARIANTS)
	cmd_fifo = new FIFO(16);
	data_fifo = new FIFO(16);
	rxrdy_status = true;
	key_ack_status = false;
	init_output_signals(&rxrdy);
	init_output_signals(&key_ack);
	
	rtc_count24h = false;
	rtc_dayofweek = 0;
	rtc_ispm = false;
	rtc_set = false;
	rtc_set_flag = false;
	rtc_yy = 0;
	rtc_mm = 0;
	rtc_dd = 0;
	rtc_hour = 0;
	rtc_minute = 0;
	rtc_sec = 0;
	event_key_rtc = -1;
	event_hidden1_av = -1;
	hidden1_ptr = 0;
#endif
	key_fifo = new FIFO(256);
	event_int = -1;

	init_output_signals(&break_line);
	init_output_signals(&int_line);
	
	init_output_signals(&kana_led);
	init_output_signals(&caps_led);
	init_output_signals(&ins_led);
}

void KEYBOARD::release(void)
{
#if defined(_FM77AV_VARIANTS)
	cmd_fifo->release();
	data_fifo->release();
	delete cmd_fifo;
	delete data_fifo;
#endif   
	key_fifo->release();
	delete key_fifo;
}

KEYBOARD::~KEYBOARD()
{
}

#define STATE_VERSION 1
void KEYBOARD::save_state(FILEIO *state_fio)
{
	int ch;
	int addr;
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);

	// Version 1
	{
		int id;
		state_fio->FputUint32_BE(keycode_7);
		state_fio->FputInt32_BE(keymode);
	   
		state_fio->FputBool(ctrl_pressed);
		state_fio->FputBool(lshift_pressed);
		state_fio->FputBool(rshift_pressed);
		state_fio->FputBool(shift_pressed);
		state_fio->FputBool(graph_pressed);
		state_fio->FputBool(caps_pressed);
		state_fio->FputBool(kana_pressed);
		state_fio->FputBool(break_pressed);

		state_fio->FputInt32_BE(event_keyrepeat);
	   
		state_fio->FputUint32(scancode);
		state_fio->FputUint8(datareg);
		state_fio->FputUint32(older_vk);
	   
		state_fio->FputBool(repeat_mode);
		state_fio->FputInt32_BE(repeat_time_short);
		state_fio->FputInt32_BE(repeat_time_long);
		state_fio->FputUint8(repeat_keycode);
	   
#if defined(_FM77AV_VARIANTS)
		state_fio->FputInt32_BE(event_key_rtc);
  
		state_fio->FputUint8(rtc_yy);
		state_fio->FputUint8(rtc_mm);
		state_fio->FputUint8(rtc_dd);
		state_fio->FputUint8(rtc_dayofweek);
		state_fio->FputUint8(rtc_hour);
		state_fio->FputUint8(rtc_minute);
		state_fio->FputUint8(rtc_sec);

		state_fio->FputBool(rtc_count24h);
		state_fio->FputBool(rtc_ispm);

		state_fio->FputBool(rtc_set);
		state_fio->FputBool(rtc_set_flag);
		state_fio->FputBool(rxrdy_status);
		state_fio->FputBool(key_ack_status);
		state_fio->FputInt32_BE(cmd_phase);

		state_fio->FputInt32_BE(event_hidden1_av);
		state_fio->FputUint16_BE(hidden1_ptr);

		cmd_fifo->save_state((void *)state_fio);
		data_fifo->save_state((void *)state_fio);
		cur_time.save_state((void *)state_fio);
#endif
		state_fio->FputInt32_BE(event_int);
		key_fifo->save_state((void *)state_fio);
	}
}

bool KEYBOARD::load_state(FILEIO *state_fio)
{
	int ch;
	int addr;
	bool stat = false;
	uint32 version;
	
	version = state_fio->FgetUint32_BE();
	if(this_device_id != state_fio->FgetInt32_BE()) return false;

	if(version >= 1) {
		int id;
		keycode_7 = state_fio->FgetUint32_BE();
		keymode = state_fio->FgetInt32_BE();
	   
		ctrl_pressed = state_fio->FgetBool();
		lshift_pressed = state_fio->FgetBool();
		rshift_pressed = state_fio->FgetBool();
		shift_pressed = state_fio->FgetBool();
		graph_pressed = state_fio->FgetBool();
		caps_pressed = state_fio->FgetBool();
		kana_pressed = state_fio->FgetBool();
		break_pressed = state_fio->FgetBool();

		event_keyrepeat = state_fio->FgetInt32_BE();
	   
		scancode = state_fio->FgetUint32();
		datareg = state_fio->FgetUint8();
		older_vk = state_fio->FgetUint32();
	   
		repeat_mode = state_fio->FgetBool();
		repeat_time_short = state_fio->FgetInt32_BE();
		repeat_time_long = state_fio->FgetInt32_BE();
		repeat_keycode = state_fio->FgetUint8();
	   
#if defined(_FM77AV_VARIANTS)
		event_key_rtc = state_fio->FgetInt32_BE();
		rtc_yy = state_fio->FgetUint8();
		rtc_mm = state_fio->FgetUint8();
		rtc_dd = state_fio->FgetUint8();
		rtc_dayofweek = state_fio->FgetUint8();
		rtc_hour = state_fio->FgetUint8();
		rtc_minute = state_fio->FgetUint8();
		rtc_sec = state_fio->FgetUint8();

		rtc_count24h = state_fio->FgetBool();
		rtc_ispm = state_fio->FgetBool();

		rtc_set = state_fio->FgetBool();
		rtc_set_flag = state_fio->FgetBool();
		rxrdy_status = state_fio->FgetBool();
		key_ack_status = state_fio->FgetBool();
		cmd_phase = state_fio->FgetInt32_BE();

		event_hidden1_av = state_fio->FgetInt32_BE();
		hidden1_ptr = state_fio->FgetUint16_BE();

		cmd_fifo->load_state((void *)state_fio);
		data_fifo->load_state((void *)state_fio);
		cur_time.load_state((void *)state_fio);
#endif
		state_fio->FputInt32_BE(event_int);
		key_fifo->save_state((void *)state_fio);
		if(version == 1) return true;
	}
	return false;
}

   
