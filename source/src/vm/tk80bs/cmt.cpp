/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'
	NEC TK-80 Emulator 'eTK-80'

	Author : Takeda.Toshiya
	Date   : 2008.08.26 -

	[ cmt ]
*/

#include "cmt.h"
#include "../datarec.h"
#if defined(_TK80BS)
#include "../i8251.h"
#endif
#include "../i8255.h"

#define EVENT_PULSE	0

void CMT::initialize()
{
	mic = ear = pulse = false;
	pulse_count = 0;
	register_event(this, EVENT_PULSE, 1000000 / 4000, true, NULL);
	
#if defined(_TK80BS)
	fio = new FILEIO();
	play = rec = false;
#endif
}

#if defined(_TK80BS)
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
#endif

void CMT::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CMT_MIC) {
		// recv mic signal from pio
		mic = ((data & mask) != 0);
	} else if(id == SIG_CMT_EAR) {
		// recv ear signal from datarec
		bool next = ((data & mask) != 0);
		if(ear != next) {
			pulse_count = 0;
			ear = next;
		}
#if defined(_TK80BS)
	} else if(id == SIG_CMT_OUT) {
		// recv data from sio
		if(rec) {
			buffer[bufcnt++] = data & mask;
			if(bufcnt >= BUFFER_SIZE) {
				fio->Fwrite(buffer, bufcnt, 1);
				bufcnt = 0;
			}
		}
#endif
	}
}

void CMT::event_callback(int event_id, int err)
{
	// send mic signal to datarec
	d_drec->write_signal(SIG_DATAREC_MIC, (mic && pulse) ? 1 : 0, 1);
	pulse = !pulse;
	
	// send ear signal to pio
	if(pulse_count > 4) {
		// ear signal is not changed for 1 msec or more
		d_pio->write_signal(SIG_I8255_PORT_B, 0, 1);
	} else {
		d_pio->write_signal(SIG_I8255_PORT_B, 1, 1);
		pulse_count++;
	}
}

#if defined(_TK80BS)
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
#endif

#define STATE_VERSION	2

bool CMT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(mic);
	state_fio->StateValue(ear);
	state_fio->StateValue(pulse);
	state_fio->StateValue(pulse_count);
#if defined(_TK80BS)
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
#endif
	return true;
}

