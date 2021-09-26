
#include "fileio.h"
#include "./mb87078.h"

void MB87078::reset()
{
	device_reset();
	for(int ch = 0; ch < 4; ch++) {
		set_volume_internal(ch, channels[ch].intvalue, true);
//		set_enable_internal(ch, current_en, true);
	}
}

void MB87078::initialize()
{
	device_reset();
	for(int ch = 0; ch < 4; ch++) {
		set_volume_internal(ch, channels[ch].intvalue, true);
		set_enable_internal(ch, current_en, true);
	}
}

void MB87078::write_io8(uint32_t addr, uint32_t data)
{
	
	regs[addr & 1] = data;
	int vol = reconfig_device();
	set_volume_internal(current_channel, vol);
	set_enable_internal(current_channel, current_en);
	
}

uint32_t MB87078::read_io8(uint32_t addr)
{
	return regs[addr & 1];
}

int MB87078::reconfig_device()
{
	current_c32 = ((regs[1] & 0x10) != 0) ? true : false;
	current_c0 = ((regs[1] & 0x08) != 0) ? true : false;
	current_en = ((regs[1] & 0x04) != 0) ? true : false;
	current_channel = regs[1] & 3;
		
	int vol = regs[0] & 0x3f;
	vol = vol - 63; // -31.5db
	if(current_c32) {
		vol = -64;
	}
	if(current_c0) {
		vol = 0;
	}
	return vol;
}

void MB87078::set_enable_internal(int ch, bool en, bool _force)
{
	bool tmp_en = channels[ch].enabled;
	channels[ch].enabled = en;
	
	if(channels[ch].dev != nullptr) {
		if((tmp_en != en) || (_force)) {
			if(channels[ch].mutesig >= 0) {
				bool _b = en;
				if(channels[ch].is_negative) {
					_b = !_b;
				}
				uint32_t mval = (_b) ? 0 : channels[ch].muteval;
				channels[ch].dev->write_signal(channels[ch].mutesig,
											   mval,
											   channels[ch].mutemask);
			}
		}
	}
}

void MB87078::set_volume_internal(int ch, int vol, bool _force)
{
	int tmp_vol = channels[ch].intvalue;
	channels[ch].intvalue = vol;
	
	if(channels[ch].dev != nullptr) {
		if(((vol != tmp_vol) || (_force)) && ((channels[ch].channelmask & 3) != 0)) {
			int left, right;
			channels[ch].dev->get_volume(channels[ch].devch, left, right);
			switch(channels[ch].channelmask & 3) {
			case 0:  // NONE
				break;
			case MB87078_TYPE_SET_LEFT: //
				left = channels[ch].extvalue + vol;
				break;	
			case MB87078_TYPE_SET_RIGHT: //
				right = channels[ch].extvalue + vol;
				break;
			case MB87078_TYPE_SET_CENTER: //
				left = channels[ch].extvalue + vol;
				right = left;
				break;
			}
			channels[ch].dev->set_volume(channels[ch].devch, left, right);
		}
	}
}

void MB87078::device_reset()
{
	regs[0] = 0x3f;
	regs[1] = 0x0c; // ENABLE AND C0
	int vol = reconfig_device();
	channels[current_channel].intvalue = vol;
	
	for(int ch = 0; ch < 4; ch++) {
		set_volume_internal(ch, channels[ch].intvalue, true);
//		set_enable_internal(ch, current_en, true);
	}	
}

void MB87078::set_volume_per_channel(int ch, int db)
{
	if((ch >= 0) && (ch < 4)) {
		int bak_db = channels[ch].extvalue;
		channels[ch].extvalue = db;
		if(bak_db != db) {
			set_volume_internal(ch, channels[ch].intvalue, true);
		}
	}
}

void MB87078::set_volumes(int left_ch, int left_db, int right_ch, int right_db)
{
	set_volume_per_channel(left_ch, left_db);
	set_volume_per_channel(right_ch, right_db);
}


#define STATE_VERSION	1

bool MB87078::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}

	state_fio->StateArray(regs, sizeof(regs), 1);
	
	for(int ch = 0; ch < 4; ch++) {
		state_fio->StateValue(channels[ch].enabled);
		state_fio->StateValue(channels[ch].intvalue);
	}
	if(loading) {
		int vol = reconfig_device();
		for(int ch = 0; ch < 4; ch++) {
			if(ch == current_channel) {
				set_volume_internal(ch, vol, true);
				set_enable_internal(ch, current_en, true);
			} else {
				set_volume_internal(ch, channels[ch].intvalue, true);
				set_enable_internal(ch, channels[ch].enabled, true);
			}
		}
	}
	return true;
}
