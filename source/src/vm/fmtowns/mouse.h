/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2021.06.16 -
    History : 2020.06.16 Initial.
	[ Towns Mouse]

*/

#pragma once

#include "../device.h"

#define SIG_MOUSE_ENABLE	1
#define SIG_MOUSE_STROBE	2
#define SIG_MOUSE_NUM		3
#define SIG_MOUSE_DATA		4
#define SIG_MOUSE_QUERY		5

namespace FMTOWNS {
	
class MOUSE : public DEVICE
{
private:
	DEVICE* d_joyport;
	const int32_t* mouse_state;
	
	int phase;
	bool strobe;
	int dx, dy;
	int lx, ly;
	
	bool mouse_connected;
	int port_num;

	uint8_t mouse_mask;
	uint8_t axisdata;
	
	int event_timeout;
	int event_sampling;

	void sample_mouse_xy();
	void update_strobe();
	uint32_t update_mouse();
	uint32_t check_mouse_data(bool is_send_data);
	
public:
	MOUSE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_joyport = NULL;
		set_device_name(_T("FM-Towns MOUSE"));
	}
	~MOUSE() {}
	
	void initialize();
	void release();

	void reset();
	void __FASTCALL event_callback(int event_id, int err);
	void event_pre_frame();
	
	uint32_t __FASTCALL read_signal(int ch);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);

	bool process_state(FILEIO* state_fio, bool loading);
	
	void set_context_com(DEVICE* dev)
	{
		d_joyport = dev;
	}
};

}

