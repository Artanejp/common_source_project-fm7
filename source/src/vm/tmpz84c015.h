/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2022.07.02-

	[ TMPZ84C015 ]
*/

#pragma once

#include "./tmpz84c013.h"

#include "./z80pio.h"

#define SIG_TMPZ84C015_PIO_PORT_A	30
#define SIG_TMPZ84C015_PIO_PORT_B	31
#define SIG_TMPZ84C015_PIO_STROBE_A	32
#define SIG_TMPZ84C015_PIO_STROBE_B	33

class TMPZ84C015 : public TMPZ84C013
{
protected:
	Z80PIO *d_pio;
	virtual void __FASTCALL update_priority(uint8_t val) override;
public:
	TMPZ84C015(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : TMPZ84C013(parent_vm, parent_emu)
	{
		d_child = d_1st = NULL;
		set_device_name(_T("TMPZ84C015"));
	}
	~TMPZ84C015() {}
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	
	// interrupt common functions
	void set_context_intr(DEVICE* device, uint32_t bit) override
	{
		TMPZ84C013::set_context_intr(device, bit);
		d_pio->set_context_intr(device, bit + 2);
	}
	// unique functions
	void set_context_pio(Z80PIO* device)
	{
		d_pio = device;
		d_pio->set_device_name(_T("TMPZ84C015 PIO"));
	}
	void set_iomap(IO* io) override
	{
		TMPZ84C013::set_iomap(io);
		io->set_iomap_range_rw(0x1c, 0x1f, d_pio);
	}

	// unique functions (from Z80PIO)
	void set_context_pio_port_a(DEVICE* device, int id, uint32_t mask, int shift)
	{
		d_pio->set_context_port_a(device, id, mask, shift);
	}
	void set_context_pio_port_b(DEVICE* device, int id, uint32_t mask, int shift)
	{
		d_pio->set_context_port_b(device, id, mask, shift);
	}
	void set_context_pio_ready_a(DEVICE* device, int id, uint32_t mask)
	{
		d_pio->set_context_ready_a(device, id, mask);
	}
	void set_context_pio_ready_b(DEVICE* device, int id, uint32_t mask)
	{
		d_pio->set_context_ready_b(device, id, mask);
	}
	void set_pio_hand_shake(int ch, bool value)
	{
		d_pio->set_hand_shake(ch, value);
	}

};
