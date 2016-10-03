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
	queue = new FIFO(256);
	play_flag = true;
	mute_flag = false;
	p_emu = parent_emu;
	volume_l = decibel_to_volume(0);
	volume_r = decibel_to_volume(0);
	this->set_device_name("WAV SOUNDER");
}

WAV_SOUNDER::~WAV_SOUNDER()
{
	delete queue;
}

void WAV_SOUNDER::write_signal(int id, uint32_t data, uint32_t mask)
{
	if((queue == NULL) || (_data == NULL)) return;
	switch(id) {
	case SIG_WAV_SOUNDER_ADD:
		if(!queue->full()) {
			queue->write(0);
		}
		break;
	case SIG_WAV_SOUNDER_PLAY:
		play_flag = ((data & mask) != 0);
		break;
	case SIG_WAV_SOUNDER_MUTE:
		mute_flag = ((data & mask) != 0);
		break;
	case SIG_WAV_SOUNDER_CLEAR:
		queue->clear();
		break;
	}
}

void WAV_SOUNDER::mix(int32_t *buffer, int cnt)
{
	if((queue == NULL) || (_data == NULL)) return;
	
	int32_t* buffer_tmp;
	int sounds = queue->count();
	int ptr = 0;
	int pp;
	int32_t data_l, data_r;
	if(!mute_flag) {
		for(int i = 0; i < sounds; i++) {
			ptr = queue->read();
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
			if(ptr < dst_size) queue->write(ptr);
		}
	} else {
		for(int i = 0; i < sounds; i++) {
			ptr = queue->read();
			ptr = ptr + cnt;
			if(ptr < dst_size) queue->write(ptr);
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

#define STATE_VERSION	1
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
	queue->save_state(state_fio);
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
	if(!queue->load_state(state_fio)) {
		return false;
	}
	_TCHAR tmp_path[_MAX_PATH];
	strncpy(tmp_path, data_path, _MAX_PATH);
	return load_data(tmp_path);
}

