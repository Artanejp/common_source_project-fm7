/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'

	Author : Takeda.Toshiya
	Date   : 2011.04.08-

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

void CMT::write_io8(uint32 addr, uint32 data)
{
	remote = ((data & 0x20) != 0);
}

void CMT::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_CMT_OUT) {
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
	if(rec && bufcnt) {
		fio->Fwrite(buffer, bufcnt, 1);
	}
	if(play || rec) {
		fio->Fclose();
	}
	play = rec = false;
}

#define STATE_VERSION	1

void CMT::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputInt32(bufcnt);
	state_fio->Fwrite(buffer, sizeof(buffer), 1);
	state_fio->FputBool(play);
	state_fio->FputBool(rec);
	state_fio->FputBool(remote);
}

bool CMT::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	bufcnt = state_fio->FgetInt32();
	state_fio->Fread(buffer, sizeof(buffer), 1);
	play = state_fio->FgetBool();
	rec = state_fio->FgetBool();
	remote = state_fio->FgetBool();
	return true;
}

