/*
	TOSHIBA T-200/250 Emulator 'eT-250'

	Author : Takeda.Toshiya
	Date   : 2023.12.29-

	[ memory bus ]
*/

#ifndef _MEMBUS_H_
#define _MEMBUS_H_

#include "../memory.h"

#define SIG_MEMBUS_KEY_SEL	0
#define SIG_MEMBUS_KEY_IRQ	1
#define SIG_MEMBUS_PRN_ACK	2
#define SIG_MEMBUS_PRN_BUSY	3
#define SIG_MEMBUS_SIO_TXR	4
#define SIG_MEMBUS_SIO_RXR	5
#define SIG_MEMBUS_SIO_CI	6
#define SIG_MEMBUS_SIO_CTS	7

class I8080;

class MEMBUS : public MEMORY
{
private:
	DEVICE* d_beep;
	I8080* d_cpu;
	DEVICE* d_fdc;
	DEVICE* d_kbc;
	DEVICE* d_prn;
	
	uint8_t ram[0x10000];
	uint8_t rom[0x1000];
	uint8_t vpg[0x800];
	
	uint8_t mode_reg;
	uint8_t data_reg;
	uint8_t cmd_reg;
	uint8_t fdd_reg;
	
	bool key_repeat, key_ctrl, key_shift, key_irq;
	bool prn_ack, prn_busy;
	bool sio_txr, sio_rxr, sio_ci, sio_cts;
	bool stepcnt;
	
	uint8_t *regs;
	int cblink;
	int sync_count;
	scrntype_t screen[200][640];
	
	void update_irq();
	void update_beep();
	void update_rom_bank();
	void update_vpg_bank();
	
public:
	MEMBUS(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMBUS() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_beep(DEVICE* device)
	{
		d_beep = device;
	}
	void set_context_cpu(I8080* device)
	{
		d_cpu = device;
	}
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
	void set_context_kbc(DEVICE* device)
	{
		d_kbc = device;
	}
	void set_context_prn(DEVICE* device)
	{
		d_prn = device;
	}
	void set_regs_ptr(uint8_t* ptr)
	{
		regs = ptr;
	}
	void key_down(int code);
	void key_up(int code);
	void draw_screen();
};

#endif
