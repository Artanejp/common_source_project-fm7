/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ device base class ]
*/

#ifndef _LIBCPU_NEWDEV_DEVICE_H_
#define _LIBCPU_NEWDEV_DEVICE_H_

#include <stdarg.h>
#include "vm.h"
#include "../emu.h"
#if defined(_USE_QT)
#include "osd.h"

//#define USE_DEVICE_NAME
#endif
// max devices connected to the output port
#define MAX_OUTPUT	16

// common signal id
#define SIG_CPU_IRQ		101
#define SIG_CPU_FIRQ		102
#define SIG_CPU_NMI		103
#define SIG_CPU_BUSREQ		104
#define SIG_CPU_DEBUG		105

#define SIG_PRINTER_DATA	201
#define SIG_PRINTER_STROBE	202
#define SIG_PRINTER_RESET	203
#define SIG_PRINTER_BUSY	204
#define SIG_PRINTER_ACK		205
#define SIG_PRINTER_SELECT	206

#define SIG_SCSI_DAT		301
#define SIG_SCSI_BSY		302
#define SIG_SCSI_CD		303
#define SIG_SCSI_IO		304
#define SIG_SCSI_MSG		305
#define SIG_SCSI_REQ		306
#define SIG_SCSI_SEL		307
#define SIG_SCSI_ATN		308
#define SIG_SCSI_ACK		309
#define SIG_SCSI_RST		310

class CSP_Logger;
class csp_state_utils;
class VM;
class EMU;
class OSD;
class DEVICE
{
protected:
	VM* vm;
	EMU* emu;
	OSD* osd;
	csp_state_utils *state_entry;
	CSP_Logger *p_logger;
public:
	DEVICE(VM* parent_vm, EMU* parent_emu);
	//ToDo: Will implement real destructor per real classes and below destructor decl. with "virtual".
	// This makes warning:
	//"deleting object of polymorphic class type 'DEVICE' which has non-virtual
	// destructor might cause undefined behavior [-Wdelete-non-virtual-dtor]".
	//~DEVICE(void);
	virtual ~DEVICE() {}
	
	virtual void initialize() { /* osd = emu->get_osd(); */}
	virtual void release();
	
	virtual void update_config() {}
	virtual void save_state(FILEIO* state_fio) {}
	virtual bool load_state(FILEIO* state_fio)
	{
		return true;
	}
	
	virtual void decl_state(void);
	virtual void enter_decl_state(int version);
	virtual void enter_decl_state(int version,  _TCHAR *name); 
	virtual void leave_decl_state(void);
	
	// control
	virtual void reset() {}
	virtual void special_reset()
	{
		reset();
	}
	
	// NOTE: the virtual bus interface functions for 16/32bit access invite the cpu is little endian.
	// if the cpu is big endian, you need to implement them in the virtual machine memory/io classes.
	
	// memory bus
	virtual void write_data8(uint32_t addr, uint32_t data) {}
	virtual uint32_t read_data8(uint32_t addr)
	{
		return 0xff;
	}
	virtual void write_data16(uint32_t addr, uint32_t data)
	{
		write_data8(addr, data & 0xff);
		write_data8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32_t read_data16(uint32_t addr)
	{
		uint32_t val = read_data8(addr);
		val |= read_data8(addr + 1) << 8;
		return val;
	}
	virtual void write_data32(uint32_t addr, uint32_t data)
	{
		write_data16(addr, data & 0xffff);
		write_data16(addr + 2, (data >> 16) & 0xffff);
	}
	virtual uint32_t read_data32(uint32_t addr)
	{
		uint32_t val = read_data16(addr);
		val |= read_data16(addr + 2) << 16;
		return val;
	}
	virtual void write_data8w(uint32_t addr, uint32_t data, int* wait)
	{
		*wait = 0;
		write_data8(addr, data);
	}
	virtual uint32_t read_data8w(uint32_t addr, int* wait)
	{
		*wait = 0;
		return read_data8(addr);
	}
	virtual void write_data16w(uint32_t addr, uint32_t data, int* wait)
	{
		int wait_l, wait_h;
		write_data8w(addr, data & 0xff, &wait_l);
		write_data8w(addr + 1, (data >> 8) & 0xff, &wait_h);
		*wait = wait_l + wait_h;
	}
	virtual uint32_t read_data16w(uint32_t addr, int* wait)
	{
		int wait_l, wait_h;
		uint32_t val = read_data8w(addr, &wait_l);
		val |= read_data8w(addr + 1, &wait_h) << 8;
		*wait = wait_l + wait_h;
		return val;
	}
	virtual void write_data32w(uint32_t addr, uint32_t data, int* wait)
	{
		int wait_l, wait_h;
		write_data16w(addr, data & 0xffff, &wait_l);
		write_data16w(addr + 2, (data >> 16) & 0xffff, &wait_h);
		*wait = wait_l + wait_h;
	}
	virtual uint32_t read_data32w(uint32_t addr, int* wait)
	{
		int wait_l, wait_h;
		uint32_t val = read_data16w(addr, &wait_l);
		val |= read_data16w(addr + 2, &wait_h) << 16;
		*wait = wait_l + wait_h;
		return val;
	}
	virtual uint32_t fetch_op(uint32_t addr, int *wait)
	{
		return read_data8w(addr, wait);
	}
	virtual void write_dma_data8(uint32_t addr, uint32_t data)
	{
		write_data8(addr, data);
	}
	virtual uint32_t read_dma_data8(uint32_t addr)
	{
		return read_data8(addr);
	}
	virtual void write_dma_data16(uint32_t addr, uint32_t data)
	{
		write_data16(addr, data);
	}
	virtual uint32_t read_dma_data16(uint32_t addr)
	{
		return read_data16(addr);
	}
	virtual void write_dma_data32(uint32_t addr, uint32_t data)
	{
		write_data32(addr, data);
	}
	virtual uint32_t read_dma_data32(uint32_t addr)
	{
		return read_data32(addr);
	}
	virtual void write_dma_data8w(uint32_t addr, uint32_t data, int* wait)
	{
		write_data8w(addr, data, wait);
	}
	virtual uint32_t read_dma_data8w(uint32_t addr, int* wait)
	{
		return read_data8w(addr, wait);
	}
	virtual void write_dma_data16w(uint32_t addr, uint32_t data, int* wait)
	{
		write_data16w(addr, data, wait);
	}
	virtual uint32_t read_dma_data16w(uint32_t addr, int* wait)
	{
		return read_data16w(addr, wait);
	}
	virtual void write_dma_data32w(uint32_t addr, uint32_t data, int* wait)
	{
		write_data32w(addr, data, wait);
	}
	virtual uint32_t read_dma_data32w(uint32_t addr, int* wait)
	{
		return read_data32w(addr, wait);
	}
	
	// i/o bus
	virtual void write_io8(uint32_t addr, uint32_t data) {}
	virtual uint32_t read_io8(uint32_t addr);
	virtual void write_io16(uint32_t addr, uint32_t data)
	{
		write_io8(addr, data & 0xff);
		write_io8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32_t read_io16(uint32_t addr)
	{
		uint32_t val = read_io8(addr);
		val |= read_io8(addr + 1) << 8;
		return val;
	}
	virtual void write_io32(uint32_t addr, uint32_t data)
	{
		write_io16(addr, data & 0xffff);
		write_io16(addr + 2, (data >> 16) & 0xffff);
	}
	virtual uint32_t read_io32(uint32_t addr)
	{
		uint32_t val = read_io16(addr);
		val |= read_io16(addr + 2) << 16;
		return val;
	}
	virtual void write_io8w(uint32_t addr, uint32_t data, int* wait)
	{
		*wait = 0;
		write_io8(addr, data);
	}
	virtual uint32_t read_io8w(uint32_t addr, int* wait)
	{
		*wait = 0;
		return read_io8(addr);
	}
	virtual void write_io16w(uint32_t addr, uint32_t data, int* wait)
	{
		int wait_l, wait_h;
		write_io8w(addr, data & 0xff, &wait_l);
		write_io8w(addr + 1, (data >> 8) & 0xff, &wait_h);
		*wait = wait_l + wait_h;
	}
	virtual uint32_t read_io16w(uint32_t addr, int* wait)
	{
		int wait_l, wait_h;
		uint32_t val = read_io8w(addr, &wait_l);
		val |= read_io8w(addr + 1, &wait_h) << 8;
		*wait = wait_l + wait_h;
		return val;
	}
	virtual void write_io32w(uint32_t addr, uint32_t data, int* wait)
	{
		int wait_l, wait_h;
		write_io16w(addr, data & 0xffff, &wait_l);
		write_io16w(addr + 2, (data >> 16) & 0xffff, &wait_h);
		*wait = wait_l + wait_h;
	}
	virtual uint32_t read_io32w(uint32_t addr, int* wait)
	{
		int wait_l, wait_h;
		uint32_t val = read_io16w(addr, &wait_l);
		val |= read_io16w(addr + 2, &wait_h) << 16;
		*wait = wait_l + wait_h;
		return val;
	}
	virtual void write_dma_io8(uint32_t addr, uint32_t data)
	{
		write_io8(addr, data);
	}
	virtual uint32_t read_dma_io8(uint32_t addr)
	{
		return read_io8(addr);
	}
	virtual void write_dma_io16(uint32_t addr, uint32_t data)
	{
		write_io16(addr, data);
	}
	virtual uint32_t read_dma_io16(uint32_t addr)
	{
		return read_io16(addr);
	}
	virtual void write_dma_io32(uint32_t addr, uint32_t data)
	{
		write_io32(addr, data);
	}
	virtual uint32_t read_dma_io32(uint32_t addr)
	{
		return read_io32(addr);
	}
	virtual void write_dma_io8w(uint32_t addr, uint32_t data, int* wait)
	{
		write_io8w(addr, data, wait);
	}
	virtual uint32_t read_dma_io8w(uint32_t addr, int* wait)
	{
		return read_io8w(addr, wait);
	}
	virtual void write_dma_io16w(uint32_t addr, uint32_t data, int* wait)
	{
		write_io16w(addr, data, wait);
	}
	virtual uint32_t read_dma_io16w(uint32_t addr, int* wait)
	{
		return read_io16w(addr, wait);
	}
	virtual void write_dma_io32w(uint32_t addr, uint32_t data, int* wait)
	{
		write_io32w(addr, data, wait);
	}
	virtual uint32_t read_dma_io32w(uint32_t addr, int* wait)
	{
		return read_io32w(addr, wait);
	}
	
	// memory mapped i/o
	virtual void write_memory_mapped_io8(uint32_t addr, uint32_t data)
	{
		write_io8(addr, data);
	}
	virtual uint32_t read_memory_mapped_io8(uint32_t addr)
	{
		return read_io8(addr);
	}
	virtual void write_memory_mapped_io16(uint32_t addr, uint32_t data)
	{
		write_memory_mapped_io8(addr, data & 0xff);
		write_memory_mapped_io8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32_t read_memory_mapped_io16(uint32_t addr)
	{
		uint32_t val = read_memory_mapped_io8(addr);
		val |= read_memory_mapped_io8(addr + 1) << 8;
		return val;
	}
	virtual void write_memory_mapped_io32(uint32_t addr, uint32_t data)
	{
		write_memory_mapped_io16(addr, data & 0xffff);
		write_memory_mapped_io16(addr + 2, (data >> 16) & 0xffff);
	}
	virtual uint32_t read_memory_mapped_io32(uint32_t addr)
	{
		uint32_t val = read_memory_mapped_io16(addr);
		val |= read_memory_mapped_io16(addr + 2) << 16;
		return val;
	}
	virtual void write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait)
	{
		*wait = 0;
		write_memory_mapped_io8(addr, data);
	}
	virtual uint32_t read_memory_mapped_io8w(uint32_t addr, int* wait)
	{
		*wait = 0;
		return read_memory_mapped_io8(addr);
	}
	virtual void write_memory_mapped_io16w(uint32_t addr, uint32_t data, int* wait)
	{
		int wait_l, wait_h;
		write_memory_mapped_io8w(addr, data & 0xff, &wait_l);
		write_memory_mapped_io8w(addr + 1, (data >> 8) & 0xff, &wait_h);
		*wait = wait_l + wait_h;
	}
	virtual uint32_t read_memory_mapped_io16w(uint32_t addr, int* wait)
	{
		int wait_l, wait_h;
		uint32_t val = read_memory_mapped_io8w(addr, &wait_l);
		val |= read_memory_mapped_io8w(addr + 1, &wait_h) << 8;
		*wait = wait_l + wait_h;
		return val;
	}
	virtual void write_memory_mapped_io32w(uint32_t addr, uint32_t data, int* wait)
	{
		int wait_l, wait_h;
		write_memory_mapped_io16w(addr, data & 0xffff, &wait_l);
		write_memory_mapped_io16w(addr + 2, (data >> 16) & 0xffff, &wait_h);
		*wait = wait_l + wait_h;
	}
	virtual uint32_t read_memory_mapped_io32w(uint32_t addr, int* wait)
	{
		int wait_l, wait_h;
		uint32_t val = read_memory_mapped_io16w(addr, &wait_l);
		val |= read_memory_mapped_io16w(addr + 2, &wait_h) << 16;
		*wait = wait_l + wait_h;
		return val;
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
	virtual void write_signals(outputs_t *items, uint32_t data)
	{
		for(int i = 0; i < items->count; i++) {
			output_t *item = &items->item[i];
			int shift = item->shift;
			uint32_t val = (shift < 0) ? (data >> (-shift)) : (data << shift);
			uint32_t mask = (shift < 0) ? (item->mask >> (-shift)) : (item->mask << shift);
			item->device->write_signal(item->id, val, mask);
		}
	};
	virtual void write_signal(int id, uint32_t data, uint32_t mask) {}
	virtual uint32_t read_signal(int ch)
	{
		return 0;
	}
	
	// z80 daisy chain
	virtual void set_context_intr(DEVICE* device, uint32_t bit) {}
	virtual void set_context_child(DEVICE* device) {}
	
	// interrupt device to device
	virtual void set_intr_iei(bool val) {}
	
	// interrupt device to cpu
	virtual void set_intr_line(bool line, bool pending, uint32_t bit) {}
	
	// interrupt cpu to device
	virtual uint32_t get_intr_ack()
	{
		return 0xff;
	}
	virtual void notify_intr_reti() {}
	virtual void notify_intr_ei() {}
	
	// dma
	virtual void do_dma() {}
	
	// cpu
	virtual int run(int clock)
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
	virtual bool bios_call_far_i86(uint32_t PC, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
	{
		return false;
	}
	virtual bool bios_int_i86(int intnum, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
	{
		return false;
	}
	virtual bool bios_ret_z80(uint16_t PC, pair_t* af, pair_t* bc, pair_t* de, pair_t* hl, pair_t* ix, pair_t* iy, uint8_t* iff1)	{
		return false;
	}
	// misc
	const _TCHAR *get_device_name(void)
	{
		return (const _TCHAR *)this_device_name;
	}
   
	// event manager
	DEVICE* event_manager;
	
	virtual void set_context_event_manager(DEVICE* device)
	{
		event_manager = device;
	}
	virtual int get_event_manager_id();
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
	virtual void out_debug_log(const char *fmt, ...);
	virtual void force_out_debug_log(const char *fmt, ...);
#if 1
	// debugger
	// DEBUGGER is enabled by default.
	virtual void *get_debugger();
	virtual uint32_t get_debug_prog_addr_mask();
	virtual uint32_t get_debug_data_addr_mask();
	virtual void write_debug_data8(uint32_t addr, uint32_t data);
	virtual uint32_t read_debug_data8(uint32_t addr);
	virtual void write_debug_data16(uint32_t addr, uint32_t data);
	virtual uint32_t read_debug_data16(uint32_t addr);
	virtual void write_debug_data32(uint32_t addr, uint32_t data);
	virtual uint32_t read_debug_data32(uint32_t addr);
	virtual void write_debug_io8(uint32_t addr, uint32_t data);
	virtual uint32_t read_debug_io8(uint32_t addr);
	virtual void write_debug_io16(uint32_t addr, uint32_t data);
	virtual uint32_t read_debug_io16(uint32_t addr);
	virtual void write_debug_io32(uint32_t addr, uint32_t data);
	virtual uint32_t read_debug_io32(uint32_t addr);
	virtual bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	virtual void get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
#endif
	_TCHAR this_device_name[128];
	
	DEVICE* prev_device;
	DEVICE* next_device;
	int this_device_id;
};

#endif
