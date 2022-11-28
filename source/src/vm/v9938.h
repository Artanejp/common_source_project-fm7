// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Nathan Woods
/***************************************************************************

    v9938 / v9958 emulation

***************************************************************************/

#pragma once

#ifndef __V9938_H__
#define __V9938_H__

#include "vm.h"
#include "../emu.h"
#include "device.h"



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************
/*
#define MCFG_V9938_ADD(_tag, _screen, _vramsize, _clock) \
	MCFG_DEVICE_ADD(_tag, V9938, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen) \
	v9938_device::static_set_vram_size(*device, _vramsize);
#define MCFG_V9958_ADD(_tag, _screen, _vramsize, _clock) \
	MCFG_DEVICE_ADD(_tag, V9958, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen) \
	v9938_device::static_set_vram_size(*device, _vramsize);

#define MCFG_V99X8_SCREEN_ADD_NTSC(_screen_tag, _v9938_tag, _clock) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER) \
	MCFG_SCREEN_RAW_PARAMS(_clock, \
		v99x8_device::HTOTAL, \
		0, \
		v99x8_device::HVISIBLE - 1, \
		v99x8_device::VTOTAL_NTSC * 2, \
		v99x8_device::VERTICAL_ADJUST * 2, \
		v99x8_device::VVISIBLE_NTSC * 2 - 1 - v99x8_device::VERTICAL_ADJUST * 2) \
	MCFG_SCREEN_UPDATE_DEVICE(_v9938_tag, v9938_device, screen_update) \
	MCFG_SCREEN_PALETTE(_v9938_tag":palette")

#define MCFG_V99X8_SCREEN_ADD_PAL(_screen_tag, _v9938_tag, _clock) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER) \
	MCFG_SCREEN_RAW_PARAMS(_clock, \
		v99x8_device::HTOTAL, \
		0, \
		v99x8_device::HVISIBLE - 1, \
		v99x8_device::VTOTAL_PAL * 2, \
		v99x8_device::VERTICAL_ADJUST * 2, \
		v99x8_device::VVISIBLE_PAL * 2 - 1 - v99x8_device::VERTICAL_ADJUST * 2) \
	MCFG_SCREEN_UPDATE_DEVICE(_v9938_tag, v9938_device, screen_update) \
	MCFG_SCREEN_PALETTE(_v9938_tag":palette")

#define MCFG_V99X8_INTERRUPT_CALLBACK(_irq) \
	downcast<v99x8_device *>(device)->set_interrupt_callback(DEVCB_##_irq);
*/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
//extern const device_type V9938;
//extern const device_type V9958;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> v99x8_device

class v99x8_device :    public DEVICE
{
protected:
	// construction/destruction
	//v99x8_device(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu){}
	v99x8_device(VM_TEMPLATE* parent_vm, EMU* parent_emu);

public:
	//template<class _irq> void set_interrupt_callback(_irq irq) {
	//	m_int_callback.set_callback(irq);
	//}
	int get_transpen();
	//bitmap_ind16 &get_bitmap() { return m_bitmap; }
	//void update_mouse_state(int mx_delta, int my_delta, int button_state);

	//UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	//DECLARE_READ8_MEMBER( read );
	//DECLARE_WRITE8_MEMBER( write );

	UINT8 vram_r();
	UINT8 status_r();
	void palette_w(UINT8 data);
	void vram_w(UINT8 data);
	void command_w(UINT8 data);
	void register_w(UINT8 data);

	//static void static_set_vram_size(device_t &device, UINT32 vram_size);

	/* RESET pin */
	//void reset_line(int state) { if (state==ASSERT_LINE) device_reset(); }

	static const int HTOTAL = 684;
	static const int HVISIBLE = 544;
	static const int VTOTAL_NTSC = 262;
	static const int VTOTAL_PAL = 313;
	static const int VVISIBLE_NTSC = 26 + 192 + 25;
	static const int VVISIBLE_PAL = 53 + 192 + 49;
	// Looking at some youtube videos of real units on real monitors
	// there appear to be small vertical timing differences. Some (LCD)
	// monitors show the full borders, other CRT monitors seem to
	// display ~5 lines less at the top and bottom of the screen.
	static const int VERTICAL_ADJUST = 5;
	static const int TOP_ERASE = 13;
	static const int VERTICAL_SYNC = 3;

protected:
	//static const device_timer_id TIMER_LINE = 0;
	//const address_space_config      m_space_config;
	//address_space*                  m_vram_space;

	int m_model;

	// device overrides
	//virtual void device_start() override;
	//virtual void device_reset() override;
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void device_start();
	void device_reset();
	void device_timer(int v);

	// device_memory_interface overrides
	//virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_DATA) const override { return (spacenum == AS_DATA) ? &m_space_config : nullptr; }

	void configure_pal_ntsc();
	void set_screen_parameters();

private:
	// internal helpers
	inline int position_offset(UINT8 value) { value &= 0x0f; return (value < 8) ? -value : 16 - value; }
	void reset_palette();
	void vram_write(int offset, int data);
	int vram_read(int offset);
	void check_int();
	void register_write(int reg, int data);

	void default_border(const scrntype_t *pens, scrntype_t *ln);
	void graphic7_border(const scrntype_t *pens, scrntype_t *ln);
	void graphic5_border(const scrntype_t *pens, scrntype_t *ln);
	void mode_text1(const scrntype_t *pens, scrntype_t *ln, int line);
	void mode_text2(const scrntype_t *pens, scrntype_t *ln, int line);
	void mode_multi(const scrntype_t *pens, scrntype_t *ln, int line);
	void mode_graphic1(const scrntype_t *pens, scrntype_t *ln, int line);
	void mode_graphic23(const scrntype_t *pens, scrntype_t *ln, int line);
	void mode_graphic4(const scrntype_t *pens, scrntype_t *ln, int line);
	void mode_graphic5(const scrntype_t *pens, scrntype_t *ln, int line);
	void mode_graphic6(const scrntype_t *pens, scrntype_t *ln, int line);
	void mode_graphic7(const scrntype_t *pens, scrntype_t *ln, int line);
//  template<typename _PixelType, int _Width> void mode_yae(const scrntype_t *pens, _PixelType *ln, int line);
//  template<typename _PixelType, int _Width> void mode_yjk(const scrntype_t *pens, _PixelType *ln, int line);
	void mode_unknown(const scrntype_t *pens, scrntype_t *ln, int line);
	void default_draw_sprite(const scrntype_t *pens, scrntype_t *ln, UINT8 *col);
	void graphic5_draw_sprite(const scrntype_t *pens, scrntype_t *ln, UINT8 *col);
	void graphic7_draw_sprite(const scrntype_t *pens, scrntype_t *ln, UINT8 *col);

	void sprite_mode1(int line, UINT8 *col);
	void sprite_mode2(int line, UINT8 *col);
	void set_mode();
	void refresh_16(int line);
	void refresh_line(int line);

	void interrupt_start_vblank();

	int VDPVRMP(UINT8 M, int MX, int X, int Y);

	UINT8 VDPpoint5(int MXS, int SX, int SY);
	UINT8 VDPpoint6(int MXS, int SX, int SY);
	UINT8 VDPpoint7(int MXS, int SX, int SY);
	UINT8 VDPpoint8(int MXS, int SX, int SY);

	UINT8 VDPpoint(UINT8 SM, int MXS, int SX, int SY);

	void VDPpsetlowlevel(int addr, UINT8 CL, UINT8 M, UINT8 OP);

	void VDPpset5(int MXD, int DX, int DY, UINT8 CL, UINT8 OP);
	void VDPpset6(int MXD, int DX, int DY, UINT8 CL, UINT8 OP);
	void VDPpset7(int MXD, int DX, int DY, UINT8 CL, UINT8 OP);
	void VDPpset8(int MXD, int DX, int DY, UINT8 CL, UINT8 OP);

	void VDPpset(UINT8 SM, int MXD, int DX, int DY, UINT8 CL, UINT8 OP);

	int get_vdp_timing_value(const int *);

	void srch_engine();
	void line_engine();
	void lmmv_engine();
	void lmmm_engine();
	void lmcm_engine();
	void lmmc_engine();
	void hmmv_engine();
	void hmmm_engine();
	void ymmm_engine();
	void hmmc_engine();

	inline bool v9938_second_field();

	void cpu_to_vdp(UINT8 V);
	UINT8 vdp_to_cpu();
	void report_vdp_command(UINT8 Op);
	UINT8 command_unit_w(UINT8 Op);
	void update_command();

	// general
	int m_offset_x, m_offset_y, m_visible_y, m_mode;
	// palette
	int m_pal_write_first, m_cmd_write_first;
	UINT8 m_pal_write, m_cmd_write;
	UINT8 m_pal_reg[32], m_stat_reg[10], m_cont_reg[48], m_read_ahead;
	UINT8 m_v9958_sp_mode;

	// memory
	UINT16 m_address_latch;
	int m_vram_size;

	// interrupt
	UINT8 m_int_state;
	//devcb_write_line   m_int_callback;
	int m_scanline;
	// blinking
	int m_blink, m_blink_count;
	// mouse
	UINT8 m_mx_delta, m_my_delta;
	// mouse & lightpen
	UINT8 m_button_state;
	// palette
	UINT16 m_pal_ind16[16];
	UINT16 m_pal_ind256[256];
	// render bitmap
	//bitmap_ind16 m_bitmap;
	// Command unit
	typedef struct mmc_s {
		int SX,SY;
		int DX,DY;
		int TX,TY;
		int NX,NY;
		int MX;
		int ASX,ADX,ANX;
		UINT8 CL;
		UINT8 LO;
		UINT8 CM;
		UINT8 MXS, MXD;
	} mmc_t;
	mmc_t m_mmc;
	int  m_vdp_ops_count;
	void (v99x8_device::*m_vdp_engine)();

	struct v99x8_mode
	{
		UINT8 m;
		void (v99x8_device::*visible_16)(const scrntype_t *, scrntype_t*, int);
		void (v99x8_device::*border_16)(const scrntype_t *, scrntype_t*);
		void (v99x8_device::*sprites)(int, UINT8*);
		void (v99x8_device::*draw_sprite_16)(const scrntype_t *, scrntype_t*, UINT8*);
	} ;
	static const v99x8_mode s_modes[];
	//required_device<palette_device> m_palette;
	//emu_timer *m_line_timer;
	UINT8 m_pal_ntsc;
	int m_scanline_start;
	int m_vblank_start;
	int m_scanline_max;
	int m_height;
protected:
	static UINT16 s_pal_indYJK[0x20000];

	/*
	   for common source code project
	 */
private:
	// output signals
	outputs_t outputs_irq;
	uint8_t vram[0x20000];
	scrntype_t screen[(512 + 32)*246*2]; // (LONG_WIDTH) * (m_vblank_start*2)
	/* for m_vram_space of MAME/MESS */
	inline int read_byte(int address){if (address<m_vram_size) return vram[address];else return 0xff;}
	inline void write_byte(int address, int val){if (address<m_vram_size) vram[address]=val;}
	/* for m_palette and palette of MAME/MESS */
	scrntype_t pens[19780];
	void save_load_state(FILEIO* state_fio, bool is_save);
protected:
	/* for m_palette and palette of MAME/MESS */
	virtual void init_palette()=0;
	inline void set_pen_color(int index, int r, int g, int b){if (index<19780) pens[index]=RGB_COLOR(r,g,b);}
	inline int pal3bit(int i) {return (i&7)*36;}
	inline int pal5bit(int i) {return (i&31)<<3;}
public:
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_vline(int v, int clock);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	const _TCHAR *get_device_name()
	{
		return _T("v99x8");
	}
	
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void draw_screen();
};


class v9938_device : public v99x8_device
{
public:
	v9938_device(VM_TEMPLATE* parent_vm, EMU* parent_emu);

	//DECLARE_PALETTE_INIT(v9938);
protected:
	//virtual machine_config_constructor device_mconfig_additions() const override;
	void init_palette();
};

class v9958_device : public v99x8_device
{
public:
	v9958_device(VM_TEMPLATE* parent_vm, EMU* parent_emu);

	//DECLARE_PALETTE_INIT(v9958);

protected:
	//virtual machine_config_constructor device_mconfig_additions() const override;
	void init_palette();
};

#define SIG_VDP_COMMAND_COMPLETION	0

#endif

/*
	Common Source Code Project
	MSX Series (experimental)

	Origin : mame0172s.zip
		mame.zip\src\devices\video\v9938.h
	modified by umaiboux
	Date   : 2016.04.xx-

	[ V99x8 ]
*/
