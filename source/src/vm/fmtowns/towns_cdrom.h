/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.31 -

	[FM-Towns CD-ROM based on SCSI CDROM]
*/
#pragma once

#include "../../common.h"
#include "../scsi_dev.h"
#include "../scsi_cdrom.h"

// 0 - 9 : SCSI_CDROM::
// 100 - : SCSI_DEV::
#define SIG_TOWNS_CDROM_SET_TRACK 10

// Virtual (pseudo) SCSI command.
#define TOWNS_CDROM_CDDA_PLAY    0xf0
#define TOWNS_CDROM_CDDA_PAUSE   0xf1
#define TOWNS_CDROM_CDDA_UNPAUSE 0xf2
#define TOWNS_CDROM_CDDA_STOP    0xf3


class SCSI_HOST;
class FIFO;
class FILEIO;

namespace TOWNS {
	class CDC;
}

namespace TOWNS {
class TOWNS_CDROM : public SCSI_CDROM {
protected:
	FIFO* subq_buffer;
	bool subq_overrun;
	int stat_track;
	
	virtual void play_cdda_from_cmd();
	virtual void pause_cdda_from_cmd();
	virtual void unpause_cdda_from_cmd();
	virtual void stop_cdda_from_cmd();
	
public:
	TOWNS_CDROM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : SCSI_CDROM(parent_vm, parent_emu)
	{
		my_sprintf_s(vendor_id, 9, "FUJITSU");
		my_sprintf_s(product_id, 17, "CDROM");
		device_type = 0x05; // CD-ROM drive
		is_removable = true;
		is_hot_swappable = false;
//		seek_time = 400000; // 400msec (temporary)
		seek_time = 10.0;
		bytes_per_sec = 2048 * 75; // speed x1
		max_logical_block = 0;
		access = false;
		set_device_name(_T("FM-Towns CD-ROM drive"));
	}
	~TOWNS_CDROM() { }
	virtual void initialize();
	virtual void release();

	virtual void reset();
	virtual void write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t read_signal(int id);
	
	virtual void event_callback(int event_id, int err);

	virtual bool process_state(FILEIO* state_fio, bool loading);

	// SCSI SPECIFIC COMMANDS
	virtual int get_command_length(int value);
	virtual void start_command();

	// Towns specified command
	virtual void set_subq(void);
	virtual uint8_t get_subq_status();
	virtual uint8_t read_subq();
};

}
