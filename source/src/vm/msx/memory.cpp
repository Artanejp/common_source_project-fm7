/*
	ASCII MSX1 Emulator 'yaMSX1'
	ASCII MSX2 Emulator 'yaMSX2'
	Pioneer PX-7 Emulator 'ePX-7'

	Author : Takeda.Toshiya
	Date   : 2014.01.09-

	modified by tanam
	modified by umaiboux

	[ memory ]
*/

#include "memory.h"
#if defined(_PX7)
#include "../ld700.h"
#include "../tms9918a.h"
#else
#include "../disk.h"
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

#if !defined(_PX7)
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

static bool load_cart(const _TCHAR *file_path, uint8_t *rom)
{
	bool result = false;
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(rom, 0xff, 0x10000);
		
		fio->Fseek(0, FILEIO_SEEK_END);
		int file_size = fio->Ftell();
		fio->Fseek(0, FILEIO_SEEK_SET);
		
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
			// 64KB: 01234567
			fio->Fread(rom, 0x10000, 1);
		}
		fio->Fclose();
		result = true;
	}
	delete fio;
	return result;
}

// slot #0

void SLOT0::initialize()
{
	memset(rom, 0xff, sizeof(rom));
#if defined(_PX7)
	memset(ram, 0, sizeof(ram));
#endif
	FILEIO* fio = new FILEIO();
#if defined(_MSX2)
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
#if defined(_PX7)
	SET_BANK(0x8000, 0xffff, ram, ram);
#else
	SET_BANK(0x8000, 0xffff, wdmy, rdmy);
#endif
}

void SLOT0::write_data8(uint32_t addr, uint32_t data)
{
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t SLOT0::read_data8(uint32_t addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}

#define SLOT0_STATE_VERSION	1

bool SLOT0::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(SLOT0_STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#if defined(_PX7)
	state_fio->StateArray(ram, sizeof(ram), 1);
#endif
	return true;
}

// slot #1

void SLOT1::initialize()
{
	memset(rdmy, 0xff, sizeof(rdmy));
	close_cart();
}

void SLOT1::write_data8(uint32_t addr, uint32_t data)
{
#if defined(_MSX2)
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
#endif
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t SLOT1::read_data8(uint32_t addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}

void SLOT1::open_cart(const _TCHAR *file_path)
{
	if(load_cart(file_path, rom)) {
		SET_BANK(0x0000, 0xffff, wdmy, rom);
		inserted = true;
#if defined(_MSX2)
		mapper[0] = mapper[1] = 4;
#endif
	}
}

void SLOT1::close_cart()
{
	SET_BANK(0x0000, 0xffff, wdmy, rdmy);
	inserted = false;
#if defined(_MSX2)
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
#endif
}

#define SLOT1_STATE_VERSION	1

bool SLOT1::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(SLOT1_STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(inserted);
#if defined(_MSX2)
	state_fio->StateArray(mapper, sizeof(mapper), 1);
#endif
	
	// post process
	if(loading) {
		if(inserted) {
			SET_BANK(0x0000, 0xffff, wdmy, rom);
#if defined(_MSX2)
		} else if(mapper[0] < 4) {
				SET_BANK(0x0000, 0x3fff, wdmy, rdmy);
				SET_BANK(0x4000, 0x7fff, wdmy, rom + mapper[0] * 0x4000);
				SET_BANK(0x8000, 0xbfff, wdmy, rom + mapper[1] * 0x4000);
				SET_BANK(0xc000, 0xffff, wdmy, rdmy);
#endif
		} else {
			SET_BANK(0x0000, 0xffff, wdmy, rdmy);
		}
	}
	return true;
}

// slot #2

#if defined(_PX7)
void SLOT2::initialize()
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

void SLOT2::reset()
{
	super_impose = false;
	req_intr = false;
	pc4 = false;
	mute_l = mute_r = true;
	
	d_ldp->write_signal(SIG_LD700_MUTE_L, 1, 1);
	d_ldp->write_signal(SIG_LD700_MUTE_R, 1, 1);
	d_vdp->write_signal(SIG_TMS9918A_SUPER_IMPOSE, 0, 0);
}

void SLOT2::write_data8(uint32_t addr, uint32_t data)
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

uint32_t SLOT2::read_data8(uint32_t addr)
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

void SLOT2::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_SLOT2_EXV) {
		bool prev = exv;
		exv = ((data & mask) != 0);
		if(prev && !exv) {
			req_intr = true;
			if(super_impose) {
				d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
			}
		}
	} else if(id == SIG_SLOT2_ACK) {
		ack = ((data & mask) != 0);
	} else if(id == SIG_SLOT2_MUTE) {
		pc4 = ((data & mask) != 0);
	}
}

void SLOT2::event_callback(int event_id, int err)
{
	if(event_id == EVENT_CLOCK) {
		clock = !clock;
	}
}

#define SLOT2_STATE_VERSION	1

bool SLOT2::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(SLOT2_STATE_VERSION)) {
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
#else
void SLOT2::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	FILEIO* fio = new FILEIO();
#if defined(_MSX2)
	if(fio->Fopen(create_local_path(_T("MSX2JEXT.ROM")), FILEIO_READ_BINARY) ||
	   fio->Fopen(create_local_path(_T("MSX2EXT.ROM" )), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
#endif
	if(fio->Fopen(create_local_path(_T("DISK.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom + 0x4000, sizeof(rom) - 0x4000, 1);
		fio->Fclose();
	}
	delete fio;
	
	// patch for pseudo disk bios
	if(rom[0x4010] == 0xc3) {
		rom[DSKIO  = rom[0x4011] + (int)rom[0x4012] * 256] = 0xc9;
	}
	if(rom[0x4013] == 0xc3) {
		rom[DSKCHG = rom[0x4014] + (int)rom[0x4015] * 256] = 0xc9;
	}
	if(rom[0x4016] == 0xc3) {
		rom[GETDPB = rom[0x4017] + (int)rom[0x4018] * 256] = 0xc9;
	}
	if(rom[0x401c] == 0xc3) {
		rom[DSKFMT = rom[0x401d] + (int)rom[0x401e] * 256] = 0xc9;
	}
	if(rom[0x401f] == 0xc3) {
		rom[            rom[0x4020] + (int)rom[0x4021] * 256] = 0xc9;
	}
	SET_BANK(0x0000, 0x7fff, wdmy, rom);
	SET_BANK(0x8000, 0xffff, wdmy, rdmy);
}

void SLOT2::write_data8(uint32_t addr, uint32_t data)
{
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t SLOT2::read_data8(uint32_t addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}
#endif

// slot #3

void SLOT3::initialize()
{
	memset(ram, 0xff, sizeof(ram));
	close_cart();
}

void SLOT3::write_data8(uint32_t addr, uint32_t data)
{
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t SLOT3::read_data8(uint32_t addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}

void SLOT3::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xfc:
	case 0xfd:
	case 0xfe:
	case 0xff:
		if(!inserted) {
			addr &= 3;
			data &= 7;
			if(mapper[addr] != data) {
				mapper[addr] = data;
				SET_BANK(addr * 0x4000, addr * 0x4000 + 0x3fff, ram + data * 0x4000, ram + data * 0x4000);
			}
		}
		break;
	}
}

void SLOT3::open_cart(const _TCHAR *file_path)
{
	if(load_cart(file_path, rom)) {
		SET_BANK(0x0000, 0xffff, wdmy, rom);
		inserted = true;
	}
}

void SLOT3::close_cart()
{
	for(int i = 0; i < 4; i++) {
		mapper[i] = i;
	}
	SET_BANK(0x0000, 0xffff, ram, ram);
	inserted = false;
}


#define SLOT3_STATE_VERSION	1

bool SLOT3::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(SLOT3_STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(inserted);
	state_fio->StateArray(mapper, sizeof(mapper), 1);
	
	// post process
	if(loading) {
		if(inserted) {
			SET_BANK(0x0000, 0xffff, wdmy, rom);
		} else {
			SET_BANK(0x0000, 0x3fff, ram + mapper[0] * 0x4000, ram + mapper[0] * 0x4000);
			SET_BANK(0x4000, 0x7fff, ram + mapper[1] * 0x4000, ram + mapper[1] * 0x4000);
			SET_BANK(0x8000, 0xbfff, ram + mapper[2] * 0x4000, ram + mapper[2] * 0x4000);
			SET_BANK(0xc000, 0xffff, ram + mapper[3] * 0x4000, ram + mapper[3] * 0x4000);
		}
	}
	return true;
}

// memory bus

#if !defined(_PX7)
void MEMORY::initialize()
{
	for(int i = 0; i < MAX_DRIVE; i++) {
		disk[i] = new DISK(emu);
		disk[i]->set_device_name(_T("%s/Disk #%d"), this_device_name, i + 1);
		disk[i]->drive_type = DRIVE_TYPE_2DD;
	}
}

void MEMORY::release()
{
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(disk[i]) {
			disk[i]->close();
			delete disk[i];
		}
	}
}
#endif

void MEMORY::reset()
{
#if !defined(_PX7)
	for(int i = 0; i < MAX_DRIVE; i++) {
		access[i] = false;
	}
#endif
	update_map((0 << 0) | (1 << 2) | (2 << 4) | (3 << 6));
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	d_map[addr >> 14]->write_data8(addr, data);
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return d_map[addr >> 14]->read_data8(addr);
}

uint32_t MEMORY::fetch_op(uint32_t addr, int* wait)
{
	*wait = 1;
	return read_data8(addr);
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
#if !defined(_PX7)
//	if(d_map[3] == d_slot[3]) {
		d_slot[3]->write_io8(addr, data);
//	}
#endif
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_SEL) {
		if(slot_select != (data & mask)) {
			update_map(data & mask);
		}
	}
}

void MEMORY::update_map(uint32_t val)
{
	d_map[0] = d_slot[(val >> 0) & 3];
	d_map[1] = d_slot[(val >> 2) & 3];
	d_map[2] = d_slot[(val >> 4) & 3];
	d_map[3] = d_slot[(val >> 6) & 3];
	slot_select = val;
}

#if !defined(_PX7)
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
	if(get_track(disk, desc, sector)) {
		for(int i = 0; i < disk->sector_num.sd; i++) {
			disk->get_sector(-1, -1, i);
			if(disk->id[2] == (sector % info[desc].per_track) + 1) {
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

uint32_t MEMORY::read_signal(int id)
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

bool MEMORY::bios_ret_z80(uint16_t PC, pair32_t* af, pair32_t* bc, pair32_t* de, pair32_t* hl, pair32_t* ix, pair32_t* iy, uint8_t* iff1)
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
	
	if(d_map[1] == d_slot[2]) {
		// pseudo disk bios from fMSX
		if(PC == DSKIO) {
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
					if(!get_sector(disk[drv], desc, sector)) {
						AF = 0x0801; // record not found
						return true;
					}
					access[drv] = true;
					
					if(disk[drv]->addr_crc_error && !disk[drv]->ignore_crc()) {
						AF = 0x0801; // record not found
						return true;
					}
					if(addr + 512/*disk[drv]->sector_size.sd*/ > 0xffff) {
						F &= ~CF;
						return true;
					}
					d_map[1] = d_slot[read_data8(0xf342) & 3];
					for(int i = 0; i < 512/*disk[drv]->sector_size.sd*/; i++) {
						disk[drv]->sector[i] = read_data8(addr++);
					}
					d_map[1] = d_slot[2];
					B--;
				}
			} else {
				for(int sector = DE; B != 0; sector++) {
					if(!get_sector(disk[drv], desc, sector)) {
						AF = 0x0801; // record not found
						return true;
					}
					access[drv] = true;
					
					if(disk[drv]->addr_crc_error && !disk[drv]->ignore_crc()) {
						AF = 0x0801; // record not found
						return true;
					}
					if(addr + 512/*disk[drv]->sector_size.sd*/ > 0xffff) {
						F &= ~CF;
						return true;
					}
					d_map[1] = d_slot[read_data8(0xf342) & 3];
					for(int i = 0; i < 512/*disk[drv]->sector_size.sd*/; i++) {
						write_data8(addr++, disk[drv]->sector[i]);
					}
					d_map[1] = d_slot[2];
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
			d_map[1] = d_slot[read_data8(0xf342) & 3];
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
			d_map[1] = d_slot[2];
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

void MEMORY::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->open(file_path, bank);
	}
}

void MEMORY::close_disk(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]->inserted) {
		disk[drv]->close();
	}
}

bool MEMORY::is_disk_inserted(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->inserted;
	}
	return false;
}

void MEMORY::is_disk_protected(int drv, bool value)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->write_protected = value;
	}
}

bool MEMORY::is_disk_protected(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->write_protected;
	}
	return false;
}

#endif

#define STATE_VERSION	1

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#if !defined(_PX7)
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(!disk[i]->process_state(state_fio, loading)) {
			return false;
		}
	}
#endif
	state_fio->StateValue(slot_select);
	
	// post process
	if(loading) {
		update_map(slot_select);
	}
	return true;
}

