/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ common header ]
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#include "./types/system_dep.h"

// include common header files
#ifdef SUPPORT_TCHAR_TYPE
	#include <tchar.h>
#endif
#ifdef SUPPORT_CPLUSPLUS_11
	#include <stdint.h>
	#include <valarray>
	#include <algorithm>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#include <math.h>
	#if _MSC_VER < 1920
		#include <typeinfo.h>
	#else
		#include <vcruntime_typeinfo.h>
	#endif
#else
#include <typeinfo>
#endif
#include <assert.h>
#include <errno.h>

// include environment/language dependent header files
#ifdef _WIN32
	#include <windows.h>
	#include <windowsx.h>
	#include <winuser.h>
	#include <mmsystem.h>
	#include <process.h>
#endif
#ifdef __GNUC__
	#include <stdarg.h>
#endif
#ifdef _USE_QT
	#ifdef _USE_QT5
		#include <QString>
		#include <QFile>
		#include <QtEndian>
		#if defined(__MINGW32__) || (__MINGW64__)
			#include <windows.h>
			#include <winbase.h>
		#endif
	#else
		#include <QtCore/QString>
		#include <QtCore/QFile>
	#endif
	#include <sys/param.h>
#endif

#include "./types/system_endians.h"

/*!
  @todo will move to another directory.
*/
#include "./types/optimizer_utils.h"

#include "./types/scrntype_t.h"
#include "./types/pair16_t.h"
#include "./types/pair32_t.h"
#include "./types/pair64_t.h"

//#include "./types/util_endians.h"

// string
/*!
  @todo will move to another directory.
*/
#include "./types/util_strings.h"

// Moved around config read/write to util_configwrapper.h .
//#include "./types/util_configwrapper.h"

/*!
  @todo will move to another directory.
*/
#include "./types/util_video.h"
/*!
 * @note You should include types/util_sound.h if you need to mix sounds.
 * see that header.
 */
//#include "./types/util_sound.h"

#include "./types/util_system.h"
#include "./types/util_disasm.h"

// time
#define LEAP_YEAR(y)	(((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

#define dll_cur_time_t DLL_PREFIX_I struct cur_time_s

typedef struct cur_time_s {
	int year, month, day, day_of_week, hour, minute, second;
	bool initialized;
	DLL_PREFIX cur_time_s()
	{
		initialized = false;
	}
	void DLL_PREFIX increment();
	void DLL_PREFIX update_year();
	void DLL_PREFIX update_day_of_week();
	bool DLL_PREFIX process_state(void *f, bool loading);
} cur_time_t;

void DLL_PREFIX get_host_time(cur_time_t* cur_time);

#endif
