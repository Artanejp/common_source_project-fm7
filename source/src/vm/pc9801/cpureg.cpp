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

#define EVENT_WAIT 1

namespace PC9801 {
#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM)
#define UPPER_I386 1
#endif

#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM) || defined(HAS_I286)
void CPUREG::initialize()
{
	use_v30 = false;
}

void CPUREG::halt_by_use_v30()
{
	if(use_v30) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		d_v30cpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
	} else {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
		d_v30cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
}
#endif

void CPUREG::halt_by_value(bool val)
{
	bool haltvalue = (val) ? 0xffffffff : 0x0000000;
#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM) || defined(HAS_I286)
	if(use_v30) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 0xffffffff, 0xffffffff);
		d_v30cpu->write_signal(SIG_CPU_BUSREQ, haltvalue, 0xffffffff);
	} else {
		d_cpu->write_signal(SIG_CPU_BUSREQ, haltvalue, 0xffffffff);
		d_v30cpu->write_signal(SIG_CPU_BUSREQ, 0xffffffff, 0xffffffff);
	}
#else
	d_cpu->write_signal(SIG_CPU_BUSREQ, haltvalue, 0xffffffff);
#endif	
}

void CPUREG::reset()
{
	d_cpu->set_address_mask(0x000fffff);
	init_clock = get_current_clock_uint64() & 0x000000ffffffffff;
	nmi_enabled = false;
	stat_wait = false;
	stat_exthalt = false;
	reg_0f0 = 0;
	if(event_wait >= 0) {
		cancel_event(this, event_wait);
		event_wait = -1;
	}
	d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM) || defined(HAS_I286)
	use_v30 = ((config.cpu_type & 0x02) != 0) ? true : false;
	halt_by_use_v30();
#endif
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
//		uint8_t reset_reg = d_pio->read_signal(SIG_I8255_PORT_C);
//		reset_reg = reset_reg & (uint8_t)(~0x20); // Reset SHUT1
//		d_pio->write_signal(SIG_I8255_PORT_C, reset_reg, 0xff);
		d_cpu->set_address_mask(0x000fffff);
		halt_by_use_v30();
	} else if(ch == SIG_CPUREG_HALT) {
		stat_exthalt = ((data & mask) != 0);
		halt_by_value(stat_exthalt);
	} else if(ch == SIG_CPUREG_USE_V30) {
#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM) || defined(HAS_I286)
		use_v30 = ((data & mask) != 0);
		halt_by_use_v30();
#endif
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
	case 0x005f:
		// ToDo: Both Pseudo BIOS.
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM) || defined(HAS_I286)
		d_v30cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
#endif
		stat_wait = true;
		if(event_wait >= 0) {
			cancel_event(this, event_wait);
			event_wait = -1;
		}
		register_event(this, EVENT_WAIT, 0.6, false, &event_wait);
		break;
	case 0x00f0:
		{
//			uint8_t reset_reg = d_pio->read_signal(SIG_I8255_PORT_C);
//			reset_reg = reset_reg & (uint8_t)(~0x20); // Reset SHUT1
//			d_pio->write_signal(SIG_I8255_PORT_C, reset_reg, 0xff);
			// ToDo: Reflesh
			reg_0f0 = data;
#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM) || defined(HAS_I286)
			use_v30 = ((data & 1) != 0);
			d_v30cpu->reset();
			// halt_by_use_v30();
#endif
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
#if defined(UPPER_I386)
	case 0x00f6:
		switch(data) {
		case 0x02:
#if defined(SUPPORT_32BIT_ADDRESS)
			d_cpu->set_address_mask(0xffffffff);
#else
			d_cpu->set_address_mask(0x00ffffff);
#endif
			break;
		case 0x03:
			d_cpu->set_address_mask(0x000fffff);
			break;
			// ToDo: Software DIPSWITCH.
		case 0xa0:
		case 0xe0:
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
		value |= 0x10; // Unknown
		value |= 0x08;
//		value |= ((d_mem->read_signal(SIG_LAST_ACCESS_INTERAM) != 0) ? 0x00: 0x08); // RAM access, 1 = Internal-standard/External-enhanced RAM, 0 = Internal-enhanced RAM
		value |= 0x04; // Refresh mode, 1 = Standard, 0 = High speed
//#if defined(HAS_I86) || defined(HAS_V30)
		// ToDo: Older VMs.
		//value |= ((reg_0f0 & 0x01) == 0) ? 0x00 : 0x02; // CPU mode, 1 = V30, 0 = 80286/80386
		value |= 0x00; // CPU mode, 1 = V30, 0 = 80286/80386
//#endif
		value |= 0x01; // RAM access, 1 = Internal RAM, 0 = External-enhanced RAM
		return value;
	case 0x00f2:
		return ((d_cpu->get_address_mask() & (1 << 20)) ? 0x00 : 0x01) | 0xfe;
		break;
	case 0x00f4: // ToDo: DMA SPEED (after 9801DA)
		break;
#if defined(UPPER_I386)
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


void CPUREG::event_callback(int id, int err)
{
	if(id == EVENT_WAIT) {
		// ToDo: Both Pseudo BIOS.
		if(!(stat_exthalt)) {
			
#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM) || defined(HAS_I286)
			halt_by_use_v30();
#else
			d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
#endif
		}
		stat_wait = false;
		event_wait = -1;
	}
}
	
#define STATE_VERSION	3

bool CPUREG::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(nmi_enabled);
	state_fio->StateValue(init_clock);
	state_fio->StateValue(stat_wait);
	state_fio->StateValue(stat_exthalt);
	state_fio->StateValue(reg_0f0);	
	state_fio->StateValue(event_wait);
#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM) || defined(HAS_I286)
	state_fio->StateValue(use_v30);
#endif	
 	return true;
}

}

