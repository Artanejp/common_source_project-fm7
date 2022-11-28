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

bool CMT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(play);
	state_fio->StateValue(rec);
	state_fio->StateArray(rec_file_path, sizeof(rec_file_path), 1);
	if(loading) {
		int length_tmp = state_fio->FgetInt32_LE();
		if(rec) {
			fio->Fopen(rec_file_path, FILEIO_READ_WRITE_NEW_BINARY);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				state_fio->Fread(buffer_tmp, length_rw, 1);
				if(fio->IsOpened()) {
					fio->Fwrite(buffer_tmp, length_rw, 1);
				}
				length_tmp -= length_rw;
			}
		}
	} else {
		if(rec && fio->IsOpened()) {
			int length_tmp = (int)fio->Ftell();
			fio->Fseek(0, FILEIO_SEEK_SET);
			state_fio->FputInt32_LE(length_tmp);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				fio->Fread(buffer_tmp, length_rw, 1);
				state_fio->Fwrite(buffer_tmp, length_rw, 1);
				length_tmp -= length_rw;
			}
		} else {
			state_fio->FputInt32_LE(0);
		}
	}
	state_fio->StateValue(bufcnt);
	state_fio->StateArray(buffer, sizeof(buffer), 1);
	return true;
}

