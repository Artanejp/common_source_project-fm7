/*
 * Common source code project -> FM-7 -> Display
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *  Feb 10, 2015 : Initial.
 */

#include "fm7_display.h"

extern "C" {

  extern void initvramtbl_4096_vec(void);
  extern void detachvramtbl_4096_vec(void);

  extern void CreateVirtualVram8_Line(uint8 *src, uint32 *p, int ybegin, uint32 *pal);
  extern void CreateVirtualVram8_WindowedLine(uint8 *vram_1, uint8 *vram_w, Uint32 *p, int ybegin, int xbegin, int xend, uint32 *pal);
}

DISPLAY::DISPLAY()
{
  initvramtbl_4096_vec();
}

void DISPLAY::getvram(uint32 *pvram, uint32 pitch)
{
  int y;
  int height = (display_mode == DISPLAY_MODE_8_400L)? 400 : 200;
  Uint32 *p;
  Uint32 offset = 0x4000;
  if(display_mode == DISPLAY_MODE_8_400L) offset = 0x8000;
  
  if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_8_200L)) {
    for(y = 0; y < height; y++) {
      p = &pvram[y * pitch];
      if(((y < window_low) && (y > window_high)) || (!window_opened)) {
	CreateVirtualVram8_Line(p, y, digital_palette, offset, multimode_dispmask);
      } else {
	CreateVirtualVram8_WindowedLine(p, y, window_xbegin, window_xbegin, digital_palette, offset, multimode_dispmask);
      }
    }
  } else if(display_mode == DISPLAY_MODE_4096) {
    for(y = 0; y < height; y++) {
      p = &pvram[y * pitch];
      if(((y < window_low) && (y > window_high)) || (!window_opened)) {
	CreateVirtualVram4096_Line(p, y, analog_palette, multimode_dispmask);
      } else {
	CreateVirtualVram4096_WindowedLine(p, y, window_xbegin, window_xbegin, analog_palette, multimode_dispmask);
      }
    }
  } else { // 256k
    for(y = 0; y < height; y++) {
      p = &pvram[y * pitch];
      if(((y < window_low) && (y > window_high)) || (!window_opened)) {
	CreateVirtualVram256k_Line(p, y, multimode_dispmask);
      } else {
	CreateVirtualVram256k_WindowedLine(p, y, window_xbegin, window_xbegin, multimode_dispmask);
      }
    }
  }      
}

void set_multimode(uint8 val)
{
  multimode_accessmask = val & 0x0f;
  multimode_dispmask = (val & 0xf0) >> 4;
}

uint8 get_multimode(void)
{
  uint8 val = multimode_acdcessmask & 0x0f;
  val |= ((multimode_dispmask << 4) & 0xf0);
  return val;
}

void set_dpalette(uint32 addr, uint8 val)
{
  addr &= 7;

  dpalet[addr][0] = ((val & 0x01) != 0x00)? 0x01 : 0x00;
  dpalet[addr][1] = ((val & 0x02) != 0x00)? 0x01 : 0x00;
  dpalet[addr][2] = ((val & 0x04) != 0x00)? 0x01 : 0x00;
  dpalet[addr][3] = ((val & 0x08) != 0x00)? 0x01 : 0x00;
}

uint8 get_dpalette(uint32 addr)
{
  uint8 data;
  addr = addr & 7;

  data = (dpalet[addr][0] & 0x01) | ((dpalet[addr][1] & 0x01) << 1) |
         ((dpalet[addr][2] & 0x01) << 2) | ((dpalet[addr][3] & 0x01) << 3);
  return data;
}
