/*
	Skelton for retropc emulator

	Origin : MAME 0.227
	Author : Takeda.Toshiya
	Date   : 2021.01.21-

	[ i8279 ]
*/

// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Intel 8279 Programmable Keyboard/Display Interface emulation

****************************************************************************
                            _____   _____
                   RL2   1 |*    \_/     | 40  Vcc
                   RL3   2 |             | 39  RL1
                   CLK   3 |             | 38  RL0
                   IRQ   4 |             | 37  CNTL/STB
                   RL4   5 |             | 36  SHIFT
                   RL5   6 |             | 35  SL3
                   RL6   7 |             | 34  SL2
                   RL7   8 |             | 33  SL1
                 RESET   9 |             | 32  SL0
                   /RD  10 |     8279    | 31  OUT B0
                   /WR  11 |             | 30  OUT B1
                   DB0  12 |             | 29  OUT B2
                   DB1  13 |             | 28  OUT B3
                   DB2  14 |             | 27  OUT A0
                   DB3  15 |             | 26  OUT A1
                   DB4  16 |             | 25  OUT A2
                   DB5  17 |             | 24  OUT A3
                   DB6  18 |             | 23  /BD
                   DB7  19 |             | 22  /CS
                   Vss  20 |_____________| 21  A0 (CTRL/DATA)


***************************************************************************/

#ifndef _I8279_H_
#define _I8279_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I8279_RL	0
#define SIG_I8279_SHIFT	1
#define SIG_I8279_CTRL	2

class I8279 :  public DEVICE
{
private:
	// read & write handlers
//	uint8_t read(offs_t offset);
	uint8_t status_r();
	uint8_t data_r();
//	void write(offs_t offset, uint8_t data);
	void cmd_w(uint8_t data);
	void data_w(uint8_t data);
	void timer_mainloop();

	void timer_adjust();
	void clear_display();
	void new_fifo(uint8_t data);
	void set_irq(bool state);

	outputs_t           m_out_irq_cb;       // IRQ
	outputs_t           m_out_sl_cb;        // Scanlines SL0-3
	outputs_t           m_out_disp_cb;      // Display outputs B0-3, A0-3
	outputs_t           m_out_bd_cb;        // BD
	uint8_t             m_in_rl_cb;         // kbd readlines RL0-7
	bool                m_in_shift_cb;      // Shift key
	bool                m_in_ctrl_cb;       // Ctrl-Strobe line

//	emu_timer *m_timer;
	int m_timer;

	uint8_t m_d_ram[16];     // display ram
	uint8_t m_d_ram_ptr;
	uint8_t m_s_ram[8];      // might be same as fifo ram
	uint8_t m_s_ram_ptr;
	uint8_t m_fifo[8];       // queued keystrokes
	uint8_t m_cmd[8];        // Device settings
	uint8_t m_status;        // Returned via status_r
	uint32_t m_scanclock;    // Internal scan clock
	uint8_t m_scanner;       // next output on SL lines

	bool m_autoinc;     // auto-increment flag
	bool m_read_flag;   // read from where
	bool m_ctrl_key;    // previous state of strobe input
	bool m_se_mode;     // special error mode flag
	uint8_t m_key_down;      // current key being debounced
	uint8_t m_debounce;      // debounce counter

public:
	// construction/destruction
	I8279(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&m_out_irq_cb);
		initialize_output_signals(&m_out_sl_cb);
		initialize_output_signals(&m_out_disp_cb);
		initialize_output_signals(&m_out_bd_cb);
		set_device_name(_T("8279 KDC"));
	}
	~I8279() {}
	
	// common functions
	void initialize();
//	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
//	uint32_t read_signal(int ch);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&m_out_irq_cb, device, id, mask);
	}
	void set_context_sl(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&m_out_sl_cb, device, id, mask);
	}
	void set_context_disp(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&m_out_disp_cb, device, id, mask);
	}
	void set_context_bd(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&m_out_bd_cb, device, id, mask);
	}
};

#endif
