/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ uPD71071 ]
*/

#ifndef _UPD71071_H_
#define _UPD71071_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#ifndef SIG_UPD71071_CH0
#define SIG_UPD71071_CH0	0
#define SIG_UPD71071_CH1	1
#define SIG_UPD71071_CH2	2
#define SIG_UPD71071_CH3	3
#endif

class DEBUGGER;
class UPD71071 : public DEVICE
{
protected:
	DEVICE* d_cpu;
	DEVICE* d_mem;
	DEVICE* d_dma;
	DEBUGGER *d_debugger;
	bool _SINGLE_MODE_DMA;

	outputs_t outputs_tc[4];

	struct {
		DEVICE* dev;
		uint32_t areg, bareg;
		uint16_t creg, bcreg;
		uint8_t mode;
	} dma[4];

	uint8_t b16, selch, base;
	uint16_t cmd, tmp;
	uint8_t req, sreq, mask, tc;
	bool running;

	virtual void __FASTCALL inc_dec_ptr_a_byte(const int c, const bool inc);
	virtual void __FASTCALL inc_dec_ptr_two_bytes(const int c, const bool inc);

public:
	UPD71071(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 4; i++) {
			dma[i].dev = vm->dummy;
		}
		d_cpu = NULL;
		d_dma = NULL;
		d_debugger = NULL;
		_SINGLE_MODE_DMA = false;
		for(int c = 0; c < 4; c++) {
			initialize_output_signals(&outputs_tc[c]);
		}
		set_device_name(_T("uPD71071 DMAC"));
	}
	~UPD71071() {}

	// common functions
	virtual void initialize();
	virtual void reset();
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_io8(uint32_t addr) override;
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	virtual void do_dma() override;
	// for debug
	virtual void __FASTCALL write_via_debugger_data8w(uint32_t addr, uint32_t data, int *wait) override;
	virtual uint32_t __FASTCALL read_via_debugger_data8w(uint32_t addr, int *wait) override;
	virtual void __FASTCALL write_via_debugger_data16w(uint32_t addr, uint32_t data, int *wait) override;
	virtual uint32_t __FASTCALL read_via_debugger_data16w(uint32_t addr, int *wait) override;
	bool is_debugger_available() override
	{
		return (d_debugger != NULL) ? true : false;
	}
	void *get_debugger() override
	{
		return d_debugger;
	}
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;
	virtual bool process_state(FILEIO* state_fio, bool loading) override;

	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_memory(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_ch0(DEVICE* device)
	{
		dma[0].dev = device;
	}
	void set_context_ch1(DEVICE* device)
	{
		dma[1].dev = device;
	}
	void set_context_ch2(DEVICE* device)
	{
		dma[2].dev = device;
	}
	void set_context_ch3(DEVICE* device)
	{
		dma[3].dev = device;
	}
	void set_context_child_dma(DEVICE* device)
	{
		d_dma = device;
	}
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
	constexpr void set_context_tc(int ch, DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_tc[ch & 3], device, id, mask);
	}
	void set_context_tc0(DEVICE* device, int id, uint32_t mask)
	{
		set_context_tc(0, device, id, mask);
	}
	void set_context_tc1(DEVICE* device, int id, uint32_t mask)
	{
		set_context_tc(1, device, id, mask);
	}
	void set_context_tc2(DEVICE* device, int id, uint32_t mask)
	{
		set_context_tc(2, device, id, mask);
	}
	void set_context_tc3(DEVICE* device, int id, uint32_t mask)
	{
		set_context_tc(3, device, id, mask);
	}

};

#endif
