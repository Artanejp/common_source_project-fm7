/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.17-

	[ main pcb ]
*/

#include "./main.h"
#include "./sub.h"

#define SET_BANK_W(s, e, w) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		} \
	} \
}

#define SET_BANK_R(s, e, r, w) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x1000 * (i - sb); \
		} \
		wait[i] = w; \
	} \
}

void MAIN::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK_W(0x0000, 0xffff, ram);
	SET_BANK_R(0x0000, 0xffff, ram, 0);
}

void MAIN::reset()
{
	rom_sel = true;
	update_memory_map();
	slot_sel = slot_exp[0] = slot_exp[1] = 0;
	intr_mask = intr_request = intr_in_service = 0;
}

void MAIN::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32_t MAIN::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[addr >> 12][addr & 0xfff];
}

/*
void MAIN::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	*wait = 0;
	write_data8(addr, data);
}

uint32_t MAIN::read_data8w(uint32_t addr, int *wait)
{
	addr &= 0xffff;
	*wait = wait[addr >> 12];
	return read_data8(addr);
}
*/

void MAIN::write_io8(uint32_t addr, uint32_t data)
{
#ifdef _IO_DEBUG_LOG
	this->out_debug_log(_T("%06x\tOUT8\t%04x,%02x\n"), get_cpu_pc(0), addr, data);
#endif
	switch(addr & 0xffe0) {
	case 0xff00:
	case 0xff20:
	case 0xff40:
	case 0xff60:
		slot_exp[slot_sel] = data & 0x0f;
		break;
	case 0xff80:
		if(intr_mask != data) {
			if(!(intr_mask & 0x80) && (data & 0x80)) {
				d_sub->write_signal(SIG_SUB_INT2, 1, 1);
				// FIXME: ugly patch for floppy drives
				intr_request &= ~0x10;
			} else if((intr_mask & 0x80) && !(data & 0x80)) {
				d_sub->write_signal(SIG_SUB_INT2, 0, 1);
			}
			intr_mask = data;
			update_intr();
		}
		break;
	case 0xffa0:
		rom_sel = ((data & 2) == 0);
		update_memory_map();
		slot_sel = data & 1;
		break;
	case 0xffc0:
		d_sub->write_signal(SIG_SUB_COMM, data, 0xff);
		break;
	case 0xffe0:
		break;
	default:
		if(slot_exp[slot_sel] < 4) {
			d_slot[slot_sel][slot_exp[slot_sel]]->write_io8(addr & 0xffff, data);
		}
		break;
	}
}

uint32_t MAIN::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	switch(addr & 0xffe0) {
	case 0xff80:
	case 0xffa0:
	case 0xffc0:
	case 0xffe0:
		val = comm_data;
		break;
	default:
		if(slot_exp[slot_sel] < 4) {
			val = d_slot[slot_sel][slot_exp[slot_sel]]->read_io8(addr & 0xffff);
		}
		break;
	}
#ifdef _IO_DEBUG_LOG
	this->out_debug_log(_T("%06x\tIN8\t%04x = %02x\n"), get_cpu_pc(0), addr, val);
#endif
	return val;
}

void MAIN::write_io8w(uint32_t addr, uint32_t data, int *wait)
{
	*wait = 1;
	write_io8(addr, data);
}

uint32_t MAIN::read_io8w(uint32_t addr, int *wait)
{
	*wait = 1;
	return read_io8(addr);
}

static const uint8_t bits[5] = {
	0x10, 0x01, 0x02, 0x04, 0x08
};
static const uint32_t vector[5] = {
	0xf0, 0xf2, 0xf4, 0xf6, 0xf8
};

void MAIN::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_MAIN_INTS:
	case SIG_MAIN_INTA:
	case SIG_MAIN_INTB:
	case SIG_MAIN_INTC:
	case SIG_MAIN_INTD:
		if(data & mask) {
			if(!(intr_request & bits[id])) {
				intr_request |= bits[id];
				update_intr();
			}
		} else {
			if(intr_request & bits[id]) {
				intr_request &= ~bits[id];
				update_intr();
			}
		}
		break;
	case SIG_MAIN_COMM:
		comm_data = data & 0xff;
		break;
	}
}

void MAIN::update_memory_map()
{
	if(rom_sel) {
		SET_BANK_R(0x0000, 0x8fff, rom, 24);
	} else {
		SET_BANK_R(0x0000, 0x8fff, ram, 0);
	}
}

void MAIN::update_intr()
{
	for(int i = 0; i < 5; i++) {
		if((intr_request & bits[i]) && (intr_mask & bits[i]) && !(intr_in_service & bits[i])) {
			d_cpu->set_intr_line(true, true, 0);
			return;
		}
	}
	d_cpu->set_intr_line(false, true, 0);
}

uint32_t MAIN::get_intr_ack()
{
	for(int i = 0; i < 5; i++) {
		if((intr_request & bits[i]) && (intr_mask & bits[i]) && !(intr_in_service & bits[i])) {
			intr_request &= ~bits[i];
			intr_in_service |= bits[i];
			return vector[i];
		}
	}
	// invalid interrupt status
	return 0xff;
}

void MAIN::notify_intr_reti()
{
	notify_intr_ei();
}

void MAIN::notify_intr_ei()
{
	// FP-1100 uses EI and RET to leave interrupt routine
	for(int i = 0; i < 5; i++) {
		if(intr_in_service & bits[i]) {
			intr_in_service &= ~bits[i];
			update_intr();
			break;
		}
	}
}

#define STATE_VERSION	3

bool MAIN::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(comm_data);
	state_fio->StateValue(rom_sel);
	state_fio->StateValue(slot_sel);
	state_fio->StateArray(slot_exp, sizeof(slot_exp), 1);
	state_fio->StateValue(intr_mask);
	state_fio->StateValue(intr_request);
	state_fio->StateValue(intr_in_service);
	
	// post process
	if(loading) {
		update_memory_map();
	}
	return true;
}

