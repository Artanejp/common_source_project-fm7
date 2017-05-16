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


#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
#define __FM7_GVRAM_PAG_SIZE (0x2000 * 24)
#elif defined(_FM77AV40)
#define __FM7_GVRAM_PAG_SIZE (0x2000 * 18)
#elif defined(_FM77AV_VARIANTS)
#define __FM7_GVRAM_PAG_SIZE (0x2000 * 12)
#else
#define __FM7_GVRAM_PAG_SIZE (0x4000 * 3)
#endif

class DEVICE;
class MC6809;
class DISPLAY: public DEVICE
{
private:

	uint16_t bit_trans_table_0[256][8];
	uint16_t bit_trans_table_1[256][8];
	uint16_t bit_trans_table_2[256][8];
	uint16_t bit_trans_table_3[256][8];
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint16_t bit_trans_table_4[256][8];
	uint16_t bit_trans_table_5[256][8];
#endif
protected:
	EMU *p_emu;
	VM *p_vm;

	uint32_t (DISPLAY::*read_cpu_func_table[512])(uint32_t);
	uint32_t (DISPLAY::*read_dma_func_table[512])(uint32_t);
	void (DISPLAY::*write_cpu_func_table[512])(uint32_t, uint8_t);
	void (DISPLAY::*write_dma_func_table[512])(uint32_t, uint8_t);
	
	int clr_count;
	bool screen_update_flag;
	bool crt_flag_bak;
	
	void go_subcpu();
	void halt_subcpu();
	void setup_display_mode(void);
   
	void do_nmi(bool);
	void do_irq(bool);
	void do_firq(bool);
	void set_multimode(uint8_t val);
	uint8_t get_multimode(void);
	uint8_t get_cpuaccessmask(void);
	void set_dpalette(uint32_t addr, uint8_t val);
	uint8_t get_dpalette(uint32_t addr);
	void enter_display(void);
	void leave_display(void);
	void halt_subsystem(void);
	void restart_subsystem(void);
	void set_crtflag(void);
	void reset_crtflag(void);
	uint8_t acknowledge_irq(void);
	uint8_t beep(void);
	uint8_t attention_irq(void);
	void set_cyclesteal(uint8_t val);
	uint8_t set_vramaccess(void);
	void reset_vramaccess(void);
	uint8_t reset_subbusy(void);
	void set_subbusy(void);
	void reset_cpuonly(void);
   
#if defined(_FM77AV_VARIANTS)
	void alu_write_cmdreg(uint32_t val);
	void alu_write_logical_color(uint8_t val);
	void alu_write_mask_reg(uint8_t val);
	void alu_write_cmpdata_reg(int addr, uint8_t val);
	void alu_write_disable_reg(uint8_t val);
	void alu_write_tilepaint_data(uint32_t addr, uint8_t val);
	void alu_write_offsetreg_hi(uint8_t val);
	void alu_write_offsetreg_lo(uint8_t val);
	void alu_write_linepattern_hi(uint8_t val);
	void alu_write_linepattern_lo(uint8_t val);
	void alu_write_line_position(int addr, uint8_t val);
	
	uint8_t get_miscreg(void);
	void set_miscreg(uint8_t val);
	void set_monitor_bank(uint8_t var);
	void set_apalette_index_hi(uint8_t val);
	void set_apalette_index_lo(uint8_t val);
	void set_apalette_b(uint8_t val);
	void set_apalette_r(uint8_t val);
	void set_apalette_g(uint8_t val);
	void calc_apalette(uint16_t idx);

#endif // _FM77AV_VARIANTS
	
	void copy_vram_all();
	void copy_vram_per_line(int begin, int end);
	void copy_vram_blank_area(void);

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
	int active_page; // GVRAM is Double-Buffer.
	uint32_t page_offset;
	uint32_t page_mask;
	uint32_t pagemod_mask;
	
	uint32_t prev_clock;
	uint8_t frame_skip_count_draw;
	uint8_t frame_skip_count_transfer;
	bool need_transfer_line;
	bool palette_changed;
	
	// Event handler
	int nmi_event_id;

#if defined(_FM77AV_VARIANTS) || defined(_FM77L4)
	int hblank_event_id;
	int hdisp_event_id;
	int vsync_event_id;
	int vstart_event_id;

	int vblank_count;
#endif
#if defined(_FM77AV_VARIANTS)
	bool subcpu_resetreq;
	bool power_on_reset;
#endif	
	uint32_t displine;
	
	DEVICE *ins_led;
	DEVICE *kana_led;
	DEVICE *caps_led;
#if defined(_FM77_VARIANTS)
# if defined(_FM77L4)
	bool mode400line;
	bool stat_400linecard;
# endif	
	bool kanjisub;
	pair_t kanjiaddr;
#elif defined(_FM77AV_VARIANTS)
	bool kanjisub;
	pair_t kanjiaddr;
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)|| \
     defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
	bool mode400line;
	bool mode256k;
# endif
	bool mode320;
	int8_t display_page;
	int8_t display_page_bak;
	int cgrom_bank;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)|| \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
	int vram_bank;
	bool vram_page;
	uint8_t console_ram_bank;
	uint8_t vram_active_block;
	uint8_t vram_display_block;
	
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint16_t window_low;
	uint16_t window_high;
	uint16_t window_xbegin;
	uint16_t window_xend;

	bool window_opened;
# endif	
#endif	
	bool nmi_enable;
	bool diag_load_subrom_a;
	bool diag_load_subrom_b;
	bool diag_load_subrom_cg;
#endif
	bool diag_load_subrom_c;

	scrntype_t dpalette_pixel[8];
	uint8_t dpalette_data[8];
#if defined(_FM77AV_VARIANTS)
	pair_t apalette_index;
	uint8_t analog_palette_r[4096];
	uint8_t analog_palette_g[4096];
	uint8_t analog_palette_b[4096];
	scrntype_t analog_palette_pixel[4096];
#endif // FM77AV etc...
#if defined(_FM77AV_VARIANTS)
	uint8_t io_w_latch[0x40];
#elif !defined(_FM77AV40EX) && !defined(_FM77AV40SX)
	uint8_t io_w_latch[0x10];
#else
	uint8_t io_w_latch[0x100];
#endif
	uint8_t multimode_accessmask;
	uint8_t multimode_dispmask;
	bool multimode_accessflags[4];
	bool multimode_dispflags[4];
   
	uint32_t offset_point;
	pair_t tmp_offset_point[2];
	bool offset_changed[2];
	bool offset_77av;
   
#if defined(_FM77AV_VARIANTS)
	uint8_t subrom_bank;
	uint8_t subrom_bank_using;
	uint32_t offset_point_bank1;
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	bool monitor_ram;
	bool ram_protect;
# endif
#endif	

	// VRAM Write Page.
	//uint8_t  write_access_page;
	
	// ROM/RAM on Sub-System.
	uint8_t gvram[__FM7_GVRAM_PAG_SIZE];
	uint8_t gvram_shadow[__FM7_GVRAM_PAG_SIZE];
	
	uint8_t console_ram[0x1000];
	uint8_t work_ram[0x380];
	uint8_t shared_ram[0x80];
   
	uint8_t subsys_c[0x2800];
#if defined(_FM77AV_VARIANTS)
	uint8_t subsys_a[0x2000];
	uint8_t subsys_b[0x2000];
	uint8_t subsys_cg[0x2000];
	uint8_t submem_hidden[0x300];
#endif

	bool crt_flag;
	bool vram_accessflag;
	bool is_cyclesteal;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint8_t submem_cgram[0x4000];
	uint8_t submem_console_av40[0x2000];
	uint8_t subsys_ram[0x2000];
	uint8_t cgram_bank;
	bool kanji_level2;
	DEVICE *kanjiclass1;
	DEVICE *kanjiclass2;
#elif defined(_FM77_VARIANTS)
	DEVICE *kanjiclass1;
#endif
	bool vram_wrote_shadow;
	bool vram_wrote_table[411 * 5];
	bool vram_draw_table[411];
	//uint8_t vram_wrote_pages[411];
	uint32_t vram_wrote_addr_1[411];
	uint32_t vram_wrote_addr_2[411];
#if defined(_FM77AV_VARIANTS)
	bool use_alu;
	DEVICE *alu;
#endif	
	DEVICE *mainio;
	DEVICE *subcpu;
	DEVICE *keyboard;
	bool vram_wrote;
	void GETVRAM_8_200L(int yoff, scrntype_t *p, bool window_inv);
	void GETVRAM_4096(int yoff, scrntype_t *p, uint32_t rgbmask, bool window_inv);
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	void GETVRAM_8_400L(int yoff, scrntype_t *p, bool window_inv);
	void GETVRAM_256k(int yoff, scrntype_t *p);
#endif   
	uint8_t read_vram_l4_400l(uint32_t addr, uint32_t offset);
	uint32_t read_mmio(uint32_t addr);
	
	void init_read_table(void);
	void init_write_table(void);
	
	uint32_t read_vram_data8(uint32_t addr);
	uint32_t read_cpu_vram_data8(uint32_t addr);
	uint32_t read_dma_vram_data8(uint32_t addr);
	uint32_t read_console_ram(uint32_t addr);
	uint32_t read_work_ram(uint32_t addr);
	uint32_t read_shared_ram(uint32_t addr);
#if defined(_FM77AV_VARIANTS)
	uint32_t read_hidden_ram(uint32_t addr);
#endif
	uint32_t read_cgrom(uint32_t addr);
	uint32_t read_subsys_monitor(uint32_t addr);
	
	void write_vram_data8(uint32_t addr, uint8_t data);
	void write_cpu_vram_data8(uint32_t addr, uint8_t data);
	void write_dma_vram_data8(uint32_t addr, uint8_t data);
	void write_console_ram(uint32_t addr, uint8_t data);
	void write_work_ram(uint32_t addr, uint8_t data);
	void write_shared_ram(uint32_t addr, uint8_t data);
#if defined(_FM77AV_VARIANTS)
	void write_hidden_ram(uint32_t addr, uint8_t data);
#endif
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
	void write_subsys_cgram(uint32_t addr, uint8_t data);
	void write_subsys_ram(uint32_t addr, uint8_t data);
#endif
	void write_vram_l4_400l(uint32_t addr, uint32_t offset, uint32_t data);
	void write_mmio(uint32_t addr, uint8_t data);
	void write_dummy(uint32_t addr, uint8_t data);
   
	uint32_t read_bios(const _TCHAR *name, uint8_t *ptr, uint32_t size);
	void draw_screen2();
  public:
	DISPLAY(VM *parent_vm, EMU *parent_emu);
	~DISPLAY();
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);
	
	uint32_t read_data8(uint32_t addr);
	void write_data8(uint32_t addr, uint32_t data);
	
	uint32_t read_dma_data8(uint32_t addr);
	void write_dma_data8(uint32_t addr, uint32_t data);
	
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
	uint32_t read_io8(uint32_t addr) { // This is only for debug.
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		return io_w_latch[addr & 0xff];
#elif defined(_FM77AV_VARIANTS) // Really?
		return io_w_latch[addr & 0x3f];
#else
		return io_w_latch[addr & 0x0f];
#endif
	}
	bool screen_update(void);
	void reset_screen_update(void);
	void set_context_kanjiclass1(DEVICE *p)	{
#if defined(_FM77_VARIANTS) || \
    defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
		kanjiclass1 = p;
#endif
	}
	void set_context_kanjiclass2(DEVICE *p)	{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)|| \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
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
