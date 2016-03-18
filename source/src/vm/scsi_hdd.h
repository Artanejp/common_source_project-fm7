/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI hard disk drive ]
*/

#ifndef _SCSI_HDD_H_
#define _SCSI_HDD_H_

#include "scsi_dev.h"

class SCSI_HDD : public SCSI_DEV
{
public:
	SCSI_HDD(VM* parent_vm, EMU* parent_emu) : SCSI_DEV(parent_vm, parent_emu) 
	{
		my_sprintf_s(vendor_id, 9, "NECITSU");
		my_sprintf_s(product_id, 17, "SCSI-HDD");
		device_type = 0x00;
		is_removable = false;
		logical_block_size = physical_block_size = 512;
		seek_time = 10000; // 10msec
		bytes_per_sec = 0x500000; // 5MB/sec
		
		default_drive_size = 0x2800000;	// 40MB
	}
	~SCSI_HDD() {}
	
	// common functions
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("SCSI Hard Disk Drive");
	}
	
	// virtual scsi functions
	void read_buffer(int length);
	void write_buffer(int length);
	void initialize_max_logical_block_addr();
	
	// unique variable
	uint32_t default_drive_size;
};

#endif

