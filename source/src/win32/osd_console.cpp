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
	use_telnet = config.use_telnet;
	telnet_closed = true;
	svr_socket = cli_socket = INVALID_SOCKET;
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
		if(use_telnet) {
			open_telnet(title);
			return;
		}
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
		if(use_telnet) {
			close_telnet();
			return;
		}
		SetConsoleCtrlHandler(ctrl_c_handler, FALSE);
		FreeConsole();
	}
}

unsigned int OSD::get_console_code_page()
{
	return GetConsoleCP();
}

void OSD::set_console_text_attribute(unsigned short attr)
{
	unsigned short new_attr = 0;
	
	if(use_telnet) {
		char buffer[32];
		
		if(attr & OSD_CONSOLE_BLUE     ) new_attr |= 4;
		if(attr & OSD_CONSOLE_GREEN    ) new_attr |= 2;
		if(attr & OSD_CONSOLE_RED      ) new_attr |= 1;
		if(attr & OSD_CONSOLE_INTENSITY) new_attr |= 8;
		
		sprintf_s(buffer, 32, "\033[%dm\033[3%dm", (new_attr >> 3) & 1, (new_attr & 7));
		send_telnet(buffer);
		return;
	}
	if(attr & OSD_CONSOLE_BLUE     ) new_attr |= FOREGROUND_BLUE;
	if(attr & OSD_CONSOLE_GREEN    ) new_attr |= FOREGROUND_GREEN;
	if(attr & OSD_CONSOLE_RED      ) new_attr |= FOREGROUND_RED;
	if(attr & OSD_CONSOLE_INTENSITY) new_attr |= FOREGROUND_INTENSITY;
	
	SetConsoleTextAttribute(hStdOut, attr);
}

void OSD::write_console(const _TCHAR* buffer, unsigned int length)
{
	if(use_telnet) {
		send_telnet(tchar_to_char(buffer));
		return;
	}
	DWORD dwWritten;
	WriteConsole(hStdOut, buffer, length, &dwWritten, NULL);
}

int OSD::read_console_input(_TCHAR* buffer, unsigned int length)
{
	if(use_telnet) {
		char temp[256];
		int len = 0;
		if(cli_socket != INVALID_SOCKET && (len = recv(cli_socket, temp, length, 0)) > 0) {
			temp[len] = '\0';
			const _TCHAR *temp_t = char_to_tchar(temp);
			for(int i = 0; i < len; i++) {
				buffer[i] = temp_t[i];
			}
			return len;
		}
		if(len == 0) {
			telnet_closed = true;
		}
		return 0;
	}
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
	if(use_telnet) {
		char temp[256];
		int len = 0;
		if(cli_socket != INVALID_SOCKET && (len = recv(cli_socket, temp, sizeof(temp), 0)) > 0) {
			for(int i = 0; i < len; i++) {
				if(temp[i] == vk) {
					return true;
				}
			}
		}
		if(len == 0) {
			telnet_closed = true;
			return true;
		}
		return false;
	}
	HWND hWnd = GetForegroundWindow();
	if(hWnd != NULL && hWnd == FindWindow(_T("ConsoleWindowClass"), NULL)) {
		return ((GetAsyncKeyState(vk) & 0x8000) != 0);
	}
	return false;
}

bool OSD::is_console_closed()
{
	if(use_telnet) {
		return telnet_closed;
	}
	return false;
}

void OSD::close_debugger_console()
{
	PostMessage(main_window_handle, WM_COMMAND, ID_CLOSE_DEBUGGER, 0L);
}

const _TCHAR *get_ttermpro_path()
{
	static _TCHAR path[MAX_PATH] = {0};
	
	if(_tgetenv(_T("ProgramFiles"))) {
		my_stprintf_s(path, MAX_PATH, _T("%s\\teraterm\\ttermpro.exe"), _tgetenv(_T("ProgramFiles")));
	}
	return(path);
}

const _TCHAR *get_ttermpro_x86_path()
{
	static _TCHAR path[MAX_PATH] = {0};
	
	if(_tgetenv(_T("ProgramFiles(x86)"))) {
		my_stprintf_s(path, MAX_PATH, _T("%s\\teraterm\\ttermpro.exe"), _tgetenv(_T("ProgramFiles(x86)")));
	}
	return(path);
}

const _TCHAR *get_putty_path()
{
	static _TCHAR path[MAX_PATH] = {0};
	
	if(_tgetenv(_T("ProgramFiles"))) {
		my_stprintf_s(path, MAX_PATH, _T("%s\\PuTTY\\putty.exe"), _tgetenv(_T("ProgramFiles")));
	}
	return(path);
}

const _TCHAR *get_putty_x86_path()
{
	static _TCHAR path[MAX_PATH] = {0};
	
	if(_tgetenv(_T("ProgramFiles(x86)"))) {
		my_stprintf_s(path, MAX_PATH, _T("%s\\PuTTY\\putty.exe"), _tgetenv(_T("ProgramFiles(x86)")));
	}
	return(path);
}

const _TCHAR *get_telnet_path()
{
	static _TCHAR path[MAX_PATH] = {0};
	
	if(_tgetenv(_T("windir")) != NULL) {
#ifdef _WIN64
		my_stprintf_s(path, MAX_PATH, _T("%s\\System32\\telnet.exe"), _tgetenv(_T("windir")));
#else
		// prevent System32 is redirected to SysWOW64 in 32bit process on Windows x64
		my_stprintf_s(path, MAX_PATH, _T("%s\\Sysnative\\telnet.exe"), _tgetenv(_T("windir")));
#endif
	}
	return(path);
}

const _TCHAR *get_telnet_x86_path()
{
	static _TCHAR path[MAX_PATH] = {0};
	
	if(_tgetenv(_T("windir")) != NULL) {
#ifdef _WIN64
		my_stprintf_s(path, MAX_PATH, _T("%s\\SysWOW64\\telnet.exe"), _tgetenv(_T("windir")));
#else
		// System32 will be redirected to SysWOW64 in 32bit process on Windows x64
		my_stprintf_s(path, MAX_PATH, _T("%s\\System32\\telnet.exe"), _tgetenv(_T("windir")));
#endif
	}
	return(path);
}

void OSD::open_telnet(const _TCHAR* title)
{
	WSADATA was_data;
	struct sockaddr_in svr_addr;
	struct sockaddr_in cli_addr;
	int cli_addr_len = sizeof(cli_addr);
	int port = 23;
	int bind_stat = SOCKET_ERROR;
	struct timeval timeout;
	
	WSAStartup(MAKEWORD(2,0), &was_data);
	
	if((svr_socket = socket(AF_INET, SOCK_STREAM, 0)) != INVALID_SOCKET) {
		memset(&svr_addr, 0, sizeof(svr_addr));
		svr_addr.sin_family = AF_INET;
		svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		
		while(port < 65536) {
			svr_addr.sin_port = htons(port);
			if((bind_stat = bind(svr_socket, (struct sockaddr *)&svr_addr, sizeof(svr_addr))) == 0) {
				break;
			} else {
				port = (port == 23) ? 49152 : (port + 1);
			}
		}
		if(bind_stat == 0) {
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			setsockopt(svr_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
			
			listen(svr_socket, 1);
			
			_TCHAR command[MAX_PATH] = {0};
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			
			if(_taccess(get_ttermpro_path(), 0) == 0) {
				my_stprintf_s(command, MAX_PATH, _T("%s localhost:%d /T=1"), get_ttermpro_path(), port);
			} else if(_taccess(get_ttermpro_x86_path(), 0) == 0) {
				my_stprintf_s(command, MAX_PATH, _T("%s localhost:%d /T=1"), get_ttermpro_x86_path(), port);
			} else if(_taccess(get_putty_path(), 0) == 0) {
				my_stprintf_s(command, MAX_PATH, _T("%s -telnet localhost %d"), get_putty_path(), port);
			} else if(_taccess(get_putty_x86_path(), 0) == 0) {
				my_stprintf_s(command, MAX_PATH, _T("%s -telnet localhost %d"), get_putty_x86_path(), port);
			} else if(_taccess(get_telnet_path(), 0) == 0) {
				my_stprintf_s(command, MAX_PATH, _T("%s -t vt100 localhost %d"), get_telnet_path(), port);
			} else if(_taccess(get_telnet_x86_path(), 0) == 0) {
				my_stprintf_s(command, MAX_PATH, _T("%s -t vt100 localhost %d"), get_telnet_x86_path(), port);
			}
			if(command[0] != _T('\0')) {
				memset(&si, 0, sizeof(STARTUPINFO));
				memset(&pi, 0, sizeof(PROCESS_INFORMATION));
				CreateProcess(NULL, command, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
			}
			if((cli_socket = accept(svr_socket, (struct sockaddr *) &cli_addr, &cli_addr_len)) != INVALID_SOCKET) {
				u_long val = 1;
				ioctlsocket(cli_socket, FIONBIO, &val);
				telnet_closed = false;
#if 0
				sprintf_s(command, MAX_PATH, "\033]0;%s\007", tchar_to_char(title));
				send_telnet(command);
#endif
				uint8_t will_echo[] = {0xff, 0xfb, 0x01};
				send(cli_socket, (char *)will_echo, 3, 0);
				send_telnet("\033[2l");  // key unlock
				send_telnet("\033[12h"); // local echo off
			}
		}
	}
}

void OSD::close_telnet()
{
	if(svr_socket != INVALID_SOCKET) {
		shutdown(svr_socket, /*SD_BOTH*/2);
		closesocket(svr_socket);
		svr_socket = cli_socket = INVALID_SOCKET;
	}
	WSACleanup();
}

void OSD::send_telnet(const char* string)
{
	if(cli_socket != INVALID_SOCKET) {
		for(unsigned int i = 0; i < strlen(string); i++) {
			if(string[i] == 0x0d || string[i] == 0x0a) {
				send_telnet("\033E");
			} else if(string[i] == 0x08) {
				send_telnet("\033[1D");
			} else {
				send(cli_socket, &string[i], 1, 0);
			}
		}
	}
}
