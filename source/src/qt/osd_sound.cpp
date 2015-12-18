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

extern "C" {
	static int uBufSize;
	static int nSndDataLen, nSndDataPos, nSndWritePos;
	static bool bSndExit;
	static bool bSoundDebug;
	static SDL_sem *pSndApplySem;
	static Sint16 *pSoundBuf;
	Uint8 iTotalVolume;
#if defined(USE_SDL2)   
	static SDL_AudioDeviceID nAudioDevid;
#else
	static int nAudioDevid;
#endif
	static SDL_AudioSpec SndSpecReq, SndSpecPresented;
}

void AudioCallbackSDL(void *udata, Uint8 *stream, int len)
{
	int pos;
	int blen = len;
	int len2 = len;
	int channels = 2;
	int spos;
	struct timespec req, remain;
	Uint8 *p;
	Uint8 *s;
	int writepos;
	int sndlen;
	
	sdl_snddata_t *pData = (sdl_snddata_t *)udata;
	//   printf("Called SND: %d %08x len = %d\n", SDL_GetTicks(), pData, len);
	//if(pData == NULL) return;
	
	req.tv_sec = 0;
	req.tv_nsec = 500 * 1000; //  0.5ms
	
	if(len <= 0) return;
	spos = 0;
	memset(stream, 0x00, len);
	do {
		SDL_SemWait(*pData->pSndApplySem);
		if(config.general_sound_level < -32768) config.general_sound_level = -32768;
		if(config.general_sound_level > 32767)  config.general_sound_level = 32767;
		iTotalVolume = (uint8)(((uint32)(config.general_sound_level + 32768)) >> 9);
		sndlen = nSndDataLen;
		if(uBufSize  <= nSndWritePos) { // Wrap
			nSndWritePos = 0;
		}
		len2 = uBufSize - nSndWritePos;
		if(bSndExit) {
			SDL_SemPost(*pData->pSndApplySem);
			return;
		}
		if(len2 >= sndlen) len2 = sndlen;  // Okay
		if((spos + len2) >= (len / sizeof(Sint16))) {
			len2 = (len / sizeof(Sint16)) - spos;
		}
		if((nSndWritePos + len2) >= uBufSize ) len2 = uBufSize - nSndWritePos;
		
		if((len2 > 0) && (sndlen > 0)){
			writepos = nSndWritePos;
			p = (Uint8 *)pSoundBuf;
			SDL_SemPost(*pData->pSndApplySem);
			p = &p[writepos * 2];
			s = &stream[spos * 2];
			SDL_MixAudio(s, (Uint8 *)p, len2 * 2, iTotalVolume);
			if(bSoundDebug) AGAR_DebugLog(AGAR_LOG_DEBUG, "SND:Time=%d,Callback,nSndWritePos=%d,spos=%d,len=%d,len2=%d", SDL_GetTicks(), 
				  writepos, spos, len, len2);
			SDL_SemWait(*pData->pSndApplySem);
			nSndDataLen -= len2;
			if(nSndDataLen <= 0) nSndDataLen = 0;
			nSndWritePos += len2;
			SDL_SemPost(*pData->pSndApplySem);
		} else {
			len2 = 0;
			SDL_SemPost(*pData->pSndApplySem);
			if(spos >= (len / 2)) return;
			while(nSndDataLen <= 0) {
#if defined(Q_OS_WIN32)
				SDL_Delay(1);
#else
				nanosleep(&req, &remain); // Wait 500uS
#endif
				if(bSndExit) return;
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
	nSndWritePos = 0;
	nSndDataLen = 0;
	uBufSize = 0;
	bSndExit = false;
	bSoundDebug = false;
	pSoundBuf = NULL;
	pSndApplySem = NULL;
	// initialize direct sound

	iTotalVolume = 127;
   
	snddata.pSoundBuf = &pSoundBuf;
	snddata.uBufSize = &uBufSize;
	snddata.nSndWritePos = &nSndWritePos;
	snddata.nSndDataLen = &nSndDataLen;
	snddata.pSndApplySem = &pSndApplySem;
	snddata.iTotalVolume = &iTotalVolume;
	snddata.bSndExit = &bSndExit;
	snddata.bSoundDebug = &bSoundDebug;

	SndSpecReq.format = AUDIO_S16SYS;
	SndSpecReq.channels = 2;
	SndSpecReq.freq = sound_rate;
	SndSpecReq.samples = ((sound_rate * 20) / 1000);
	SndSpecReq.callback = AudioCallbackSDL;
	SndSpecReq.userdata = (void *)&snddata;
#if defined(USE_SDL2)      
	for(i = 0; i < SDL_GetNumAudioDevices(0); i++) {
		devname = SDL_GetAudioDeviceName(i, 0);
		AGAR_DebugLog(AGAR_LOG_INFO, "Audio Device: %s", devname.c_str());
	}
#endif   
	SDL_OpenAudio(&SndSpecReq, &SndSpecPresented);
	nAudioDevid = 1;
   
	// secondary buffer
	uBufSize = (100 * SndSpecPresented.freq * SndSpecPresented.channels * 2) / 1000;
	//uBufSize = sound_samples * 2;
	pSoundBuf = (Sint16 *)malloc(uBufSize * sizeof(Sint16)); 
	if(pSoundBuf == NULL) {
#if defined(USE_SDL2)   	   
		SDL_CloseAudioDevice(nAudioDevid);
#else	   
		SDL_CloseAudio();
#endif	   
		return;
	}
	pSndApplySem = SDL_CreateSemaphore(1);
	if(pSndApplySem == NULL) {
		free(pSoundBuf);
		pSoundBuf = NULL;
		return;
	}
	AGAR_DebugLog(AGAR_LOG_INFO, "Sound OK: BufSize = %d", uBufSize);
	ZeroMemory(pSoundBuf, uBufSize * sizeof(Sint16));
#if defined(USE_SDL2)   
	SDL_PauseAudioDevice(nAudioDevid, 0);
#else   
	SDL_PauseAudio(0);
#endif   
	
	sound_ok = sound_first_half = true;
}

void OSD::release_sound()
{
	// release SDL sound
	bSndExit = TRUE;
#if defined(USE_SDL2)   
	SDL_CloseAudioDevice(nAudioDevid);
#else   
	SDL_CloseAudio();
#endif   
	if(pSndApplySem != NULL) {
		SDL_DestroySemaphore(pSndApplySem);
	}
	if(pSoundBuf != NULL) free(pSoundBuf);
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
		play_c = nSndWritePos;
		if(!sound_first_half) {
			if(play_c < (uBufSize / 2)) {
				return;
			}
			offset = 0;
		} else {
			if(play_c >= (uBufSize / 2)) {
				return;
			}
			offset = uBufSize / 2;
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
			}
			rec_sound_buffer_ptr = 0;
		}
		if(sound_buffer) {
		        int ssize;
		        int pos;
		        int pos2;
		        if(pSndApplySem) {
				SDL_SemWait(*snddata.pSndApplySem);
				ssize = sound_samples * SndSpecPresented.channels;
			        pos = nSndDataPos;
			        pos2 = pos + ssize;
		        	ptr1 = &pSoundBuf[pos];
			        if(pos2 >= uBufSize) {
					size1 = uBufSize  - pos;
					size2 = pos2 - uBufSize;
					ptr2 = &pSoundBuf[0];
				} else {
					size1 = ssize;
					size2 = 0;
					ptr2 = NULL;
				}
				if(ptr1) {
					memcpy(ptr1, sound_buffer, size1 * sizeof(Sint16));
				}
				if(ptr2) {
					memcpy(ptr2, sound_buffer + size1, size2 * sizeof(Sint16));
				}
				nSndDataLen = nSndDataLen + ssize;
				if(nSndDataLen >= uBufSize) nSndDataLen = uBufSize;
				nSndDataPos = nSndDataPos + ssize;
				if(nSndDataPos >= uBufSize) nSndDataPos = nSndDataPos - uBufSize;
				SDL_SemPost(*snddata.pSndApplySem);
			}
//		        SDL_PauseAudioDevice(nAudioDevid, 0);
		}
	   
//	        SDL_PauseAudioDevice(nAudioDevid, 0);
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
		SDL_SemWait(*snddata.pSndApplySem);
		ssize = uBufSize / 2;
		pos = nSndDataPos;
		pos2 = pos + ssize;
		ptr1 = &pSoundBuf[pos];
		if(pos2 >= uBufSize) {
			size1 = uBufSize - pos;
			size2 = pos2 - uBufSize;
			ptr2 = &pSoundBuf[0];
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
		nSndDataPos = (nSndDataPos + ssize) % uBufSize;
		SDL_SemPost(*snddata.pSndApplySem);
	}
	now_mute = true;
}

void OSD::stop_sound()
{
	if(sound_ok && sound_started) {
#if defined(USE_SDL2)   
		SDL_PauseAudioDevice(nAudioDevid, 1);
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
		QString tmps = QString::fromUtf8("Screen_Save_emu");
		tmps = tmps + QString::fromUtf8(CONFIG_NAME);
		tmps = tmps + QString::fromUtf8("_");
		tmps = tmps + nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz"));
		tmps = tmps + QString::fromUtf8(".wav");
		strncpy(sound_file_name, tmps.toUtf8().constData(), sizeof(sound_file_name));
		// create wave file
		rec_sound_fio = new FILEIO();
		if(rec_sound_fio->Fopen(bios_path(sound_file_name), FILEIO_WRITE_BINARY)) {
			// write dummy wave header
			wav_header_t header;
			memset(&header, 0, sizeof(wav_header_t));
			rec_sound_fio->Fwrite(&header, sizeof(wav_header_t), 1);
			rec_sound_bytes = 0;
			rec_sound_buffer_ptr = vm->sound_buffer_ptr();
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
			
			memcpy(wav_header.fmt_chunk.id, "data", 4);
			wav_header.fmt_chunk.size = EndianToLittle_DWORD(rec_sound_bytes);

			rec_sound_fio->Fseek(0, FILEIO_SEEK_SET);
			rec_sound_fio->Fwrite(&wav_header, sizeof(wav_header_t), 1);
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

