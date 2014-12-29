/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

	[ serial ]
*/

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SERIAL_RXRDY_KB	0
#define SIG_SERIAL_RXRDY_SUB	1
#define SIG_SERIAL_RXRDY_CH1	2
#define SIG_SERIAL_RXRDY_CH2	3
#define SIG_SERIAL_TXRDY_KB	4
#define SIG_SERIAL_TXRDY_SUB	5
#define SIG_SERIAL_TXRDY_CH1	6
#define SIG_SERIAL_TXRDY_CH2	7

class SERIAL : public DEVICE
{
private:
	DEVICE *d_pic, *d_kb, *d_sub, *d_ch1, *d_ch2;
	
	typedef struct {
		uint8 baud;
		uint8 ctrl;
		bool rxrdy;
		bool txrdy;
		uint8 intmask;
		uint8 intstat;
	} sioctrl_t;
	sioctrl_t sioctrl[4];
	
	void update_intr(int ch);
	
public:
	SERIAL(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SERIAL() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_sio(DEVICE* device_kb, DEVICE* device_sub, DEVICE* device_ch1, DEVICE* device_ch2)
	{
		d_kb = device_kb; d_sub = device_sub; d_ch1 = device_ch1; d_ch2 = device_ch2;
	}
};

#endif

