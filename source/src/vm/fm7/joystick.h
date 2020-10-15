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

namespace FM7 {
class JOYSTICK : public DEVICE {
 private:
	bool emulate_mouse[2];
	uint32_t joydata[2];
	const uint32_t *rawdata;
	const int32_t *mouse_state;
	int dx, dy;
	int lx, ly;
	uint32_t mouse_button;
	bool mouse_strobe;
	uint32_t mouse_data;
	int mouse_phase;
	int mouse_timeout_event;
	uint8_t port_b_val;
	uint8_t lpmask;
	int lpt_type;
 protected:
	DEVICE *opn;
	
 private:
	uint32_t update_mouse(uint32_t mask);
	void   update_strobe(bool flag);
	uint32_t mouse_type;
 public:
	JOYSTICK(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu);
	~JOYSTICK();

	void initialize(void);
	void event_frame(void);
	//void event_vline(int v, int clock);
	
	uint32_t __FASTCALL read_data8(uint32_t addr);
	void __FASTCALL write_data8(uint32_t addr, uint32_t data);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	void __FASTCALL event_callback(int event_id, int err);
	
	void reset(void);
	void update_config();
	bool process_state(FILEIO *state_fio, bool loading);
	
	void set_context_opn(DEVICE *p) {
		opn = p;
	}
};

}
#endif
