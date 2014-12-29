/*
	FUJITSU FMR-30 Emulator 'eFMR-30'
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.10.06 -

	[ bios ]
*/

#ifndef _BIOS_H_
#define _BIOS_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class DISK;

class BIOS : public DEVICE
{
private:
	DEVICE *d_mem, *d_io;
	DISK *disk[MAX_DRIVE];
	
	// pseudo bios
	uint8 *cmos, *vram, *cvram;
#ifdef _FMR60
	uint8 *avram;
#else
	uint8 *kvram;
#endif
	int secnum, timeout;
	
	// disk bios
	bool access_fdd[MAX_DRIVE], access_scsi;
	int scsi_blocks[MAX_SCSI];
	_TCHAR scsi_path[MAX_SCSI][_MAX_PATH];
	int memcard_blocks[MAX_MEMCARD];
	bool memcard_protected[MAX_MEMCARD];
	_TCHAR memcard_path[MAX_SCSI][_MAX_PATH];
	uint8 buffer[0x10000];
	
	// power management
	uint8 powmode;
	
	uint32 disk_pc1, disk_pc2, cmos_pc, wait_pc;
	
public:
	BIOS(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~BIOS() {}
	
	// common functions
	void initialize();
	void reset();
	void event_frame();
	bool bios_call(uint32 PC, uint16 regs[], uint16 sregs[], int32* ZeroFlag, int32* CarryFlag);
	bool bios_int(int intnum, uint16 regs[], uint16 sregs[], int32* ZeroFlag, int32* CarryFlag);
	uint32 read_signal(int ch);
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_disk_handler(int drv, DISK* dsk)
	{
		disk[drv] = dsk;
	}
	void set_cmos_ptr(uint8* ptr)
	{
		cmos = ptr;
	}
	void set_vram_ptr(uint8* ptr)
	{
		vram = ptr;
	}
	void set_cvram_ptr(uint8* ptr)
	{
		cvram = ptr;
	}
#ifdef _FMR60
	void set_avram_ptr(uint8* ptr)
	{
		avram = ptr;
	}
#else
	void set_kvram_ptr(uint8* ptr)
	{
		kvram = ptr;
	}
#endif
};

#endif

