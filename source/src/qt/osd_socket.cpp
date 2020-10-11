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
		socket_delay[i] = 0;
		host_mode[i] = false;
		is_tcp[i] = false;
	}
}

void OSD_BASE::release_socket()
{
	// release sockets

	// release winsock
}

void OSD_BASE::notify_socket_connected(int ch)
{
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
}

bool OSD_BASE::initialize_socket_tcp(int ch)
{
	is_tcp[ch] = true;
	return false;
	//return true;
}

bool OSD_BASE::initialize_socket_udp(int ch)
{
	is_tcp[ch] = false;
	return false;
}

bool OSD_BASE::connect_socket(int ch, uint32_t ipaddr, int port)
{
	return true;
}

void OSD_BASE::disconnect_socket(int ch)
{
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

SOCKET OSD_BASE::get_socket(int ch)
{
	return (SOCKET)0;
}

