/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

	[ serial ]
*/

#include "serial.h"
#include "../i8251.h"
#include "../i8259.h"

void SERIAL::initialize()
{
	memset(sioctrl, sizeof(sioctrl), 1);
}

void SERIAL::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0x0a:
		sioctrl[0].baud = data;
		break;
	case 0x0b:
		sioctrl[0].ctrl = data;
		d_kb->write_signal(SIG_I8251_LOOPBACK, data, 8);
		update_intr(0);
		break;
	case 0x12:
		sioctrl[1].baud = data;
		break;
	case 0x13:
		sioctrl[1].ctrl = data;
		d_sub->write_signal(SIG_I8251_LOOPBACK, data, 8);
		update_intr(1);
		break;
	case 0x62:
		sioctrl[2].baud = data;
		break;
	case 0x63:
		sioctrl[2].ctrl = data;
		d_ch1->write_signal(SIG_I8251_LOOPBACK, data, 8);
		update_intr(2);
		break;
	case 0x64:
		sioctrl[2].intmask = data;
		break;
	case 0x65:
		sioctrl[2].intstat &= ~data;
		break;
	case 0x72:
		sioctrl[3].baud = data;
		break;
	case 0x73:
		sioctrl[3].ctrl = data;
		d_ch2->write_signal(SIG_I8251_LOOPBACK, data, 8);
		update_intr(3);
		break;
	case 0x74:
		sioctrl[3].intmask = data;
		break;
	case 0x75:
		sioctrl[3].intstat &= ~data;
		break;
	}
}

uint32_t SERIAL::read_io8(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0x0a:
		return sioctrl[0].baud;
	case 0x0b:
		return sioctrl[0].ctrl;
	case 0x12:
		return sioctrl[1].baud;
	case 0x13:
		return sioctrl[1].ctrl;
	case 0x62:
		return sioctrl[2].baud;
	case 0x63:
		return sioctrl[2].ctrl;
	case 0x64:
		return sioctrl[2].intmask;
	case 0x65:
		return sioctrl[2].intstat;
	case 0x66:
		return 0xf0;	// append register
	case 0x72:
		return sioctrl[3].baud;
	case 0x73:
		return sioctrl[3].ctrl;
	case 0x74:
		return sioctrl[3].intmask;
	case 0x75:
		return sioctrl[3].intstat;
	case 0x76:
		return 0xf0;	// append register
	}
	return 0xff;
}

void SERIAL::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SERIAL_RXRDY_KB:
	case SIG_SERIAL_RXRDY_SUB:
	case SIG_SERIAL_RXRDY_CH1:
	case SIG_SERIAL_RXRDY_CH2:
		sioctrl[id & 3].rxrdy = ((data & mask) != 0);
		update_intr(id & 3);
		break;
	case SIG_SERIAL_TXRDY_KB:
	case SIG_SERIAL_TXRDY_SUB:
	case SIG_SERIAL_TXRDY_CH1:
	case SIG_SERIAL_TXRDY_CH2:
		sioctrl[id & 3].txrdy = ((data & mask) != 0);
		update_intr(id & 3);
		break;
	}
}

void SERIAL::update_intr(int ch)
{
	static const int pic_ids[4] = {
		SIG_I8259_CHIP0 | SIG_I8259_IR2,	// keyboard
		SIG_I8259_CHIP0 | SIG_I8259_IR3,	// sub
		SIG_I8259_CHIP0 | SIG_I8259_IR4,	// rs-232c ch.1
		SIG_I8259_CHIP1 | SIG_I8259_IR4		// rs-232c ch.2
	};
	
	if((sioctrl[ch].rxrdy && (sioctrl[ch].ctrl & 0x40)) || (sioctrl[ch].txrdy && (sioctrl[ch].ctrl & 0x20))) {
		d_pic->write_signal(pic_ids[ch], 1, 1);
		sioctrl[ch].intstat |=  4;
	} else {
		d_pic->write_signal(pic_ids[ch], 0, 1);
		sioctrl[ch].intstat &=  ~4;
	}
}

#define STATE_VERSION	1

#include "../../statesub.h"

void SERIAL::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_UINT8_STRIDE((sioctrl[0].baud),    4, sizeof(sioctrl[0]));
	DECL_STATE_ENTRY_UINT8_STRIDE((sioctrl[0].ctrl),    4, sizeof(sioctrl[0]));
	DECL_STATE_ENTRY_BOOL_STRIDE((sioctrl[0].rxrdy),    4, sizeof(sioctrl[0]));
	DECL_STATE_ENTRY_BOOL_STRIDE((sioctrl[0].txrdy),    4, sizeof(sioctrl[0]));
	DECL_STATE_ENTRY_UINT8_STRIDE((sioctrl[0].intmask), 4, sizeof(sioctrl[0]));
	DECL_STATE_ENTRY_UINT8_STRIDE((sioctrl[0].intstat), 4, sizeof(sioctrl[0]));

	leave_decl_state();
}	

void SERIAL::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}

//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->Fwrite(sioctrl, sizeof(sioctrl), 1);
}

bool SERIAL::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		return false;
	}

//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	state_fio->Fread(sioctrl, sizeof(sioctrl), 1);
	return true;
}

