/*
 * FM77AV/FM16β ALU [77av_alu.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Mar 28, 2015 : Initial
 *
 */

#ifndef _VM_FM77AV_16beta_ALU_H_
#define _VM_FM77AV_16beta_ALU_H_


#include "../device.h"
#include "../memory.h"
#include "fm7_common.h"

class FMALU: public DEVICE {

 protected:
	EMU *p_emu;
	VM *p_vm;
	MEMORY *target;
	
	// Registers
	uint8 command_reg;        // D410 (RW)
	uint8 color_reg;          // D411 (RW)
	uint8 mask_reg;           // D412 (RW)
	uint8 cmp_status_reg;     // D413 (RO)
	uint8 cmp_status_data[8]; // D413-D41A (WO)
	uint8 bank_disable_reg;   // D41B (RW)
	uint8 tile_reg[4];        // D41C-D41F (WO)
	
	pair  line_addr_offset; // D420-D421 (WO)
	pair  line_pattern;     // D422-D423 (WO)
	pair  line_xbegin;      // D424-D425 (WO)
	pair  line_ybegin;      // D426-D427 (WO)
	pair  line_xend;        // D428-D429 (WO)
	pair  line_yend;        // D42A-D42B (WO)
	
	bool  disable_bit;
	bool busy_flag;
	// ALU COMMANDS
	void read_common(uint32 addr,  uint8 data[], uint32 planes);
	void write_common(uint32 addr, uint8 data[], uint32 planes);
	void do_pset(uint32 addr);
	void do_black(uint32 addr);
	void do_or(uint32 addr);
	void do_and(uint32 addr);
	void do_xor(uint32 addr);
	void do_not(uint32 addr);
	void do_tilepaint(uint32 addr);
	void do_compare(uint32 addr);
	// LINE
	void do_line(void);
 public:
	FMALU(VO *parent_vm, EMU *parent_emu);
	~FMALU();

	void event_callback(int event_id, int err);
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_signal(int id); 
	uint32 read_data8(uint32 addr);
	void write_data8(uint32 addr, uint32 data);
	void initialize(void);
	void reset(void);
	void update_config(void);
	
	void set_context_memory(MEMORY *p)
	{
		target = p;
	}
};	


#endif // _VM_FM77AV_16beta_ALU_H_
