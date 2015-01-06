/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
        Port to Agar : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -

	[ win32 main ] -> [ agar main ]
*/

#ifndef _AGAR_MAIN_H_
#define _AGAR_MAIN_H_

#include <agar/core.h>
#include <agar/gui.h>
#include <string>
#include <vector>

#include "agar_sdlview.h"
#include "agar_logger.h"
#include "sdl_cpuid.h"
#include "simd_types.h"
#include "emu.h"

extern EMU* emu;
#ifdef USE_BUTTON
#define MAX_FONT_SIZE 32
extern AG_Font   *hFont[MAX_FONT_SIZE];
extern AG_Widget *hButton[MAX_BUTTONS];
extern AG_Event  *buttonWndProc[MAX_BUTTONS];
#endif

// menu
extern AG_GLView *hGLView;
extern AGAR_SDLView *hSDLView;
extern AG_Widget *hScreenWidget;
extern AG_Menu *hMenu; // Global Variable.
extern AG_Window *hWindow;
extern std::string cpp_homedir;
extern std::string cpp_confdir;
extern std::string my_procname;
extern std::string cpp_simdtype;
extern std::string sAG_Driver;
extern bool now_menuloop;

extern const int screen_mode_width[];
extern const int screen_mode_height[];
extern bool bRunEmuThread;
extern void set_window(AG_Widget * hWnd, int mode);


// dialog
#ifdef USE_CART1
extern void open_cart_dialog(AG_Widget *hWnd, int drv);
#endif
#ifdef USE_FD1
extern void open_disk_dialog(AG_Widget * hWnd, int drv);
extern void open_disk(int drv, _TCHAR* path, int bank);
extern void close_disk(int drv);
#endif
#ifdef USE_QD1
extern void open_quickdisk_dialog(AG_Widget * hWnd, int drv);
#endif
#ifdef USE_TAPE
extern void open_tape_dialog(AG_Widget * hWnd, bool play);
#endif
#ifdef USE_LASER_DISC
extern void open_laser_disc_dialog(AG_Widget * hWnd);
#endif
#ifdef USE_BINARY_FILE1
extern void open_binary_dialog(AG_Widget * hWnd, int drv, bool load);
#endif

#if defined(USE_CART1) || defined(USE_FD1) || defined(USE_TAPE) || defined(USE_BINARY_FILE1)
#define SUPPORT_DRAG_DROP
#endif
#ifdef SUPPORT_DRAG_DROP
extern void open_any_file(_TCHAR* path);
#endif

#endif // END

