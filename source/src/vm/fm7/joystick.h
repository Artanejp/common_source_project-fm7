/*
 * FM-7 Keyboard [joystick.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jun 16, 2015 : Initial
 *
 */

#ifndef _VM_FM7_JOYSTICK_H_
#define _VM_FM7_JOYSTICK_H_
#include "../device.h"

#include "fm7_common.h"
#include "../../fileio.h"

class JOYSTICK : public DEVICE {
 private:
	bool emulate_mouse[2];
	uint32 joydata[2];
	uint32 *rawdata;
	int32 *mouse_state;
	int dx, dy;
	int lx, ly;
	uint32 mouse_button;
	bool mouse_strobe;
	uint32 mouse_data;
	int mouse_phase;
	int mouse_timeout_event;
 protected:
	VM* p_vm;
	EMU* p_emu;
	DEVICE *opn;
 private:
	uint32 update_mouse(bool flag, uint32 mask);
 public:
	JOYSTICK(VM *parent_vm, EMU *parent_emu);
	~JOYSTICK();

	void initialize(void);
	void event_frame(void);
	
	uint32 read_data8(uint32 addr);
	void write_data8(uint32 addr, uint32 data);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	
	void reset(void);
	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);
	
	void set_context_opn(DEVICE *p) {
		opn = p;
	}
};

#endif
