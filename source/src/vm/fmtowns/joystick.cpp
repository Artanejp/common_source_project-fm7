/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.01.28 -
    History : 2020.01.28 Initial from FM7.
	[ Towns Joystick ports]

*/

#include "./joystick.h"
#include "./js_template.h"

namespace FMTOWNS {
	
void JOYSTICK::reset()
{
	update_config(); // Update MOUSE PORT.
	write_data_to_port(0xff);
}

void JOYSTICK::initialize()
{
	for(int i = 0; i < 2; i++) {
		stat_com[i] = true;
		stat_triga[i] = false;
		stat_trigb[i] = false;
		data_reg[i] = 0xff;
		data_mask[i] = 0x7f;
	}
}

void JOYSTICK::release()
{
}

void JOYSTICK::write_data_to_port(uint8_t data)
{
	std::unique_lock<std::mutex> _l = lock_device();
	
	reg_val = data;
	for(int num = 0; num < 2; num++) {
		uint32_t triga_mask = 0x01;
		uint32_t trigb_mask = 0x02;
		uint32_t com_mask   = 0x10;
		uint8_t _com = data;
		uint8_t _trig = data;
		if(num != 0) {
			_com  >>= 1;
			_trig >>= 2;
			triga_mask <<= 2;
			trigb_mask <<= 2;
			com_mask   <<= 1;
		}
		data_mask[num] = ((_com & 1) << 6) | ((_trig & 3) << 4) | 0x0f; 
		
		if((port_using[num] >= 0) && (port_using[num] < port_count[num])) {
			JSDEV_TEMPLATE *p = d_port[num][port_using[num]];
			if(p != nullptr) {
				uint32_t e_num = num << 16;
				p->write_signal(e_num | SIG_JS_COM, data, com_mask);
				p->write_signal(e_num | SIG_JS_TRIG_A, data, triga_mask);
				p->write_signal(e_num | SIG_JS_TRIG_B, data, trigb_mask);
				bool _stat;
				p->query(_stat);
			}
		}
	}	
}

	
void JOYSTICK::write_io8(uint32_t address, uint32_t data)
{
	// ToDo: Mouse
	if(address == 0x04d6) {
		if(data != reg_val) { // OK?
			write_data_to_port(data);
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
		data_reg[ch] = (data_reg[ch] & 0x3f) | ((stat_com[ch]) ? 0x40 : 0x00);
		break;
	case SIG_JSPORT_DATA:
		data_reg[ch] = (data & 0x3f) | ((stat_com[ch]) ? 0x40 : 0x00);
		stat_triga[ch] = ((data_reg[ch] & 0x10) == 0) ? true : false;
		stat_trigb[ch] = ((data_reg[ch] & 0x20) == 0) ? true : false;
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
		data = data_reg[ch] & 0x7f;
		break;
	}
	return data;
}

void JOYSTICK::update_config(void)
{
	std::unique_lock<std::mutex> _l = lock_device();
	// BEGIN JOYPORT.
	// config.machine_features[0,1] : JOYPORT 1,2
	// value =
	// 0: None connected
	// 1: Towns PAD 2buttons
	// 2: Towns PAD 6buttons
	// 3: Towns MOUSE
	// 4: Analog Pad (reserved)
	// 5: Libble Rabble stick (reserved)
	bool change_jsport = false;
	const int js_limit = 2;
	for(int i = 0; i < 2; i++) {
		if((port_using[i] + 1) != config.machine_features[i]) {
			change_jsport = true ;
			if((port_using[i] >= 0) && (port_using[i] < port_count[i])) {
				JSDEV_TEMPLATE* p = d_port[i][port_using[i]];
				if(p != nullptr) {
					p->set_enable(false); // Disconnect
				}
			}
			if((config.machine_features[i] > 0) && (config.machine_features[i] <= js_limit)) {
				JSDEV_TEMPLATE* p = d_port[i][config.machine_features[i] - 1];
				if(p != nullptr) {
					p->set_enable(true);
					p->reset_device();
					p->query(_stat); // Query Twice.
				}
			}
		}
	}
//	if(change_jsport) {
	write_data_to_port(reg_val);
//	}
	for(int i = 0; i < 2; i++) {
		if((config.machine_features[i] > 0) && (config.machine_features[i] <= js_limit)) {
			JSDEV_TEMPLATE* p = d_port[i][config.machine_features[i] - 1];
			if(p != nullptr) {
				port_using[i] = ((int)config.machine_features[i]) - 1;
			} else {
				port_using[i] = -1;
			}
		} else {
			port_using[i] = -1;
		}
	}
	// END JOYPORT.
}


#define STATE_VERSION 17

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
	state_fio->StateArray(data_reg, sizeof(data_reg), 1);
	state_fio->StateArray(stat_com, sizeof(stat_com), 1);
	
	state_fio->StateArray(port_using, sizeof(port_using), 1);

	if(loading) {
		for(int i = 0; i < 2; i++) {
			uint8_t _trig = reg_val;
			uint8_t _com  = reg_val >> 4;
			
			if(i != 0) {
				_com >>= 1;
				_trig >>= 2;
			}
			data_mask[i] = ((_com & 1) << 6) | ((_trig & 3) << 4) | 0x0f; 
			stat_triga[i] = ((data_reg[i] & 0x10) == 0) ? true : false;
			stat_trigb[i] = ((data_reg[i] & 0x20) == 0) ? true : false;
		}
	}
	return true;
}

}
