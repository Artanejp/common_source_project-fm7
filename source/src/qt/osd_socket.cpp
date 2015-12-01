/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 socket ]
*/

#include "osd.h"

#ifdef USE_SOCKET
void OSD::initialize_socket()
{
	for(int i = 0; i < SOCKET_MAX; i++) {
		soc[i] = -1;
		socket_delay[i] = 0;
		recv_r_ptr[i] = recv_w_ptr[i] = 0;
	}
}

void OSD::release_socket()
{
	// release sockets

	// release winsock
}

void OSD::socket_connected(int ch)
{
	// winmain notify that network is connected
	vm->network_connected(ch);
}

void OSD::socket_disconnected(int ch)
{
	// winmain notify that network is disconnected
	if(!socket_delay[ch]) {
		socket_delay[ch] = 1;//56;
	}
}

void OSD::update_socket()
{
	for(int i = 0; i < SOCKET_MAX; i++) {
		if(recv_r_ptr[i] < recv_w_ptr[i]) {
			// get buffer
			int size0, size1;
			uint8* buf0 = vm->get_recvbuffer0(i, &size0, &size1);
			uint8* buf1 = vm->get_recvbuffer1(i);
			
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
			vm->inc_recvbuffer_ptr(i, size);
		} else if(socket_delay[i] != 0) {
			if(--socket_delay[i] == 0) {
				vm->network_disconnected(i);
			}
		}
	}
}

bool OSD::init_socket_tcp(int ch)
{
	is_tcp[ch] = true;
	soc[ch] = -1;
	recv_r_ptr[ch] = recv_w_ptr[ch] = 0;
	return false;
	//return true;
}

bool OSD::init_socket_udp(int ch)
{
	is_tcp[ch] = false;
	soc[ch] = -1;
	recv_r_ptr[ch] = recv_w_ptr[ch] = 0;
	return false;
}

bool OSD::connect_socket(int ch, uint32 ipaddr, int port)
{
	return true;
}

void OSD::disconnect_socket(int ch)
{
	soc[ch] = -1;
	vm->network_disconnected(ch);
}

bool OSD::listen_socket(int ch)
{
	return false;
}

void OSD::send_data_tcp(int ch)
{
}

void OSD::send_data_udp(int ch, uint32 ipaddr, int port)
{
}

void OSD::send_data(int ch)
{
}

void OSD::recv_data(int ch)
{
}
#endif
