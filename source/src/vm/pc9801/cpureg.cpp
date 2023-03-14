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
#include "../i8255.h"
#if defined(SUPPORT_32BIT_ADDRESS) || defined(UPPER_I386)
#include "../i386_np21.h"
//#include "../i386.h"
#else
//#include "../i286_np21.h"
#include "../i286.h"
#endif
#if !defined(SUPPORT_HIRESO)
#include "../i86.h"
#include "../i8255.h"
#endif

#define EVENT_WAIT 1

namespace PC9801 {

void CPUREG::initialize()
{
	reg_0f0 = 0x00;
	event_wait = -1;
#if !defined(SUPPORT_HIRESO)
	reg_0f0 = reg_0f0 | ((cpu_mode) ? 1 : 0);
#endif
}

void CPUREG::reset()
{
	d_cpu->set_address_mask(0x000fffff);
	nmi_enabled = false;
	init_clock = get_current_clock_uint64() & 0x000000ffffffffff;
#if !defined(SUPPORT_HIRESO)
	reg_0f0 = reg_0f0 & 0xfe;
	reg_0f0 = reg_0f0 | ((cpu_mode) ? 1 : 0);
	d_pio->write_signal(SIG_I8255_PORT_B, ((reg_0f0 & 1) != 0) ? 2 : 0, 2);
	if(d_v30 != NULL) {
		d_v30->write_signal(SIG_CPU_BUSREQ, ~reg_0f0, 1);
	}
	d_cpu->write_signal(SIG_CPU_BUSREQ, reg_0f0, 1);
#endif
}

void CPUREG::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0050:
		nmi_enabled = false;
		break;
	case 0x0052:
		nmi_enabled = true;
		break;
	case 0x005f:
		// ToDo: Both Pseudo BIOS.
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
#if !defined(SUPPORT_HIRESO)
		if(d_v30 != NULL) {
			d_v30->write_signal(SIG_CPU_BUSREQ, 1, 1);
		}
#endif
		if(event_wait >= 0) {
			cancel_event(this, event_wait);
			event_wait = -1;
		}
		register_event(this, EVENT_WAIT, 0.6, false, &event_wait);
		break;
	case 0x00f0:
		out_debug_log(_T("00F0h=%02X"), data);
		reg_0f0 = data;
		d_cpu->write_signal(SIG_CPU_BUSREQ,  reg_0f0, 1);
#if !defined(SUPPORT_HIRESO)
		if(d_v30 != NULL) {
			d_v30->reset();
			d_v30->write_signal(SIG_CPU_BUSREQ, ~reg_0f0, 1);
		}
		d_pio->write_signal(SIG_I8255_PORT_B, ((reg_0f0 & 1) != 0) ? 2 : 0, 2);
//		d_pio->write_signal(SIG_I8255_PORT_B, reg_0f0, 2);
#endif
		write_signals(&outputs_cputype, ((reg_0f0 & 1) != 0) ? 0xffffffff : 0x00000000);
		d_cpu->reset();
		d_cpu->set_address_mask(0x000fffff);
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

	switch(addr) {
	case 0x005c:
	case 0x005d:
	case 0x005e:
	case 0x005f:
		{
			// Timestamp register (from MAME 0.208)
			// 307.2KHz
			pair32_t n;
			uint64_t c;
			uint64_t elapsed;
			uint8_t nn;
		    c = get_current_clock_uint64() & 0x000000ffffffffff;
			if(c < init_clock) {
				elapsed = c + 0x0000010000000000 - init_clock;
			} else {
				elapsed = c - init_clock;
			}
			uint64_t ticks = (elapsed * 307200) / get_cpu_clock(0);
			n.d = (uint32_t)ticks;
			n.b.h3 = 0;
			switch(addr & 0x03) {
			case 0:
				nn = n.b.l;
				break;
			case 1:
			case 2: // OK? From NP2 v0.83 20190619
				nn = n.b.h;
				break;
			case 3: // OK? From NP2 v0.83 20190619
				nn = n.b.h2;
				break;
//			case 3:
//				nn = 0;
//				break;
			}
			return (uint32_t)nn;
		}
		break;
	case 0x00f0:
		value  = 0x00;
//#if defined(_PC9821_VARIANTS) || defined(_PC9801NA)
//		value |= 0x80; // 1 = PC-9801NA, 0 = PC-9801NA/C
//		value |= 0x80; // 1 = PC-9821modelS1, 0 = PC-9821modelS2
//		value |= 0x80; // 1 = PC-9821CemodelS1, 0 = PC-9821CemodelS2
//		value |= 0x80; // 1 = PC-9821Xt, 0 = PC-9821Xa
//		value |= 0x80; // CPU MODE, 1 = High/Low, 0 = Middle (PC-9821Ap/As/Ae/Af)
//		value |= 0x40; // ODP, 1 = Existing (PC-9821Ts)
//#else
#if defined(SUPPORT_SCSI_IF)
//		value |= 0x40; // Internal 55-type SCSI-HDD, 0 = Existing
#endif
#if defined(SUPPORT_SASI_IF)
//		value |= 0x20; // Internal 27-type SASI-HDD, 0 = Existing
#endif
#if defined(_PC9801RA) || defined(_PC9801RS) || defined(_PC9821_VARIANTS) || \
	defined(_PC98NOTE_VARIANTS) || defined(_PC98DOPLUS)
	#if !defined(SUPPORT_SASI_IF) || defined(SUPPORT_IDE_IF)
		value |= 0x20; // Internal 27-type SASI-HDD, 0 = Existing
	#endif
#else
		value |= 0x20;
#endif
		// ToDo: AMD98
//		value |= 0x10; // Unknown
		value |= 0x08; // RAM access, 1 = Internal-standard/External-enhanced RAM, 0 = Internal-enhanced RAM
//		value |= 0x04; // Refresh mode, 1 = Standard, 0 = High speed
#if defined(HAS_I86) || defined(HAS_V30)
		value |= 0x02; // CPU mode, 1 = V30, 0 = 80286/80386
#endif
#if !defined(SUPPORT_HIRESO)
		if(cpu_mode) {
			value |= 0x02; // CPU mode, 1 = V30, 0 = 80286/80386
		}
#endif
		value |= 0x01; // RAM access, 1 = Internal RAM, 0 = External-enhanced RAM
		return value;
	case 0x00f2:
		return ((d_cpu->get_address_mask() & (1 << 20)) ? 0x00 : 0x01) | 0xfe;
		break;
	case 0x00f4: // ToDo: DMA SPEED (after 9801DA)
		return 0xff;
		break;
#if defined(SUPPORT_32BIT_ADDRESS)
	case 0x00f6:
		value  = 0x00;
#if defined(SUPPORT_HIRESO) && !defined(_PC98RL)
		value |= 0x10; // SASI-HDD, 1 = DMA ch0, 0 = DMA ch1
#endif
		if(nmi_enabled) {
			value |= 0x02; // NMI, 1 = Enabled
		}
		return ((d_cpu->get_address_mask() & (1 << 20)) ? 0x00 : 0x01) | value;
#endif
	}
	return 0xff;
}

void CPUREG::event_callback(int id, int err)
{
	if(id == EVENT_WAIT) {
#if !defined(SUPPORT_HIRESO)
		// ToDo: Both Pseudo BIOS.
		uint32_t haltvalue = 0;
		uint32_t haltvalue_v30 = 1;
		if(cpu_mode) {
			haltvalue = 1;
			haltvalue_v30 = 0;
		}
		if(d_v30 != NULL) {
			d_v30->write_signal(SIG_CPU_BUSREQ, haltvalue_v30, 1);
		}
		d_cpu->write_signal(SIG_CPU_BUSREQ, haltvalue, 1);
		event_wait = -1;
#endif
 	}
}

#if !defined(SUPPORT_HIRESO)
void CPUREG::set_intr_line(bool line, bool pending, uint32_t bit)
{
	if(d_v30 != NULL) {
		if(((reg_0f0 & 1) != 0)){
			d_v30->set_intr_line(line, pending, bit);
		} else {
			d_cpu->set_intr_line(line, pending, bit);
		}
	} else {
//		if(cpu_mode == 0) {
			d_cpu->set_intr_line(line, pending, bit);
//		}
	}
}

void CPUREG::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_CPU_NMI) {
		out_debug_log("NMI\n");
		//if(nmi_enabled) {
			write_signals(&outputs_nmi, data);
		//}
	} else if(ch == SIG_CPUREG_RESET) {
		// This don't need at PC9801?
//		out_debug_log("RESET FROM CPU!!!\n");
	}
}

#endif

#define STATE_VERSION	3

bool CPUREG::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(reg_0f0);
#if !defined(SUPPORT_HIRESO)
	state_fio->StateValue(cpu_mode);
#endif
	state_fio->StateValue(nmi_enabled);
	state_fio->StateValue(init_clock);
	state_fio->StateValue(event_wait);
	return true;
}

}
