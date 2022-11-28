/*
	SHARP MZ-3500 Emulator 'EmuZ-3500'

	Author : Takeda.Toshiya
	Date   : 2010.08.31-

	[ main pcb ]
*/

#include "./main.h"
#include "../upd765a.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x800 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x800 * (i - sb); \
		} \
	} \
}

void MAIN::initialize()
{
	// init memory
	memset(ipl, 0xff, sizeof(ipl));
	memset(ram, 0, sizeof(ram));
	memset(common, 0, sizeof(common));
	memset(basic, 0xff, sizeof(basic));
	memset(ext, 0xff, sizeof(ext));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(basic, sizeof(basic), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("EXT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ext, sizeof(ext), 1);
		fio->Fclose();
	}
	delete fio;
	
	crt_400line = (config.monitor_type == 0 || config.monitor_type == 1);
}

void MAIN::reset()
{
	// memory mapper
	ms = ma = mo = 0;
	me1 = me2 = false;
	update_bank();
	
	// sub cpu
	srqb = 2;
	sres = 0;
	sack = true;
	srdy = false;
	
	// interrupt
	intfd = int0 = int1 = int2 = int3 = int4 = false;
	me = e1 = false;
	inp = 0;
	
	// mfd
	drq = index = false;
}

void MAIN::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32_t MAIN::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[addr >> 11][addr & 0x7ff];
}

uint32_t MAIN::fetch_op(uint32_t addr, int *wait)
{
	// mz3500sm p.23
	*wait = 1;
	return read_data8(addr);
}

void MAIN::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xec:	// mz3500sm p.17
	case 0xed:
	case 0xee:
	case 0xef:
		if(int0) {
			int0 = false;
			update_irq();
		}
		break;
	case 0xf8:	// mz3500sm p.59
	case 0xfa:
		if(data & 0x40) {
			for(int i = 0; i < 3; i++) {
				if(data & (1 << i)) {
//					this->out_debug_log(_T("MAIN->FDC\tDRIVE=%d\n"), i);
					d_fdc->write_signal(SIG_UPD765A_DRVSEL, i, 3);
					break;
				}
			}
		}
		d_fdc->write_signal(SIG_UPD765A_MOTOR, data, 0x10);
		d_fdc->write_signal(SIG_UPD765A_TC, data, 0x20);
		motor = ((data & 0x10) != 0);
		{
			bool new_me = ((data & 0x80) != 0);
			if(me != new_me) {
				me = new_me;
				update_irq();
			}
		}
		break;
	case 0xf9:	// mz3500sm p.59
	case 0xfb:
		d_fdc->write_dma_io8(1, data);
		break;
	case 0xfc:	// mz3500sm p.23
		if((srqb & 2) != (data & 2)) {
//			this->out_debug_log(_T("MAIN->SUB\tBUSREQ=%d\n"), (data&2)?1:0);
			d_subcpu->write_signal(SIG_CPU_BUSREQ, data, 2);
		}
		srqb = data;
		{
			bool new_e1 = ((data & 1) != 0);
			if(e1 != new_e1) {
				e1 = new_e1;
				update_irq();
			}
		}
		break;
	case 0xfd:	// mz3500sm p.23
		if(!(sres & 0x80) && (data & 0x80)) {
//			this->out_debug_log(_T("MAIN->SUB\tRESET\n"));
			d_subcpu->reset();
		}
		sres = data;
		{
			uint8_t new_ms = data & 3;
			if(ms != new_ms) {
				ms = new_ms;
				update_bank();
			}
		}
		break;
	case 0xfe:	// mz3500sm p.23
		{
			uint8_t new_mo = data & 7;
			uint8_t new_ma = (data >> 4) & 0x0f;
			if(mo != new_mo || ma != new_ma) {
				mo = new_mo;
				ma = new_ma;
				update_bank();
			}
		}
		break;
	case 0xff:	// mz3500sm p.23
		{
			bool new_me1 = ((data & 1) != 0);
			bool new_me2 = ((data & 2) != 0);
			if(me1 != new_me1 || me2 != new_me2) {
				me1 = new_me1;
				me2 = new_me2;
				update_bank();
			}
		}
		break;
	}
}

uint32_t MAIN::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	
	switch(addr & 0xff) {
	case 0xec:	// mz3500sm p.17
	case 0xed:
	case 0xee:
	case 0xef:
		if(int0) {
			int0 = false;
			update_irq();
		}
		break;
	case 0xf8:	// mz3500sm p.59
	case 0xfa:
		return 0xf8 | (drq ? 1 : 0) | (index ? 2 : 0) | (motor ? 4 : 0);
	case 0xf9:	// mz3500sm p.59
	case 0xfb:
		return d_fdc->read_dma_io8(1);
	case 0xfc:	// mz3500sm p.23
		return srqb;
	case 0xfd:	// mz3500sm p.23
		return sres;
	case 0xfe:	// mz3500sm p.23,85-86
		// bit4: sw4=on,  select period for decimal point
		// bit3: sw3=on,  select hiresolution crt
		// bit2: sw2=off, select MZ-1P02
		// bit1: sw1=on
		// bit0: sec=on,  select double side mini floppy
		return 0xe0 | (((~config.dipswitch) & 0x0b) << 1) | (crt_400line ? 0 : 8);
	case 0xff:	// mz3500sm p.23,85-86
		// bit7: fd3=off, select double side mini floppy
		// bit6: fd2=off, select double side mini floppy
		// bit5: fd1=off, normally small letter and in capital letter when shifted
		return 0xc0 | ((config.dipswitch & 0x80) ? 0 : 0x20) | (srdy ? 0x10 : 0) | (sack ? 0 : 8) | (inp & 7);
	}
	return 0xff;
}

void MAIN::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MAIN_SACK) {
		sack = ((data & mask) != 0);
//		this->out_debug_log(_T("SUB->MAIN\tSACK=%d\n"), sack?1:0);
	} else if(id == SIG_MAIN_SRDY) {
		srdy = ((data & mask) != 0);
//		this->out_debug_log(_T("SUB->MAIN\tSRDY=%d\n"), srdy?1:0);
	} else if(id == SIG_MAIN_INTFD) {
		intfd = ((data & mask) != 0);
//		this->out_debug_log(_T("FDC->MAIN\tINTFD=%d\n"), intfd?1:0);
		update_irq();
	} else if(id == SIG_MAIN_INT0) {
		int0 = ((data & mask) != 0);
//		this->out_debug_log(_T("SUB->MAIN\tINT0=%d\n"), int0?1:0);
		update_irq();
	} else if(id == SIG_MAIN_INT1) {
		int1 = ((data & mask) != 0);
		update_irq();
	} else if(id == SIG_MAIN_INT2) {
		int2 = ((data & mask) != 0);
		update_irq();
	} else if(id == SIG_MAIN_INT3) {
		int3 = ((data & mask) != 0);
		update_irq();
	} else if(id == SIG_MAIN_INT4) {
		int4 = ((data & mask) != 0);
		update_irq();
	} else if(id == SIG_MAIN_DRQ) {
		drq = ((data & mask) != 0);
	} else if(id == SIG_MAIN_INDEX) {
		index = ((data & mask) != 0);
	}
}

void MAIN::update_irq()
{
	bool next = false;
	
	if(intfd && me) {
		intfd = false;
		inp = 0;
		next = true;
	} else if(e1) {
		if(int0) {
			inp = 1;
			next = true;
		} else if(int1) {
			inp = 2;
			next = true;
		} else if(int2) {
			inp = 3;
			next = true;
		} else if(int3) {
			inp = 4;
			next = true;
		} else if(int4) {
			inp = 5;
			next = true;
		}
	}
	if(next) {
//		this->out_debug_log(_T("MAIN IRQ=%d SRC=%d\n"), next?1:0,inp);
		d_maincpu->set_intr_line(true, true, 0);
	}
}

void MAIN::update_bank()
{
	SET_BANK(0x0000, 0xffff, wdmy, rdmy);
	
	if((ms & 3) == 0) {
		// SD0: INITIALIZE STATE, mz3500sm p.7
		SET_BANK(0x0000, 0x0fff, wdmy, ipl + 0x1000);
		SET_BANK(0x1000, 0x1fff, wdmy, ipl + 0x1000);
		SET_BANK(0x2000, 0x3fff, wdmy, basic + 0x2000);
		if(!me1) {
			SET_BANK(0x4000, 0x7fff, ram + 0x4000, ram + 0x4000);
		}
		if(!me2) {
			SET_BANK(0x8000, 0xbfff, ram + 0x8000, ram + 0x8000);
		}
		switch(ma & 0x0f) {
		case 0x00: SET_BANK(0xc000, 0xffff, ram + 0x0c000, ram + 0x0c000); break;
		case 0x01: SET_BANK(0xc000, 0xffff, ram + 0x00000, ram + 0x00000); break;
		case 0x0f: SET_BANK(0xf800, 0xffff, common, common); break;
		}
	} else if((ms & 3) == 1) {
		// SD1: SYSTEM LOADING & CP/M, mz3500sm p.9
		SET_BANK(0x0000, 0xf7ff, ram, ram);
		SET_BANK(0xf800, 0xffff, common, common);
	} else if((ms & 3) == 2) {
		// SD2: ROM based BASIC, mz3500sm p.10
		SET_BANK(0x0000, 0x1fff, wdmy, basic);
		switch(mo & 0x07) {
		case 0x00: SET_BANK(0x2000, 0x3fff, wdmy, basic + 0x2000); break;
		case 0x01: SET_BANK(0x2000, 0x3fff, wdmy, basic + 0x4000); break;
		case 0x02: SET_BANK(0x2000, 0x3fff, wdmy, basic + 0x6000); break;
		case 0x03: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x0000); break;
		case 0x04: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x2000); break;
		case 0x05: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x4000); break;
		case 0x06: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x6000); break;
		}
		if(!me1) {
			SET_BANK(0x4000, 0x7fff, ram + 0x4000, ram + 0x4000);
		}
		if(!me2) {
			SET_BANK(0x8000, 0xbfff, ram + 0x8000, ram + 0x8000);
		}
		switch(ma & 0x0f) {
		case 0x00: SET_BANK(0xc000, 0xffff, ram + 0x0c000, ram + 0x0c000); break;
		case 0x01: SET_BANK(0xc000, 0xffff, ram + 0x00000, ram + 0x00000); break;
		case 0x02: SET_BANK(0xc000, 0xffff, ram + 0x10000, ram + 0x10000); break;
		case 0x03: SET_BANK(0xc000, 0xffff, ram + 0x14000, ram + 0x14000); break;
		case 0x04: SET_BANK(0xc000, 0xffff, ram + 0x18000, ram + 0x18000); break;
		case 0x05: SET_BANK(0xc000, 0xffff, ram + 0x1c000, ram + 0x1c000); break;
		case 0x06: SET_BANK(0xc000, 0xffff, ram + 0x20000, ram + 0x20000); break;
		case 0x07: SET_BANK(0xc000, 0xffff, ram + 0x24000, ram + 0x24000); break;
		case 0x08: SET_BANK(0xc000, 0xffff, ram + 0x28000, ram + 0x28000); break;
		case 0x09: SET_BANK(0xc000, 0xffff, ram + 0x2c000, ram + 0x2c000); break;
		case 0x0a: SET_BANK(0xc000, 0xffff, ram + 0x30000, ram + 0x30000); break;
		case 0x0b: SET_BANK(0xc000, 0xffff, ram + 0x34000, ram + 0x34000); break;
		case 0x0c: SET_BANK(0xc000, 0xffff, ram + 0x38000, ram + 0x38000); break;
		case 0x0d: SET_BANK(0xc000, 0xffff, ram + 0x3c000, ram + 0x3c000); break;
		case 0x0f: SET_BANK(0xf800, 0xffff, common, common); break;
		}
	} else {
		// SD3: RAM based BASIC, mz3500sm p.11
		SET_BANK(0x0000, 0x1fff, ram, ram);
		switch(mo & 0x07) {
		case 0x0: SET_BANK(0x2000, 0x3fff, ram + 0x2000, ram + 0x2000); break;
		case 0x1: SET_BANK(0x2000, 0x3fff, ram + 0xc000, ram + 0xc000); break;
		case 0x2: SET_BANK(0x2000, 0x3fff, ram + 0xe000, ram + 0xe000); break;
		case 0x3: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x0000); break;
		case 0x4: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x2000); break;
		case 0x5: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x4000); break;
		case 0x6: SET_BANK(0x2000, 0x3fff, wdmy, ext + 0x6000); break;
		}
		if(!me1) {
			SET_BANK(0x4000, 0x7fff, ram + 0x4000, ram + 0x4000);
		}
		if(!me2) {
			SET_BANK(0x8000, 0xbfff, ram + 0x8000, ram + 0x8000);
		}
		switch(ma & 0x0f) {
		case 0x00: SET_BANK(0xc000, 0xffff, ram + 0x10000, ram + 0x10000); break;
		case 0x01: SET_BANK(0xc000, 0xffff, ram + 0x14000, ram + 0x14000); break;
		case 0x02: SET_BANK(0xc000, 0xffff, ram + 0x18000, ram + 0x18000); break;
		case 0x03: SET_BANK(0xc000, 0xffff, ram + 0x1c000, ram + 0x1c000); break;
		case 0x04: SET_BANK(0xc000, 0xffff, ram + 0x20000, ram + 0x20000); break;
		case 0x05: SET_BANK(0xc000, 0xffff, ram + 0x24000, ram + 0x24000); break;
		case 0x06: SET_BANK(0xc000, 0xffff, ram + 0x28000, ram + 0x28000); break;
		case 0x07: SET_BANK(0xc000, 0xffff, ram + 0x2c000, ram + 0x2c000); break;
		case 0x08: SET_BANK(0xc000, 0xffff, ram + 0x30000, ram + 0x30000); break;
		case 0x09: SET_BANK(0xc000, 0xffff, ram + 0x34000, ram + 0x34000); break;
		case 0x0a: SET_BANK(0xc000, 0xffff, ram + 0x38000, ram + 0x38000); break;
		case 0x0b: SET_BANK(0xc000, 0xffff, ram + 0x3c000, ram + 0x3c000); break;
		case 0x0f: SET_BANK(0xf800, 0xffff, common, common); break;
		}
	}
}

#define STATE_VERSION	2

bool MAIN::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(common, sizeof(common), 1);
	state_fio->StateValue(ma);
	state_fio->StateValue(ms);
	state_fio->StateValue(mo);
	state_fio->StateValue(me1);
	state_fio->StateValue(me2);
	state_fio->StateValue(srqb);
	state_fio->StateValue(sres);
	state_fio->StateValue(sack);
	state_fio->StateValue(srdy);
	state_fio->StateValue(intfd);
	state_fio->StateValue(int0);
	state_fio->StateValue(int1);
	state_fio->StateValue(int2);
	state_fio->StateValue(int3);
	state_fio->StateValue(int4);
	state_fio->StateValue(me);
	state_fio->StateValue(e1);
	state_fio->StateValue(inp);
	state_fio->StateValue(motor);
	state_fio->StateValue(drq);
	state_fio->StateValue(index);
	state_fio->StateValue(crt_400line);
	
	// post process
	if(loading) {
		 update_bank();
	}
	return true;
}

