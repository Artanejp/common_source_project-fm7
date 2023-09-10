/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.07 -

	[ memory ]
*/

#ifndef _TOWNS_MEMORY_H_
#define _TOWNS_MEMORY_H_

#include "device.h"
#include "../../common.h"
#include "../memory.h"
#include "./towns_common.h"

#define SIG_FMTOWNS_MACHINE_ID	1
#define SIG_MEMORY_EXTNMI       2

// MAP:
// 00000000 - 000fffff : SYSTEM RAM PAGE 0 (Similar to FMR-50).
// 00100000 - 3fffffff : EXTRA RAM (for i386 native mode.Page size is 1MB.)
// 40000000 - 7fffffff : External I/O BOX.Not accessible.
// 80000000 - bfffffff : VRAM (Reserved 01020000H bytes with earlier Towns.)
// c0000000 - c21fffff : ROM card and dictionaly/font/DOS ROMs.
// c2200000 - c2200fff : PCM RAM (Banked).
// c2201000 - fffbffff : Reserved.
// FFFC0000 - FFFFFFFF : Towns System ROM.

namespace FMTOWNS {
enum {
	TOWNS_MEMORY_NORMAL = 0,
	TOWNS_MEMORY_FMR_VRAM,
	TOWNS_MEMORY_FMR_TEXT,
	TOWNS_MEMORY_FMR_VRAM_RESERVE,
	TOWNS_MEMORY_SPRITE_ANKCG1,
	TOWNS_MEMORY_ANKCG2,

	TOWNS_MEMORY_MMIO_0CC,

	TOWNS_MEMORY_TYPE_EXT_MMIO,
	TOWNS_MEMORY_TYPE_LEARN_RAM,
	TOWNS_MEMORY_TYPE_WAVERAM,
};
}
// Please set from config
#define TOWNS_EXTRAM_PAGES 6

class BEEP;
class I386;

namespace FMTOWNS {
	class TOWNS_VRAM;
	class TOWNS_SPRITE;
	class TOWNS_ROM_CARD;
}

namespace FMTOWNS {
#undef  TOWNS_MEMORY_MAP_SHIFT
#define TOWNS_MEMORY_MAP_SHIFT 15
#undef  TOWNS_MEMORY_MAP_SIZE
#define  TOWNS_MEMORY_MAP_SIZE  (1 << (32 - TOWNS_MEMORY_MAP_SHIFT))

class TOWNS_MEMORY : public DEVICE
{
protected:
	enum {
		WAITVAL_RAM  = -1,
		WAITVAL_VRAM = -2,
	};
	DEVICE* d_vram;
	DEVICE* d_sprite;       // 0x81000000 - 0x8101ffff ?
	DEVICE* d_pcm;             // 0xc2200000 - 0xc2200fff
	DEVICE* d_timer;
	DEVICE* d_dmac;
	DEVICE* d_crtc;
	DEVICE* d_planevram;
	I386*   d_cpu;

	DEVICE* d_dictionary;
	DEVICE* d_sysrom;
	DEVICE* d_msdos;
	DEVICE* d_serialrom;
	DEVICE* d_font;
	DEVICE* d_font_20pix;

	DEVICE* d_iccard[2];
	const uint32_t 	NOT_NEED_TO_OFFSET = UINT32_MAX;

	typedef struct memory_device_map_s {
		uint8_t* mem_ptr;
		DEVICE* device_ptr;
		uint32_t base_offset;
		int32_t  waitval;
	} memory_device_map_t;
	memory_device_map_t membus_read_map[TOWNS_MEMORY_MAP_SIZE];
	memory_device_map_t membus_write_map[TOWNS_MEMORY_MAP_SIZE];
	memory_device_map_t dma_read_map[TOWNS_MEMORY_MAP_SIZE];
	memory_device_map_t dma_write_map[TOWNS_MEMORY_MAP_SIZE];

 	outputs_t outputs_ram_wait;
	outputs_t outputs_rom_wait;

	bool bankc0_vram;
	bool ankcg_enabled;
	bool select_d0_rom;
	bool select_d0_dict;

	uint16_t machine_id;
	uint8_t cpu_id;
	bool is_compatible;
	bool dma_is_vram;

	// RAM
	uint8_t *extra_ram;                  // 0x00000000 - (0x3fffffff) : Size is defined by extram_size + 0x00100000;
	uint32_t extram_size;
	uint32_t cpu_clock_val;
	uint32_t vram_wait_val;
	uint32_t mem_wait_val;

	uint8_t wait_register_older;  // 05E0h
	uint8_t wait_register_ram;    // 05E2h
	uint8_t wait_register_vram;   // 05E6h

	bool extra_nmi_mask;
	bool extra_nmi_val;
	bool nmi_mask;
	bool software_reset;
	bool nmi_vector_protect;
	bool poff_status;
	bool reset_happened;

	// misc
	uint32_t vram_size; // Normally 512KB.
	bool initialized;

	// MISC1-MISC4
	uint8_t reg_misc3; // 0024
	uint8_t reg_misc4; // 0025

	// Functions
	virtual void reset_wait_values();
	virtual void set_wait_values();
	virtual void update_machine_features();
	virtual void __FASTCALL config_page0f(const bool sysrombank, const bool force);
	virtual void __FASTCALL config_page0c_0e(const bool vrambank, const bool dictbank, const bool force);

	virtual bool set_cpu_clock_by_wait();
	virtual void     __FASTCALL write_fmr_ports8(uint32_t addr, uint32_t data);
	virtual uint8_t  __FASTCALL read_fmr_ports8(uint32_t addr);
	virtual void     __FASTCALL write_sys_ports8(uint32_t addr, uint32_t data);
	virtual uint8_t __FASTCALL read_sys_ports8(uint32_t addr);

	void __FASTCALL set_memory_devices_map_values(uint32_t start, uint32_t end, memory_device_map_t* dataptr, uint8_t* baseptr, DEVICE* device, uint32_t base_offset = UINT32_MAX);
	void __FASTCALL set_memory_devices_map_wait(uint32_t start, uint32_t end, memory_device_map_t* dataptr, int wait = WAITVAL_RAM);
	void __FASTCALL unset_memory_devices_map(uint32_t start, uint32_t end, memory_device_map_t* dataptr, int wait = WAITVAL_RAM);

	constexpr uint32_t read_beyond_boundary_data16(memory_device_map_t *_map, const uint32_t addr, const uint32_t offset, const uint32_t mapptr, const bool is_dma, int* wait)
	{
		pair16_t w;
		int waitvals[2] = {0};
		if(is_dma) {
			w.b.l = read_dma_data8w(addr    , &(waitvals[0]));
			w.b.h = read_dma_data8w(addr + 1, &(waitvals[1]));
		} else {
			w.b.l = read_data8w(addr    , &(waitvals[0]));
			w.b.h = read_data8w(addr + 1, &(waitvals[1]));
		}
		__LIKELY_IF(wait != NULL) {
			*wait = waitvals[0];
			//*wait = waitvals[0] + waitvals[1];
		}
		return (uint32_t)(w.w);
	}
	constexpr uint32_t read_beyond_boundary_data32(memory_device_map_t *_map, const uint32_t addr, const uint32_t offset, const uint32_t mapptr, const bool is_dma, int* wait)
	{
		pair32_t d;
		pair16_t w;
		int waitvals[3] = {0};
		switch(offset & 3) {
		case 1:
		case 3:
			if(is_dma) {
				d.b.l  = read_dma_data8w (addr    , &(waitvals[0]));
				w.w    = read_dma_data16w(addr + 1, &(waitvals[1]));
				d.b.h3 = read_dma_data8w (addr + 3, &(waitvals[2]));
			} else {
				d.b.l  = read_data8w (addr    ,  &(waitvals[0]));
				w.w    = read_data16w(addr + 1,  &(waitvals[1]));
				d.b.h3 = read_data8w (addr + 3,  &(waitvals[2]));
			}
			d.b.h  = w.b.l;
			d.b.h2 = w.b.h;
			break;
		case 2:
			if(is_dma) {
				d.w.l  = read_dma_data16w(addr    ,  &(waitvals[0]));
				d.w.h  = read_dma_data16w(addr + 2,  &(waitvals[1]));
			} else {
				d.w.l  = read_data16w(addr    ,  &(waitvals[0]));
				d.w.h  = read_data16w(addr + 2,  &(waitvals[1]));
			}
			break;
		default:
			d.d = 0xffffffff;
			waitvals[0] = _map[mapptr].waitval;
			break;
		}

		__LIKELY_IF(wait != NULL) {
			for(int i = 0; i < 2; i++) {
				__UNLIKELY_IF(waitvals[i] < 0) {
					waitvals[i] = (waitvals[i] == WAITVAL_VRAM) ? vram_wait_val : mem_wait_val;
				}
			}
			*wait = waitvals[0];
			//*wait = waitvals[0] + waitvals[1] + waitvals[2];
		}
		return d.d;
	}

	constexpr void write_beyond_boundary_data16(memory_device_map_t *_map, const uint32_t addr, const uint32_t offset, const uint32_t mapptr, const bool is_dma, uint32_t data, int* wait)
	{
		pair16_t w;
		w.w = data;
		int waitvals[2] = {0};
		if(is_dma) {
			write_dma_data8w(addr    , w.b.l, &(waitvals[0]));
			write_dma_data8w(addr + 1, w.b.h, &(waitvals[1]));
		} else {
			write_data8w(addr    , w.b.l, &(waitvals[0]));
			write_data8w(addr + 1, w.b.h, &(waitvals[1]));
		}
		__LIKELY_IF(wait != NULL) {
			*wait = waitvals[0];
			//*wait = waitvals[0] + waitvals[1];
		}
	}
	constexpr void write_beyond_boundary_data32(memory_device_map_t *_map, const uint32_t addr, const uint32_t offset, const uint32_t mapptr, const bool is_dma, uint32_t data, int* wait)
	{
		pair32_t d;
		pair16_t w;
		d.d = data;
		int waitvals[3] = {0};
		switch(offset & 3) {
		case 1:
		case 3:
			w.b.l = d.b.h;
			w.b.h = d.b.h2;
			if(is_dma) {
				write_dma_data8w (addr    , d.b.l, &(waitvals[0]));
				write_dma_data16w(addr + 1, w.w, &(waitvals[1]));
				write_dma_data8w (addr + 3, d.b.h3, &(waitvals[2]));
			} else {
				write_data8w (addr    , d.b.l, &(waitvals[0]));
				write_data16w(addr + 1, w.w, &(waitvals[1]));
				write_data8w (addr + 3, d.b.h3, &(waitvals[2]));
			}
			break;
		case 2:
			if(is_dma) {
				write_dma_data16w(addr    , d.w.l, &(waitvals[0]));
				write_dma_data16w(addr + 2, d.w.h, &(waitvals[1]));
			} else {
				write_data16w(addr    , d.w.l, &(waitvals[0]));
				write_data16w(addr + 2, d.w.h, &(waitvals[1]));
			}
			break;
		default:
			waitvals[0] = _map[mapptr].waitval;
			break;
		}

		__LIKELY_IF(wait != NULL) {
			for(int i = 0; i < 2; i++) {
				__UNLIKELY_IF(waitvals[i] < 0) {
					waitvals[i] = (waitvals[i] == WAITVAL_VRAM) ? vram_wait_val : mem_wait_val;
				}
			}
			*wait = waitvals[0];
			//*wait = waitvals[0] + waitvals[1] + waitvals[2];
		}
	}

	constexpr bool check_device_boundary(memory_device_map_t *_map, uint32_t offset, uint32_t mapptr, const uint8_t bytewidth)
	{
		__UNLIKELY_IF((offset + bytewidth) > memory_map_grain()) {
			__LIKELY_IF(mapptr < memory_map_size()) {
				__LIKELY_IF((_map[mapptr].device_ptr == _map[mapptr + 1].device_ptr) && (_map[mapptr].mem_ptr == _map[mapptr + 1].mem_ptr)) {
					return false;
				}
				return true;
			} else {
				return true;
			}
		}
		return false;
	}
	constexpr uint32_t read_8bit_data(memory_device_map_t *_map, uint32_t mapptr, uint32_t addr, uint32_t offset, const bool is_dma, int* wait)
	{
		uint8_t val = 0xff;
		int waitval;
		uint32_t base = _map[mapptr].base_offset;
		__LIKELY_IF(_map[mapptr].mem_ptr != NULL) {
			uint8_t* ptr = _map[mapptr].mem_ptr;
			__UNLIKELY_IF(base == UINT32_MAX) {
				base = 0;
			}
			val = ptr[offset + base];
		} else if(_map[mapptr].device_ptr != NULL) {
			DEVICE* dev = _map[mapptr].device_ptr;
			__LIKELY_IF(base == UINT32_MAX) {
				offset = addr;
			} else {
				offset += base;
			}
			const bool is_this = (dev == this);
			if((is_dma) && !(is_this)) {
				val = dev->read_dma_data8(offset);
			} else {
				val = dev->read_memory_mapped_io8(offset);
			}
		}
		__LIKELY_IF(wait != NULL) {
			waitval = _map[mapptr].waitval;
			__LIKELY_IF(waitval < 0) {
				waitval = (waitval == WAITVAL_VRAM) ? vram_wait_val : mem_wait_val;
			}
			*wait = waitval;
		}
		return val;
	}
	constexpr uint32_t read_16bit_data(memory_device_map_t *_map, uint32_t mapptr, uint32_t addr, uint32_t offset, const bool is_dma, int* wait)
	{
		uint16_t val = 0xffff;
		int waitval;
		uint32_t base = _map[mapptr].base_offset;
		__LIKELY_IF(_map[mapptr].mem_ptr != NULL) {
			uint8_t* ptr = _map[mapptr].mem_ptr;
			__UNLIKELY_IF(base == UINT32_MAX) {
				base = 0;
			}
			ptr = &(ptr[offset + base]);
			pair16_t w;
			w.read_2bytes_le_from(ptr);
			val = w.w;
		} else if(_map[mapptr].device_ptr != NULL) {
			DEVICE* dev = _map[mapptr].device_ptr;
			__LIKELY_IF(base == UINT32_MAX) {
				offset = addr;
			} else {
				offset += base;
			}
			const bool is_this = (dev == this);
			if((is_dma) && !(is_this)) {
				val = dev->read_dma_data16(offset);
			} else {
				val = dev->read_memory_mapped_io16(offset);
			}
		}
		__LIKELY_IF(wait != NULL) {
			waitval = _map[mapptr].waitval;
			__LIKELY_IF(waitval < 0) {
				waitval = (waitval == WAITVAL_VRAM) ? vram_wait_val : mem_wait_val;
			}
			//__LIKELY_IF((offset & 1) == 0) {
			*wait = waitval;
			//} else {
			//	*wait = waitval * 2;
			//}
		}
		return val;
	}
	constexpr uint32_t read_32bit_data(memory_device_map_t *_map, uint32_t mapptr, uint32_t addr, uint32_t offset, const bool is_dma, int* wait)
	{
		uint32_t val = 0xffffffff;
		uint32_t base = _map[mapptr].base_offset;
		__LIKELY_IF(_map[mapptr].mem_ptr != NULL) {
			uint8_t* ptr = _map[mapptr].mem_ptr;
			__UNLIKELY_IF(base == UINT32_MAX) {
				base = 0;
			}
			ptr = &(ptr[offset + base]);
			pair32_t d;
			d.read_4bytes_le_from(ptr);
			val = d.d;
		} else if(_map[mapptr].device_ptr != NULL) {
			DEVICE* dev = _map[mapptr].device_ptr;
			__LIKELY_IF(base == UINT32_MAX) {
				offset = addr;
			} else {
				offset += base;
			}
			const bool is_this = (dev == this);
			if((is_dma) && !(is_this)) {
				val = dev->read_dma_data32(offset);
			} else {
				val = dev->read_memory_mapped_io32(offset);
			}
		}
		__LIKELY_IF(wait != NULL) {
			int waitval = _map[mapptr].waitval;
			__LIKELY_IF(waitval < 0) {
				waitval = (waitval == WAITVAL_VRAM) ? vram_wait_val : mem_wait_val;
			}
			//__LIKELY_IF((offset & 1) == 0) {
			*wait = waitval;
			//} else {
			//	*wait = waitval * 2;
			//}
		}
		return val;
	}
	constexpr void write_8bit_data(memory_device_map_t *_map, uint32_t mapptr, uint32_t addr, uint32_t offset, const bool is_dma, uint32_t data, int* wait)
	{
		int waitval;
		uint32_t base = _map[mapptr].base_offset;
		__LIKELY_IF(_map[mapptr].mem_ptr != NULL) {
			uint8_t* ptr = _map[mapptr].mem_ptr;
			__UNLIKELY_IF(base == UINT32_MAX) {
				base = 0;
			}
			ptr[offset + base] = data;
		} else if(_map[mapptr].device_ptr != NULL) {
			DEVICE* dev = _map[mapptr].device_ptr;
			__LIKELY_IF(base == UINT32_MAX) {
				offset = addr;
			} else {
				offset += base;
			}
			const bool is_this = (dev == this);
			if((is_dma) && !(is_this)) {
				dev->write_dma_data8(offset, data);
			} else {
				dev->write_memory_mapped_io8(offset, data);
			}
		}
		__LIKELY_IF(wait != NULL) {
			waitval = _map[mapptr].waitval;
			__LIKELY_IF(waitval < 0) {
				waitval = (waitval == WAITVAL_VRAM) ? vram_wait_val : mem_wait_val;
			}
			*wait = waitval;
		}
	}

	constexpr void write_16bit_data(memory_device_map_t *_map, uint32_t mapptr, uint32_t addr, uint32_t offset, const bool is_dma, uint32_t data, int* wait)
	{
		int waitval;
		uint32_t base = _map[mapptr].base_offset;
		__LIKELY_IF(_map[mapptr].mem_ptr != NULL) {
			uint8_t* ptr = _map[mapptr].mem_ptr;
			__UNLIKELY_IF(base == UINT32_MAX) {
				base = 0;
			}
			ptr = &(ptr[offset + base]);
			pair16_t w;
			w.w = data;
			w.write_2bytes_le_to(ptr);
		} else if(_map[mapptr].device_ptr != NULL) {
			DEVICE* dev = _map[mapptr].device_ptr;
			__LIKELY_IF(base == UINT32_MAX) {
				offset = addr;
			} else {
				offset += base;
			}
			const bool is_this = (dev == this);
			if((is_dma) && !(is_this)) {
				dev->write_dma_data16(offset, data);
			} else {
				dev->write_memory_mapped_io16(offset, data);
			}
		}
		__LIKELY_IF(wait != NULL) {
			waitval = _map[mapptr].waitval;
			__LIKELY_IF(waitval < 0) {
				waitval = (waitval == WAITVAL_VRAM) ? vram_wait_val : mem_wait_val;
			}
			//__LIKELY_IF((offset & 1) == 0) {
			*wait = waitval;
			//} else {
			//	*wait = waitval * 2;
			//}
		}
	}
	constexpr void write_32bit_data(memory_device_map_t *_map, uint32_t mapptr, uint32_t addr, uint32_t offset, const bool is_dma, uint32_t data, int* wait)
	{
		uint32_t base = _map[mapptr].base_offset;
		__LIKELY_IF(_map[mapptr].mem_ptr != NULL) {
			uint8_t* ptr = _map[mapptr].mem_ptr;
			__UNLIKELY_IF(base == UINT32_MAX) {
				base = 0;
			}
			ptr = &(ptr[offset + base]);
			pair32_t d;
			d.d = data;
			d.write_4bytes_le_to(ptr);
		} else if(_map[mapptr].device_ptr != NULL) {
			DEVICE* dev = _map[mapptr].device_ptr;
			__LIKELY_IF(base == UINT32_MAX) {
				offset = addr;
			} else {
				offset += base;
			}
			const bool is_this = (dev == this);
			if((is_dma) && !(is_this)) {
				dev->write_dma_data32(offset, data);
			} else {
				dev->write_memory_mapped_io32(offset, data);
			}
		}
		__LIKELY_IF(wait != NULL) {
			int waitval = _map[mapptr].waitval;
			__LIKELY_IF(waitval < 0) {
				waitval = (waitval == WAITVAL_VRAM) ? vram_wait_val : mem_wait_val;
			}
			//__LIKELY_IF((offset & 1) == 0) {
			*wait = waitval;
			//} else {
			//	*wait = waitval * 2;
			//}
		}
	}
	inline bool is_faster_wait()
	{
		return ((mem_wait_val == 0) && (vram_wait_val < 3)) ? true : false;
	}
	/*!
	  @note Use memory map per 32KB (131072 entries) as 1st tier.
	        And, at Some pages hadles special devices,
			000C8000h - 000CFFFFh : this
			C2140000h - C2141FFFh : DICTIONARY (CMOS)
			C2200000h - C2200FFFh : RF5C68 (PCM SOUND)
			MAP select usage:
	*/
	inline const uint64_t memory_map_size()  { return TOWNS_MEMORY_MAP_SIZE; }
	inline const uint64_t memory_map_shift() { return TOWNS_MEMORY_MAP_SHIFT; }
	constexpr uint64_t memory_map_mask() { return ((1 << TOWNS_MEMORY_MAP_SHIFT) - 1); }
	constexpr uint64_t memory_map_grain() { return (1 << TOWNS_MEMORY_MAP_SHIFT); }

public:
	TOWNS_MEMORY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("FMTOWNS_MEMORY"));

		extram_size = 0x00200000; // Basically 2MB

		d_cpu = NULL;

		d_vram = NULL;
		d_sprite = NULL;
		d_pcm = NULL;
		d_timer = NULL;
		d_dmac = NULL;
		d_crtc = NULL;
		d_planevram = NULL;
		d_iccard[0] = NULL;
		d_iccard[1] = NULL;

		d_dictionary = NULL;
		d_sysrom = NULL;
		d_msdos = NULL;
		d_serialrom = NULL;
		d_font = NULL;
		d_font_20pix = NULL;
		initialized = false;

		initialize_output_signals(&outputs_ram_wait);
		initialize_output_signals(&outputs_rom_wait);
		// Note: machine id must set before initialize() from set_context_machine_id() by VM::VM().
		// machine_id = 0x0100;   // FM-Towns 1,2
		// machine_id = 0x0200 // FM-Towns  1F/2F/1H/2H
		// machine_id = 0x0300 // FM-Towns  UX10/UX20/UX40
		// machine_id = 0x0400 // FM-Towns  10F/20F/40H/80H
		// machine_id = 0x0500 // FM-Towns2 CX10/CX20/CX40/CX100
		// machine_id = 0x0600 // FM-Towns2 UG10/UG20/UG40/UG80
		// machine_id = 0x0700 // FM-Towns2 HR20/HR100/HR200
		// machine_id = 0x0800 // FM-Towns2 HG20/HG40/HG100
		// machine_id = 0x0900 // FM-Towns2 UR20/UR40/UR80
		// machine_id = 0x0b00 // FM-Towns2 MA20/MA170/MA340
		// machine_id = 0x0c00 // FM-Towns2 MX20/MX170/MX340
		// machine_id = 0x0d00 // FM-Towns2 ME20/ME170
		// machine_id = 0x0f00 // FM-Towns2 MF20/MF170/Fresh
		machine_id = 0x0100;   // FM-Towns 1,2

		// Note: cpu id must set before initialize() from set_context_cpu_id() by VM::VM().
		// cpu_id = 0x00; // 80286.
		// cpu_id = 0x01; // 80386DX.
		// cpu_id = 0x02; // 80486SX/DX.
		// cpu_id = 0x03; // 80386SX.
		cpu_id = 0x01; // 80386DX.

		unset_mmio_rw(0x00000000, 0xffffffff);
		unset_dma_rw(0x00000000, 0xffffffff);
		extra_ram = NULL;
	}
	~TOWNS_MEMORY() {}

	// common functions
	void initialize() override;
	void release() override;
	void reset() override;

	virtual uint32_t __FASTCALL read_data8w(uint32_t addr, int* wait) override;
	virtual uint32_t __FASTCALL read_data16w(uint32_t addr, int* wait) override;
	virtual uint32_t __FASTCALL read_data32w(uint32_t addr, int* wait) override;

	virtual void __FASTCALL write_data8w(uint32_t addr, uint32_t data, int* wait) override;
	virtual void __FASTCALL write_data16w(uint32_t addr, uint32_t data, int* wait) override;
	virtual void __FASTCALL write_data32w(uint32_t addr, uint32_t data, int* wait) override;

	virtual uint32_t __FASTCALL read_dma_data8w(uint32_t addr, int* wait) override;
	virtual uint32_t __FASTCALL read_dma_data16w(uint32_t addr, int* wait) override;
	virtual uint32_t __FASTCALL read_dma_data32w(uint32_t addr, int* wait) override;
	virtual void __FASTCALL write_dma_data8w(uint32_t addr, uint32_t data, int* wait) override;
	virtual void __FASTCALL write_dma_data16w(uint32_t addr, uint32_t data, int* wait) override;
	virtual void __FASTCALL write_dma_data32w(uint32_t addr, uint32_t data, int* wait) override;

	virtual void     __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_io8(uint32_t addr) override;

	virtual void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr) override;

	virtual void __FASTCALL write_memory_mapped_io16(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_memory_mapped_io16(uint32_t addr) override;
;
	virtual void __FASTCALL write_memory_mapped_io32(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_memory_mapped_io32(uint32_t addr) override;

	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	virtual uint32_t __FASTCALL read_signal(int ch) override;

	//void event_frame();
	virtual void __FASTCALL set_intr_line(bool line, bool pending, uint32_t bit) override;

	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique functions
	void set_extra_ram_size(uint32_t megabytes)
	{
		uint32_t limit = 5;
		uint32_t minimum = 2;
		switch((machine_id & 0xff00) >> 8) {
		case 0x00:
		case 0x01: // Towns 1/2 : Not Supported.
			minimum = 1;
			break;
		case 0x03: // TOWNS2 UX
		case 0x06: // TOWNS2 UG
			minimum = 1;
			limit = 9; // 2MB + 4MB x 2? - 1MB
			break;
		case 0x02: // TOWNS 2F/2H
		case 0x04: // TOWNS 10F/10H/20F/20H
			minimum = 1;
			limit = 7; // 2MB + 2MB x 3
			break;
		case 0x05: // TOWNS II CX
		case 0x08: // TOWNS II HG
			limit = 15; // 8MB x 2 - 1MB?
			break;
		case 0x07: // Towns II HR
		case 0x09: // Towns II UR
			limit = 31; // 16MB x 2 - 1MB?
			break;
		default:   // After MA/MX/ME/MF, Fresh
			limit = 127; // 128MB - 1MB?
			break;
		}
		if(megabytes > limit) megabytes = limit;
		if(megabytes < minimum) megabytes = minimum;
		extram_size = megabytes << 20;
	}

	virtual void __FASTCALL set_mmio_memory_r(uint32_t start, uint32_t end, uint8_t* ptr, uint32_t base_offset = 0);
	virtual void __FASTCALL set_mmio_memory_w(uint32_t start, uint32_t end, uint8_t* ptr, uint32_t base_offset = 0);
	inline  void __FASTCALL set_mmio_memory_rw(uint32_t start, uint32_t end, uint8_t* ptr, uint32_t base_offset = 0)
	{
		set_mmio_memory_r(start, end, ptr, base_offset);
		set_mmio_memory_w(start, end, ptr, base_offset);
	}

	virtual void  __FASTCALL set_mmio_wait_r(uint32_t start, uint32_t end, int wait);
	virtual void  __FASTCALL set_mmio_wait_w(uint32_t start, uint32_t end, int wait);
	inline void  __FASTCALL set_mmio_wait_rw(uint32_t start, uint32_t end, int wait)
	{
		set_mmio_wait_r(start, end, wait);
		set_mmio_wait_w(start, end, wait);
	}

	virtual void  __FASTCALL set_mmio_device_r(uint32_t start, uint32_t end, DEVICE* baseptr, uint32_t baseaddress = UINT32_MAX);
	virtual void  __FASTCALL set_mmio_device_w(uint32_t start, uint32_t end, DEVICE* baseptr, uint32_t baseaddress = UINT32_MAX);
	inline void  __FASTCALL set_mmio_device_rw(uint32_t start, uint32_t end, DEVICE* baseptr, uint32_t baseaddress = UINT32_MAX)
	{
		set_mmio_device_r(start, end, baseptr, baseaddress);
		set_mmio_device_w(start, end, baseptr, baseaddress);
	}

	virtual void  __FASTCALL set_dma_memory_r(uint32_t start, uint32_t end, uint8_t* ptr, uint32_t base_offset = 0);
	virtual void  __FASTCALL set_dma_memory_w(uint32_t start, uint32_t end, uint8_t* ptr, uint32_t base_offset = 0);
	inline void  __FASTCALL set_dma_memory_rw(uint32_t start, uint32_t end, uint8_t* ptr, uint32_t base_offset = 0)
	{
		set_dma_memory_r(start, end, ptr, base_offset);
		set_dma_memory_w(start, end, ptr, base_offset);
	}

	virtual void  __FASTCALL set_dma_wait_r(uint32_t start, uint32_t end, int wait);
	virtual void  __FASTCALL set_dma_wait_w(uint32_t start, uint32_t end, int wait);
	inline void  __FASTCALL set_dma_wait_rw(uint32_t start, uint32_t end, int wait)
	{
		set_dma_wait_r(start, end, wait);
		set_dma_wait_w(start, end, wait);
	}

	virtual void  __FASTCALL set_dma_device_r(uint32_t start, uint32_t end, DEVICE* ptr, uint32_t baseaddress = UINT32_MAX);
	virtual void  __FASTCALL set_dma_device_w(uint32_t start, uint32_t end, DEVICE* ptr, uint32_t baseaddress = UINT32_MAX);
	inline void  __FASTCALL set_dma_device_rw(uint32_t start, uint32_t end, DEVICE* ptr, uint32_t baseaddress = UINT32_MAX)
	{
		set_dma_device_r(start, end, ptr, baseaddress);
		set_dma_device_w(start, end, ptr, baseaddress);
	}

	inline void __FASTCALL set_region_memory_r(uint32_t start, uint32_t end, uint8_t* baseptr, uint32_t base_offset = 0)
	{
		set_mmio_memory_r(start, end, baseptr, base_offset);
		set_dma_memory_r (start, end, baseptr, base_offset);
	}

	inline void __FASTCALL set_region_memory_w(uint32_t start, uint32_t end, uint8_t* baseptr,uint32_t base_offset = 0)
	{
		set_mmio_memory_w(start, end, baseptr, base_offset);
		set_dma_memory_w (start, end, baseptr, base_offset);
	}
	inline void __FASTCALL set_region_memory_rw(uint32_t start, uint32_t end, uint8_t* baseptr, uint32_t base_offset = 0)
	{
		set_region_memory_r(start, end, baseptr, base_offset);
		set_region_memory_w(start, end, baseptr, base_offset);
	}

	inline void __FASTCALL set_region_device_r(uint32_t start, uint32_t end, DEVICE* ptr, uint32_t baseaddress = UINT32_MAX)
	{
		set_mmio_device_r(start, end, ptr, baseaddress);
		set_dma_device_r (start, end, ptr, baseaddress);
	}
	inline void __FASTCALL set_region_device_w(uint32_t start, uint32_t end, DEVICE* ptr, uint32_t baseaddress = UINT32_MAX)
	{
		set_mmio_device_w(start, end, ptr, baseaddress);
		set_dma_device_w (start, end, ptr, baseaddress);
	}
	inline void __FASTCALL set_region_device_rw(uint32_t start, uint32_t end, DEVICE* ptr, uint32_t baseaddress = UINT32_MAX)
	{
		set_region_device_r(start, end, ptr, baseaddress);
		set_region_device_w(start, end, ptr, baseaddress);
	}
	virtual void  __FASTCALL unset_mmio_r(uint32_t start, uint32_t end, int wait = WAITVAL_RAM);
	virtual void  __FASTCALL unset_mmio_w(uint32_t start, uint32_t end, int wait = WAITVAL_RAM);
	inline  void  __FASTCALL unset_mmio_rw(uint32_t start, uint32_t end, int wait = WAITVAL_RAM)
	{
		unset_mmio_r(start, end, wait);
		unset_mmio_w(start, end, wait);
	}

	virtual void  __FASTCALL unset_dma_r(uint32_t start, uint32_t end, int wait = WAITVAL_RAM);
	virtual void  __FASTCALL unset_dma_w(uint32_t start, uint32_t end, int wait = WAITVAL_RAM);
	inline void  __FASTCALL  unset_dma_rw(uint32_t start, uint32_t end, int wait = WAITVAL_RAM)
	{
		unset_dma_r(start, end, wait);
		unset_dma_w(start, end, wait);
	}

	inline void  __FASTCALL unset_range_r(uint32_t start, uint32_t end, int wait = WAITVAL_RAM)
	{
		unset_mmio_r(start, end, wait);
		unset_dma_r(start, end, wait);
	}

	inline void  __FASTCALL unset_range_w(uint32_t start, uint32_t end, int wait = WAITVAL_RAM)
	{
		unset_mmio_w(start, end, wait);
		unset_dma_w(start, end, wait);
	}
	inline void  __FASTCALL unset_range_rw(uint32_t start, uint32_t end, int wait = WAITVAL_RAM)
	{
		unset_range_r(start, end, wait);
		unset_range_w(start, end, wait);
	}

	void set_context_cpu(I386* device)
	{
		d_cpu = device;
	}
	void set_context_dmac(DEVICE* device)
	{
		d_dmac = device;
	}
	virtual void set_context_vram(DEVICE* device)
	{
		d_vram = device;

	}
	void set_context_system_rom(DEVICE* device)
	{
		d_sysrom = device;
	}
	void set_context_font_rom(DEVICE* device)
	{
		d_font = device;
	}
	void set_context_font_20pix_rom(DEVICE* device)
	{
		d_font_20pix = device;
	}
	void set_context_dictionary(DEVICE* device)
	{
		d_dictionary = device;
	}
	void set_context_msdos(DEVICE* device)
	{
		d_msdos = device;
	}
	void set_context_timer(DEVICE* device)
	{
		d_timer = device;
	}
	void set_context_sprite(DEVICE* device)
	{
		d_sprite = device;
	}
	void set_context_crtc(DEVICE* device)
	{
		d_crtc = device;
	}
	void set_context_iccard(DEVICE* device, int num)
	{
		d_iccard[num & 1] = device;
	}
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void set_context_serial_rom(DEVICE* device)
	{
		d_serialrom = device;
	}
	void set_context_planevram(DEVICE *dev)
	{
		d_planevram = dev;
	}
	void set_machine_id(uint16_t val)
	{
		machine_id = val & 0xfff8;
	}
	void set_cpu_id(uint16_t val)
	{
		cpu_id = val & 0x07;
	}
};


}
#endif
