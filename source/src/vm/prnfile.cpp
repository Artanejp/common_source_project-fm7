/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.12.18-

	[ dummy printer ]
*/

#include "prnfile.h"

void PRNFILE::initialize()
{
	fio = new FILEIO();
	register_frame_event(this);
	
	value = -1;
	strobe = false;
}

void PRNFILE::release()
{
	close_file();
	delete fio;
}

void PRNFILE::reset()
{
	close_file();
	value = -1;
	strobe = false;
}

void PRNFILE::event_frame()
{
	if(fio->IsOpened() && --wait_frames == 0) {
		close_file();
	}
}

void PRNFILE::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_PRINTER_DATA) {
		value = data;
	} else if(id == SIG_PRINTER_STROBE) {
		bool new_strobe = ((data & mask) == 0);
		bool falling = (strobe && !new_strobe);
		strobe = new_strobe;
		
		if(falling) {
			if(!fio->IsOpened()) {
				if(value == -1) {
					return;
				}
				open_file();
			}
			fio->Fputc(value);
			// wait 10sec
#ifdef SUPPORT_VARIABLE_TIMING
			wait_frames = (int)(vm->frame_rate() * 10.0 + 0.5);
#else
			wait_frames = (int)(FRAMES_PER_SEC * 10.0 + 0.5);
#endif
		}
	}
}

uint32 PRNFILE::read_signal(int ch)
{
	if(ch == SIG_PRINTER_BUSY) {
		return 0;
	}
	return 0;
}

void PRNFILE::open_file()
{
	create_date_file_path(file_path, _MAX_PATH, _T("txt"));
	fio->Fopen(file_path, FILEIO_WRITE_BINARY);
}

void PRNFILE::close_file()
{
	if(fio->IsOpened()) {
		// remove if the file size is less than 2 bytes
		bool remove = (fio->Ftell() < 2);
		fio->Fclose();
		if(remove) {
			FILEIO::RemoveFile(file_path);
		}
	}
}

