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

#include "fm7_common.h"


class KEYBOARD : public DEVICE {
 protected:
	VM* p_vm;
	EMU* p_emu;
	uint8 get_keycode_high(void);
	uint8 get_keycode_low(void);
	void turn_on_ins_led(void);
	void turn_off_ins_led(void);
	
	outputs_t caps_led;
	outputs_t kana_led;
	outputs_t ins_led;
	
	outputs_t rxrdy;
	outputs_t key_ack;
	outputs_t break_line;
	
	uint32 keycode_7;
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
	uint8 read_data_reg(void);
	uint8 read_stat_reg(void);
	
	int event_ids[0x70];
	bool key_pressed_flag[0x70];
   
	uint8 datareg;
	uint32 older_vk;
   
	uint16 vk2scancode(uint32 vk);
	bool isModifier(uint16 scancode);
	void set_modifiers(uint16 scancode, bool flag);
	uint16 scan2fmkeycode(uint16 scancode);
	void do_repeatkey(uint16 scancode);
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
	void rtc_adjust(void);
#endif
	bool repeat_mode;
	int repeat_time_short;
	int repeat_time_long;
	FIFO *cmd_fifo;
	FIFO *data_fifo;
	
	DEVICE *display;
	DEVICE *mainio;
 public:
	KEYBOARD(VM *parent_vm, EMU *parent_emu);
	~KEYBOARD();
   
	void key_up(uint32 vk);
	void key_down(uint32 vk);
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_data8(uint32 addr);
	void write_data8(uint32 addr, uint32 data);
	void reset(void);
	
	void set_context_display(DEVICE *p) {
		display = p;
	}
	void set_context_mainio(DEVICE *p) {
		mainio = p;
	}
	void set_context_rxrdy(DEVICE *p, int id, uint32 mask) {
		register_output_signal(&rxrdy, p, id, mask);
	}
	void set_context_key_ack(DEVICE *p, int id, uint32 mask) {
		register_output_signal(&key_ack, p, id, mask);
	}
	void set_context_caps_led(DEVICE *p, int id, uint32 mask) {
		register_output_signal(&caps_led, p, id, mask);
	}
	void set_context_kana_led(DEVICE *p, int id, uint32 mask) {
		register_output_signal(&kana_led, p, id, mask);
	}
	void set_context_ins_led(DEVICE *p, int id, uint32 mask) {
		register_output_signal(&ins_led, p, id, mask);
	}
	void set_context_break_line(DEVICE *p, int id, uint32 mask) {
		register_output_signal(&break_line, p, id, mask);
	}
   
};


	 
#endif
