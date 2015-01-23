/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.16 -

	[ cmt ]
*/

#include "cmt.h"
#include "../i8251.h"
#include "../../fileio.h"

void CMT::initialize()
{
	fio = new FILEIO();
	play = rec = remote = false;
}

void CMT::release()
{
	release_tape();
	delete fio;
}

void CMT::reset()
{
	close_tape();
	play = rec = remote = false;
}

void CMT::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_CMT_REMOTE) {
		remote = ((data & mask) != 0);
	} else if(id == SIG_CMT_OUT) {
		if(rec && remote) {
			// recv from sio
			buffer[bufcnt++] = data & mask;
			if(bufcnt >= BUFFER_SIZE) {
				fio->Fwrite(buffer, bufcnt, 1);
				bufcnt = 0;
			}
		}
	}
}

void CMT::play_tape(_TCHAR* file_path)
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

void CMT::rec_tape(_TCHAR* file_path)
{
	close_tape();
	
	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		_tcscpy_s(rec_file_path, _MAX_PATH, file_path);
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

void CMT::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(rec_file_path, sizeof(rec_file_path), 1);
	if(rec && fio->IsOpened()) {
		int length_tmp = (int)fio->Ftell();
		fio->Fseek(0, FILEIO_SEEK_SET);
		state_fio->FputInt32(length_tmp);
		while(length_tmp != 0) {
			uint8 buffer_tmp[1024];
			int length_rw = min(length_tmp, sizeof(buffer_tmp));
			fio->Fread(buffer_tmp, length_rw, 1);
			state_fio->Fwrite(buffer_tmp, length_rw, 1);
			length_tmp -= length_rw;
		}
	} else {
		state_fio->FputInt32(0);
	}
	state_fio->FputInt32(bufcnt);
	state_fio->Fwrite(buffer, sizeof(buffer), 1);
	state_fio->FputBool(play);
	state_fio->FputBool(rec);
	state_fio->FputBool(remote);
}

bool CMT::load_state(FILEIO* state_fio)
{
	int length_tmp;
	
	release_tape();
	
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(rec_file_path, sizeof(rec_file_path), 1);
	if((length_tmp = state_fio->FgetInt32()) != 0) {
		fio->Fopen(rec_file_path, FILEIO_READ_WRITE_NEW_BINARY);
		while(length_tmp != 0) {
			uint8 buffer_tmp[1024];
			int length_rw = min(length_tmp, sizeof(buffer_tmp));
			state_fio->Fread(buffer_tmp, length_rw, 1);
			if(fio->IsOpened()) {
				fio->Fwrite(buffer_tmp, length_rw, 1);
			}
			length_tmp -= length_rw;
		}
	}
	bufcnt = state_fio->FgetInt32();
	state_fio->Fread(buffer, sizeof(buffer), 1);
	play = state_fio->FgetBool();
	rec = state_fio->FgetBool();
	remote = state_fio->FgetBool();
	return true;
}

