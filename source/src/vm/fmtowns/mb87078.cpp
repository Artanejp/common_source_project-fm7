
#include "fileio.h"
#include "./mb87078.h"

void MB87078::reset()
{
	com_reg = 0x00;
	for(int ch = 0; ch < 4; ch++) {
		data_reg[ch] = 0x00; // OK?
		write_signals(&(target_volume[ch]), data_reg[ch]);
		write_signals(&(target_mute[ch]), 0xffffffff); // OK?
	}
}

void MB87078::write_io8(uint32_t addr, uint32_t data)
{
	uint8_t ch_reg;
	switch(addr & 1) {
	case 0: // VALUE
		ch_reg = com_reg & 3;
		data_reg[ch_reg] = data & 0x3f;
		if((com_reg & 0x04) == 0) { // EN bit
			return;
		}
		if((com_reg & 0x18) != 0) { // C0 or C32 has SET
			return;
		}
		write_signals(&(target_volume[ch_reg]), 0x3f - data_reg[ch_reg]);
		break;
	case 1: // COM
		com_reg = data;
		ch_reg = com_reg & 3;
		write_signals(&(target_mute[ch_reg]), ((com_reg & 0x04) == 0) ? 0xffffffff : 0x0);
		if((com_reg & 0x10) != 0) { // C32
			write_signals(&(target_volume[ch_reg]), 0x40);
		} else if((com_reg & 0x08) != 0) { // C0
			write_signals(&(target_volume[ch_reg]), 0x00);
		}
		break;
	default:
		break;
	}
}

uint32_t MB87078::read_io8(uint32_t addr)
{
	switch(addr & 1) {
	case 0: // DATA
		return data_reg[com_reg & 3] & 0x3f;
		break;
	case 1: // COM
		return com_reg;
		break;
	default:
		break;
	}
	return 0xff;
}


#define STATE_VERSION	2

bool MB87078::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}

	state_fio->StateValue(com_reg);
	state_fio->StateArray(data_reg, sizeof(data_reg), 1);
	// ToDo: After Loading.
	return true;
}
