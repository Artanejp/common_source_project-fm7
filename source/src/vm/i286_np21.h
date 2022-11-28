/*
	Skelton for retropc emulator

	Origin : np21/w i386c core
	Author : Takeda.Toshiya
	Date   : 2020.02.02-

	[ i286 ]
*/

#ifndef _I286_NP21_H_
#define _I286_NP21_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I286_A20	0

enum {
	INTEL_80286 = 0,
	NEC_V30,
	INTEL_8086,
	INTEL_80186,
};

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class I286 : public DEVICE
{
private:
	DEVICE *device_pic;
#ifdef USE_DEBUGGER
//	DEBUGGER *device_debugger;
	DEVICE *device_mem_stored;
	DEVICE *device_io_stored;
	uint64_t total_cycles;
	uint64_t prev_total_cycles;
#endif
	int remained_cycles, extra_cycles;
	bool busreq;
	bool nmi_pending, irq_pending;
	uint32_t PREV_CS_BASE;
	uint16_t CPU_PREV_IP;
	int run_one_opecode();
	
public:
	I286(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef USE_DEBUGGER
		total_cycles = prev_total_cycles = 0;
#endif
		busreq = false;
		device_model = INTEL_80286;
	}
	~I286() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int run(int cycles);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void set_intr_line(bool line, bool pending, uint32_t bit);
	void set_extra_clock(int cycles);
	int get_extra_clock();
	uint32_t get_pc();
	uint32_t get_next_pc();
#ifdef USE_DEBUGGER
	bool is_cpu()
	{
		return true;
	}
	bool is_debugger_available()
	{
		return true;
	}
	void *get_debugger();
//	{
//		return device_debugger;
//	}
	uint32_t get_debug_prog_addr_mask()
	{
		if(device_model == INTEL_80286) {
			return 0xffffff;
		} else {
			return 0xfffff;
		}
	}
	uint32_t get_debug_data_addr_mask()
	{
		if(device_model == INTEL_80286) {
			return 0xffffff;
		} else {
			return 0xfffff;
		}
	}
	void write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t read_debug_data8(uint32_t addr);
	void write_debug_data16(uint32_t addr, uint32_t data);
	uint32_t read_debug_data16(uint32_t addr);
	void write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t read_debug_io8(uint32_t addr);
	void write_debug_io16(uint32_t addr, uint32_t data);
	uint32_t read_debug_io16(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	uint32_t read_debug_reg(const _TCHAR *reg);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_mem(DEVICE* device);
//	{
//		device_mem = device;
//	}
	void set_context_io(DEVICE* device);
//	{
//		device_io = device;
//	}
	void set_context_intr(DEVICE* device)
	{
		device_pic = device;
	}
#ifdef I86_PSEUDO_BIOS
	void set_context_bios(DEVICE* device);
//	{
//		device_bios = device;
//	}
#endif
#ifdef SINGLE_MODE_DMA
	void set_context_dma(DEVICE* device);
//	{
//		device_dma = device;
//	}
#endif
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device);
//	{
//		device_debugger = device;
//	}
#endif
	void set_address_mask(uint32_t mask);
	uint32_t get_address_mask();
	void set_shutdown_flag(int shutdown);
	int get_shutdown_flag();
	int device_model;
};

#endif
