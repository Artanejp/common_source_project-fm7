/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ device base class ]
*/

#ifndef _LIBCPU_NEWDEV_DEVICE_H_
#define _LIBCPU_NEWDEV_DEVICE_H_

#include <stdarg.h>
//#include "vm_template.h"
#include "../../emu_template.h"
// max devices connected to the output port
#define MAX_OUTPUT	16

// common signal id
#define SIG_CPU_IRQ				101
#define SIG_CPU_FIRQ			102
#define SIG_CPU_NMI				103
#define SIG_CPU_BUSREQ			104
#define SIG_CPU_HALTREQ			105
#define SIG_CPU_DEBUG			106
#define SIG_CPU_ADDRESS_DIRTY	107
#define SIG_CPU_TOTAL_CYCLE_LO	108
#define SIG_CPU_TOTAL_CYCLE_HI	109
#define SIG_CPU_WAIT_FACTOR		110

#define SIG_PRINTER_DATA	201
#define SIG_PRINTER_STROBE	202
#define SIG_PRINTER_RESET	203
#define SIG_PRINTER_BUSY	204
#define SIG_PRINTER_ACK		205
#define SIG_PRINTER_SELECT	206

#define SIG_SCSI_DAT			301
#define SIG_SCSI_BSY			302
#define SIG_SCSI_CD				303
#define SIG_SCSI_IO				304
#define SIG_SCSI_MSG			305
#define SIG_SCSI_REQ			306
#define SIG_SCSI_SEL			307
#define SIG_SCSI_ATN			308
#define SIG_SCSI_ACK			309
#define SIG_SCSI_RST			310
#define SIG_SCSI_16BIT_BUS		311
#define SIG_SCSI_CLEAR_QUEUE	312

#include "vm_template.h"

class CSP_Logger;
class VM_TEMPLATE;
class EMU_TEMPLATE;
class OSD;
class DEVICE
{
protected:
	VM_TEMPLATE* vm;
	EMU_TEMPLATE* emu;
	OSD_BASE* osd;
	CSP_Logger *p_logger;
public:
	DEVICE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu);
	~DEVICE() {}

	virtual void initialize() { }
	virtual void release();

	virtual void clear_sound_in_source(int bank);
	// this function may be before (or after) initialize().
	virtual int add_sound_in_source(int rate, int samples, int channels);
	// this function may be before (or after) initialize().
	virtual int release_sound_in_source(int bank);
	virtual bool is_sound_in_source_exists(int bank);
	virtual int increment_sound_in_passed_data(int bank, double passed_usec);
	virtual int get_sound_in_buffers_count();
	virtual int get_sound_in_samples(int bank);
	virtual int get_sound_in_rate(int bank);
	virtual int get_sound_in_channels(int bank);
	// this function may be before (or after) initialize().
	virtual int16_t* get_sound_in_buf_ptr(int bank);

	virtual int write_sound_in_buffer(int bank, int32_t* src, int samples);
	// Add sampled values to sample buffer;value may be -32768 to +32767.
	// this function may be before (or after) initialize().
	virtual int get_sound_in_latest_data(int bank, int32_t* dst, int expect_channels);
	virtual int get_sound_in_data(int bank, int32_t* dst, int expect_samples, int expect_rate, int expect_channels);
	virtual void set_high_pass_filter_freq(int freq, double quality) { } // If freq < 0 disable HPF.
	virtual void set_low_pass_filter_freq(int freq, double quality) { }  // If freq <= 0 disable LPF.
	
	virtual void update_config() {}
	
	// control
	virtual void reset() {}
	virtual void special_reset()
	{
		reset();
	}
	virtual void save_state(FILEIO* state_fio) {}
	virtual bool load_state(FILEIO* state_fio)
	{
		return true;
	}
	virtual bool process_state(FILEIO* state_fio, bool loading)
	{
		if(loading) {
			return load_state(state_fio);
		} else {
			save_state(state_fio);
			return true;
		}
	}
	
	// NOTE: the virtual bus interface functions for 16/32bit access invite the cpu is little endian.
	// if the cpu is big endian, you need to implement them in the virtual machine memory/io classes.
	virtual uint32_t __FASTCALL translate_address(int segment, uint32_t offset) { /* offset must be numeric, not value */ return offset; }
	// memory bus
	virtual void __FASTCALL write_data8(uint32_t addr, uint32_t data) {}
	virtual uint32_t __FASTCALL read_data8(uint32_t addr)
	{
		return 0xff;
	}
	virtual void __FASTCALL write_data16(uint32_t addr, uint32_t data)
	{
		write_data8(addr,     (data     ) & 0xff);
		write_data8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32_t __FASTCALL read_data16(uint32_t addr)
	{
		uint32_t val;
		val  = read_data8(addr    );
		val |= read_data8(addr + 1) << 8;
		return val;
	}
	virtual void __FASTCALL write_data32(uint32_t addr, uint32_t data)
	{
		if(!(addr & 1)) {
			write_data16(addr,     (data      ) & 0xffff);
			write_data16(addr + 2, (data >> 16) & 0xffff);
		} else {
			write_data8 (addr,     (data      ) & 0x00ff);
			write_data16(addr + 1, (data >>  8) & 0xffff);
			write_data8 (addr + 3, (data >> 24) & 0x00ff);
		}
	}
	virtual uint32_t __FASTCALL read_data32(uint32_t addr)
	{
		if(!(addr & 1)) {
			uint32_t val;
			val  = read_data16(addr    );
			val |= read_data16(addr + 2) << 16;
			return val;
		} else {
			uint32_t val;
			val  = read_data8 (addr    );
			val |= read_data16(addr + 1) <<  8;
			val |= read_data8 (addr + 3) << 24;
			return val;
		}
	}
	virtual void __FASTCALL write_data8w(uint32_t addr, uint32_t data, int* wait)
	{
		*wait = 0;
		write_data8(addr, data);
	}
	virtual uint32_t __FASTCALL read_data8w(uint32_t addr, int* wait)
	{
		*wait = 0;
		return read_data8(addr);
	}
	virtual void __FASTCALL write_data16w(uint32_t addr, uint32_t data, int* wait)
	{
		int wait_0, wait_1;
		write_data8w(addr,     (data     ) & 0xff, &wait_0);
		write_data8w(addr + 1, (data >> 8) & 0xff, &wait_1);
		*wait = wait_0 + wait_1;
	}
	virtual uint32_t __FASTCALL read_data16w(uint32_t addr, int* wait)
	{
		int wait_0, wait_1;
		uint32_t val;
		val  = read_data8w(addr,     &wait_0);
		val |= read_data8w(addr + 1, &wait_1) << 8;
		*wait = wait_0 + wait_1;
		return val;
	}
	virtual void __FASTCALL write_data32w(uint32_t addr, uint32_t data, int* wait)
	{
		if(!(addr & 1)) {
			int wait_0, wait_1;
			write_data16w(addr,     (data      ) & 0xffff, &wait_0);
			write_data16w(addr + 2, (data >> 16) & 0xffff, &wait_1);
			*wait = wait_0 + wait_1;
		} else {
			int wait_0, wait_1, wait_2;
			write_data8w (addr,     (data      ) & 0x00ff, &wait_0);
			write_data16w(addr + 1, (data >>  8) & 0xffff, &wait_1);
			write_data8w (addr + 3, (data >> 24) & 0x00ff, &wait_2);
			*wait = wait_0 + wait_1 + wait_2;
		}
	}
	virtual uint32_t __FASTCALL read_data32w(uint32_t addr, int* wait)
	{
		if(!(addr & 1)) {
			int wait_0, wait_1;
			uint32_t val;
			val  = read_data16w(addr,     &wait_0);
			val |= read_data16w(addr + 2, &wait_1) << 16;
			*wait = wait_0 + wait_1;
			return val;
		} else {
			int wait_0, wait_1, wait_2;
			uint32_t val;
			val  = read_data8w (addr,     &wait_0);
			val |= read_data16w(addr + 1, &wait_1) <<  8;
			val |= read_data8w (addr + 3, &wait_2) << 24;
			*wait = wait_0 + wait_1 + wait_2;
			return val;
		}
	}
	virtual uint32_t __FASTCALL fetch_op(uint32_t addr, int *wait)
	{
		return read_data8w(addr, wait);
	}
	virtual void __FASTCALL write_dma_data8(uint32_t addr, uint32_t data)
	{
		write_data8(addr, data);
	}
	virtual uint32_t __FASTCALL read_dma_data8(uint32_t addr)
	{
		return read_data8(addr);
	}
	virtual void __FASTCALL write_dma_data16(uint32_t addr, uint32_t data)
	{
		write_data16(addr, data);
	}
	virtual uint32_t __FASTCALL read_dma_data16(uint32_t addr)
	{
		return read_data16(addr);
	}
	virtual void __FASTCALL write_dma_data32(uint32_t addr, uint32_t data)
	{
		write_data32(addr, data);
	}
	virtual uint32_t __FASTCALL read_dma_data32(uint32_t addr)
	{
		return read_data32(addr);
	}
	virtual void __FASTCALL write_dma_data8w(uint32_t addr, uint32_t data, int* wait)
	{
		write_data8w(addr, data, wait);
	}
	virtual uint32_t __FASTCALL read_dma_data8w(uint32_t addr, int* wait)
	{
		return read_data8w(addr, wait);
	}
	virtual void __FASTCALL write_dma_data16w(uint32_t addr, uint32_t data, int* wait)
	{
		write_data16w(addr, data, wait);
	}
	virtual uint32_t __FASTCALL read_dma_data16w(uint32_t addr, int* wait)
	{
		return read_data16w(addr, wait);
	}
	virtual void __FASTCALL write_dma_data32w(uint32_t addr, uint32_t data, int* wait)
	{
		write_data32w(addr, data, wait);
	}
	virtual uint32_t __FASTCALL read_dma_data32w(uint32_t addr, int* wait)
	{
		return read_data32w(addr, wait);
	}
	
	// i/o bus
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data) {}
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	virtual void __FASTCALL write_io16(uint32_t addr, uint32_t data)
	{
		write_io8(addr,     (data     ) & 0xff);
		write_io8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32_t __FASTCALL read_io16(uint32_t addr)
	{
		uint32_t val;
		val  = read_io8(addr    );
		val |= read_io8(addr + 1) << 8;
		return val;
	}
	virtual void __FASTCALL write_io32(uint32_t addr, uint32_t data)
	{
		if(!(addr & 1)) {
			write_io16(addr,     (data      ) & 0xffff);
			write_io16(addr + 2, (data >> 16) & 0xffff);
		} else {
			write_io8 (addr,     (data      ) & 0x00ff);
			write_io16(addr + 1, (data >>  8) & 0xffff);
			write_io8 (addr + 3, (data >> 24) & 0x00ff);
		}
	}
	virtual uint32_t __FASTCALL read_io32(uint32_t addr)
	{
		if(!(addr & 1)) {
			uint32_t val;
			val  = read_io16(addr    );
			val |= read_io16(addr + 2) << 16;
			return val;
		} else {
			uint32_t val;
			val  = read_io8 (addr    );
			val |= read_io16(addr + 1) <<  8;
			val |= read_io8 (addr + 3) << 24;
			return val;
		}
	}
	virtual void __FASTCALL write_io8w(uint32_t addr, uint32_t data, int* wait)
	{
		*wait = 0;
		write_io8(addr, data);
	}
	virtual uint32_t __FASTCALL read_io8w(uint32_t addr, int* wait)
	{
		*wait = 0;
		return read_io8(addr);
	}
	virtual void __FASTCALL write_io16w(uint32_t addr, uint32_t data, int* wait)
	{
		int wait_0, wait_1;
		write_io8w(addr,     (data     ) & 0xff, &wait_0);
		write_io8w(addr + 1, (data >> 8) & 0xff, &wait_1);
		*wait = wait_0 + wait_1;
	}
	virtual uint32_t __FASTCALL read_io16w(uint32_t addr, int* wait)
	{
		int wait_0, wait_1;
		uint32_t val;
		val  = read_io8w(addr,     &wait_0);
		val |= read_io8w(addr + 1, &wait_1) << 8;
		*wait = wait_0 + wait_1;
		return val;
	}
	virtual void __FASTCALL write_io32w(uint32_t addr, uint32_t data, int* wait)
	{
		if(!(addr & 1)) {
			int wait_0, wait_1;
			write_io16w(addr,     (data      ) & 0xffff, &wait_0);
			write_io16w(addr + 2, (data >> 16) & 0xffff, &wait_1);
			*wait = wait_0 + wait_1;
		} else {
			int wait_0, wait_1, wait_2;
			write_io8w (addr,     (data      ) & 0x00ff, &wait_0);
			write_io16w(addr + 1, (data >>  8) & 0xffff, &wait_1);
			write_io8w (addr + 3, (data >> 24) & 0x00ff, &wait_2);
			*wait = wait_0 + wait_1 + wait_2;
		}
	}
	virtual uint32_t __FASTCALL read_io32w(uint32_t addr, int* wait)
	{
		if(!(addr & 1)) {
			int wait_0, wait_1;
			uint32_t val;
			val  = read_io16w(addr,     &wait_0);
			val |= read_io16w(addr + 2, &wait_1) << 16;
			*wait = wait_0 + wait_1;
			return val;
		} else {
			int wait_0, wait_1, wait_2;
			uint32_t val;
			val  = read_io8w (addr,     &wait_0);
			val |= read_io16w(addr + 1, &wait_1) <<  8;
			val |= read_io8w (addr + 3, &wait_2) << 24;
			*wait = wait_0 + wait_1 + wait_2;
			return val;
		}
	}
	virtual void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data)
	{
		write_io8(addr, data);
	}
	virtual uint32_t __FASTCALL read_dma_io8(uint32_t addr)
	{
		return read_io8(addr);
	}
	virtual void __FASTCALL write_dma_io16(uint32_t addr, uint32_t data)
	{
		write_io16(addr, data);
	}
	virtual uint32_t __FASTCALL read_dma_io16(uint32_t addr)
	{
		return read_io16(addr);
	}
	virtual void __FASTCALL write_dma_io32(uint32_t addr, uint32_t data)
	{
		write_io32(addr, data);
	}
	virtual uint32_t __FASTCALL read_dma_io32(uint32_t addr)
	{
		return read_io32(addr);
	}
	virtual void __FASTCALL write_dma_io8w(uint32_t addr, uint32_t data, int* wait)
	{
		write_io8w(addr, data, wait);
	}
	virtual uint32_t __FASTCALL read_dma_io8w(uint32_t addr, int* wait)
	{
		return read_io8w(addr, wait);
	}
	virtual void __FASTCALL write_dma_io16w(uint32_t addr, uint32_t data, int* wait)
	{
		write_io16w(addr, data, wait);
	}
	virtual uint32_t __FASTCALL read_dma_io16w(uint32_t addr, int* wait)
	{
		return read_io16w(addr, wait);
	}
	virtual void __FASTCALL write_dma_io32w(uint32_t addr, uint32_t data, int* wait)
	{
		write_io32w(addr, data, wait);
	}
	virtual uint32_t __FASTCALL read_dma_io32w(uint32_t addr, int* wait)
	{
		return read_io32w(addr, wait);
	}
	
	// memory mapped i/o
	virtual void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data)
	{
		write_io8(addr, data);
	}
	virtual uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr)
	{
		return read_io8(addr);
	}
	virtual void __FASTCALL write_memory_mapped_io16(uint32_t addr, uint32_t data)
	{
		write_memory_mapped_io8(addr,     (data     ) & 0xff);
		write_memory_mapped_io8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32_t __FASTCALL read_memory_mapped_io16(uint32_t addr)
	{
		uint32_t val;
		val  = read_memory_mapped_io8(addr    );
		val |= read_memory_mapped_io8(addr + 1) << 8;
		return val;
	}
	virtual void __FASTCALL write_memory_mapped_io32(uint32_t addr, uint32_t data)
	{
		if(!(addr & 1)) {
			write_memory_mapped_io16(addr,     (data      ) & 0xffff);
			write_memory_mapped_io16(addr + 2, (data >> 16) & 0xffff);
		} else {
			write_memory_mapped_io8 (addr,     (data      ) & 0x00ff);
			write_memory_mapped_io16(addr + 1, (data >>  8) & 0xffff);
			write_memory_mapped_io8 (addr + 3, (data >> 24) & 0x00ff);
		}
	}
	virtual uint32_t __FASTCALL read_memory_mapped_io32(uint32_t addr)
	{
		if(!(addr & 1)) {
			uint32_t val;
			val  = read_memory_mapped_io16(addr    );
			val |= read_memory_mapped_io16(addr + 2) << 16;
			return val;
		} else {
			uint32_t val;
			val  = read_memory_mapped_io8 (addr    );
			val |= read_memory_mapped_io16(addr + 1) <<  8;
			val |= read_memory_mapped_io8 (addr + 3) << 24;
			return val;
		}
	}
	virtual void __FASTCALL write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait)
	{
		*wait = 0;
		write_memory_mapped_io8(addr, data);
	}
	virtual uint32_t __FASTCALL read_memory_mapped_io8w(uint32_t addr, int* wait)
	{
		*wait = 0;
		return read_memory_mapped_io8(addr);
	}
	virtual void __FASTCALL write_memory_mapped_io16w(uint32_t addr, uint32_t data, int* wait)
	{
		int wait_0, wait_1;
		write_memory_mapped_io8w(addr,     (data     ) & 0xff, &wait_0);
		write_memory_mapped_io8w(addr + 1, (data >> 8) & 0xff, &wait_1);
		*wait = wait_0 + wait_1;
	}
	virtual uint32_t __FASTCALL read_memory_mapped_io16w(uint32_t addr, int* wait)
	{
		int wait_0, wait_1;
		uint32_t val;
		val  = read_memory_mapped_io8w(addr,     &wait_0);
		val |= read_memory_mapped_io8w(addr + 1, &wait_1) << 8;
		*wait = wait_0 + wait_1;
		return val;
	}
	virtual void __FASTCALL write_memory_mapped_io32w(uint32_t addr, uint32_t data, int* wait)
	{
		if(!(addr & 1)) {
			int wait_0, wait_1;
			write_memory_mapped_io16w(addr,     (data      ) & 0xffff, &wait_0);
			write_memory_mapped_io16w(addr + 2, (data >> 16) & 0xffff, &wait_1);
			*wait = wait_0 + wait_1;
		} else {
			int wait_0, wait_1, wait_2;
			write_memory_mapped_io8w (addr,     (data      ) & 0x00ff, &wait_0);
			write_memory_mapped_io16w(addr + 1, (data >>  8) & 0xffff, &wait_1);
			write_memory_mapped_io8w (addr + 3, (data >> 24) & 0x00ff, &wait_2);
			*wait = wait_0 + wait_1 + wait_2;
		}
	}
	virtual uint32_t __FASTCALL read_memory_mapped_io32w(uint32_t addr, int* wait)
	{
		if(!(addr & 1)) {
			int wait_0, wait_1;
			uint32_t val;
			val  = read_memory_mapped_io16w(addr,     &wait_0);
			val |= read_memory_mapped_io16w(addr + 2, &wait_1) << 16;
			*wait = wait_0 + wait_1;
			return val;
		} else {
			int wait_0, wait_1, wait_2;
			uint32_t val;
			val  = read_memory_mapped_io8w (addr,     &wait_0);
			val |= read_memory_mapped_io16w(addr + 1, &wait_1) <<  8;
			val |= read_memory_mapped_io8w (addr + 3, &wait_2) << 24;
			*wait = wait_0 + wait_1 + wait_2;
			return val;
		}
	}
	
	// device to device
	typedef struct {
		DEVICE *device;
		int id;
		uint32_t mask;
		int shift;
	} output_t;
	
	typedef struct {
		int count;
		output_t item[MAX_OUTPUT];
	} outputs_t;
	
	virtual void initialize_output_signals(outputs_t *items)
	{
		items->count = 0;
	}
	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask, int shift)
	{
		int c = items->count++;
		items->item[c].device = device;
		items->item[c].id = id;
		items->item[c].mask = mask;
		items->item[c].shift = shift;
	}
	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask)
	{
		int c = items->count++;
		items->item[c].device = device;
		items->item[c].id = id;
		items->item[c].mask = mask;
		items->item[c].shift = 0;
	}
	virtual void update_signal_mask(outputs_t *items, DEVICE *device, uint32_t mask)
	{
		if(items == NULL) return;
		int c = items->count;
		if(c <= 0) return;
		if(c >= MAX_OUTPUT) c = MAX_OUTPUT - 1;
		// if (ARG:device == NULL) apply to all devices.
		for(int i = 0; i < c; i++) {
			if((device == NULL) || (device == items->item[i].device)) {
				items->item[i].mask = mask;
			}
		}
	}
	virtual void __FASTCALL write_signals(outputs_t *items, uint32_t data)
	{
		for(int i = 0; i < items->count; i++) {
			output_t *item = &items->item[i];
			int shift = item->shift;
			uint32_t val = (shift < 0) ? (data >> (-shift)) : (data << shift);
			uint32_t mask = (shift < 0) ? (item->mask >> (-shift)) : (item->mask << shift);
			item->device->write_signal(item->id, val, mask);
		}
	};
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) {}
	virtual uint32_t __FASTCALL read_signal(int ch)
	{
		return 0;
	}
	
	// z80 daisy chain
	virtual void set_context_intr(DEVICE* device, uint32_t bit) {}
	virtual void set_context_child(DEVICE* device) {}
	virtual DEVICE *get_context_child()
	{
		return NULL;
	}
	
	// interrupt device to device
	virtual void set_intr_iei(bool val) {}
	
	// interrupt device to cpu
	virtual void set_intr_line(bool line, bool pending, uint32_t bit) {}
	
	// interrupt cpu to device
	virtual uint32_t get_intr_ack()
	{
		return 0xff;
	}
	virtual void update_intr() {}
	virtual void notify_intr_reti() {}
	virtual void notify_intr_ei() {}
	
	// dma
	virtual void __FASTCALL do_dma() {}
	
	// cpu
	virtual int __FASTCALL run(int clock)
	{
		// when clock == -1, run one opecode
		return (clock == -1 ? 1 : clock);
	}
	virtual void set_extra_clock(int clock) {}
	virtual int get_extra_clock()
	{
		return 0;
	}
	virtual uint32_t get_pc()
	{
		return 0;
	}
	virtual uint32_t get_next_pc()
	{
		return 0;
	}
	
	// bios
	virtual bool bios_call_far_i86(uint32_t PC, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
	{
		return false;
	}
	virtual bool bios_int_i86(int intnum, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
	{
		return false;
	}
	virtual bool bios_call_far_ia32(uint32_t PC, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
	{
		return false;
	}
	virtual bool bios_int_ia32(int intnum, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
	{
		return false;
	}
	virtual bool bios_ret_z80(uint16_t PC, pair32_t* af, pair32_t* bc, pair32_t* de, pair32_t* hl, pair32_t* ix, pair32_t* iy, uint8_t* iff1)	{
		return false;
	}
	// misc
	const _TCHAR *get_device_name(void)
	{
		return (const _TCHAR *)this_device_name;
	}
	virtual bool address_translate(int space, int intention, uint64_t &taddress);
   
	// event manager
	DEVICE* event_manager;
	
	virtual void set_context_event_manager(DEVICE* device)
	{
		event_manager = device;
	}
	virtual int get_event_manager_id();
	virtual uint32_t get_event_clocks();
	virtual bool is_primary_cpu(DEVICE* device);
	virtual uint32_t get_cpu_clocks(DEVICE* device);
	virtual void update_extra_event(int clock);
	virtual void register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id);
	virtual void register_event_by_clock(DEVICE* device, int event_id, uint64_t clock, bool loop, int* register_id);
	virtual void cancel_event(DEVICE* device, int register_id);
	virtual void register_frame_event(DEVICE* device);
	virtual void register_vline_event(DEVICE* device);
	virtual uint32_t get_event_remaining_clock(int register_id);
	virtual double get_event_remaining_usec(int register_id);
	virtual uint32_t get_current_clock();
	virtual uint32_t get_passed_clock(uint32_t prev);
	virtual double get_passed_usec(uint32_t prev);
	virtual uint32_t get_passed_clock_since_vline();
	virtual double get_passed_usec_since_vline();
	virtual int get_cur_vline();
	virtual int get_cur_vline_clocks();
	virtual uint32_t get_cpu_pc(int index);
	virtual uint64_t get_current_clock_uint64();
	virtual uint32_t get_cpu_clock(int index);
	
	virtual void request_skip_frames();
	virtual void set_frames_per_sec(double frames);
	
	virtual void set_lines_per_frame(int lines);
	virtual int get_lines_per_frame(void);
	// Force render sound immediately when device's status has changed.
	// You must call this after you changing registers (or enything).
	// If has problems, try set_realtime_render.
	// See mb8877.cpp and ym2203.cpp. 
	// -- 20161010 K.O
	virtual void touch_sound(void);
	// Force render per 1 sample automatically.
	// See pcm1bit.cpp .
	// -- 20161010 K.O
	virtual void set_realtime_render(DEVICE *device, bool flag);
	virtual void set_realtime_render(bool flag)
	{
		set_realtime_render(this, flag);
	}
	virtual void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame) {}
	
	// event callback
	virtual void event_callback(int event_id, int err) {}
	virtual void event_pre_frame() {}	// this event is to update timing settings
	virtual void event_frame() {}
	virtual void event_vline(int v, int clock) {}
	virtual void event_hsync(int v, int h, int clock) {}
	
	// sound
	virtual void mix(int32_t* buffer, int cnt) {}
	virtual void set_volume(int ch, int decibel_l, int decibel_r) {} // +1 equals +0.5dB (same as fmgen)
	virtual void set_device_name(const _TCHAR *format, ...);

	// debugger
	// DEBUGGER is enabled by default.
	virtual bool is_cpu();
	virtual bool is_debugger();
	virtual bool is_debugger_available();
	virtual void *get_debugger();
	virtual uint32_t get_debug_prog_addr_mask();
	virtual uint32_t get_debug_data_addr_mask();
	virtual uint64_t get_debug_data_addr_space();
	virtual void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	virtual void __FASTCALL write_debug_data16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_data16(uint32_t addr);
	virtual void __FASTCALL write_debug_data32(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_data32(uint32_t addr);
	virtual void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_io8(uint32_t addr);
	virtual void __FASTCALL write_debug_io16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_io16(uint32_t addr);
	virtual void __FASTCALL write_debug_io32(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_io32(uint32_t addr);
	virtual bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_reg(const _TCHAR *reg);
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual bool get_debug_regs_description(_TCHAR *buffer, size_t buffer_len);
	virtual int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
	virtual int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
	virtual bool debug_rewind_call_trace(uint32_t pc, int &size, _TCHAR* buffer, size_t buffer_len, uint64_t userdata = 0);
/*
	These functions are used for debugging non-cpu device
	Insert debugger between standard read/write functions and these functions for checking breakpoints

	void DEVICE::write_data8(uint32_t addr, uint32_t data)
	{
		if(debugger != NULL && debugger->now_device_debugging) {
			// debugger->mem = this;
			// debugger->mem->write_via_debugger_data8(addr, data)
			debugger->write_via_debugger_data8(addr, data);
		} else {
			this->write_via_debugger_data8(addr, data);
		}
	}
	void DEVICE::write_via_debugger_data8(uint32_t addr, uint32_t data)
	{
		// write memory
	}
*/
	virtual void __FASTCALL write_via_debugger_data8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_via_debugger_data8(uint32_t addr);
	virtual void __FASTCALL write_via_debugger_data16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_via_debugger_data16(uint32_t addr);
	virtual void __FASTCALL write_via_debugger_data32(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_via_debugger_data32(uint32_t addr);
	virtual void __FASTCALL write_via_debugger_data8w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t __FASTCALL read_via_debugger_data8w(uint32_t addr, int* wait);
	virtual void __FASTCALL write_via_debugger_data16w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t __FASTCALL read_via_debugger_data16w(uint32_t addr, int* wait);
	virtual void __FASTCALL write_via_debugger_data32w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t __FASTCALL read_via_debugger_data32w(uint32_t addr, int* wait);
	virtual void __FASTCALL write_via_debugger_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_via_debugger_io8(uint32_t addr);
	virtual void __FASTCALL write_via_debugger_io16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_via_debugger_io16(uint32_t addr);
	virtual void __FASTCALL write_via_debugger_io32(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_via_debugger_io32(uint32_t addr);
	virtual void __FASTCALL write_via_debugger_io8w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t __FASTCALL read_via_debugger_io8w(uint32_t addr, int* wait);
	virtual void __FASTCALL write_via_debugger_io16w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t __FASTCALL read_via_debugger_io16w(uint32_t addr, int* wait);
	virtual void __FASTCALL write_via_debugger_io32w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t __FASTCALL read_via_debugger_io32w(uint32_t addr, int* wait);
	virtual void out_debug_log(const char *fmt, ...);
	virtual void force_out_debug_log(const char *fmt, ...);

	// misc
	const _TCHAR *get_lib_common_vm_version(void);
	_TCHAR this_device_name[128];
	
	// device node using with iterator.
	DEVICE* prev_device;
	DEVICE* next_device;
	int this_device_id;
};

#endif
