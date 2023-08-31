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
class TOWNS_MEMORY : public DEVICE
{
protected:
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

	// Add memory MAP
	DEVICE*  devmap_c0000h_read[0x00030000 >> 12]; // Per 1000h bytes
	uint32_t offsetmap_c0000h_read[0x00030000 >> 12];
	DEVICE*  devmap_c0000h_write[0x00030000 >> 12]; // Per 1000h bytes
	uint32_t offsetmap_c0000h_write[0x00030000 >> 12];

	DEVICE*  devmap_80000000h[0x04000000 >> 17];
	uint32_t offsetmap_80000000h[0x04000000 >> 17];

	DEVICE*  devmap_c0000000h_read[0x04000000 >> 12];
	uint32_t offsetmap_c0000000h_read[0x04000000 >> 12];
	DEVICE*  devmap_c0000000h_write[0x04000000 >> 12];
	uint32_t offsetmap_c0000000h_write[0x04000000 >> 12];

	virtual inline __FASTCALL DEVICE* select_bank_memory_mpu(uint32_t addr, const bool is_dma, const bool is_read, bool& is_exists, uintptr_t& memptr, uint32_t& offset, int& waitval)
	{
		memptr = UINTPTR_MAX;
		offset = UINT32_MAX;
		waitval = mem_wait_val;
		is_exists = false;
		#if 1
		__LIKELY_IF(addr < 0x00100000) {
			switch((addr >> 16) & 0x0f) {
			case 0x0c:
				__LIKELY_IF(!(dma_is_vram)) {
					is_exists = true;
					offset = addr - 0x000c0000;
					memptr = (uintptr_t)ram_pagec;
					return NULL;
				} else {
					__LIKELY_IF(addr < 0x000c8000) {
						__LIKELY_IF(d_planevram != NULL) {
							is_exists = true;
							// offset = addr - 0x000c0000;
							waitval = vram_wait_val;
						}
						return d_planevram;
					}
					__LIKELY_IF(addr < 0x000ca000) {
						__LIKELY_IF(d_sprite != NULL) {
							is_exists = true;
							// offset = addr - 0x000c0000;
							waitval = vram_wait_val;
						}
						return d_sprite;
					}
					__LIKELY_IF(addr < 0x000cc000) {
						if(ankcg_enabled) {
							if(is_read) {
								__LIKELY_IF(d_font != NULL) {
									is_exists = true;
									// offset = addr - 0x000c0000;
								}
								return d_font;
							} else {
								is_exists = false;
								return NULL;
							}
						} else {
							__LIKELY_IF(d_sprite != NULL) {
								is_exists = true;
								// offset = addr - 0x000c0000;
								waitval = vram_wait_val;
							}
							return d_sprite;
						}
					}
					__UNLIKELY_IF(addr >= 0x000cff80) {
						is_exists = true;
						// offset = addr - 0xcff80;
						return this;
					}
					// MEMORY (OK?)
					is_exists = true;
					offset = addr - 0x000c0000;
					memptr = (uintptr_t)ram_pagec;
					return NULL;
				}
				break;
			case 0x0d:
				__LIKELY_IF(!(dma_is_vram)) {
					is_exists = true;
					offset = addr - 0x000c0000;
					memptr = (uintptr_t)ram_pagec;
					return NULL;
				} else {
					if(select_d0_dict) {
						if((addr < 0x000d8000) && (is_read)){
							__LIKELY_IF(d_dictionary != NULL) {
								is_exists = true;
							}
							return d_dictionary;
						}
						if((addr >= 0x000d8000) && (addr < 0x000da000)){
							__LIKELY_IF(d_dictionary != NULL) {
								is_exists = true;
							}
							return d_dictionary;
						}
					}
				}
				is_exists = false;
				return NULL;
				break;
			case 0x0e:
				__LIKELY_IF(!(dma_is_vram)) {
					is_exists = true;
					offset = addr - 0x000c0000;
					memptr = (uintptr_t)ram_pagec;
					return NULL;
				}
				is_exists = false;
				return NULL;
				break;
			case 0x0f:
				if(addr < 0x000f8000) {
					is_exists = true;
					offset = addr - 0x000c0000;
					memptr = (uintptr_t)ram_pagec;
					return NULL;
				} else {
					if(select_d0_rom) {
						if(is_read) {
							__LIKELY_IF(d_sysrom != NULL) {
								is_exists = true;
								// OFFSET

							}
							return d_sysrom;
						} else {
							is_exists = false;
							return NULL;
						}
					} else {
						is_exists = true;
						offset = addr - 0x000c0000;
						memptr = (uintptr_t)ram_pagec;
						return NULL;
					}
				}
				break;
			default: // 00000000 - 000bffff
				is_exists = true;
				offset = addr;
				memptr = (uintptr_t)ram_page0;
				return NULL;
				break;
			}
		}
		__LIKELY_IF(addr < (0x00100000 + extram_size)) {
			__LIKELY_IF(extra_ram != NULL) {
				is_exists = true;
				offset = addr - 0x00100000;
				memptr = (uintptr_t)extra_ram;
			}
			return NULL;
		}
		// I/O, ROMs
		switch(addr >> 24) {
		case 0x80: // 80xxxxxx : VRAM
			__LIKELY_IF(d_vram != NULL) {
				//offset = addr - 0x80000000; // OK?
				is_exists = true;
				waitval = vram_wait_val;
				return d_vram;
			}
			break;
		case 0x81: // 81000000 - 8101ffff : SPRITE
			__LIKELY_IF((d_sprite != NULL) && (addr < 0x81020000)) {
				//offset = addr - 0x80000000; // OK?
				is_exists = true;
				waitval = vram_wait_val; // OK?
				return d_sprite;
			}
			break;
		case 0xc0: // c0xxxxxx : ICCARD
		case 0xc1: // c1xxxxxx : ICCARD
			__LIKELY_IF(d_iccard[(addr >> 24) & 1] != NULL) {
				is_exists = true;
				offset = addr & 0x00ffffff;
				return d_iccard[(addr >> 24) & 1];
			}
			break;
		case 0xc2: // ROMs
		     {
				 switch((addr >> 20) & 0x0f) {
				 case 0: // MSDOS or DICT
					 if(is_read) {
						 if(addr >= 0xc2080000) {
							 __LIKELY_IF(d_dictionary != NULL) {
								 is_exists = true;
								 //offset = addr - 0xc2080000; // OK?
							 }
							 return d_dictionary;
						 } else {
							 __LIKELY_IF(d_msdos != NULL) {
								 is_exists = true;
								 //offset = addr - 0xc2000000; // OK?
							 }
							 return d_msdos;
						 }
					 }
					 break;
				 case 1: // FONT or CMOS
					 __LIKELY_IF((is_read) && (addr < 0xc2140000)) {
						 // FONT
						 __LIKELY_IF(d_font != NULL) {
							 is_exists = true;
							 // offset = addr - 0xc2100000;
						 }
						 return d_font;
					 }
					 __LIKELY_IF((addr >= 0xc2140000) && (addr < 0xc2142000)) {
						 // CMOS
						 __LIKELY_IF(d_dictionary != NULL) {
							 is_exists = true;
							 // offset = addr - 0xc2100000;
						 }
						 return d_dictionary ;
					 }
					 __LIKELY_IF((addr >= 0xc2180000) && (addr < 0xc2200000)) {
						 // FONT (20pixels)
						 __LIKELY_IF(d_font_20pix != NULL) {
							 is_exists = true;
							 // offset = addr - 0xc2100000;
						 }
						 return d_font_20pix;
					 }
					 break;
				 case 2: // PCM
					 if(addr < 0xc2200fff) {
						 __LIKELY_IF(d_pcm != NULL) {
							 // offset = addr - 0xc2200000;
							 is_exists = true;
						 }
						 return d_pcm;
					 }
				 default:
					 return NULL;
					 break;
				 }
			 }
		case 0xff:
			__LIKELY_IF((is_read) && (addr >= 0xfffc0000)) {
				__LIKELY_IF(d_sysrom != NULL) {
					is_exists = true;
					//offset = addr - 0xfffc0000;
				}
				return d_sysrom;
			}
			break;
		default:
			break;
		}
		#else
		__LIKELY_IF(addr < (extram_size + 0x00100000)) {
			__LIKELY_IF(addr >= 0x00100000) { // Extra RAM
				is_exists = (extra_ram != NULL) ? true : false;
				offset = addr - 0x00100000;
				memptr = (uintptr_t)extra_ram;
				return NULL;
			}
			__LIKELY_IF(addr < 0x000c0000) { // 1st RAM
				is_exists = true;
				offset = addr;
				memptr = (uintptr_t)ram_page0;
				return NULL;
			}
			__LIKELY_IF((dma_is_vram) && (addr < 0x000f0000)) {
				uint32_t map_ptr = (addr - 0x000c0000) >> 12; // Per 4KBytes
				DEVICE* p = (is_read) ? devmap_c0000h_read[map_ptr] : devmap_c0000h_write[map_ptr];
				__LIKELY_IF(p != NULL) {
					is_exists = true;
					offset = (is_read) ? offsetmap_c0000h_read[map_ptr] : offsetmap_c0000h_write[map_ptr];
					__UNLIKELY_IF(addr < 0x000d0000) {
						waitval = vram_wait_val;
					}
				}
				return p;
			}
			__UNLIKELY_IF((addr >= 0x000cff80) && (addr < 0x000d0000)) { // MMIO
				is_exists = true;
				return this;
			}
			// 0x000f8000 - 0x000fffff
			__LIKELY_IF((select_d0_rom) && (addr >= 0x000f8000)) {
				is_exists = (is_read) ? true : false;
				offset = (addr - 0x000f8000) + 0x38000;
				return (is_read) ? d_sysrom : NULL;
			}
			is_exists = true;
			offset = addr - 0x0000c0000;
			memptr = (uintptr_t)ram_pagec;
			return NULL;
		}
		__LIKELY_IF(addr >= 0x80000000) {
			__LIKELY_IF(addr >= 0xfffc0000) { // SYSROM
				is_exists = true;
				offset = addr - 0xfffc0000;
				return d_sysrom;
			}
			__LIKELY_IF(addr < 0x84000000) { // VRAM
				uint32_t map_ptr = (addr - 0x80000000) >> 17; // Per 128KBytes
				DEVICE* p = devmap_80000000h[map_ptr];
				waitval = vram_wait_val; // ToDo.
				__LIKELY_IF(p != NULL) {
					offset = offsetmap_80000000h[map_ptr];
					is_exists = true;
				}
				return p;
			}
			__LIKELY_IF(addr >= 0xc0000000) {
				__LIKELY_IF(addr < 0xc2400000) { // MISC DEVICES
					uint32_t map_ptr = (addr - 0xc0000000) >> 12; // Per 4KBytes
					DEVICE* p = (is_read) ? devmap_c0000000h_read[map_ptr] : devmap_c0000000h_write[map_ptr];
					__LIKELY_IF(p != NULL) {
						offset = (is_read) ? offsetmap_c0000000h_read[map_ptr] : offsetmap_c0000000h_write[map_ptr];
						is_exists = true;
					}
					return p;
				}
				return NULL;
			}
			return NULL;
		}
		// ToDo: I/O SLOTS (40000000h)
		#endif
		return NULL;
	}
	virtual void __FASTCALL set_device_range_r(DEVICE* dev, uint32_t begin_addr, uint32_t end_addr);
	virtual void __FASTCALL set_device_range_w(DEVICE* dev, uint32_t begin_addr, uint32_t end_addr);
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
	uint8_t ram_page0[0xc0000];       // 0x00000000 - 0x000bffff : RAM
	uint8_t ram_pagec[0x40000];       // 0x000c0000 - 0x000fffff : URA? RAM

	uint8_t *extra_ram;                  // 0x00100000 - (0x3fffffff) : Size is defined by extram_size;
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
	virtual void set_wait_values();
	virtual void config_page_c0();
	virtual void config_page_d0_e0();
	virtual void config_page_f8_rom();
	virtual void config_page00();
	virtual void update_machine_features();

	virtual bool set_cpu_clock_by_wait();
	virtual void     __FASTCALL write_fmr_ports8(uint32_t addr, uint32_t data);
	virtual uint8_t  __FASTCALL read_fmr_ports8(uint32_t addr);
	virtual void     __FASTCALL write_sys_ports8(uint32_t addr, uint32_t data);
	virtual uint8_t __FASTCALL read_sys_ports8(uint32_t addr);
	inline bool is_faster_wait()
	{
		return ((mem_wait_val == 0) && (vram_wait_val < 3)) ? true : false;
	}
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
		for(int i = 0; i < (sizeof(devmap_c0000h_read) / sizeof(DEVICE*)); i++) {
			devmap_c0000h_read[i] = NULL;
			devmap_c0000h_write[i] = NULL;
			offsetmap_c0000h_read[i] = UINT32_MAX;
			offsetmap_c0000h_write[i] = UINT32_MAX;
		}
		for(int i = 0; i < (sizeof(devmap_80000000h) / sizeof(DEVICE*)); i++) {
			devmap_80000000h[i] = NULL;
			offsetmap_80000000h[i] = UINT32_MAX;
		}
		for(int i = 0; i < (sizeof(devmap_c0000000h_read) / sizeof(DEVICE*)); i++) {
			devmap_c0000000h_read[i] = NULL;
			devmap_c0000000h_write[i] = NULL;
			offsetmap_c0000000h_read[i] = UINT32_MAX;
			offsetmap_c0000000h_write[i] = UINT32_MAX;
		}
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
		extra_ram = NULL;
	}
	~TOWNS_MEMORY() {}

	// common functions
	void initialize();
	void release();
	void reset();

	virtual uint32_t __FASTCALL read_data8w(uint32_t addr, int* wait) override;
	//virtual uint32_t __FASTCALL read_data16w(uint32_t addr, int* wait) override;
	//virtual uint32_t __FASTCALL read_data32w(uint32_t addr, int* wait) override;
	virtual void __FASTCALL write_data8w(uint32_t addr, uint32_t data, int* wait) override;
	//virtual void __FASTCALL write_data16w(uint32_t addr, uint32_t data, int* wait) override;
	//virtual void __FASTCALL write_data32w(uint32_t addr, uint32_t data, int* wait) override;

	virtual void     __FASTCALL write_io8(uint32_t addr, uint32_t data);
	virtual void     __FASTCALL write_io8w(uint32_t addr, uint32_t data, int *wait);

	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	virtual uint32_t __FASTCALL read_io8w(uint32_t addr, int *wait);

	virtual void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr);
	virtual void __FASTCALL write_memory_mapped_io8w(uint32_t addr, uint32_t data, int *wait);
	virtual uint32_t __FASTCALL read_memory_mapped_io8w(uint32_t addr, int *wait);
	virtual void __FASTCALL write_memory_mapped_io16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_memory_mapped_io16(uint32_t addr);
	virtual void __FASTCALL write_memory_mapped_io16w(uint32_t addr, uint32_t data, int *wait);
	virtual uint32_t __FASTCALL read_memory_mapped_io16w(uint32_t addr, int *wait);

	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t __FASTCALL read_signal(int ch);

	//void event_frame();
	virtual void __FASTCALL set_intr_line(bool line, bool pending, uint32_t bit);

	bool process_state(FILEIO* state_fio, bool loading);

	// unique functions
	void set_extra_ram_size(uint32_t megabytes)
	{
		uint32_t limit = 5;
		uint32_t minimum = 2;
		switch(machine_id & 0xff00) {
		case 0x0000: // ???
		case 0x0100: // Towns Model 1/2
			minimum = 1;
			break;
		case 0x0200: // TOWNS 2F/2H
		case 0x0400: // TOWNS 10F/10H/20F/20H
			limit = 8; // 2MB + 2MB x 3
			break;
		case 0x0300: // TOWNS2 UX
		case 0x0600: // TOWNS2 UG
			limit = 9; // 2MB + 4MB x 2? - 1MB
			break;
		case 0x0500: // TOWNS2 CX
		case 0x0800: // TOWNS2 HG
			limit = 15; // 8MB x 2 - 1MB?
			break;
		case 0x0700: // TOWNS2 HR
		case 0x0900: // TOWNS2 UR
		case 0x0B00: // TOWNS2 MA
		case 0x0C00: // TOWNS2 MX
		case 0x0D00: // TOWNS2 ME
		case 0x0F00: // TOWNS2 MF/Fresh
			limit = 31; // 16MB x 2 - 1MB?
			break;
		}
		if(megabytes > limit) megabytes = limit;
		if(megabytes < minimum) megabytes = minimum;
		extram_size = megabytes << 20;
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
		set_device_range_r(device, 0xc2100000, 0xc2140000);
	}
	void set_context_font_20pix_rom(DEVICE* device)
	{
		d_font_20pix = device;
		set_device_range_r(device, 0xc2180000, 0xc2200000);
	}
	void set_context_dictionary(DEVICE* device)
	{
		d_dictionary = device;
		set_device_range_r(device, 0xc2080000, 0xc2100000);
		set_device_range_r(device, 0xc2140000, 0xc2142000);
		// CMOS
		set_device_range_w(device, 0xc2140000, 0xc2142000);
	}
	void set_context_msdos(DEVICE* device)
	{
		d_msdos = device;
		set_device_range_r(device, 0xc2000000, 0xc2080000);
	}
	void set_context_timer(DEVICE* device)
	{
		d_timer = device;
	}
	void set_context_sprite(DEVICE* device)
	{
		d_sprite = device;
		set_device_range_r(device, 0x81000000, 0x81020000);
		set_device_range_w(device, 0x81000000, 0x81020000);
	}
	void set_context_crtc(DEVICE* device)
	{
		d_crtc = device;
	}
	void set_context_iccard(DEVICE* device, int num)
	{
		d_iccard[num & 1] = device;
		uint32_t begin_addr = ((num & 1) == 0) ? 0xc0000000 : 0xc1000000;
		set_device_range_r(device, begin_addr, begin_addr + 0x01000000);
		set_device_range_w(device, begin_addr, begin_addr + 0x01000000);
	}
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
		set_device_range_r(device, 0xc2200000, 0xc2200fff);
		set_device_range_w(device, 0xc2200000, 0xc2200fff);
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
