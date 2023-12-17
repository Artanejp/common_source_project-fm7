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
	uint8_t addr, bit;
	pos2addr(pos, addr, bit);
	uint8_t val = ((rom[addr] & (1 << (bit & 7))) != 0) ? 0x01 : 0x00;
	return val;
}

void SERIAL_ROM::initialize()
{
	bool loaded = false;
	FILEIO *fio = new FILEIO();

	// Default values are from Tsugaru, physmem.cpp.
	// -> // Just took from my 2MX.
	const unsigned char defSerialROM[32]=
	{
		0x04,0x65,0x54,0xA4,0x95,0x45,0x35,0x5F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0x0C,0x02,0x00,0x00,0x00,0x15,0xE0,0x00,0x00,
	};
	uint8_t tmprom[32] = { 0xff };
	cs = true;
	clk = false;
	reset_reg = false;
	rom_addr = 0;

	//memset(rom, 0xff, sizeof(rom));
	memcpy(rom, defSerialROM, sizeof(rom));

	// Q: Is override machineid? 20200627 K.O
//	tmprom[24] = (machine_id >> 8);
//	tmprom[25] = (machine_id & 0xf8) | (cpu_id & 0x07);

	if(fio->Fopen(create_local_path(_T("MYTOWNS.ROM")), FILEIO_READ_BINARY)) { // FONT
		if(fio->Fread(tmprom, sizeof(tmprom), 1) == 1) {
			loaded = true;
		}
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("SERIAL.ROM")), FILEIO_READ_BINARY)) { // FONT
		if(fio->Fread(tmprom, sizeof(tmprom), 1) == 1) {
			loaded = true;
		}
		fio->Fclose();
	}
	if(loaded) {
		memcpy(rom, tmprom, sizeof(rom));
	}
}

void SERIAL_ROM::reset()
{
	reset_reg = false;
	clk = false;
	cs = true;

	rom_addr = 0;
}

void SERIAL_ROM::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SERIALROM_CLK:
		if(cs) {
			if(!(clk)) {
				if((data & mask) != 0) { // RISE UP
					rom_addr = (rom_addr + 1) & 0xff;
				}
			}
		}
		clk = ((data & mask) != 0) ? true : false;
		break;
	case SIG_SERIALROM_CS:
		cs = ((data & mask) == 0) ? true : false;
		break;
	case SIG_SERIALROM_RESET:
		reset_reg = ((data & mask) != 0);
		if((cs) && (reset_reg)) {
			reset();
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
	case SIG_SERIALROM_DATA:
		return (read_rom_bits(rom_addr) == 0x00) ? 0x00000000 : 0xffffffff;
		break;
	}
	return 0;
}

uint32_t SERIAL_ROM::read_io8(uint32_t addr)
{
	__UNLIKELY_IF(addr != 0) {
		return 0xff;
	}
	uint8_t val;
	uint8_t val2;
	val = ((reset_reg) ? 0x80 : 0x00) | ((clk) ? 0x40 : 0x00);
	val2 = read_rom_bits(rom_addr);
	return val | val2;
}

void SERIAL_ROM::write_io8(uint32_t addr, uint32_t data)
{
	__LIKELY_IF(addr == 0) {
		if((reset_reg) && ((data & 0x80) == 0) && ((data & 0x20) == 0)) {
			// Reset
			rom_addr = 0x00;
		} else if(((data & 0x80) == 0x00) && ((data & 0x20) == 0x00)) {
			// CS DOWN, ADDRESS UP.
			if(!(clk) && ((data & 0x40) != 0)) {
				rom_addr = (rom_addr + 1) & 0xff;
			}
		}

		cs = ((data & 0x20) == 0) ? true : false;
		clk = ((data & 0x40) != 0) ? true : false;
		reset_reg = ((data & 0x80) != 0) ? true : false;
	}
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

	uint8_t nibble, bit;
	pos2addr(rom_addr, nibble, bit);

	my_tcscat_s(buffer, buffer_len, _T("** STATS:\n"));
	my_tcscat_s(buffer, buffer_len,
				create_string(_T("    CS=%s CLK=%d RESET REG=%d ROM ADDR=%d\n   ROM BIT POSITION=%02d word + %01d bit\n\n"),
							  (cs) ? _T("ON ") : _T("OFF"),
							  (clk) ? 1 : 0,
							  (reset_reg) ? 1 : 0,
							  rom_addr,
							  nibble, bit)
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

#define STATE_VERSION	2

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
	state_fio->StateValue(rom_addr);
	state_fio->StateArray(rom, sizeof(rom), 1);

	return true;
}

}
