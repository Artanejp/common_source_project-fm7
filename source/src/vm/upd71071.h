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

#define SIG_UPD71071_CH0	0
#define SIG_UPD71071_CH1	1
#define SIG_UPD71071_CH2	2
#define SIG_UPD71071_CH3	3
#define SIG_UPD71071_IS_TRANSFERING 4 /* 4 - 7 */
#define SIG_UPD71071_IS_16BITS_TRANSFER 8 /* 8 - 11 */

class DEBUGGER;
class UPD71071 : public DEVICE
{
private:
	DEVICE* d_mem;
//#ifdef SINGLE_MODE_DMA
	DEVICE* d_dma;
//#endif
	DEBUGGER *d_debugger;
	outputs_t outputs_tc;
	outputs_t outputs_wrote_mem_byte;
	outputs_t outputs_wrote_mem_word;
	
	struct {
		DEVICE* dev;
		uint32_t areg, bareg;
		uint16_t creg, bcreg;
		uint8_t mode;
	} dma[4];
	
	uint8_t b16, selch, base;
	uint16_t cmd, tmp;
	uint8_t req, sreq, mask, tc;

	bool _SINGLE_MODE_DMA;
	bool _USE_DEBUGGER;
public:
	UPD71071(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		// TIP: if((DEVICE::prev_device == NULL) || (DEVICE::this_device_id == 0)) DEVICE must be DUMMY.
		// And, at this device, should not be FIRST DEVICE. 20170613 Ohta.
		DEVICE *__dev = this;
		while((__dev->prev_device != NULL) && (__dev->this_device_id > 0)) {
			__dev = __dev->prev_device;
		}
		for(int i = 0; i < 4; i++) {
			//dma[i].dev = vm->dummy;
			dma[i].dev = __dev;
		}
		d_mem = __dev;
//#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
//#endif
		d_debugger = NULL;
		_SINGLE_MODE_DMA = false;
		_USE_DEBUGGER = false;
		initialize_output_signals(&outputs_tc);
		initialize_output_signals(&outputs_wrote_mem_word);
		initialize_output_signals(&outputs_wrote_mem_byte);
		set_device_name(_T("uPD71071 DMAC"));
	}
	~UPD71071() {}
	
	// common functions
	void initialize();
	void reset();
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t __FASTCALL read_signal(int id);
	void __FASTCALL do_dma();
	// for debug
	void __FASTCALL write_via_debugger_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_via_debugger_data8(uint32_t addr);
	void __FASTCALL write_via_debugger_data16(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_via_debugger_data16(uint32_t addr);
	bool is_debugger_available()
	{
		return true;
	}
	void *get_debugger()
	{
		return d_debugger;
	}
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	bool process_state(FILEIO* state_fio, bool loading);
	// unique functions
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
//#ifdef SINGLE_MODE_DMA
	void set_context_child_dma(DEVICE* device)
	{
		d_dma = device;
	}
//#endif
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
	void set_context_tc(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_tc, device, id, mask);
	}
	void set_context_wrote_mem(DEVICE* device, int id)
	{
		register_output_signal(&outputs_wrote_mem_byte, device, id, 1);
		register_output_signal(&outputs_wrote_mem_word, device, id, 2);
	}
};

#endif

