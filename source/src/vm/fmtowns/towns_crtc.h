/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns CRTC ]
	History: 2016.12.28 Initial from HD46505 .
*/

#ifndef _TOWNS_CRTC_H_
#define _TOWNS_CRTC_H_

#include "../vm.h"
#include "../emu.h"
#include "device.h"

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
#define TOWNS_CRTC_MAX_LINES  1024
#define TOWNS_CRTC_MAX_PIXELS 1024
namespace FMTOWNS {

	enum {
		TOWNS_CRTC_REG_HSW1 = 0,
		TOWNS_CRTC_REG_HSW2 = 1,
		TOWNS_CRTC_REG_HST  = 4,
		TOWNS_CRTC_REG_VST1,
		TOWNS_CRTC_REG_VST2,
		TOWNS_CRTC_REG_EET,
	
		TOWNS_CRTC_REG_VST, // 8
		TOWNS_CRTC_REG_HDS0,
		TOWNS_CRTC_REG_HDE0,
		TOWNS_CRTC_REG_HDS1,
		TOWNS_CRTC_REG_HDE1,
		TOWNS_CRTC_REG_VDS0,
		TOWNS_CRTC_REG_VDE0,
		TOWNS_CRTC_REG_VDS1,
	
		TOWNS_CRTC_REG_VDE1,
		TOWNS_CRTC_REG_FA0,
		TOWNS_CRTC_REG_HAJ0,
		TOWNS_CRTC_REG_FO0,
		TOWNS_CRTC_REG_LO0,
		TOWNS_CRTC_REG_FA1,
		TOWNS_CRTC_REG_HAJ1,
		TOWNS_CRTC_REG_FO1,
	
		TOWNS_CRTC_REG_LO1,
		TOWNS_CRTC_REG_EHAJ,
		TOWNS_CRTC_REG_EVAJ,
		TOWNS_CRTC_REG_ZOOM,
		TOWNS_CRTC_REG_DISPMODE, // 28
		TOWNS_CRTC_REG_CLK, // 29
		TOWNS_CRTC_REG_DUMMY, // 30
		TOWNS_CRTC_REG_CTRL, // 31
	};
}

namespace FMTOWNS {
class TOWNS_CRTC : public DEVICE
{
private:
	// output signals
	outputs_t outputs_int_vsync;  // Connect to int 11.
	uint16_t regs[32];      // I/O 0442H, 0443H
	bool regs_written[32];
	uint8_t reg_ch;         // I/O 0440H
	bool timing_changed;

	// Not include around video input/dizitize features yet.
	bool line_changed[2][TOWNS_CRTC_MAX_LINES];

	double crtc_clock; // 
	// They are not saved.Must be calculate when loading.
	double horiz_us, next_horiz_us; // (HST + 1) * clock
	double horiz_width_posi_us, horiz_width_nega_us; // HSW1, HSW2
	double vert_us, next_vert_us; // (VST +1) * horiz_us / 2.0
	double vstart_us;
	double vert_sync_pre_us; // VST1 * horiz_us / 2.0
	double vert_sync_end_us; // VST2 * horiz_us / 2.0
	// End

	double frames_per_sec;
	
    uint32_t hstart_words[2]; // HSTART ((HDS[01] * clock) : Horizonal offset words (Related by ZH[01]). Maybe 0.
    uint32_t hend_words[2];   // HEND   ((HDE[01] * clock) : Horizonal offset words (Related by ZH[01]). Maybe 0.
    uint32_t vstart_lines[2]; // VSTART ((VDS[01] * clock) : Horizonal offset words (Related by VH[01]).
    uint32_t vend_lines[2];   // VEND   ((VDE[01] * clock) : Horizonal offset words (Related by VH[01]).

	uint32_t zoom_factor_vert; // Related display resolutions of two layers and zoom factors. Multiplied by 1024.
	uint32_t zoom_factor_horiz; // Related display resolutions of two layers and zoom factors. Multiplied by 1024.
	
	uint32_t line_count[2]; // Separate per layer.
	
	int vert_line_count; // Not separate per layer.Total count.

	int buffer_num; // Frame buffer number.
	// Note: To display to real screen, use blending of OpenGL/DirectX
	//       from below framebuffers per layer if you can.
	//       And, recommand to use (hardware) shader to rendering (= not to use framebuffer of below) if enabled.
	//       Not recommanded to use draw_screen() type rendering.
	//       Rendering precesses may be heavily to use only CPU (I think). 20170107 K.Ohta.
	
	// Not Saved?.
	uint8_t layer_colors[2];
	uint16_t *vram_ptr[2];   // Layer [01] address.
	uint32_t vram_size[2];   // Layer [01] size [bytes].
	uint32_t vram_offset[2]; // Layer [01] address offset.
	
	uint32_t layer_virtual_width[2];
	uint32_t layer_virtual_height[2];
	uint32_t layer_display_width[2];
	uint32_t layer_display_height[2];
	// End.
	
	bool vdisp, vblank, vsync, hsync, hdisp, frame_in;

	// Video output controller (I/O 0448H, 044AH)
	// Register 00 : Display mode.
	bool display_on[2]; // CL11-CL00 : bit3-0
	bool one_layer_mode; // PMODE : bit4
	// Register 11: Priority mode.
	uint8_t vout_palette_mode;
	bool vout_ys;
	bool video_brightness; // false = high.
	bool layer1_front; // if false, layer 0 is front.
	
	// Video output registers. May be split to separate files?
	// FMR50 Compatible registers. They are mostly dummy.

	// MMIO? 000C:FF80H
	uint8_t r50_mix_reg;
	// MMIO? 000C:FF81H
	uint8_t r50_update_mode;
	// MMIO? 000C:FF82H Not dummy?
	uint8_t r50_dispmode;
	// MMIO? 000C:FF83H Not dummy?
	uint8_t r50_pagesel;
	// MMIO? 000C:FF86H Not dummy?
	uint8_t r50_status;
	// I/O FDA0H Not Dummy?
	//uint8_t r50_sub_statreg;

	
	// Others.
	uint8_t video_out_reg_addr;  // I/O 0448H
	uint8_t video_out_reg_data;  // I/O 044AH
	uint8_t video_out_regs[2];
	// End.

	
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
	int event_id_vblank;
	
	void set_display(bool val);
	void set_vblank(bool val);
	void set_vsync(bool val);
	void set_hsync(bool val);

protected:
	bool render_a_line(int layer, int linenum, int xoffset, uint8_t *vramptr, uint32_t words);
	void render_line_16(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t words);
	void render_line_256(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t words);
	void render_line_32768(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t words);
	void render_clear(int layer, scrntype_t *framebuffer);
	
public:
	TOWNS_CRTC(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_int_vsync);
		set_device_name(_T("FM-Towns CRTC"));
	}
	~TOWNS_CRTC() {}

	void initialize();
	void reset();
	
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch);

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	void write_io16(uint32_t addr, uint32_t data);
	uint32_t read_io16(uint32_t addr);
	
	uint32_t read_data8(uint32_t addr);
	uint32_t read_data16(uint32_t addr);
	uint32_t read_data32(uint32_t addr);
	
	void write_data8(uint32_t addr, uint32_t data);
	void write_data16(uint32_t addr, uint32_t data);
	void write_data32(uint32_t addr, uint32_t data);
	void event_callback(int event_id, int err);
	//void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	// unique function
	void set_context_vsync(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_vsync, device, id, mask);
	}
	uint16_t* get_regs_ptr()
	{
		return regs;
	}

};
}

#endif

