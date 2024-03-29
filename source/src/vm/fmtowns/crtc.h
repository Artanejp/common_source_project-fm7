/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns CRTC ]
	History: 2016.12.28 Initial from HD46505 .
*/

#ifndef _TOWNS_CRTC_H_
#define _TOWNS_CRTC_H_

#include <atomic>
#include <cmath>
#include "device.h"
#include "towns_common.h"
#include "types/simd.h"
/*
 * I/O Address :
 *  0440H : Register address (8bit W/O : bit7 to 5 must be '0').
 *  0442H : Register data LOW  (8bit W/O).
 *  0443H : Register data HIGH (8bit W/O).
 * Interrupts (8259) :
 *  #11 : VSYNC.
 * Registers description:
 *  Note1: Values are hex, register numbers are decimal.
 *  Note2: Register #00 to #16 ,#18, #22 and #25 to #26 has 11bit.
 *         #17, #19 to #21, #23 to #24 and #27 has 16bit.
 *         #28 has bit 15,14  and 7 to 0.
 *         #29 has 4bit.
 *         #31 has bit15 and bit 8 to 0.
 *
 *  #00 / bit 7 - 1 : HSW1 (Bit 10 to 8 and 0 must be '0')
 *  #01 / bit 7 - 1 : HSW2 (Bit 10 to 8 and 0 must be '0')
 *  #02 / ??
 *  #03 / ??
 *  #04 / bit10 - 1 : HST  (Bit 0 bust be '1')
 *  #05 / bit 4 - 0 : VST1 (Bit 10 to 5 must be '0')
 *  #06 / bit 4 - 0 : VST2 (Bit 10 to 5 must be '0')
 *  #07 / bit 4 - 0 : EET  (Bit 10 to 5 must be '0')
 *  #08 / bit10 - 0 : VST
 *  #09 / bit10 - 0 : HDS0 (Layer 0)
 *  #10 / bit10 - 0 : HDE0 (Layer 0)
 *  #11 / bit10 - 0 : HDS1 (Layer 1)
 *  #12 / bit10 - 0 : HDE1 (Layer 1)
 *  #13 / bit10 - 0 : VDS0 (Layer 0)
 *  #14 / bit10 - 0 : VDE0 (Layer 0)
 *  #15 / bit10 - 0 : VDS1 (Layer 1)
 *  #16 / bit10 - 0 : VDE1 (Layer 1)
 *  #17 / bit15 - 0 : FA0  (Layer 0)
 *  #18 / bit10 - 0 : HAJ0 (Layer 0)
 *  #19 / bit15 - 0 : FO0  (Layer 0)
 *  #20 / bit15 - 0 : LO0  (Layer 0)
 *  #21 / bit15 - 0 : FA1  (Layer 1)
 *  #22 / bit10 - 0 : HAJ1 (Layer 1)
 *  #23 / bit15 - 0 : FO1  (Layer 1)
 *  #24 / bit15 - 0 : LO1  (Layer 1)
 *  #25 / bit10 - 0 : EHAJ (External horizonal sync adjusting)
 *  #26 / bit10 - 0 : EVAJ (External horizonal sync adjusting)
 *  #27 / bit15 - 0 : bit15-12 : ZV1 / bit11-8 : ZH1 / bit7-4 : ZV0 / bit3-0 : ZH0 (Zooming)
 *  #28 / bit15 : START ('1' = START) / bit14 : ESYN ('0' = Internal)
 *        bit7 : ESM1 ('0'=Impose / '1' = Digitize) / bit 6 : ESM0
 *        bit5 : CEN1 ('1' = Enable) / bit4 : CEN0
 *        bit3-2 : CL1 (Colors : 00 = ND / 01 = 32768x2 / 10 = 32768x1 / 11 = 256x1 or 16x2)
 *        bit1-0 : CL0
 *  #29 / bit3-2 : SCSEL (Clock divide = 2 * (SCSEL + 1)).
 *        bit1-0 : CLKSEL (00 = 28.6363MHz / 01 = 24.5454MHz / 10 = 25.175MHz / 11 = 21.0525MHz)
 *  #30 / Status register (RO):
 *        bit15 : DSPTV1 / bit14 : DSPTV0 (VDISP = '1')
 *        bit13 : DSPTH1 / bit12 : DSPTH0 (HDISP = '1')
 *        bit11 : FIELD (Field number)
 *        bit10 : VSYNC / bit9 : HSYNC ('1' = ACTIVE)
 *        bit8  : VIN ('1' = EXTERNAL VIDEO has exists)
 *       Dummy register  (WO):
 *        bit3 : FR3 ('1' = Halve tone) /  bit2 : FR2 ('1' = Using sync generator)
 *        bit1 : VCRDEN ('1' = Video card enable) / bit0 : SCEN ('1' = Sub carry enable)
 *
 *  #31 / bit15 = RETRG ('1' = Re-Trigger) / bit8-0 : PM (Pulse width : Width = PM * Clock).
 */
/*
 * Around Video Out.
 *  I/O 044CH : Digital palette modified flags.
 *	I/O 0448H (WO) : Register Address.
 *	I/O 044AH (WO) : Register Data.
 *  Registers:
 *   #00 : Control registers.
 *   #01 : Priority registers.
 */

#define SIG_TOWNS_CRTC_HSYNC			1
#define SIG_TOWNS_CRTC_VSYNC			2
#define SIG_TOWNS_CRTC_FIELD			3
#define SIG_TOWNS_CRTC_VDISP0			4
#define SIG_TOWNS_CRTC_VDISP1			5
#define SIG_TOWNS_CRTC_HDISP0			6
#define SIG_TOWNS_CRTC_HDISP1			7
#define SIG_TOWNS_CRTC_MMIO_CFF82H		8
#define SIG_TOWNS_CRTC_SPRITE_BUFFER	9
#define SIG_TOWNS_CRTC_COMPATIBLE_MMIO	10
#define SIG_TOWNS_CRTC_ADD_VAL_FO1		11
#define SIG_TOWNS_CRTC_CRTOUT			12
#define SIG_TOWNS_CRTC_MMIO_CFF86H		13

class DEBUGGER;
namespace FMTOWNS {

	enum {
		TOWNS_CRTC_PALETTE_INDEX = 0xff,
		TOWNS_CRTC_PALETTE_R = 0,
		TOWNS_CRTC_PALETTE_G,
		TOWNS_CRTC_PALETTE_B,
		TOWNS_CRTC_PALETTE_I,
	};
	enum {
		TOWNS_CRTC_REG_HSW1 = 0,
		TOWNS_CRTC_REG_HSW2 = 1,
		TOWNS_CRTC_REG_HST  = 4,
		TOWNS_CRTC_REG_VST1 = 5,
		TOWNS_CRTC_REG_VST2 = 6,
		TOWNS_CRTC_REG_EET  = 7,

		TOWNS_CRTC_REG_VST  = 8,
		TOWNS_CRTC_REG_HDS0 = 9,
		TOWNS_CRTC_REG_HDE0 = 10,
		TOWNS_CRTC_REG_HDS1 = 11,
		TOWNS_CRTC_REG_HDE1 = 12,
		TOWNS_CRTC_REG_VDS0 = 13,
		TOWNS_CRTC_REG_VDE0 = 14,
		TOWNS_CRTC_REG_VDS1 = 15,

		TOWNS_CRTC_REG_VDE1 = 16,
		TOWNS_CRTC_REG_FA0  = 17,
		TOWNS_CRTC_REG_HAJ0 = 18,
		TOWNS_CRTC_REG_FO0  = 19,
		TOWNS_CRTC_REG_LO0  = 20,
		TOWNS_CRTC_REG_FA1  = 21,
		TOWNS_CRTC_REG_HAJ1 = 22,
		TOWNS_CRTC_REG_FO1  = 23,

		TOWNS_CRTC_REG_LO1  = 24,
		TOWNS_CRTC_REG_EHAJ = 25,
		TOWNS_CRTC_REG_EVAJ = 26,
		TOWNS_CRTC_REG_ZOOM = 27,
		TOWNS_CRTC_REG_DISPMODE = 28,
		TOWNS_CRTC_REG_CLK      = 29,
		TOWNS_CRTC_REG_DUMMY    = 30,
		TOWNS_CRTC_REG_CTRL     = 31,
	};

	enum {
		DISPMODE_NONE = 0,
		DISPMODE_256,
		DISPMODE_32768,
		DISPMODE_16,
	};
}

namespace FMTOWNS {

typedef struct {
	uint8_t raw[256][4];
	scrntype_t pixels[256];
} palette_backup_t;
// May align to be faster.
#pragma pack(push, 4)
typedef struct {
	int32_t mode[4];
	int32_t pixels[4];
	int32_t mag[4];
	int32_t num[4];
	uint32_t prio_dummy;
#pragma pack(push, 1)
	uint8_t r50_planemask; // MMIO 000CF882h : BIT 5(C0) and BIT2 to 0
	uint8_t crtout[2];
	uint8_t r50_pad;
#pragma pack(pop)
	int32_t  bitshift[2];
	uint32_t pad;
	// Align of 4 * (4 + 1 + 3) = 4 * 8 [bytes] = 256 [bits]
	__DECL_ALIGNED(16) uint8_t pixels_layer[2][TOWNS_CRTC_MAX_PIXELS * sizeof(uint16_t)]; // RAW VALUE
	__DECL_ALIGNED(16) palette_backup_t palettes[2];
	// pixels_lauyer[][] : 1024 * 2 * 8 = 1024 * 16 [bytes]
} linebuffer_t;
#pragma pack(pop)


class TOWNS_VRAM;
class TOWNS_SPRITE;
class FONT_ROMS;
class TOWNS_CRTC : public DEVICE
{
protected:
	TOWNS_VRAM* d_vram;
	TOWNS_SPRITE* d_sprite;
	FONT_ROMS*       d_font;

	uint16_t machine_id;
	uint8_t cpu_id;
	bool is_compatible;
	// output signals
	outputs_t outputs_int_vsync;  // Connect to int 11.
	uint16_t regs[32];      // I/O 0442H, 0443H
	bool regs_written[32];

	// Shadow REGS
	uint16_t hsw1;
	uint16_t hsw2;
	uint16_t hst_reg;
	uint16_t vst1;
	uint16_t vst2;
	uint16_t eet;
	uint16_t vst_reg;

	uint16_t vds[2];
	uint16_t vde[2];
	uint16_t hds[2];
	uint16_t hde[2];
	uint16_t haj[2];
	int hstart_reg[2];
	int hend_reg[2];
	int vstart_reg[2];
	int vend_reg[2];
	uint16_t hwidth_reg[2];
	uint16_t vheight_reg[2];

	uint8_t crtc_ch;         // I/O 0440H
	bool timing_changed[2];
	bool address_changed[2];
	bool mode_changed[2];
	uint32_t fo1_offset_value;

	uint8_t display_mode[2];
	bool line_changed[2][TOWNS_CRTC_MAX_LINES];
	bool display_enabled;

	double crtc_clock; //
	int cpu_clocks;

	// They are not saved.Must be calculate when loading.
	int horiz_khz;
	double horiz_us; // (HST + 1) * clock
	double horiz_width_posi_us, horiz_width_nega_us; // HSW1, HSW2
	double vert_us; // (VST +1) * horiz_us / 2.0
	double vert_sync_end_us; // VST2 * horiz_us / 2.0
	double eet_us;
	double frame_us;
	double vst1_us; // VST1 * horiz_us / 2.0
	double vst2_us;
	int hst[4], vst[4];

	double horiz_us_next; // (HST + 1) * clock
	double horiz_width_posi_us_next, horiz_width_nega_us_next; // HSW1, HSW2

	double vert_start_us[2];
	double vert_end_us[2];
	double horiz_start_us[2];
	double horiz_end_us[2];

	double horiz_start_us_next[2];
	double horiz_end_us_next[2];
	bool req_recalc;
	// End

	double frames_per_sec;

	uint32_t vstart_addr[2];  // VSTART ADDRESS
    uint32_t hstart_words[2]; // HSTART ((HDS[01] * clock) : Horizonal offset words (Related by ZH[01]). Maybe 0.
    uint32_t hend_words[2];   // HEND   ((HDE[01] * clock) : Horizonal offset words (Related by ZH[01]). Maybe 0.
	uint32_t frame_offset[2]; // FOx.
	uint32_t line_offset[2]; // LOx.

	uint32_t frame_offset_bak[2]; // FOx(Backup).
	uint32_t line_offset_bak[2]; // LOx(Backup).

	uint32_t head_address[2];
	int horiz_offset_tmp[2];
	int vert_offset_tmp[2];
	bool impose_mode[2]; // OK?
	bool carry_enable[2]; //OK?

	bool is_sprite;
	uint32_t sprite_offset;

	uint8_t zoom_factor_vert[2]; // Related display resolutions of two layers and zoom factors.
	uint8_t zoom_factor_horiz[2]; // Related display resolutions of two layers and zoom factors.
	uint8_t zoom_count_vert[2];
	uint32_t line_count[2]; // Separate per layer.

	uint8_t scsel;
	uint8_t clksel;

	int pixels_per_line;
	int lines_per_frame;
	int max_lines;

	int vert_line_count; // Not separate per layer.Total count.
	// Note: To display to real screen, use blending of OpenGL/DirectX
	//       from below framebuffers per layer if you can.
	//       And, recommand to use (hardware) shader to rendering (= not to use framebuffer of below) if enabled.
	//       Not recommanded to use draw_screen() type rendering.
	//       Rendering precesses may be heavily to use only CPU (I think). 20170107 K.Ohta.

	// Not Saved?.
	// End.
	bool vsync, hsync, hdisp[2], frame_in[2];
	bool interlace_field;


	// Around Analog palette.
	uint8_t apalette_code; // I/O FD90H (RW). 16 or 256 colors.
	uint8_t apalette_b;    // I/O FD92H (RW).
	uint8_t apalette_r;    // I/O FD94H (RW).
	uint8_t apalette_g;    // I/O FD96H (RW).
	uint8_t   apalette_16_rgb[2][16][4];   // R * 256 + G * 16 + B
	scrntype_t apalette_16_pixel[2][16]; // Not saved. Must be calculated.
	uint8_t   apalette_256_rgb[256][4];    // R * 65536 + G * 256 + B
	scrntype_t apalette_256_pixel[256];  // Not saved. Must be calculated.

	uint8_t tvram_snapshot[0x4000];

	// FM-R50 emulation
	uint8_t r50_planemask; // MMIO 000CF882h : BIT 5(C0) and BIT2 to 0
	uint8_t r50_pagesel;   // MMIO 000CF882h : BIT 4
	uint8_t dpalette_regs[8]; // I/O FD98H - FD9FH
	bool dpalette_changed;

	// Video output controller (I/O 0448H, 044AH)
	// Register 00 : Display mode.
	// Register 11: Priority mode.
	bool video_brightness; // false = high.

	// Others.
	// VRAM CONTROL REGISTER.
	uint8_t voutreg_num;  // I/O 0448h
	uint8_t voutreg_ctrl; // I/O 044Ah : voutreg_num = 0.
	uint8_t voutreg_prio; // I/O 044Ah : voutreg_num = 1.
	uint8_t video_out_regs[2];
	bool crtout[2];              // I/O FDA0H WRITE
	uint8_t crtout_reg;
	uint8_t voutreg_ctrl_bak;
	uint8_t voutreg_prio_bak;
	// End.
	bool is_single_layer;

	//
	// Event IDs. Saved.
	int event_id_hsync;
	int event_id_hsw;
	//int event_id_hsw1; //??
	//int event_id_hsw2; //??
	int event_id_vsync;
	int event_id_vstart;
	int event_id_vst1;
	int event_id_vst2;
	int event_id_hstart;
	int event_id_vds[2];
	int event_id_vde[2];
	int event_id_hds[2];
	int event_id_hde[2];

	std::atomic<int> display_linebuf;
	std::atomic<int> render_linebuf;
	const int display_linebuf_mask = 3;

	__DECL_ALIGNED(32) linebuffer_t linebuffers[4][TOWNS_CRTC_MAX_LINES];

	// Render buffer
		// ToDo: faster alpha blending.
	__DECL_ALIGNED(16) scrntype_t lbuffer0[TOWNS_CRTC_MAX_PIXELS + 16];
	__DECL_ALIGNED(16) scrntype_t lbuffer1[TOWNS_CRTC_MAX_PIXELS + 16];
	__DECL_ALIGNED(16) scrntype_t abuffer0[TOWNS_CRTC_MAX_PIXELS + 16];
	__DECL_ALIGNED(16) scrntype_t abuffer1[TOWNS_CRTC_MAX_PIXELS + 16];

	virtual void vline_hook();
	virtual void copy_regs();
	virtual void calc_pixels_lines();

	void reset_vsync();
	void __FASTCALL set_vsync(bool val);
	virtual void force_recalc_crtc_param(void);
	virtual bool calc_screen_parameters(void);
	void __FASTCALL calc_zoom_regs(uint16_t val);

	virtual void restart_display();
	virtual void stop_display();
	virtual void notify_mode_changed(int layer, uint8_t mode);

	void cancel_event_by_id(int& event_num);

	void __FASTCALL set_crtc_clock(uint16_t val, bool force);
	uint16_t read_reg30();
	uint32_t __FASTCALL get_font_address(uint32_t c, uint8_t &attr);

	virtual void __FASTCALL update_crtc_reg(uint8_t ch, uint32_t data);
	virtual void __FASTCALL calc_apalette16(int layer, int index);
	virtual void __FASTCALL calc_apalette256(int index);

	virtual uint8_t __FASTCALL get_apalette_b();
	virtual uint8_t __FASTCALL get_apalette_r();
	virtual uint8_t __FASTCALL get_apalette_g();

	bool __FASTCALL render_16(int trans, scrntype_t* dst, scrntype_t *mask, int y, int layer, bool is_transparent, bool do_alpha, int& rendered_pixels);
	bool __FASTCALL render_256(int trans, scrntype_t* dst, int y, int& rendered_pixels);
	bool __FASTCALL render_32768(int trans, scrntype_t* dst, scrntype_t *mask, int y, int layer, bool is_transparent, bool do_alpha, int& rendered_pixels);

	virtual void __FASTCALL pre_transfer_line(int line);
	virtual void __FASTCALL transfer_line(int line, int layer);
	inline void __FASTCALL transfer_pixels(scrntype_t* dst, scrntype_t* src, int w);

	virtual void __FASTCALL mix_screen(int y, int width, bool do_mix0, bool do_mix1, int bitshift0, int bitshift1, int words0, int words1);

	virtual void update_horiz_khz()
	{
		double horiz_us_tmp;
		__LIKELY_IF(hst_reg != 0) {
			horiz_us_tmp = crtc_clock * (double)hst_reg;
		} else {
			horiz_us_tmp = crtc_clock;
		}
		horiz_khz = std::lrint(1.0e3 / horiz_us_tmp);
	}
	inline void make_crtout_from_fda0h(uint8_t data)
	{
		crtout[0] = ((data & 0x0c) != 0) ? true : false;
		crtout[1] = ((data & 0x03) != 0) ? true : false;
	}
	inline void __FASTCALL make_dispmode(bool& is_single, int& layer0, int& layer1)
	{
		//const uint8_t _mode0 = voutreg_ctrl & 0x03;
		//const uint8_t _mode1 = (voutreg_ctrl & 0x0c) >> 2;
		is_single = ((voutreg_ctrl & 0x10) == 0) ? true : false;
		static const int modes_by_voutreg_ctrl[4] = { DISPMODE_NONE, DISPMODE_16, DISPMODE_256, DISPMODE_32768 };
		static const int modes_by_CR0_single[4] = { DISPMODE_NONE, DISPMODE_NONE, DISPMODE_32768, DISPMODE_256 };
		static const int modes_by_CR0_multi[4] = { DISPMODE_NONE, DISPMODE_32768, DISPMODE_NONE, DISPMODE_16 };
		// ToDo: High resolution.
		if(is_single) {
			layer0 =  ((voutreg_ctrl & 0x08) != 0) ? modes_by_CR0_single[display_mode[0]] : DISPMODE_NONE;
			layer1 = DISPMODE_NONE;
		} else {
			layer0 = modes_by_CR0_multi[display_mode[0] & 3];
			layer1 = modes_by_CR0_multi[display_mode[1] & 3];
		}
	}
	inline void __FASTCALL recalc_cr0(uint16_t cr0, bool calc_only)
	{
		if(!(calc_only)) {
			if((cr0 & 0x8000) == 0) {
			// START BIT
				restart_display();
			} else {
				stop_display();
			}
		}
		if((cr0 & 0x4000) == 0) {
			// ESYN BIT
			// EXTERNAL SYNC OFF
		} else {
			// EXTERNAL SYNC ON
		}
		impose_mode[1]  = ((cr0 & 0x0080) == 0);
		impose_mode[0]  = ((cr0 & 0x0040) == 0);
		carry_enable[1] = ((cr0 & 0x0020) != 0);
		carry_enable[0] = ((cr0 & 0x0010) != 0);

		uint8_t dmode[2];
		dmode[0] = cr0 & 0x03;
		dmode[1] = (cr0 & 0x0c) >> 2;
		for(int i = 0; i < 2; i++) {
			__UNLIKELY_IF((dmode[i] != display_mode[i]) && !(calc_only)) {
				mode_changed[i] = true;
				notify_mode_changed(i, dmode[i]);
			}
			display_mode[i] = dmode[i];
		}
	}
	inline scrntype_t *scaling_store(scrntype_t *dst, csp_vector8<scrntype_t> *src, const int mag, const int words, size_t& width);

	virtual void __FASTCALL set_apalette(uint8_t ch, uint8_t val, bool recalc);
	virtual void render_text();

public:
	TOWNS_CRTC(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_int_vsync);
		d_sprite = NULL;
		d_vram = NULL;
		d_font = NULL;
		is_sprite = false;
		set_device_name(_T("FM-Towns CRTC"));
	}
	~TOWNS_CRTC() {}

	void initialize() override;
	void release() override;
	void reset() override;
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame) override;
	void event_pre_frame() override;
	void event_frame() override;

	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	uint32_t __FASTCALL read_signal(int ch) override;

	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;

	void __FASTCALL write_io16(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io16(uint32_t addr) override;

	void __FASTCALL event_callback(int event_id, int err) override;

	bool process_state(FILEIO* state_fio, bool loading) override;

	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;
	bool write_debug_reg(const _TCHAR *reg, uint32_t data) override;

	bool is_debugger_available() override
	{
		return true;
	}
	uint64_t get_debug_data_addr_space() override
	{
		return 0x1;
	}
	// unique functions
	void draw_screen();
	inline linebuffer_t* __FASTCALL get_line_buffer(int page, int line)
	{
		page = page & 1;
		__UNLIKELY_IF(line < 0) return NULL;
		__UNLIKELY_IF(line >= TOWNS_CRTC_MAX_LINES) return NULL;
		return &(linebuffers[page][line]);
	}

	void set_context_sprite(DEVICE* dev)
	{
		d_sprite = (TOWNS_SPRITE*)dev;
	}
	void set_context_vram(DEVICE* dev)
	{
		d_vram = (TOWNS_VRAM*)dev;
	}
	void set_context_font(FONT_ROMS* dev)
	{
		d_font = dev;
	}
	void set_machine_id(uint16_t val)
	{
		machine_id = val & 0xfff8;
	}
	void set_cpu_id(uint16_t val)
	{
		cpu_id = val & 0x07;
	}

	void set_context_vsync(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_int_vsync, device, id, mask);
	}
	uint16_t* get_regs_ptr()
	{
		return regs;
	}

};

inline scrntype_t* TOWNS_CRTC::scaling_store(scrntype_t *dst, csp_vector8<scrntype_t> *src, const int mag, const int words, size_t& width)
{
	__UNLIKELY_IF((dst == NULL) || (src == NULL)) return NULL;

	uintptr_t dstval = (uintptr_t)dst;
	const uintptr_t as = alignof(csp_vector8<scrntype_t>) - 1;
	__LIKELY_IF((dstval & as) == 0) { // ALIGNED
		for(int x = 0; (x < words) && (width > 0) ; x++) {
			switch(mag) {
			case 1:
				__LIKELY_IF(width >= 8) {
					src[x].store_aligned(dst);
					dst += 8;
					width -= 8;
				} else {
					src[x].store_limited(dst, width);
					dst += width;
					width = 0;
					break;
				}
				break;
			case 2:
				__LIKELY_IF(width >= 16) {
					src[x].store2_aligned(dst);
					dst += 16;
					width -= 16;
				} else {
					src[x].store2_limited(dst, width);
					dst += (2 * width);
					width = 0;
					break;
				}
				break;
			case 4:
				__LIKELY_IF(width >= 32) {
					src[x].store4_aligned(dst);
					dst += 32;
					width -= 32;
				} else {
					src[x].store4_limited(dst, width);
					dst += (width * 4);
					width = 0;
					break;
				}
				break;
			default:
				__LIKELY_IF(width >= (8 * mag)) {
					src[x].store_n(dst, mag);
					dst += (8 * mag);
					width -= (8 * mag);
				} else {
					src[x].store_n_limited(dst, mag, width);
					dst += (width * mag);
					width = 0;
				}
				break;
			}
		}
	} else { // Not aligned
		for(int x = 0; (x < words) && (width > 0) ; x++) {
			switch(mag) {
			case 1:
				__LIKELY_IF(width >= 8) {
					src[x].store(dst);
					dst += 8;
					width -= 8;
				} else {
					src[x].store_limited(dst, width);
					dst += width;
					width = 0;
					break;
				}
				break;
			case 2:
				__LIKELY_IF(width >= 16) {
					src[x].store2(dst);
					dst += 16;
					width -= 16;
				} else {
					src[x].store2_limited(dst, width);
					dst += (2 * width);
					width = 0;
					break;
				}
				break;
			case 4:
				__LIKELY_IF(width >= 32) {
					src[x].store4(dst);
					dst += 32;
					width -= 32;
				} else {
					src[x].store4_limited(dst, width);
					dst += (width * 4);
					width = 0;
					break;
				}
				break;
			default:
				__LIKELY_IF(width >= (8 * mag)) {
					src[x].store_n(dst, mag);
					dst += (8 * mag);
					width -= (8 * mag);
				} else {
					src[x].store_n_limited(dst, mag, width);
					dst += (width * mag);
					width = 0;
				}
				break;
			}
		}
	}
	return dst;
}
}



#endif
