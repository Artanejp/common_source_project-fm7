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
class HARDDISK;

namespace FMR50 {

class BIOS : public DEVICE
{
private:
	DEVICE *d_mem, *d_io;
	DISK *disk[MAX_DRIVE];
	HARDDISK *harddisk[USE_HARD_DISK];
	
	// pseudo bios
	uint8_t *cmos, *vram, *cvram;
#ifdef _FMR60
	uint8_t *avram;
#else
	uint8_t *kvram;
#endif
	int secnum, timeout;
	
	// disk bios
	bool access_fdd[MAX_DRIVE];
	uint8_t  drive_mode1[MAX_DRIVE];
	uint16_t drive_mode2[MAX_DRIVE];
	int scsi_blocks[USE_HARD_DISK];
	
public:
	BIOS(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu) {
		for(int i = 0; i < MAX_DRIVE; i++) disk[i] = NULL;
		//for(int i = 0; i < USE_HARD_DISK; i++) harddisk[i] = NULL;
		set_device_name(_T("Pseudo BIOS"));
	}
	~BIOS() {}
	
	// common functions
	void initialize() override;
	void reset() override;
	void event_frame() override;
	
	bool bios_call_far_i86(uint32_t PC, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles) override;
	bool bios_call_far_ia32(uint32_t PC, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles) override;
	bool bios_int_i86(int intnum, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles) override;
	bool bios_int_ia32(int intnum, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles) override;
	
	uint32_t __FASTCALL read_signal(int ch) override;
	
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_floppy_disk_handler(int drv, DISK* device)
	{
		disk[drv] = device;
	}
	void set_hard_disk_handler(int drv, HARDDISK* device)
	{
		harddisk[drv] = device;
	}
	void set_cmos_ptr(uint8_t* ptr)
	{
		cmos = ptr;
	}
	void set_vram_ptr(uint8_t* ptr)
	{
		vram = ptr;
	}
	void set_cvram_ptr(uint8_t* ptr)
	{
		cvram = ptr;
	}
#ifdef _FMR60
	void set_avram_ptr(uint8_t* ptr)
	{
		avram = ptr;
	}
#else
	void set_kvram_ptr(uint8_t* ptr)
	{
		kvram = ptr;
	}
#endif
};

}

#endif

