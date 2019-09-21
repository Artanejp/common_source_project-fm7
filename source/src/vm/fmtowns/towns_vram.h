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
	
	uint16_t *vram_ptr[2];   // Layer [01] address.
	uint32_t vram_size[2];   // Layer [01] size [bytes].
	uint32_t vram_offset[2]; // Layer [01] address offset.
#if defined(_USE_QT)
	// If you use other framework, place mutex lock.
	QMutex vram_lock[2][2]; // [bank][layer];
#endif
	
	bool access_page1;
	uint32_t write_plane_mask; // for plane-access.
	uint8_t packed_access_mask_lo;
	uint8_t packed_access_mask_hi;

	bool dirty_flag[0x80000 >> 3]; // Per 8bytes : 16pixels(16colors) / 8pixels(256) / 4pixels(32768)

	
	
	// FMR50 Compatible registers. They are mostly dummy.
	// Digital paletts. I/O FD98H - FD9FH.
	uint8_t r50_digital_palette[8];
	bool layer_display_flags[2]; // I/O FDA0H (WO) : bit3-2 (Layer1) or bit1-0 (Layer0).Not 0 is true.
	
	bool r50_dpalette_updated;   // I/O 044CH (RO) : bit7
	
	bool sprite_busy;            // I/O 044CH (RO) : bit1. Must update from write_signal().
	bool splite_disp_page;       // I/O 044CH (RO) : bit0. Must update from write_signal().
	uint8_t mix_reg;             // MMIO 000CH:FF80H
	uint8_t r50_readplane;       // MMIO 000CH:FF81H : BIT 7 and 6.
	uint8_t r50_ramsel;          // MMIO 000CH:FF81H : BIT 3 to 0.
	uint8_t r50_gvramsel;        // MMIO 000CH:FF83H : bit4 (and 3).
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
	pair32_t packed_pixel_mask_reg; // '1' = Write. I/O 0458H - 045BH.
	uint8_t vram[0x80000]; // Related by machine.
	// End.

	// Flags related by host renderer. Not saved.
	bool has_hardware_rendering;
	bool has_hardware_blending;
	// End.


public:
	TOWNS_VRAM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(vram, 0x00, sizeof(vram));
	}
	~TOWNS_VRAM() {}
	
	virtual uint32_t read_memory_mapped_io8(uint32_t addr);
	virtual uint32_t read_memory_mapped_io16(uint32_t addr);
	virtual uint32_t read_memory_mapped_io32(uint32_t addr);
	virtual void write_memory_mapped_io8(uint32_t addr, uint32_t data);
	virtual void write_memory_mapped_io16(uint32_t addr, uint32_t data);
	virtual void write_memory_mapped_io32(uint32_t addr, uint32_t data);

	uint32_t read_io8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);

	
	void write_signal(int id, uint32_t data, uint32_t mask); // Do render

	// Unique Functions
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
	// End.
};

}
#endif
