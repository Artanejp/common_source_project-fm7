/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	[ win32 dependent ]
*/

#include "osd.h"

void OSD::initialize(int rate, int samples)
{
	// get module path
	_TCHAR tmp_path[_MAX_PATH], *ptr;
	GetModuleFileName(NULL, tmp_path, _MAX_PATH);
	GetFullPathName(tmp_path, _MAX_PATH, app_path, &ptr);
	*ptr = _T('\0');
	
	// check os version
	OSVERSIONINFO os_info;
	os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&os_info);
	vista_or_later = (os_info.dwPlatformId == 2 && os_info.dwMajorVersion >= 6);
	
#ifdef ONE_BOARD_MICRO_COMPUTER
	GdiplusStartup(&gdiToken, &gdiSI, NULL);
#endif
	initialize_input();
	initialize_screen();
	initialize_sound(rate, samples);
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	CoInitialize(NULL);
	initialize_video();
#endif
	initialize_printer();
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
	release_printer();
#ifdef USE_SOCKET
	release_socket();
#endif
#ifdef ONE_BOARD_MICRO_COMPUTER
	GdiplusShutdown(gdiToken);
#endif
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

_TCHAR* OSD::bios_path(const _TCHAR* file_name)
{
	static _TCHAR file_path[_MAX_PATH];
	my_stprintf_s(file_path, _MAX_PATH, _T("%s%s"), app_path, file_name);
	return file_path;
}

void OSD::get_host_time(cur_time_t* time)
{
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);
	time->year = sTime.wYear;
	time->month = sTime.wMonth;
	time->day = sTime.wDay;
	time->day_of_week = sTime.wDayOfWeek;
	time->hour = sTime.wHour;
	time->minute = sTime.wMinute;
	time->second = sTime.wSecond;
}

void OSD::sleep(uint32 ms)
{
	Sleep(ms);
}

void OSD::create_date_file_name(_TCHAR *name, int length, _TCHAR *extension)
{
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);
	my_stprintf_s(name, length, _T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.%s"), sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond, extension);
}

