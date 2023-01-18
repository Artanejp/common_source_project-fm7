/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2022.07.02-

	[ TMPZ84C015/TMP84C013 ]
*/

#pragma once

#include "./vm_template.h"
#include "../emu.h"
#include "./device.h"

#include "./io.h"
#include "./z80ctc.h"
#include "./z80sio.h"

#define SIG_TMPZ84C015_CTC_TRIG_0	0
#define SIG_TMPZ84C015_CTC_TRIG_1	1
#define SIG_TMPZ84C015_CTC_TRIG_2	2
#define SIG_TMPZ84C015_CTC_TRIG_3	3

#define SIG_TMPZ84C015_SIO_RECV_CH0	10
#define SIG_TMPZ84C015_SIO_RECV_CH1	11
#define SIG_TMPZ84C015_SIO_BREAK_CH0	12
#define SIG_TMPZ84C015_SIO_BREAK_CH1	13
#define SIG_TMPZ84C015_SIO_DCD_CH0	14
#define SIG_TMPZ84C015_SIO_DCD_CH1	15
#define SIG_TMPZ84C015_SIO_CTS_CH0	16
#define SIG_TMPZ84C015_SIO_CTS_CH1	17
#define SIG_TMPZ84C015_SIO_SYNC_CH0	18
#define SIG_TMPZ84C015_SIO_SYNC_CH1	19
#define SIG_TMPZ84C015_SIO_TX_CLK_CH0	20
#define SIG_TMPZ84C015_SIO_TX_CLK_CH1	21
#define SIG_TMPZ84C015_SIO_RX_CLK_CH0	22
#define SIG_TMPZ84C015_SIO_RX_CLK_CH1	23
// hack: clear recv buffer
#define SIG_TMPZ84C015_SIO_CLEAR_CH0	24
#define SIG_TMPZ84C015_SIO_CLEAR_CH1	25

class TMPZ84C013 : public DEVICE
{
protected:
	Z80CTC *d_ctc;
	Z80SIO *d_sio;
	
	// daisy chain
	DEVICE *d_child;
	DEVICE *d_1st;
	bool iei;
	uint8_t priority;
	
	virtual void __FASTCALL update_priority(uint8_t val);
	
public:
	TMPZ84C013(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_child = d_1st = NULL;
		set_device_name(_T("TMPZ84C013"));
	}
	~TMPZ84C013() {}
	
	// common functions
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	virtual bool process_state(FILEIO* state_fio, bool loading) override;
	
	// interrupt common functions
	virtual void set_context_intr(DEVICE* device, uint32_t bit) override
	{
		d_ctc->set_context_intr(device, bit + 0);
		d_sio->set_context_intr(device, bit + 1);
	}
	void set_context_child(DEVICE* device) override
	{
		d_child = device;
	}
	DEVICE *get_context_child() override
	{
		return d_child;
	}
	void set_intr_iei(bool val) override
	{
		if(d_1st) {
			d_1st->set_intr_iei(val);
		}
		iei = val;
	}
	uint32_t get_intr_ack() override
	{
		if(d_1st) {
			return d_1st->get_intr_ack();
		}
		return 0xff;
	}
	void notify_intr_reti() override
	{
		if(d_1st) {
			d_1st->notify_intr_reti();
		}
	}
	
	// unique functions
	void set_context_ctc(Z80CTC* device)
	{
		d_ctc = device;
		d_ctc->set_device_name(_T("TMPZ84C015 CTC"));
	}
	void set_context_sio(Z80SIO* device)
	{
		d_sio = device;
		d_sio->set_device_name(_T("TMPZ84C015 SIO"));
	}
	virtual void set_iomap(IO* io)
	{
		io->set_iomap_range_rw(0x10, 0x13, d_ctc);
		io->set_iomap_range_rw(0x18, 0x1b, d_sio);
		io->set_iomap_single_w (0xf0, this);
		io->set_iomap_single_w (0xf1, this);
		io->set_iomap_single_rw(0xf4, this);
	}
	
	// unique functions (from Z80CTC)
	void set_context_ctc_zc0(DEVICE* device, int id, uint32_t mask)
	{
		d_ctc->set_context_zc0(device, id, mask);
	}
	void set_context_ctc_zc1(DEVICE* device, int id, uint32_t mask)
	{
		d_ctc->set_context_zc1(device, id, mask);
	}
	void set_context_ctc_zc2(DEVICE* device, int id, uint32_t mask)
	{
		d_ctc->set_context_zc2(device, id, mask);
	}
	void set_context_ctc_zc3(DEVICE* device, int id, uint32_t mask)
	{
		d_ctc->set_context_zc3(device, id, mask);
	}
	void set_ctc_constant_clock(int ch, uint32_t hz)
	{
		d_ctc->set_constant_clock(ch, hz);
	}
	
	// unique functions (from Z80SIO)
	void set_context_sio_rts(int ch, DEVICE* device, int id, uint32_t mask)
	{
		d_sio->set_context_rts(ch, device, id, mask);
	}
	void set_context_sio_dtr(int ch, DEVICE* device, int id, uint32_t mask)
	{
		d_sio->set_context_dtr(ch, device, id, mask);
	}
	void set_context_sio_send(int ch, DEVICE* device, int id)
	{
		d_sio->set_context_send(ch, device, id);
	}
	void set_context_sio_sync(int ch, DEVICE* device, int id, uint32_t mask)
	{
		d_sio->set_context_sync(ch, device, id, mask);
	}
	void set_context_sio_break(int ch, DEVICE* device, int id, uint32_t mask)
	{
		d_sio->set_context_break(ch, device, id, mask);
	}
	void set_context_sio_rxdone(int ch, DEVICE* device, int id, uint32_t mask)
	{
		d_sio->set_context_rxdone(ch, device, id, mask);
	}
	void set_context_sio_txdone(int ch, DEVICE* device, int id, uint32_t mask)
	{
		d_sio->set_context_txdone(ch, device, id, mask);
	}
	void set_sio_tx_clock(int ch, double clock)
	{
		d_sio->set_tx_clock(ch, clock);
	}
	void set_sio_rx_clock(int ch, double clock)
	{
		d_sio->set_rx_clock(ch, clock);
	}
	
};


