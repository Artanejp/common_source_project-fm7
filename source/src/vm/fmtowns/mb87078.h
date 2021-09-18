/*!
 * @file mb87078.h
 * @brief Electric volume (for FM-Towns), MB87078
 * @author Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
 * @date 2021.09.18-
 */


#pragma once

#include "device.h"

#define MB87078_TYPE_MASK_LEFT		0x01
#define MB87078_TYPE_MASK_RIGHT		0x02
#define MB87078_TYPE_MASK_CENTER	(MB87078_TYPE_MASK_LEFT | MB87078_TYPE_MASK_RIGHT)
	
class MB87078 : public DEVICE {
protected:
	struct channels_s {
		DEVICE*		dev;
		int			devch;			// channel at device.
		int			mutesig;		// signal number to mute target.
		uint32_t	mutemask;		// signal mask to mute target.
		uint32_t	muteval;		// signal value to mute target.
		bool		is_negative;	// Negative logic for muting.
		
		uint8_t		channelmask;
		int			extvalue;		// Set by set_volume_per_channel();
		bool		enabled;
		int			intvalue;	// Set by I/O ports.
	} channels[4];

	uint8_t regs[2];
	
	uint8_t current_channel;
	bool current_c32;
	bool current_c0;
	bool current_en;
	
	virtual void set_volume_internal(int ch, int vol, bool _force = false);
	virtual void set_enable_internal(int ch, bool en, bool _force = false);
	virtual void device_reset();
	virtual int reconfig_device();

public:
	MB87078(VM_TEMPLATE *parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 4; i++) {
			channels[i].dev = NULL;
			channels[i].devch = 0;
			channels[i].mutesig = -1;
			channels[i].muteval = 0xffffffff;
			channels[i].mutemask = 0xffffffff;
			channels[i].is_negative = false;
			
			channels[i].channelmask = 0x00; // None used
			channels[i].enabled = false;
			channels[i].intvalue = -64; // -32.0db * 2
			channels[i].extvalue = 0;
		}
		set_device_name(_T("MB87078 Electric volumes"));
	}
	~MB87078()
	{
	}

	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data);

	virtual void reset();
	virtual void initialize();
	virtual bool process_state(FILEIO* state_fio, bool loading);
	
	// Unique functions.
	virtual void set_volume_per_channel(int ch, int db);
	virtual void set_volumes(int left_ch, int left_db, int right_ch, int right_db);

	void set_context_device(int ch, DEVICE* dev, int devch,
							uint8_t chmask, int mutesig = -1,
							uint32_t muteval = 0xffffffff,
							uint32_t mutemask = 0xffffffff,
							bool is_negative = false)
	{
		if((ch >= 0) && (ch < 4)) {
			channels[ch].dev = dev;
			channels[ch].mutesig = mutesig;
			channels[ch].muteval = muteval;
			channels[ch].mutemask = mutemask;
			channels[ch].is_negative = is_negative;

			channels[ch].devch = devch;
			channels[ch].channelmask = chmask;
			channels[ch].extvalue = -64;
			channels[ch].enabled = true;
			channels[ch].intvalue = 0;
		}
	}
};
	

	
