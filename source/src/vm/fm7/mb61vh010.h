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

enum {
	ALU_WRITE_PROXY = 0x00000,
	ALU_CMDREG = 0x10000,
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
	ALU_CMPDATA_REG,
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

class VM;
class EMU;
class MB61VH010: public DEVICE {
 protected:
	DEVICE *target;

	// Registers
	uint8_t command_reg;        // D410 (RW)
	uint8_t color_reg;          // D411 (RW)
	uint8_t mask_reg;           // D412 (RW)
	uint8_t cmp_status_reg;     // D413 (RO)
	uint8_t cmp_color_data[8]; // D413-D41A (WO)
	uint8_t bank_disable_reg;   // D41B (RW)
	uint8_t tile_reg[4];        // D41C-D41F (WO)
	uint8_t multi_page;
	uint32_t direct_access_offset;	
	pair32_t  line_addr_offset; // D420-D421 (WO)
	pair32_t  line_pattern;     // D422-D423 (WO)
	pair32_t  line_xbegin;      // D424-D425 (WO)
	pair32_t  line_ybegin;      // D426-D427 (WO)
	pair32_t  line_xend;        // D428-D429 (WO)
	pair32_t  line_yend;        // D42A-D42B (WO)
	
	bool busy_flag;
	int eventid_busy;

	uint32_t planes;
	uint32_t total_bytes;
	bool is_400line;
	uint32_t screen_width;
	uint32_t screen_height;
	uint32_t oldaddr;
	uint32_t alu_addr;
	pair32_t line_style;
	bool disable_flags[4];
	bool multi_flags[4];
	// ALU COMMANDS
	inline uint8_t do_read(uint32_t addr,  uint32_t bank);
	inline void do_write(uint32_t addr, uint32_t bank, uint8_t data);
	void do_pset(uint32_t addr);
	void do_blank(uint32_t addr);
	void do_or(uint32_t addr);
	void do_and(uint32_t addr);
	void do_xor(uint32_t addr);
	void do_not(uint32_t addr);
	void do_tilepaint(uint32_t addr);
	void do_compare(uint32_t addr);
	void do_alucmds(uint32_t addr);
	void do_alucmds_dmyread(uint32_t addr);
	inline void put_dot(int x, int y);
	inline void put_dot8(int x, int y);

	// LINE
	void do_line(void);
 public:
	MB61VH010(VM_TEMPLATE* parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		target = NULL;
		direct_access_offset = 0;
		set_device_name(_T("MB61VH010 ALU"));
	}
	~MB61VH010() {}

	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);
	bool decl_state(FILEIO *state_fio, bool loading);
	
	void event_callback(int event_id, int err);
	void write_data8(uint32_t id, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	uint32_t read_signal(int id); 
	void write_signal(int id, uint32_t data, uint32_t mask);
	void initialize(void);
	void reset(void);
	//void update_config(void);
	
	void set_context_memory(DEVICE *p)
	{
		target = p;
	}
	void set_direct_access_offset(uint32_t offset)
	{
		direct_access_offset = offset;	
	}
};	

inline void MB61VH010::put_dot(int x, int y)
{
	const uint8_t vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	uint8_t tmp8a;
	uint8_t mask8;
	
	//bool updated;
   
	if((command_reg & 0x80) == 0) return; // Not compare.
	if((x < 0) || (y < 0)) {
		return; // Lower address
	}
   
	//if(y >= (int)screen_height) return; // Workaround of overflow
	
	alu_addr = (y * screen_width + x)  >> 3;
	alu_addr = alu_addr + line_addr_offset.w.l;
	if(!is_400line) {
		alu_addr = alu_addr & 0x3fff;
	} else {
		alu_addr = alu_addr & 0x7fff;
	}
	
	mask8 = ~vmask[x & 7];
	//updated = false;
	tmp8a = line_style.b.h & 0x80;
	
  	if(oldaddr != alu_addr) {
		if(oldaddr == 0xffffffff) {
			if(tmp8a != 0) {
				mask_reg &= mask8;
			}
			oldaddr = alu_addr;
		}
		do_alucmds(oldaddr);
		mask_reg = 0xff;
		oldaddr = alu_addr;
		//updated = true;
	}
	line_style.w.l <<= 1;
	if(tmp8a != 0) {
	  	mask_reg &= mask8;
		line_style.w.l |= 0x01;
	}
	return;
}

inline void MB61VH010::put_dot8(int x, int y)
{
	const uint8_t vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	uint8_t tmp8a;
	//bool updated;
   
	if((command_reg & 0x80) == 0) return; // Not compare.
	if((x < 0) || (y < 0)) {
		return; // Lower address
	}
   
	//if(y >= (int)screen_height) return; // Workaround of overflow
	
	alu_addr = (y * screen_width + x)  >> 3;
	alu_addr = alu_addr + line_addr_offset.w.l;
	if(!is_400line) {
		alu_addr = alu_addr & 0x3fff;
	} else {
		alu_addr = alu_addr & 0x7fff;
	}
	//updated = false;
	if(oldaddr != alu_addr) {
		if(oldaddr == 0xffffffff) {
			if((line_style.b.h & 0x80) != 0) {
				mask_reg &= ~vmask[x & 7];
			}
			oldaddr = alu_addr;
		}
		do_alucmds(oldaddr);
		mask_reg = 0xff;
		oldaddr = alu_addr;
		//updated = true;
	}
	tmp8a = line_style.b.h;
	mask_reg = mask_reg & ~tmp8a;
	tmp8a = line_style.b.l;
	line_style.b.l = line_style.b.h;
	line_style.b.h = tmp8a;
	return;
}

inline uint8_t MB61VH010::do_read(uint32_t addr, uint32_t bank)
{
	uint32_t raddr;
	
	//if(((1 << bank) & multi_page) != 0) return 0xff;
	if(multi_flags[bank]) return 0xff;
	if(is_400line) {
		if((addr & 0xffff) < 0x8000) {
			raddr = (addr  & 0x7fff) | (0x8000 * bank);
			return target->read_data8(raddr + direct_access_offset);
		}
	} else {
		raddr = (addr & 0x3fff) | (0x4000 * bank);
		return target->read_data8(raddr + direct_access_offset);
	}
	return 0xff;
}

inline void MB61VH010::do_write(uint32_t addr, uint32_t bank, uint8_t data)
{
	uint32_t raddr;
	uint8_t readdata;

	//if(((1 << bank) & multi_page) != 0) return;
	if(multi_flags[bank]) return;
	if((command_reg & 0x40) != 0) { // Calculate before writing
	  	readdata = do_read(addr, bank);
		//readdata = data;
		if((command_reg & 0x20) != 0) { // NAND
			readdata = readdata & cmp_status_reg;
			data = data & (~cmp_status_reg);
		} else { // AND
			readdata = readdata & (~cmp_status_reg);
			data = data & cmp_status_reg;
		}
		readdata = readdata | data;
	} else {
		readdata = data;
	}
	if(is_400line) {
		if((addr & 0xffff) < 0x8000) {
			raddr = (addr & 0x7fff) | (0x8000 * bank);
			target->write_data8(raddr + direct_access_offset, readdata);
		}
	} else {
		raddr = (addr & 0x3fff) | (0x4000 * bank);
		target->write_data8(raddr + direct_access_offset, readdata);
	}
	return;
}
#endif // _VM_FM77AV_16beta_ALU_H_
