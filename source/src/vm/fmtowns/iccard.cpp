/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.08.10-

	[FM-Towns IC CARD]

		I/O:	
			048Ah: R: IC CARD STATUS.
*/

#include "./iccard.h"
#include "fileio.h"

namespace FMTOWNS {

void TOWNS_ICCARD::initialize()
{
	DEVICE::initialize();
	memset(filename, 0x00, sizeof(filename) / sizeof(_TCHAR));

	card_changed = false;	// 048Ah: bit7 (R)
	batt_level = 0x00;		// 048Ah: bit 5-4 (R)
	eeprom_ready = false;	// 048Ah: bit 3 (R)
	card_state = 0x03;		// 048Ah: bit2-1 (R) : 00=exists, 01,02=imcomplete, 03=none.
	write_protect = true;	// 048Ah: bit0 (R)
	is_rom = true; // ToDo: Will implement RAM.

	is_dirty = false;
	if(limit_size == 0) limit_size = 0x01000000; // Default limit is 16MB
}

void TOWNS_ICCARD::release()
{
	if(!(is_rom) && (is_dirty) && (card_state == 0x00)) {
		close_cart();
	}
	if(mem != NULL) {
		free(mem);
		mem = NULL;
	}
}
	
bool TOWNS_ICCARD::open_cart(const _TCHAR *file_path)
{
	if(file_path == NULL) return false;
	if(strlen(file_path) <= 0) return false;
   	FILEIO* fio = new FILEIO();
	uint32_t file_size;
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fseek(0, FILEIO_SEEK_SET);
		file_size = fio->Ftell();
		if(new_alloc(file_size)) {
			if(fio->Fread(mem, mem_size, 1) == mem_size) {
				fio->Fclose();
				delete fio;
				// Update filename
				strncpy(filename, file_path, sizeof(filename) / sizeof(_TCHAR));
				// ToDo: Implement RAM
				write_protect = true;
				is_rom = true;
				is_dirty = false;
				card_state = 0x00;
				card_changed = true;
				return true;
			} else {
				if(mem != NULL) free(mem);
				mem = NULL;
			}
		}
		is_dirty = false;
		mem_size = 0;
	}
	fio->Fclose();
	delete fio;
	return false; // file not exists, keep data.
}

bool TOWNS_ICCARD::close_cart()
{
	if((mem == NULL) || (mem_size == 0)) return false; // Nothing to save.
	if(card_state != 0x00) return false; // Not using card.
	
	FILEIO* fio = new FILEIO();

	card_changed = true;
	if((is_dirty) && !(is_rom)) {
		if(fio->Fopen(filename, FILEIO_WRITE_BINARY)) {
			fio->Fwrite(mem, mem_size, 1);
			is_dirty = false;
			free(mem);
			mem = NULL;
			mem_size = 0;
			memset(filename, 0x00, sizeof(filename));
			fio->Fclose();
			delete fio;
			return true;
		}
	} else {
		// Discard only
		is_dirty = false;
		free(mem);
		mem = NULL;
		mem_size = 0;
		memset(filename, 0x00, sizeof(filename));
		return true;
	}
	delete fio;
	return false;
}
	
bool TOWNS_ICCARD::new_alloc(uint32_t new_size)
{
	if(((mem_size != new_size) || (mem == NULL)) && (new_size > 0)) {
		if(mem != NULL) free(mem);
		mem_size = 0;
		if(new_size > limit_size) new_size = limit_size;
		mem = (uint8_t*)malloc(new_size);
		if(mem != NULL) { // Maybe Alloc
			mem_size = new_size;
			memset(mem, 0x00, mem_size); // Re-Initialize.
			return true; // Success
		}
	}
	return false;
}

uint32_t TOWNS_ICCARD::read_memory_mapped_io8(uint32_t addr)
{
	if(addr <  0xc0000000) return 0xff;
	if(addr >= 0xc2000000) return 0xff;
	addr = addr & 0x00ffffff; // Map 16M
	if((addr >= mem_size) || (addr >= 0x01000000)) return 0xff;
	if(mem == NULL) return 0xff;
	
	// ToDo: Bank register.
	return mem[addr];
}

void TOWNS_ICCARD::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	if(addr <  0xc0000000) return;
	if(addr >= 0xc2000000) return;
	addr = addr & 0x00ffffff; // Map 16M
	if((addr >= mem_size) || (addr >= 0x01000000)) return;
	if(mem == NULL) return;
	if(!(is_rom) && !(write_protect) && (card_state == 0x00)) {
		// ToDo: Bank register.
		uint8_t bak = mem[addr];
		mem[addr] = data;
		is_dirty |= ((bak != (uint8_t)data) ? true : false);
	}
}

uint32_t TOWNS_ICCARD::read_io8(uint32_t addr)
{
	// ToDo: Bank register.
	uint8_t val = 0xff;
	switch(addr) {
	case 0:
		val = 0x00;
		val |= ((card_changed) ? 0x80 : 0x00);
		val |= (((batt_level) & 0x03) << 4);
		val |= ((eeprom_ready) ? 0x08 : 0x00);
		val |= ((card_state & 0x03) << 1);
		val |= ((write_protect) ? 0x01 : 0x00);
		card_changed = false;
		break;
	}
	return val;
}

void TOWNS_ICCARD::write_io8(uint32_t addr, uint32_t data)
{
		// ToDo: Bank register.
}

#define STATE_VERSION	1

bool TOWNS_ICCARD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	
	state_fio->StateValue(limit_size);
	if(loading) {
		uint32_t tmplen = state_fio->FgetUint32_LE();
		if(tmplen == 0) {
			if(mem != NULL) free(mem);
			mem = NULL;
			mem_size = 0;
		} else {
			if(!(new_alloc(tmplen))) {
			// Alloc failed
				return false;
			} else {
				mem_size = tmplen;
				state_fio->Fread(mem, (size_t)mem_size, 1);
			}
		}
	} else {
		// saving
		if(mem == NULL) {
			state_fio->FputUint32_LE(0);
		} else {
			state_fio->FputUint32_LE(mem_size);
			state_fio->Fwrite(mem, (size_t)mem_size, 1);
		}
	}
	state_fio->StateBuffer(filename, sizeof(filename), 1);

	state_fio->StateValue(is_rom);
	state_fio->StateValue(is_dirty);

	// 048Ah
	state_fio->StateValue(card_changed);
	state_fio->StateValue(batt_level);
	state_fio->StateValue(eeprom_ready);
	state_fio->StateValue(card_state);
	state_fio->StateValue(write_protect);
	
	return true;
}
	
}

