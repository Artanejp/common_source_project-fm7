/*
 * DMAC HD6844/MC6844 [hd6844.h]
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
	HD6844_EVENT_END_TRANSFER = 8,
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
	HD6844_ACK_DRQ1,
	HD6844_ACK_DRQ2,

};

class HD6844: public DEVICE {

 protected:
	EMU *p_emu;
	VM *p_vm;
	
 private:
	DEVICE *src[4];
	DEVICE *dest[4];

	outputs_t interrupt_line[4];
	outputs_t drq_line[2];
	// Registers

	uint32_t addr_reg[4];
	uint16_t words_reg[4];
	uint8_t channel_control[4];
	
	uint8_t priority_reg;
	uint8_t interrupt_reg;
	uint8_t datachain_reg;
	uint8_t num_reg;
	uint32_t addr_offset;
	
	bool transfering[4];
	bool first_transfer[4];
	bool cycle_steal[4];
	bool halt_flag[4];
   
	uint32_t fixed_addr[4];
	uint8_t data_reg[4];
	int event_dmac[4];

	void do_transfer(int ch);
 public:
	HD6844(VM *parent_vm, EMU *parent_emu);
	~HD6844();

	void event_callback(int event_id, int err);
	void write_data8(uint32_t id, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	
	uint32_t read_signal(int id); 
	void write_signal(int id, uint32_t data, uint32_t mask);
	void initialize(void);
	void reset(void);
	//void update_config(void);
	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("HD6844_DMAC");
	}
	
	void set_context_int_line(DEVICE *p, int ch, int id, uint32_t mask) {
		register_output_signal(&interrupt_line[ch & 3], p, id, mask);
	}
	void set_context_drq_line(DEVICE *p, int ch, int id, uint32_t mask) {
		register_output_signal(&drq_line[ch & 1], p, id, mask);
	}
	void set_context_src(DEVICE *p, uint32_t ch) {
		src[ch & 3]  = p;
	}
	void set_context_dst(DEVICE *p, uint32_t ch) {
		dest[ch & 3]  = p;
	}
};	


#endif // _VM_FM77AV_16beta_ALU_H_
