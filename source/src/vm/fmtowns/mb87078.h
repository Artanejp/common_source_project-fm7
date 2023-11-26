/*!
 * @file mb87078.h
 * @brief Electric volume (for FM-Towns), MB87078
 * @author Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
 * @date 2021.09.18-
 */


#pragma once

#include "device.h"

class MB87078 : public DEVICE {
protected:
	outputs_t target_volume[4];
	outputs_t target_mute[4];
	uint8_t data_reg[4];
	uint8_t com_reg;

public:
	MB87078(VM_TEMPLATE *parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 4; i++) {
			initialize_output_signals(&(target_volume[i]));
			initialize_output_signals(&(target_mute[i]));
			data_reg[i] = 0x3f; // Minimum
		}
		com_reg = 0x00;
		set_device_name(_T("MB87078 Electric volumes"));
	}
	~MB87078()
	{
	}

	virtual void reset() override;
	virtual bool process_state(FILEIO* state_fio, bool loading) override;

	virtual uint32_t __FASTCALL read_io8(uint32_t addr) override;
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;

	// unique functions
	void set_context_target(int ch, DEVICE* dev, int volume_id, int mute_id, uint32_t mute_mask)
	{
		if(volume_id >= 0) {
			register_output_signal(&(target_volume[ch & 3]), dev, volume_id, 0x003f);
		}
		if(mute_id >= 0) {
			register_output_signal(&(target_mute[ch & 3]), dev, mute_id, mute_mask);
		}
	}
};
