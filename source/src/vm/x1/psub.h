/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2009.03.15-

	[ pseudo sub system ]
*/

#ifndef _PSUB_H_
#define _PSUB_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_PSUB_TAPE_REMOTE	0
#define SIG_PSUB_TAPE_END	1

class DATAREC;
class FIFO;

namespace X1 {

class PSUB : public DEVICE
{
private:
	DEVICE *d_cpu, *d_pio;
	DATAREC *d_drec;
	dll_cur_time_t cur_time;
	int time_register_id;
	
	uint8_t databuf[32][8], *datap;
	uint8_t mode, inbuf, outbuf;
	bool ibf, obf;
	int cmdlen, datalen;
	
	FIFO* key_buf;
	const uint8_t* key_stat;
	int key_prev, key_break;
	bool key_shift, key_ctrl, key_graph;
	bool key_caps_locked, key_kana_locked;
	int key_register_id;
	
	bool play, rec, eot;
	
	bool iei, intr;
	uint32_t intr_bit;

	void update_intr() override;
	void process_cmd();
	void __FASTCALL set_ibf(bool val);
	void __FASTCALL set_obf(bool val);
	uint8_t get_key_low();
	uint16_t __FASTCALL get_key(int code, bool repeat);
	
public:
	PSUB(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Pseudo Sub System"));
	}
	~PSUB() {}
	
	// common functions
	void initialize() override;
	void release() override;
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	void __FASTCALL event_callback(int event_id, int err) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// interrupt common functions
	void set_context_intr(DEVICE* device, uint32_t bit) override
	{
		d_cpu = device;
		intr_bit = bit;
	}
	void __FASTCALL set_intr_iei(bool val) override;
	uint32_t get_intr_ack() override;
	void notify_intr_reti() override;
	
	// unique functions
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_context_drec(DATAREC* device)
	{
		d_drec = device;
	}
	void key_down(int code, bool repeat);
	void key_up(int code);
	bool get_caps_locked()
	{
		return key_caps_locked;
	}
	bool get_kana_locked()
	{
		return key_kana_locked;
	}
	void play_tape(bool value);
	void rec_tape(bool value);
	void close_tape();
};

}
#endif

