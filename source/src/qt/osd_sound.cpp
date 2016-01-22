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
#include "agar_logger.h"

#include <QString>
#include <QDateTime>
#include <QThread>

void OSD::audio_callback(void *udata, Uint8 *stream, int len)
{
	int pos;
	int blen = len;
	int len2 = len;
	int channels = 2;
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
		if(config.general_sound_level < -32768) config.general_sound_level = -32768;
		if(config.general_sound_level > 32767)  config.general_sound_level = 32767;
		*pData->snd_total_volume = (uint8)(((uint32)(config.general_sound_level + 32768)) >> 9);
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
		if((spos + len2) >= (len / sizeof(Sint16))) {
			len2 = (len / sizeof(Sint16)) - spos;
		}
		if((*(pData->sound_write_pos) + len2) >= *(pData->sound_buffer_size) ) len2 = *(pData->sound_buffer_size) - *(pData->sound_write_pos);
		
		if(*(pData->sound_debug)) AGAR_DebugLog(AGAR_LOG_DEBUG, "SND:Callback,sound_write_pos=%d,spos=%d,len=%d,len2=%d",
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

void OSD::initialize_sound(int rate, int samples)
{
	std::string devname;
	int i;

	sound_rate = rate;
	sound_samples = samples;
	sound_ok = sound_started = now_mute = now_rec_sound = false;
	rec_sound_buffer_ptr = 0;
	sound_ok = sound_started = now_mute = now_rec_sound = false;
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

	snd_spec_req.format = AUDIO_S16SYS;
	snd_spec_req.channels = 2;
	snd_spec_req.freq = sound_rate;
	//snd_spec_req.samples = ((sound_rate * 100) / 1000);
	snd_spec_req.samples = samples;
	snd_spec_req.callback = &(this->audio_callback);
	snd_spec_req.userdata = (void *)&snddata;
#if defined(USE_SDL2)      
	for(i = 0; i < SDL_GetNumAudioDevices(0); i++) {
		devname = SDL_GetAudioDeviceName(i, 0);
		AGAR_DebugLog(AGAR_LOG_INFO, "Audio Device: %s", devname.c_str());
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
	AGAR_DebugLog(AGAR_LOG_INFO, "Sound OK: BufSize = %d", sound_buffer_size);
	ZeroMemory(sound_buf_ptr, sound_buffer_size * sizeof(Sint16));
#if defined(USE_SDL2)   
	SDL_PauseAudioDevice(audio_dev_id, 0);
#else   
	SDL_PauseAudio(0);
#endif   
	
	sound_ok = sound_first_half = true;
}

void OSD::release_sound()
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
	}
	if(sound_buf_ptr != NULL) free(sound_buf_ptr);
	// stop recording
	stop_rec_sound();
}

void OSD::update_sound(int* extra_frames)
{
	*extra_frames = 0;
#ifdef USE_DEBUGGER
//	if(now_debugging) {
//		return;
//	}
#endif
	now_mute = false;
	if(sound_ok) {
		uint32_t play_c, offset, size1, size2;
		Sint16 *ptr1, *ptr2;
		
		// start play
		// check current position
		play_c = sound_write_pos;
		if(sound_debug) AGAR_DebugLog(AGAR_LOG_DEBUG, "SND: Called time=%d sound_write_pos=%d\n", osd_timer.elapsed(), play_c);
		if(!sound_first_half) {
			if(play_c < (sound_buffer_size / 2)) {
				return;
			}
			offset = 0;
		} else {
			if(play_c >= (sound_buffer_size / 2)) {
				return;
			}
			offset = sound_buffer_size / 2;
		}
		//SDL_UnlockAudio();
		// sound buffer must be updated
		Sint16* sound_buffer = (Sint16 *)vm->create_sound(extra_frames);
		if(now_rec_sound) {
			// record sound
			if(sound_samples > rec_sound_buffer_ptr) {
				int samples = sound_samples - rec_sound_buffer_ptr;
				int length = samples * sizeof(uint16) * 2; // stereo
				rec_sound_fio->Fwrite(sound_buffer + rec_sound_buffer_ptr * 2, length, 1);
				rec_sound_bytes += length;
				if(now_rec_video) {
					// sync video recording
					static double frames = 0;
					static int prev_samples = -1;
#ifdef SUPPORT_VARIABLE_TIMING
					static double prev_fps = -1;
					double fps = vm->frame_rate();
					if(prev_samples != samples || prev_fps != fps) {
						prev_samples = samples;
						prev_fps = fps;
						frames = fps * (double)samples / (double)sound_rate;
					}
#else
					if(prev_samples != samples) {
						prev_samples = samples;
						frames = FRAMES_PER_SEC * (double)samples / (double)sound_rate;
					}
#endif
					rec_video_frames -= frames;
					if(rec_video_frames > 2) {
						rec_video_run_frames -= (rec_video_frames - 2);
					} else if(rec_video_frames < -2) {
						rec_video_run_frames -= (rec_video_frames + 2);
					}
//					rec_video_run_frames -= rec_video_frames;
				}
//				printf("Wrote %d samples\n", samples);
				rec_sound_buffer_ptr += samples;
				if(rec_sound_buffer_ptr >= sound_samples) rec_sound_buffer_ptr = 0;
			}
		}
		
		if(sound_buffer) {
		        int ssize;
		        int pos;
		        int pos2;
		        if(snd_apply_sem) {
					if(sound_debug) AGAR_DebugLog(AGAR_LOG_DEBUG, "SND:Pushed time=%d samples=%d\n",
												  osd_timer.elapsed(), sound_samples);
					//SDL_PauseAudioDevice(audio_dev_id, 1);
					//SDL_LockAudio();
					SDL_SemWait(*snddata.snd_apply_sem);
					ssize = sound_samples * snd_spec_presented.channels;
					//ssize = sound_buffer_size / 2;
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
					SDL_SemPost(*snddata.snd_apply_sem);
					//SDL_UnlockAudio();
					//SDL_PauseAudioDevice(audio_dev_id, 0);
			}
		}
	   
//	        SDL_PauseAudioDevice(audio_dev_id, 0);
		sound_first_half = !sound_first_half;
	}
}

void OSD::mute_sound()
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
			ZeroMemory(ptr1, size1 * sizeof(Sint16));
		}
		if(ptr2) {
			ZeroMemory(ptr2, size2 * sizeof(Sint16));
		}
		sound_data_pos = (sound_data_pos + ssize) % sound_buffer_size;
		SDL_SemPost(*snddata.snd_apply_sem);
	}
	now_mute = true;
}

void OSD::stop_sound()
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

void OSD::start_rec_sound()
{
   
	if(!now_rec_sound) {
		//LockVM();
		QDateTime nowTime = QDateTime::currentDateTime();
		QString tmps = QString::fromUtf8("Sound_Save_emu");
		tmps = tmps + QString::fromUtf8(CONFIG_NAME);
		tmps = tmps + QString::fromUtf8("_");
		tmps = tmps + nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz"));
		tmps = tmps + QString::fromUtf8(".wav");
		strncpy(sound_file_name, tmps.toUtf8().constData(), sizeof(sound_file_name));
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
			now_rec_sound = true;
		} else {
			// failed to open the wave file
			delete rec_sound_fio;
		}
		//UnlockVM();
	}
}

void OSD::stop_rec_sound()
{
		if(now_rec_sound) {
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
			wav_header.sample_rate = EndianToLittle_DWORD(sound_rate);
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
		now_rec_sound = false;
		//UnlockVM();
	}
}

void OSD::restart_rec_sound()
{
	bool tmp = now_rec_sound;
	stop_rec_sound();
	if(tmp) {
		start_rec_sound();
	}
}

