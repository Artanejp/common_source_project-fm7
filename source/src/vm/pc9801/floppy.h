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
	Date   : 2010.09.15-

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#if defined(SUPPORT_2HD_FDD_IF)
#define SIG_FLOPPY_2HD_IRQ	0
#define SIG_FLOPPY_2HD_DRQ	1
#endif
#if defined(SUPPORT_2DD_FDD_IF)
#define SIG_FLOPPY_2DD_IRQ	2
#define SIG_FLOPPY_2DD_DRQ	3
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
#define SIG_FLOPPY_IRQ	0
#define SIG_FLOPPY_DRQ	1
#endif

class UPD765A;

namespace PC9801 {

class FLOPPY : public DEVICE
{
private:
#if defined(SUPPORT_2HD_FDD_IF)
	UPD765A *d_fdc_2hd;
	uint8_t ctrlreg_2hd;
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	UPD765A *d_fdc_2dd;
	uint8_t ctrlreg_2dd;
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	UPD765A *d_fdc;
	uint8_t ctrlreg, modereg;
#endif
	DEVICE *d_dma, *d_pic;
	
	int timer_id;
	
public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#if defined(SUPPORT_2HD_FDD_IF)
		d_fdc_2hd = NULL;
#endif
#if defined(SUPPORT_2DD_FDD_IF)
		d_fdc_2dd = NULL;
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
		d_fdc = NULL;
#endif
		d_dma = NULL;
		d_pic = NULL;
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}
	
	// common functions
	void reset();
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	void __FASTCALL event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
#if defined(SUPPORT_2HD_FDD_IF)
	void set_context_fdc_2hd(UPD765A* device)
	{
		d_fdc_2hd = device;
	}
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	void set_context_fdc_2dd(UPD765A* device)
	{
		d_fdc_2dd = device;
	}
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	void set_context_fdc(UPD765A* device)
	{
		d_fdc = device;
	}
#endif
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

}
#endif

