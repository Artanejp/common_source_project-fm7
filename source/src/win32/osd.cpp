/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	[ win32 dependent ]
*/

#include "osd.h"

void OSD::initialize(int rate, int samples)
{
	// check os version
	OSVERSIONINFO os_info;
	os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&os_info);
	vista_or_later = (os_info.dwPlatformId == 2 && (os_info.dwMajorVersion > 6 || (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion >= 2)));
	
	GdiplusStartup(&gdiToken, &gdiSI, NULL);
	initialize_input();
	initialize_screen();
	initialize_sound(rate, samples);
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	CoInitialize(NULL);
	initialize_video();
#endif
#ifdef USE_SOCKET
	initialize_socket();
#endif
}

void OSD::release()
{
	release_input();
	release_screen();
	release_sound();
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	release_video();
	CoUninitialize();
#endif
#ifdef USE_SOCKET
	release_socket();
#endif
	GdiplusShutdown(gdiToken);
}

void OSD::power_off()
{
	PostMessage(main_window_handle, WM_CLOSE, 0, 0L);
}

void OSD::suspend()
{
#ifdef USE_MOVIE_PLAYER
	if(now_movie_play && !now_movie_pause) {
		pause_movie();
		now_movie_pause = false;
	}
#endif
	mute_sound();
}

void OSD::restore()
{
#ifdef USE_MOVIE_PLAYER
	if(now_movie_play && !now_movie_pause) {
		play_movie();
	}
#endif
}

void OSD::lock_vm()
{
	lock_count++;
}

void OSD::unlock_vm()
{
	if(--lock_count <= 0) {
		force_unlock_vm();
	}
}

void OSD::force_unlock_vm()
{
	lock_count = 0;
}

void OSD::sleep(uint32 ms)
{
	Sleep(ms);
}

