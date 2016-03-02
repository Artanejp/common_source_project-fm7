/*
	SEGA GAME GEAR Emulator 'yaGAME GEAR'

	Author : tanam
	Date   : 2013.08.24-

	[ memory ]
*/

#include "memory.h"

void MEMORY::sms_mapper_w(uint32_t addr, uint32_t data)
{
	/* Calculate ROM page index */
	uint8_t page = (data % pages);

	/* Save frame control register data */
	fcr[addr] = data;

	switch(addr)
	{
	case 0:
		if (data & 8) {
			save = 1;
			/* Page in ROM */
			cpu_readmap[4]  = &sram[(data & 4) ? 0x4000 : 0x0000];
			cpu_readmap[5]  = &sram[(data & 4) ? 0x6000 : 0x2000];
			cpu_writemap[4] = &sram[(data & 4) ? 0x4000 : 0x0000];
			cpu_writemap[5] = &sram[(data & 4) ? 0x6000 : 0x2000];
		} else {
			/* Page in RAM */
			cpu_readmap[4]  = &cart[((fcr[3] % pages) << 14) + 0x0000];
			cpu_readmap[5]  = &cart[((fcr[3] % pages) << 14) + 0x2000];
			cpu_writemap[4] = ram + 0x8000;
			cpu_writemap[5] = ram + 0xA000;
		}
		break;
	case 1:
		cpu_readmap[0] = &cart[(page << 14) + 0x0000];
		cpu_readmap[1] = &cart[(page << 14) + 0x2000];
		break;
	case 2:
		cpu_readmap[2] = &cart[(page << 14) + 0x0000];
		cpu_readmap[3] = &cart[(page << 14) + 0x2000];
		break;
	case 3:
		if (!(fcr[0] & 0x08)) {
			cpu_readmap[4] = &cart[(page << 14) + 0x0000];
			cpu_readmap[5] = &cart[(page << 14) + 0x2000];
		}
		break;
	}
}

void MEMORY::initialize()
{
	cart=NULL;
	
	memset(ram, 0, sizeof(ram));
	memset(rdmy, 0xff, sizeof(rdmy));
    memset(sram, 0, sizeof(sram));
	
	// set memory map
	cpu_readmap[0] = ram;
	cpu_readmap[1] = ram + 0x2000;
	cpu_readmap[2] = ram + 0x4000;
	cpu_readmap[3] = ram + 0x6000;
	cpu_readmap[4] = ram + 0x8000;
	cpu_readmap[5] = ram + 0xA000;
	cpu_readmap[6] = ram + 0xC000;
	cpu_readmap[7] = ram + 0xE000;
	
	cpu_writemap[0] = ram;
	cpu_writemap[1] = ram + 0x2000;
	cpu_writemap[2] = ram + 0x4000;
	cpu_writemap[3] = ram + 0x6000;
	cpu_writemap[4] = ram + 0x8000;
	cpu_writemap[5] = ram + 0xA000;
	cpu_writemap[6] = ram + 0xC000;
	cpu_writemap[7] = ram + 0xE000;
	
	fcr[0] = 0x00;
	fcr[1] = 0x00;
	fcr[2] = 0x01;
	fcr[3] = 0x00;
	
	inserted = false;
}

void MEMORY::release()
{
	if (cart) free(cart);
}

void MEMORY::bios()
{
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(create_local_path(_T("COLECO.ROM")), FILEIO_READ_BINARY)) {
		fio->Fseek(0, FILEIO_SEEK_END);
		size=fio->Ftell();
		pages = (size / 0x4000);
		fio->Fseek(0, FILEIO_SEEK_SET);
		if (size == 0x2000 && cpu_readmap[0] != cart) {
			fio->Fread(ram, size, 1);
			/* $0000-$1FFF mapped to internal ROM (8K) */
			cpu_readmap[0]  = ram;
			cpu_writemap[0] = ram;
			/* $2000-$5FFF mapped to expansion */
			cpu_readmap[1]  = rdmy;
			cpu_readmap[2]  = rdmy;
			/* $6000-$7FFF mapped to RAM (1K mirrored) */
			cpu_readmap[3]  = ram + 0x6000;
			/* $8000-$FFFF mapped to Cartridge ROM (max. 32K) */
			if (!cart) {
				cpu_readmap[4] = ram + 0x8000;
				cpu_readmap[5] = ram + 0xA000;
				cpu_readmap[6] = ram + 0xC000;
				cpu_readmap[7] = ram + 0xE000;
			}
			cpu_writemap[0] = ram;
			cpu_writemap[1] = ram + 0x2000;
			cpu_writemap[2] = ram + 0x4000;
			cpu_writemap[3] = ram + 0x6000;
			cpu_writemap[4] = ram + 0x8000;
			cpu_writemap[5] = ram + 0xA000;
			cpu_writemap[6] = ram + 0xC000;
			cpu_writemap[7] = ram + 0xE000;
			fcr[0] = 0x00;
			fcr[1] = 0x00;
			fcr[2] = 0x01;
			fcr[3] = 0x00;
		}
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	cpu_writemap[(addr >> 13)][(addr & 0x1FFF)] = data;
	if (pages < 4) return;
	if (addr >= 0xFFFC) sms_mapper_w(addr & 3, data);
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	return cpu_readmap[(addr >> 13)][(addr & 0x1FFF)];
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	// from PIO-P6
	if(data & mask) {
		cpu_readmap[0] = ram;
		cpu_readmap[1] = ram + 0x2000;
		cpu_readmap[2] = ram + 0x4000;
		cpu_readmap[3] = ram + 0x6000;
		cpu_readmap[4] = ram + 0x8000;
		cpu_writemap[0] = ram;
		cpu_writemap[1] = ram + 0x2000;
		cpu_writemap[2] = ram + 0x4000;
		cpu_writemap[3] = ram + 0x6000;
		cpu_writemap[4] = ram + 0x8000;
	}
	else {
		// ROM
		cpu_readmap[0] = cart;
		cpu_readmap[1] = rdmy;
		cpu_readmap[2] = ram + 0x4000;
		cpu_readmap[3] = ram + 0x6000;
		cpu_readmap[4] = ram + 0x8000;
		cpu_writemap[0] = ram;
		cpu_writemap[1] = ram + 0x2000;
		cpu_writemap[2] = ram + 0x4000;
		cpu_writemap[3] = ram + 0x6000;
		cpu_writemap[4] = ram + 0x8000;
	}
}

void MEMORY::open_cart(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fseek(0, FILEIO_SEEK_END);
		size=fio->Ftell();
		pages = (size / 0x4000);
		fio->Fseek(0, FILEIO_SEEK_SET);
		if (cart) free(cart);
		cart=(uint8_t *)malloc(size);
		fio->Fread(cart, size, 1);
		fio->Fclose();
		delete fio;
		inserted = true;
	} else {
		delete fio;
		return;
	}
	if (check_file_extension(file_path, _T(".col"))) {
		/* $0000-$1FFF mapped to internal ROM (8K) */
		cpu_readmap[0]  = ram;
		cpu_writemap[0] = ram;
		/* $2000-$5FFF mapped to expansion */
		cpu_readmap[1]  = rdmy;
		cpu_readmap[2]  = rdmy;
		/* $6000-$7FFF mapped to RAM (1K mirrored) */
		cpu_readmap[3]  = ram + 0x6000;
		/* $8000-$FFFF mapped to Cartridge ROM (max. 32K) */
		cpu_readmap[4] = cart;
		if (size>0x2000) cpu_readmap[5] = cart + 0x2000;
		else cpu_readmap[5] = rdmy;
		if (size>0x4000) cpu_readmap[6] = cart + 0x4000;
		else cpu_readmap[6] = rdmy;
		if (size>0x6000) cpu_readmap[7] = cart + 0x6000;
		else cpu_readmap[7] = rdmy;
	} else {
		// set memory map
		cpu_readmap[0] = cart;
		if (size>0x2000) cpu_readmap[1] = cart + 0x2000;
		else cpu_readmap[1] = rdmy;
		if (size>0x4000) cpu_readmap[2] = cart + 0x4000;
		else cpu_readmap[2] = rdmy;
		if (size>0x6000) cpu_readmap[3] = cart + 0x6000;
		else cpu_readmap[3] = rdmy;
		if (size>0x8000) cpu_readmap[4] = cart + 0x8000;
		else cpu_readmap[4] = ram + 0x8000;
		if (size>0xA000) cpu_readmap[5] = cart + 0xA000;
		else cpu_readmap[5] = ram + 0xA000;
		cpu_readmap[6] = ram + 0xC000;
		cpu_readmap[7] = ram + 0xE000;
	}
	if (_tcsicmp(file_path, _T("SMS.ROM"))) {
		cart[0x77]=0;
		cart[0x79]=0;
	}
	cpu_writemap[0] = ram;
	cpu_writemap[1] = ram + 0x2000;
	cpu_writemap[2] = ram + 0x4000;
	cpu_writemap[3] = ram + 0x6000;
	cpu_writemap[4] = ram + 0x8000;
	cpu_writemap[5] = ram + 0xA000;
	cpu_writemap[6] = ram + 0xC000;
	cpu_writemap[7] = ram + 0xE000;
	fcr[0] = 0x00;
	fcr[1] = 0x00;
	fcr[2] = 0x01;
	fcr[3] = 0x00;
}

void MEMORY::close_cart()
{
	if (cart) free(cart);
	initialize();
}
