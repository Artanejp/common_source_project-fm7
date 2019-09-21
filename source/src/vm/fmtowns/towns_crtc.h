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

#define SIG_TOWNS_CRTC_HSYNC          1
#define SIG_TOWNS_CRTC_VSYNC          2
#define SIG_TOWNS_CRTC_FIELD          3
#define SIG_TOWNS_CRTC_VDISP0         4
#define SIG_TOWNS_CRTC_VDISP1         5
#define SIG_TOWNS_CRTC_HDISP0         6
#define SIG_TOWNS_CRTC_HDISP1         7
#define SIG_TOWNS_CRTC_MMIO_CF882H    8
#define SIG_TOWNS_CRTC_SPRITE_BUFFER  9
#define SIG_TOWNS_CRTC_SPRITE_DISP    10

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

	enum {
		DISPMODE_NONE = 0,
		DISPMODE_256,
		DISPMODE_32768,
		DISPMODE_16,
	};
}

namespace FMTOWNS {
	
typedef struct {
	int32_t mode[2];
	int32_t pixels[2];
	int32_t mag[2];
	int32_t num[2];
	uint32_t prio;
	uint32_t pad[7];
	uint8_t pixels_layer[2][TOWNS_CRTC_MAX_PIXELS]; // RAW VALUE
} linebuffer_t;

class TOWNS_VRAM;
class TOWNS_SPRITE;
class TOWNS_CRTC : public DEVICE
{
protected:
	TOWNS_VRAM* d_vram;
	TOWNS_SPRITE* d_sprite;
	// output signals
	outputs_t outputs_int_vsync;  // Connect to int 11.
	uint16_t regs[32];      // I/O 0442H, 0443H
	bool regs_written[32];
	uint8_t crtc_ch;         // I/O 0440H
	bool timing_changed[2];
	bool address_changed[2];
	bool mode_changed[2];
	
	uint8_t display_mode[2]; 
	bool display_enabled;
	
	double crtc_clock; // 
	// They are not saved.Must be calculate when loading.
	double horiz_us, next_horiz_us; // (HST + 1) * clock
	double horiz_width_posi_us, horiz_width_nega_us; // HSW1, HSW2
	double vert_us, next_vert_us; // (VST +1) * horiz_us / 2.0
	double vstart_us;
	double vert_sync_pre_us; // VST1 * horiz_us / 2.0
	double vert_sync_end_us; // VST2 * horiz_us / 2.0
	double eet_us;
	double frame_us;
	
	bool req_recalc;
	// End

	double frames_per_sec;

	uint32_t vstart_addr[2];  // VSTART ADDRESS
    uint32_t hstart_words[2]; // HSTART ((HDS[01] * clock) : Horizonal offset words (Related by ZH[01]). Maybe 0.
    uint32_t hend_words[2];   // HEND   ((HDE[01] * clock) : Horizonal offset words (Related by ZH[01]). Maybe 0.
    uint32_t vstart_lines[2]; // VSTART ((VDS[01] * clock) : Horizonal offset words (Related by VH[01]).
    uint32_t vend_lines[2];   // VEND   ((VDE[01] * clock) : Horizonal offset words (Related by VH[01]).
	uint32_t frame_offset[2]; // FO.
	uint32_t head_address[2];
	
	uint8_t zoom_factor_vert[2]; // Related display resolutions of two layers and zoom factors.
	uint8_t zoom_factor_horiz[2]; // Related display resolutions of two layers and zoom factors.
	uint8_t zoom_count_vert[2];
	
	uint32_t line_count[2]; // Separate per layer.
	
	int vert_line_count; // Not separate per layer.Total count.
	// Note: To display to real screen, use blending of OpenGL/DirectX
	//       from below framebuffers per layer if you can.
	//       And, recommand to use (hardware) shader to rendering (= not to use framebuffer of below) if enabled.
	//       Not recommanded to use draw_screen() type rendering.
	//       Rendering precesses may be heavily to use only CPU (I think). 20170107 K.Ohta.
	
	// Not Saved?.
	// End.
	
	bool vdisp, vblank, vsync, hsync, hdisp[2], frame_in[2];

	// around sprite
	uint8_t sprite_disp_page;
	bool sprite_enabled;
	
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
	int event_id_hstart;
	int event_id_vds[2];
	int event_id_vde[2];
	int event_id_hds[2];
	int event_id_hde[2];

	int display_linebuf;
	linebuffer_t *linebuffers[4];
	
	void set_display(bool val);
	void set_vblank(bool val);
	void set_vsync(bool val);
	void set_hsync(bool val);
	void transfer_line(int line);

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
		for(int i = 0; i < 4; i++) {
			linebuffers[i] = NULL;
		}
		d_sprite = NULL;
		d_vram = NULL;
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
	linebuffer_t* get_line_buffer(int page, int line)
	{
		page = page & 1;
		if(line < 0) return NULL;
		if(line >= TOWNS_CRTC_MAX_LINES) return NULL;
		if(linebuffers[page] == NULL) return NULL;
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
	
	void set_context_vsync(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_int_vsync, device, id, mask);
	}
	uint16_t* get_regs_ptr()
	{
		return regs;
	}

};
}

#endif

