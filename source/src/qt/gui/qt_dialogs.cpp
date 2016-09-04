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
//#include "emu.h"
#include "qt_main.h"
#include "qt_dialogs.h"

//#include "csp_logger.h"
#include "commonclasses.h"
//#include "menuclasses.h"

void CSP_DiskParams::_open_disk(QString s)
{
	int d = getDrive();
	//int n = d + CSP_LOG_TYPE_VFILE_FLOPPY;
	//csp_logger->debug_log(CSP_LOG_INFO, n, "Try to open media image: %s", s.toLocal8Bit().constData());
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

