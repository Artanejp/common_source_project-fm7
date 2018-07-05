/*
	TOSHIBA EX-80 Emulator 'eEX-80'

	Author : Takeda.Toshiya
	Date   : 2015.12.10-

	[ cmt ]
*/

#include "cmt.h"
#include "../i8251.h"

void CMT::initialize()
{
	fio = new FILEIO();
	play = rec = false;
}

void CMT::release()
{
	release_tape();
	delete fio;
}

void CMT::reset()
{
	close_tape();
	play = rec = false;
}

void CMT::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(rec) {
		// recv from sio
		buffer[bufcnt++] = data & mask;
		if(bufcnt >= BUFFER_SIZE) {
			fio->Fwrite(buffer, bufcnt, 1);
			bufcnt = 0;
		}
	}
}

void CMT::play_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fseek(0, FILEIO_SEEK_END);
		int size = (fio->Ftell() + 9) & (BUFFER_SIZE - 1);
		fio->Fseek(0, FILEIO_SEEK_SET);
		memset(buffer, 0, sizeof(buffer));
		fio->Fread(buffer, sizeof(buffer), 1);
		fio->Fclose();
		
		// send data to sio
		// this implement does not care the sio buffer size... :-(
		for(int i = 0; i < size; i++) {
			d_sio->write_signal(SIG_I8251_RECV, buffer[i], 0xff);
		}
		play = true;
	}
}

void CMT::rec_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		my_tcscpy_s(rec_file_path, _MAX_PATH, file_path);
		bufcnt = 0;
		rec = true;
	}
}

void CMT::close_tape()
{
	// close file
	release_tape();
	
	// clear sio buffer
	d_sio->write_signal(SIG_I8251_CLEAR, 0, 0);
}

void CMT::release_tape()
{
	// close file
	if(fio->IsOpened()) {
		if(rec && bufcnt) {
			fio->Fwrite(buffer, bufcnt, 1);
		}
		fio->Fclose();
	}
	play = rec = false;
}

#define STATE_VERSION	1

#include "../../statesub.h"

void CMT::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_BOOL(play);
	DECL_STATE_ENTRY_BOOL(rec);
	DECL_STATE_ENTRY_STRING(rec_file_path, sizeof(rec_file_path));
	DECL_STATE_ENTRY_CMT_RECORDING(fio, rec, rec_file_path);
	
	DECL_STATE_ENTRY_INT32(bufcnt);
	DECL_STATE_ENTRY_1D_ARRAY(buffer, sizeof(buffer));

	leave_decl_state();
}
void CMT::save_state(FILEIO* state_fio)
{
	uint32_t crc_value = 0xffffffff;
	if(state_entry != NULL) {
		state_entry->save_state(state_fio, &crc_value);
	}
	
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputBool(play);
//	state_fio->FputBool(rec);
//	state_fio->Fwrite(rec_file_path, sizeof(rec_file_path), 1);
//	state_fio->FputInt32(bufcnt);
//	state_fio->Fwrite(buffer, sizeof(buffer), 1);
}

bool CMT::load_state(FILEIO* state_fio)
{
	release_tape();
	
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	play = state_fio->FgetBool();
//	rec = state_fio->FgetBool();
//	state_fio->Fread(rec_file_path, sizeof(rec_file_path), 1);
//	int length_tmp = state_fio->FgetInt32();
	bool mb = false;
	bool stat;
	uint32_t crc_value = 0xffffffff;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio, &crc_value);
	}
	if(!mb) return false;
//	bufcnt = state_fio->FgetInt32();
//	state_fio->Fread(buffer, sizeof(buffer), 1);
	return true;
}

