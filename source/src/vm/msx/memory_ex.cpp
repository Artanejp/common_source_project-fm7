/*
	Common Source Code Project
	MSX Series (experimental)

	Origin : src/vm/msx/memory.cpp

	modified by umaiboux
	Date   : 2016.03.xx-

	[ memory ]
*/

#include "memory_ex.h"
#if defined(LDC_SLOT)
#include "../ld700.h"
#include "../tms9918a.h"
#endif
#if defined(FDD_PATCH_SLOT)
#include "../disk.h"
#if defined(MSX_FDD_PATCH_WITH_2HD)
#define MSX_SECTOR_SIZE (disk[drv]->sector_size.sd)
#else
#define MSX_SECTOR_SIZE 512
#endif
#endif
#if defined(FDD_PATCH_SLOT) && defined(_MSX_VDP_MESS)
#include "../v9938.h"
#endif

#ifdef USE_MEGAROM
//#include "scc.h"
#include "romtype1.h"

#define EVENT_ROMTYPE	1
#define ROMTYPE_NONE	(-1)
static struct s_typestr {
	int type;
	const _TCHAR *p;
} typestr[] = {
	{ROM_ASCII8,		_T("ASCII8")},
	{ROM_ASCII16,		_T("ASCII16")},
	{ROM_KONAMI,		_T("KONAMI")},
	{ROM_KONAMI_SCC,	_T("KONAMI_SCC")},
	{ROMTYPE_NONE,		_T("NONE")}
};

#include "romtype2.h"
#endif

#define EVENT_CLOCK	0

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 13, eb = (e) >> 13; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x2000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x2000 * (i - sb); \
		} \
	} \
}

#if defined(FDD_PATCH_SLOT)
static const struct {
	int sectors;
	uint8_t heads, names, per_track, per_fat, per_cluster;
} info[8] = {
	{ 720,  1, 112, 9, 2, 2 },
	{ 1440, 2, 112, 9, 3, 2 },
	{ 640,  1, 112, 8, 1, 2 },
	{ 1280, 2, 112, 8, 2, 2 },
	{ 360,  1, 64,  9, 2, 1 },
	{ 720,  2, 112, 9, 2, 2 },
	{ 320,  1, 64,  8, 1, 1 },
	{ 640,  2, 112, 8, 1, 2 }
};

static const uint8_t boot_block[] = {
	0xeb, 0xfe, 0x90, 0x56, 0x46, 0x42, 0x2d, 0x31, 0x39, 0x38, 0x39, 0x00, 0x02, 0x02, 0x01, 0x00,
	0x02, 0x70, 0x00, 0xa0, 0x05, 0xf9, 0x03, 0x00, 0x09, 0x00, 0x02, 0x00, 0x00, 0x00, 0xd0, 0xed,
	0x53, 0x58, 0xc0, 0x32, 0xc2, 0xc0, 0x36, 0x55, 0x23, 0x36, 0xc0, 0x31, 0x1f, 0xf5, 0x11, 0x9d,
	0xc0, 0x0e, 0x0f, 0xcd, 0x7d, 0xf3, 0x3c, 0x28, 0x28, 0x11, 0x00, 0x01, 0x0e, 0x1a, 0xcd, 0x7d,
	0xf3, 0x21, 0x01, 0x00, 0x22, 0xab, 0xc0, 0x21, 0x00, 0x3f, 0x11, 0x9d, 0xc0, 0x0e, 0x27, 0xcd,
	0x7d, 0xf3, 0xc3, 0x00, 0x01, 0x57, 0xc0, 0xcd, 0x00, 0x00, 0x79, 0xe6, 0xfe, 0xfe, 0x02, 0x20,
	0x07, 0x3a, 0xc2, 0xc0, 0xa7, 0xca, 0x22, 0x40, 0x11, 0x77, 0xc0, 0x0e, 0x09, 0xcd, 0x7d, 0xf3,
	0x0e, 0x07, 0xcd, 0x7d, 0xf3, 0x18, 0xb4, 0x42, 0x6f, 0x6f, 0x74, 0x20, 0x65, 0x72, 0x72, 0x6f,
	0x72, 0x0d, 0x0a, 0x50, 0x72, 0x65, 0x73, 0x73, 0x20, 0x61, 0x6e, 0x79, 0x20, 0x6b, 0x65, 0x79,
	0x20, 0x66, 0x6f, 0x72, 0x20, 0x72, 0x65, 0x74, 0x72, 0x79, 0x0d, 0x0a, 0x24, 0x00, 0x4d, 0x53,
	0x58, 0x44, 0x4f, 0x53, 0x20, 0x20, 0x53, 0x59, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf3, 0x2a,
	0x51, 0xf3, 0x11, 0x00, 0x01, 0x19, 0x01, 0x00, 0x01, 0x11, 0x00, 0xc1, 0xed, 0xb0, 0x3a, 0xee,
	0xc0, 0x47, 0x11, 0xef, 0xc0, 0x21, 0x00, 0x00, 0xcd, 0x51, 0x52, 0xf3, 0x76, 0xc9, 0x18, 0x64,
	0x3a, 0xaf, 0x80, 0xf9, 0xca, 0x6d, 0x48, 0xd3, 0xa5, 0x0c, 0x8c, 0x2f, 0x9c, 0xcb, 0xe9, 0x89,
	0xd2, 0x00, 0x32, 0x26, 0x40, 0x94, 0x61, 0x19, 0x20, 0xe6, 0x80, 0x6d, 0x8a, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static int DSKIO = -1, DSKCHG = -1, GETDPB = -1, DSKFMT = -1;
#endif

bool SLOT_CART::load_cart(const _TCHAR *file_path/*, uint8_t *rom*/)
{
	bool result = false;
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(rom, 0xff, 0x10000);
		
		fio->Fseek(0, FILEIO_SEEK_END);
		uint32_t file_size = fio->Ftell();
		fio->Fseek(0, FILEIO_SEEK_SET);
		
#ifdef USE_MEGAROM
		type = ROMTYPE_NONE;
		SET_BANK(0x0000, 0xffff, wdmy, rom);
		bank_scc = false;
		d_sound->disable_all();
#endif
		if(file_size <= 0x2000) {
			// 8KB: 00000000
			fio->Fread(rom, 0x2000, 1);
			memcpy(rom + 0x2000, rom, 0x2000);
			memcpy(rom + 0x4000, rom, 0x4000);
			memcpy(rom + 0x8000, rom, 0x8000);
		} else if(file_size <= 0x4000) {
			// 16KB: 01010101
			fio->Fread(rom, 0x4000, 1);
			memcpy(rom + 0x4000, rom, 0x4000);
			memcpy(rom + 0x8000, rom, 0x8000);
		} else if(file_size <= 0x8000) {
			// 32KB: 01012323
			fio->Fread(rom + 0x4000, 0x8000, 1);
			memcpy(rom + 0x0000, rom + 0x4000, 0x4000);
			memcpy(rom + 0xc000, rom + 0x8000, 0x4000);
		} else {
#ifdef USE_MEGAROM
			if(file_size > 0x10000) {
				if(file_size > sizeof(rom)) file_size = sizeof(rom);
				fio->Fread(rom, file_size, 1);
				type = hashRomType(rom, file_size);
				if (ROMTYPE_NONE == type) type = guessRomType(rom, file_size);
				switch(type) {
				case ROM_ASCII8:
				case ROM_ASCII16:
				case ROM_KONAMI:
				case ROM_KONAMI_SCC:
					break;
				default:
					type = ROMTYPE_NONE;
					break;
				}
				//register_event(this, EVENT_ROMTYPE, 1000000.0 * 5, false, NULL);
				if (-1 != event_register_id) {
					cancel_event(this, event_register_id);
					event_register_id = -1;
				}
				register_event(this, EVENT_ROMTYPE, 1000000.0 * 5, true, &event_register_id);
				SET_BANK(0x0000, 0x3fff, wdmy, rdmy);
				SET_BANK(0xc000, 0xffff, wdmy, rdmy);
				SET_BANK(0x4000, 0xbfff, wdmy, rom);
				if(file_size <= 0x20000) {
					memcpy(rom + 0x20000, rom, 0x20000);
				}
				if(file_size <= 0x40000) {
					memcpy(rom + 0x40000, rom, 0x40000);
				}
				if(file_size <= 0x80000) {
					memcpy(rom + 0x80000, rom, 0x80000);
				}
				if(file_size <= 0x100000) {
					memcpy(rom + 0x100000, rom, 0x100000);
				}
				if(file_size <= 0x200000) {
					memcpy(rom + 0x200000, rom, 0x200000);
				}

				if (ROM_KONAMI_SCC == type) {
					d_sound->enable_c(SOUND_CHIP_SCC, true);
				}
				// for IKARI(ASCII 16Kb)
				if (ROM_ASCII16 == type) {
					SET_BANK(0x4000, 0x7fff, wdmy, rom+0x2000*(0*2));
					SET_BANK(0x8000, 0xbfff, wdmy, rom+0x2000*(0*2));
				}
			}
			else {
				// 64KB: 01234567
				fio->Fread(rom, 0x10000, 1);
			}
#else
			// 64KB: 01234567
			fio->Fread(rom, 0x10000, 1);
#endif
		}
		fio->Fclose();
		result = true;
	}
	delete fio;
	return result;
}

// MAIN ROM 32K
// or MAIN ROM 32K + MAIN RAM 32K

void SLOT_MAINROM::initialize()
{
	memset(rom, 0xff, sizeof(rom));
#ifdef MAINROM_PLUS_RAM_32K
	memset(ram, 0, sizeof(ram));
#endif
	FILEIO* fio = new FILEIO();
#if defined(_MSX2P_VARIANTS)
	if(fio->Fopen(create_local_path(_T("MSX2P.ROM")), FILEIO_READ_BINARY) ||
#elif defined(_MSX2_VARIANTS)
	if(fio->Fopen(create_local_path(_T("MSX2J.ROM")), FILEIO_READ_BINARY) ||
	   fio->Fopen(create_local_path(_T("MSX2.ROM" )), FILEIO_READ_BINARY) ||
#else
	if(fio->Fopen(create_local_path(_T("MSXJ.ROM")), FILEIO_READ_BINARY) ||
	   fio->Fopen(create_local_path(_T("MSX.ROM" )), FILEIO_READ_BINARY) ||
#endif
	   fio->Fopen(create_local_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK(0x0000, 0x7fff, wdmy, rom);
#ifdef MAINROM_PLUS_RAM_32K
	SET_BANK(0x8000, 0xffff, ram, ram);
#else
	SET_BANK(0x8000, 0xffff, wdmy, rdmy);
#endif
}

void SLOT_MAINROM::write_data8(uint32_t addr, uint32_t data)
{
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t SLOT_MAINROM::read_data8(uint32_t addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}

#define SLOT_MAINROM_STATE_VERSION	1

bool SLOT_MAINROM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(SLOT_MAINROM_STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#ifdef MAINROM_PLUS_RAM_32K
	state_fio->StateArray(ram, sizeof(ram), 1);
#endif
	return true;
}

// Cart

void SLOT_CART::initialize()
{
	memset(rdmy, 0xff, sizeof(rdmy));
	close_cart();
}

void SLOT_CART::write_data8(uint32_t addr, uint32_t data)
{
#ifdef USE_MEGAROM
	if (ROM_KONAMI == type) {
		// Konami without SCC
		addr &= 0xffff;
		if ((0x6000 <=addr) && (addr < 0x8000)) {
			SET_BANK(0x6000, 0x7fff, wdmy, rom+0x2000*(data));
		}
		if ((0x8000 <=addr) && (addr < 0xa000)) {
			SET_BANK(0x8000, 0x9fff, wdmy, rom+0x2000*(data));
		}
		if ((0xa000 <=addr) && (addr < 0xc000)) {
			SET_BANK(0xa000, 0xbfff, wdmy, rom+0x2000*(data));
		}
		//wbank[addr >> 13][addr & 0x1fff] = data;
	}
	else if (ROM_KONAMI_SCC == type) {
		// Konami with SCC
		addr &= 0xffff;
		if ((0x5000 <=addr) && (addr < 0x5800)) {
			SET_BANK(0x4000, 0x5fff, wdmy, rom+0x2000*(data));
		}
		if ((0x7000 <=addr) && (addr < 0x7800)) {
			SET_BANK(0x6000, 0x7fff, wdmy, rom+0x2000*(data));
		}
		if ((0x9000 <=addr) && (addr < 0x9800)) {
			SET_BANK(0x8000, 0x9fff, wdmy, rom+0x2000*(data));
			bank_scc = ((data&0x3F)==0x3F);
		}
		if ((0xb000 <=addr) && (addr < 0xb800)) {
			SET_BANK(0xa000, 0xbfff, wdmy, rom+0x2000*(data));
		}
		if ((bank_scc) && (0x9000 <=addr) && (addr < 0x9900)) {
			d_sound->write_data8_c(SOUND_CHIP_SCC, addr & 0xFFFF, data);
		}
		//wbank[addr >> 13][addr & 0x1fff] = data;
	}
	else if (ROM_ASCII8 == type) {
		// ASCII 8Kb
		addr &= 0xffff;
		if ((0x6000 <=addr) && (addr < 0x6800)) {
			SET_BANK(0x4000, 0x5fff, wdmy, rom+0x2000*(data));
		}
		if ((0x6800 <=addr) && (addr < 0x7000)) {
			SET_BANK(0x6000, 0x7fff, wdmy, rom+0x2000*(data));
		}
		if ((0x7000 <=addr) && (addr < 0x7800)) {
			SET_BANK(0x8000, 0x9fff, wdmy, rom+0x2000*(data));
		}
		if ((0x7800 <=addr) && (addr < 0x8000)) {
			SET_BANK(0xa000, 0xbfff, wdmy, rom+0x2000*(data));
		}
		//wbank[addr >> 13][addr & 0x1fff] = data;
	}
	else if (ROM_ASCII16 == type) {
		// ASCII 16Kb
		data &= 0x0f;
		addr &= 0xffff;
		if ((0x6000 <=addr) && (addr < 0x6800)) {
			//SET_BANK(0x4000, 0x5fff, wdmy, rom+0x2000*(data*2));
			//SET_BANK(0x6000, 0x7fff, wdmy, rom+0x2000*(data*2+1));
			SET_BANK(0x4000, 0x7fff, wdmy, rom+0x2000*(data*2));
		}
		if ((0x7000 <=addr) && (addr < 0x7800)) {
			//SET_BANK(0x8000, 0x9fff, wdmy, rom+0x2000*(data*2));
			//SET_BANK(0xa000, 0xbfff, wdmy, rom+0x2000*(data*2+1));
			SET_BANK(0x8000, 0xbfff, wdmy, rom+0x2000*(data*2));
		}
		//wbank[addr >> 13][addr & 0x1fff] = data;
	}
	else /*if (ROMTYPE_NONE == type)*/ {
		wbank[addr >> 13][addr & 0x1fff] = data;
	}
#else
	wbank[addr >> 13][addr & 0x1fff] = data;
#endif
}

uint32_t SLOT_CART::read_data8(uint32_t addr)
{
#ifdef USE_MEGAROM
	addr &= 0xffff;
	if ((ROM_KONAMI_SCC == type) && (bank_scc) && (0x9000 <=addr) && (addr < 0x9900)) {
		return d_sound->read_data8_c(SOUND_CHIP_SCC, addr & 0xFFFF);
	}
	else {
		return rbank[addr >> 13][addr & 0x1fff];
	}
#else
	return rbank[addr >> 13][addr & 0x1fff];
#endif
}

void SLOT_CART::open_cart(const _TCHAR *file_path)
{
	if(load_cart(file_path/*, rom*/)) {
//		SET_BANK(0x0000, 0xffff, wdmy, rom);
		inserted = true;
	}
}

void SLOT_CART::close_cart()
{
#ifdef USE_MEGAROM
	type = ROMTYPE_NONE;
//	SET_BANK(0x0000, 0xffff, wdmy, rom);
	bank_scc = false;
	d_sound->disable_all();
#endif
	SET_BANK(0x0000, 0xffff, wdmy, rdmy);
	inserted = false;
}


#ifdef USE_MEGAROM
void SLOT_CART::event_callback(int event_id, int err)
{
	if(event_id == EVENT_ROMTYPE) {
		const _TCHAR *p;
		int i;
		for(i=0;; i++) {
			if ((typestr[i].type == type) || (typestr[i].type == ROMTYPE_NONE)) {
				p = typestr[i].p;
				break;
			}
		}
		emu->out_message(p);
		if (-1 != event_register_id) {
			cancel_event(this, event_register_id);
			event_register_id = -1;
		}
	}
}
#endif

#define SLOT_CART_STATE_VERSION	1

bool SLOT_CART::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(SLOT_CART_STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(inserted);
#ifdef USE_MEGAROM
	state_fio->StateValue(type);
	state_fio->StateValue(bank_scc);
	/* Todo: MEGA ROM bank select */
	/* is this OK? */
	if(loading) {
		if(inserted) {
			SET_BANK(0x0000, 0xffff, wdmy, rom);
			int i32;
			i32 = state_fio->FgetInt32_LE() ; rbank[0] = ((i32 == -1) ? rdmy : rom + i32);
			i32 = state_fio->FgetInt32_LE() ; rbank[1] = ((i32 == -1) ? rdmy : rom + i32);
			i32 = state_fio->FgetInt32_LE() ; rbank[2] = ((i32 == -1) ? rdmy : rom + i32);
			i32 = state_fio->FgetInt32_LE() ; rbank[3] = ((i32 == -1) ? rdmy : rom + i32);
			i32 = state_fio->FgetInt32_LE() ; rbank[4] = ((i32 == -1) ? rdmy : rom + i32);
			i32 = state_fio->FgetInt32_LE() ; rbank[5] = ((i32 == -1) ? rdmy : rom + i32);
			i32 = state_fio->FgetInt32_LE() ; rbank[6] = ((i32 == -1) ? rdmy : rom + i32);
			i32 = state_fio->FgetInt32_LE() ; rbank[7] = ((i32 == -1) ? rdmy : rom + i32);
		} else {
			SET_BANK(0x0000, 0xffff, wdmy, rdmy);
		}
	} else {
		if(inserted) {
			state_fio->FputInt32_LE(rbank[0]==rdmy ? (-1) : (int)(rbank[0] - rom));
			state_fio->FputInt32_LE(rbank[1]==rdmy ? (-1) : (int)(rbank[1] - rom));
			state_fio->FputInt32_LE(rbank[2]==rdmy ? (-1) : (int)(rbank[2] - rom));
			state_fio->FputInt32_LE(rbank[3]==rdmy ? (-1) : (int)(rbank[3] - rom));
			state_fio->FputInt32_LE(rbank[4]==rdmy ? (-1) : (int)(rbank[4] - rom));
			state_fio->FputInt32_LE(rbank[5]==rdmy ? (-1) : (int)(rbank[5] - rom));
			state_fio->FputInt32_LE(rbank[6]==rdmy ? (-1) : (int)(rbank[6] - rom));
			state_fio->FputInt32_LE(rbank[7]==rdmy ? (-1) : (int)(rbank[7] - rom));
		}
	}
#else
	// post process
	if(loading) {
		if(inserted) {
			SET_BANK(0x0000, 0xffff, wdmy, rom);
		} else {
			SET_BANK(0x0000, 0xffff, wdmy, rdmy);
		}
	}
#endif
	return true;
}

// MSXDOS2

#if defined(MSXDOS2_SLOT)
void SLOT_MSXDOS2::initialize()
{
	memset(rdmy, 0xff, sizeof(rdmy));
	/*close_cart();*/
	SET_BANK(0x0000, 0xffff, wdmy, rdmy);
	mapper[0] = mapper[1] = 4;
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("MSXDOS2.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
		mapper[0] = 0;
		mapper[1] = 3;
		SET_BANK(0x4000, 0x7fff, wdmy, rom + mapper[0] * 0x4000);
		SET_BANK(0x8000, 0xbfff, wdmy, rom + mapper[1] * 0x4000);
	}
	delete fio;
}

void SLOT_MSXDOS2::write_data8(uint32_t addr, uint32_t data)
{
	if(addr >= 0x4000 && addr < 0xc000 && mapper[0] < 4) {
		addr >>= 15;
		addr &= 1;
		data &= 3;
		if(mapper[addr] != data) {
			mapper[addr] = data;
			SET_BANK(addr * 0x4000 + 0x4000, addr * 0x4000 + 0x7fff, wdmy, rom + data * 0x4000);
		}
		return;
	}
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t SLOT_MSXDOS2::read_data8(uint32_t addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}

#define SLOT_MSXDOS2_STATE_VERSION	1

bool SLOT_MSXDOS2::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(SLOT_MSXDOS2_STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(mapper, sizeof(mapper), 1);
	
	// post process
	if(loading) {
		if(mapper[0] < 4) {
				SET_BANK(0x0000, 0x3fff, wdmy, rdmy);
				SET_BANK(0x4000, 0x7fff, wdmy, rom + mapper[0] * 0x4000);
				SET_BANK(0x8000, 0xbfff, wdmy, rom + mapper[1] * 0x4000);
				SET_BANK(0xc000, 0xffff, wdmy, rdmy);
		} else {
			SET_BANK(0x0000, 0xffff, wdmy, rdmy);
		}
	}
	return true;
}
#endif

// LD Control

#if defined(LDC_SLOT)
void SLOT_LDC::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("PX7EXT.ROM")), FILEIO_READ_BINARY) ||
	   fio->Fopen(create_local_path(_T("EXT.ROM")   ), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK(0x0000, 0x3fff, wdmy, rdmy);
	SET_BANK(0x4000, 0x5fff, wdmy, rom);
	SET_BANK(0x6000, 0xffff, wdmy, rdmy);
	
	clock = exv = ack = false;
	
	register_event(this, EVENT_CLOCK, 1000000.0 / 7812.5, true, NULL);
}

void SLOT_LDC::reset()
{
	super_impose = false;
	req_intr = false;
	pc4 = false;
	mute_l = mute_r = true;
	
	d_ldp->write_signal(SIG_LD700_MUTE_L, 1, 1);
	d_ldp->write_signal(SIG_LD700_MUTE_R, 1, 1);
	d_vdp->write_signal(SIG_TMS9918A_SUPER_IMPOSE, 0, 0);
}

void SLOT_LDC::write_data8(uint32_t addr, uint32_t data)
{
	if(addr == 0x7ffe) {
		d_ldp->write_signal(SIG_LD700_REMOTE, data, 1);
	} else if(addr == 0x7fff) {
		// super impose
		bool prev_super_impose = super_impose;
		super_impose = ((data & 1) == 0);
		if(super_impose) {
			if(req_intr && !prev_super_impose) {
				d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
			}
		} else {
			d_cpu->write_signal(SIG_CPU_IRQ, 0, 0);
		}
		d_vdp->write_signal(SIG_TMS9918A_SUPER_IMPOSE, super_impose ? 1 : 0, 1);
		
		// mute
		bool prev_mute_l = mute_l;
		mute_l = ((data & 0x80) == 0);
		if(!prev_mute_l && mute_l) {
			mute_r = !pc4;
		}
		d_ldp->write_signal(SIG_LD700_MUTE_L, mute_l ? 1 : 0, 1);
		d_ldp->write_signal(SIG_LD700_MUTE_R, mute_r ? 1 : 0, 1);
	} else {
		wbank[addr >> 13][addr & 0x1fff] = data;
	}
}

uint32_t SLOT_LDC::read_data8(uint32_t addr)
{
	if(addr == 0x7ffe) {
		return (clock ? 0 : 1) | (ack ? 0 : 0x80) | 0x7e;
	} else if(addr == 0x7fff) {
		uint32_t data = (req_intr ? 1 : 0) | (exv ? 0 : 0x80) | 0x7e;
		req_intr = false;
		d_cpu->write_signal(SIG_CPU_IRQ, 0, 0);
		return data;
	} else {
		return rbank[addr >> 13][addr & 0x1fff];
	}
}

void SLOT_LDC::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_LDC_EXV) {
		bool prev = exv;
		exv = ((data & mask) != 0);
		if(prev && !exv) {
			req_intr = true;
			if(super_impose) {
				d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
			}
		}
	} else if(id == SIG_LDC_ACK) {
		ack = ((data & mask) != 0);
	} else if(id == SIG_LDC_MUTE) {
		pc4 = ((data & mask) != 0);
	}
}

void SLOT_LDC::event_callback(int event_id, int err)
{
	if(event_id == EVENT_CLOCK) {
		clock = !clock;
	}
}

#define SLOT_LDC_STATE_VERSION	1

bool SLOT_LDC::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(SLOT_LDC_STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(clock);
	state_fio->StateValue(exv);
	state_fio->StateValue(ack);
	state_fio->StateValue(super_impose);
	state_fio->StateValue(req_intr);
	state_fio->StateValue(pc4);
	state_fio->StateValue(mute_l);
	state_fio->StateValue(mute_r);
	return true;
}
#endif

// SUBROM

#if defined(SUBROM_SLOT)
void SLOT_SUBROM::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	FILEIO* fio = new FILEIO();
#if defined(_MSX2_VARIANTS)
	if(fio->Fopen(create_local_path(_T("MSX2JEXT.ROM")), FILEIO_READ_BINARY) ||
	   fio->Fopen(create_local_path(_T("MSX2EXT.ROM" )), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
#elif defined(_MSX2P_VARIANTS)
	if(fio->Fopen(create_local_path(_T("MSX2PEXT.ROM" )), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("KNJDRV.ROM" )), FILEIO_READ_BINARY)) {
		fio->Fread(rom + 0x4000, sizeof(rom) - 0x4000, 1);
		fio->Fclose();
	}
#endif
	delete fio;
	
	SET_BANK(0x4000, 0xffff, wdmy, rdmy);
#if defined(_MSX2_VARIANTS)
	SET_BANK(0x0000, 0x3fff, wdmy, rom);
#elif defined(_MSX2P_VARIANTS)
	SET_BANK(0x0000, 0xbfff, wdmy, rom);
#endif
}

void SLOT_SUBROM::write_data8(uint32_t addr, uint32_t data)
{
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t SLOT_SUBROM::read_data8(uint32_t addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}
#endif

// FDD with ROM-PATCH

#if defined(FDD_PATCH_SLOT)
void SLOT_FDD_PATCH::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("DISK.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
	
	// patch for pseudo disk bios
	if(rom[0x0010] == 0xc3) {
		rom[(DSKIO  = rom[0x0011] + (int)rom[0x0012] * 256) - 0x4000] = 0xc9;
	}
	if(rom[0x0013] == 0xc3) {
		rom[(DSKCHG = rom[0x0014] + (int)rom[0x0015] * 256) - 0x4000] = 0xc9;
	}
	if(rom[0x0016] == 0xc3) {
		rom[(GETDPB = rom[0x0017] + (int)rom[0x0018] * 256) - 0x4000] = 0xc9;
	}
	if(rom[0x001c] == 0xc3) {
		rom[(DSKFMT = rom[0x001d] + (int)rom[0x001e] * 256) - 0x4000] = 0xc9;
	}
	if(rom[0x001f] == 0xc3) {
		rom[(            rom[0x0020] + (int)rom[0x0021] * 256) - 0x4000] = 0xc9;
	}
	SET_BANK(0x0000, 0xffff, wdmy, rdmy);
	SET_BANK(0x4000, 0x7fff, wdmy, rom);
}

void SLOT_FDD_PATCH::write_data8(uint32_t addr, uint32_t data)
{
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t SLOT_FDD_PATCH::read_data8(uint32_t addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}
#endif

// MAPPER RAM

#if defined(MAPPERRAM_SLOT)
void SLOT_MAPPERRAM::initialize()
{
	memset(ram, 0xff, sizeof(ram));
	/*close_cart();*/
	for(int i = 0; i < 4; i++) {
		mapper[i] = i;
	}
	SET_BANK(0x0000, 0xffff, ram, ram);
}

void SLOT_MAPPERRAM::write_data8(uint32_t addr, uint32_t data)
{
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t SLOT_MAPPERRAM::read_data8(uint32_t addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}

void SLOT_MAPPERRAM::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xfc:
	case 0xfd:
	case 0xfe:
	case 0xff:
		addr &= 3;
		data &= MAPPERRAM_MASK;
		if(mapper[addr] != data) {
			mapper[addr] = data;
			SET_BANK(addr * 0x4000, addr * 0x4000 + 0x3fff, ram + data * 0x4000, ram + data * 0x4000);
		}
		break;
	}
}

#define SLOT_MAPPERRAM_STATE_VERSION	1

bool SLOT_MAPPERRAM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(SLOT_MAPPERRAM_STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(mapper, sizeof(mapper), 1);
	
	// post process
	if(loading) {
		SET_BANK(0x0000, 0x3fff, ram + mapper[0] * 0x4000, ram + mapper[0] * 0x4000);
		SET_BANK(0x4000, 0x7fff, ram + mapper[1] * 0x4000, ram + mapper[1] * 0x4000);
		SET_BANK(0x8000, 0xbfff, ram + mapper[2] * 0x4000, ram + mapper[2] * 0x4000);
		SET_BANK(0xc000, 0xffff, ram + mapper[3] * 0x4000, ram + mapper[3] * 0x4000);
	}
	return true;
}
#endif

// NORMAL RAM 64K

#if defined(RAM64K_SLOT)
void SLOT_RAM64K::initialize()
{
	memset(ram, 0xff, sizeof(ram));
}

void SLOT_RAM64K::write_data8(uint32_t addr, uint32_t data)
{
	ram[addr & 0xffff] = data;
}

uint32_t SLOT_RAM64K::read_data8(uint32_t addr)
{
	return ram[addr & 0xffff];
}

#define SLOT_RAM64K_STATE_VERSION	1

bool SLOT_RAM64K::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(SLOT_RAM64K_STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}
#endif

// FIRMWARE 32K

#if defined(FIRMWARE32K1_SLOT)
void SLOT_FIRMWARE32K::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(m_filename), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK(0x0000, 0xffff, wdmy, rdmy);
	SET_BANK(0x4000, 0xbfff, wdmy, rom);
}

void SLOT_FIRMWARE32K::write_data8(uint32_t addr, uint32_t data)
{
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t SLOT_FIRMWARE32K::read_data8(uint32_t addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}
#endif

// MSX-MUSIC

#if defined(MSXMUSIC_SLOT)
void SLOT_MSXMUSIC::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
		
	SET_BANK(0x0000, 0xffff, wdmy, rom);
	FILEIO* fio = new FILEIO();
	if (fio->Fopen(create_local_path(_T("FMBIOS.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
		SET_BANK(0x0000, 0x3fff, wdmy, rdmy);
		SET_BANK(0x4000, 0x7fff, wdmy, rom);
		SET_BANK(0x8000, 0xffff, wdmy, rdmy);
	}
}

void SLOT_MSXMUSIC::write_data8(uint32_t addr, uint32_t data)
{
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t SLOT_MSXMUSIC::read_data8(uint32_t addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}
#endif

// memory bus

#if defined(FDD_PATCH_SLOT)
void MEMORY_EX::initialize()
{
	for(int i = 0; i < MAX_DRIVE; i++) {
		disk[i] = new DISK(emu);
		disk[i]->set_device_name(_T("%s/Disk #%d"), this_device_name, i + 1);
		disk[i]->drive_type = DRIVE_TYPE_2DD;
	}
}

void MEMORY_EX::release()
{
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(disk[i]) {
			disk[i]->close();
			delete disk[i];
		}
	}
}
#endif

void MEMORY_EX::reset()
{
#if defined(FDD_PATCH_SLOT)
	for(int i = 0; i < MAX_DRIVE; i++) {
		access[i] = false;
	}
#endif
	ssl[0] = ssl[1] = ssl[2] = ssl[3] = 0;
//	update_map((0 << 0) | (1 << 2) | (2 << 4) | (3 << 6));
	update_map(0);
}

void MEMORY_EX::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	if ((addr != 0xffff) || !expanded[(psl>>6)&3]) {
		d_map[addr >> 14]->write_data8(addr, data);
	}
	else {
		ssl[(psl>>6)&3] = data;
		update_map(psl);
	}
}

uint32_t MEMORY_EX::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	if ((addr != 0xffff) || !expanded[(psl>>6)&3]) {
		return d_map[addr >> 14]->read_data8(addr);
	}
	else {
		return 0xff ^ ssl[(psl>>6)&3];
	}
}

uint32_t MEMORY_EX::fetch_op(uint32_t addr, int* wait)
{
	*wait = 1;
	return read_data8(addr);
}

void MEMORY_EX::write_io8(uint32_t addr, uint32_t data)
{
#if defined(MAPPERRAM_SLOT)
//	if(d_map[3] == d_slot[3]) {
		d_mapper->write_io8(addr, data);
//	}
#endif
}

void MEMORY_EX::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_SEL) {
		if(psl != (data & mask)) {
			update_map(data & mask);
		}
	}
}

void MEMORY_EX::update_map(uint32_t val)
{
	d_map[0] = d_slot[((val >> 0) & 3) | (expanded[(val >> 0) & 3] ? ((ssl[((val >> 0) & 3)] << 2) & 0x0c) : 0)];
	d_map[1] = d_slot[((val >> 2) & 3) | (expanded[(val >> 2) & 3] ? ((ssl[((val >> 2) & 3)] >> 0) & 0x0c) : 0)];
	d_map[2] = d_slot[((val >> 4) & 3) | (expanded[(val >> 4) & 3] ? ((ssl[((val >> 4) & 3)] >> 2) & 0x0c) : 0)];
	d_map[3] = d_slot[((val >> 6) & 3) | (expanded[(val >> 6) & 3] ? ((ssl[((val >> 6) & 3)] >> 4) & 0x0c) : 0)];
	psl = val;
}

#if defined(FDD_PATCH_SLOT)
static int get_track_number(int desc, int sector)
{
	int trkside = sector / info[desc].per_track;
	return trkside >> (info[desc].heads - 1);
}

static int get_side_number(int desc, int sector)
{
	int trkside = sector / info[desc].per_track;
	return trkside & (info[desc].heads - 1);
}

static bool get_track(DISK *disk, int desc, int sector)
{
	return disk->get_track(get_track_number(desc, sector), get_side_number(desc, sector));
}

static bool get_sector(DISK *disk, int desc, int sector)
{
	static int last_index = 0;
	if(get_track(disk, desc, sector)) {
		for(int i = 0; i < disk->sector_num.sd; i++) {
			int j = (last_index + 2 + i) % (disk->sector_num.sd);
			disk->get_sector(-1, -1, j);
			if(disk->id[2] == (sector % info[desc].per_track) + 1) {
				last_index = j;
				return true;
			}
		}
	}
	return false;
}

static bool get_boot_sector(DISK *disk)
{
	if(disk->get_track(0, 0)) {
		for(int i = 0; i < disk->sector_num.sd; i++) {
			disk->get_sector(0, 0, i);
			if(disk->id[2] == 1) {
				return true;
			}
		}
	}
	return false;
}

#if defined(MSX_FDD_PATCH_WITH_2HD)
static bool get_sector_2(DISK *disk, int sector)
{
	if(get_boot_sector(disk)) {
		if (512 != disk->sector_size.sd) return false;
		int per_track = (int)disk->sector[0x19] * 256 + disk->sector[0x18];
		if (per_track <= 0) return false;
		int heads = (int)disk->sector[0x1b] * 256 + disk->sector[0x1a];
		if ((heads != 2)/* && (heads != 1)*/) return false;
		int trkside = sector / per_track;
		int track_number = trkside >> (heads - 1);
		int side_number = trkside & (heads - 1);
		if(disk->get_track(track_number, side_number)) {
			for(int i = 0; i < disk->sector_num.sd; i++) {
				disk->get_sector(-1, -1, i);
				if(disk->id[2] == (sector % per_track) + 1) {
					return true;
				}
			}
		}
	}
	return false;
}
#endif

uint32_t MEMORY_EX::read_signal(int id)
{
	uint32_t stat = 0;
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(access[i]) {
			stat |= 1 << i;
			access[i] = false;
		}
	}
	return stat;
}

bool MEMORY_EX::bios_ret_z80(uint16_t PC, pair32_t* af, pair32_t* bc, pair32_t* de, pair32_t* hl, pair32_t* ix, pair32_t* iy, uint8_t* iff1)
{
	#define AF	af->w.l
	#define A	af->b.h
	#define F	af->b.l
	#define BC	bc->w.l
	#define B	bc->b.h
	#define C	bc->b.l
	#define DE	de->w.l
	#define D	de->b.h
	#define E	de->b.l
	#define HL	hl->w.l
	#define H	hl->b.h
	#define L	hl->b.l
	
	#define CF	0x01
	
	if(d_map[1] == d_fdpat) {
		// pseudo disk bios from fMSX
		if(PC == DSKIO) {
#if defined(_MSX_VDP_MESS)
			d_vdp->write_signal(SIG_VDP_COMMAND_COMPLETION, 1, 1);
#endif
			// read/write sectors
			*iff1 |= 1;
			int desc = C & 7;
			int drv = A;
			int addr = HL;
			if(!(drv < MAX_DRIVE && disk[drv]->inserted)) {
				AF = 0x0201; // not ready
				return true;
			}
			if(F & CF) {
				if(disk[drv]->write_protected) {
					AF = 0x0001; // write protected
					return true;
				}
				for(int sector = DE; B != 0; sector++) {
#if defined(MSX_FDD_PATCH_WITH_2HD)
					if((MEDIA_TYPE_2HD == disk[drv]->media_type) || (MEDIA_TYPE_144 == disk[drv]->media_type)) {
						if(!get_sector_2(disk[drv], sector)) {
							AF = 0x0801; // record not found
							return true;
						}
					}
					else if(!get_sector(disk[drv], desc, sector)) {
						AF = 0x0801; // record not found
						return true;
					}
#else
					if(!get_sector(disk[drv], desc, sector)) {
						AF = 0x0801; // record not found
						return true;
					}
#endif
					access[drv] = true;
					
					if(disk[drv]->addr_crc_error && !disk[drv]->ignore_crc()) {
						AF = 0x0801; // record not found
						return true;
					}
					if(addr + MSX_SECTOR_SIZE/*disk[drv]->sector_size.sd*/ > 0xffff) {
						F &= ~CF;
						return true;
					}
					d_map[1] = d_slot[read_data8(0xf342) & 0x0f];
					for(int i = 0; i < MSX_SECTOR_SIZE/*disk[drv]->sector_size.sd*/; i++) {
						disk[drv]->sector[i] = read_data8(addr++);
					}
					d_map[1] = d_fdpat;
					B--;
				}
			} else {
				for(int sector = DE; B != 0; sector++) {
#if defined(MSX_FDD_PATCH_WITH_2HD)
					if((MEDIA_TYPE_2HD == disk[drv]->media_type) || (MEDIA_TYPE_144 == disk[drv]->media_type)) {
						if(!get_sector_2(disk[drv], sector)) {
							AF = 0x0801; // record not found
							return true;
						}
					}
					else if(!get_sector(disk[drv], desc, sector)) {
						AF = 0x0801; // record not found
						return true;
					}
#else
					if(!get_sector(disk[drv], desc, sector)) {
						AF = 0x0801; // record not found
						return true;
					}
#endif
					access[drv] = true;
					
					if(disk[drv]->addr_crc_error && !disk[drv]->ignore_crc()) {
						AF = 0x0801; // record not found
						return true;
					}
					if(addr + MSX_SECTOR_SIZE/*disk[drv]->sector_size.sd*/ > 0xffff) {
						F &= ~CF;
						return true;
					}
					d_map[1] = d_slot[read_data8(0xf342) & 0x0f];
					for(int i = 0; i < MSX_SECTOR_SIZE/*disk[drv]->sector_size.sd*/; i++) {
						write_data8(addr++, disk[drv]->sector[i]);
					}
					d_map[1] = d_fdpat;
					B--;
					if(disk[drv]->data_crc_error && !disk[drv]->ignore_crc()) {
						AF = 0x0401; // data crc error
						return true;
					}
				}
			}
			F &= ~CF;
			return true;
		} else if(PC == DSKCHG) {
			// detect disk changed
			*iff1 |= 1;
			int drv = A;
			if(!(drv < MAX_DRIVE && disk[drv]->inserted)) {
				AF = 0x0201; // not ready
				return true;
			}
			B = disk[drv]->changed ? 0xff : 0x01;
			disk[drv]->changed = false;
			F &= ~CF;
			return true;
		} else if(PC == GETDPB) {
			// get drive parameter block
			int drv = A;
			if(!(drv < MAX_DRIVE && disk[drv]->inserted)) {
				AF = 0x0201; // not ready
				return true;
			}
			if(!get_boot_sector(disk[drv])) {
				AF = 0x0c01; // other error
				return true;
			}
			access[drv] = true;
			
			if(disk[drv]->data_crc_error && !disk[drv]->ignore_crc()) {
				AF = 0x0401; // data crc error
				return true;
			}
			int bytes_per_sector = (int)disk[drv]->sector[0x0c] * 256 + disk[drv]->sector[0x0b];
			int sectors_per_disk = (int)disk[drv]->sector[0x14] * 256 + disk[drv]->sector[0x13];
			int sectors_per_fat  = (int)disk[drv]->sector[0x17] * 256 + disk[drv]->sector[0x16];
			int reserved_sectors = (int)disk[drv]->sector[0x0f] * 256 + disk[drv]->sector[0x0e];
			int addr = HL + 1, num, bits;
			d_map[1] = d_slot[read_data8(0xf342) & 0x0f];
			write_data8(addr++, disk[drv]->sector[0x15]);	// format id [f8h-ffh]
			write_data8(addr++, disk[drv]->sector[0x0b]);	// sector size
			write_data8(addr++, disk[drv]->sector[0x0c]);
			num = (bytes_per_sector >> 5) - 1;
			for(bits = 0; num & (1 << bits); bits++);
			write_data8(addr++, num);			// directory mask/shft
			write_data8(addr++, bits);
			num = disk[drv]->sector[0x0d] - 1;
			for(bits = 0; num & (1 << bits); bits++);
			write_data8(addr++, num);			// cluster mask/shift
			write_data8(addr++, bits + 1);
			write_data8(addr++, disk[drv]->sector[0x0e]);	// sector # of 1st fat
			write_data8(addr++, disk[drv]->sector[0x0f]);
			write_data8(addr++, disk[drv]->sector[0x10]);	// number of fats
			write_data8(addr++, disk[drv]->sector[0x11]);	// number of dirent-s
			num = reserved_sectors + disk[drv]->sector[0x10] * sectors_per_fat;
			num += 32 * disk[drv]->sector[0x11] / bytes_per_sector;
			write_data8(addr++, num & 0xff);		// sector # of data
			write_data8(addr++, (num >> 8) & 0xff);
			num = (sectors_per_disk - num) / disk[drv]->sector[0x0d];
			write_data8(addr++, num & 0xff);		// number of clusters
			write_data8(addr++, (num >> 8) & 0xff);
			write_data8(addr++, disk[drv]->sector[0x16]);	// sectors per fat
			num = reserved_sectors + disk[drv]->sector[0x10] * sectors_per_fat;
			write_data8(addr++, num & 0xff);		// sector # of dir.
			write_data8(addr, (num >> 8) & 0xff);
			d_map[1] = d_fdpat;
			F &= ~CF;
			return true;
		} else if(PC == DSKFMT) {
			// format disk
			*iff1 |= 1;
//			int desc = 2 - A;
			int desc = A - 1; // A: 1=single-side 2=double-side
			int drv = D;
			if(desc != 0 && desc != 1) {
				AF = 0x0c01; // bad parameter
				return true;
			}
			if(!(drv < MAX_DRIVE && disk[drv]->inserted)) {
				AF = 0x0201; // not ready
				return true;
			}
			if(disk[drv]->write_protected) {
				AF = 0x0001; // write protected
				return true;
			}
			access[drv] = true;
			
			// physical format
			int max_trkside = info[desc].sectors / info[desc].per_track;
			for(int trkside = 0; trkside < max_trkside; trkside++) {
				int trk = trkside >> (info[desc].heads - 1);
				int side = trkside & (info[desc].heads - 1);
				disk[drv]->format_track(trk, side);
				for(int sct = 0; sct < info[desc].per_track; sct++) {
					disk[drv]->insert_sector(trk, side, sct + 1, 2, false, false, 0, 512);
				}
			}
			// fill boot block with data:
			int sector = 0;
			get_sector(disk[drv], desc, sector++);
			memcpy(disk[drv]->sector, boot_block, 512);
			uint8_t *ptr = disk[drv]->sector + 3;
			memcpy(ptr, "fMSXdisk", 8); ptr += 10;		// manufacturer's id
			*ptr   = info[desc].per_cluster; ptr += 4;	// sectors per cluster
			*ptr++ = info[desc].names; *ptr++ = 0x00;	// number of names
			*ptr++ = info[desc].sectors & 0xff;		// number of sectors
			*ptr++ = (info[desc].sectors >> 8) & 0xff;
			*ptr++ = desc + 0xf8;				// format id [f8h-ffh]
			*ptr++ = info[desc].per_fat; *ptr++ = 0x00;	// sectors per fat
			*ptr++ = info[desc].per_track; *ptr++ = 0x00;	// sectors per track
			*ptr++ = info[desc].heads; *ptr = 0x00;		// number of heads
			// writing fats:
			for(int j = 0; j < 2; j++) {
				get_sector(disk[drv], desc, sector++);
				memset(disk[drv]->sector, 0x00, 512);
				disk[drv]->sector[0] = desc + 0xf8;
				disk[drv]->sector[1] = disk[drv]->sector[2] = 0xff;
				for(int i = info[desc].per_fat; i > 1; i--) {
					get_sector(disk[drv], desc, sector++);
					memset(disk[drv]->sector, 0x00, 512);
				}
			}
			for(int i = info[desc].names / 16; i; i--) {
				get_sector(disk[drv], desc, sector++);
				memset(disk[drv]->sector, 0x00, 512);
			}
			for(int i = info[desc].sectors - 2 * info[desc].per_fat - info[desc].names / 16 - 1; i; i--) {
				get_sector(disk[drv], desc, sector++);
				memset(disk[drv]->sector, 0xff, 512);
			}
			F &= ~CF;
			return true;
		}
	}
	return false;
}

void MEMORY_EX::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->open(file_path, bank);
	}
}

void MEMORY_EX::close_disk(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]->inserted) {
		disk[drv]->close();
	}
}

bool MEMORY_EX::is_disk_inserted(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->inserted;
	}
	return false;
}

void MEMORY_EX::is_disk_protected(int drv, bool value)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->write_protected = value;
	}
}

bool MEMORY_EX::is_disk_protected(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->write_protected;
	}
	return false;
}

#endif

#define STATE_VERSION	1

bool MEMORY_EX::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#if defined(FDD_PATCH_SLOT)
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(!disk[i]->process_state(state_fio, loading)) {
			return false;
		}
	}
#endif
	state_fio->StateValue(psl);
	state_fio->StateValue(ssl[0]);
	state_fio->StateValue(ssl[1]);
	state_fio->StateValue(ssl[2]);
	state_fio->StateValue(ssl[3]);
	
	// post process
	if(loading) {
		update_map(psl);
	}
	return true;
}

