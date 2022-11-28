/*
 * Common Source code Project -> VM -> FM-7/77AV -> Keyboard
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Licence: GPLv2
 * History : 
 *  Feb 12, 2015 : Initial 
 */

#include "../vm.h"
#include "../../emu.h"
#include "../../fifo.h"
#include "../device.h"
#include "fm7_keyboard.h"
#include "keyboard_tables.h"

#if defined(_FM77AV_VARIANTS)
#include "../beep.h"
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
	ID_KEYBOARD_HIDDENMESSAGE_AV,
	ID_KEYBOARD_HIDDEN_BEEP_ON,
	ID_KEYBOARD_HIDDEN_BEEP_OFF,
	ID_KEYBOARD_AUTO_8KEY_START,
	ID_KEYBOARD_AUTO_8KEY_END,
	ID_KEYBOARD_AUTO_5KEY_START,
	ID_KEYBOARD_AUTO_5KEY_END,
	ID_KEYBOARD_BREAK_ONESHOT,
};

//
/*
 * I/O API (subio)
 */
// 0xd400(SUB) or 0xfd00(MAIN)
uint8_t KEYBOARD::get_keycode_high(void)
{
	uint8_t data = 0x00;
	if((keycode_7 & 0x0100) != 0) data = 0x80;
	return data;
}

// 0xd401(SUB) or 0xfd01(MAIN)
uint8_t KEYBOARD::get_keycode_low(void)
{
	uint8_t data = keycode_7 & 0xff;
	this->write_signals(&int_line, 0x00000000);
	return data;
}

// 0xd40d : R
void KEYBOARD::turn_on_ins_led(void)
{
	ins_led_status = true;
}

// 0xd40d : W
void KEYBOARD::turn_off_ins_led(void)
{
	ins_led_status = false;
}

// UI Handler. 
uint16_t KEYBOARD::vk2scancode(uint32_t vk)
{
	uint16_t i;
	i = 0;
	if(vk == VK_PAUSE) vk = VK_KANJI; // Workaround some desktop environments for [ESC].
	do {
		if(vk_matrix_106[i] == vk) return i;
		i++;
	} while(vk_matrix_106[i] != 0xffff);
	return 0x0000;
}

bool KEYBOARD::isModifier(uint8_t sc)
{
	if(((sc >= 0x52) && (sc <= 0x56)) || // CTRL LSHIFT RSHIFT CAPS GRPH
     		(sc == 0x5a) || (sc == 0x5c)) { // KANA BREAK
		return true;
	}
	return false;
}

void KEYBOARD::set_modifiers(uint8_t sc, bool flag)
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
			if(keymode == KEYMODE_STANDARD) caps_led_status = caps_pressed;
		}
	} else if(sc == 0x5a) { // KANA
		// Toggle on press.
		if(flag) {
			if(kana_pressed) {
				kana_pressed = false;
			} else {
				kana_pressed = true;
			}
			if(keymode == KEYMODE_STANDARD) kana_led_status = kana_pressed;
		}
	} else if(sc == 0x5c) { // Break
		if(!override_break_key) {
			break_pressed = flag;
		}
	}
}

uint16_t KEYBOARD::scan2fmkeycode(uint8_t sc)
{
	const struct key_tbl_t *keyptr = NULL;
	bool stdkey = false;
	int i;
	uint16_t retval;
	
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
		retval = (uint16_t)sc;
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
		if(keyptr[i].phy == (uint16_t)sc) {
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

void KEYBOARD::key_up_main(uint8_t bak_scancode)
{
	bool stat_break = break_pressed;
	older_vk = 0;
	if(bak_scancode == 0) return;
	if((event_keyrepeat >= 0) && (repeat_keycode == bak_scancode)) { // Not Break
		cancel_event(this, event_keyrepeat);
		event_keyrepeat = -1;
		repeat_keycode = 0;
	}
	if(keymode != KEYMODE_SCAN) {
		if(this->isModifier(bak_scancode)) {
			set_modifiers(bak_scancode, false);
			if(break_pressed != stat_break) { // Break key UP.
				this->write_signals(&break_line, 0x00);
			}
		}
		if((config.dipswitch & FM7_DIPSW_AUTO_5_OR_8KEY) != 0) {
			if((config.dipswitch & FM7_DIPSW_SELECT_5_OR_8KEY) == 0) { // Auto 8
				switch(bak_scancode) {
					case 0x42: // 1
					case 0x43: // 2
					case 0x44: // 3
					case 0x3f: // 5
					case 0x46: // 0
						register_event(this,
									   ID_KEYBOARD_AUTO_8KEY_START,
									   10.0 * 1000.0, false, NULL);
						break;
					default:
			   			if(autokey_backup != 0) key_fifo->write(scan2fmkeycode(autokey_backup));
						break;
				}
			} else { // Auto 5
				switch(bak_scancode) {
					case 0x42: // 1
					case 0x43: // 2
					case 0x44: // 3
					case 0x3e: // 4
					case 0x40: // 6
					case 0x3a: // 7
					case 0x3b: // 8
					case 0x3c: // 9
						register_event(this,
									   ID_KEYBOARD_AUTO_5KEY_START,
									   10.0 * 1000.0, false, NULL);
						break;
					default:
			   			if(autokey_backup != 0) key_fifo->write(scan2fmkeycode(autokey_backup));
						break;
				}
			}				
		}
	} else {
		//scancode = 0;
		if(bak_scancode != 0) { // Notify even key-up, when using SCAN mode.
			uint8_t code = (bak_scancode & 0x7f) | 0x80;
			if(this->isModifier(bak_scancode)) {
				set_modifiers(bak_scancode, false);
			}
#if defined(_FM77AV_VARIANTS)	   
			if(bak_scancode != 0x5c) {
				if(beep_phase == 1) {
					if(break_pressed) this->write_signals(&break_line, 0x00);
					break_pressed = false;
					beep_phase++;
					bak_scancode = 0x7f;
					code = 0xff;
				} else if(beep_phase == 2) {
					beep_phase = 0;
					register_event(this,
								   ID_KEYBOARD_HIDDEN_BEEP_ON,
								   100.0, false, NULL); // 100.0 us is dummy.
					bak_scancode = 0x7f;
					code = 0xff;
				}
			} else { // 0x5c : BREAK is up.
				beep_phase = 0;
				if(stat_break) this->write_signals(&break_line, 0x00);
			}
#endif		   
			if(code != 0x80) {
				key_fifo->write(code);
				scancode = bak_scancode;
			}
		}
	}
}

void KEYBOARD::key_up(uint32_t vk)
{
	uint8_t bak_scancode = (uint8_t)(vk2scancode(vk) & 0x00ff);
	key_up_main(bak_scancode);
}

void KEYBOARD::key_down(uint32_t vk)
{
	if(older_vk == vk) return;
	older_vk = vk;
	
	scancode = (uint8_t)vk2scancode(vk);
#if defined(_FM77AV_VARIANTS)
	// Below are FM-77AV's hidden message , see :
	// https://twitter.com/maruan/status/499558392092831745
	//if(caps_pressed && kana_pressed) {
	//	if(ctrl_pressed && lshift_pressed && rshift_pressed && graph_pressed) {
	if(caps_pressed && kana_pressed && graph_pressed && shift_pressed && ctrl_pressed && !did_hidden_message_av_1) { // IT's deprecated key pressing
		if(scancode == 0x15) { // "T"
			if(event_hidden1_av < 0) {
				hidden1_ptr = 0;
				did_hidden_message_av_1 = true;
				register_event(this,
						ID_KEYBOARD_HIDDENMESSAGE_AV,
						130.0 * 1000, true, &event_hidden1_av);
			}
			return;
		}
	}
#endif 
	key_down_main(true);
}

void KEYBOARD::key_down_main(bool repeat_auto_key)
{
	bool stat_break = break_pressed;
	uint32_t code;
	if(scancode == 0) return;
	if(keymode == KEYMODE_SCAN) {
		code = scancode & 0x7f;
		if(this->isModifier(scancode)) {  // modifiers
			set_modifiers(scancode, true);
		}
#if defined(_FM77AV_VARIANTS)
		if(break_pressed) {
			if(!stat_break) {
				beep_phase = 1;
				// It's dirty hack for AMNORK ; Set dead zone for FIRQ to 0.25sec.
				// I wish to replace this solution to more simple. 
				register_event(this,
							   ID_KEYBOARD_BREAK_ONESHOT,
							   250.0 * 1000.0, false, NULL);
			} else if(beep_phase == 1) {
				if(code != 0x5c) {
					if(break_pressed) this->write_signals(&break_line, 0x00);
					break_pressed = false;
					beep_phase++;
					code = 0x7f; // Special
					scancode = 0x7f;	
				}
			}
		}
#endif

		if(code != 0) {
			key_fifo->write(code);
			// NOTE: With scan key code mode, auto repeat seems to be not effectable.
			// See : http://hanabi.2ch.net/test/read.cgi/i4004/1430836648/607
			// 20160409 K.Ohta
		}
	} else {
		if(this->isModifier(scancode)) {  // modifiers
			set_modifiers(scancode, true);
			if(break_pressed != stat_break) { // Break key Down.
				this->write_signals(&break_line, 0xff);
			}
			return;
		}
		code = scan2fmkeycode(scancode);
		if((code > 0x3ff) || (code == 0)) return;
		if((config.dipswitch & FM7_DIPSW_AUTO_5_OR_8KEY) != 0) {
			if((config.dipswitch & FM7_DIPSW_SELECT_5_OR_8KEY) == 0) { // Auto 8
				switch(scancode) {
					case 0x42: // 1
					case 0x43: // 2
					case 0x44: // 3
					case 0x3f: // 5
					case 0x46: // 0
						autokey_backup = scancode;
						break;
				}
			} else { // Auto 5
				switch(scancode) {
					case 0x42: // 1
					case 0x43: // 2
					case 0x44: // 3
					case 0x3e: // 4
					case 0x40: // 6
					case 0x3a: // 7
					case 0x3b: // 8
					case 0x3c: // 9
						autokey_backup = scancode;
						break;
				}
			}				
		} else {
			autokey_backup = 0x00;
		}
		if(code != 0) {
			key_fifo->write(code);
			if((scancode < 0x5c) && repeat_auto_key) {
				double usec = (double)repeat_time_long * 1000.0;
				if((repeat_keycode == 0) && repeat_mode) {
					if(event_keyrepeat >= 0) cancel_event(this, event_keyrepeat);
					event_keyrepeat = -1;
					register_event(this,
								   ID_KEYBOARD_AUTOREPEAT_FIRST,
								   usec, false, &event_keyrepeat);
				}
				repeat_keycode = scancode;
			}
		}
	}
}

#if defined(_FM77AV_VARIANTS)
void KEYBOARD::adjust_rtc(void)
{
	get_host_time(&cur_time);
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

void KEYBOARD::do_repeatkey(uint8_t sc)
{
	uint16_t code;
	if((sc == 0) || (sc >= 0x5c)) return; // scancode overrun.
	if(!repeat_mode) {
		if(event_keyrepeat >= 0) {
			cancel_event(this, event_keyrepeat);
			event_keyrepeat = -1;
		}
		return;
	}
	if(keymode == KEYMODE_SCAN) {
		code = (uint16_t)(sc & 0x7f);
		key_fifo->write((uint32_t)code); // Make
		//key_fifo->write((uint32_t)(code | 0x80)); // Break
	} else {
		code = scan2fmkeycode(sc);
		if(code < 0x400) {
			key_fifo->write((uint32_t)code);
		}
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
		beep->write_signal(SIG_BEEP_MUTE, 0, 1);
		beep->write_signal(SIG_BEEP_ON, 1, 1);
		register_event(this,
		       ID_KEYBOARD_HIDDEN_BEEP_OFF,
		       25.0 * 1000.0, false, NULL);
	} else	if(event_id == ID_KEYBOARD_HIDDEN_BEEP_ON) {
		beep->write_signal(SIG_BEEP_MUTE, 0, 1);
		beep->write_signal(SIG_BEEP_ON, 1, 1);
		register_event(this,
		       ID_KEYBOARD_HIDDEN_BEEP_OFF,
		       25.0 * 1000.0, false, NULL);

	} else	if(event_id == ID_KEYBOARD_HIDDEN_BEEP_OFF) {
		beep->write_signal(SIG_BEEP_MUTE, 0, 1);
		beep->write_signal(SIG_BEEP_ON, 0, 1);
	} else if(event_id == ID_KEYBOARD_BREAK_ONESHOT) {
		if(break_pressed) this->write_signals(&break_line, 0xff);
	} else
#endif
	if(event_id == ID_KEYBOARD_AUTOREPEAT_FIRST) {
		double usec = (double)repeat_time_short * 1000.0;

		do_repeatkey(repeat_keycode);
		register_event(this,
			       ID_KEYBOARD_AUTOREPEAT,
			       usec, true, &event_keyrepeat);
		// Key repeat.
	} else if(event_id == ID_KEYBOARD_AUTOREPEAT){
		if(repeat_keycode != 0) {
			do_repeatkey(repeat_keycode);
		} else {
			cancel_event(this, event_keyrepeat);
			event_keyrepeat = -1;
		}
	} else if(event_id == ID_KEYBOARD_INT) {
		if(!(key_fifo->empty())) {
			keycode_7 = key_fifo->read();
			this->write_signals(&int_line, 0xffffffff);
		}
	} else if(event_id == ID_KEYBOARD_AUTO_8KEY_START) {
		if(keymode != KEYMODE_SCAN) {
			scancode = 0x3b; // 8
			autokey_backup = 0x00;
			key_down_main(false);
			register_event(this,
						   ID_KEYBOARD_AUTO_8KEY_END,
						   50.0 * 1000.0, false, NULL);
		}
	} else if(event_id == ID_KEYBOARD_AUTO_8KEY_END) {
		key_up_main(0x3b); // 8
	} else if(event_id == ID_KEYBOARD_AUTO_5KEY_START) {
		if(keymode != KEYMODE_SCAN) {
			scancode = 0x3f; // 5
			autokey_backup = 0x00;
			key_down_main(false);
			register_event(this,
						   ID_KEYBOARD_AUTO_5KEY_END,
						   50.0 * 1000.0, false, NULL);
		}
	} else if(event_id == ID_KEYBOARD_AUTO_5KEY_END) {
		key_up_main(0x3f); // 5
	}
}

// Commands
void KEYBOARD::reset_unchange_mode(void)
{
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
	ins_led_status = false;
	kana_led_status = false;
	caps_led_status = false;
	datareg = 0x00;
	repeat_keycode = 0x00;
	autokey_backup = 0x00;

	if(override_break_key) write_signals(&break_line, (break_pressed) ? 0xff : 0);
	key_fifo->clear();
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
}


void KEYBOARD::reset(void)
{
	keymode = KEYMODE_STANDARD;
	scancode = 0x00;

	keycode_7 = 0xffffffff; 
	//keycode_7 = 0; 
	reset_unchange_mode();
	this->write_signals(&int_line, 0x00000000);

#if defined(_FM77AV_VARIANTS)  
	adjust_rtc();
	did_hidden_message_av_1 = false;
	beep_phase = 0;
#endif

	if(override_break_key) write_signals(&break_line, (break_pressed) ? 0xff : 0);

	if(event_int >= 0) cancel_event(this, event_int);
	register_event(this,
				   ID_KEYBOARD_INT,
				   20000.0, true, &event_int);
}


#if defined(_FM77AV_VARIANTS)  
// 0xd431 : Read
uint8_t KEYBOARD::read_data_reg(void)
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
uint8_t KEYBOARD::read_stat_reg(void)
{
	uint8_t data = 0xff;
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
		beep_phase = 0;
		autokey_backup = 0x00;
		this->write_signals(&break_line, 0x00);
		if(scancode != 0) key_down_main(true); 
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
			kana_led_status = kana_pressed;
		} else {
			// Caps
			caps_pressed = ((ledvar & 0x01) == 0);
			caps_led_status = caps_pressed;
		}
	}
	cmd_fifo->clear();
	data_fifo->clear(); // right?
	rxrdy_status = false;
	write_signals(&rxrdy, 0x00);
}

void KEYBOARD::get_leds(void)
{
	uint8_t ledvar = 0x00;
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
	uint32_t time_high = 0;
	uint32_t time_low = 0;
	cmd = cmd_fifo->read();
	if(cmd_fifo->empty()) goto _end;
	time_high = cmd_fifo->read();
	if(cmd_fifo->empty()) goto _end;
	time_low = cmd_fifo->read();
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

uint32_t KEYBOARD::read_signal(int id)
{
	if(id == SIG_FM7KEY_BREAK_KEY) {
		return break_pressed ? 0xfffffff : 0x00000000;
	} else if(id == SIG_FM7KEY_LED_STATUS) {
		uint32_t _l;
		_l  = (ins_led_status) ? 0x00000001 : 0;
		_l |= (kana_led_status) ? 0x00000002 : 0;
		_l |= (caps_led_status) ? 0x00000004 : 0;
		return _l;
	}
	return 0;
}


void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_FM7KEY_SET_INSLED) {
		ins_led_status = ((data & mask) != 0);
	} else if(id == SIG_FM7KEY_OVERRIDE_PRESS_BREAK) {
		override_break_key = ((data & mask) != 0);
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
					if(keymode == KEYMODE_SCAN) key_down_main(true);
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
					if(keymode == KEYMODE_SCAN) key_down_main(true);
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

uint32_t KEYBOARD::read_data8(uint32_t addr)
{
	uint32_t retval = 0xff;
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

void KEYBOARD::write_data8(uint32_t addr, uint32_t data)
{
	switch(addr) {
#if defined(_FM77AV_VARIANTS)			
		case 0x31:
			this->write_signal(SIG_FM7KEY_PUSH_TO_ENCODER, data, 0x000000ff);
			break;
#endif
	}
}

KEYBOARD::KEYBOARD(VM_TEMPLATE* parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
#if defined(_FM77AV_VARIANTS)
	beep = NULL;
#endif	
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
	autokey_backup = 0x00;

	keymode = KEYMODE_STANDARD;
	override_break_key = false;
#if defined(_FM77AV_VARIANTS)
	cmd_fifo = new FIFO(16);
	data_fifo = new FIFO(16);
	rxrdy_status = true;
	key_ack_status = false;
	initialize_output_signals(&rxrdy);
	initialize_output_signals(&key_ack);
	
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
	key_fifo = new FIFO(512);
	event_int = -1;
	
	initialize_output_signals(&break_line);
	initialize_output_signals(&int_line);
	
	ins_led_status = false;
	kana_led_status = false;
	caps_led_status = false;
	set_device_name(_T("KEYBOARD SUBSYSTEM"));
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

#define STATE_VERSION 9
//#if defined(Q_OS_WIN)
//DLL_PREFIX_I struct cur_time_s cur_time;
//#endif

bool KEYBOARD::decl_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	
	state_fio->StateValue(keycode_7);
	state_fio->StateValue(keymode);
	   
	state_fio->StateValue(ctrl_pressed);
	state_fio->StateValue(lshift_pressed);
	state_fio->StateValue(rshift_pressed);
	state_fio->StateValue(shift_pressed);
	state_fio->StateValue(graph_pressed);
	state_fio->StateValue(caps_pressed);
	state_fio->StateValue(kana_pressed);
	state_fio->StateValue(break_pressed);

	state_fio->StateValue(event_keyrepeat);
	   
	state_fio->StateValue(scancode); // After V.4, uint8_t
	state_fio->StateValue(datareg);
	state_fio->StateValue(older_vk);
	   
	state_fio->StateValue(repeat_mode);
	state_fio->StateValue(repeat_time_short);
	state_fio->StateValue(repeat_time_long);
	state_fio->StateValue(repeat_keycode);
	   
#if defined(_FM77AV_VARIANTS)
	state_fio->StateValue(event_key_rtc);
  
	state_fio->StateValue(rtc_yy);
	state_fio->StateValue(rtc_mm);
	state_fio->StateValue(rtc_dd);
	state_fio->StateValue(rtc_dayofweek);
	state_fio->StateValue(rtc_hour);
	state_fio->StateValue(rtc_minute);
	state_fio->StateValue(rtc_sec);

	state_fio->StateValue(rtc_count24h);
	state_fio->StateValue(rtc_ispm);

	state_fio->StateValue(rtc_set);
	state_fio->StateValue(rtc_set_flag);
	state_fio->StateValue(rxrdy_status);
	state_fio->StateValue(key_ack_status);
		
	state_fio->StateValue(did_hidden_message_av_1);
	state_fio->StateValue(event_hidden1_av);

	state_fio->StateValue(cmd_phase);
	state_fio->StateValue(hidden1_ptr);
	state_fio->StateValue(beep_phase);
	//cmd_fifo->save_state((void *)state_fio);
	//data_fifo->save_state((void *)state_fio);
	//cur_time.save_state((void *)state_fio);
#endif
#if defined(_FM77AV_VARIANTS)
	if(!cmd_fifo->process_state(state_fio, loading)) {
		return false;
	}
	if(!data_fifo->process_state(state_fio, loading)) {
		return false;
	}
	if(!cur_time.process_state(state_fio, loading)) {
		return false;
	}
#endif
	if(!key_fifo->process_state(state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(event_int);
	//key_fifo->save_state((void *)state_fio);
	state_fio->StateValue(autokey_backup);
	// Version 5
	state_fio->StateValue(ins_led_status);
	state_fio->StateValue(kana_led_status);
	state_fio->StateValue(caps_led_status);
	// Version 6
	state_fio->StateValue(override_break_key);

	return true;
}

void KEYBOARD::save_state(FILEIO *state_fio)
{
	decl_state(state_fio, false);
	//state_fio->FputUint32_BE(STATE_VERSION);
	//state_fio->FputInt32_BE(this_device_id);
	this->out_debug_log(_T("Save State: KEYBOARD: id=%d ver=%d\n"), this_device_id, STATE_VERSION);
}

bool KEYBOARD::load_state(FILEIO *state_fio)
{
	uint32_t version;
	
	//version = state_fio->FgetUint32_BE();
	//if(this_device_id != state_fio->FgetInt32_BE()) return false;
	//this->out_debug_log(_T("Load State: KEYBOARD: id=%d ver=%d\n"), this_device_id, version);
	bool mb = decl_state(state_fio, true);
	if(!mb) return false;
	return true;
}

   
