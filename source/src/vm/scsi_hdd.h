/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI hard disk drive ]
*/

#ifndef _SCSI_HDD_H_
#define _SCSI_HDD_H_

#include "scsi_dev.h"

class HARDDISK;
//class csp_state_utils;

class SCSI_HDD : public SCSI_DEV
{
private:
	HARDDISK* disk[8];
	
//protected:
//	csp_state_utils *state_entry;

public:
	SCSI_HDD(VM* parent_vm, EMU* parent_emu) : SCSI_DEV(parent_vm, parent_emu) 
	{
		for(int i = 0; i < 8; i++) {
			disk[i] = NULL;
		}
		my_sprintf_s(vendor_id, 9, "NECITSU");
		my_sprintf_s(product_id, 17, "SCSI-HDD");
		device_type = 0x00;
		is_removable = false;
		seek_time = 10000; // 10msec
		bytes_per_sec = 0x500000; // 5MB/sec
		
		default_drive_size = 0x2800000;	// 40MB
		set_device_name(_T("SCSI HDD"));
	}
	~SCSI_HDD() {}
	
	// common functions
	void decl_state();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// virtual scsi functions
	void release();
	bool is_device_existing();
	uint32_t physical_block_size();
	uint32_t logical_block_size();
	uint32_t max_logical_block_addr();
	void read_buffer(int length);
	void write_buffer(int length);
	
	// unique functions
	void set_disk_handler(int drv, HARDDISK* device)
	{
		if(drv < 8) {
			disk[drv] = device;
		}
	}
	HARDDISK* get_disk_handler(int drv)
	{
		if(drv < 8) {
			return disk[drv];
		}
		return NULL;
	}
	uint32_t default_drive_size;
};

#endif

