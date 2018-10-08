/*
 * FM-7 Keyboard [fm7_keyboard.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Mar 11, 2015 : Initial
 *
 */

#ifndef _VM_FM7_KEYBOARD_H_
#define _VM_FM7_KEYBOARD_H_

#include "../device.h"
#include "../memory.h"
#include "../../fileio.h"

#include "fm7_common.h"

#if defined(_FM77AV_VARIANTS)  
class BEEP;
#endif
class KEYBOARD : public DEVICE {
 protected:
	uint8_t get_keycode_high(void);
	uint8_t get_keycode_low(void);
	void turn_on_ins_led(void);
	void turn_off_ins_led(void);
	
#if defined(_FM77AV_VARIANTS)  
	outputs_t rxrdy;
	outputs_t key_ack;
#endif
	outputs_t break_line;
	outputs_t int_line;

	uint32_t keycode_7;
	int keymode;
private:
	bool ctrl_pressed; 
	bool lshift_pressed; 
	bool rshift_pressed; 
	bool shift_pressed; 
	bool graph_pressed;
	bool caps_pressed;
	bool kana_pressed;
	bool break_pressed;

	bool ins_led_status;
	bool kana_led_status;
	bool caps_led_status;
	uint8_t read_data_reg(void);
	uint8_t read_stat_reg(void);
	
	int event_keyrepeat;
	int event_key_rtc;
	
	uint8_t scancode;
	uint8_t autokey_backup;
	uint8_t repeat_keycode;
	
	uint8_t datareg;
	uint32_t older_vk;
	bool override_break_key;
	
#if defined(_FM77AV_VARIANTS)
	dll_cur_time_t cur_time; 
	uint8_t rtc_yy;
	uint8_t rtc_mm;
	uint8_t rtc_dd;
	bool  rtc_count24h;
	uint8_t rtc_dayofweek;
	bool  rtc_ispm;
	uint8_t rtc_hour;
	uint8_t rtc_minute;
	uint8_t rtc_sec;
	bool rtc_set;
	bool rtc_set_flag;
	bool rxrdy_status;
	bool key_ack_status;
	int cmd_phase;
	FIFO *cmd_fifo;
	FIFO *data_fifo;
	int event_hidden1_av;
	uint16_t hidden1_ptr;

	DEVICE *beep;
	bool did_hidden_message_av_1;
	int beep_phase;
#endif
	FIFO *key_fifo;
	int event_int;
   
	uint16_t vk2scancode(uint32_t vk);
	bool isModifier(uint8_t sc);
	void set_modifiers(uint8_t sc, bool flag);
	uint16_t scan2fmkeycode(uint8_t sc);
	void do_repeatkey(uint8_t sc);
	void reset_unchange_mode(void);
	void key_down_main(bool repeat_auto_key);
	void key_up_main(uint8_t bak_scancode);
   
#if defined(_FM77AV_VARIANTS)   
	void set_mode(void);
	void get_mode(void);
	void set_leds(void);
	void get_leds(void);
	void set_repeat_type(void);
	void set_repeat_time(void);
	void set_rtc(void);
	void get_rtc(void);
	void rtc_count(void);
	void adjust_rtc(void);
	void do_digitize(void) {};
	void set_screen_mode(void) {};
	void get_screen_mode(void) {};
	void set_brightness(void) {};
#endif
	bool repeat_mode;
	int repeat_time_short;
	int repeat_time_long;
	
 public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU *parent_emu);
	~KEYBOARD();
   
	void key_up(uint32_t vk);
	void key_down(uint32_t vk);
	bool get_caps_locked()
	{
		return caps_pressed;
	}
	bool get_kana_locked()
	{
		return kana_pressed;
	}
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);

	uint32_t read_data8(uint32_t addr);
	void write_data8(uint32_t addr, uint32_t data);
	void reset(void);
	void release(void);
	bool decl_state(FILEIO *state_fio, bool loading);
	void save_state(FILEIO *f);
	bool load_state(FILEIO *f);
	void set_context_rxrdy(DEVICE *p, int id, uint32_t mask) {
#if defined(_FM77AV_VARIANTS)  
		register_output_signal(&rxrdy, p, id, mask);
#endif
	}
	void set_context_key_ack(DEVICE *p, int id, uint32_t mask) {
#if defined(_FM77AV_VARIANTS)  
		register_output_signal(&key_ack, p, id, mask);
#endif
	}
	void set_context_break_line(DEVICE *p, int id, uint32_t mask) {
		register_output_signal(&break_line, p, id, mask);
	}
	void set_context_int_line(DEVICE *p, int id, uint32_t mask) {
		register_output_signal(&int_line, p, id, mask);
	}
#if defined(_FM77AV_VARIANTS)  
	void set_context_beep(DEVICE *p) {
		beep = p;
	}
#endif
};


	 
#endif
