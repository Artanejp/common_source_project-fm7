/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.10.02 -

	[ beep ]
*/

#include "vm.h"
#include "../emu.h"

#include "fifo.h"
#include "common.h"
#include "wav_sounder.h"

WAV_SOUNDER::WAV_SOUNDER(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	_data = NULL;
	play_flag = true;
	mute_flag = false;
	p_emu = parent_emu;
	volume_l = decibel_to_volume(0);
	volume_r = decibel_to_volume(0);
	this->set_device_name("WAV SOUNDER");
	for(int i = 0; i < (sizeof(render_table) / sizeof(int)); i++) {
		render_table[i] = -1;
	}
	sound_count = 0;
}

WAV_SOUNDER::~WAV_SOUNDER()
{
}

void WAV_SOUNDER::reset()
{
	touch_sound();
	sound_count = 0;
	for(int i = 0; i < (sizeof(render_table) / sizeof(int)); i++) {
		render_table[i] = -1;
	}

}

void WAV_SOUNDER::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(_data == NULL) return;
	switch(id) {
	case SIG_WAV_SOUNDER_ADD:
		touch_sound();
		if(sound_count < (sizeof(render_table) / sizeof(int))) {
			for(int i = 0; i < (sizeof(render_table) / sizeof(int)); i++) {
				if(render_table[i] < 0) {
					render_table[i] = 0;
					sound_count++;
					if(sound_count >= (sizeof(render_table) / sizeof(int))) sound_count = sizeof(render_table) / sizeof(int); 
					break;
				}
			}
		}
		break;
	case SIG_WAV_SOUNDER_PLAY:
		touch_sound();
		play_flag = ((data & mask) != 0);
		break;
	case SIG_WAV_SOUNDER_MUTE:
		touch_sound();
		mute_flag = ((data & mask) != 0);
		break;
	case SIG_WAV_SOUNDER_CLEAR:
		touch_sound();
		for(int i = 0; i < (sizeof(render_table) / sizeof(int)); i++) {
			render_table[i] = -1;
		}
		sound_count = 0;
		break;
	}
}

void WAV_SOUNDER::mix(int32_t *buffer, int cnt)
{
	if(_data == NULL) return;
	
	int32_t* buffer_tmp;
	int sounds = sizeof(render_table) / sizeof(int);
	int ptr = 0;
	int pp;
	int32_t data_l, data_r;
	if(!mute_flag) {
		for(int i = 0; i < sounds; i++) {
			ptr = render_table[i];
			if(ptr >= 0) {
				pp = ptr << 1;
				if(play_flag) {
					buffer_tmp = buffer;
					for(int j = 0; j < cnt; j++) {
						if(ptr >= dst_size) break;
						data_l = (int32_t)_data[pp + 0];
						data_r = (int32_t)_data[pp + 1];
						*buffer_tmp++ += apply_volume((int32_t)data_l, volume_l);
						*buffer_tmp++ += apply_volume((int32_t)data_r, volume_r);
						pp += 2;
						ptr++;
					}
				} else {
					ptr = ptr + cnt;
				}
				if(ptr < dst_size) {
					render_table[i] = ptr;
				} else {
					render_table[i] = -1;
					sound_count--;
					if(sound_count <= 0) sound_count = 0;
				}
			}
		}
	} else {
		for(int i = 0; i < sounds; i++) {
			ptr = render_table[i];
			if(ptr >= 0) {
				ptr = ptr + cnt;
				if(ptr < dst_size) {
					render_table[i] = ptr;
				} else {
					render_table[i] = -1;
					sound_count--;
					if(sound_count <= 0) sound_count = 0;
				}
			}
		}
	}
}

void WAV_SOUNDER::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

bool WAV_SOUNDER::load_data(const _TCHAR *path)
{
	if(path == NULL) return false;
	bool stat = false;
	_TCHAR sname[128];
	const _TCHAR *s;
	s = create_local_path(path);
	p_emu->load_sound_file(this_device_id, s, &_data, &dst_size);
	if((_data == NULL) || (dst_size <= 0)) {
		snprintf(sname, 127, "WAV SOUNDER #%d:%s (FAILED)", this_device_id, path);
	} else {
		snprintf(sname, 127, "WAV SOUNDER #%d:%s", this_device_id, path);
		stat = true;
	}
	this->set_device_name((const _TCHAR *)sname);
	return stat;
}

const _TCHAR *WAV_SOUNDER::get_file_name(void)
{
	return (const _TCHAR *)data_path;
}

void WAV_SOUNDER::release()
{
	if(_data != NULL) {
		p_emu->free_sound_file(this_device_id, &_data);
	}
	_data = NULL;
	dst_size = NULL;
}

#define STATE_VERSION	2
void WAV_SOUNDER::save_state(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputInt32(volume_l);
	state_fio->FputInt32(volume_r);
	state_fio->FputBool(play_flag);
	state_fio->FputBool(mute_flag);
	state_fio->Fwrite(data_path, sizeof(data_path), 1);
	state_fio->FputInt32(sound_count);
	state_fio->Fwrite(render_table, sizeof(render_table), 1);
}

bool WAV_SOUNDER::load_state(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;
	if(state_fio->FgetUint32() != STATE_VERSION) return false;
	if(state_fio->FgetInt32() != this_device_id) return false;
	
	volume_l = state_fio->FgetInt32();
	volume_r = state_fio->FgetInt32();
	play_flag = state_fio->FgetBool();
	mute_flag = state_fio->FgetBool();
	state_fio->Fread(data_path, sizeof(data_path), 1);
	sound_count = state_fio->FgetInt32();
	state_fio->Fread(render_table, sizeof(render_table), 1);
	
	_TCHAR tmp_path[_MAX_PATH];
	strncpy(tmp_path, data_path, _MAX_PATH);
	return load_data(tmp_path);
}

