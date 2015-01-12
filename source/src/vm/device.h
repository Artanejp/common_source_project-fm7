/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ device base class ]
*/

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "vm.h"
#include "../emu.h"

// max devices connected to the output port
#define MAX_OUTPUT	16

// common signal id
#define SIG_CPU_IRQ	101
#define SIG_CPU_FIRQ	102
#define SIG_CPU_NMI	103
#define SIG_CPU_BUSREQ	104
#define SIG_CPU_DEBUG	201

class DEVICE
{
protected:
	VM* vm;
	EMU* emu;
public:
	DEVICE(VM* parent_vm, EMU* parent_emu) : vm(parent_vm), emu(parent_emu)
	{
		prev_device = vm->last_device;
		next_device = NULL;
		if(vm->first_device == NULL) {
			// this is the first device
			vm->first_device = this;
			this_device_id = 0;
		} else {
			// this is not the first device
			vm->last_device->next_device = this;
			this_device_id = vm->last_device->this_device_id + 1;
		}
		vm->last_device = this;
		
		// primary event manager
		event_manager = NULL;
	}
	~DEVICE(void) {}
	
	virtual void initialize() {}
	virtual void release() {}
	
	virtual void update_config() {}
	virtual void save_state(FILEIO* state_fio) {}
	virtual bool load_state(FILEIO* state_fio)
	{
		return true;
	}
	
	// control
	virtual void reset() {}
	virtual void special_reset()
	{
		reset();
	}
	
	// NOTE: the virtual bus interface functions for 16/32bit access invite the cpu is little endian.
	// if the cpu is big endian, you need to implement them in the virtual machine memory/io classes.
	
	// memory bus
	virtual void write_data8(uint32 addr, uint32 data) {}
	virtual uint32 read_data8(uint32 addr)
	{
		return 0xff;
	}
	virtual void write_data16(uint32 addr, uint32 data)
	{
		write_data8(addr, data & 0xff);
		write_data8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32 read_data16(uint32 addr)
	{
		uint32 val = read_data8(addr);
		val |= read_data8(addr + 1) << 8;
		return val;
	}
	virtual void write_data32(uint32 addr, uint32 data)
	{
		write_data16(addr, data & 0xffff);
		write_data16(addr + 2, (data >> 16) & 0xffff);
	}
	virtual uint32 read_data32(uint32 addr)
	{
		uint32 val = read_data16(addr);
		val |= read_data16(addr + 2) << 16;
		return val;
	}
	virtual void write_data8w(uint32 addr, uint32 data, int* wait)
	{
		*wait = 0;
		write_data8(addr, data);
	}
	virtual uint32 read_data8w(uint32 addr, int* wait)
	{
		*wait = 0;
		return read_data8(addr);
	}
	virtual void write_data16w(uint32 addr, uint32 data, int* wait)
	{
		int wait_l, wait_h;
		write_data8w(addr, data & 0xff, &wait_l);
		write_data8w(addr + 1, (data >> 8) & 0xff, &wait_h);
		*wait = wait_l + wait_h;
	}
	virtual uint32 read_data16w(uint32 addr, int* wait)
	{
		int wait_l, wait_h;
		uint32 val = read_data8w(addr, &wait_l);
		val |= read_data8w(addr + 1, &wait_h) << 8;
		*wait = wait_l + wait_h;
		return val;
	}
	virtual void write_data32w(uint32 addr, uint32 data, int* wait)
	{
		int wait_l, wait_h;
		write_data16w(addr, data & 0xffff, &wait_l);
		write_data16w(addr + 2, (data >> 16) & 0xffff, &wait_h);
		*wait = wait_l + wait_h;
	}
	virtual uint32 read_data32w(uint32 addr, int* wait)
	{
		int wait_l, wait_h;
		uint32 val = read_data16w(addr, &wait_l);
		val |= read_data16w(addr + 2, &wait_h) << 16;
		*wait = wait_l + wait_h;
		return val;
	}
	virtual uint32 fetch_op(uint32 addr, int *wait)
	{
		return read_data8w(addr, wait);
	}
	virtual void write_dma_data8(uint32 addr, uint32 data)
	{
		write_data8(addr, data);
	}
	virtual uint32 read_dma_data8(uint32 addr)
	{
		return read_data8(addr);
	}
	virtual void write_dma_data16(uint32 addr, uint32 data)
	{
		write_data16(addr, data);
	}
	virtual uint32 read_dma_data16(uint32 addr)
	{
		return read_data16(addr);
	}
	virtual void write_dma_data32(uint32 addr, uint32 data)
	{
		write_data32(addr, data);
	}
	virtual uint32 read_dma_data32(uint32 addr)
	{
		return read_data32(addr);
	}
	virtual void write_dma_data8w(uint32 addr, uint32 data, int* wait)
	{
		write_data8w(addr, data, wait);
	}
	virtual uint32 read_dma_data8w(uint32 addr, int* wait)
	{
		return read_data8w(addr, wait);
	}
	virtual void write_dma_data16w(uint32 addr, uint32 data, int* wait)
	{
		write_data16w(addr, data, wait);
	}
	virtual uint32 read_dma_data16w(uint32 addr, int* wait)
	{
		return read_data16w(addr, wait);
	}
	virtual void write_dma_data32w(uint32 addr, uint32 data, int* wait)
	{
		write_data32w(addr, data, wait);
	}
	virtual uint32 read_dma_data32w(uint32 addr, int* wait)
	{
		return read_data32w(addr, wait);
	}
	
	// i/o bus
	virtual void write_io8(uint32 addr, uint32 data) {}
	virtual uint32 read_io8(uint32 addr)
	{
#ifdef IOBUS_RETURN_ADDR
		return (addr & 1 ? addr >> 8 : addr) & 0xff;
#else
		return 0xff;
#endif
	}
	virtual void write_io16(uint32 addr, uint32 data)
	{
		write_io8(addr, data & 0xff);
		write_io8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32 read_io16(uint32 addr)
	{
		uint32 val = read_io8(addr);
		val |= read_io8(addr + 1) << 8;
		return val;
	}
	virtual void write_io32(uint32 addr, uint32 data)
	{
		write_io16(addr, data & 0xffff);
		write_io16(addr + 2, (data >> 16) & 0xffff);
	}
	virtual uint32 read_io32(uint32 addr)
	{
		uint32 val = read_io16(addr);
		val |= read_io16(addr + 2) << 16;
		return val;
	}
	virtual void write_io8w(uint32 addr, uint32 data, int* wait)
	{
		*wait = 0;
		write_io8(addr, data);
	}
	virtual uint32 read_io8w(uint32 addr, int* wait)
	{
		*wait = 0;
		return read_io8(addr);
	}
	virtual void write_io16w(uint32 addr, uint32 data, int* wait)
	{
		int wait_l, wait_h;
		write_io8w(addr, data & 0xff, &wait_l);
		write_io8w(addr + 1, (data >> 8) & 0xff, &wait_h);
		*wait = wait_l + wait_h;
	}
	virtual uint32 read_io16w(uint32 addr, int* wait)
	{
		int wait_l, wait_h;
		uint32 val = read_io8w(addr, &wait_l);
		val |= read_io8w(addr + 1, &wait_h) << 8;
		*wait = wait_l + wait_h;
		return val;
	}
	virtual void write_io32w(uint32 addr, uint32 data, int* wait)
	{
		int wait_l, wait_h;
		write_io16w(addr, data & 0xffff, &wait_l);
		write_io16w(addr + 2, (data >> 16) & 0xffff, &wait_h);
		*wait = wait_l + wait_h;
	}
	virtual uint32 read_io32w(uint32 addr, int* wait)
	{
		int wait_l, wait_h;
		uint32 val = read_io16w(addr, &wait_l);
		val |= read_io16w(addr + 2, &wait_h) << 16;
		*wait = wait_l + wait_h;
		return val;
	}
	virtual void write_dma_io8(uint32 addr, uint32 data)
	{
		write_io8(addr, data);
	}
	virtual uint32 read_dma_io8(uint32 addr)
	{
		return read_io8(addr);
	}
	virtual void write_dma_io16(uint32 addr, uint32 data)
	{
		write_io16(addr, data);
	}
	virtual uint32 read_dma_io16(uint32 addr)
	{
		return read_io16(addr);
	}
	virtual void write_dma_io32(uint32 addr, uint32 data)
	{
		write_io32(addr, data);
	}
	virtual uint32 read_dma_io32(uint32 addr)
	{
		return read_io32(addr);
	}
	virtual void write_dma_io8w(uint32 addr, uint32 data, int* wait)
	{
		write_io8w(addr, data, wait);
	}
	virtual uint32 read_dma_io8w(uint32 addr, int* wait)
	{
		return read_io8w(addr, wait);
	}
	virtual void write_dma_io16w(uint32 addr, uint32 data, int* wait)
	{
		write_io16w(addr, data, wait);
	}
	virtual uint32 read_dma_io16w(uint32 addr, int* wait)
	{
		return read_io16w(addr, wait);
	}
	virtual void write_dma_io32w(uint32 addr, uint32 data, int* wait)
	{
		write_io32w(addr, data, wait);
	}
	virtual uint32 read_dma_io32w(uint32 addr, int* wait)
	{
		return read_io32w(addr, wait);
	}
	
	// memory mapped i/o
	virtual void write_memory_mapped_io8(uint32 addr, uint32 data)
	{
		write_io8(addr, data);
	}
	virtual uint32 read_memory_mapped_io8(uint32 addr)
	{
		return read_io8(addr);
	}
	virtual void write_memory_mapped_io16(uint32 addr, uint32 data)
	{
		write_memory_mapped_io8(addr, data & 0xff);
		write_memory_mapped_io8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32 read_memory_mapped_io16(uint32 addr)
	{
		uint32 val = read_memory_mapped_io8(addr);
		val |= read_memory_mapped_io8(addr + 1) << 8;
		return val;
	}
	virtual void write_memory_mapped_io32(uint32 addr, uint32 data)
	{
		write_memory_mapped_io16(addr, data & 0xffff);
		write_memory_mapped_io16(addr + 2, (data >> 16) & 0xffff);
	}
	virtual uint32 read_memory_mapped_io32(uint32 addr)
	{
		uint32 val = read_memory_mapped_io16(addr);
		val |= read_memory_mapped_io16(addr + 2) << 16;
		return val;
	}
	virtual void write_memory_mapped_io8w(uint32 addr, uint32 data, int* wait)
	{
		*wait = 0;
		write_memory_mapped_io8(addr, data);
	}
	virtual uint32 read_memory_mapped_io8w(uint32 addr, int* wait)
	{
		*wait = 0;
		return read_memory_mapped_io8(addr);
	}
	virtual void write_memory_mapped_io16w(uint32 addr, uint32 data, int* wait)
	{
		int wait_l, wait_h;
		write_memory_mapped_io8w(addr, data & 0xff, &wait_l);
		write_memory_mapped_io8w(addr + 1, (data >> 8) & 0xff, &wait_h);
		*wait = wait_l + wait_h;
	}
	virtual uint32 read_memory_mapped_io16w(uint32 addr, int* wait)
	{
		int wait_l, wait_h;
		uint32 val = read_memory_mapped_io8w(addr, &wait_l);
		val |= read_memory_mapped_io8w(addr + 1, &wait_h) << 8;
		*wait = wait_l + wait_h;
		return val;
	}
	virtual void write_memory_mapped_io32w(uint32 addr, uint32 data, int* wait)
	{
		int wait_l, wait_h;
		write_memory_mapped_io16w(addr, data & 0xffff, &wait_l);
		write_memory_mapped_io16w(addr + 2, (data >> 16) & 0xffff, &wait_h);
		*wait = wait_l + wait_h;
	}
	virtual uint32 read_memory_mapped_io32w(uint32 addr, int* wait)
	{
		int wait_l, wait_h;
		uint32 val = read_memory_mapped_io16w(addr, &wait_l);
		val |= read_memory_mapped_io16w(addr + 2, &wait_h) << 16;
		*wait = wait_l + wait_h;
		return val;
	}
	
	// device to device
	typedef struct {
		DEVICE *device;
		int id;
		uint32 mask;
		int shift;
	} output_t;
	
	typedef struct {
		int count;
		output_t item[MAX_OUTPUT];
	} outputs_t;
	
	virtual void init_output_signals(outputs_t *items)
	{
		items->count = 0;
	}
	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32 mask, int shift)
	{
		int c = items->count++;
		items->item[c].device = device;
		items->item[c].id = id;
		items->item[c].mask = mask;
		items->item[c].shift = shift;
	}
	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32 mask)
	{
		int c = items->count++;
		items->item[c].device = device;
		items->item[c].id = id;
		items->item[c].mask = mask;
		items->item[c].shift = 0;
	}
	virtual void write_signals(outputs_t *items, uint32 data)
	{
		for(int i = 0; i < items->count; i++) {
			output_t *item = &items->item[i];
			int shift = item->shift;
			uint32 val = (shift < 0) ? (data >> (-shift)) : (data << shift);
			uint32 mask = (shift < 0) ? (item->mask >> (-shift)) : (item->mask << shift);
			item->device->write_signal(item->id, val, mask);
		}
	};
	virtual void write_signal(int id, uint32 data, uint32 mask) {}
	virtual uint32 read_signal(int ch)
	{
		return 0;
	}
	
	// z80 daisy chain
	virtual void set_context_intr(DEVICE* device, uint32 bit) {}
	virtual void set_context_child(DEVICE* device) {}
	
	// interrupt device to device
	virtual void set_intr_iei(bool val) {}
	
	// interrupt device to cpu
	virtual void set_intr_line(bool line, bool pending, uint32 bit) {}
	
	// interrupt cpu to device
	virtual uint32 intr_ack()
	{
		return 0xff;
	}
	virtual void intr_reti() {}
	virtual void intr_ei() {}
	
	// dma
	virtual void do_dma() {}
	
	// cpu
	virtual int run(int clock)
	{
		// when clock == -1, run one opecode
		return 0;
	}
	virtual void set_extra_clock(int clock) {}
	virtual int get_extra_clock()
	{
		return 0;
	}
	virtual uint32 get_pc()
	{
		return 0;
	}
	virtual uint32 get_next_pc()
	{
		return 0;
	}
	
	// bios
	virtual bool bios_call(uint32 PC, uint16 regs[], uint16 sregs[], int32* ZeroFlag, int32* CarryFlag)
	{
		return false;
	}
	virtual bool bios_int(int intnum, uint16 regs[], uint16 sregs[], int32* ZeroFlag, int32* CarryFlag)
	{
		return false;
	}
	
	// event manager
	DEVICE* event_manager;
	
	virtual void set_context_event_manager(DEVICE* device)
	{
		event_manager = device;
	}
	virtual int event_manager_id()
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->this_device_id;
	}
	virtual void register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->register_event(device, event_id, usec, loop, register_id);
	}
	virtual void register_event_by_clock(DEVICE* device, int event_id, int clock, bool loop, int* register_id)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->register_event_by_clock(device, event_id, clock, loop, register_id);
	}
	virtual void cancel_event(DEVICE* device, int register_id)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->cancel_event(device, register_id);
	}
	virtual void register_frame_event(DEVICE* device)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->register_frame_event(device);
	}
	virtual void register_vline_event(DEVICE* device)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->register_vline_event(device);
	}
	virtual uint32 current_clock()
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->current_clock();
	}
	virtual uint32 passed_clock(uint32 prev)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->passed_clock(prev);
	}
	virtual double passed_usec(uint32 prev)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->passed_usec(prev);
	}
	virtual uint32 get_cpu_pc(int index)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_cpu_pc(index);
	}
	virtual void request_skip_frames()
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->request_skip_frames();
	}
	virtual void set_frames_per_sec(double frames)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->set_frames_per_sec(frames);
	}
	virtual void set_lines_per_frame(int lines)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->set_lines_per_frame(lines);
	}
	virtual void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame) {}
	
	// event callback
	virtual void event_callback(int event_id, int err) {}
	virtual void event_pre_frame() {}	// this event is to update timing settings
	virtual void event_frame() {}
	virtual void event_vline(int v, int clock) {}
	virtual void event_hsync(int v, int h, int clock) {}
	
	// sound
	virtual void mix(int32* buffer, int cnt) {}
	
#ifdef USE_DEBUGGER
	// debugger
	virtual void *get_debugger()
	{
		return NULL;
	}
	virtual uint32 debug_prog_addr_mask()
	{
		return 0;
	}
	virtual uint32 debug_data_addr_mask()
	{
		return 0;
	}
	virtual void debug_write_data8(uint32 addr, uint32 data) {}
	virtual uint32 debug_read_data8(uint32 addr)
	{
		return 0xff;
	}
	virtual void debug_write_data16(uint32 addr, uint32 data)
	{
		debug_write_data8(addr, data & 0xff);
		debug_write_data8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32 debug_read_data16(uint32 addr)
	{
		uint32 val = debug_read_data8(addr);
		val |= debug_read_data8(addr + 1) << 8;
		return val;
	}
	virtual void debug_write_data32(uint32 addr, uint32 data)
	{
		debug_write_data16(addr, data & 0xffff);
		debug_write_data16(addr + 2, (data >> 16) & 0xffff);
	}
	virtual uint32 debug_read_data32(uint32 addr)
	{
		uint32 val = debug_read_data16(addr);
		val |= debug_read_data16(addr + 2) << 16;
		return val;
	}
	virtual void debug_write_io8(uint32 addr, uint32 data) {}
	virtual uint32 debug_read_io8(uint32 addr)
	{
		return 0xff;
	}
	virtual void debug_write_io16(uint32 addr, uint32 data)
	{
		debug_write_io8(addr, data & 0xff);
		debug_write_io8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32 debug_read_io16(uint32 addr)
	{
		uint32 val = debug_read_io8(addr);
		val |= debug_read_io8(addr + 1) << 8;
		return val;
	}
	virtual void debug_write_io32(uint32 addr, uint32 data)
	{
		debug_write_io16(addr, data & 0xffff);
		debug_write_io16(addr + 2, (data >> 16) & 0xffff);
	}
	virtual uint32 debug_read_io32(uint32 addr)
	{
		uint32 val = debug_read_io16(addr);
		val |= debug_read_io16(addr + 2) << 16;
		return val;
	}
	virtual bool debug_write_reg(_TCHAR *reg, uint32 data)
	{
		return false;
	}
	virtual void debug_regs_info(_TCHAR *buffer) {}
	virtual int debug_dasm(uint32 pc, _TCHAR *buffer)
	{
		return 0;
	}
#endif
	
	DEVICE* prev_device;
	DEVICE* next_device;
	int this_device_id;
};

#endif
