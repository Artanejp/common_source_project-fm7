/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 sound ]
*/

#include "emu.h"
#include "vm/vm.h"
#include "fileio.h"

#define DSOUND_BUFFER_SIZE (DWORD)(sound_samples * 8)
#define DSOUND_BUFFER_HALF (DWORD)(sound_samples * 4)

void EMU::initialize_sound()
{
	sound_ok = sound_started = now_mute = now_rec_sound = false;
	rec_buffer_ptr = 0;
	
	// initialize direct sound
	PCMWAVEFORMAT pcmwf;
	DSBUFFERDESC dsbd;
	WAVEFORMATEX wfex;
	
	if(FAILED(DirectSoundCreate(NULL, &lpds, NULL))) {
		return;
	}
	if(FAILED(lpds->SetCooperativeLevel(main_window_handle, DSSCL_PRIORITY))) {
		return;
	}
	
	// primary buffer
	ZeroMemory(&dsbd, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if(FAILED(lpds->CreateSoundBuffer(&dsbd, &lpdsp, NULL))) {
		return;
	}
	ZeroMemory(&wfex, sizeof(wfex));
	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nChannels = 2;
	wfex.wBitsPerSample = 16;
	wfex.nSamplesPerSec = sound_rate;
	wfex.nBlockAlign = wfex.nChannels * wfex.wBitsPerSample / 8;
	wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
	if(FAILED(lpdsp->SetFormat(&wfex))) {
		return;
	}
	
	// secondary buffer
	ZeroMemory(&pcmwf, sizeof(pcmwf));
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = 2;
	pcmwf.wBitsPerSample = 16;
	pcmwf.wf.nSamplesPerSec = sound_rate;
	pcmwf.wf.nBlockAlign = pcmwf.wf.nChannels * pcmwf.wBitsPerSample / 8;
	pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
	ZeroMemory(&dsbd, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_STICKYFOCUS | DSBCAPS_GETCURRENTPOSITION2;
	dsbd.dwBufferBytes = DSOUND_BUFFER_SIZE;
	dsbd.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;
	if(FAILED(lpds->CreateSoundBuffer(&dsbd, &lpdsb, NULL))) {
		return;
	}
	
	sound_ok = first_half = true;
}

void EMU::release_sound()
{
	// release direct sound
	if(lpdsp) {
		lpdsp->Release();
	}
	if(lpdsb) {
		lpdsb->Release();
	}
	if(lpds) {
		lpds->Release();
	}
	lpdsp = NULL;
	lpdsb = NULL;
	lpds = NULL;
	
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
		DWORD play_c, write_c, offset, size1, size2;
		WORD *ptr1, *ptr2;
		
		// start play
		if(!sound_started) {
			lpdsb->Play(0, 0, DSBPLAY_LOOPING);
			sound_started = true;
			return;
		}
		
		// check current position
		if(FAILED(lpdsb->GetCurrentPosition(&play_c, &write_c))) {
			return;
		}
		if(first_half) {
			if(play_c < DSOUND_BUFFER_HALF) {
				return;
			}
			offset = 0;
		} else {
			if(play_c >= DSOUND_BUFFER_HALF) {
				return;
			}
			offset = DSOUND_BUFFER_HALF;
		}
		
		// sound buffer must be updated
		uint16* sound_buffer = vm->create_sound(extra_frames);
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
		if(lpdsb->Lock(offset, DSOUND_BUFFER_HALF, (void **)&ptr1, &size1, (void**)&ptr2, &size2, 0) == DSERR_BUFFERLOST) {
			lpdsb->Restore();
		}
		if(sound_buffer) {
			if(ptr1) {
				CopyMemory(ptr1, sound_buffer, size1);
			}
			if(ptr2) {
				CopyMemory(ptr2, sound_buffer + size1, size2);
			}
		}
		lpdsb->Unlock(ptr1, size1, ptr2, size2);
		first_half = !first_half;
	}
}

void EMU::mute_sound()
{
	if(!now_mute && sound_ok) {
		// check current position
		DWORD size1, size2;
		WORD *ptr1, *ptr2;
		
		if(lpdsb->Lock(0, DSOUND_BUFFER_SIZE, (void **)&ptr1, &size1, (void**)&ptr2, &size2, 0) == DSERR_BUFFERLOST) {
			lpdsb->Restore();
		}
		if(ptr1) {
			ZeroMemory(ptr1, size1);
		}
		if(ptr2) {
			ZeroMemory(ptr2, size2);
		}
		lpdsb->Unlock(ptr1, size1, ptr2, size2);
	}
	now_mute = true;
}

void EMU::start_rec_sound()
{
	if(!now_rec_sound) {
		// create file name
		SYSTEMTIME sTime;
		GetLocalTime(&sTime);
		
		_stprintf_s(sound_file_name, _MAX_PATH, _T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.wav"), sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
		
		// create wave file
		rec = new FILEIO();
		if(rec->Fopen(bios_path(sound_file_name), FILEIO_WRITE_BINARY)) {
			// write dummy wave header
			wav_header_t wav_header;
			wav_chunk_t wav_chunk;
			memset(&wav_header, 0, sizeof(wav_header));
			memset(&wav_chunk, 0, sizeof(wav_chunk));
			rec->Fwrite(&wav_header, sizeof(wav_header), 1);
			rec->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
			
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
			FILEIO::Remove(sound_file_name);
		} else {
			// update wave header
			wav_header_t wav_header;
			wav_chunk_t wav_chunk;
			
			memcpy(wav_header.riff_chunk.id, "RIFF", 4);
			wav_header.riff_chunk.size = rec_bytes + sizeof(wav_header) + sizeof(wav_chunk) - 8;
			memcpy(wav_header.wave, "WAVE", 4);
			memcpy(wav_header.fmt_chunk.id, "fmt ", 4);
			wav_header.fmt_chunk.size = 16;
			wav_header.format_id = 1;
			wav_header.channels = 2;
			wav_header.sample_bits = 16;
			wav_header.sample_rate = sound_rate;
			wav_header.block_size = wav_header.channels * wav_header.sample_bits / 8;
			wav_header.data_speed = wav_header.sample_rate * wav_header.block_size;
			
			memcpy(wav_chunk.id, "data", 4);
			wav_chunk.size = rec_bytes;
			
			rec->Fseek(0, FILEIO_SEEK_SET);
			rec->Fwrite(&wav_header, sizeof(wav_header), 1);
			rec->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
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

