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

namespace PC9801 {
	class DISPLAY;
}
#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM)
	#if defined(SUPPORT_32BIT_ADDRESS)
	#define RAM_SIZE	0x1000000	// 16MB
	#elif defined(SUPPORT_24BIT_ADDRESS)
	#define RAM_SIZE	0x800000	// 8MB
	#endif
#elif defined(SUPPORT_24BIT_ADDRESS)
	#define RAM_SIZE	0x400000	// 4MB
#else
	#define RAM_SIZE	0x100000	// 1MB
#endif

namespace PC9801 {

#define SIG_LAST_ACCESS_INTERAM 1
#define SIG_INTRAM_WAIT			2
#define SIG_BANK08_WAIT			3
#define SIG_EXMEM_WAIT			4
#define SIG_SLOTMEM_WAIT		5
#define SIG_EXBOARDS_WAIT		6
#define SIG_INTROM_WAIT			7
#define SIG_TVRAM_WAIT			8
#define SIG_GVRAM_WAIT			9
	
class MEMBUS : public MEMORY
{
private:
	DISPLAY *d_display;
	
//	csp_state_utils *state_entry;
	// RAM
	uint8_t ram[RAM_SIZE];
	// BIOS/ITF
#if !defined(SUPPORT_HIRESO)
	uint8_t bios[0x18000];
#else
	uint8_t bios[0x10000];
#endif
#if defined(SUPPORT_BIOS_RAM)
#if !defined(SUPPORT_HIRESO)
	uint8_t bios_ram[0x18000];
#else
	uint8_t bios_ram[0x10000];
#endif
	bool bios_ram_selected;
#endif
	bool shadow_ram_selected;
	bool last_access_is_interam;
#if defined(SUPPORT_ITF_ROM)
	uint8_t itf[0x8000];
	bool itf_selected;
#endif

	int intram_wait;
	int bank08_wait;
	int exmem_wait;
	int slotmem_wait;
	int exboards_wait;
	int introm_wait;
	int gvram_wait_val;
	int tvram_wait_val;
	void update_bios();
	
#if !defined(SUPPORT_HIRESO)
	// EXT BIOS
#if defined(_PC9801) || defined(_PC9801E)
	uint8_t fd_bios_2hd[0x1000];
	uint8_t fd_bios_2dd[0x1000];
#endif
#endif
	uint8_t sound_bios[0x4000];
//	uint8_t sound_bios_ram[0x4000];
	bool sound_bios_selected;
	bool sound_bios_load;
	bool using_sound_bios;
	void update_sound_bios();
#if defined(SUPPORT_SCSI_IF)
	uint8_t scsi_bios[0x1000];
	uint8_t scsi_bios_ram[0x1000];
	bool scsi_bios_selected;
	bool scsi_bios_ram_selected;
	void update_scsi_bios();
#endif
#if defined(SUPPORT_SASI_IF)
	uint8_t sasi_bios[0x1000];
	uint8_t sasi_bios_ram[0x1000];
	bool sasi_bios_selected;
	bool sasi_bios_ram_selected;
	bool sasi_bios_load;
	void update_sasi_bios();
#endif
#if defined(SUPPORT_IDE_IF)
	uint8_t ide_bios[0x4000];
	uint8_t ide_bios_ram[0x2000];
	bool ide_bios_selected;
	bool ide_bios_bank;
	void update_ide_bios();
#endif
	// EMS
#if defined(SUPPORT_NEC_EMS)
	uint8_t nec_ems[0x10000];
	bool nec_ems_selected;
	void update_nec_ems();
	bool use_ems_as_protected;
	uint32_t ems_protected_base;
#endif

	bool page08_intram_selected;
	
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	uint8_t dma_access_ctrl;
	bool dma_access_a20;
	uint32_t window_80000h;
	uint32_t window_a0000h;
#endif
	void config_intram();
	
public:
	MEMBUS(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMBUS() {}
	
	// common functions
	void initialize();
	void reset();
	uint32_t __FASTCALL read_signal(int ch);
	void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	uint32_t __FASTCALL read_dma_data8(uint32_t addr);
	void __FASTCALL write_dma_data8(uint32_t addr, uint32_t data);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_display(DISPLAY* device)
	{
		d_display = device;
	}

	bool is_sasi_bios_load()
	{
#if defined(SUPPORT_SASI_IF) && !defined(SUPPORT_HIRESO)
		return sasi_bios_load;
#endif
		return false;
	}
};

	
}
#endif
