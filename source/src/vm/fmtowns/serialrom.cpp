/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[serial rom]
*/

#include "../../fileio.h"
#include "./serialrom.h"

namespace FMTOWNS {

// POS=rom bit position, c=data
void SERIAL_ROM::store_reversed_byte(uint8_t pos, uint8_t c)
{
	int npos = (256 - (uint16_t)pos) >> 3;
	int bitpos = (256 - (uint16_t)pos) & 7;
	uint8_t dst = 0x00;
	for(int i = 0; i < 8; i++) {
		dst <<= 1;
		dst = dst | ((c >> i)& 0x01);
	}
	if(bitpos == 0) {
		rom[npos] = dst;
	} else {
		pair16_t tmp;
		pair16_t mask;
		tmp.b.l = rom[npos];
		if(npos < 31) {
			tmp.b.h = rom[npos + 1];
		} else {
			tmp.b.h = 0x00;
		}
		mask.b.h = 0x00;
		mask.b.l = 0xff;
		mask.w <<= bitpos;
		tmp.w = tmp.w & (~mask.w);
		uint16_t data;
		data = dst;
		data <<= bitpos;
		tmp.w = tmp.w | data;
		if(npos < 31) {
			rom[npos + 1] = tmp.b.h;
		}
		rom[npos] = tmp.b.l;
	}
}

uint8_t SERIAL_ROM::read_rom_bits(uint8_t pos)
{
	return ((rom[pos >> 3] >> (pos & 7)) & 0x01);
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
	if(fio->Fopen(create_local_path(_T("MYTOWNS.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(tmprom, sizeof(tmprom), 1);
		fio->Fclose();
		loaded = true;
	} else if(fio->Fopen(create_local_path(_T("SERIAL.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(tmprom, sizeof(tmprom), 1);
		fio->Fclose();
		loaded = true;
	}
	if(loaded) {
		// Q: IS MYTOWNS.ROM reverse bit-order? 20191117 K.O
		//memcpy(rom, tmprom, sizeof(rom));
		for(uint8_t i = 0; i < 32; i++) {
			store_reversed_byte(i << 3, tmprom[i]);
		}
	} else {
		rom[255 >> 3] = rom[255 >> 3] & 0x0f; // Clear head of 4 bits.
		static const _TCHAR signaure[] = _T("FUJITSU");
		for(int i = 0; i < strlen(signature); i++) {
			store_reversed_byte((uint8_t)(244 - (i * 8)), signature[i]);
		}
		// ToDo: Reserved BITs (bit 195 - bit 72)
		// Machine ID
		store_reversed_byte(64, 0x01);
		store_reversed_byte(56, 0x01);
		// SERIAL (DUMMY)
		static const uint8_t serial_num[5] = {0xbc, 0xde, 0xf0, 0x12, 0x34};
		uint8_t tmp1 = rom[48 >> 3];
		uint8_t tmp2 = serial_num[0] & 0x0f;
		uint8_t dst = 0;
		for(int i = 0; i < 4; i++) {
			dst <<= 1;
			dst = dst | (tmp2 & 0x01);
			tmps >>= 1;
		}
		tmp1 = tmp1 & 0xf0;
		rom[48 >> 3] = tmp1 | (dst & 0x0f);
		for(int i = 1; i < 5; i++) {
			store_reversed_byte(20 + ((4 - i) << 3), serial_num[i]);
		}
		// Clear bit 19 - bit0
		rom[16 >> 3] = rom[16 >> 3] & 0xf0;
		rom[8 >> 3] = 0x00;
		rom[0 >> 3] = 0x00;
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
	if((reg[0] == 'R') || (reg[0] == 'r')){
		if(strlen(reg) < 2) return false;
		if((reg[1] == 'R') || (reg[1] == 'r')) { // Reversed bit
			noff = 1;
		}
		for(int i = 0; i < 2; i++) {
			if(reg[i + noff + 1] == '\0') break;
			if((reg[i + noff + 1] < '0') || (reg[i + noff + 1] >'9')) break;
			numseg[i] = reg[i + noff + 1];
		}
		if(strlen(numseg) < 1) return false;
		int pos = atoi(numseg);
		if((pos < 0) || (pos > 31)) return false;
		if((reg[1] == 'R') || (reg[1] == 'r')) { // Reversed bit
			uint8_t dst = 0;
			for(int i = 0; i <8; i++) {
				dst <<= 1;
				dst = dst | (data & 0x01);
				data >>= 1;
			}
			rom[pos] = dst;
		} else {
			rom[pos] = data;
		}
		return true;
	} else if((reg[0] == 'B') || (reg[0] == 'b')){
		if(strlen(reg) < 2) return false;
		if((reg[1] == 'R') || (reg[i] == 'r')) { // Reversed bit
			noff = 1;
		}		
		for(int i = 0; i < 3; i++) {
			if(reg[i + noff + 1] == '\0') break;
			if((reg[i + noff + 1] < '0') || (reg[i + noff + 1] >'9')) break;
			numseg[i] = reg[i + noff + 1];
		}
		if(strlen(numseg) < 1) return false;
		int bitpos = atoi(numseg);
		if((pos < 0) || (pos > 255)) return false;
		int bytepos = bitpos >> 3;
		int offs = bitpos & 7;
		uint8_t dst = rom[bytepos];
		if((reg[1] == 'R') || (reg[i] == 'r')) { // Reversed bit
			offs = 7 - offs;
		}
		dst = dst & (~(0x01 << offs));
		dst = dst | ((data & 0x01) << offs);
		rom[bytepos] = dst;
		return true;
	}
	return false;
}

bool SERIAL_ROM::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	uint8_t reverse_mem[32];

	memset(reverse_mem, 0x00, sizeof(reverse_mem));
	for(int i = 0; i < 32; i++) {
		uint8_t dst = 0x00;
		uint8_t src = rom[i];
		for(int j = 0; j < 8; j++) {
			dst <<= 1;
			dst |= (src & 0x01);
			src >>= 1;
		}
		reverse_mem[i] = dst;
	}
	// Dump raw value
	my_tcscat_s(buffer, buffer_len, _T("** INFO:\n"));
	my_tcscat_s(buffer, buffer_len, _T("ROM value is enable to modify, \n"));
	my_tcscat_s(buffer, buffer_len, _T("  R00-R32 : Overwrite rom raw value by byte\n"));
	my_tcscat_s(buffer, buffer_len, _T("  RR00-RR32 : Overwrite rom reversed value by byte\n"));
	my_tcscat_s(buffer, buffer_len, _T("  B000-B256 : Overwrite bit foo to (value != 0) ? 1 : 0\n"));
	my_tcscat_s(buffer, buffer_len, _T("  BR000-BR256 : Overwrite bit foo to (value != 0) ? 1 : 0 by reversed order.\n\n"));
	
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
	my_tcscat_s(buffer, buffer_len, _T("\n"));
	my_tcscat_s(buffer, buffer_len, _T("** BIT REVERSED VALUE:\n"));
	my_tcscat_s(buffer, buffer_len, _T("    +0  +1  +2  +3  +4  +5  +6  +7\n"));
	my_tcscat_s(buffer, buffer_len, _T("    ------------------------------\n"));
	for(int n = 0; n < 4; n++) {
		my_tcscat_s(buffer, buffer_len,
					create_string(_T("+%02X %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\n"),
								  n * 4,
								  reverse_mem[n * 4 + 0], reverse_mem[n * 4 + 1], reverse_mem[n * 4 + 2], reverse_mem[n * 4 + 3],
								  reverse_mem[n * 4 + 4], reverse_mem[n * 4 + 5], reverse_mem[n * 4 + 6], reverse_mem[n * 4 + 7])
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
	state_fio->StateValue(cs);
	state_fio->StateValue(clk);
	state_fio->StateValue(reset_reg);
	state_fio->StateValue(reset_state);
	state_fio->StateValue(rom_addr);
	state_fio->StateArray(rom, sizeof(rom), 1);

	return true;
}

}
