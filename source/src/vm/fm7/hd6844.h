/*
 * FM77AVDMA [hd6844.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jun 18, 2015 : Initial
 *
 */

#ifndef _VM_HD6844_H_
#define _VM_HD6844_H_


#include "../device.h"

class EMU;
class VM;

enum {
	HD6844_EVENT_START_TRANSFER = 0,
	HD6844_EVENT_DO_TRANSFER = 4,
};

enum {
	HD6844_TRANSFER_START = 1,
	HD6844_DO_TRANSFER,
	HD6844_ADDR_REG_0 = 4,
	HD6844_ADDR_REG_1,
	HD6844_ADDR_REG_2,
	HD6844_ADDR_REG_3,
	HD6844_WORDS_REG_0,
	HD6844_WORDS_REG_1,
	HD6844_WORDS_REG_2,
	HD6844_WORDS_REG_3,
	HD6844_SRC_FIXED_ADDR_CH0 = 16,
	HD6844_SRC_FIXED_ADDR_CH1,
	HD6844_SRC_FIXED_ADDR_CH2,
	HD6844_SRC_FIXED_ADDR_CH3,
	HD6844_SET_CONST_OFFSET,
	HD6844_IS_TRANSFER_0 = 24,
	HD6844_IS_TRANSFER_1,
	HD6844_IS_TRANSFER_2,
	HD6844_IS_TRANSFER_3,
};

class HD6844: public DEVICE {

 protected:
	EMU *p_emu;
	VM *p_vm;
	
 private:
	DEVICE *src[4];
	DEVICE *dest[4];

	outputs_t interrupt_line;
	outputs_t halt_line;
	// Registers

	uint32 addr_reg[4];
	uint16 words_reg[4];
	uint8 channel_control[4];
	
	uint8 priority_reg;
	uint8 interrupt_reg;
	uint8 datachain_reg;
	uint8 num_reg;
	uint32 addr_offset;
	
	bool transfering[4];
	bool first_transfer[4];
	bool cycle_steal;
   
	uint32 fixed_addr[4];
	uint8 data_reg[4];
	int event_dmac[4];

	void do_transfer(int ch);
 public:
	HD6844(VM *parent_vm, EMU *parent_emu);
	~HD6844();

	void event_callback(int event_id, int err);
	void write_data8(uint32 id, uint32 data);
	uint32 read_data8(uint32 addr);
	
	uint32 read_signal(int id); 
	void write_signal(int id, uint32 data, uint32 mask);
	void initialize(void);
	void reset(void);
	//void update_config(void);
	
	void set_context_int_line(DEVICE *p, int id, uint32 mask) {
		register_output_signal(&interrupt_line, p, id, mask);
	}
	void set_context_halt_line(DEVICE *p, int id, uint32 mask) {
		register_output_signal(&halt_line, p, id, mask);
	}
	void set_context_src(DEVICE *p, uint32 ch) {
		src[ch & 3]  = p;
	}
	void set_context_dst(DEVICE *p, uint32 ch) {
		dest[ch & 3]  = p;
	}
};	


#endif // _VM_FM77AV_16beta_ALU_H_
