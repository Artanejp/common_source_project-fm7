/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.07.15-

	[ memory ]
*/

#include "./memory.h"
#include "./timer.h"

#define RAM		(MEMORY_BASE + RAM_BASE)
#define BASICROM	(MEMORY_BASE + BASICROM_BASE)
#define EXTROM		(MEMORY_BASE + EXTROM_BASE)
#define CGROM1		(MEMORY_BASE + CGROM1_BASE)
#define EmptyRAM	(MEMORY_BASE + EmptyRAM_BASE)

namespace PC6001 {

void MEMORY::initialize()
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("BASICROM.60")), FILEIO_READ_BINARY)) {
		fio->Fread(BASICROM, 0x4000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("CGROM60.60")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM1, 0x1000, 1);
		fio->Fclose();
	}
	delete fio;
	
	register_vline_event(this);
}

void MEMORY::reset()
{
	int J;
	if (!inserted) {
		EXTROM1 = RAM + 0x4000;
		EXTROM2 = RAM + 0x6000;
		FILEIO* fio = new FILEIO();
		if (fio->Fopen(create_local_path(_T("EXTROM.60")), FILEIO_READ_BINARY)) {
			fio->Fread(EXTROM, 0x4000, 1);
			fio->Fclose();
			EXTROM1 = EXTROM;
			EXTROM2 = EXTROM + 0x2000;
			inserted = true;
		}
		delete fio;
	}
	memset(RAM ,0,0x10000);
	memset(EmptyRAM, 0, 0x2000);
	CGROM = CGROM1;
	CGSW93 = 0;
	VRAM = RAM;
	for(J=0;J<4;J++) {RdMem[J]=BASICROM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	RdMem[2] = EXTROM1; RdMem[3] = EXTROM2;
	for(J=4;J<8;J++) {RdMem[J]=RAM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	EnWrite[0]=0; EnWrite[1]=EnWrite[2]=EnWrite[3]=1;
	CGSW93 = CRTKILL = 0;
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	/* normal memory write */
	if(EnWrite[addr >> 14]) 
		WrMem[addr >> 13][addr & 0x1FFF] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	return(RdMem[addr >> 13][addr & 0x1FFF]);
}

void MEMORY::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	*wait = addr < 0x8000 ? 1 : 0;
	write_data8(addr, data);
}

uint32_t MEMORY::read_data8w(uint32_t addr, int *wait)
{
	*wait = addr < 0x8000 ? 1 : 0;
	return read_data8(addr);
}

uint32_t MEMORY::fetch_op(uint32_t addr, int *wait)
{
	*wait = 1;
	return read_data8(addr);
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	unsigned int VRAMHead[2][4] = {
		{ 0xc000, 0xe000, 0x8000, 0xa000 },
		{ 0x8000, 0xc000, 0x0000, 0x4000 }
	};
	uint16_t port=(addr & 0x00ff);
	uint8_t Value=data;
	switch(port)
	{
	/// 64K RAM ///
	case 0x00:
		if (Value & 1) {
			RdMem[0]=RAM;
			RdMem[1]=RAM+0x2000;
			EnWrite[0]=1;
		} else {
			RdMem[0]=BASICROM;
			RdMem[1]=BASICROM+0x2000;
			EnWrite[0]=0;
		}
		break;
	/// CP/M ///
	case 0xf0:
		if (Value ==0xdd) {
			RdMem[0]=RAM;
			RdMem[1]=RAM+0x2000;
			RdMem[2]=RAM+0x4000;
			RdMem[3]=RAM+0x6000;
			EnWrite[0]=EnWrite[1]=1;
		} else {
			RdMem[0]=BASICROM;
			RdMem[1]=BASICROM+0x2000;
			RdMem[2]=EXTROM1;
			RdMem[3]=EXTROM2;
			EnWrite[0]=EnWrite[1]=0;
		}
		break;
	}
	return;
}

void MEMORY::write_io8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = (addr & 0xf0) == 0xa0 ? 1 : 0;
	write_io8(addr, data);
}

uint32_t MEMORY::read_io8w(uint32_t addr, int* wait)
{
	*wait = (addr & 0xf0) == 0xa0 ? 1 : 0;
	return read_io8(addr);
}

#define EVENT_HBLANK	1

void MEMORY::event_vline(int v, int clock)
{
	{
		if (!CRTKILL) {
			if (v < 192) {
				d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				register_event_by_clock(this, EVENT_HBLANK, (uint64_t)((double)CPU_CLOCKS / FRAMES_PER_SEC / LINES_PER_FRAME * 296 / 455), false, NULL);
			}
		}
	}
}

void MEMORY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_HBLANK) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
	}
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_PIO_PORT_C) {
		if(data & 4) {
			CGSW93=0;RdMem[3]=EXTROM2;
		} else {
			CGSW93=1; RdMem[3]=CGROM1;
		}
		CRTKILL = (data & 2) ? 0 : 1;
		if (CRTKILL) {
			d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
		}
	}
}

void MEMORY::open_cart(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(EXTROM, 0x4000, 1);
		fio->Fclose();
		EXTROM1 = EXTROM;
		EXTROM2 = EXTROM + 0x2000;
		EnWrite[1]=0;
		inserted = true;
	} else {
		EXTROM1 = RAM + 0x4000;
		EXTROM2 = RAM + 0x6000;
		EnWrite[1]=1;
		inserted = false;
	}
	delete fio;
}

void MEMORY::close_cart()
{
	EXTROM1 = RAM + 0x4000;
	EXTROM2 = RAM + 0x6000;
	EnWrite[1]=1;
	inserted = false;
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
	state_fio->StateArray(RAM, RAM_SIZE, 1);
	if(loading) {
		CGROM = MEMORY_BASE + state_fio->FgetInt32_LE();
		EXTROM1 = MEMORY_BASE + state_fio->FgetInt32_LE();
		EXTROM2 = MEMORY_BASE + state_fio->FgetInt32_LE();
		for(int i = 0; i < 8; i++) {
			RdMem[i] = MEMORY_BASE + state_fio->FgetInt32_LE();
			WrMem[i] = MEMORY_BASE + state_fio->FgetInt32_LE();
		}
		VRAM = MEMORY_BASE + state_fio->FgetInt32_LE();
	} else {
		state_fio->FputInt32_LE((int)(CGROM - MEMORY_BASE));
		state_fio->FputInt32_LE((int)(EXTROM1 - MEMORY_BASE));
		state_fio->FputInt32_LE((int)(EXTROM2 - MEMORY_BASE));
		for(int i = 0; i < 8; i++) {
			state_fio->FputInt32_LE((int)(RdMem[i] - MEMORY_BASE));
			state_fio->FputInt32_LE((int)(WrMem[i] - MEMORY_BASE));
		}
		state_fio->FputInt32_LE((int)(VRAM - MEMORY_BASE));
	}
	state_fio->StateArray(EnWrite, sizeof(EnWrite), 1);
	state_fio->StateValue(CGSW93);
	state_fio->StateValue(inserted);
	return true;
}
}
