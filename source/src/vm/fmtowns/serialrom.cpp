/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[serial rom]
*/

#include "../../fileio.h"
#include "./towns_memory.h"
#include "./serialrom.h"
#include "../i386.h"

namespace FMTOWNS {
	
void SERIAL_ROM::initialize()
{
	cs = true;
	clk = false;
	reset_reg = false;
	reset_state = 0;
	rom_addr = 0;
	
	memset(rom, 0xff, sizeof(rom));
	FILEIO *fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("MYTOWNS.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("SERIAL.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	} else {
		// Header
		const _TCHAR *id = _T("FUJITSU");
		int _len = strlen(id);
		if(_len < 0) _len = 0; // Bit 251 - 72
		if(_len >= 22) _len = 21; // Bit 251 - 72
		for(int i = 0; i < (_len + 1); i++) {
			rom[32 - i] = 0x00;
		}
		for(int i = 0; i < _len; i++) {
			uint8_t _c = (uint8_t)id[i];
			uint8_t _revc = 0x00;
			uint8_t val = 0x80;
			for(int j = 0; j < 8; j++) {
				if((_c & 0x01) != 0) _revc = _revc | val;
				val >>= 1;
				_c  >>= 1;
			}
			rom[31 - i] = rom[31 - i] | ((_revc & 0xf0) >> 4); // High
			rom[31 - (i + 1)] = rom[31 - (i + 1)] | ((_revc & 0x0f) << 4); // Low
		}
		rom[31 - _len] = rom[31 - _len] | 0x0f; // Last bit
		// Machine ID (bit 71 - bit 56) must be dummy.

		// Serial (bit 55 - bit20)
		auto serial = static_cast<uint64_t>(0x00000123);
		auto nmask  = static_cast<uint64_t>(0x0f);
		nmask = nmask << 32;
		int nibblepos = 8;
		// Initialize footer and serial ID.
		for(int i = 0; i < 7; i++) {
			rom[7 - i] = 0x00;
		}
		
		for(int i = 0; i < 9; i++) {
			uint64_t nval = (nmask & serial) >> 32;
			uint8_t _c = ((uint8_t)nval) & 0x0f;
			uint8_t _revc = 0x00;
			uint8_t val = 0x08;
			for(int j = 0; j < 4; j++) {
				if((_c & 0x01) != 0) _revc = _revc | val;
				val >>= 1;
				_c  >>= 1;
			}
			serial <<= 4;
			// High
			if((i & 1) == 0) { // Lower
				rom[6 - (i / 2)] = rom[6 - (i / 2)] | (_revc << 4);  
			} else { // Lower
				rom[6 - (i / 2)] = rom[6 - (i / 2)] | _revc;  
			}
		}
	}
}

void SERIAL_ROM::reset()
{
//	cs = true;
//	clk = 0;
//	reset_state = 0;
//	rom_addr = 0;
}

void SERIAL_ROM::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_SERIALROM_CLK:
		{
			bool oldclk = clk;
			bool newclk = clk;
			if(cs) {
				newclk = ((data & mask) != 0);
			}
			if((oldclk != newclk) && !(reset_reg)) {
				clk = newclk;
				if(!(oldclk)) {
					// Rise up
					rom_addr = (rom_addr + 1) & 0xff;
				}
			}
		}
		break;
	case SIG_SERIALROM_CS:
		cs = ((data & mask) == 0);
		break;
	case SIG_SERIALROM_RESET:
		reset_reg = ((data & mask) != 0);
		if((cs) && (clk)) {
			switch(reset_state) {
			case 0:
				if(reset_reg) reset_state++;
				break;
			case 1:
				if(!(reset_reg)) {
					// Do Reset
					rom_addr = 0;
					reset_state = 0;
				}
				break;
			default:
				break;
			}
					
		}
		break;
	}
}
				
uint32_t SERIAL_ROM::read_signal(int ch)
{
	switch(ch) {
	case SIG_SERIALROM_CLK:
		return ((clk) ? 0xffffffff : 0x00000000);
		break;
	case SIG_SERIALROM_CS:
		return 0;
		break;
	case SIG_SERIALROM_RESET:
		return ((reset_reg) ? 0xffffffff : 0x00000000);
		break;
	case SIG_SERIALROM_DATA:
		{
			if((rom_addr >= 56) && (rom_addr < 72)) {
				// Serial id
				uint32_t machine_id = d_mem->read_signal(SIG_FMTOWNS_MACHINE_ID);
				uint32_t bitaddr = 15 - (rom_addr - 56);
				uint32_t bitmask = 0x8000 >> bitaddr;
				return (((bitmask & machine_id) != 0) ? 0xffffffff : 0x00000000);
			} else {
				uint32_t localaddr = (rom_addr & 0xff) >> 3;
				uint32_t localbit  = (rom_addr & 0xff) & 0x07;
				uint8_t _c = rom[localaddr];
				uint8_t _bmask = 0x01 << localbit;
				return (((_c & _bmask) != 0) ? 0xffffffff : 0x00000000);
			}
		}
		break;
	}
	return 0;
}

#define STATE_VERSION	1

bool SERIAL_ROM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(cs);
	state_fio->StateValue(clk);
	state_fio->StateValue(reset_reg);
	state_fio->StateValue(reset_state);
	state_fio->StateValue(rom_addr);
	state_fio->StateArray(rom, sizeof(rom), 1);

	return true;
}

}
