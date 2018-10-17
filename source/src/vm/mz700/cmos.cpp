/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2010.09.02 -

	[ cmos memory ]
*/

#include "cmos.h"

#define DATA_SIZE	0x8000
#define ADDR_MASK	(DATA_SIZE - 1)

void CMOS::initialize()
{
	// init memory
	data_buffer = (uint8_t *)malloc(DATA_SIZE);
	memset(data_buffer, 0, DATA_SIZE);
	modified = false;
	
	// load cmos image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("CMOS.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(data_buffer, DATA_SIZE, 1);
		fio->Fclose();
	}
	delete fio;
}

void CMOS::release()
{
	// save cmos image
	if(modified) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("CMOS.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(data_buffer, DATA_SIZE, 1);
			fio->Fclose();
		}
		delete fio;
	}
	
	// release memory
	free(data_buffer);
}

void CMOS::reset()
{
	data_addr = 0;
}

void CMOS::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xf8:
		data_addr = (data_addr & 0x00ff) | (data << 8);
		break;
	case 0xf9:
		data_addr = (data_addr & 0xff00) | data;
		break;
	case 0xfa:
		if(data_buffer[data_addr & ADDR_MASK] != data) {
			data_buffer[data_addr & ADDR_MASK] = data;
			modified = true;
		}
		data_addr++;
		break;
	}
}

uint32_t CMOS::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xf8:
		data_addr = 0;
		return 0xff;
	case 0xf9:
		return data_buffer[(data_addr++) & ADDR_MASK];
	}
	return 0xff;
}

#define STATE_VERSION	1

#include "../../statesub.h"

void CMOS::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_1D_ARRAY(data_buffer, DATA_SIZE);
	DECL_STATE_ENTRY_UINT32(data_addr);
	DECL_STATE_ENTRY_BOOL(modified);
	
	leave_decl_state();
}

void CMOS::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
//	
//	state_fio->Fwrite(data_buffer, DATA_SIZE, 1);
//	state_fio->FputUint32(data_addr);
//	state_fio->FputBool(modified);
}

bool CMOS::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		return false;
	}
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	state_fio->Fread(data_buffer, DATA_SIZE, 1);
//	data_addr = state_fio->FgetUint32();
//	modified = state_fio->FgetBool();
	return true;
}

bool CMOS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateBuffer(data_buffer, DATA_SIZE, 1);
	state_fio->StateUint32(data_addr);
	state_fio->StateBool(modified);
	return true;
}
