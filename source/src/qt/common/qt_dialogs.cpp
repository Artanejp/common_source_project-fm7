/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.01.07

	[Qt dialogs]
*/

#include <stdio.h>
#include <string>
#include <vector>
#include "common.h"
#include "emu.h"
#include "qt_main.h"
#include "qt_dialogs.h"

#include "agar_logger.h"
#include "commonclasses.h"
#include "menuclasses.h"

void CSP_DiskParams::_open_disk(QString s)
{
	int d = getDrive();
	AGAR_DebugLog(AGAR_LOG_INFO, "Try to open media image: %s", s.toLocal8Bit().constData());
	AGAR_DebugLog(AGAR_LOG_INFO, "Drive = %d\n", d);
	emit do_open_disk(d, s);
}

void CSP_DiskParams::_open_cart(QString s)
{
	int d = getDrive();
	emit sig_open_cart(d, s);
}
void CSP_DiskParams::_open_cmt(QString s)
{
	emit do_open_cmt(play, s);
}

void CSP_DiskParams::_open_binary(QString s) {
	emit sig_open_binary_file(drive, s, play);
}

// Will move separate file, around drag'n drop.
void open_any_file(_TCHAR* path)
{
#if defined(USE_CART1)
	if(check_file_extension(path, _T(".rom")) || 
	   check_file_extension(path, _T(".bin")) || 
	   check_file_extension(path, _T(".gg" )) || 
	   check_file_extension(path, _T(".col")) || 
	   check_file_extension(path, _T(".sms")) || 
	   check_file_extension(path, _T(".60" )) || 
	   check_file_extension(path, _T(".pce"))) {
		UPDATE_HISTORY(path, config.recent_cart_path[0]);
		strcpy(config.initial_cart_dir, get_parent_dir(path));
		emu->open_cart(0, path);
		return;
	}
#endif
#if defined(USE_FD1)
	if(check_file_extension(path, _T(".d88")) || 
	   check_file_extension(path, _T(".d77")) || 
	   check_file_extension(path, _T(".td0")) || 
	   check_file_extension(path, _T(".imd")) || 
	   check_file_extension(path, _T(".dsk")) || 
	   check_file_extension(path, _T(".fdi")) || 
	   check_file_extension(path, _T(".hdm")) || 
	   check_file_extension(path, _T(".tfd")) || 
	   check_file_extension(path, _T(".xdf")) || 
	   check_file_extension(path, _T(".2d" )) || 
	   check_file_extension(path, _T(".sf7"))) {
		UPDATE_HISTORY(path, config.recent_disk_path[0]);
		strcpy(config.initial_disk_dir, get_parent_dir(path));
		open_disk(0, path, 0);
		return;
	}
#endif
#if defined(USE_TAPE)
	if(check_file_extension(path, _T(".wav")) || 
	   check_file_extension(path, _T(".cas")) || 
	   check_file_extension(path, _T(".p6" )) || 
	   check_file_extension(path, _T(".cmt")) || 
	   check_file_extension(path, _T(".n80")) || 
	   check_file_extension(path, _T(".t88")) || 
	   check_file_extension(path, _T(".mzt")) || 
	   check_file_extension(path, _T(".m12")) || 
	   check_file_extension(path, _T(".mti")) || 
	   check_file_extension(path, _T(".mtw")) || 
	   check_file_extension(path, _T(".tap"))) {
		UPDATE_HISTORY(path, config.recent_tape_path);
		strcpy(config.initial_tape_dir, get_parent_dir(path));
		emu->play_tape(path);
		return;
	}
#endif
#if defined(USE_BINARY_FILE1)
	if(check_file_extension(path, _T(".ram")) || 
	   check_file_extension(path, _T(".bin"))) {
		UPDATE_HISTORY(path, config.recent_binary_path[0]);
		strcpy(config.initial_binary_dir, get_parent_dir(path));
		emu->load_binary(0, path);
		return;
	}
#endif
}

