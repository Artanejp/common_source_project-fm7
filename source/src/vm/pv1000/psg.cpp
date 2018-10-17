/*
	CASIO PV-1000 Emulator 'ePV-1000'

	Author : Takeda.Toshiya
	Date   : 2006.11.16 -

	[ psg ]
*/

#include <math.h>
#include "psg.h"

#define PSG_CLOCK
#define PSG_VOLUME	8192

void PSG::reset()
{
	touch_sound();
	memset(ch, 0, sizeof(ch));
}

void PSG::write_io8(uint32_t addr, uint32_t data)
{
	touch_sound();
	ch[addr & 3].period = 0x3f - (data & 0x3f);
}

void PSG::initialize_sound(int rate)
{
	diff = (int)(1.3 * (double)CPU_CLOCKS / (double)rate + 0.5);
}

void PSG::mix(int32_t* buffer, int cnt)
{
	// create sound buffer
	for(int i = 0; i < cnt; i++) {
		int vol = 0;
		for(int j = 0; j < 3; j++) {
			if(!ch[j].period) {
				continue;
			}
			bool prev_signal = ch[j].signal;
			int prev_count = ch[j].count;
			ch[j].count -= diff;
			if(ch[j].count < 0) {
				ch[j].count += ch[j].period << 8;
				ch[j].signal = !ch[j].signal;
			}
			int vol_tmp = (prev_signal != ch[j].signal && prev_count < diff) ? (PSG_VOLUME * (2 * prev_count - diff)) / diff : PSG_VOLUME;
			vol += prev_signal ? vol_tmp : -vol_tmp;
		}
		*buffer++ += apply_volume(vol, volume_l); // L
		*buffer++ += apply_volume(vol, volume_l); // R
	}
}

void PSG::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

#define STATE_VERSION	1

#include "../../statesub.h"

void PSG::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_INT32_STRIDE((ch[0].period), 3, sizeof(ch[0]));
	
	leave_decl_state();
}

void PSG::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	for(int i = 0; i < 3; i++) {
//		state_fio->FputInt32(ch[i].period);
//	}
}

bool PSG::load_state(FILEIO* state_fio)
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
//	for(int i = 0; i < 3; i++) {
//		ch[i].period = state_fio->FgetInt32();
//	}
	return true;
}

bool PSG::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < 3; i++) {
		state_fio->StateInt32(ch[i].period);
	}
	return true;
}
