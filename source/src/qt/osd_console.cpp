/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	[ Qt console ]
*/

#include "fifo.h"
#include "./osd.h"

#include <QString>

//extern EMU *emu;
//BOOL WINAPI ctrl_c_handler(DWORD type)
//{
//	return TRUE;
//}
void OSD_BASE::initialize_console()
{
	console_count = 0;
	use_telnet = false;
	if(p_config != nullptr) {
		use_telnet = p_config->use_telnet;
	}
	telnet_closed = true;
	//svr_socket = cli_socket = INVALID_SOCKET;
}

void OSD_BASE::release_console()
{
	close_console();
}


void OSD_BASE::do_write_inputdata(QString s)
{
	//emit sig_console_input_string(s);
}

void OSD_BASE::do_set_input_string(QString s)
{
	if(!s.isEmpty()) {
		console_cmd_str.append(s);
		console_cmd_str.append(QString::fromUtf8("\n"));
	}
}

_TCHAR *OSD_BASE::console_input_string(void)
{
	//DebugSemaphore->acquire(1);
	if(console_cmd_str.isEmpty()) {
		//DebugSemaphore->release(1);
		return NULL;
	}
	_TCHAR *p = (_TCHAR *)console_cmd_str.toUtf8().constData();
	//DebugSemaphore->release();
	return p;
}

void OSD_BASE::clear_console_input_string(void)
{
	//DebugSemaphore->acquire(1);
	console_cmd_str.clear();
	//DebugSemaphore->release();
}

void OSD_BASE::open_console(int width, int height, const _TCHAR* title)
{
	//DebugSemaphore->acquire(1);
	int _count = console_count.load();
	console_count++;
	if(_count == 0) {
		console_cmd_str.clear();
		if(use_telnet) {
			open_telnet(title);
			return;
		}
	}
	//DebugSemaphore->release();

}

void OSD_BASE::close_console()
{
	int _count = console_count.load();
	if(_count > 0 ) {
		console_count--;
		if(console_count.load() == 0) {
			console_cmd_str.clear();
			if(use_telnet) {
				close_telnet();
				return;
			}
			emit sig_close_console();
		}
	}
}

unsigned int OSD_BASE::get_console_code_page()
{
	//return GetConsoleCP();
	return 0;
}

void OSD_BASE::set_console_text_attribute(unsigned short attr)
{
	QString attr_table[] = {
//		QString::fromUtf8("<FONT COLOR=black>"), // 0
		QString::fromUtf8("<FONT COLOR=white>"), // 0 : OK?
		QString::fromUtf8("<FONT COLOR=blue>"),  // 1
		QString::fromUtf8("<FONT COLOR=green>"), // 2
		QString::fromUtf8("<FONT COLOR=aqua>"),  // 3
		QString::fromUtf8("<FONT COLOR=red>"),   // 4
		QString::fromUtf8("<FONT COLOR=fuchsia>"),  // 5
		QString::fromUtf8("<FONT COLOR=yellow>"),   // 6
//		QString::fromUtf8("<FONT COLOR=gray>"),     // 7
		QString::fromUtf8("<FONT COLOR=black>"),     // 7
	};
	uint32_t new_color = 0;
	bool is_strong = false;
	if(attr & OSD_CONSOLE_BLUE)  new_color |= 1;
	if(attr & OSD_CONSOLE_GREEN) new_color |= 2;
	if(attr & OSD_CONSOLE_RED)   new_color |= 4;

	QString new_attr = attr_table[new_color];
	if(attr & OSD_CONSOLE_INTENSITY) {
		is_strong = true;
	}
	emit sig_set_attribute_debugger(new_attr, is_strong);
	//SetConsoleTextAttribute(hStdOut, new_attr);
}

void OSD_BASE::write_console(const _TCHAR* buffer, unsigned int length)
{
	if(use_telnet) {
		send_telnet(tchar_to_char(buffer));
		return;
	}
	QString s = QString::fromLocal8Bit((const char *)buffer, length);
	emit sig_put_string_debugger(s);
}

int OSD_BASE::read_console_input(_TCHAR* buffer, int length)
{
	if(use_telnet) {
#if 0 // ToDo: Implement Qt related API. 20221124 K.O
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
#endif
		return 0;
	}

	int count = 0;
	QString tmps;
	//DebugSemaphore->acquire(1);
	tmps = console_cmd_str.left(16);
	//DebugSemaphore->release(1);
	if(buffer == NULL) return 0;
	memset(buffer, 0x00, 16);
	if(tmps.isEmpty()) {
		return 0;
	}
	int locallen = tmps.indexOf(QString::fromUtf8("\n"));
	if(locallen >= 16) locallen = 15;
	if(locallen >= 0) {
		tmps = tmps.left(locallen + 1);
		locallen = locallen + 1;
	}

	count = tmps.length();
	if(tmps.isEmpty() || (count <= 0)) return 0; 
	if(count > 16) count = 16;
	if(count > length) count = length;
	//DebugSemaphore->acquire(1);
	int l = console_cmd_str.length();
	
	console_cmd_str = console_cmd_str.right(l - count);	
	strncpy((char *)buffer, tmps.toLocal8Bit().constData(), count);
	//DebugSemaphore->release(1);

	return count;
}

// This is not recognise char code.
bool OSD_BASE::is_console_key_pressed(uint32_t ch)
{
	if(use_telnet) {
#if 0 // ToDo: Implement Qt related API. 20221124 K.O
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
#endif
		return false;
	}
	_TCHAR buf[17];
	if(read_console_input(buf, 16) > 0) return true;
	return false;
}
	
bool OSD_BASE::is_console_closed()
{
	if(use_telnet) {
		return telnet_closed.load();
	}
	return false;
}

void OSD_BASE::close_debugger_console()
{
	emit sig_debugger_finished(); // It's dirty...
}

void OSD_BASE::do_close_debugger_thread()
{
	emit sig_debugger_finished();
}

#if 0 // ToDo: Implement Qt related API. 20221124 K.O
void OSD_BASE::open_telnet(const _TCHAR* title)
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

void OSD_BASE::close_telnet()
{
	if(svr_socket != INVALID_SOCKET) {
		shutdown(svr_socket , /*SD_BOTH*/2);
		closesocket(svr_socket);
		svr_socket = cli_socket = INVALID_SOCKET;
	}
	WSACleanup();
}

void OSD_BASE::send_telnet(const char* string)
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
#else /* These are dummy. 20221124 K.O */
void OSD_BASE::open_telnet(const _TCHAR* title)
{
}
void OSD_BASE::close_telnet()
{
}
void OSD_BASE::send_telnet(const char* string)
{
}
#endif
