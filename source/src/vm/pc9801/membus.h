/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2017.06.22-

	[ memory bus ]
*/

#ifndef _MEMBUS_H_
#define _MEMBUS_H_

#include "../memory.h"

class DISPLAY;

#if defined(SUPPORT_32BIT_ADDRESS)
	#define RAM_SIZE	0x800000	// 8MB
#elif defined(SUPPORT_24BIT_ADDRESS)
	#define RAM_SIZE	0x400000	// 4MB
#else
	#define RAM_SIZE	0x100000	// 1MB
#endif

class MEMBUS : public MEMORY
{
private:
	DISPLAY *d_display;
	
	// RAM
	uint8_t ram[RAM_SIZE];
	
	// BIOS/ITF
#if !defined(SUPPORT_HIRESO)
	uint8_t bios[0x18000];
#else
	uint8_t bios[0x10000];
#endif
#if defined(SUPPORT_BIOS_RAM)
	bool bios_ram_selected;
#endif
#if defined(SUPPORT_ITF_ROM)
	uint8_t itf[0x8000];
	bool itf_selected;
#endif
	void update_bios();
	
#if !defined(SUPPORT_HIRESO)
	// EXT BIOS
#if defined(_PC9801) || defined(_PC9801E)
	uint8_t fd_bios_2hd[0x1000];
	uint8_t fd_bios_2dd[0x1000];
#endif
	uint8_t sound_bios[0x4000];
//	uint8_t sound_bios_ram[0x4000];
	bool sound_bios_selected;
//	bool sound_bios_ram_selected;
	void update_sound_bios();
#if defined(SUPPORT_SASI_IF)
	uint8_t sasi_bios[0x1000];
	uint8_t sasi_bios_ram[0x1000];
	bool sasi_bios_selected;
	bool sasi_bios_ram_selected;
	void update_sasi_bios();
#endif
#if defined(SUPPORT_SCSI_IF)
	uint8_t scsi_bios[0x1000];
	uint8_t scsi_bios_ram[0x1000];
	bool scsi_bios_selected;
	bool scsi_bios_ram_selected;
	void update_scsi_bios();
#endif
#if defined(SUPPORT_IDE_IF)
	uint8_t ide_bios[0x4000];
//	uint8_t ide_bios_ram[0x4000];
	bool ide_bios_selected;
//	bool ide_bios_ram_selected;
	void update_ide_bios();
#endif
	
	// EMS
#if defined(SUPPORT_NEC_EMS)
	uint8_t nec_ems[0x10000];
	bool nec_ems_selected;
	void update_nec_ems();
#endif
#endif
	
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	uint8_t dma_access_ctrl;
	uint32_t window_80000h;
	uint32_t window_a0000h;
#endif
	inline bool get_memory_addr(uint32_t *addr);
	
public:
	MEMBUS(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMBUS() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	void write_data8w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_data8w(uint32_t addr, int *wait);
	void write_data16w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_data16w(uint32_t addr, int *wait);
	void write_data32w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_data32w(uint32_t addr, int *wait);
#endif
	void write_dma_data8w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_dma_data8w(uint32_t addr, int *wait);
	void write_dma_data16w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_dma_data16w(uint32_t addr, int *wait);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_display(DISPLAY* device)
	{
		d_display = device;
	}
};

#endif
