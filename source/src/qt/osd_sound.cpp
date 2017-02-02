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
#include "csp_logger.h"
#include "menu_flags.h"

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
	
	sdl_snddata_t *pData = (sdl_snddata_t *)udata;
	if(pData == NULL) return;
	
	if(len <= 0) return;
	spos = 0;
	memset(stream, 0x00, len);
	do {
		SDL_SemWait(*pData->snd_apply_sem);
		if(pData->p_config->general_sound_level < -32768) pData->p_config->general_sound_level = -32768;
		if(pData->p_config->general_sound_level > 32767)  pData->p_config->general_sound_level = 32767;
		*pData->snd_total_volume = (uint8_t)(((uint32_t)(pData->p_config->general_sound_level + 32768)) >> 9);
		sndlen = *pData->sound_data_len;
		if(*(pData->sound_buffer_size)  <= *(pData->sound_write_pos)) { // Wrap
			*(pData->sound_write_pos) = 0;
		}
		len2 = *(pData->sound_buffer_size) - *(pData->sound_write_pos);
		if(*pData->sound_exit) {
			SDL_SemPost(*pData->snd_apply_sem);
			return;
		}
		if(len2 >= sndlen) len2 = sndlen;  // Okay
		if((spos + len2) >= (int)(len / sizeof(Sint16))) {
			len2 = (len / sizeof(Sint16)) - spos;
		}
		if((*(pData->sound_write_pos) + len2) >= *(pData->sound_buffer_size) ) len2 = *(pData->sound_buffer_size) - *(pData->sound_write_pos);
		
		if(*(pData->sound_debug)) csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_SOUND,
														"Callback,sound_write_pos=%d,spos=%d,len=%d,len2=%d",
														*(pData->sound_write_pos), spos, len, len2);
		if((len2 > 0) && (sndlen > 0)){
			writepos = *pData->sound_write_pos;
			p = (Uint8 *)(*pData->sound_buf_ptr);
			SDL_SemPost(*pData->snd_apply_sem);
			p = &p[writepos * 2];
			s = &stream[spos * 2];
			SDL_MixAudio(s, (Uint8 *)p, len2 * 2, *(pData->snd_total_volume));
			SDL_SemWait(*pData->snd_apply_sem);
			*(pData->sound_data_len) -= len2;
			if(*(pData->sound_data_len) <= 0) *(pData->sound_data_len) = 0;
			*pData->sound_write_pos += len2;
			SDL_SemPost(*pData->snd_apply_sem);
		} else {
			len2 = 0;
			SDL_SemPost(*pData->snd_apply_sem);
			if(spos >= (len / 2)) return;
			while(*(pData->sound_data_len) <= 0) {
				QThread::usleep(500);
				if(*pData->sound_exit) return;
			}
		}
		spos += len2;
	} while(spos < len); 
}

void OSD_BASE::initialize_sound(int rate, int samples)
{
	std::string devname;
	int i;

	sound_rate = rate;
	sound_samples = samples;
	sound_ok = sound_started = now_mute = now_record_sound = false;
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
	snd_apply_sem = NULL;
	// initialize direct sound

	snd_total_volume = 127;
   
	snddata.sound_buf_ptr = &sound_buf_ptr;
	snddata.sound_buffer_size = &sound_buffer_size;
	snddata.sound_write_pos = &sound_write_pos;
	snddata.sound_data_len = &sound_data_len;
	snddata.snd_apply_sem = &snd_apply_sem;
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
	for(i = 0; i < SDL_GetNumAudioDevices(0); i++) {
		//devname = SDL_GetAudioDeviceName(i, 0);
		QString tmps = QString::fromUtf8(SDL_GetAudioDeviceName(i, 0));
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,
							  "Audio Device: %s", tmps.toLocal8Bit().constData());
	}
#endif   
	SDL_OpenAudio(&snd_spec_req, &snd_spec_presented);
	audio_dev_id = 1;
   
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
	snd_apply_sem = SDL_CreateSemaphore(1);
	if(snd_apply_sem == NULL) {
		free(sound_buf_ptr);
		sound_buf_ptr = NULL;
		return;
	}
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_SOUND,
						  "Sound OK: BufSize = %d", sound_buffer_size);
	memset(sound_buf_ptr, 0x00, sound_buffer_size * sizeof(Sint16));
#if defined(USE_SDL2)   
	SDL_PauseAudioDevice(audio_dev_id, 0);
#else   
	SDL_PauseAudio(0);
#endif   

	sound_ok = sound_first_half = true;
}

void OSD_BASE::release_sound()
{
	// release SDL sound
	sound_exit = true;
#if defined(USE_SDL2)   
	SDL_CloseAudioDevice(audio_dev_id);
#else   
	SDL_CloseAudio();
#endif   
	if(snd_apply_sem != NULL) {
		SDL_DestroySemaphore(snd_apply_sem);
		snd_apply_sem = NULL;
	}
	if(sound_buf_ptr != NULL) free(sound_buf_ptr);
	sound_buf_ptr = NULL;
	// stop recording
	stop_record_sound();
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
		if(sound_debug) csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_SOUND,
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
		        if(snd_apply_sem) {
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
					if(ptr1) {
						memcpy(ptr1, sound_buffer, size1 * sizeof(Sint16));
					}
					if(ptr2) {
						memcpy(ptr2, &sound_buffer[size1], size2 * sizeof(Sint16));
					}
					sound_data_len = sound_data_len + ssize;
					if(sound_data_len >= sound_buffer_size) sound_data_len = sound_buffer_size;
					sound_data_pos = sound_data_pos + ssize;
					if(sound_data_pos >= sound_buffer_size) sound_data_pos = sound_data_pos - sound_buffer_size;
					//SDL_SemPost(*snddata.snd_apply_sem);
					//SDL_UnlockAudio();
					//SDL_PauseAudioDevice(audio_dev_id, 0);
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
		SDL_SemWait(*snddata.snd_apply_sem);
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
		SDL_SemPost(*snddata.snd_apply_sem);
	}
	now_mute = true;
}

void OSD_BASE::stop_sound()
{
	if(sound_ok && sound_started) {
#if defined(USE_SDL2)   
		SDL_PauseAudioDevice(audio_dev_id, 1);
#else   
		SDL_PauseAudio(1);
#endif   
		sound_started = false;
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
		strncpy(sound_file_name, tmps.toLocal8Bit().constData(), sizeof(sound_file_name));
		// create wave file
		rec_sound_fio = new FILEIO();
		if(rec_sound_fio->Fopen(bios_path(sound_file_name), FILEIO_WRITE_BINARY)) {
			// write dummy wave header
			wav_header_t wav_header;
			wav_chunk_t wav_chunk;
			memset(&wav_header, 0, sizeof(wav_header));
			memset(&wav_chunk, 0, sizeof(wav_chunk));
			rec_sound_fio->Fwrite(&wav_header, sizeof(wav_header), 1);
			rec_sound_fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
			
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
			
			memcpy(wav_header.riff_chunk.id, "RIFF", 4);
			wav_header.riff_chunk.size = EndianToLittle_DWORD(rec_sound_bytes + sizeof(wav_header_t) - 8);
			memcpy(wav_header.wave, "WAVE", 4);
			memcpy(wav_header.fmt_chunk.id, "fmt ", 4);
			wav_header.fmt_chunk.size = EndianToLittle_DWORD(16);
			wav_header.format_id = EndianToLittle_WORD(1);
			wav_header.channels =  EndianToLittle_WORD(2);
			wav_header.sample_bits = EndianToLittle_WORD(16);
			wav_header.sample_rate = EndianToLittle_DWORD(snd_spec_presented.freq);
			wav_header.block_size = EndianToLittle_WORD(wav_header.channels * wav_header.sample_bits / 8);
			wav_header.data_speed = EndianToLittle_DWORD(wav_header.sample_rate * wav_header.block_size);
			
			memcpy(wav_chunk.id, "data", 4);
			wav_chunk.size = EndianToLittle_DWORD(rec_sound_bytes);

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
