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

class SCSI_HDD : public SCSI_DEV
{
private:
	HARDDISK* disk[8];
	_TCHAR image_path[8][_MAX_PATH];
	int sector_size[8];
	uint64_t cur_position[8];
	
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
		is_removable = false;
		is_hot_swappable = true; //false;
		seek_time = 10000; // 10msec
		bytes_per_sec = 0x500000; // 5MB/sec
		data_req_delay = 0.1;
		step_period = 3000; // 3ms
		
		set_device_name(_T("SCSI Hard Disk Drive"));
	}
	~SCSI_HDD() {}
	
	// common function
	bool process_state(FILEIO* state_fio, bool loading);
	
	// virtual scsi functions
	void release();
	void reset();
	bool is_device_existing();
	uint32_t physical_block_size();
	uint32_t logical_block_size();
	uint32_t max_logical_block_addr();
	bool read_buffer(int length);
	bool write_buffer(int length);
	double get_seek_time(uint64_t new_position, uint64_t length);
	
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
	
	int step_period;
};

class SASI_HDD : public SCSI_HDD
{
public:
	SASI_HDD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : SCSI_HDD(parent_vm, parent_emu)
	{
		set_device_name(_T("SASI Hard Disk Drive"));
	}
	~SASI_HDD() {}
	
	// virtual scsi functions
	void start_command();
	bool write_buffer(int length);
};

#endif

