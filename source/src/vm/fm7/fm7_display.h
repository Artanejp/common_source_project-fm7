/*
 * Common source code project -> FM-7 -> Display
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *  Feb 10, 2015 : Initial.
 */

#ifndef _CSP_FM7_DISPLAY_H
#define _CSP_FM7_DISPLAY_H

#include "../device.h"
#include "../device.h"
#include "../mc6809.h"
#include "fm7_common.h"


class DEVICE;
class MC6809;

class DISPLAY: public DEVICE
{
 protected:
	EMU *p_emu;
	VM *p_vm;

	int clr_count;
   
	void go_subcpu();
	void halt_subcpu();
   
	void do_nmi(bool);
	void do_irq(bool);
	void do_firq(bool);
	void set_multimode(uint8 val);
	uint8 get_multimode(void);
	uint8 get_cpuaccessmask(void);
	void set_dpalette(uint32 addr, uint8 val);
	uint8 get_dpalette(uint32 addr);
	void enter_display(void);
	void leave_display(void);
	void halt_subsystem(void);
	void restart_subsystem(void);
	void set_crtflag(void);
	void reset_crtflag(void);
	uint8 acknowledge_irq(void);
	uint8 beep(void);
	uint8 attention_irq(void);
	void set_cyclesteal(uint8 val);
	uint8 set_vramaccess(void);
	void reset_vramaccess(void);
	uint8 reset_subbusy(void);
	void set_subbusy(void);
	void reset_cpuonly(void);
   
#if defined(_FM77AV_VARIANTS)
	void alu_write_cmdreg(uint32 val);
	void alu_write_logical_color(uint8 val);
	void alu_write_mask_reg(uint8 val);
	void alu_write_cmpdata_reg(int addr, uint8 val);
	void alu_write_disable_reg(uint8 val);
	void alu_write_tilepaint_data(uint32 addr, uint8 val);
	void alu_write_offsetreg_hi(uint8 val);
	void alu_write_offsetreg_lo(uint8 val);
	void alu_write_linepattern_hi(uint8 val);
	void alu_write_linepattern_lo(uint8 val);
	void alu_write_line_position(int addr, uint8 val);
	
	uint8 get_miscreg(void);
	void set_miscreg(uint8 val);
	void set_monitor_bank(uint8 var);
	void set_apalette_index_hi(uint8 val);
	void set_apalette_index_lo(uint8 val);
	void set_apalette_b(uint8 val);
	void set_apalette_r(uint8 val);
	void set_apalette_g(uint8 val);
	void calc_apalette(uint16 idx);

#endif // _FM77AV_VARIANTS

 private:
	bool sub_busy;
	bool firq_mask;
	bool vblank;
	bool vsync;
	bool hblank;
	bool cancel_request;
	bool key_firq_req;
	bool clock_fast;
	int display_mode;
	bool halt_flag;
	int active_page;
	uint32 prev_clock;
	uint32 frame_skip_count;
	// Event handler
	int nmi_event_id;

#if defined(_FM77AV_VARIANTS)
	uint32 displine;
	int vblank_count;
	bool subcpu_resetreq;
	bool power_on_reset;
	
	int hblank_event_id;
	int hdisp_event_id;
	int vsync_event_id;
	int vstart_event_id;
#endif	
	DEVICE *ins_led;
	DEVICE *kana_led;
	DEVICE *caps_led;
#if defined(_FM77_VARIANTS)
# if defined(_FM77L4)
	bool mode400line;
	bool stat_400linecard;
# endif	
	bool kanjisub;
	pair kanjiaddr;
#elif defined(_FM77AV_VARIANTS)
	bool kanjisub;
	pair kanjiaddr;
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)|| \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
	bool mode400line;
	bool mode256k;
# endif
	bool mode320;
	int display_page;
	int cgrom_bank;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)|| \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
	int vram_bank;
	bool vram_page;
   
	uint8 console_ram_bank;

	uint8 vram_active_block;
	uint8 vram_display_block;
	
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint16 window_low;
	uint16 window_high;
	uint16 window_xbegin;
	uint16 window_xend;

	bool window_opened;
# endif	
#endif	
	bool nmi_enable;
	bool diag_load_subrom_a;
	bool diag_load_subrom_b;
	bool diag_load_subrom_cg;
#endif
	bool diag_load_subrom_c;

	scrntype dpalette_pixel[8];
	uint8 dpalette_data[8];
#if defined(_FM77AV_VARIANTS)
	pair apalette_index;
	uint8 analog_palette_r[4096];
	uint8 analog_palette_g[4096];
	uint8 analog_palette_b[4096];
	scrntype analog_palette_pixel[4096];
#endif // FM77AV etc...
#if defined(_FM77AV_VARIANTS)
	uint8 io_w_latch[0x40];
#elif !defined(_FM77AV40EX) && !defined(_FM77AV40SX)
	uint8 io_w_latch[0x10];
#else
	uint8 io_w_latch[0x100];
#endif
	uint8 multimode_accessmask;
	uint8 multimode_dispmask;
   
	uint32 offset_point;
	pair tmp_offset_point[2];
	bool offset_changed[2];
	bool offset_77av;
   
#if defined(_FM77AV_VARIANTS)
	uint8 subrom_bank;
	uint8 subrom_bank_using;
	uint32 offset_point_bank1;
	uint32 offset_point_bak;
	uint32 offset_point_bank1_bak;
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	bool monitor_ram;
	bool ram_protect;
# endif
#endif	

#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint8 gvram[0x8000 * 6];
	uint8 gvram_shadow[0x8000 * 6];
#elif defined(_FM77AV40)
	uint8 gvram[0x2000 * 18];
	uint8 gvram_shadow[0x2000 * 18];
#elif defined(_FM77AV_VARIANTS)
	uint8 gvram[0x2000 * 12];
	uint8 gvram_shadow[0x2000 * 12];
#else
	uint8 gvram[0x4000 * 3];
#endif
	uint8 console_ram[0x1000];
	uint8 work_ram[0x380];
	uint8 shared_ram[0x80];
   
	uint8 subsys_c[0x2800];
#if defined(_FM77AV_VARIANTS)
	uint8 subsys_a[0x2000];
	uint8 subsys_b[0x2000];
	uint8 subsys_cg[0x2000];
	uint8 submem_hidden[0x300];
#endif

	bool crt_flag;
	bool vram_accessflag;
	bool is_cyclesteal;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint8 submem_cgram[0x4000];
	uint8 submem_console_av40[0x2000];
	uint8 subsys_ram[0x2000];
	uint8 cgram_bank;
	bool kanji_level2;
	DEVICE *kanjiclass1;
	DEVICE *kanjiclass2;
#elif defined(_FM77_VARIANTS)
	DEVICE *kanjiclass1;
#endif
	
#if defined(_FM77AV_VARIANTS)
	bool use_alu;
	DEVICE *alu;
	bool vram_wrote_shadow;
	bool vram_wrote_table[411];
	bool vram_draw_table[411];
#endif	
	DEVICE *mainio;
	DEVICE *subcpu;
	DEVICE *keyboard;
	bool vram_wrote;
	inline void GETVRAM_8_200L(int yoff, scrntype *p, uint32 rgbmask, bool window_inv);
	inline void GETVRAM_4096(int yoff, scrntype *p, uint32 rgbmask, bool window_inv);
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	inline void GETVRAM_8_400L(int yoff, scrntype *p, uint32 mask, bool window_inv);
	inline void GETVRAM_256k(int yoff, scrntype *p, uint32 mask);
#endif   
	uint8 read_vram_l4_400l(uint32 addr, uint32 offset);
	uint8 read_mmio(uint32 addr);
	
	uint32 read_vram_data8(uint32 addr);
	uint32 read_data8_main(uint32 addr);

	void write_vram_data8(uint32 addr, uint8 data);
	void write_data8_main(uint32 addr, uint8 data);

	
	void write_vram_l4_400l(uint32 addr, uint32 offset, uint32 data);
	void write_mmio(uint32 addr, uint32 data);
   
	uint32 read_bios(const char *name, uint8 *ptr, uint32 size);
  public:
	DISPLAY(VM *parent_vm, EMU *parent_emu);
	~DISPLAY();
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_signal(int id);
	
	uint32 read_data8(uint32 addr);
	void write_data8(uint32 addr, uint32 data);
	
	uint32 read_dma_data8(uint32 addr);
	void write_dma_data8(uint32 addr, uint32 data);
	
	void initialize();
	void release();
	void reset();
	void update_config();
	
	void draw_screen();
	void event_frame();
	void event_vline(int v, int clock);
	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);

	int get_screen_mode(void) {
		return display_mode;
	}
	uint32 read_io8(uint32 addr) { // This is only for debug.
#if defined(_FM77AV40SX) || defined(_FM77AV40EX)
		return io_w_latch[addr & 0xff];
#elif defined(_FM77AV_VARIANTS) // Really?
		return io_w_latch[addr & 0x3f];
#else
		return io_w_latch[addr & 0x0f];
#endif
	}
   
	void set_context_kanjiclass1(DEVICE *p)	{
#if defined(_FM77_VARIANTS) || \
	defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
		kanjiclass1 = p;
#endif
	}
	void set_context_kanjiclass2(DEVICE *p)	{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)|| \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
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
#if defined(_FM77AV_VARIANTS)
	void set_context_alu(DEVICE *p) {
		alu = p;
	}
#endif
};  
#endif //  _CSP_FM7_DISPLAY_H
