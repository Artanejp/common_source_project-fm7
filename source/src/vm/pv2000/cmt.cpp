/*
	CASIO PV-2000 Emulator 'EmuGaki'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ cmt ]
*/

#include "cmt.h"

void CMT::initialize()
{
	fio = new FILEIO();
	memset(buffer, 0, sizeof(buffer));
	bufcnt = 0;
	bit = 1;
	start = 0;
	play = rec = false;
}

void CMT::release()
{
	close_tape();
	delete fio;
}

void CMT::reset()
{
	close_tape();
}

void CMT::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x00:
		// bit0 = motor, bit1 = rec, bit3 = play
		if(start == 0 && data == 3 && rec) {
			for(int i = 0; i < 60; i++) {
				buffer[bufcnt] |= bit;
				if(!(bit & 0x80)) {
					bit <<= 1;
				} else {
					if(++bufcnt == BUFFER_SIZE) {
						fio->Fwrite(buffer, sizeof(buffer), 1);
						memset(buffer, 0, sizeof(buffer));
						bufcnt = 0;
					}
					bit = 1;
				}
			}
		}
		start = data;
		break;
	case 0x60:
		// bit0 = signal
		if((start & 0x3) == 0x3 && rec) {
			buffer[bufcnt] |= (data & 1) ? bit : 0;
			if(!(bit & 0x80)) {
				bit <<= 1;
			} else {
				if(++bufcnt == BUFFER_SIZE) {
					fio->Fwrite(buffer, sizeof(buffer), 1);
					memset(buffer, 0, sizeof(buffer));
					bufcnt = 0;
				}
				bit = 1;
			}
		}
		break;
	}
}

uint32_t CMT::read_io8(uint32_t addr)
{
	// bit0 = signal
	uint32_t val = 2;
	if((start & 0x9) == 0x9 && play) {
		val |= (buffer[bufcnt] & bit ? 1 : 0);
		if(!(bit & 0x80)) {
			bit <<= 1;
		} else {
			if(++bufcnt == BUFFER_SIZE) {
				memset(buffer, 0, sizeof(buffer));
				fio->Fread(buffer, sizeof(buffer), 1);
				bufcnt = 0;
			}
			bit = 1;
		}
	}
	return val;
}

void CMT::play_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		// open for play
		fio->Fread(buffer, sizeof(buffer), 1);
		bufcnt = 0;
		bit = 1;
		play = true;
	}
}

void CMT::rec_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		// open for rec
		memset(buffer, 0, sizeof(buffer));
		bufcnt = 0;
		bit = 1;
		rec = true;
	}
}

void CMT::close_tape()
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

bool CMT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(bufcnt);
	state_fio->StateArray(buffer, sizeof(buffer), 1);
	state_fio->StateValue(play);
	state_fio->StateValue(rec);
	state_fio->StateValue(start);
	state_fio->StateValue(bit);
	return true;
}

