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
private:
	uint32_t max_logical_block_addr;
	void initialize_max_logical_block_addr();
	
public:
	SCSI_HDD(VM* parent_vm, EMU* parent_emu) : SCSI_DEV(parent_vm, parent_emu) 
	{
		max_logical_block_addr = 0;	// uninitialized
		logical_block_size = 512;
		bytes_per_sec = 0x500000;	 // 5mbytes/sec
		
		my_sprintf_s(vendor_id, 9, "NECITSU");
		my_sprintf_s(product_id, 17, "SCSI-HDD");
		default_drive_size = 0;
	}
	~SCSI_HDD() {}
	
	// unique functions
	void start_command();
	void read_buffer(int length);
	void write_buffer(int length);
	
	char vendor_id[8 + 1];
	char product_id[16 + 1];
	uint32_t default_drive_size;
};

#endif

