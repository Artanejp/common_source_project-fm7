/*
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'

	Author : Takeda.Toshiya
	Date   : 2017.06.25-

	[ cpu regs ]
*/

#include "cpureg.h"
#include "membus.h"
#if defined(SUPPORT_32BIT_ADDRESS)
#include "../i386.h"
#else
#include "../i286.h"
#endif
#include "../i8255.h"

namespace PC9801 {

void CPUREG::reset()
{
	d_cpu->set_address_mask(0x000fffff);
	nmi_enabled = false;
}

void CPUREG::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_CPU_NMI) {
		out_debug_log("NMI\n");
		//if(nmi_enabled) {
			write_signals(&outputs_nmi, data);
		//}
	} else if(ch == SIG_CPUREG_RESET) {
		out_debug_log("RESET FROM CPU!!!\n");
		uint8_t reset_reg = d_pio->read_signal(SIG_I8255_PORT_C);
		reset_reg = reset_reg & (uint8_t)(~0x20); // Reset SHUT1
		d_pio->write_signal(SIG_I8255_PORT_C, reset_reg, 0xff);
		d_cpu->set_address_mask(0x000fffff);
		//d_cpu->reset();
	}		
}
	
void CPUREG::write_io8(uint32_t addr, uint32_t data)
{
	//out_debug_log(_T("I/O WRITE: %04x %04x\n"), addr, data);
	switch(addr) {
	case 0x0050:
		nmi_enabled = false;
		break;
	case 0x0052:
		nmi_enabled = true;
		break;
	case 0x00f0:
		{
			uint8_t reset_reg = d_pio->read_signal(SIG_I8255_PORT_C);
			reset_reg = reset_reg & (uint8_t)(~0x20); // Reset SHUT1
			d_pio->write_signal(SIG_I8255_PORT_C, reset_reg, 0xff);
			d_cpu->set_address_mask(0x000fffff);
			d_cpu->reset();
		}
		break;
	case 0x00f2:
#if defined(SUPPORT_32BIT_ADDRESS)
		d_cpu->set_address_mask(0xffffffff);
#else
		d_cpu->set_address_mask(0x00ffffff);
#endif
		break;
#if defined(SUPPORT_32BIT_ADDRESS)
	case 0x00f6:
		switch(data) {
		case 0x02:
			d_cpu->set_address_mask(0xffffffff);
//			d_cpu->set_address_mask(0x00ffffff);
			break;
		case 0x03:
			d_cpu->set_address_mask(0x000fffff);
			break;
		}
		break;
#endif
	}
}

uint32_t CPUREG::read_io8(uint32_t addr)
{
	uint32_t value;
	//out_debug_log(_T("I/O READ: %04x \n"), addr);
	
	switch(addr) {
	case 0x00f0:
		value  = 0x00;
//		value |= 0x80; // 1 = PC-9801NA, 0 = PC-9801NA/C
//		value |= 0x80; // 1 = PC-9821modelS1, 0 = PC-9821modelS2
//		value |= 0x80; // 1 = PC-9821CemodelS1, 0 = PC-9821CemodelS2
//		value |= 0x80; // 1 = PC-9821Xt, 0 = PC-9821Xa
//		value |= 0x80; // CPU MODE, 1 = High/Low, 0 = Middle (PC-9821Ap/As/Ae/Af)
//		value |= 0x40; // ODP, 1 = Existing (PC-9821Ts)
#if !defined(SUPPORT_SCSI_IF)
		value |= 0x40; // Internal 55-type SCSI-HDD, 0 = Existing
#endif
#if !defined(SUPPORT_SASI_IF)
		value |= 0x20; // Internal 27-type SASI-HDD, 0 = Existing
#endif
		// ToDo: AMD98
//		value |= 0x10; // Unknown
		value |= ((d_mem->read_signal(SIG_LAST_ACCESS_INTERAM) != 0) ? 0x00: 0x08); // RAM access, 1 = Internal-standard/External-enhanced RAM, 0 = Internal-enhanced RAM
//		value |= 0x04; // Refresh mode, 1 = Standard, 0 = High speed
#if defined(HAS_I86) || defined(HAS_V30)
		value |= 0x02; // CPU mode, 1 = V30, 0 = 80286/80386
#endif
		value |= 0x01; // RAM access, 1 = Internal RAM, 0 = External-enhanced RAM
		return value;
	case 0x00f2:
		return ((d_cpu->get_address_mask() & (1 << 20)) ? 0x00 : 0x01) | 0xfe;
#if defined(SUPPORT_32BIT_ADDRESS)
	case 0x00f6:
		value  = ((d_cpu->get_address_mask() & (1 << 20)) != 0) ? 0x00 : 0x01;
#if defined(SUPPORT_HIRESO) && !defined(_PC98RL)
		value |= 0x10; // SASI-HDD, 1 = DMA ch0, 0 = DMA ch1
#endif
		if(nmi_enabled) {
			value |= 0x02; // NMI, 1 = Enabled
		}
		return value;
#endif
	}
	return 0xff;
}

#define STATE_VERSION	1

bool CPUREG::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(nmi_enabled);
 	return true;
}

}

