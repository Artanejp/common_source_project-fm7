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
	QByteArray p = s.toUtf8();
	//printf("%s\n", p.constData());
	//lock_vm();
	for(i = 0; i < p.length(); i++){
		osd_console_input->write(p.at(i));
		printf("%c", p.at(i));
	}
	//unlock_vm();
}

void OSD::open_console(_TCHAR* title)
{

	if(osd_console_opened || (osd_console_input != NULL)) return;
	osd_console_opened = true;
	osd_console_input = new FIFO(65536);
}

void OSD::close_console()
{
	osd_console_input->release();
	delete osd_console_input;
	osd_console_input = NULL;
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
	int count;
	//lock_vm();
	count = osd_console_input->count();
	for(i = 0; i < count; i++) {
		buffer[i] = (_TCHAR)osd_console_input->read();
	}
	//unlock_vm();
	return count;
}

void OSD::do_close_debugger_console()
{
	emit sig_debugger_finished(); // It's dirty...
}

void OSD::do_close_debugger_thread()
{
	emu->close_debugger();
}
