/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 socket ]
*/

#include "../emu.h"


//#ifdef USE_SOCKET
void OSD_BASE::initialize_socket()
{
	for(int i = 0; i < SOCKET_MAX; i++) {
		soc[i] = -1;
		socket_delay[i] = 0;
		recv_r_ptr[i] = recv_w_ptr[i] = 0;
	}
}

void OSD_BASE::release_socket()
{
	// release sockets

	// release winsock
}

void OSD_BASE::notify_socket_connected(int ch)
{
#ifdef USE_SOCKET
	vm->notify_socket_connected(ch);
#endif	
}

void OSD_BASE::notify_socket_disconnected(int ch)
{
	// winmain notify that network is disconnected
	if(!socket_delay[ch]) {
		socket_delay[ch] = 1;//56;
	}
}

void OSD_BASE::update_socket()
{
#ifdef USE_SOCKET
	for(int i = 0; i < SOCKET_MAX; i++) {
		if(recv_r_ptr[i] < recv_w_ptr[i]) {
			// get buffer
			int size0, size1;
			uint8_t* buf0 = vm->get_socket_recv_buffer0(i, &size0, &size1);
			uint8_t* buf1 = vm->get_socket_recv_buffer1(i);
			
			int size = recv_w_ptr[i] - recv_r_ptr[i];
			if(size > size0 + size1) {
				size = size0 + size1;
			}
			char* src = &recv_buffer[i][recv_r_ptr[i]];
			recv_r_ptr[i] += size;
			
			if(size <= size0) {
				memcpy(buf0, src, size);
			} else {
				memcpy(buf0, src, size0);
				memcpy(buf1, src + size0, size - size0);
			}
			vm->inc_socket_recv_buffer_ptr(i, size);
		} else if(socket_delay[i] != 0) {
			if(--socket_delay[i] == 0) {
				vm->notify_socket_disconnected(i);
			}
		}
	}
#endif	
}

bool OSD_BASE::initialize_socket_tcp(int ch)
{
	is_tcp[ch] = true;
	soc[ch] = -1;
	recv_r_ptr[ch] = recv_w_ptr[ch] = 0;
	return false;
	//return true;
}

bool OSD_BASE::initialize_socket_udp(int ch)
{
	is_tcp[ch] = false;
	soc[ch] = -1;
	recv_r_ptr[ch] = recv_w_ptr[ch] = 0;
	return false;
}

bool OSD_BASE::connect_socket(int ch, uint32_t ipaddr, int port)
{
	return true;
}

void OSD_BASE::disconnect_socket(int ch)
{
	soc[ch] = -1;
#ifdef USE_SOCKET
	vm->notify_socket_disconnected(ch);
#endif	
}

bool OSD_BASE::listen_socket(int ch)
{
	return false;
}

void OSD_BASE::send_socket_data_tcp(int ch)
{
}

void OSD_BASE::send_socket_data_udp(int ch, uint32_t ipaddr, int port)
{
}

void OSD_BASE::send_socket_data(int ch)
{
}

void OSD_BASE::recv_socket_data(int ch)
{
}

int OSD_BASE::get_socket(int ch)
{
	return soc[ch];
}
