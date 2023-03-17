/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.06.10 -

	[ memory ]
*/

#include "membus.h"

void MEMBUS::initialize()
{
	MEMORY::initialize();
	
	// init memory
	memset(ram, 0, sizeof(ram));
	memset(vram, 0, sizeof(vram));
	memset(ipl, 0xff, sizeof(ipl));
	memset(learn, 0xff, sizeof(learn));
	memset(dic, 0xff, sizeof(dic));
	memset(kanji, 0xff, sizeof(kanji));
	memset(romdrv, 0xff, sizeof(romdrv));
#ifdef _PC98HA
	memset(ramdrv, 0, sizeof(ramdrv));
	for(int i = 0; i < sizeof(memcard); i++) {
		memcard[i] = ((i & 1) ? (i >> 8) : i) & 0xff;
	}
#endif
	for(int i = 0; i < sizeof(rdmy); i++) {
		rdmy[i] = ((i & 1) ? (i >> 8) : i) & 0xff;
	}
	
	// load rom/ram images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("BACKUP.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(learn, sizeof(learn), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("DICT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(dic, sizeof(dic), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, sizeof(kanji), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("ROMDRV.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(romdrv, sizeof(romdrv), 1);
		fio->Fclose();
	}
#ifdef _PC98HA
	if(fio->Fopen(create_local_path(_T("RAMDRV.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(ramdrv, sizeof(ramdrv), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("MEMCARD.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(memcard, sizeof(memcard), 1);
		fio->Fclose();
	}
#endif
	delete fio;
	
	learn_crc32 = get_crc32(learn, sizeof(learn));
#ifdef _PC98HA
	ramdrv_crc32 = get_crc32(ramdrv, sizeof(ramdrv));
	memcard_crc32 = get_crc32(memcard, sizeof(memcard));
#endif
}

void MEMBUS::release()
{
	// save ram images
	FILEIO* fio = new FILEIO();
	if(learn_crc32 != get_crc32(learn, sizeof(learn))) {
		if(fio->Fopen(create_local_path(_T("BACKUP.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(learn, sizeof(learn), 1);
			fio->Fclose();
		}
	}
#ifdef _PC98HA
	if(ramdrv_crc32 != get_crc32(ramdrv, sizeof(ramdrv))) {
		if(fio->Fopen(create_local_path(_T("RAMDRV.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(ramdrv, sizeof(ramdrv), 1);
			fio->Fclose();
		}
	}
	if(memcard_crc32 != get_crc32(memcard, sizeof(memcard))) {
		if(fio->Fopen(create_local_path(_T("MEMCARD.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(memcard, sizeof(memcard), 1);
			fio->Fclose();
		}
	}
#endif
	delete fio;
}

void MEMBUS::reset()
{
	// set memory bank
	learn_bank = dic_bank = kanji_bank = romdrv_bank = 0;
#ifdef _PC98HA
	ramdrv_bank = 0;
	ramdrv_sel = 0x81;
	ems_bank[0] = 0; ems_bank[1] = 1; ems_bank[2] = 2; ems_bank[3] = 3;
#endif
	update_bank();
}

#ifdef _PC98HA
void MEMBUS::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	MEMORY::write_data8w(addr, data, wait);
	
	// patch for pcmcia
	if(ram[0x59e] == 0x3e) {
		ram[0x59e] &= ~0x20;
	}
}
#endif

void MEMBUS::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
#ifdef _PC98HA
	case 0x8e1:
		ems_bank[0] = data & 0x7f;
		update_bank();
		break;
	case 0x8e3:
		ems_bank[1] = data & 0x7f;
		update_bank();
		break;
	case 0x8e5:
		ems_bank[2] = data & 0x7f;
		update_bank();
		break;
	case 0x8e7:
		ems_bank[3] = data & 0x7f;
		update_bank();
		break;
	case 0x0c10:
		learn_bank = data & 0x0f;
		update_bank();
		break;
	case 0x0e8e:
		ramdrv_bank = data & 0x7f;
		update_bank();
		break;
	case 0x1e8e:
		ramdrv_sel = data;
		update_bank();
		break;
	case 0x4c10:
		dic_bank = data & 0x3f;
		update_bank();
		break;
	case 0xcc10:
		romdrv_bank = data & 0x0f;
		update_bank();
		break;
#else
	case 0x0c10:
		learn_bank = data & 3;
		update_bank();
		break;
	case 0x4c10:
		dic_bank = data & 0x1f;
		update_bank();
		break;
	case 0xcc10:
		romdrv_bank = data & 7;
		update_bank();
		break;
#endif
	case 0x8c10:
		kanji_bank = data & 0x0f;
		update_bank();
		break;
	}
}

uint32_t MEMBUS::read_io8(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0x0c10:
		return learn_bank | 0x40;
	case 0x4c10:
		return dic_bank | 0x40;
	case 0x8c10:
		return kanji_bank | 0x40;
	case 0xcc10:
		return romdrv_bank | 0x40;
	}
	return 0xff;
}

void MEMBUS::update_bank()
{
	for(uint32_t addr = 0; addr < 0x100000; addr += 0x10000) {
		set_memory_r(addr, addr + 0xffff, rdmy);
	}
	unset_memory_w(0x00000, 0xfffff);
	
	set_memory_rw(0x00000, 0x9ffff, ram);
	set_memory_rw(0xa8000, 0xaffff, vram);
#ifdef _PC98HA
	set_memory_rw(0xc0000, 0xc3fff, ems + 0x4000 * ems_bank[0]);
	set_memory_rw(0xc4000, 0xc7fff, ems + 0x4000 * ems_bank[1]);
	set_memory_rw(0xc8000, 0xcbfff, ems + 0x4000 * ems_bank[2]);
	set_memory_rw(0xcc000, 0xcffff, ems + 0x4000 * ems_bank[3]);
#endif
	set_memory_rw(0xd0000, 0xd3fff, learn + 0x4000 * learn_bank);
	if(dic_bank < 48) {
		set_memory_r(0xd4000, 0xd7fff, dic + 0x4000 * dic_bank);
	}
	set_memory_r(0xd8000, 0xdbfff, kanji + 0x4000 * kanji_bank);
#ifdef _PC98HA
	if(ramdrv_sel == 0x80) {
		// ???
	} else if(ramdrv_sel == 0x81 && ramdrv_bank < 88) {
		set_memory_rw(0xdc000, 0xdffff, ramdrv + 0x4000 * ramdrv_bank);
	} else if(ramdrv_sel == 0x82) {
		// memory card
		set_memory_rw(0xdc000, 0xdffff, memcard + 0x4000 * ramdrv_bank);
	}
#endif
	if(romdrv_bank < 16) {
		set_memory_r(0xe0000, 0xeffff, romdrv + 0x10000 * romdrv_bank);
	}
	set_memory_r(0xf0000, 0xfffff, ipl);
}

void MEMBUS::draw_screen()
{
	// draw to real screen
	scrntype_t cd = RGB_COLOR(48, 56, 16);
	scrntype_t cb = RGB_COLOR(160, 168, 160);
	int ptr = 0;
	
	for(int y = 0; y < 400; y++) {
		scrntype_t* dest = emu->get_screen_buffer(y);
		for(int x = 0; x < 640; x += 8) {
			uint8_t pat = vram[ptr++];
			dest[x + 0] = (pat & 0x80) ? cd : cb;
			dest[x + 1] = (pat & 0x40) ? cd : cb;
			dest[x + 2] = (pat & 0x20) ? cd : cb;
			dest[x + 3] = (pat & 0x10) ? cd : cb;
			dest[x + 4] = (pat & 0x08) ? cd : cb;
			dest[x + 5] = (pat & 0x04) ? cd : cb;
			dest[x + 6] = (pat & 0x02) ? cd : cb;
			dest[x + 7] = (pat & 0x01) ? cd : cb;
		}
	}
}

#define STATE_VERSION	2

bool MEMBUS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateArray(learn, sizeof(learn), 1);
#ifdef _PC98HA
	state_fio->StateArray(ramdrv, sizeof(ramdrv), 1);
	state_fio->StateArray(ems, sizeof(ems), 1);
	state_fio->StateArray(memcard, sizeof(memcard), 1);
#endif
	state_fio->StateValue(learn_crc32);
#ifdef _PC98HA
	state_fio->StateValue(ramdrv_crc32);
	state_fio->StateValue(memcard_crc32);
#endif
	state_fio->StateValue(learn_bank);
	state_fio->StateValue(dic_bank);
	state_fio->StateValue(kanji_bank);
	state_fio->StateValue(romdrv_bank);
#ifdef _PC98HA
	state_fio->StateValue(ramdrv_bank);
	state_fio->StateValue(ramdrv_sel);
	state_fio->StateArray(ems_bank, sizeof(ems_bank), 1);
#endif
	
	// post process
	if(loading) {
		update_bank();
	}
	return true;
}

