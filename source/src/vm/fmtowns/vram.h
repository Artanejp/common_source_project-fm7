/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns VRAM ]
	History: 2016.12.28 Initial.
*/

#ifndef _TOWNS_VRAM_H_
#define _TOWNS_VRAM_H_

#include "device.h"
#include "../../common.h"
#include "towns_common.h"

#if defined(_USE_QT)
#include <QMutex>
#endif

// Older Towns.
#define TOWNS_VRAM_ADDR_MASK 0x7ffff
// VRAM DIRECT ACCESS: For Sprite. You should access with 16bit
// You can write raw data, drawing with colorkey is automatically.
#define SIG_TOWNS_TRANSFER_SPRITE_DATA   0x100000
#define SIG_TOWNS_SET_SPRITE_BANK        0x140000
#define SIG_TOWNS_CLEAR_SPRITE_BUFFER    0x140001
// Do render with any mode. You should set vline to arg.
#define SIG_TOWNS_RENDER_RASTER          0x01
#define SIG_TOWNS_RENDER_FULL            0x02
#define SIG_TOWNS_VRAM_VSTART            0x03
#define SIG_TOWNS_VRAM_VBLANK            0x04
#define SIG_TOWNS_VRAM_VSYNC             0x05
#define SIG_TOWNS_VRAM_HSYNC             0x06
#define SIG_TOWNS_VRAM_SET_VLINE         0x07
#define SIG_TOWNS_RENDER_FLAG            0x08
#define SIG_TOWNS_VRAM_FRAMEBUFFER_READY 0x10
#define SIG_TOWNS_VRAM_SWAP_FRAMEBUFFER  0x11

namespace FMTOWNS {
class TOWNS_VRAM : public DEVICE
{
protected:
	DEVICE* d_sprite;
	DEVICE* d_crtc;
	
#if defined(_USE_QT)
	// If you use other framework, place mutex lock.
	QMutex vram_lock[2][2]; // [bank][layer];
#endif
	
	bool access_page1;

	bool dirty_flag[0x80000 >> 3]; // Per 8bytes : 16pixels(16colors) / 8pixels(256) / 4pixels(32768)
	
	// FMR50 Compatible registers. They are mostly dummy.
	// Digital paletts. I/O FD98H - FD9FH.
	bool layer_display_flags[2]; // I/O FDA0H (WO) : bit3-2 (Layer1) or bit1-0 (Layer0).Not 0 is true.
	
	
	bool sprite_busy;            // I/O 044CH (RO) : bit1. Must update from write_signal().
	bool sprite_disp_page;       // I/O 044CH (RO) : bit0. Must update from write_signal().
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
	uint8_t vram_access_reg_addr;
	pair32_t packed_pixel_mask_reg; // '1' = Write. I/O 0458H - 045BH.
	uint8_t vram[0x80000]; // Related by machine.
	// End.

	// Flags related by host renderer. Not saved.
	bool has_hardware_rendering;
	bool has_hardware_blending;
	// End.

public:
	TOWNS_VRAM(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(vram, 0x00, sizeof(vram));
		d_sprite = NULL;
		d_crtc = NULL;
		set_device_name(_T("FM-Towns VRAM"));
	}
	~TOWNS_VRAM() {}

	virtual void initialize();
	virtual void reset();
	
	virtual uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr);
	virtual uint32_t __FASTCALL read_memory_mapped_io16(uint32_t addr);
	virtual uint32_t __FASTCALL read_memory_mapped_io32(uint32_t addr);
	virtual void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data);
	virtual void __FASTCALL write_memory_mapped_io16(uint32_t addr, uint32_t data);
	virtual void __FASTCALL write_memory_mapped_io32(uint32_t addr, uint32_t data);
	
	virtual void __FASTCALL write_io8(uint32_t address, uint32_t data);
	virtual void __FASTCALL write_io16(uint32_t address, uint32_t data);
	
	virtual uint32_t __FASTCALL read_io8(uint32_t address);
	virtual uint32_t __FASTCALL read_io16(uint32_t address);
	
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask); // Do render
	virtual bool process_state(FILEIO* state_fio, bool loading);

	// Unique Functions
	virtual uint8_t* __FASTCALL get_vram_address(uint32_t offset)
	{
		if(offset >= 0x80000) return NULL; // ToDo
		return &(vram[offset]);
	}
	virtual bool __FASTCALL set_buffer_to_vram(uint32_t offset, uint8_t *buf, int words)
	{
		offset &= 0x7ffff;
//		if(words > 16) return false;
		if(words <= 0) return false;
		uint8_t* p = &(vram[offset]);
		if((offset + (words << 1)) <= 0x80000) {
			memcpy(p, buf, words << 1);
		} else {
			int nb = 0x80000 - offset;
			memcpy(p, buf, nb);
			int nnb = (words << 1) - nb;
			if(nnb > 0) {
				memcpy(vram, &(buf[nb]), nnb);
			}
		}
		return true;
	}
	virtual bool __FASTCALL get_vram_to_buffer(uint32_t offset, uint8_t *buf, int words)
	{
		offset &= 0x7ffff;
//		if(words > 16) return false;
		if(words <= 0) return false;
		uint8_t* p = &(vram[offset]);
		if((offset + (words << 1)) <= 0x80000) {
			memcpy(buf, p, words << 1);
		} else {
			uint32_t nb = 0x80000 - offset;
			memcpy(buf, p, nb);
			int nnb = (words << 1) - nb;
			if(nnb > 0) {
				memcpy(&(buf[nb]), vram, nnb);
			}
		}
		return true;
	}
	virtual void __FASTCALL make_dirty_vram(uint32_t addr, int bytes);
	virtual uint32_t __FASTCALL get_vram_size()
	{
		return 0x80000; // ToDo
	}
	virtual void __FASTCALL lock_framebuffer(int layer, int bank)
	{
#if defined(_USE_QT)
		vram_lock[bank][layer].lock();
#endif
	}
	virtual void __FASTCALL unlock_framebuffer(int layer, int bank)
	{
#if defined(_USE_QT)
		vram_lock[bank][layer].unlock();
#endif
	}
	void set_context_sprite(DEVICE *dev)
	{
		d_sprite = dev;
	}
	void set_context_crtc(DEVICE *dev)
	{
		d_crtc = dev;
	}
	// New APIs?
	// End.
};

}
#endif
