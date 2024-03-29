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
#define SIG_TOWNS_CRTC_R50_PAGESEL		14
#define SIG_TOWNS_CRTC_REG_LO0			16
#define SIG_TOWNS_CRTC_REG_LO1			17
#define SIG_TOWNS_CRTC_REGISTER_VALUE	32
class DEBUGGER;
namespace FMTOWNS {
	enum {
		CRTC_BUFFER_NUM = 4,
	};
	enum {
		NOT_LOOP = 0,
		IS_LOOP = 255
	};
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
		DISPMODE_DUP = 0x80,
	};
	enum {
		VOUTREG_CTRL = 0,
		VOUTREG_PRIO = 1,
		VOUTREG_2,
		VOUTREG_3,
	};
		
}

namespace FMTOWNS {

typedef struct {
	uint8_t raw[256][4];
	scrntype_t pixels[256];
} palette_backup_t;
// May align to be faster.

typedef struct {
#pragma pack(push, 1)
	// 32 * 4
	uint8_t mode[4];
	uint8_t is_hloop[4];
	int8_t mag[4];
	uint8_t r50_planemask[2]; // MMIO 000CF882h : BIT 5(C0) and BIT2 to 0
	uint8_t crtout[2];
#pragma pack(pop)
#pragma pack(push, 4)
	// 32 * 12
	int32_t pixels[4];
	int32_t num[4];
	int32_t bitoffset[2];
	int32_t prev_y[2];
#pragma pack(pop)
	// Align of 16 * 32 bits = 512 bits.
//#pragma pack(push, 16)
	uint8_t pixels_layer[2][TOWNS_CRTC_MAX_PIXELS * sizeof(uint16_t)]; // RAW VALUE
	palette_backup_t palettes[2];
//#pragma pack(pop)
	// pixels_lauyer[][] : 1024 * 2 * 8 = 1024 * 16 [bytes]
} linebuffer_t;



class TOWNS_VRAM;
class TOWNS_SPRITE;
class FONT_ROMS;
class TOWNS_CRTC : public DEVICE
{
protected:
	TOWNS_VRAM* d_vram;
	TOWNS_SPRITE* d_sprite;

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
	int hstart_val[2];
	int hbitshift_val[2];
	int hoffset_val[2];
	int hwidth_val[2];
	int voffset_val[2];
	int vheight_val[2];

	uint8_t crtc_ch;         // I/O 0440H
	bool timing_changed[2];
	bool address_changed[2];
	bool mode_changed[2];
	uint32_t fo1_offset_value;

	uint8_t display_mode[2];
	int     real_display_mode[2];
	bool display_enabled;
	bool display_enabled_pre;

	double crtc_clock; //
	int cpu_clocks;

	// They are not saved.Must be calculate when loading.
	int horiz_khz;
	double horiz_us; // (HST + 1) * clock
	double horiz_width_posi_us, horiz_width_nega_us; // HSW1, HSW2
	double vert_us; // (VST +1) * horiz_us / 2.0
	double vert_sync_end_us; // VST2 * horiz_us / 2.0
	double frame_us;
	
	uint16_t vst1_count; // VST1 * horiz_us / 2.0
	uint16_t vst2_count;
	uint16_t eet_count;
	uint16_t lines_per_frame_current;
	int hst[FMTOWNS::CRTC_BUFFER_NUM], vst[FMTOWNS::CRTC_BUFFER_NUM];
	int voffset[FMTOWNS::CRTC_BUFFER_NUM][2];

	double horiz_us_next; // (HST + 1) * clock
	double horiz_width_posi_us_next, horiz_width_nega_us_next; // HSW1, HSW2

	double horiz_start_us[2];
	double horiz_end_us[2];

	double horiz_start_us_next[2];
	double horiz_end_us_next[2];
	bool req_recalc;
	// End

	double frames_per_sec;

	uint32_t vstart_addr[2];  // VSTART ADDRESS
	uint32_t frame_offset[2]; // FOx.
	uint32_t line_offset[2]; // LOx.

	uint32_t frame_offset_bak[2]; // FOx(Backup).

	uint32_t head_address[2];
	bool impose_mode[2]; // OK?
	bool carry_enable[2]; //OK?
	uint8_t priority_cache[FMTOWNS::CRTC_BUFFER_NUM];
	uint8_t control_cache[FMTOWNS::CRTC_BUFFER_NUM];

	uint32_t sprite_offset;
	int sprite_count;
	int sprite_limit;
	int32_t sprite_zoom;
	int32_t sprite_zoom_factor;
	
	uint8_t zoom_raw_vert[2];
	uint8_t zoom_raw_horiz[2];
	uint8_t zoom_factor_vert[2]; // Related display resolutions of two layers and zoom factors.
	uint8_t zoom_factor_horiz[2]; // Related display resolutions of two layers and zoom factors.
	uint8_t zoom_count_vert[2];
	uint32_t line_count[2]; // Separate per layer.

	uint8_t scsel;
	uint8_t clksel;

	int pixels_per_line;
	int lines_per_frame;
	int max_lines;
	int hstart_position;

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

	// FM-R50 emulation
	uint8_t r50_planemask; // MMIO 000CF882h : BIT 5(C0) and BIT2 to 0
	uint8_t r50_pagesel;   // MMIO 000CF882h : BIT 4
	uint8_t dpalette_regs[8]; // I/O FD98H - FD9FH
	bool dpalette_changed;

	// Video output controller (I/O 0448H, 044AH)
	// Register 00 : Display mode.
	// Register 11: Priority mode.
	bool video_brightness; // false = high.
	bool is_interlaced[FMTOWNS::CRTC_BUFFER_NUM][2]; // cache values of layuer_is_interlaced().
	// Others.
	// VRAM CONTROL REGISTER.
	uint8_t voutreg_num;       // I/O 0448h
	uint8_t video_out_regs[4]; // I/O 044Ah 
	bool crtout_towns[2];      // I/O 044AH WRITE (REG #0)
	bool crtout_fmr[2];        // I/O FDA0H WRITE
	uint8_t crtout_reg;
	// End.
	bool is_single_layer[FMTOWNS::CRTC_BUFFER_NUM];
	//int render_vpoint[2];
	//
	// Event IDs. Saved.
	int event_hsync;
	int event_hdisp[2];
	
	
	std::atomic<int> display_linebuf;
	std::atomic<int> render_linebuf;
	std::atomic<int> display_remain;
	
	const int display_linebuf_mask = FMTOWNS::CRTC_BUFFER_NUM - 1;

	__DECL_ALIGNED(32) linebuffer_t linebuffers[FMTOWNS::CRTC_BUFFER_NUM][TOWNS_CRTC_MAX_LINES];

	// Render buffer
		// ToDo: faster alpha blending.
	__DECL_ALIGNED(16) scrntype_t lbuffer0[TOWNS_CRTC_MAX_PIXELS + 16];
	__DECL_ALIGNED(16) scrntype_t lbuffer1[TOWNS_CRTC_MAX_PIXELS + 16];
	__DECL_ALIGNED(16) scrntype_t abuffer0[TOWNS_CRTC_MAX_PIXELS + 16];
	__DECL_ALIGNED(16) scrntype_t abuffer1[TOWNS_CRTC_MAX_PIXELS + 16];
	__DECL_ALIGNED(16) scrntype_t pix_cache[TOWNS_CRTC_MAX_PIXELS + 16];
	__DECL_ALIGNED(16) scrntype_t pix_cache0[TOWNS_CRTC_MAX_PIXELS + 16];
	__DECL_ALIGNED(16) scrntype_t alpha_cache[TOWNS_CRTC_MAX_PIXELS + 16];

	virtual void copy_regs_v();
	virtual void copy_regs_h();
	virtual void __FASTCALL update_regs_h(const int layer);
	virtual void __FASTCALL update_regs_v(const int layer);
	virtual void calc_pixels_lines();

	void reset_paletts();

	void reset_vsync();
	void __FASTCALL set_vsync(bool val);
	virtual void force_recalc_crtc_param(void);
	virtual bool calc_screen_parameters(void);
	void __FASTCALL calc_zoom_regs(uint16_t val);
	virtual void set_crtc_parameters_from_regs();
	virtual void __FASTCALL calc_width(const bool is_single, int64_t& hwidth_p);
	virtual void __FASTCALL recalc_width_by_clock(const uint32_t magx, int64_t& width);
	virtual void __FASTCALL recalc_offset_by_clock(const uint32_t magx, int& hoffset_p, int64_t& hbitshift_p);

	virtual void restart_display();
	virtual void stop_display();
	virtual void __FASTCALL notify_mode_changed(int layer, uint8_t mode);
	virtual void __FASTCALL recalc_hdisp_from_crtc_params(int layer, double& start_us, double& end_us);

	inline void cancel_event_by_id(int& event_num)
	{
		if(event_num > -1) {
			cancel_event(this, event_num);
		}
		event_num = -1;
	}

	void __FASTCALL set_crtc_clock(uint16_t val, bool force);
	uint16_t read_reg30();

	virtual void __FASTCALL update_crtc_reg(uint8_t ch, uint32_t data);
	virtual void __FASTCALL calc_apalette16(int layer, int index);
	virtual void __FASTCALL calc_apalette256(int index);

	virtual uint8_t __FASTCALL get_apalette_b();
	virtual uint8_t __FASTCALL get_apalette_r();
	virtual uint8_t __FASTCALL get_apalette_g();

	
	bool __FASTCALL render_16(int trans, scrntype_t* dst, scrntype_t *mask, int y, int layer, bool is_transparent, bool do_alpha, int& rendered_pixels);
	bool __FASTCALL render_256(int trans, scrntype_t* dst, int y, int& rendered_pixels);
	bool __FASTCALL render_32768(int trans, scrntype_t* dst, scrntype_t *mask, int y, int layer, bool is_transparent, bool do_alpha, int& rendered_pixels);

	virtual void __FASTCALL pre_transfer_line(int layer, int line);
	virtual void __FASTCALL transfer_line(int layer, int line);
	inline void __FASTCALL transfer_pixels(scrntype_t* dst, scrntype_t* src, int w);
	void __FASTCALL copy_line(const int trans, int layer, const int from_y, const int to_y);
	virtual void __FASTCALL clear_line(const int trans, int layer, const int y);

	virtual void __FASTCALL mix_screen(int y, int width, bool do_mix0, bool do_mix1, int bitshift0, int bitshift1, int words0, int words1, bool is_hloop0, bool is_hloop1);

	inline void simd_fill(scrntype_t* dst, csp_vector8<scrntype_t> data, size_t words)
	{
		const uintptr_t pdst = (uintptr_t)dst;
		const size_t width_8 = sizeof(scrntype_t) * 8;
		const size_t mask_8 = ~width_8;
		__LIKELY_IF(words > 7) {
			__LIKELY_IF((pdst & mask_8) == 0) {
				for(size_t xx = 0; xx < words; xx += 8) {
					data.store_aligned(&(dst[xx]));
				}
			} else {
				for(size_t xx = 0; xx < words; xx += 8) {
					data.store(&(dst[xx]));
				}
			}
		}
		if((words & 7) != 0) {
			size_t xx = words & (~7);
			data.store_limited(&(dst[xx]), words & 7);
		}
	}

	inline void simd_copy(scrntype_t* dst, scrntype_t* src, size_t words)
	{
		const uintptr_t pdst = (uintptr_t)dst;
		const uintptr_t psrc = (uintptr_t)src;
		const size_t width_8 = sizeof(scrntype_t) * 8;
		const size_t mask_8 = ~width_8;
		csp_vector8<scrntype_t> pix;
		__LIKELY_IF(words > 7) {
			__LIKELY_IF((psrc & mask_8) == 0) { // Aligned
				if((pdst & mask_8) == 0) { // Aligned
					for(size_t xx = 0; xx < words; xx += 8) {
						pix.load_aligned(&(src[xx]));
						pix.store_aligned(&(dst[xx]));
					}
				} else {
					for(size_t xx = 0; xx < words; xx += 8) {
						pix.load_aligned(&(src[xx]));
						pix.store(&(dst[xx]));
					}
				}
			} else __LIKELY_IF((pdst & mask_8) == 0) { // DST ONLY ALIGNED
				for(size_t xx = 0; xx < words; xx += 8) {
					pix.load(&(src[xx]));
					pix.store_aligned(&(dst[xx]));
				}
			} else {
				for(size_t xx = 0; xx < words; xx += 8) {
					pix.load(&(src[xx]));
					pix.store(&(dst[xx]));
				}
			}
		}
		if((words & 7) != 0) {
			size_t xx = words & 0xfffffff8;
			size_t w = words & 7;
			pix.load_limited(&(src[xx]), w);
			pix.store_limited(&(dst[xx]), w);
		}
	}
	virtual void begin_of_display();
	inline void update_vstart(const int layer)
	{
		vstart_addr[layer]  = regs[(layer * 4) + TOWNS_CRTC_REG_FA0]  & 0xffff;
	}
	
	inline void update_line_offset(const int layer)
	{
		line_offset[layer]  = regs[(layer * 4) + TOWNS_CRTC_REG_LO0]  & 0xffff;
	}
	
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
		crtout_fmr[0] = ((data & 0x0c) != 0) ? true : false;
		crtout_fmr[1] = ((data & 0x03) != 0) ? true : false;
	}
	constexpr bool is_single_mode_for_standard(const uint8_t control_reg_val)
	{
		return (((control_reg_val & 0x10) == 0) ? true : false);
	}
	inline void __FASTCALL make_dispmode(bool& is_single, int& layer0, int& layer1)
	{
		//const uint8_t _mode0 = voutreg_ctrl & 0x03;
		//const uint8_t _mode1 = (voutreg_ctrl & 0x0c) >> 2;
		uint8_t _ctrl = control_cache[render_linebuf & display_linebuf_mask];
		is_single = is_single_mode_for_standard(_ctrl);
		static const int modes_by_voutreg_ctrl[4] = { DISPMODE_NONE, DISPMODE_16, DISPMODE_256, DISPMODE_32768 };
		static const int modes_by_CR0_single[4] = { DISPMODE_NONE, DISPMODE_NONE, DISPMODE_32768, DISPMODE_256 };
		static const int modes_by_CR0_multi[4] = { DISPMODE_NONE, DISPMODE_32768, DISPMODE_NONE, DISPMODE_16 };
		// ToDo: High resolution.
		if(is_single) {
			layer0 =  ((_ctrl & 0x08) != 0) ? modes_by_CR0_single[display_mode[0]] : DISPMODE_NONE;
			layer1 = DISPMODE_NONE;
		} else {
			layer0 = modes_by_CR0_multi[display_mode[0] & 3];
			layer1 = modes_by_CR0_multi[display_mode[1] & 3];
		}
	}
	constexpr bool layer_is_interlaced(int layer)
	{
		bool _b = (frame_offset[layer & 1] == 0) ? true : false;
		return _b;
	}
	inline void set_io_044a(const uint32_t data)
	{
		video_out_regs[voutreg_num & 3] = data & 0xff;
		if((voutreg_num & 3) == FMTOWNS::VOUTREG_CTRL) {
			make_crtout_from_044a(data);
		}
	}
	inline virtual void update_control_registers(const int trans)
	{
		const int num = trans & display_linebuf_mask;
		priority_cache[num] = video_out_regs[FMTOWNS::VOUTREG_PRIO];
		control_cache[num] = video_out_regs[FMTOWNS::VOUTREG_CTRL];
	}
	inline virtual void make_crtout_from_044a(uint8_t data)
	{
		bool is_single = ((data & 0x10) == 0) ? true : false;
		if(is_single) {
			crtout_towns[0] = ((data & 0x08) != 0) ? true : false;
			crtout_towns[1] = false;
		} else {
			crtout_towns[0] = ((data & 0x01) != 0) ? true : false;
			crtout_towns[1] = ((data & 0x04) != 0) ? true : false;
		}
	}
	inline uint8_t get_io_044a()
	{
		return video_out_regs[voutreg_num & 3];
	}
	inline void __FASTCALL recalc_cr0(uint16_t cr0, bool calc_only)
	{
		if(!(calc_only)) {
			if((cr0 & 0x8000) != 0) {
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
			__UNLIKELY_IF(dmode[i] != display_mode[i]) {
				notify_mode_changed(i, dmode[i]);
			}
		}
	}
	inline size_t scaling_store(scrntype_t *dst, csp_vector8<scrntype_t> *src, const int mag, const size_t words, size_t& width);
	inline size_t scaling_store_by_map(scrntype_t *dst, csp_vector8<scrntype_t> *src, csp_vector8<uint16_t> magx_map, const size_t words, size_t& width);
	
	inline size_t store1_aligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width);
	inline size_t store2_aligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width);
	inline size_t store4_aligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width);
	inline size_t store1_unaligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width);
	inline size_t store2_unaligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width);
	inline size_t store4_unaligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width);
	inline size_t store_n_any(scrntype_t *dst, csp_vector8<scrntype_t> *src, const int mag, const size_t words, size_t& width);

	virtual void __FASTCALL set_apalette(uint8_t ch, uint8_t val, bool recalc);
	uint32_t get_sprite_offset();

public:
	TOWNS_CRTC(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_int_vsync);
		d_sprite = NULL;
		d_vram = NULL;
		set_device_name(_T("FM-Towns CRTC"));
	}
	~TOWNS_CRTC() {}

	void initialize() override;
	void release() override;
	void reset() override;

	void event_pre_frame() override;
	void event_frame() override;
	void event_vline(int v, int clock) override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	uint32_t __FASTCALL read_signal(int ch) override;

	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;

	void __FASTCALL write_io16(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io16(uint32_t addr) override;

	void __FASTCALL write_io32(uint32_t addr, uint32_t data) override;
	//uint32_t __FASTCALL read_io16(uint32_t addr) override;
	
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
	
	inline void request_update_screen()
	{
		//display_linebuf = render_linebuf.load();
	}
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

inline size_t TOWNS_CRTC::store1_aligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= 8) {
			src[x].store_aligned(dst);
			dst += 8;
			width -= 8;
			pixels_count += 8;
		} else {
			src[x].store_limited(dst, width);
			dst += width;
			pixels_count += width;
			width = 0;
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::store2_aligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= 16) {
			src[x].store2_aligned(dst);
			dst += 16;
			width -= 16;
			pixels_count += 16;
		} else {
			src[x].store2_limited(dst, width);
			dst += (2 * width);
			pixels_count += (2 * width);
			width = 0;
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::store4_aligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= 32) {
			src[x].store4_aligned(dst);
			dst += 32;
			width -= 32;
			pixels_count += 32;
		} else {
			src[x].store4_limited(dst, width);
			dst += (width * 4);
			pixels_count += (width * 4);
			width = 0;
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::store_n_any(scrntype_t *dst, csp_vector8<scrntype_t> *src, const int mag, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= (8 * mag)) {
			src[x].store_n(dst, mag);
			dst += (8 * mag);
			width -= (8 * mag);
			pixels_count += (8 * mag);
		} else {
			src[x].store_n_limited(dst, mag, width);
			dst += (width * mag);
			pixels_count += (width * mag);
			width = 0;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::store1_unaligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= 8) {
			src[x].store(dst);
			dst += 8;
			width -= 8;
			pixels_count += 8;
		} else {
			src[x].store_limited(dst, width);
			dst += width;
			pixels_count += width;
			width = 0;
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::store2_unaligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= 16) {
			src[x].store2(dst);
			dst += 16;
			width -= 16;
			pixels_count += 16;
		} else {
			src[x].store2_limited(dst, width);
			dst += (2 * width);
			pixels_count += (2 * width);
			width = 0;
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::store4_unaligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= 32) {
			src[x].store4(dst);
			dst += 32;
			width -= 32;
			pixels_count += 32;
		} else {
			src[x].store4_limited(dst, width);
			dst += (width * 4);
			pixels_count += (width * 4);
			width = 0;
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::scaling_store(scrntype_t *dst, csp_vector8<scrntype_t> *src, const int mag, const size_t words, size_t& width)
{
	__UNLIKELY_IF((dst == NULL) || (src == NULL)) return 0;

	uintptr_t dstval = (uintptr_t)dst;
	size_t pixels_count = 0;
	const uintptr_t as = alignof(csp_vector8<scrntype_t>) - 1;
	__LIKELY_IF((dstval & as) == 0) { // ALIGNED
		switch(mag) {
		case 1:
			pixels_count = store1_aligned(dst, src, words, width);
			break;
		case 2:
			pixels_count = store2_aligned(dst, src, words, width);
			break;
		case 4:
			pixels_count = store4_aligned(dst, src, words, width);
			break;
		default:
			pixels_count = store_n_any(dst, src, mag, words, width);
			break;
		}
	} else { // Not aligned
		switch(mag) {
		case 1:
			pixels_count = store1_unaligned(dst, src, words, width);
			break;
		case 2:
			pixels_count = store2_unaligned(dst, src, words, width);
			break;
		case 4:
			pixels_count = store4_unaligned(dst, src, words, width);
			break;
		default:
			pixels_count = store_n_any(dst, src, mag, words, width);
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::scaling_store_by_map(scrntype_t *dst, csp_vector8<scrntype_t> *src, csp_vector8<uint16_t> magx_map, const size_t words, size_t& width)
{
	if((words == 0) || (width == 0)) return 0;
	size_t pixels_count = 0;
	const size_t width_limit = width;
	size_t rwidth = width & 7;
	size_t __sum = 0;
	__DECL_VECTORIZED_LOOP
	for(size_t x = 0; x < 8; x++) {
		__sum += magx_map.at(x);
	}
	scrntype_t pix;
	size_t ptr = 0;
	for(size_t x = 0; x < words; x++) {
		__UNLIKELY_IF(pixels_count >= width_limit ) {
			pixels_count = width_limit;
			break;
		}
		__LIKELY_IF(width >= __sum) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				pix = src[x].at(i);
				for(size_t j = 0; j < magx_map.at(i); j++) {
					dst[ptr++] = pix; 
				}
			}
			width -= __sum;
			pixels_count += __sum;
		} else {
			__UNLIKELY_IF(rwidth == 0) {
				rwidth = 8; // Temporally value
			}
			for(size_t i = 0; i < rwidth; i++) {
				pix = src[x].at(i);
				for(size_t j = 0; j < magx_map.at(i); j++) {
					dst[ptr++] = pix;
					pixels_count++;
					__UNLIKELY_IF(pixels_count >= width_limit) {
						break;
					}
				}
				__UNLIKELY_IF(pixels_count >= width_limit) {
					pixels_count = width_limit;
					break;
				}
			}
			width = 0;
			break;
		}
	}
	return pixels_count;
}
}


#endif
