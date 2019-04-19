/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[Qt/SDL sound ]
*/

#include "../emu.h"
#include "../fileio.h"

#include <SDL.h>

#include "qt_main.h"
//#include "csp_logger.h"
#include "gui/menu_flags.h"

#include <QString>
#include <QDateTime>
#include <QThread>

void OSD_BASE::audio_callback(void *udata, Uint8 *stream, int len)
{
	int len2 = len;
	int spos;
	Uint8 *p;
	Uint8 *s;
	int writepos;
	int sndlen;
	//printf("Callback: udata=%08x stream=%08x len=%d\n", udata, stream, len);
	sdl_snddata_t *pData = (sdl_snddata_t *)udata;
	if(pData == NULL) return;
	
	if(len <= 0) return;
	spos = 0;
	memset(stream, 0x00, len);
	do {
		if(pData->p_config->general_sound_level < -32768) pData->p_config->general_sound_level = -32768;
		if(pData->p_config->general_sound_level > 32767)  pData->p_config->general_sound_level = 32767;
		*pData->snd_total_volume = (uint8_t)(((uint32_t)(pData->p_config->general_sound_level + 32768)) >> 9);
		sndlen = *pData->sound_data_len;
		if(*(pData->sound_buffer_size)  <= *(pData->sound_write_pos)) { // Wrap
			*(pData->sound_write_pos) = 0;
		}
		len2 = *(pData->sound_buffer_size) - *(pData->sound_write_pos);
		if(*pData->sound_exit) {
			return;
		}
		if(len2 >= sndlen) len2 = sndlen;  // Okay
		if((spos + len2) >= (int)(len / sizeof(Sint16))) {
			len2 = (len / sizeof(Sint16)) - spos;
		}
		if((*(pData->sound_write_pos) + len2) >= *(pData->sound_buffer_size) ) len2 = *(pData->sound_buffer_size) - *(pData->sound_write_pos);
		
		//if(*(pData->sound_debug)) debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_SOUND,
		//												"Callback,sound_write_pos=%d,spos=%d,len=%d,len2=%d",
		//												*(pData->sound_write_pos), spos, len, len2);
		if((len2 > 0) && (sndlen > 0)){
			writepos = *pData->sound_write_pos;
			p = (Uint8 *)(*pData->sound_buf_ptr);
			p = &p[writepos * 2];
			s = &stream[spos * 2];
#if defined(USE_SDL2)
			SDL_MixAudioFormat(s, (Uint8*)p, pData->sound_format, len2 * 2, *(pData->snd_total_volume));
#else
			SDL_MixAudio(s, (Uint8 *)p, len2 * 2, *(pData->snd_total_volume));
#endif
			*(pData->sound_data_len) -= len2;
			if(*(pData->sound_data_len) <= 0) *(pData->sound_data_len) = 0;
			*pData->sound_write_pos += len2;
		} else {
			len2 = 0;
			if(spos >= (len / 2)) return;
			if(*(pData->sound_data_len) <= 0) return;
			//while(*(pData->sound_data_len) <= 0) {
			//	QThread::usleep(500);
			//	if(*pData->sound_exit) return;
			//}
		}
		spos += len2;
		if(*pData->sound_exit) return;
	} while(spos < len); 
}

const _TCHAR *OSD_BASE::get_sound_device_name(int num)
{
	if(num < 0) return NULL;
	if(num >= sound_device_list.count()) return NULL;
	QString sdev = sound_device_list.value(num);
	sdev.truncate(64);
	return (const _TCHAR*)(sdev.toUtf8().constData());
}

int OSD_BASE::get_sound_device_num()
{
	return sound_device_list.count();
}

void OSD_BASE::initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples)
{
	std::string devname;
	int i;

	if(sound_initialized) {
		release_sound();
	}
	sound_rate = rate;
	sound_samples = samples;
	rec_sound_buffer_ptr = 0;
	sound_ok = sound_started = now_mute = now_record_sound = false;
	sound_write_pos = 0;
	sound_data_len = 0;
	sound_buffer_size = 0;
	sound_data_pos = 0;
	sound_exit = false;
	sound_debug = false;
	//sound_debug = true;
	sound_buf_ptr = NULL;
	sound_initialized = false;
	// initialize direct sound

	snd_total_volume = 127;
   
	snddata.sound_buf_ptr = &sound_buf_ptr;
	snddata.sound_buffer_size = &sound_buffer_size;
	snddata.sound_write_pos = &sound_write_pos;
	snddata.sound_data_len = &sound_data_len;
	snddata.snd_total_volume = &snd_total_volume;
	snddata.sound_exit = &sound_exit;
	snddata.sound_debug = &sound_debug;
	snddata.p_config = p_config;
	
	snd_spec_req.format = AUDIO_S16SYS;
	snd_spec_req.channels = 2;
	snd_spec_req.freq = sound_rate;
	//snd_spec_req.samples = ((sound_rate * 100) / 1000);
	snd_spec_req.samples = samples;
	snd_spec_req.callback = &(this->audio_callback);
	snd_spec_req.userdata = (void *)&snddata;
#if defined(USE_SDL2)
	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND, "Using Sound Driver: %s\n", SDL_GetCurrentAudioDriver());
	sound_device_list.clear();
	for(i = 0; i < SDL_GetNumAudioDevices(0); i++) {
		QString tmps = QString::fromUtf8(SDL_GetAudioDeviceName(i, 0));
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,
				  "Audio Device #%d: %s", i, tmps.toLocal8Bit().constData());
		sound_device_list.append(tmps);
	}
	
	//QString sdev = QString::fromUtf8("\"") + sound_device_list.at(audio_dev_id) + QString::fromUtf8("\"");
	//audio_dev_id = SDL_OpenAudioDevice(NULL, 0, &snd_spec_req, &snd_spec_presented, SDL_AUDIO_ALLOW_ANY_CHANGE);
	QString sdev;
	sdev = sound_device_list.at(p_config->sound_device_num);
    audio_dev_id = SDL_OpenAudioDevice(sdev.toUtf8().constData(), 0,
									   &snd_spec_req, &snd_spec_presented,
									   0);
	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND, "Try to open DEVICE #%d: %s -> %s: DEVID=%d\n",
			  p_config->sound_device_num, sdev.toUtf8().constData(), (audio_dev_id <= 0) ? "FAIL" : "SUCCESS", audio_dev_id);
#else
	audio_dev_id = 1;
	SDL_OpenAudio(&snd_spec_req, &snd_spec_presented);
#endif

#if defined(USE_SDL2)
	if(audio_dev_id <= 0) {
		debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,"Failed to initialize sound\n");
		if(presented_rate != NULL) {
			*presented_rate = sound_rate;
		}
		if(presented_samples != NULL) {
			*presented_samples = sound_samples;
		}
		sound_initialized = false;
		sound_ok = sound_first_half = false;
		return;
	}
#endif
	snddata.sound_format = snd_spec_presented.format;
	if((snd_spec_presented.freq != sound_rate) ||
	   (snd_spec_presented.samples != sound_samples)) { // DEINI
		sound_rate = snd_spec_presented.freq;
		sound_samples = snd_spec_presented.samples;
	}
	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,"Sample rate=%d samples=%d\n", sound_rate, sound_samples);
	if(presented_rate != NULL) {
		*presented_rate = sound_rate;
	}
	if(presented_samples != NULL) {
		*presented_samples = sound_samples;
	}
	// secondary buffer
	sound_buffer_size = sound_samples * snd_spec_presented.channels * 2;
	sound_buf_ptr = (Sint16 *)malloc(sound_buffer_size * sizeof(Sint16)); 
	if(sound_buf_ptr == NULL) {
#if defined(USE_SDL2)   	   
		SDL_CloseAudioDevice(audio_dev_id);
#else	   
		SDL_CloseAudio();
#endif	   
		return;
	}

	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,
						  "Sound OK: BufSize = %d", sound_buffer_size);
	memset(sound_buf_ptr, 0x00, sound_buffer_size * sizeof(Sint16));
	sound_initialized = true;
	sound_ok = sound_first_half = true;
}

void OSD_BASE::release_sound()
{
	// release SDL sound
	sound_exit = true;
	sound_initialized = false;

#if defined(USE_SDL2)   
	//SDL_PauseAudioDevice(audio_dev_id, 1);
	SDL_CloseAudioDevice(audio_dev_id);
#else   
	SDL_CloseAudio();
#endif   
	stop_record_sound();
	if(sound_buf_ptr != NULL) free(sound_buf_ptr);
	sound_buf_ptr = NULL;
	// stop recording
}

void OSD_BASE::update_sound(int* extra_frames)
{
	*extra_frames = 0;
	
	now_mute = false;
	if(sound_ok) {
		uint32_t play_c, size1, size2;
		//uint32_t offset;
		Sint16 *ptr1, *ptr2;
		
		// start play
		// check current position
		play_c = sound_write_pos;
		if(sound_debug) debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_SOUND,
											  "Called time=%d sound_write_pos=%d\n", osd_timer.elapsed(), play_c);
		if(!sound_first_half) {
			if((int)play_c < (sound_buffer_size / 2)) {
				return;
			}
			//offset = 0;
		} else {
			if((int)play_c >= (sound_buffer_size / 2)) {
				return;
			}
			//offset = sound_buffer_size / 2;
		}
		//SDL_UnlockAudio();
		// sound buffer must be updated
		Sint16* sound_buffer = (Sint16 *)this->create_sound(extra_frames);
		if(now_record_sound || now_record_video) {
			if(sound_samples > rec_sound_buffer_ptr) {
				int samples = sound_samples - rec_sound_buffer_ptr;
				int length = samples * sizeof(int16_t) * 2; // stereo
				rec_sound_bytes += length;
				if(now_record_video) {
					//AGAR_DebugLog(AGAR_LOG_DEBUG, "Push Sound %d bytes\n", length);
					emit sig_enqueue_audio((int16_t *)(&(sound_buffer[rec_sound_buffer_ptr * 2])), length);
				}
				// record sound
				if(now_record_sound) {
					rec_sound_fio->Fwrite(sound_buffer + rec_sound_buffer_ptr * 2, length, 1);
				}
				//if(now_record_video) {
				//	// sync video recording
				//	static double frames = 0;
				//	static int prev_samples = -1;
				//	static double prev_fps = -1;
				//	double fps = this->vm_frame_rate();
				//	frames = fps * (double)samples / (double)sound_rate;
				//}
				//printf("Wrote %d samples ptr=%d\n", samples, rec_sound_buffer_ptr);
				rec_sound_buffer_ptr += samples;
				if(rec_sound_buffer_ptr >= sound_samples) rec_sound_buffer_ptr = 0;
			}
		}
		if(sound_buffer) {
		        int ssize;
		        int pos;
		        int pos2;
		        if(sound_initialized) {
					ssize = sound_samples * snd_spec_presented.channels;

			        pos = sound_data_pos;
			        pos2 = pos + ssize;
		        	ptr1 = &sound_buf_ptr[pos];
			        if(pos2 >= sound_buffer_size) {
						size1 = sound_buffer_size  - pos;
						size2 = pos2 - sound_buffer_size;
						ptr2 = &sound_buf_ptr[0];
					} else {
						size1 = ssize;
						size2 = 0;
						ptr2 = NULL;
					}
#if defined(USE_SDL2)   
					SDL_LockAudioDevice(audio_dev_id);
#else
					SDL_LockAudio();
#endif
					if(ptr1) {
						my_memcpy(ptr1, sound_buffer, size1 * sizeof(Sint16));
					}
					if(ptr2) {
						my_memcpy(ptr2, &sound_buffer[size1], size2 * sizeof(Sint16));
					}
					sound_data_len = sound_data_len + ssize;
					if(sound_data_len >= sound_buffer_size) sound_data_len = sound_buffer_size;
					sound_data_pos = sound_data_pos + ssize;
					if(sound_data_pos >= sound_buffer_size) sound_data_pos = sound_data_pos - sound_buffer_size;
					if(!sound_started) sound_started = true;
#if defined(USE_SDL2)   
					SDL_UnlockAudioDevice(audio_dev_id);
#else
					SDL_UnlockAudio();
#endif
					//SDL_UnlockAudio();
					SDL_PauseAudioDevice(audio_dev_id, 0);
			}
		}
	   
//	        SDL_PauseAudioDevice(audio_dev_id, 0);
		sound_first_half = !sound_first_half;
	}
}

void OSD_BASE::mute_sound()
{
	if(!now_mute && sound_ok) {
		// check current position
		uint32_t size1, size2;
	    
		Sint16 *ptr1, *ptr2;
		// WIP
		int ssize;
		int pos;
		int pos2;
#if defined(USE_SDL2)   
		SDL_LockAudioDevice(audio_dev_id);
#else
		SDL_LockAudio();
#endif
		ssize = sound_buffer_size / 2;
		pos = sound_data_pos;
		pos2 = pos + ssize;
		ptr1 = &sound_buf_ptr[pos];
		if(pos2 >= sound_buffer_size) {
			size1 = sound_buffer_size - pos;
			size2 = pos2 - sound_buffer_size;
			ptr2 = &sound_buf_ptr[0];
		} else {
			size1 = ssize;
			size2 = 0;
			ptr2 = NULL;
		}
		
		if(ptr1) {
			memset(ptr1, 0x00, size1 * sizeof(Sint16));
		}
		if(ptr2) {
			memset(ptr2, 0x00, size2 * sizeof(Sint16));
		}
		sound_data_pos = (sound_data_pos + ssize) % sound_buffer_size;
#if defined(USE_SDL2)   
		SDL_UnlockAudioDevice(audio_dev_id);
#else
		SDL_UnlockAudio();
#endif
	}
	now_mute = true;
}

void OSD_BASE::stop_sound()
{
	if(sound_ok && sound_started) {
		//sound_exit = true;
#if defined(USE_SDL2)   
		SDL_PauseAudioDevice(audio_dev_id, 1);
#else   
		SDL_PauseAudio(1);
#endif   
		sound_started = false;
		//sound_exit = false;
	}
}

void OSD_BASE::start_record_sound()
{
   
	if(!now_record_sound) {
		//LockVM();
		QDateTime nowTime = QDateTime::currentDateTime();
		QString tmps = QString::fromUtf8("Sound_Save_emu");
		tmps = tmps + get_vm_config_name();
		tmps = tmps + QString::fromUtf8("_");
		tmps = tmps + nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz"));
		tmps = tmps + QString::fromUtf8(".wav");
		strncpy((char *)sound_file_name, tmps.toLocal8Bit().constData(), sizeof(sound_file_name) - 1);
		// create wave file
		rec_sound_fio = new FILEIO();
		if(rec_sound_fio->Fopen(bios_path(sound_file_name), FILEIO_WRITE_BINARY)) {
			// write dummy wave header
			write_dummy_wav_header((void *)rec_sound_fio);
			
			rec_sound_bytes = 0;
			rec_sound_buffer_ptr = 0;
			now_record_sound = true;
		} else {
			// failed to open the wave file
			delete rec_sound_fio;
		}
		//UnlockVM();
	}
}

void OSD_BASE::stop_record_sound()
{
		if(now_record_sound) {
			//LockVM();
		if(rec_sound_bytes == 0) {
			rec_sound_fio->Fclose();
			rec_sound_fio->RemoveFile(sound_file_name);
		} else {
			// update wave header
			wav_header_t wav_header;
			wav_chunk_t wav_chunk;
#if 0
			pair16_t tmpval16;
			pair32_t tmpval32;
			
			memcpy(wav_header.riff_chunk.id, "RIFF", 4);
			
			tmpval32.d = rec_sound_bytes + sizeof(wav_header_t) - 8;
			wav_header.riff_chunk.size = tmpval32.get_4bytes_le_to();
			
			memcpy(wav_header.wave, "WAVE", 4);
			memcpy(wav_header.fmt_chunk.id, "fmt ", 4);
			
			tmpval32.d = 16;
			wav_header.fmt_chunk.size = tmpval32.get_4bytes_le_to();

			tmpval16.w = 1;
			wav_header.format_id = tmpval16.get_2bytes_le_to();
			
			tmpval16.w = 2;
			wav_header.channels =  tmpval16.get_2bytes_le_to();
			
			tmpval16.w = 16;
			wav_header.sample_bits = tmpval16.get_2bytes_le_to();

			tmpval32.d = snd_spec_presented.freq;
			wav_header.sample_rate = tmpval32.get_4bytes_le_to();

			tmpval16.w = wav_header.channels * wav_header.sample_bits / 8;
			wav_header.block_size = tmpval16.get_2bytes_le_to();

			tmpval32.d = wav_header.sample_rate * wav_header.block_size;
			wav_header.data_speed = tmpval32.get_4bytes_le_to();
			
			memcpy(wav_chunk.id, "data", 4);

			tmpval32.d = rec_sound_bytes;
			wav_chunk.size = tmpval32.get_4bytes_le_to();
#else
			if(!set_wav_header(&wav_header, &wav_chunk, 2, snd_spec_presented.freq, 16,
							 (size_t)(rec_sound_bytes + sizeof(wav_header) + sizeof(wav_chunk)))) {
				delete rec_sound_fio;
				now_record_sound = false;
				return;
			}
#endif
			rec_sound_fio->Fseek(0, FILEIO_SEEK_SET);
			rec_sound_fio->Fwrite(&wav_header, sizeof(wav_header_t), 1);
			rec_sound_fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
			rec_sound_fio->Fclose();
		}
		delete rec_sound_fio;
		now_record_sound = false;
		//UnlockVM();
	}
}

void OSD_BASE::restart_record_sound()
{
	bool tmp = now_record_sound;
	stop_record_sound();
	if(tmp) {
		start_record_sound();
	}
}

int OSD_BASE::get_sound_rate()
{
	return snd_spec_presented.freq;
}

void OSD_BASE::load_sound_file(int id, const _TCHAR *name, int16_t **data, int *dst_size)
{
	if(data != NULL) *data = NULL;
	if(dst_size != NULL) *dst_size = 0;
}

void OSD_BASE::free_sound_file(int id, int16_t **data)
{
}

void OSD_BASE::init_sound_files()
{
}

void OSD_BASE::release_sound_files()
{
}
