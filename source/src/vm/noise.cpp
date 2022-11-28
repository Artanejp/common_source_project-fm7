/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2017.03.08-

	[ noise player ]
*/

#include "noise.h"

#define EVENT_SAMPLE	0

void NOISE::initialize()
{
	register_id = -1;
	ptr = 0;
	sample_l = sample_r = 0;
}

void NOISE::release()
{
	if(buffer_l != NULL) {
		free(buffer_l);
		buffer_l = NULL;
	}
	if(buffer_r != NULL) {
		free(buffer_r);
		buffer_r = NULL;
	}
}

void NOISE::reset()
{
	stop();
}

void NOISE::event_callback(int event_id, int err)
{
	if(++ptr < samples) {
		get_sample();
	} else if(loop) {
		ptr = 0;
		get_sample();
	} else {
		stop();
	}
}

void NOISE::mix(int32_t* buffer, int cnt)
{
	if(register_id != -1 && !mute) {
		int32_t val_l = apply_volume(sample_l, volume_l);
		int32_t val_r = apply_volume(sample_r, volume_r);
		
		for(int i = 0; i < cnt; i++) {
			*buffer++ += val_l; // L
			*buffer++ += val_r; // R
		}
	}
}

void NOISE::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

bool NOISE::load_wav_file(const _TCHAR *file_name)
{
	if(samples != 0) {
		// already loaded
		return true;
	}
	FILEIO *fio = new FILEIO();
	bool result = false;
	
	if(fio->Fopen(create_local_path(file_name), FILEIO_READ_BINARY)) {
		wav_header_t header;
		wav_chunk_t chunk;
		
		fio->Fread(&header, sizeof(header), 1);
		
		if(header.format_id == 1 && (header.sample_bits == 8 || header.sample_bits == 16)) {
			fio->Fseek(header.fmt_chunk.size - 16, FILEIO_SEEK_CUR);
			while(1) {
				fio->Fread(&chunk, sizeof(chunk), 1);
				if(strncmp(chunk.id, "data", 4) == 0) {
					break;
				}
				fio->Fseek(chunk.size, FILEIO_SEEK_CUR);
			}
			if((samples = chunk.size / header.channels) > 0) {
				if(header.sample_bits == 16) {
					samples /= 2;
				}
				sample_rate = header.sample_rate;
				
				buffer_l = (int16_t *)malloc(samples * sizeof(int16_t));
				buffer_r = (int16_t *)malloc(samples * sizeof(int16_t));
				
				for(int i = 0; i < samples; i++) {
					int sample_lr[2];
					for(int ch = 0; ch < header.channels; ch++) {
						int16_t sample = 0;
						if(header.sample_bits == 16) {
							union {
								int16_t s16;
								struct {
									uint8_t l, h;
								} b;
							} pair;
							pair.b.l = fio->FgetUint8();
							pair.b.h = fio->FgetUint8();
							sample = pair.s16;
						} else {
							sample = (int16_t)(fio->FgetUint8());
							sample = (sample - 128) * 256;
						}
						if(ch < 2) sample_lr[ch] = sample;
					}
					buffer_l[i] = sample_lr[0];
					buffer_r[i] = sample_lr[(header.channels > 1) ? 1 : 0];
				}
				result = true;
			}
		}
		fio->Fclose();
	}
	delete fio;
	
	return result;
}

void NOISE::play()
{
	if(samples > 0 && register_id == -1 && !mute) {
		touch_sound();
//		if(register_id == -1) {
			register_event(this, EVENT_SAMPLE, 1000000.0 / sample_rate, true, &register_id);
//		}
		ptr = 0;
		get_sample();
		set_realtime_render(this, true);
	}
}

void NOISE::stop()
{
	if(samples > 0 && register_id != -1) {
		touch_sound();
//		if(register_id != -1) {
			cancel_event(this, register_id);
			register_id = -1;
//		}
		sample_l = sample_r = 0;
		set_realtime_render(this, false);
	}
}

void NOISE::get_sample()
{
	if(buffer_l != NULL && ptr < samples) {
		sample_l = buffer_l[ptr];
	} else {
		sample_l = 0;
	}
	if(buffer_r != NULL && ptr < samples) {
		sample_r = buffer_l[ptr];
	} else {
		sample_r = 0;
	}
}

#define STATE_VERSION	1

bool NOISE::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(register_id);
	state_fio->StateValue(ptr);
	state_fio->StateValue(sample_l);
	state_fio->StateValue(sample_r);
	state_fio->StateValue(loop);
	state_fio->StateValue(mute);
	return true;
}

