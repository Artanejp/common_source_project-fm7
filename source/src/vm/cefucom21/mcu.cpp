/*
	Hino Electronics CEFUCOM-21 Emulator 'eCEFUCOM-21'

	Author : Takeda.Toshiya
	Date   : 2019.03.28-

	[ MCU02 (main control unit ???) ]
*/

#include "mcu.h"
#include "../ay_3_891x.h"
#include "../datarec.h"
#include "../mc6847.h"

void MCU::initialize()
{
	system_port = 0;
	
	// register event to update the key status
	register_frame_event(this);
}

void MCU::reset()
{
	memset(key_status, 0, sizeof(key_status));
	d_vdp->write_signal(SIG_MC6847_INTEXT, 1, 1);
}

void MCU::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x40:
		d_drec->write_signal(SIG_DATAREC_MIC, data, 0x01);
		d_drec->write_signal(SIG_DATAREC_REMOTE, ~data, 0x02);
		// bit2 : kana lock led ???
		// bit3 : printer strobe
		d_vdp->write_signal(SIG_MC6847_GM, (data & 0x20) ? 7 : 6, 7);
		d_vdp->write_signal(SIG_MC6847_CSS, data, 0x40);
		d_vdp->write_signal(SIG_MC6847_AG, data, 0x80);
		break;
	}
}

uint32_t MCU::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0x40:
		return system_port;
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87:
	case 0x88:
	case 0x89:
		return ~key_status[addr & 0x0f];
	}
	return 0xff;
}

void MCU::event_frame()
{
	const uint8_t  *key_stat = emu->get_key_buffer();
	const uint32_t *joy_stat = emu->get_joy_buffer();
	
	for(int i = 0; i < 10; i++) {
		uint8_t val = 0;
		for(int j = 0; j < 8; j++) {
			val |= key_stat[key_map[i][j]] ? (1 << j) : 0;
		}
		key_status[i] = val;
	}
	d_psg->write_signal(SIG_AY_3_891X_PORT_A, ~(joy_stat[0] & 0x1f), 0xff);
	d_psg->write_signal(SIG_AY_3_891X_PORT_B, ~(joy_stat[1] & 0x1f), 0xff);
}

void MCU::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MCU_SYSPORT) {
		system_port = (system_port & ~mask) | (system_port & mask);
	}
}

#define STATE_VERSION	1

bool MCU::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(key_status, sizeof(key_status), 1);
	state_fio->StateValue(system_port);
	return true;
}

