/*
	GAKKEN TV BOY Emulator 'yaTVBOY'

	Author : tanam
	Date   : 2020.06.13

	[ memory ]
*/

#include "memory.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 10, eb = (e) >> 10; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x400 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x400 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));	
	// set memory map
	SET_BANK(0x0000, 0x0fff, ram,  ram );
	SET_BANK(0x1000, 0x1fff, vram, vram);
	SET_BANK(0x2000, 0xefff, wdmy, rdmy);
	SET_BANK(0xf000, 0xffff, wdmy, rom );
	// register event
	register_event_by_clock(this, 0, 256, true, NULL);
	event = false;
	inserted = false;
}

void MEMORY::reset()
{
	memset(ram, 0, sizeof(ram));
	for (int i=0; i<sizeof(vram); i++) {
		vram[i]=rand() % 256;
	}
	d_vdp->write_signal(SIG_MC6847_AS,     0x00, 0x08);
	d_vdp->write_signal(SIG_MC6847_AG,     0x10, 0x10);
	d_vdp->write_signal(SIG_MC6847_CSS,    0x20, 0x20);
	d_vdp->write_signal(SIG_MC6847_GM,     0x00, 0x02);
	d_vdp->write_signal(SIG_MC6847_GM,     0x01, 0x01);
	d_vdp->write_signal(SIG_MC6847_INTEXT, 0x00, 0x04);
	shot1 = shot2 = up = down = left = right = 0;
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	if(addr >= 0x80 && addr < 0x100) {
		d_cpu->ram[addr-0x80]=data;
	}
    if(addr == 0x2000) {
		d_vdp->write_signal(SIG_MC6847_AS,     data, 0x08);
		d_vdp->write_signal(SIG_MC6847_AG,     data, 0x10);
		d_vdp->write_signal(SIG_MC6847_CSS,    data, 0x20);
		d_vdp->write_signal(SIG_MC6847_GM,     data << 1, 0x02);
		d_vdp->write_signal(SIG_MC6847_GM,     data >> 1, 0x01);
		d_vdp->write_signal(SIG_MC6847_INTEXT, data, 0x04);
		return;
	}
	wbank[addr >> 10][addr & 0x3ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	if(addr >= 0x80 && addr < 0x100) {
		return d_cpu->ram[addr-0x80];
	}
	return rbank[addr >> 10][addr & 0x3ff];
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	d_cpu->write_signal(SIG_MC6801_PORT_2, 0x1E, 0x1E);
	if (shot2==1 && d_cpu->port[0].wreg==1) {
		d_cpu->write_signal(SIG_MC6801_PORT_2, 0x00, 0x04);
	}
	if (down==1 && d_cpu->port[0].wreg==2) {
		d_cpu->write_signal(SIG_MC6801_PORT_2, 0x00, 0x04);
	}
	if (shot1==1 && d_cpu->port[0].wreg==1) {
		d_cpu->write_signal(SIG_MC6801_PORT_2, 0x00, 0x02);
	}
	if (up==1 && d_cpu->port[0].wreg==2) {
		d_cpu->write_signal(SIG_MC6801_PORT_2, 0x00, 0x02);
	}
	if (left==1 && d_cpu->port[0].wreg==2) {
		d_cpu->write_signal(SIG_MC6801_PORT_2, 0x00, 0x08);
	}
	if (right==1 && d_cpu->port[0].wreg==2) {
		d_cpu->write_signal(SIG_MC6801_PORT_2, 0x00, 0x10);
	}
}

void MEMORY::event_callback(int event_id, int err)
{
	if (event)	{
		d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
		event = false;
	} else {
		d_cpu->write_signal(SIG_CPU_IRQ, 0, 1);
		event = true;
	}
}

void MEMORY::key_down(int code)
{
	if (code==0x20) {
		shot1 =1;
	}
	if (code==0x11) {
		shot2 =1;
	}
	if (code==0x25) {
		left =1;
	}
	if (code==0x26) {
		up =1;
	}
	if (code==0x27) {
		right =1;
	}
	if (code==0x28) {
		down =1;
	}
}

void MEMORY::key_up(int code)
{
	if (code==0x20) {
		shot1 =0;
	}
	if (code==0x11) {
		shot2 =0;
	}
	if (code==0x25) {
		left =0;
	}
	if (code==0x26) {
		up =0;
	}
	if (code==0x27) {
		right =0;
	}
	if (code==0x28) {
		down =0;
	}
}

void MEMORY::open_cart(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
		inserted = true;
	}
	delete fio;
}

void MEMORY::close_cart()
{
	memset(rom, 0xff, sizeof(rom));
	inserted = false;
}

#define STATE_VERSION	1

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(vram, sizeof(vram), 1);
	return true;
}
