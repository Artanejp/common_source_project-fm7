/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns VRAM ]
	History: 2016.12.28 Initial.
*/

#ifndef _TOWNS_VRAM_H_
#define _TOWNS_VRAM_H_

#include "../vm.h"
#include "../emu.h"
#include "device.h"
#include "../../common.h"

#if defined(_USE_QT)
#include <QMutex>
#endif

// Older Towns.
#define TOWNS_VRAM_ADDR_MASK 0x7ffff
// VRAM DIRECT ACCESS: For Sprite. You should access with 16bit
// You can write raw data, drawing with colorkey is automatically.
#define SIG_TOWNS_TRANSFER_SPRITE_DATA 0x100000
#define SIG_TOWNS_SET_SPRITE_BANK      0x140000
#define SIG_TOWNS_CLEAR_SPRITE_BUFFER  0x140001
// Do render with any mode. You should set vline to arg.
#define SIG_TOWNS_RENDER_RASTER    0x01
#define SIG_TOWNS_RENDER_FULL      0x02
#define SIG_TOWNS_VRAM_VSTART      0x03
#define SIG_TOWNS_VRAM_VBLANK      0x04
#define SIG_TOWNS_VRAM_VSYNC       0x05
#define SIG_TOWNS_VRAM_HSYNC       0x06
#define SIG_TOWNS_VRAM_SET_VLINE   0x07
#define SIG_TOWNS_RENDER_FLAG      0x08

namespace FMTOWNS {
class TOWNS_VRAM : public DEVICE
{
protected:
	uint32_t page_modes[4];
	bool line_rendered[2][TOWNS_CRTC_MAX_LINES];
	
	scrntype_t *framebuffer0[2]; // Frame Buffer Layer 0. Not saved.
	scrntype_t *framebuffer1[2]; // Frame Buffer Layer 1. Not saved.

	int framebuffer_width[2][2]; // [Bank][Layer]
	int framebuffer_height[2][2];
	int framebuffer_stride[2][2];
	int framebuffer_size[2][2];

	uint16_t *vram_ptr[2];   // Layer [01] address.
	uint32_t vram_size[2];   // Layer [01] size [bytes].
	uint32_t vram_offset[2]; // Layer [01] address offset.
#if defined(_USE_QT)
	// If you use other framework, place mutex lock.
	QMutex vram_lock[2][2]; // [bank][layer];
#endif
	
	__DECL_ALIGNED(sizeof(scrntype_t) * 4) scrntype_t table_32768c[65536];
	__DECL_ALIGNED(sizeof(scrntype_t) * 4) scrntype_t alpha_32768c[65536];
	__DECL_ALIGNED(sizeof(uint16_t) * 8)   uint16_t   mask_32768c[65536];
	
	__DECL_ALIGNED(sizeof(scrntype_t) * 4) scrntype_t alpha_16c[8 * 8 * 2];
	__DECL_ALIGNED(sizeof(uint16_t) * 8)   uint16_t   mask_16c[8 * 8];
	
	uint32_t layer_virtual_width[2];
	uint32_t layer_virtual_height[2];
	uint32_t layer_display_width[2];
	uint32_t layer_display_height[2];

	bool access_page1;
	uint32_t write_plane_mask; // for plane-access.
	uint8_t packed_access_mask_lo;
	uint8_t packed_access_mask_hi;

	bool dirty_flag[0x80000 >> 3]; // Per 8bytes : 16pixels(16colors) / 8pixels(256) / 4pixels(32768)

	scrntype_t alpha_buffer_32768[0x80000 >> 1];
	scrntype_t alpha_buffer_16[0x80000 << 1];

	uint16_t mask_buffer_32768[0x80000 >> 1];
	uint16_t mask_buffer_32768_neg[0x80000 >> 1];
	uint8_t  mask_buffer_16[0x80000];
	uint8_t  mask_buffer_16_neg[0x80000];
	
	
	uint8_t vram[0x80000]; // Related by machine.

	// VRAM CONTROL REGISTER.
	uint8_t voutreg_num;  // I/O 0448h
	uint8_t voutreg_ctrl; // I/O 044Ah : voutreg_num = 0.
	uint8_t voutreg_prio; // I/O 044Ah : voutreg_num = 1.
	
	
	// FMR50 Compatible registers. They are mostly dummy.
	// Digital paletts. I/O FD98H - FD9FH.
	uint8_t r50_digital_palette[8];
	bool layer_display_flags[2]; // I/O FDA0H (WO) : bit3-2 (Layer1) or bit1-0 (Layer0).Not 0 is true.
	
	bool r50_dpalette_updated;   // I/O 044CH (RO) : bit7
	
	bool sprite_busy;            // I/O 044CH (RO) : bit1. Must update from write_signal().
	bool splite_disp_page;       // I/O 044CH (RO) : bit0. Must update from write_signal().
	
	// Around Analog palette.
	uint8_t apalette_code; // I/O FD90H (RW). 16 or 256 colors.
	uint8_t apalette_b;    // I/O FD92H (RW).
	uint8_t apalette_r;    // I/O FD94H (RW).
	uint8_t apalette_g;    // I/O FD96H (RW).
	uint16_t   apalette_16_rgb[2][16];   // R * 256 + G * 16 + B
	scrntype_t apalette_16_pixel[2][16]; // Not saved. Must be calculated.
	uint32_t   apalette_256_rgb[256];    // R * 65536 + G * 256 + B
	scrntype_t apalette_256_pixel[256];  // Not saved. Must be calculated.
	// Accessing VRAM. Will be separated.
	// Memory description:
	// All of accessing must be little endian.
	// 000C:00000 - 000C:07fff : Plane accessing window(->FM-R50 features?). Access to Layer #0 (8000:00000).
	// 000C:08000 - 000C:0ffff : I/O CVRAM
	// 000D:00000 - 000E:0ffff : Reserved (Window for KANJI, DIC etc).
	// 8000:00000 - 8000:3ffff : Plane accessing Layer #0.
	// 8000:40000 - 8000:7ffff : Plane accessing Layer #1.
	// 8010:00000 - 8010:7ffff : Plane accessing with one layer.
	// 8100:00000 - 8100:1ffff : Sprite (and text vram).
	// I/O 0458H (RW) : VRAM ACCESS CONTROLLER reg address.
	// I/O 045AH (RW) : VRAM ACCESS CONTROLLER reg data (LOW).
	// I/O 045BH (RW) : VRAM ACCESS CONTROLLER reg data (HIGH).
	pair_t packed_pixel_mask_reg; // '1' = Write. I/O 0458H - 045BH.
	//uint8_t *vram_addr;
	uint32_t vram_bytes;
	uint32_t layer_offset[4];
	uint8_t text_vram[4096]; // 4096bytes
	uint8_t kanji_vram[4096]; // 4096bytes
	// End.

	// Flags related by host renderer. Not saved.
	bool has_hardware_rendering;
	bool has_hardware_blending;
	// End.


public:
	TOWNS_VRAM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(vram, 0x00, sizeof(vram));
		for(int bank = 0; bank < 2; bank++) {
			for(int layer = 0; layer < 2; layer++) {
				framebuffer_width[bank][layer] = 640;
				framebuffer_height[bank][layer] = 480;
				framebuffer_stride[bank][layer] = 640 * sizeof(scrntype_t);
				framebuffer_size[bank][layer] = 640 * 480 * sizeof(scrntype_t);
				if(layer == 0) {
					framebuffer0[bank] = NULL;
				} else {
					framebuffer1[bank] = NULL;
				}
			}
		}
		page_modes[0] = page_modes[1] = page_modes[2] = page_modes[3] = 0;
		packed_access_mask_hi = packed_access_mask_lo = 0xff;
		write_plane_mask = 0xffffffff;
	}
	~TOWNS_VRAM() {}
	
	uint32_t read_data8(uint32_t addr);
	uint32_t read_data16(uint32_t addr);
	uint32_t read_data32(uint32_t addr);
	void write_data8(uint32_t addr, uint32_t data);
	void write_data16(uint32_t addr, uint32_t data);
	void write_data32(uint32_t addr, uint32_t data);

	uint32_t read_io8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	void draw_screen();
	void write_signal(int id, uint32_t data, uint32_t mask); // Do render

	// Unique Functions
	uint32_t read_plane_data8(uint32_t addr);
	uint32_t read_plane_data16(uint32_t addr);
	uint32_t read_plane_data32(uint32_t addr);

	void get_vramctrl_regs(uint8_t &ctrl, uint8_t &prio)
	{
		ctrl = voutreg_ctrl; // I/O 044Ah : voutreg_num = 0.
		prio = voutreg_prio; // I/O 044Ah : voutreg_num = 1.
	}
	uint8_t* get_vram_address(uint32_t offset)
	{
		if(offset >= 0x80000) return NULL; // ToDo
		return &(vram[offset]);
	}
	uint32_t get_vram_size()
	{
		return 0x80000; // ToDo
	}
	void lock_framebuffer(int layer, int bank)
	{
#if defined(_USE_QT)
		vram_lock[bank][layer].lock();
#endif
	}
	void unlock_framebuffer(int layer, int bank)
	{
#if defined(_USE_QT)
		vram_lock[bank][layer].unlock();
#endif
	}
	
	// New APIs?
	void write_plane_data8(uint32_t addr, uint32_t data);
	void write_plane_data16(uint32_t addr, uint32_t data);
	void write_plane_data32(uint32_t addr, uint32_t data);

	void set_frame_buffer(int layer, bool buffer1, scrntype_t *framebuffer, int width, int height);
	scrntype_t *get_frame_buffer_ptr(int layer);
	int  get_frame_buffer_width(int layer);
	int  get_frame_buffer_height(int layer);
	bool is_display(int layer);
	bool is_updated(int layer, int line_num);
	void lock_frame_buffer(int layer);
	void unlock_frame_buffer(int layer);
	void set_render_features(bool blending_from_buffer, bool rendering_framebuffer);
	// End.
	
	void set_context_renderbuffer(scrntype_t *p, int layer, int bank, uint32_t width, uint32_t height, uint32_t stride){
		if((layer > 1) || (layer < 0)) return;
		if((bank > 1) || (bank < 0)) return;
		lock_framebuffer(layer, bank);
		framebuffer_width[bank][layer] = width;
		framebuffer_height[bank][layer] = height;
		framebuffer_stride[bank][layer] = stride * sizeof(scrntype_t);
		framebuffer_size[bank][layer] = stride * height * sizeof(scrntype_t);
		if(layer == 0) {
			framebuffer0[bank] = p;
		} else {
			framebuffer1[bank] = p;
		}
		unlock_framebuffer(layer, bank);
	
	};
};

}
#endif
