/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 console ]
*/

#include "osd.h"
#include "../res/resource.h"

void OSD::initialize_console()
{
	console_count = 0;
}

void OSD::release_console()
{
	close_console();
}

BOOL WINAPI ctrl_c_handler(DWORD type)
{
	return TRUE;
}

void OSD::open_console(int width, int height, const _TCHAR* title)
{
	int console_width = (width > 0) ? width : 120;
	int console_height = (height > 0) ? height : 30;
	int buffer_height = 9001;
	
	if(console_count++ == 0) {
		#define SET_RECT(rect, l, t, r, b) { \
			rect.Left = l; \
			rect.Top = t; \
			rect.Right = r; \
			rect.Bottom = b; \
		}
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		SMALL_RECT rect;
		COORD coord;
		
		AllocConsole();
		SetConsoleTitle(title);
		SetConsoleCtrlHandler(ctrl_c_handler, TRUE);
		RemoveMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);
		
		hStdIn = GetStdHandle(STD_INPUT_HANDLE);
		hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(hStdOut, &csbi);
		
		// window can't be bigger than buffer,
		// buffer can't be smaller than window,
		// so make a tiny window,
		// set the required buffer,
		// then set the required window
		int min_width  = min(csbi.srWindow.Right - csbi.srWindow.Left + 1, console_width);
		int min_height = min(csbi.srWindow.Bottom - csbi.srWindow.Top + 1, console_height);
		
		SET_RECT(rect, 0, csbi.srWindow.Top, min_width - 1, csbi.srWindow.Top + min_height - 1);
		SetConsoleWindowInfo(hStdOut, TRUE, &rect);
		
		coord.X = console_width;
		coord.Y = buffer_height;
		SetConsoleScreenBufferSize(hStdOut, coord);
		
		SET_RECT(rect, 0, 0, console_width - 1, console_height - 1);
		if(!SetConsoleWindowInfo(hStdOut, TRUE, &rect)) {
			SetWindowPos(GetConsoleWindow(), NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			SetConsoleWindowInfo(hStdOut, TRUE, &rect);
		}
		SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	}
}

void OSD::close_console()
{
	if(console_count > 0 && --console_count == 0) {
		SetConsoleCtrlHandler(ctrl_c_handler, FALSE);
		FreeConsole();
	}
}

unsigned int OSD::get_console_code_page()
{
	return GetConsoleCP();
}

bool OSD::is_console_active()
{
	HWND hWnd = GetForegroundWindow();
	return (hWnd != NULL && hWnd == FindWindow(_T("ConsoleWindowClass"), NULL));
}

void OSD::set_console_text_attribute(unsigned short attr)
{
	unsigned short new_attr = 0;
	
	if(attr & OSD_CONSOLE_BLUE     ) new_attr |= FOREGROUND_BLUE;
	if(attr & OSD_CONSOLE_GREEN    ) new_attr |= FOREGROUND_GREEN;
	if(attr & OSD_CONSOLE_RED      ) new_attr |= FOREGROUND_RED;
	if(attr & OSD_CONSOLE_INTENSITY) new_attr |= FOREGROUND_INTENSITY;
	
	SetConsoleTextAttribute(hStdOut, attr);
}

void OSD::write_console(const _TCHAR* buffer, unsigned int length)
{
	DWORD dwWritten;
	WriteConsole(hStdOut, buffer, length, &dwWritten, NULL);
}

int OSD::read_console_input(_TCHAR* buffer, unsigned int length)
{
	INPUT_RECORD ir[16];
	DWORD dwRead;
	unsigned int count = 0;
	
	if(ReadConsoleInput(hStdIn, ir, min(16, length), &dwRead)) {
		for(unsigned int i = 0; i < dwRead; i++) {
			if((ir[i].EventType & KEY_EVENT) && ir[i].Event.KeyEvent.bKeyDown) {
#ifdef _UNICODE
				if(ir[i].Event.KeyEvent.uChar.UnicodeChar) {
					if(count < length) {
						buffer[count++] = ir[i].Event.KeyEvent.uChar.UnicodeChar;
					}
#else
				if(ir[i].Event.KeyEvent.uChar.AsciiChar) {
					if(count < length) {
						buffer[count++] = ir[i].Event.KeyEvent.uChar.AsciiChar;
					}
#endif
				} else if(ir[i].Event.KeyEvent.wVirtualKeyCode >= 0x25 && ir[i].Event.KeyEvent.wVirtualKeyCode <= 0x28) {
					static const _TCHAR cursor[] = {_T('D'), _T('A'), _T('C'), _T('B')}; // left, up, right, down
					if(count + 2 < length) {
						buffer[count++] = 0x1b;
						buffer[count++] = 0x5b;
						buffer[count++] = cursor[ir[i].Event.KeyEvent.wVirtualKeyCode - 0x25];
					}
				}
			}
		}
	}
	return count;
}

bool OSD::is_console_key_pressed(int vk)
{
	return ((GetAsyncKeyState(vk) & 0x8000) != 0);
}

void OSD::close_debugger_console()
{
	PostMessage(main_window_handle, WM_COMMAND, ID_CLOSE_DEBUGGER, 0L);
}

