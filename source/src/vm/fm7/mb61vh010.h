/*
 * FM77AV/FM16β ALU [mb61vh010.h]
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

enum {
	ALU_CMDREG = 0,
	ALU_LOGICAL_COLOR,
	ALU_WRITE_MASKREG,
	ALU_CMP_STATUS_REG,
	ALU_BANK_DISABLE,
	ALU_TILEPAINT_B,
	ALU_TILEPAINT_R,
	ALU_TILEPAINT_G,
	ALU_TILEPAINT_L,
	ALU_OFFSET_REG_HIGH,
	ALU_OFFSET_REG_LO,
	ALU_LINEPATTERN_REG_HIGH,
	ALU_LINEPATTERN_REG_LO,
	ALU_LINEPOS_START_X_HIGH,
	ALU_LINEPOS_START_X_LOW,  
	ALU_LINEPOS_START_Y_HIGH,
	ALU_LINEPOS_START_Y_LOW,  
	ALU_LINEPOS_END_X_HIGH,
	ALU_LINEPOS_END_X_LOW,  
	ALU_LINEPOS_END_Y_HIGH,
	ALU_LINEPOS_END_Y_LOW,
	ALU_CMPDATA_REG = 0x10000,
	ALU_WRITE_PROXY = 0x20000,
};

enum {
	SIG_ALU_BUSYSTAT = 1,
	SIG_ALU_400LINE,
	SIG_ALU_MULTIPAGE,
	SIG_ALU_PLANES,
	SIG_ALU_X_WIDTH,
	SIG_ALU_Y_HEIGHT,
	SIG_ALU_ENABLED,
};

enum {
	EVENT_MB61VH010_BUSY_ON = 0,
	EVENT_MB61VH010_BUSY_OFF
};

class MB61VH010: public DEVICE {

 protected:
	EMU *p_emu;
	VM *p_vm;
	DEVICE *target;
	
	// Registers
	uint8 command_reg;        // D410 (RW)
	uint8 color_reg;          // D411 (RW)
	uint8 mask_reg;           // D412 (RW)
	uint8 cmp_status_reg;     // D413 (RO)
	uint8 cmp_color_data[8]; // D413-D41A (WO)
	uint8 bank_disable_reg;   // D41B (RW)
	uint8 tile_reg[4];        // D41C-D41F (WO)
	uint8 multi_page;
	
	pair  line_addr_offset; // D420-D421 (WO)
	pair  line_pattern;     // D422-D423 (WO)
	pair  line_xbegin;      // D424-D425 (WO)
	pair  line_ybegin;      // D426-D427 (WO)
	pair  line_xend;        // D428-D429 (WO)
	pair  line_yend;        // D42A-D42B (WO)
	
	bool busy_flag;
	int eventid_busy;

	uint32 planes;
	uint32 total_bytes;
	bool is_400line;
	uint32 screen_width;
	uint32 screen_height;
	uint32 oldaddr;
	uint32 alu_addr;
	pair line_style;
	
	// ALU COMMANDS
	uint8 do_read(uint32 addr,  uint32 bank);
	uint8 do_write(uint32 addr, uint32 bank, uint8 data);
	uint8 do_pset(uint32 addr);
	uint8 do_blank(uint32 addr);
	uint8 do_or(uint32 addr);
	uint8 do_and(uint32 addr);
	uint8 do_xor(uint32 addr);
	uint8 do_not(uint32 addr);
	uint8 do_tilepaint(uint32 addr);
	uint8 do_compare(uint32 addr);
	uint8 do_alucmds(uint32 addr);
	void do_alucmds_dmyread(uint32 addr);
	bool put_dot(int x, int y);

	// LINE
	void do_line(void);
 public:
	MB61VH010(VM *parent_vm, EMU *parent_emu);
	~MB61VH010();

	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);
	const _TCHAR *get_device_name(void)
	{
		return "MB61VH010_ALU";
	}
   
	void event_callback(int event_id, int err);
	void write_data8(uint32 id, uint32 data);
	uint32 read_data8(uint32 addr);
	uint32 read_signal(int id); 
	void write_signal(int id, uint32 data, uint32 mask);
	void initialize(void);
	void reset(void);
	//void update_config(void);
	
	void set_context_memory(DEVICE *p)
	{
		target = p;
	}
};	


#endif // _VM_FM77AV_16beta_ALU_H_
