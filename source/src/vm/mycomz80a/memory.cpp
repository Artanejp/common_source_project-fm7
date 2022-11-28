/*
	Japan Electronics College MYCOMZ-80A Emulator 'eMYCOMZ-80A'

	Author : Takeda.Toshiya
	Date   : 2009.05.13-

	[ memory ]
*/

#include "memory.h"
#include "../i8255.h"
#include "../mb8877.h"

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
#define SET_BANK_R(s, e, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x1000 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// init memory
	memset(ram, 0, sizeof(ram));
	memset(bios, 0xff, sizeof(bios));
	memset(basic, 0xff, sizeof(basic));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("BIOS.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(bios, sizeof(bios), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(basic, sizeof(basic), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK_W(0x0000, 0xffff, ram);
	SET_BANK_R(0x0000, 0xffff, ram);
}

void MEMORY::reset()
{
	addr_mask = 0xc000;
	rom_sel = true;
	drv_sel = nmi_req = 0;
	update_memory_map();
}

uint32_t MEMORY::fetch_op(uint32_t addr, int *wait)
{
	if(nmi_req && --nmi_req == 0) {
		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
	}
	*wait = 0;
	return read_data8(addr);
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr = (addr & 0xffff) | addr_mask;
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr = (addr & 0xffff) | addr_mask;
	return rbank[addr >> 12][addr & 0xfff];
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	// $00: system control (74LS259)
	switch(data & 0xff) {
	case 0x00:
		addr_mask = 0xc000;
		break;
	case 0x01:
		addr_mask = 0;
		break;
	case 0x02:
		rom_sel = true;
		update_memory_map();
		break;
	case 0x03:
		rom_sel = false;
		update_memory_map();
		break;
	case 0x04: // fds0 = 0
	case 0x05: // fds0 = 1
	case 0x06: // fds1 = 0
	case 0x07: // fds1 = 1
	case 0x08: // fds2 = 0
	case 0x09: // fds2 = 1
		if(data & 1) {
			drv_sel |=  (1 << ((data - 4) >> 1));
		} else {
			drv_sel &= ~(1 << ((data - 4) >> 1));
		}
		if(drv_sel & 4) {
			d_fdc->write_signal(SIG_MB8877_DRIVEREG, drv_sel, 3);
		}
		d_pio->write_signal(SIG_I8255_PORT_A, (drv_sel == 4) ? 0 : 0xff, 0x04);
		d_pio->write_signal(SIG_I8255_PORT_A, (drv_sel == 5) ? 0 : 0xff, 0x08);
		d_pio->write_signal(SIG_I8255_PORT_A, (drv_sel == 6) ? 0 : 0xff, 0x10);
		d_pio->write_signal(SIG_I8255_PORT_A, (drv_sel == 7) ? 0 : 0xff, 0x20);
		break;
	case 0x0a: // fds3 = 0
	case 0x0b: // fds3 = 1
		d_fdc->write_signal(SIG_MB8877_SIDEREG, data, 1);
		break;
	case 0x0c: // fds4 = 0
	case 0x0d: // fds4 = 1
		d_fdc->write_signal(SIG_MB8877_MOTOR, data, 1);
		break;
	case 0x0e: // fds5 = 0
		d_fdc->reset();
		break;
	case 0x0f: // fds5 = 1
		break;
	case 0x80:
		nmi_req = 3;
		break;
	}
}

void MEMORY::update_memory_map()
{
	if(rom_sel) {
		SET_BANK_R(0xc000, 0xefff, bios);
		SET_BANK_R(0xf000, 0xffff, basic);
	} else {
		SET_BANK_R(0xc000, 0xffff, ram + 0xc000);
	}
}

#define STATE_VERSION	2

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(addr_mask);
	state_fio->StateValue(rom_sel);
	state_fio->StateValue(drv_sel);
	state_fio->StateValue(nmi_req);
	
	// post process
	if(loading) {
		update_memory_map();
	}
	return true;
}

