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

#include <mutex>

//<! @note set below when using multi threaded vram accessing
//#define USE_TOWNS_VRAM_PARALLEL_ACCESS

// Older Towns.
#if !defined(TOWNS_VRAM_ADDR_SHIFT)
#define TOWNS_VRAM_ADDR_SHIFT (16 + 3)
#endif
// Normally, 0x7ffff
#define TOWNS_VRAM_ADDR_MASK ((1 << TOWNS_VRAM_ADDR_SHIFT) - 1)

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
private:
	std::recursive_mutex vram_lock; // [bank][layer];
protected:
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
	__DECL_ALIGNED(16) uint8_t packed_pixel_mask_reg[8]; // '1' = Write. I/O 0458H - 045BH.
	__DECL_ALIGNED(16) uint8_t vram[(1 << TOWNS_VRAM_ADDR_SHIFT) + 8]; // Related by machine.
	// End.

	// Flags related by host renderer. Not saved.
	bool has_hardware_rendering;
	bool has_hardware_blending;
	// End.

	uint8_t cpu_id;
	uint16_t machine_id;

	constexpr uint32_t calc_std_address_offset(uint32_t addr)
	{
		// 0x80100000 : Single
		// 0x80000000 : Double
		return ((addr & (1 << (TOWNS_VRAM_ADDR_SHIFT + 1))) != 0) ?
			calc_single_page_address(addr) : calc_double_page_address(addr);
	}
	constexpr uint32_t calc_single_page_address(uint32_t addr)
	{
		// 0x80100000 : Single
		// 0x80000000 : Double
		return 	((addr & 0x00000003) |
				 ((addr & 0x00000004) << (TOWNS_VRAM_ADDR_SHIFT - 3))
				 | ((addr & (TOWNS_VRAM_ADDR_MASK & ~(7))) >> 1));
	}
	constexpr uint32_t calc_double_page_address(uint32_t addr)
	{
		// 0x80000000 : Double
		return addr & TOWNS_VRAM_ADDR_MASK;
	}
	inline void transfer_data_from_double_pages(uint32_t offset, uint32_t bytes, uint8_t* dst)
	{
		const uint32_t bytes0 = bytes & (0xfffffff0 & TOWNS_VRAM_ADDR_MASK); // Align of 16.
		const uint32_t bytes1 = bytes & 0x0000000f; // MOD  of 16.
		uint8_t* p = dst;
		__DECL_ALIGNED(32) uint8_t data_cache[32];
		for(uint32_t ar = 0; ar < bytes0; ar += 32) {
			uint32_t ar2 = ar + offset;
			__DECL_VECTORIZED_LOOP
			for(uint32_t j = 0; j < 32; j++) {
				data_cache[j] = vram[ar2 + j];
			}
			__DECL_VECTORIZED_LOOP
			for(size_t j = 0; j < 32; j++) {
				p[j] = data_cache[j];
			}
			p += 32;
		}
		__UNLIKELY_IF(bytes1 != 0) {
			uint32_t ar = offset + bytes0;
			for(uint32_t j = 0; j < bytes1; j++) {
				data_cache[j] = vram[ar + j];
			}
			for(uint32_t j = 0; j < bytes1; j++) {
				p[j] = data_cache[j];
			}
		}
	}
	inline void transfer_data_from_single_page(uint32_t offset, uint32_t bytes, uint8_t* dst)
	{
		const uint32_t bytes0 = bytes & (0xfffffff0 & TOWNS_VRAM_ADDR_MASK); // Align of 16.
		const uint32_t bytes1 = bytes & 0x0000000f; // MOD  of 16.
		__DECL_ALIGNED(32) uint8_t data_cache[32];
		uint8_t* p = dst;
		for(uint32_t ar = 0; ar < bytes0; ar += 32) {
			uint32_t ar2 = ar + offset;
			__DECL_VECTORIZED_LOOP
			for(uint32_t j = 0; j < 32; j++) {
				data_cache[j] = vram[calc_single_page_address(ar2 + j)];
			}
			__DECL_VECTORIZED_LOOP
			for(size_t j = 0; j < 32; j++) {
				p[j] = data_cache[j];
			}
			p += 32;
		}
		__UNLIKELY_IF(bytes1 != 0) {
			uint32_t ar = offset + bytes0;
			for(uint32_t j = 0; j < bytes1; j++) {
				data_cache[j] = vram[calc_single_page_address(ar + j)];
			}
			for(uint32_t j = 0; j < bytes1; j++) {
				p[j] = data_cache[j];
			}
		}
	}

public:
	TOWNS_VRAM(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(vram, 0x00, sizeof(vram));
		cpu_id = 0x01; // for 05EEh
		machine_id = 0x0100;
		set_device_name(_T("FM-Towns VRAM"));
	}
	~TOWNS_VRAM() {}

	virtual void initialize() override;
	virtual void reset() override;

	virtual uint32_t __FASTCALL read_dma_data8w(uint32_t addr, int* wait) override;
	virtual uint32_t __FASTCALL read_dma_data16w(uint32_t addr, int* wait) override;
	virtual uint32_t __FASTCALL read_dma_data32w(uint32_t addr, int* wait) override;

	virtual void __FASTCALL write_dma_data8w(uint32_t addr, uint32_t data, int* wait) override;
	virtual void __FASTCALL write_dma_data16w(uint32_t addr, uint32_t data, int* wait) override;
	virtual void __FASTCALL write_dma_data32w(uint32_t addr, uint32_t data, int* wait) override;

	virtual uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr) override;
	virtual uint32_t __FASTCALL read_memory_mapped_io16(uint32_t addr) override;
	virtual uint32_t __FASTCALL read_memory_mapped_io32(uint32_t addr) override;
	virtual void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data) override;
	virtual void __FASTCALL write_memory_mapped_io16(uint32_t addr, uint32_t data) override;
	virtual void __FASTCALL write_memory_mapped_io32(uint32_t addr, uint32_t data) override;

	virtual void __FASTCALL write_io8(uint32_t address, uint32_t data) override;
	virtual void __FASTCALL write_io16(uint32_t address, uint32_t data) override;
	virtual uint32_t __FASTCALL read_io8(uint32_t address) override;
	virtual uint32_t __FASTCALL read_io16(uint32_t address) override;

	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override; // Do render
	virtual bool process_state(FILEIO* state_fio, bool loading) override;

	// Unique Functions
	// Get host data pointer for direct accessing.
	virtual inline uint8_t* __FASTCALL get_vram_address(uint32_t offset)
	{
		//uint32_t offset2 = calc_std_address_offset(offset);
		const uint32_t offset2 = offset & TOWNS_VRAM_ADDR_MASK;
		return &(vram[offset2]);
	}
	// Transfer VRAM data to CRTC.
	virtual inline void __FASTCALL get_data_from_vram(const bool is_single, uint32_t offset, uint32_t bytes, uint8_t* dst)
	{
		__UNLIKELY_IF((bytes == 0) || (dst == nullptr)) {
			return;
		}
		__UNLIKELY_IF(bytes > (TOWNS_CRTC_MAX_PIXELS * sizeof(uint16_t))) {
			bytes = TOWNS_CRTC_MAX_PIXELS * sizeof(uint16_t);
		}
		uint32_t addr = offset & TOWNS_VRAM_ADDR_MASK;
		uint8_t* p = dst;
		bool is_wrap = ((addr + bytes) > (TOWNS_VRAM_ADDR_MASK + 1)) ? true : false;
		__UNLIKELY_IF(is_wrap) {
			uint32_t bytes0 = (TOWNS_VRAM_ADDR_MASK + 1) - addr;
			uint32_t bytes1 = (addr + bytes) - (TOWNS_VRAM_ADDR_MASK + 1);
			uint8_t* p0 = dst;
			uint8_t* p1 = &(dst[bytes0]);
			lock();
			if(is_single) {
				transfer_data_from_single_page(addr, bytes0, p0);
				transfer_data_from_single_page(0, bytes1, p1);
			} else {
				transfer_data_from_double_pages(addr, bytes0, p0);
				transfer_data_from_double_pages(0, bytes1, p1);
			}
			unlock();
		} else {
			lock();
			if(is_single) {
				transfer_data_from_single_page(addr, bytes, dst);
			} else {
				transfer_data_from_double_pages(addr, bytes, dst);
			}
			unlock();
		}
	}

	
	virtual bool __FASTCALL set_buffer_to_vram(uint32_t offset, uint8_t *buf, int words);
	virtual bool __FASTCALL get_vram_to_buffer(uint32_t offset, uint8_t *buf, int words);
	virtual inline uint32_t __FASTCALL get_vram_size()
	{
		return TOWNS_VRAM_ADDR_MASK + 1; // ToDo
	}
	void set_cpu_id(uint16_t val)
	{
		cpu_id = val & 0x07;
	}
	void set_machine_id(uint16_t val)
	{
		machine_id = val & 0xfff8;
	}
	// If you use other framework, place mutex lock.
	//std::recursive_mutex vram_lock[2][2]; // [bank][layer];
	inline void lock() noexcept;
	inline void unlock() noexcept;
	inline bool try_lock() noexcept;

	// New APIs?
	// End.
};

inline void TOWNS_VRAM::lock() noexcept
{
#if defined(USE_TOWNS_VRAM_PARALLEL_ACCESS)
	vram_lock.lock();
#endif
}
inline void TOWNS_VRAM::unlock() noexcept
{
#if defined(USE_TOWNS_VRAM_PARALLEL_ACCESS)
	vram_lock.unlock();
#endif
}

inline bool TOWNS_VRAM::try_lock() noexcept
{
#if defined(USE_TOWNS_VRAM_PARALLEL_ACCESS)
	return vram_lock.try_lock();
#else
	return true;
#endif
}


#if defined(USE_TOWNS_VRAM_PARALLEL_ACCESS)
	#undef USE_TOWNS_VRAM_PARALLEL_ACCESS
#endif
}
#endif
