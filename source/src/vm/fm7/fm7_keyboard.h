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
   uint8 read_data_reg(void);
   uint8 read_stat_reg(void);
   uint8 get_keycode_high(void);
   uint8 get_keycode_low(void);
   void turn_on_ins_led(void);
   void turn_off_ins_led(void);
   
   DEVICE *caps_led;
   DEVICE *kana_led;
   DEVICE *ins_led;
   
   DEVICE *rxrdy;
   DEVICE *key_ack;

   DEVICE *maincpu;
   DEVICE *subcpu;
   DEVICE *z80cpu;
   
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
   
   int event_ids[0x70];
   bool key_pressed_flag[0x70];
   
   uint8 datareg;

   uint16 vk2scancode(uint32 vk);
   bool isModifier(uint16 scancode);
   void set_modifiers(uint16 scancode, bool flag);
   uint16 scan2fmkeycode(uint16 scancode);
   void do_repeatkey(uint16 scancode);
   void reset_keyboard(void);
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
   
   void set_context_display(DEVICE *p) {
      display = p;
   }
   void set_context_maincpu(DEVICE *p) {
      maincpu = p;
   }
   void set_context_subcpu(DEVICE *p) {
      subcpu = p;
   }
   void set_context_z80cpu(DEVICE *p) {
      subcpu = p;
   }
   void set_context_mainio(DEVICE *p) {
      mainio = p;
   }
   void set_context_caps_led(DEVICE *p) {
      caps_led = p;
   }
   void set_context_kana_led(DEVICE *p) {
      kana_led = p;
   }
   void set_context_ins_led(DEVICE *p) {
      ins_led = p;
   }
   
};


	 
#endif