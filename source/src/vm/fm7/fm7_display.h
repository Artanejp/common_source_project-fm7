/*
 * Common source code project -> FM-7 -> Display
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *  Feb 10, 2015 : Initial.
 */

#ifndef _CSP_FM7_DISPLAY_H
#define _CSP_FM7_DISPLAY_H

#include "../device.h"
#include "../memory.h"
#include "../mc6809.h"
#include "fm7_common.h"


class DEVICE;
class MEMORY;
class MC6809;

class DISPLAY: public MEMORY
{
 private:
	MC6809 *subcpu;

	uint32  disp_mode;
	uint8 digital_palette[8][4];
	uint8 multimode_dispmask;
	uint8 multimode_accessmask;
	DEVICE *ins_led;
	DEVICE *kana_led;
#if defined(_FM77AV_VARIANTS) 
	uint8 analog_palette[4096][3];
#endif // FM77AV etc...

	uint8 *vram_ptr;
	uint8 *tvram_ptr;
	uint32 offset_point;
	bool offset_77av;
	uint8 gvram[0xc000];
#if defined(_FM77AV_VARIANTS)
	uint8 gvram_1[0xc000];
# if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
	uint8 gvram_2[0xc000];
# endif
#endif
	uint8 console_ram[0x1000];
	uint8 work_ram[0x380];
	uint8 shared_ram;

	uint8 subsys_c[0x2800];
#if defined(_FM77AV_VARIANTS)
	
	uint8 subsys_a[0x2000];

#if defined(_FM77L4) || defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
	uint32 kanji1_addr;
	MEMORY *kanjiclass1;
 #if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)	
	bool kanji_level2;
	uint32 kanji2_addr;
	MEMORY *kanjiclass2;
 #endif
#endif
	DEVICE *mainio;
	DEVICE *subcpu;
	DEVICE *keyboard;
 public:
	DISPLAY(VM *parent_vm, EMU *parent_emu);
	~DISPLAY();
	
	uint32 read_data8(uint32 addr);
	void set_context_kanjiclass1(MEMORY *p)	{
#if defined(_FM77L4) || defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
		kanji1_addr = 0;
		kanjiclass1 = p;
#endif
	}
	void set_context_kanjiclass2(MEMORY *p)	{
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
		kanji2_addr = 0;
		kanjiclass2 = p;
		if(p != NULL) kanji_level2 = true;
#endif
	}
	void set_context_mainio(DEVICE *p) {
		mainio = p;
	}
	void set_context_keyboard(DEVICE *p) {
		keyboard = p;
	}
	void set_context_subcpu(DEVICE *p) {
		subcpu = p;
	}
};  
#endif //  _CSP_FM7_DISPLAY_H
