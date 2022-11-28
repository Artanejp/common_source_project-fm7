/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2017.03.07-

	[ HD44102 ]
*/

#ifndef _HD44102_H_
#define _HD44102_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

//#define SIG_HD44102_CS2	0

class HD44102 : public DEVICE
{
private:
	uint8_t m_ram[4][50];            // display memory
	
	uint8_t m_status;                // status register
	uint8_t m_output;                // output register
	
//	int m_cs2;                      // chip select
	int m_page;                     // display start page
	int m_x;                        // X address
	int m_y;                        // Y address
	
//	int m_sx;
//	int m_sy;
	
	uint8_t status_r();
	void control_w(uint8_t data);
	uint8_t data_r();
	void data_w(uint8_t data);
	inline void count_up_or_down();
	
public:
	HD44102(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("HD44102 LCD Controller"));
	}
	~HD44102() {}
	
	// common functions
	void initialize();
	void reset();
//	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	uint8_t read(uint32_t offset);
	void write(uint32_t offset, uint8_t data);
	void screen_update(int m_sx, int m_sy, bool reverse);
};

#endif
