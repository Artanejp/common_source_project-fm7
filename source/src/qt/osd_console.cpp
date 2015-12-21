/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	[ win32 console ]
*/

#include "emu.h"
#include "fifo.h"
#include "emu_thread.h"
#include "emu.h"
#include <QString>

extern EMU *emu;
//BOOL WINAPI ctrl_c_handler(DWORD type)
//{
//	return TRUE;
//}

void OSD::do_write_inputdata(QString s)
{
	int i;
	emit sig_console_input_string(s);
}

void OSD::do_set_input_string(QString s)
{
	memset(console_string, 0x00, sizeof(console_string));
	strncpy(console_string, s.toLocal8Bit().constData(), sizeof(console_string) - 1);
}

_TCHAR *OSD::console_input_string(void)
{
	if(strlen(console_string) <= 0) return NULL;
	return console_string;
}

void OSD::clear_console_input_string(void)
{
	memset(console_string, 0x00, sizeof(console_string));
}

void OSD::open_console(_TCHAR* title)
{
	if(osd_console_opened) return;
	memset(console_string, 0x00, sizeof(console_string));
	osd_console_opened = true;
	
}

void OSD::close_console()
{
	memset(console_string, 0x00, sizeof(console_string));
	osd_console_opened = false;
}

unsigned int OSD::get_console_code_page()
{
	//return GetConsoleCP();
	return 0;
}

bool OSD::is_console_active()
{
	return 	osd_console_opened;
}

void OSD::set_console_text_attribute(unsigned short attr)
{
	//SetConsoleTextAttribute(hStdOut, attr);
}

void OSD::write_console(_TCHAR* buffer, unsigned int length)
{
	QString s = QString::fromLocal8Bit(buffer, length);
	emit sig_put_string_debugger(s);
}

int OSD::read_console_input(_TCHAR* buffer)
{
	int i;
	int count = 0;
	return count;
}

void OSD::do_close_debugger_console()
{
	emit sig_debugger_finished(); // It's dirty...
}

void OSD::do_close_debugger_thread()
{
#if defined(USE_DEBUGGER)
	//if(emu->debugger_thread_param.request_terminate == true) {
		emit sig_debugger_finished();
		//}
	//emu->close_debugger();
#endif	
}
