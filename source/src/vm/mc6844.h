/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2017.04.14-

	[ MC6844 ]
*/

#ifndef _MC6844_H_
#define _MC6844_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_MC6844_TX_RQ_0	0
#define SIG_MC6844_TX_RQ_1	1
#define SIG_MC6844_TX_RQ_2	2
#define SIG_MC6844_TX_RQ_3	3

class MC6844 : public DEVICE
{
private:
	DEVICE* d_memory;
	
	struct {
		DEVICE *device;
		pair_t address_reg;
		pair_t byte_count_reg;
		uint8_t channel_ctrl_reg;
	} dma[4];
	
	uint8_t priority_ctrl_reg;
	uint8_t interrupt_ctrl_reg;
	uint8_t data_chain_reg;
	
	outputs_t outputs_irq;
	
	void transfer(int ch);
	void update_irq();
	
public:
	MC6844(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
//		for(int i = 0; i < 4; i++) {
//			dma[i].device = vm->dummy;
//		}
//		d_memory = vm->dummy;
		for(int i = 0; i < 4; i++) {
			dma[i].device = NULL;
		}
		d_memory = NULL;
		initialize_output_signals(&outputs_irq);
		set_device_name(_T("MC6844 DMAC"));
	}
	~MC6844() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_memory(DEVICE* device)
	{
		d_memory = device;
	}
	void set_context_ch0(DEVICE* device)
	{
		dma[0].device = device;
	}
	void set_context_ch1(DEVICE* device)
	{
		dma[1].device = device;
	}
	void set_context_ch2(DEVICE* device)
	{
		dma[2].device = device;
	}
	void set_context_ch3(DEVICE* device)
	{
		dma[3].device = device;
	}
};

#endif

