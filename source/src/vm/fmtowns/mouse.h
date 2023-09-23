/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2021.06.16 -
    History : 2020.06.16 Initial.
	[ Towns Mouse]

*/

#pragma once

#include "./js_template.h"

namespace FMTOWNS {

class MOUSE : public JSDEV_TEMPLATE
{
protected:
	const int32_t* mouse_state;

	int phase;
	int dx, dy;
	int lx, ly;

	int event_timeout;
	int event_sampling;

	virtual void initialize_status() override;

	virtual void sample_mouse_xy();
	virtual void update_strobe_mouse();
	virtual uint32_t update_mouse();
	virtual void check_mouse_data();
	virtual uint8_t __FASTCALL output_port_com(bool val, bool force)  override;

public:
	MOUSE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : JSDEV_TEMPLATE(parent_vm, parent_emu)
	{
		set_device_name(_T("FM-Towns MOUSE"));
	}
	~MOUSE() {}

	void initialize() override;
	void release() override;

	void __FASTCALL event_callback(int event_id, int err) override;

	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;

	virtual bool process_state(FILEIO* state_fio, bool loading) override;

	// unique functions
	virtual uint8_t __FASTCALL query(bool& status) override;
	virtual void reset_device(bool port_out) override;
	virtual void set_enable(bool is_enable) override;
};

}
