/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[serial rom]
*/

#include "../../fileio.h"
#include "./serialrom.h"

namespace FMTOWNS {


uint8_t SERIAL_ROM::read_rom_bits(uint8_t pos)
{
	uint8_t bytepos = 31 - (pos >> 3);
	uint8_t bit = 1 << (pos & 7);
	uint8_t val = ((rom[bytepos] & bit) != 0) ? 1 : 0;
	return val;
}

void SERIAL_ROM::initialize()
{
	cs = true;
	clk = false;
	reset_reg = false;
	reset_state = 0;
	rom_addr = 0;
	
	memset(rom, 0xff, sizeof(rom));
	uint8_t tmprom[256];
	memset(tmprom, 0xff, sizeof(tmprom));
	bool loaded = false;
	FILEIO *fio = new FILEIO();
	
	// Default values are from Tsugaru, physmem.cpp. 
	unsigned char defSerialROM[32]=
	{
		0x04,0x65,0x54,0xA4,0x95,0x45,0x35,0x5F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0x0C,0x02,0x00,0x00,0x00,0x15,0xE0,0x00,0x00,
	};
	memcpy(tmprom, defSerialROM,
		   (sizeof(tmprom) > sizeof(defSerialROM)) ? sizeof(defSerialROM) : sizeof(tmprom));

	// Q: Is override machineid? 20200627 K.O
	tmprom[24] = (machine_id >> 8);
	tmprom[25] = (machine_id & 0xf8) | (cpu_id & 0x07);
	
	if(fio->Fopen(create_local_path(_T("MYTOWNS.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(tmprom, sizeof(tmprom), 1);
		fio->Fclose();
		loaded = true;
	} else if(fio->Fopen(create_local_path(_T("SERIAL.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(tmprom, sizeof(tmprom), 1);
		fio->Fclose();
		loaded = true;
	}
}
	
void SERIAL_ROM::reset()
{
//	cs = true;
//	clk = 0;
//	reset_state = 0;
//	rom_addr = 0;
}

void SERIAL_ROM::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SERIALROM_CLK:
		{
			bool oldclk = clk;
			if(cs) {
				clk = ((data & mask) != 0);
			} else {
				return;
			}
			if((oldclk != clk) && !(reset_reg)) {
				if(clk) {
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
				if(!(reset_reg)) reset_state++;
				break;
			case 1:
				if(reset_reg) reset_state++;
				break;
			case 2:
				if(!(reset_reg)) {
					// Do Reset
					rom_addr = 0;
					reset_state = 0;
					reset_reg = false;
				}
				break;
			default:
				reset_state = 0; // ToDo
				break;
			}
		}  else {
			// Reset reset state?
			reset_state = 0;
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
		return ((cs) ? 0xffffffff : 0x00000000);;
		break;
	case SIG_SERIALROM_RESET:
		return ((reset_reg) ? 0xffffffff : 0x00000000);
		break;
	case SIG_SERIALROM_RESET_STATE:
		return reset_state;
		break;
	case SIG_SERIALROM_DATA:
		if(cs) {
			return (read_rom_bits(rom_addr) == 0x00) ? 0x00000000 : 0xffffffff;
		} else {
			return 0x00000000;
		}
		break;
	}
	return 0;
}

uint32_t SERIAL_ROM::read_io8(uint32_t addr)
{
	uint8_t val = 0x00;
	if(cs) {
		val = (rom[rom_addr >> 3] >> (rom_addr & 7)) & 0x01;
	} else {
		val = 0x01;
	}
	val = val | 0x3e;
	if(clk) val = val | 0x40;
	if(reset_reg) val = val | 0x80;
	return val;
}

void SERIAL_ROM::write_io8(uint32_t addr, uint32_t data)
{
	this->write_signal(SIG_SERIALROM_CS, data, 0x20);
	
	reset_reg = ((data & 0x80) != 0);
	this->write_signal(SIG_SERIALROM_CLK, data, 0x40);
	this->write_signal(SIG_SERIALROM_RESET, data, 0x80);
}

bool SERIAL_ROM::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	_TCHAR numseg[8] = {'\0'};
	int noff = 0;
	// ToDo: Implement
	return false;
}

bool SERIAL_ROM::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{

	// Dump raw value
	my_tcscat_s(buffer, buffer_len, _T("** INFO:\n"));
	my_tcscat_s(buffer, buffer_len, _T("ROM value is enable to modify, \n"));
	my_tcscat_s(buffer, buffer_len, _T("  R00-R32 : Overwrite rom raw value by byte\n"));
	my_tcscat_s(buffer, buffer_len, _T("  B000-B256 : Overwrite bit foo to (value != 0) ? 1 : 0\n"));
	
	my_tcscat_s(buffer, buffer_len, _T("** STATS:\n"));
	my_tcscat_s(buffer, buffer_len,
				create_string(_T("    CS=%s CLK=%d RESET REG=%d RESET STATE=%d\n   ROM BIT POSITION=%03d(0x%02X)\n\n"),
							  (cs) ? _T("ON ") : _T("OFF"),
							  (clk) ? 1 : 0,
							  (reset_reg) ? 1 : 0,
							  reset_state,
							  rom_addr, rom_addr)
		);
	
	my_tcscat_s(buffer, buffer_len, _T("** RAW MEMORY VALUE:\n"));
	my_tcscat_s(buffer, buffer_len, _T("    +0  +1  +2  +3  +4  +5  +6  +7\n"));
	my_tcscat_s(buffer, buffer_len, _T("    ------------------------------\n"));
	for(int n = 0; n < 4; n++) {
		my_tcscat_s(buffer, buffer_len,
					create_string(_T("+%02X %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\n"),
								  n * 4,
								  rom[n * 4 + 0], rom[n * 4 + 1], rom[n * 4 + 2], rom[n * 4 + 3],
								  rom[n * 4 + 4], rom[n * 4 + 5], rom[n * 4 + 6], rom[n * 4 + 7])
			);
	}
	return true;
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
	state_fio->StateValue(machine_id);
	state_fio->StateValue(cpu_id);
	state_fio->StateValue(cs);
	state_fio->StateValue(clk);
	state_fio->StateValue(reset_reg);
	state_fio->StateValue(reset_state);
	state_fio->StateValue(rom_addr);
	state_fio->StateArray(rom, sizeof(rom), 1);

	return true;
}

}
