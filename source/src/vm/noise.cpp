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
	DEVICE::initialize();
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
#if 0		
		wav_header_t header;
		wav_chunk_t chunk;
		pair16_t __fmt_id;
		pair16_t __sample_bits;
		pair16_t __channels;
		pair_t __sample_rate;
		pair_t __chunk_size;

		fio->Fread(&header, sizeof(header), 1);
		__fmt_id.set_2bytes_le_from(header.format_id);
		__sample_bits.set_2bytes_le_from(header.sample_bits);
		__chunk_size.set_4bytes_le_from(header.fmt_chunk.size);
		__channels.set_2bytes_le_from(header.channels);
		__sample_rate.set_4bytes_le_from(header.sample_rate);
		
		if((__fmt_id.w == 1) && ((__sample_bits.w == 8) || (__sample_bits.w == 16))) {
			fio->Fseek(__chunk_size.d - 16, FILEIO_SEEK_CUR);
			bool is_eof = false;
			while(1) {
				if(fio->Fread(&chunk, sizeof(chunk), 1) != 1) {
					is_eof = true;
					break;
				}
				__chunk_size.set_4bytes_le_from(chunk.size);
				if(strncmp(chunk.id, "data", 4) == 0) {
					break;
				}
				fio->Fseek(__chunk_size.d, FILEIO_SEEK_CUR);
			}
			__chunk_size.set_4bytes_le_from(chunk.size);
			if(is_eof) {
				buffer_l = buffer_r = NULL;
				fio->Fclose();
				delete fio;
				return false;
			}
			if((samples = __chunk_size.d / __channels.w) > 0) {
				if(__sample_bits.w == 16) {
					samples /= 2;
				}
				sample_rate = __sample_rate.d;
			   
				buffer_l = (int16_t *)malloc(samples * sizeof(int16_t));
				buffer_r = (int16_t *)malloc(samples * sizeof(int16_t));
				
				for(int i = 0; i < samples; i++) {
					int sample_lr[2];
					for(int ch = 0; ch < __channels.sw; ch++) {
						int16_t sample = 0;
						if(__sample_bits.w == 16) {
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
					buffer_r[i] = sample_lr[(__channels.w > 1) ? 1 : 0];
				}
				result = true;
			}
		}
#else
		result = load_wav_to_stereo((void *)fio, &buffer_l, &buffer_r, &sample_rate, &samples);
#endif
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
		sample_r = buffer_r[ptr];
	} else {
		sample_r = 0;
	}
}

#define STATE_VERSION	1

#include "../statesub.h"

void NOISE::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_INT32(register_id);
	DECL_STATE_ENTRY_INT32(ptr);
	DECL_STATE_ENTRY_INT32(sample_l);
	DECL_STATE_ENTRY_INT32(sample_r);
	DECL_STATE_ENTRY_BOOL(loop);
	DECL_STATE_ENTRY_BOOL(mute);

	leave_decl_state();
}
void NOISE::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
	//state_fio->FputUint32(STATE_VERSION);
	//state_fio->FputInt32(this_device_id);
	
	//state_fio->FputInt32(register_id);
	//state_fio->FputInt32(ptr);
	//state_fio->FputInt32(sample_l);
	//state_fio->FputInt32(sample_r);
	//state_fio->FputBool(loop);
	//state_fio->FputBool(mute);
}

bool NOISE::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) return false;
	//if(state_fio->FgetUint32() != STATE_VERSION) {
	//	return false;
	//}
	//if(state_fio->FgetInt32() != this_device_id) {
	//	return false;
	//}
	//register_id = state_fio->FgetInt32();
	//ptr = state_fio->FgetInt32();
	//sample_l = state_fio->FgetInt32();
	//sample_r = state_fio->FgetInt32();
	//loop = state_fio->FgetBool();
	//mute = state_fio->FgetBool();
	return true;
}

