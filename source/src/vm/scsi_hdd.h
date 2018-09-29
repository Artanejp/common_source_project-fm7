/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI/SASI hard disk drive ]
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

	_TCHAR image_path[8][MAX_PATH];
	int sector_size[8];
 	
public:
	SCSI_HDD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : SCSI_DEV(parent_vm, parent_emu) 
	{
		for(int i = 0; i < 8; i++) {
			disk[i] = NULL;
			image_path[i][0] = _T('\0');
		}
		my_sprintf_s(vendor_id, 9, "NECITSU");
		my_sprintf_s(product_id, 17, "SCSI-HDD");
		device_type = 0x00;
		is_removable = is_hot_swappable = false;

		seek_time = 10000; // 10msec
		bytes_per_sec = 0x500000; // 5MB/sec
		
//		default_drive_size = 0x2800000;	// 40MB
		set_device_name(_T("SCSI HDD"));
	}
	~SCSI_HDD() {}
	
	// common functions
	void decl_state();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// virtual scsi functions
	void release();
	void reset();
	bool is_device_existing();
	uint32_t physical_block_size();
	uint32_t logical_block_size();
	uint32_t max_logical_block_addr();
	bool read_buffer(int length);
	bool write_buffer(int length);
	
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
	void open(int drv, const _TCHAR* file_path, int default_sector_size);
	void close(int drv);
	bool mounted(int drv);
	bool accessed(int drv);
};

class SASI_HDD : public SCSI_HDD
{
public:
	SASI_HDD(VM* parent_vm, EMU* parent_emu) : SCSI_HDD(parent_vm, parent_emu)
	{
		set_device_name(_T("SASI Hard Disk Drive"));
	}
	~SASI_HDD() {}
	
	// virtual scsi functions
	int get_command_length(int value);
	void start_command();
};

#endif

