/*
 * Common Source code Project -> VM -> FM-7/77AV -> Keyboard
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Licence: GPLv2
 * History : 
 *  Feb 12, 2015 : Initial 
 */

#include "../../fifo.h"
#include "../device.h"
#include "keyboard.h"

enum {
  SIG_FM7KEY_KEY_UP = 0x800,
  SIG_FM7KEY_KEY_DOWN,
  SIG_FM7KEY_READ, // D400 = high , D401 = low
  SIG_FM7KEY_LED_ONOFF, // D40D: Write = OFF / Read = ON
  // D431
  SIG_FM7KEY_PUSH_TO_ENCODER,
};
//

uint16 vk_matrix[0x68] = { // VK
  // +0, +1, +2, +3, +4, +5, +6, +7
  0x00, /* ESC : KANJI or PAUSE*/ VK_KANJI, '1', '2',  '3', '4', '5', '6', // +0x00
  '7', '8',       '9', '0', /* - */ 0xbd, /* ^ */ 0xde, /* \| */ 0xdc, VK_BACK, // +0x08
  VK_TAB, 'Q',       'W', 'E', 'R', 'T', 'Y', 'U', // +0x10
  'I', 'O',       'P', /* @ */ 0xc0, /* [ */ 0xdb, VK_RETURN, 'A', 'S',  //+0x18
  'D', 'F', 'G', 'H', 'J', 'K', 'L', /* SEMICOLON */ 0xbb,  // +0x20
  /* COLON */ 0xba, /* ] */ 0xdd, 'Z', 'X', 'C', 'V', 'B', 'N',  // +0x28
  'M',  /* COMMA */ 0xbc, /* PERIOD */ 0xbe, /* SLASH */ 0xbf, /* BACKSLASH _ */ 0xe2, VK_SPACE, VK_MULTIPLY, VK_DIVIDE // +0x30
  VK_ADD, VK_SUBTRACT, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9, 0x00, VK_NUMPAD4, VK_NUMPAD5, // +0x38
  VK_NUMPAD6, /* NUMPADCOMMA */ 0x00, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, /* NUMPADENTER: Right? */ VK_RETURN, VK_NUMPAD0, VK_DECIMAL, // +0x40
  VK_INSERT, VK_HOME, VK_PRIOR, VK_DELETE, VK_END, VK_UP, VK_NEXT, VK_LEFT, // +0x48
  VK_DOWN, VK_RIGHT, VK_LCONTROL, VK_LSHIFT, VK_RSHIFT, VK_CAPITAL, /* MUHENKAN */ 0x1d, HENKAN, // +0x50
  /* VK_KANA */ 0xf2, 0x00, VK_RCONTROL, 0x00, VK_ESCAPE, VK_F1, VK_F2, VK_F3, // +0x58
  VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, 0xffff // +0x60
};
struct key_tbl_t {
  uint16 phy;
  uint16 code;
};

// Key tables value from XM7.
const struct key_tbl_t ctrl_key[] = {
  {0x0c, 0x1e},
  {0x0d, 0x1c},
  {0x11, 0x11},
  {0x12, 0x17},
  {0x13, 0x05},
  {0x14, 0x12},
  {0x15, 0x14},
  {0x16, 0x19},
  {0x17, 0x15},
  {0x18, 0x09},
  {0x19, 0x0f},
  {0x1a, 0x10},
  {0x1b, 0x00},
  {0x1c, 0x1b},
  {0x1e, 0x01},
  {0x1f, 0x13},
  {0x20, 0x04},
  {0x21, 0x06},
  {0x22, 0x07},
  {0x23, 0x08},
  {0x24, 0x0a},
  {0x25, 0x0b},
  {0x26, 0x0c},
  {0x29, 0x1d},
  {0x2a, 0x1a},
  {0x2b, 0x18},
  {0x2c, 0x03},
  {0x2d, 0x16},
  {0x2e, 0x02},
  {0x2f, 0x0e},
  {0x30, 0x0d},
  {0x34, 0x1f},
  {0xffff, 0xffff}
};

const struct key_tbl_t graph_key[] = {
  {0x5d, 0x101},
  {0x5e, 0x102},
  {0x5f, 0x103},
  {0x60, 0x104},
  {0x61, 0x105},
  {0x62, 0x106},
  {0x63, 0x107},
  {0x64, 0x108},
  {0x65, 0x109},
  {0x66, 0x10a},

  {0x01, 0x1b},
  {0x02, 0xf9},
  {0x03, 0xfa},
  {0x04, 0xfb},
  {0x05, 0xfc},
  {0x06, 0xf2},
  {0x07, 0xf3},
  {0x08, 0xf4},
  {0x09, 0xf5},
  {0x0a, 0xf6},
  {0x0b, 0xf7},
  {0x0c, 0x8c},
  {0x0d, 0x8b},
  {0x0e, 0xf1},
  {0x0f, 0x08},

  {0x10, 0x09},
  {0x11, 0xfd},
  {0x12, 0xf8},
  {0x13, 0xe4},
  {0x14, 0xe5},
  {0x15, 0x9c},
  {0x16, 0x9d},
  {0x17, 0xf0},
  {0x18, 0xe8},
  {0x19, 0xe9},
  {0x1a, 0x8d},
  {0x1b, 0x8a},
  {0x1c, 0xed},
  {0x1d, 0x0d},
  {0x1e, 0x95},
  {0x1f, 0x96},

};
/*
 * I/O API (subio)
 */
// 0xd431 : Read
uint8 KEYBOARD::read_data_reg(void)
{
  if(rxrdy->read_signal(0) != 0) {
    if(!data_fifo->empty()) {
      datareg = data_fifo->read() & 0xff;
    }
    rxrdy->write_signal(0x01, 0x00, 0x01);
  }
  return datareg;
}

// 0xd432
uint8 KEYBOARD::read_stat_reg(void)
{
  uint8 data = 0x7f;
  if(rxrdy->read_signal(0x00) != 0) {
    data |= 0x80;
  }
  // Digityze : bit0 = '0' when waiting,
}

// 0xd400(SUB) or 0xfd00(MAIN)
uint8 KEYBOARD::get_keycode_high(void)
{
  uint8 data = 0x00;
  if((keycode_7 & 0x0100) != 0) data |= 0x01;
  return data;
}

// 0xd401(SUB) or 0xfd01(MAIN)
uint8 KEYBOARD::get_keycode_low(void)
{
  uint8 data = keycode_7 & 0xff;
  maincpu->write_signal(SIG_CPU_IRQ, 0, 1);
  subcpu->write_signal(SIG_CPU_FIRQ, 0, 1);
  return data;
}



// UI Handler. 
void KEYBOARD::key_up(uint32 vk)
{
  vk = vk & 0x1ff;
  if(event_ids[vk] >= 0){
    cancel_event(this, event_ids[vk]);
    event_ids[vk] = -1;
  }
  if(this->isModifiers(vk)) {
        set_modifiers(vk, false);
  }
  if(isBreakKey(vk)) {
    break_line->write_signal(0x01, 0, 1);
    maincpu->write_signal(SIG_CPU_FIRQ, 0, 1);
  }
  key_pressed_flag[vk] = false; 
}

void KEYBOARD::key_down(uint32 vk)
{
  double usec = (double)repeat_time_long * 1000.0;
  uint32 code_7;
  vk = vk & 0x1ff;
  key_pressed_flag[vk] = true;
  if(!this->isModifiers(vk)) {
    code_7 = vk2fmkeycode(vk);
    if(code_7 < 0x200) keycode_7 = code_7;
    maincpu->write_signal(SIG_CPU_IRQ, 1, 1);
    subcpu->write_signal(SIG_CPU_FIRQ, 1, 1);
    if(repeat_mode) register_event(this, ID_KEYBOARD_AUTOREPEAT + vk, usec, false, &event_ids[vk]);
  } else {
    set_modifiers(vk, true);
  }
  if(isBreakKey(vk)) {
    break_line->write_signal(0x01, 1, 1);
    maincpu->write_signal(SIG_CPU_FIRQ, 1, 1);
  }
}

void KEYBOARD::event_callback(int event_id, int err)
{
  if((event_id >= ID_KEYBOARD_AUTOREPEAT) && (event_id <= (ID_KEYBOARD_AUTOREPEAT + 0x1ff))){
      // Key repeat.
      uint32 vk = event_id - ID_KEYBOARD_AUTOREPEAT;
      double usec = (double)repeat_time_short * 1000.0;
      uint16 code_7;
      key_pressed_flag[vk] = true;
      if(!this->isModifiers(vk)) {
	code_7 = vk2fmkeycode(vk);
	if(code_7 < 0x200) keycode_7 = code_7;
	maincpu->write_signal(SIG_CPU_IRQ, 1, 1);
	subcpu->write_signal(SIG_CPU_FIRQ, 1, 1);
	if(repeat_mode) register_event(this, ID_KEYBOARD_AUTOREPEAT + vk, usec, false, &event_ids[vk]);
      }
  }
}

// Commands
void KEYBOARD::reset_keyboard(void)
{
  repeat_time_short = 70; // mS
  repeat_time_long = 700; // mS
  repeat_on = true;
  caps_led = false;
  kana_led = false;
  ins_led = false;
  key_code = 0x00;

  lshift = false;
  rshift = false;
  ctrl   = false;
  graph = false;
  cmd_fifo->clear();
  data_fifo->clear();
  datareg = 0x00;
  rxrdy->write_signal(0x01, 0x00, 0x01);
}
  
void KEYBOARD::set_mode(void)
{
  int count = cmd_fifo->count();
  int cmd;
  if(count < 2) return;
  cmd = cmd_fifo->read();
  key_format = cmd_fifo->read();
  reset_keyboard();
}

void KEYBOARD::get_mode(void)
{
  int cmd;
  int dummy;
  cmd = cmd_fifo->read();
  if(data_fifo->full()) {
    dummy = data_fifo->read();
  }
  data_fifo->write(key_format);
  rxrdy->write_signal(0x01, 0x01, 0x01);
}



  
void KEYBOARD::write_signal(int id, uint32 data, uint32 mask)
{
  
  if(id == SIG_FM7KEY_PUSH_TO_ENCODER) {
    /*
     * I refered XM7's sourcecode : VM/keyboard.c act of key-encoder.
     * Thanks to Ryu.Takegami and PI.
     */
    int count;
    
    if(cmd_fifo->full()) {
      cmd_fifo->clear();
    }
    if(cmd_fifo->empty()) {
      cmd_phase = data & 0xff;
    }
    
    cmd_fifo->write(data & 0xff);
    count = cmd_fifo->count();
    
    switch(cmd_phase) {
    case 0: // Set mode
      if(count >= 2) set_mode();
      break;
    case 1: // Get mode
      get_mode();
      break;
    case 2: // Set LED Phase
      if(count >= 2) set_leds();
      break;
    case 3: // Get LED Phase
      get_leds();
      break;
    case 4:
      if(count >= 2) set_repeat_type();
      break;
    case 5:
      if(count >= 3) set_repeat_time();
      break;
    case 0x80: // Communicate to/from RTC.
      if(count == 1) {
	rtc_set = false;
      }
      if(count == 2) {
	if((data & 0xff) == 0) { // Get
	  get_rtc();
	} else if((data & 0xff) == 1) { // Set
	  rtc_set_flag = true;
	} else { // Illegal
	  cmd_fifo->clear(); 
	}
      }
      if(rtc_set_flag) {
	if(count >= 9) {
	  set_rtc();
	}
      }
      break;
    case 0x81: // Digitize.
      if(count >= 2) do_digitize(); // WILL Implement?
      break;
    case 0x82:
      if(count >= 2) set_screen_mode();
      break;
    case 0x83:
      get_screen_mode();
      break;
    case 0x84:
      if(count >= 2) set_brightness();
      break;
    default:
      cmd_fifo->clear();
      break;
    }
  }
}
