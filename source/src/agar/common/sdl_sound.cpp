/*
 * SDL Sound Handler
 * (C) 2014-12-30 K.Ohta <whatisthis.sowhat@gmail.com>
 */

#include <SDL/SDL.h>
#include <ctime>
#include "emu.h"
#include "vm/vm.h"
#include "fileio.h"

typedef struct {
	DWORD dwRIFF;
	DWORD dwFileSize;
	DWORD dwWAVE;
	DWORD dwfmt_;
	DWORD dwFormatSize;
	WORD wFormatTag;
	WORD wChannels;
	DWORD dwSamplesPerSec;
	DWORD dwAvgBytesPerSec;
	WORD wBlockAlign;
	WORD wBitsPerSample;
	DWORD dwdata;
	DWORD dwDataLength;
} wavheader_t;




void AudioCallbackSDL(void *udata, Uint8 *stream, int len)
{
   int pos;
   int blen = len;
   int len2 = 0;
   int channels = 2;
   int spos;
   struct timespec req, remain;
   Uint8 *p;
   Uint8 *s;
   
   sdl_snddata_t *pData = (sdl_snddata_t *)udata;
   if(pData == NULL) return;

   req.tv_sec = 0;
   req.tv_nsec = 4000 * 1000; //  0.1ms
   
   if(len <= 0) return;
   spos = 0;
   memset(stream, 0x00, len);
   do {
       if(*pData->uBufSize <= *pData->nSndWritePos) { // Wrap
	   *pData->nSndWritePos = 0;
	}
        len2 = *pData->uBufSize - *pData->nSndWritePos;
        if(*pData->bSndExit) {
	   return;
	}
        if(len2 >= *pData->nSndDataLen) len2 = *pData->nSndDataLen;  // Okay
        if((spos + len2) >= len) {
	   len2 = len - spos;
	}
        SDL_SemWait(*pData->pSndApplySem);
        if((len2 > 0) && (*pData->nSndDataLen > 0)){
	   p = (Uint8 *)(*pData->pSoundBuf);
	   p = &p[*pData->nSndWritePos];
	   s = &stream[spos];
	   SDL_MixAudio(s, p, len2, *pData->iTotalVolume);
	   if(*pData->bSoundDebug) printf("SND:Time=%d,Callback,nSndWritePos=%d,spos=%d,len=%d,len2=%d\n", SDL_GetTicks(), 
				  *pData->nSndWritePos, spos, len, len2);
	   *pData->nSndDataLen -= len2;
	   if(*pData->nSndDataLen <= 0) *pData->nSndDataLen = 0;
	   SDL_SemPost(*pData->pSndApplySem);
	} else {
	   len2 = 0;
	   SDL_SemPost(*pData->pSndApplySem);
	   if(spos >= len) return;
//	   while(nSndDataLen <= 0) {
	   nanosleep(&req, &remain); // Wait 500uS
	   if(*pData->bSndExit) return;
//	   }
	}
        *pData->nSndWritePos += len2;
        spos += len2;
   } while(spos < len);
   
}



void EMU::initialize_sound()
{
	sound_ok = sound_started = now_mute = now_rec_sound = false;
	rec_buffer_ptr = 0;
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
        SndSpecReq.samples = 1024;
        SndSpecReq.callback = AudioCallbackSDL;
        SndSpecReq.userdata = (void *)&snddata;
        SDL_OpenAudio(&SndSpecReq, &SndSpecPresented);
	
	// secondary buffer
	uBufSize = (100 * SndSpecPresented.freq * SndSpecPresented.channels * 2) / 1000;
        pSoundBuf = (Sint16 *)malloc(uBufSize * sizeof(Sint16)); 
        if(pSoundBuf == NULL) {
	   SDL_CloseAudio();
	   return;
	}
        pSndApplySem = SDL_CreateSemaphore(1);
        if(pSndApplySem == NULL) {
	   free(pSoundBuf);
	   pSoundBuf = NULL;
	   return;
	}
   
        ZeroMemory(pSoundBuf, uBufSize * sizeof(Sint16));
        sound_ok = first_half = true;
}

void EMU::release_sound()
{
	// release direct sound
	bSndExit = TRUE;
        SDL_CloseAudio();
        if(pSndApplySem != NULL) {
	   SDL_SemWait(pSndApplySem);
	   SDL_SemPost(pSndApplySem);
	   SDL_DestroySemaphore(pSndApplySem);
	}
        if(pSoundBuf != NULL) free(pSoundBuf);
	// stop recording
	stop_rec_sound();
}

void EMU::update_sound(int* extra_frames)
{
	*extra_frames = 0;
#ifdef USE_DEBUGGER
//	if(now_debugging) {
//		return;
//	}
#endif
	now_mute = false;
	
	if(sound_ok) {
		DWORD play_c, offset, size1, size2;
		Sint16 *ptr1, *ptr2;
		
		// start play
		if(!sound_started) {
		        SDL_PauseAudio(0);
			sound_started = true;
			return;
		}
	        SDL_LockAudio();
		// check current position
		play_c = nSndWritePos * sizeof(Sint16);
		if(first_half) {
			if(play_c < (uBufSize / 2)) {
				SDL_UnlockAudio();
				return;
			}
			offset = 0;
		} else {
			if(play_c >= (uBufSize / 2)) {
				SDL_UnlockAudio();
				return;
			}
			offset = uBufSize / 2;
		}
	        SDL_UnlockAudio();
		
	        // sound buffer must be updated
		Sint16* sound_buffer = (Sint16 *)vm->create_sound(extra_frames);
		if(now_rec_sound) {
			// record sound
			if(sound_samples > rec_buffer_ptr) {
				int samples = sound_samples - rec_buffer_ptr;
				int length = samples * sizeof(uint16) * 2; // stereo
				rec->Fwrite(sound_buffer + rec_buffer_ptr * 2, length, 1);
				rec_bytes += length;
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
			rec_buffer_ptr = 0;
		}
		if(sound_buffer) {
		        int ssize;
		        int pos;
		        int pos2;
		        SDL_LockAudio();
		        if(pSndApplySem) {
				SDL_SemWait(pSndApplySem);
				ssize = sound_samples * SndSpecPresented.channels;
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
					CopyMemory(ptr1, sound_buffer, size1 * sizeof(Sint16));
				}
				if(ptr2) {
					CopyMemory(ptr2, sound_buffer + size1, size2 * sizeof(Sint16));
				}
		        	nSndDataPos = (nSndDataPos + ssize) % uBufSize;
				SDL_SemPost(pSndApplySem);
			}
			SDL_UnlockAudio();
		}
	        SDL_PauseAudio(0);
		first_half = !first_half;
	}
}

void EMU::mute_sound()
{
	if(!now_mute && sound_ok) {
		// check current position
		DWORD size1, size2;
	        
		Sint16 *ptr1, *ptr2;
		// WIP
		int ssize;
		int pos;
		int pos2;
	   	if(pSndApplySem) { 
		   	SDL_SemWait(pSndApplySem);
			SDL_LockAudio();
			ssize = sound_samples * SndSpecPresented.channels;
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
	        	SDL_UnlockAudio();
		   	SDL_SemPost(pSndApplySem);
		}
	        SDL_PauseAudio(0);
	}
	now_mute = true;
}

void EMU::start_rec_sound()
{
   

	if(!now_rec_sound) {
		// create file name
		//SYSTEMTIME sTime;
	        //GetLocalTime(&sTime);
	        std::tm *tm;
	        std::time_t tnow;
	        tnow = std::time(NULL);
	        tm = std::localtime(&tnow);
		
		//_stprintf(sound_file_name, _T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.wav"), sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
		sprintf(sound_file_name, _T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.wav"), tm->tm_year + 1900, tm->tm_mon + 1, 
			tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
		
		// create wave file
		rec = new FILEIO();
		if(rec->Fopen(bios_path(sound_file_name), FILEIO_WRITE_BINARY)) {
			// write dummy wave header
			wavheader_t header;
			memset(&header, 0, sizeof(wavheader_t));
			rec->Fwrite(&header, sizeof(wavheader_t), 1);
			rec_bytes = 0;
			rec_buffer_ptr = vm->sound_buffer_ptr();
			now_rec_sound = true;
		} else {
			// failed to open the wave file
			delete rec;
		}
	}
}

void EMU::stop_rec_sound()
{
	if(now_rec_sound) {
		if(rec_bytes == 0) {
			rec->Fclose();
			rec->Remove(sound_file_name);
		} else {
			// update wave header
			wavheader_t header;

		        header.dwRIFF = EndianToLittle_DWORD(0x46464952);
			header.dwFileSize = EndianToLittle_DWORD(rec_bytes + sizeof(wavheader_t) - 8);
			header.dwWAVE = EndianToLittle_DWORD(0x45564157);
			header.dwfmt_ = EndianToLittle_DWORD(0x20746d66);
			header.dwFormatSize = EndianToLittle_DWORD(16);
			header.wFormatTag = EndianToLittle_WORD(1);
			header.wChannels =  EndianToLittle_WORD(2);
			header.wBitsPerSample = EndianToLittle_WORD(16);
			header.dwSamplesPerSec = EndianToLittle_DWORD(sound_rate);
			header.wBlockAlign = EndianToLittle_WORD(header.wChannels * header.wBitsPerSample / 8);
			header.dwAvgBytesPerSec = EndianToLittle_DWORD(header.dwSamplesPerSec * header.wBlockAlign);
			header.dwdata = EndianToLittle_DWORD(0x61746164);
			header.dwDataLength = EndianToLittle_DWORD(rec_bytes);
			rec->Fseek(0, FILEIO_SEEK_SET);
			rec->Fwrite(&header, sizeof(wavheader_t), 1);
			rec->Fclose();
		}
		delete rec;
		now_rec_sound = false;
	}
}

void EMU::restart_rec_sound()
{
	bool tmp = now_rec_sound;
	stop_rec_sound();
	if(tmp) start_rec_sound();
}

