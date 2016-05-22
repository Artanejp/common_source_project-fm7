/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.27-

	[ Qt printer ]
*/

#include "osd.h"

void OSD_BASE::initialize_printer()
{
	prn_fio = new FILEIO();
	prn_data = -1;
	prn_strobe = false;
}

void OSD_BASE::release_printer()
{
	close_printer_file();
	delete prn_fio;
}



void OSD_BASE::printer_strobe(bool value)
{
	bool falling = (prn_strobe && !value);
	prn_strobe = value;
	
	if(falling) {
		if(!prn_fio->IsOpened()) {
			if(prn_data == -1) {
				return;
			}
			open_printer_file();
		}
		prn_fio->Fputc(prn_data);
		// wait 10sec
		prn_wait_frames = (int)(vm_frame_rate() * 10.0 + 0.5);
	}
}

void OSD_BASE::reset_printer()
{
	close_printer_file();
	prn_data = -1;
	prn_strobe = false;
}

void OSD_BASE::update_printer()
{
	if(prn_fio->IsOpened() && --prn_wait_frames == 0) {
		close_printer_file();
	}
}

void OSD_BASE::printer_out(uint8_t value)
{
	prn_data = value;
}

void OSD_BASE::open_printer_file()
{
	create_date_file_name(prn_file_name, _MAX_PATH, _T("txt"));
	prn_fio->Fopen(bios_path(prn_file_name), FILEIO_WRITE_BINARY);
}

void OSD_BASE::close_printer_file()
{
	if(prn_fio->IsOpened()) {
		// remove if the file size is less than 2 bytes
		bool remove = (prn_fio->Ftell() < 2);
		prn_fio->Fclose();
		if(remove) {
			FILEIO::RemoveFile(bios_path(prn_file_name));
		}
	}
}

