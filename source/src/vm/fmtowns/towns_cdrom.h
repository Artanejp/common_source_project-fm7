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
#define SIG_TOWNS_CDROM_SET_TRACK         0x10
#define SIG_TOWNS_CDROM_MAX_TRACK         0x11
#define SIG_TOWNS_CDROM_IS_MEDIA_INSERTED 0x12
#define SIG_TOWNS_CDROM_REACHED_MAX_TRACK 0x13
#define SIG_TOWNS_CDROM_CURRENT_TRACK     0x14
#define SIG_TOWNS_CDROM_START_MSF         0x15
#define SIG_TOWNS_CDROM_START_MSF_AA      0x16
#define SIG_TOWNS_CDROM_GET_ADR           0x17
#define SIG_TOWNS_CDROM_SET_STAT_TRACK    0x18
#define SIG_TOWNS_CDROM_RELATIVE_MSF      0x20
#define SIG_TOWNS_CDROM_ABSOLUTE_MSF      0x21

// Virtual (pseudo) SCSI command.
#define TOWNS_CDROM_CDDA_PLAY             0xf0
#define TOWNS_CDROM_CDDA_PAUSE            0xf1
#define TOWNS_CDROM_CDDA_UNPAUSE          0xf2
#define TOWNS_CDROM_CDDA_STOP             0xf3

class SCSI_HOST;
class FIFO;
class FILEIO;

namespace FMTOWNS {
	class CDC;
}

namespace FMTOWNS {
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
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t __FASTCALL read_signal(int id);
	
	virtual void event_callback(int event_id, int err);

	virtual bool process_state(FILEIO* state_fio, bool loading);

	// SCSI SPECIFIC COMMANDS
	virtual int get_command_length(int value);
	virtual void start_command();

	// Towns specified command
	virtual void set_subq(void);
	virtual uint8_t get_subq_status();
	virtual uint8_t read_subq();
	virtual void set_cdda_status(uint8_t status)
	{
		SCSI_CDROM::set_cdda_status(status);
	}
	virtual int get_track(uint32_t lba)
	{
		return SCSI_CDROM::get_track(lba);
	}
	virtual double get_seek_time(uint32_t lba)
	{
		return SCSI_CDROM::get_seek_time(lba);
	}
	virtual uint8_t get_cdda_status()
	{
		return cdda_status;
	}
	void set_read_mode(bool is_mode2)
	{
		read_mode = is_mode2;
	}
};

}
