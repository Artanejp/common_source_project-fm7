/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.01.28 -
    History : 2020.01.28 Initial from FM7.
	[ Towns Joystick ports]

*/
#include "../debugger.h"
#include "./joystick.h"
#include "./js_template.h"
#include "./towns_common.h"

namespace FMTOWNS {
	
void JOYSTICK::reset()
{
	update_config(); // Update MOUSE PORT.
	force_update = true;
	write_port(0x00);
}

void JOYSTICK::initialize()
{
	for(int i = 0; i < 2; i++) {
		reset_input_data(i);
	}
	reg_val = 0x00;
	force_update = true;
	data_mask[0] = 0x00;
	data_mask[1] = 0x00;
	
	if(d_debugger != NULL) {
		d_debugger->set_device_name(_T("Debugger (JOYSTICK PORT)"));
		d_debugger->set_context_mem(this);
		d_debugger->set_context_io(vm->dummy);
	}
}

void JOYSTICK::release()
{
}

void JOYSTICK::reset_input_data(int num)
{
	if((num >= 0) && (num < 2)) {
		stat_com[num] = true; // Wire disconnected.
		data_reg[num] = 0xff;
	}
}
void JOYSTICK::make_mask(int num, uint8_t data)
{
	uint8_t _com = data;
	uint8_t _trig = data;
	if(num != 0) {
		_com  >>= 1;
		_trig >>= 2;
	}
	data_mask[num] = ((_com & 0x10) << 2) | ((_trig & 3) << 4) | 0x8f;
}

void JOYSTICK::write_data_to_port(int num, JSDEV_TEMPLATE *target_dev, uint8_t data)
{
	JSDEV_TEMPLATE *p = target_dev;
	uint8_t com_mask = (num == 0) ? 0x10 : 0x20;
	uint8_t triga_mask = (num == 0) ? 0x01 : 0x04;
	uint8_t trigb_mask = (num == 0) ? 0x02 : 0x08;
	if(p != nullptr) {
		uint32_t e_num = num << 16;
		p->write_signal(e_num | SIG_JS_COM, data, com_mask);
		p->write_signal(e_num | SIG_JS_TRIG_A, data, triga_mask);
		p->write_signal(e_num | SIG_JS_TRIG_B, data, trigb_mask);
		bool _stat;
		p->query(_stat);
	}
}	

	
void JOYSTICK::write_port(uint8_t data)
{
	std::unique_lock<std::mutex> _l = lock_device();
	
	reg_val = data;
	for(int num = 0; num < 2; num++) {
		make_mask(num, data);
		if((port_using[num] >= 0) && (port_using[num] < port_count[num])) {
			JSDEV_TEMPLATE *p = d_port[num][port_using[num]];
			if(p != nullptr) {
				write_data_to_port(num, p, reg_val);
			}
		}
	}
}

	
void JOYSTICK::write_io8(uint32_t address, uint32_t data)
{
	// ToDo: Mouse
	__LIKELY_IF(address == 0x04d6) {
		if((data != reg_val) || (force_update)) { // OK?
			force_update = false;
			write_port(data);
		}
	}
}

uint32_t JOYSTICK::read_io8(uint32_t address)
{
	// ToDo: Implement 6 buttons pad. & mouse
	switch(address) {
	case 0x04d0:
	case 0x04d2:
		{
			uint8_t num = (address & 0x02) >> 1;
			if((port_using[num] >= 0) && (port_using[num] < port_count[num])) {
				JSDEV_TEMPLATE *p = d_port[num][port_using[num]];
				if(p != nullptr) {
					bool _stat;
					p->query(_stat);
				}
			}
			return (data_reg[num] & data_mask[num]);
		}
		break;
	default:
		break;
	}
	return 0xff;
}


void JOYSTICK::write_signal(int id, uint32_t data, uint32_t mask)
{
	int ch = (id >> 16) & 1;
	int sigtype = id  & 0xffff;
	switch(sigtype) {
	case SIG_JSPORT_COM:
		stat_com[ch] = ((data & mask) != 0) ? true : false;
		data_reg[ch] = (data_reg[ch] & 0x3f) | ((stat_com[ch]) ? 0xc0 : 0x80);
		force_update = true;
		break;
	case SIG_JSPORT_DATA:
		data_reg[ch] = (data & 0x3f) | ((stat_com[ch]) ? 0xc0 : 0x80);
		force_update = true;
		break;
	}
}	

uint32_t JOYSTICK::read_signal(int id)
{
	int ch = (id >> 16) & 1;
	int sigtype = id  & 0xffff;
	uint32_t data = 0;
	switch(sigtype) {
	case SIG_JSPORT_COM:
		data = (stat_com[ch]) ? 0xffffffff : 0;
		break;
	case SIG_JSPORT_DATA:
		data = data_reg[ch] & 0xff;
		break;
	}
	return data;
}

void JOYSTICK::update_config(void)
{
	std::unique_lock<std::mutex> _l = lock_device();
	/*!
	  @fn update_config()
	  @brief Update config values to this class
	  @details config.machine_features[0,1] : JOYPORT 1,2 
	  value =
	  0: None connected
	  1: Towns PAD 2buttons
	  2: Towns PAD 6buttons
	  3: Towns MOUSE
	  4: Analog Pad (reserved)
	  5: Libble Rabble stick (reserved)
	*/
	bool change_jsport[2] = {false, false};
	const int js_limit = 3;
	for(int i = TOWNS_MACHINE_JOYPORT1; i <= TOWNS_MACHINE_JOYPORT2; i++) {
		// Remove connected device if changed.
		if((port_using[i] >= 0) && ((port_using[i] + 1) != config.machine_features[i])) {
			change_jsport[i] = true ;
			if(port_using[i] < port_count[i]) {
				JSDEV_TEMPLATE* p = d_port[i][port_using[i]];
				if(p != nullptr) {
					p->reset_device(true);
					p->set_enable(false); // Disconnect
				}
			}
			// Temporally mark "Not Connected".
		}
		port_using[i] = -1;
	}
	// Correct DATA REGISTERS.
	for(int i = 0; i < 2; i++) {
		make_mask(i, reg_val); // First, making mask.
		if(port_using[i] < 0) {
			reset_input_data(i);
		}
	}
	force_update = true;
	
	// Plug a device if changed (and usable).
	out_debug_log(_T("update_config() : PORT1=%d PORT2=%d"),
				  config.machine_features[TOWNS_MACHINE_JOYPORT1] - 1,
				  config.machine_features[TOWNS_MACHINE_JOYPORT2] - 1);
	for(int i = TOWNS_MACHINE_JOYPORT1; i <= TOWNS_MACHINE_JOYPORT2; i++) { 
		if((config.machine_features[i] > 0) && (config.machine_features[i] <= js_limit)) {
			JSDEV_TEMPLATE* p = d_port[i][config.machine_features[i] - 1];
			if(p != nullptr) {
				port_using[i] = config.machine_features[i] - 1;
				p->set_enable(true);
				p->reset_device(false);
				make_mask(i, reg_val);
				write_data_to_port(i, p, reg_val);
				bool _stat;
				p->query(_stat); // Query Twice.
			}
		}
	}
	// END JOYPORT.
}

bool JOYSTICK::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	_TCHAR sbuf[3][512] = {0};

	my_stprintf_s(sbuf[0], sizeof(sbuf[0]), _T("04D6h : MASK_REG=%02X\n"), reg_val);
	for(int i = 0; i < 2; i++) {
		my_stprintf_s(sbuf[i + 1], sizeof(sbuf[i + 1]), _T("PORT%d : TYPE=%d MASK=%02X IN: DATA=%02X COM=%d TRIG_A=%d TRIG_B=%d\n"),
					  i + 1,
					  port_using[i],
					  data_mask[i],
					  data_reg[i],
					  (stat_com[i])   ? 1 : 0,
					  (((data_reg[i] & data_mask[i]) & 0x10) != 0) ? 1 : 0,
					  (((data_reg[i] & data_mask[i]) & 0x20) != 0) ? 1 : 0
			);
	}
	my_stprintf_s(buffer, buffer_len,
				  _T("%s%s%s"),
				  sbuf[0], sbuf[1], sbuf[2]);
	return true;
}

#define STATE_VERSION 20

bool JOYSTICK::process_state(FILEIO *state_fio, bool loading)
{
	std::unique_lock<std::mutex> _l = lock_device();
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}

	state_fio->StateValue(reg_val);
	state_fio->StateValue(force_update);
	
	state_fio->StateArray(data_reg, sizeof(data_reg), 1);
	state_fio->StateArray(data_reg, sizeof(data_mask), 1);
	
	state_fio->StateArray(stat_com, sizeof(stat_com), 1);
	
	state_fio->StateArray(port_using, sizeof(port_using), 1);

	return true;
}

}
