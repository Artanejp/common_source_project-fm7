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
	//if(s.empty()
	console_cmd_str.append(s);
	console_cmd_str.append(QString::fromUtf8("\n"));
}

_TCHAR *OSD::console_input_string(void)
{
	if(console_cmd_str.isEmpty()) return NULL;
	return (_TCHAR *)console_cmd_str.toUtf8().constData();
}

void OSD::clear_console_input_string(void)
{
	console_cmd_str.clear();
}

void OSD::open_console(_TCHAR* title)
{
	if(osd_console_opened) return;
	console_cmd_str.clear();
	osd_console_opened = true;
}

void OSD::close_console()
{
	console_cmd_str.clear();
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
	QString tmps;
	tmps = console_cmd_str.left(16);
	if(buffer == NULL) return 0;
	
	memset(buffer, 0x00, 16);
	
	if(tmps.isEmpty()) return 0;
	int locallen = tmps.indexOf(QString::fromUtf8("\n"));
	if(locallen >= 16) locallen = 15;
	if(locallen >= 0) {
		tmps = tmps.left(locallen + 1);
		locallen = locallen + 1;
	}

	count = tmps.length();
	if(tmps.isEmpty() || (count <= 0)) return 0; 
	if(count > 16) count = 16;
	int l = console_cmd_str.length();
	
	console_cmd_str = console_cmd_str.right(l - count);	
	strncpy(buffer, tmps.toLocal8Bit().constData(), count);

	return count;
}

// This is not recognise char code.
bool OSD::is_console_key_pressed(uint32_t ch)
{
	_TCHAR buf[17];
	if(read_console_input(buf) > 0) return true;
	return false;
}
	
void OSD::close_debugger_console()
{
	emit sig_debugger_finished(); // It's dirty...
}

void OSD::do_close_debugger_thread()
{
#if defined(USE_DEBUGGER)
		emit sig_debugger_finished();
		//}
#endif	
}
