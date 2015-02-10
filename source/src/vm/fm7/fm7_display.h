/*
 * Common source code project -> FM-7 -> Display
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *  Feb 10, 2015 : Initial.
 */

#ifndef _CSP_FM7_DISPLAY_H
#define _CSP_FM7_DISPLAY_H

#include "../mc6809.h"
#include "./kanjirom.h"

enum {
  SIG_DISPLAY_VBLANK = 0x4000,
  SIG_DISPLAY_HBLANK,
  SIG_DISPLAY_DIGITAL_PALETTE,
  SIG_DISPLAY_ANALOG_PALETTE,
  SIG_DISPLAY_CHANGE_MODE
};
enum {
  DISPLAY_MODE_8_200L = 0,
  DISPLAY_MODE_8_400L,
  DISPLAY_MODE_8_200L_TEXT,
  DISPLAY_MODE_8_400L_TEXT,
  DISPLAY_MODE_4096,
  DISPLAY_MODE_256K
};

enum {
  DISPLAY_ADDR_MULTIPAGE = 0,
  DISPLAY_ADDR_OFFSET_H,
  DISPLAY_ADDR_OFFSET_L,
  DISPLAY_ADDR_DPALETTE,
  DISPLAY_ADDR_APALETTE_B = 0x1000,
  DISPLAY_ADDR_APALETTE_R = 0x2000,
  DISPLAY_ADDR_APALETTE_G = 0x3000
  
};

class MC6809;

class DISPLAY: public DEVICE
{
 private:
	MC6809 *subcpu;
	
  
	uint32  disp_mode;
	uint8 digital_palette[8];
	uint8 multimode_dispmask;
	uint8 multimode_accessmask;
#if defined(_FM77AV) || defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
	uint8 analog_palette_r[4096];
	uint8 analog_palette_g[4096];
	uint8 analog_palette_b[4096];
#endif // FM77AV etc...

	uint8 *vram_ptr;
	uint8 *tvram_ptr;

 public:
	uint32 read_data8(uint32 addr){
	  
};  
#endif //  _CSP_FM7_DISPLAY_H
