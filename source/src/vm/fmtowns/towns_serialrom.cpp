
#include "../../fileio.h"

#include "./debugger.h"
#include "serialrom.h"
#include "./towns_serialrom.h"

namespace FMTOWNS {

void TOWNS_SERIAL_ROM::initialize()
{
	DEVICE::initialize();
	prev_value = 0x00;
	load_ok = false;
	initialize_rom();
	if((__USE_DEBUGGER) && (d_debugger != NULL)) {
//		d_mem_stored = d_mem;
//		d_debugger->set_context_mem(this);
	}

}

void TOWNS_SERIAL_ROM::initialize_rom()
{
	if((load_ok) || (d_rom == NULL)) {
		return;
	}
	if(!(d_rom->is_initialized(32))) {
		if(d_rom->allocate_memory(32) != 32) {
			return;
		}
	}
	FILEIO *fio = new FILEIO();
	if(fio == NULL) {
		return;
	}
	bool loaded = false;
	// Default values are from Tsugaru, physmem.cpp.
	// -> // Just took from my 2MX.
	const uint8_t defSerialROM[32]=
	{
		0x04,0x65,0x54,0xA4,0x95,0x45,0x35,0x5F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0x0C,0x02,0x00,0x00,0x00,0x15,0xE0,0x00,0x00,
	};
	uint8_t tmprom[32] = { 0xff };
	// Q: Is override machineid? 20200627 K.O
//	tmprom[24] = (machine_id >> 8);
//	tmprom[25] = (machine_id & 0xf8) | (cpu_id & 0x07);
	const _TCHAR *rom_names[] = {
		_T("MYTOWNS.ROM"),
		_T("SERIAL.ROM"),
		NULL
	};
	for(int i = 0; i < (sizeof(rom_names) / sizeof(const _TCHAR *)); i++) {
		if(rom_names[i] == NULL) break;
		if(fio->Fopen(create_local_path(_T("%s"), rom_names[i]), FILEIO_READ_BINARY)) { // FONT
			if(fio->Fread(tmprom, sizeof(tmprom), 1) == 1) {
				loaded = true;
			}
			fio->Fclose();
			break;
		}
	}
	d_rom->load_data((loaded) ? (const uint8_t*)tmprom : defSerialROM, 32);
	load_ok = true;
}

void TOWNS_SERIAL_ROM::reset()
{
	if(!(load_ok)) {
		initialize_rom();
	}
	prev_value = 0x00; // RESET = false, CLK = false, CS = true
	write_to_rom(prev_value);
}

uint32_t TOWNS_SERIAL_ROM::read_io8(uint32_t addr)
{
	__UNLIKELY_IF((addr != 0) || !(load_ok)) {
		return 0xff;
	}
	uint8_t val;
	uint8_t val2;
	val = prev_value & (REG_VALUE_CLK | REG_VALUE_RESET); // RESET and CLK
	__LIKELY_IF(d_rom != NULL) {
		val2 = (d_rom->read_signal(SIG_SERIALROM_DATA) != 0) ? 1 : 0;
	}
	return val | val2;
}

void TOWNS_SERIAL_ROM::write_io8(uint32_t addr, uint32_t data)
{
	__UNLIKELY_IF((addr != 0) || !(load_ok)) {
		return;
	}
	write_to_rom(data);
	prev_value = data;
}

void TOWNS_SERIAL_ROM::write_to_rom(uint8_t data)
{
	__LIKELY_IF(d_rom != NULL) {
		d_rom->write_signal(SIG_SERIALROM_CS, ((data & REG_VALUE_CS) == 0) ? 0xffffffff : 0x00000000, 0xffffffff);
		d_rom->write_signal(SIG_SERIALROM_CLK, ((data & REG_VALUE_CLK) != 0) ? 0xffffffff : 0x00000000, 0xffffffff);
		d_rom->write_signal(SIG_SERIALROM_RESET, ((data & REG_VALUE_RESET) != 0) ? 0xffffffff : 0x00000000, 0xffffffff);
	}
}


bool TOWNS_SERIAL_ROM::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	if((buffer == NULL) || (buffer_len == 0)) {
		return false;
	}
	// Dump raw value
	my_tcscat_s(buffer, buffer_len,
				create_string(_T("\nloading: %s \ndata register: %02x\n"),
							  (load_ok) ? _T("YES") : _T("NO "),
							  prev_value)

		);
	return true;
}

#define STATE_VERSION	1

bool TOWNS_SERIAL_ROM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	//state_fio->StateValue(machine_id);
	//state_fio->StateValue(cpu_id);
	state_fio->StateValue(load_ok);
	state_fio->StateValue(prev_value);

	return true;
}

}
