/*
	BANDAI RX-78 Emulator 'eRX-78'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ memory ]
*/

#include "memory.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x1000 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	memset(ram, 0, sizeof(ram));
	memset(ext, 0, sizeof(ext));
	memset(vram, 0, sizeof(vram));
	memset(ipl, 0xff, sizeof(ipl));
	memset(cart, 0xff, sizeof(cart));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load ipl
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	SET_BANK(0x0000, 0x1fff, wdmy, ipl );
	SET_BANK(0x2000, 0x5fff, wdmy, cart);
	SET_BANK(0x6000, 0xafff, ext,  ext );
	SET_BANK(0xb000, 0xefff, ram,  ram );
	SET_BANK(0xf000, 0xffff, wdmy, rdmy);
	
	for(int i = 0; i < 6; i++) {
		vbank[i] = vram + 0x2000 * i;
	}
	rpage = wpage = 0;
	inserted = false;
}

void MEMORY::reset()
{
	memset(ram, 0, sizeof(ram));
	memset(ext, 0, sizeof(ext));
	memset(vram, 0, sizeof(vram));
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	// known bug : ram must not be mapped to $ec00-$eebf
	addr &= 0xffff;
	if(addr < 0xeec0) {
		wbank[addr >> 12][addr & 0xfff] = data;
		return;
	}
	if(wpage & 0x01) {
		vbank[0][addr - 0xeec0] = data;
	}
	if(wpage & 0x02) {
		vbank[1][addr - 0xeec0] = data;
	}
	if(wpage & 0x04) {
		vbank[2][addr - 0xeec0] = data;
	}
	if(wpage & 0x08) {
		vbank[3][addr - 0xeec0] = data;
	}
	if(wpage & 0x10) {
		vbank[4][addr - 0xeec0] = data;
	}
	if(wpage & 0x20) {
		vbank[5][addr - 0xeec0] = data;
	}
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	// known bug : ram must not be mapped to $ec00-$eebf
	addr &= 0xffff;
	if(addr < 0xeec0) {
		return rbank[addr >> 12][addr & 0xfff];
	}
	if(1 <= rpage && rpage <= 6) {
		return vbank[rpage - 1][addr - 0xeec0];
	}
	return 0xff;
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xf1:
		rpage = data;
		break;
	case 0xf2:
		wpage = data;
		break;
	}
}

void MEMORY::open_cart(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(cart, 0xff, sizeof(cart));
		fio->Fread(cart, sizeof(cart), 1);
		fio->Fclose();
		inserted = true;
	}
	delete fio;
}

void MEMORY::close_cart()
{
	memset(cart, 0xff, sizeof(cart));
	inserted = false;
}

#define STATE_VERSION	1

#include "../../statesub.h"

void MEMORY::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_1D_ARRAY(ram, sizeof(ram));
	DECL_STATE_ENTRY_1D_ARRAY(ext, sizeof(ext));
	DECL_STATE_ENTRY_1D_ARRAY(vram, sizeof(vram));
	DECL_STATE_ENTRY_INT32(rpage);
	DECL_STATE_ENTRY_INT32(wpage);
	DECL_STATE_ENTRY_BOOL(inserted);
	
	leave_decl_state();
}

void MEMORY::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->Fwrite(ram, sizeof(ram), 1);
//	state_fio->Fwrite(ext, sizeof(ext), 1);
//	state_fio->Fwrite(vram, sizeof(vram), 1);
//	state_fio->FputInt32(rpage);
//	state_fio->FputInt32(wpage);
//	state_fio->FputBool(inserted);
}

bool MEMORY::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		return false;
	}
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	state_fio->Fread(ram, sizeof(ram), 1);
//	state_fio->Fread(ext, sizeof(ext), 1);
//	state_fio->Fread(vram, sizeof(vram), 1);
//	rpage = state_fio->FgetInt32();
//	wpage = state_fio->FgetInt32();
//	inserted = state_fio->FgetBool();
	return true;
}

