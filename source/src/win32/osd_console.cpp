/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 console ]
*/

#include "osd.h"

BOOL WINAPI ctrl_c_handler(DWORD type)
{
	return TRUE;
}

void OSD::open_console(_TCHAR* title)
{
	AllocConsole();
	SetConsoleTitle(title);
	SetConsoleCtrlHandler(ctrl_c_handler, TRUE);
	RemoveMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);
	
	hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	
	COORD coord;
	coord.X = 80;
	coord.Y = 4000;
	
	SetConsoleScreenBufferSize(hStdOut, coord);
	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}

void OSD::close_console()
{
	SetConsoleCtrlHandler(ctrl_c_handler, FALSE);
	FreeConsole();
}

unsigned int OSD::get_console_code_page()
{
	return GetConsoleCP();
}

bool OSD::is_console_active()
{
	HWND hWnd = GetForegroundWindow();
	return (hWnd != NULL && hWnd == FindWindow("ConsoleWindowClass", NULL));
}

void OSD::set_console_text_attribute(unsigned short attr)
{
	SetConsoleTextAttribute(hStdOut, attr);
}

void OSD::write_console(_TCHAR* buffer, unsigned int length)
{
	DWORD dwWritten;
	WriteConsole(hStdOut, buffer, length, &dwWritten, NULL);
}

int OSD::read_console_input(_TCHAR* buffer)
{
	INPUT_RECORD ir[16];
	DWORD dwRead;
	int count = 0;
	
	if(ReadConsoleInput(hStdIn, ir, 16, &dwRead)) {
		for(unsigned int i = 0; i < dwRead; i++) {
#ifdef _UNICODE
			if((ir[i].EventType & KEY_EVENT) && ir[i].Event.KeyEvent.bKeyDown && ir[i].Event.KeyEvent.uChar.UnicodeChar) {
				buffer[count++] = ir[i].Event.KeyEvent.uChar.UnicodeChar;
			}
#else
			if((ir[i].EventType & KEY_EVENT) && ir[i].Event.KeyEvent.bKeyDown && ir[i].Event.KeyEvent.uChar.AsciiChar) {
				buffer[count++] = ir[i].Event.KeyEvent.uChar.AsciiChar;
			}
#endif
		}
	}
	return count;
}

