/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.27-

	[ Qt printer ]
*/

#include "../emu.h"

void OSD::initialize_printer()
{
	prn_fio = new FILEIO();
	prn_data = -1;
	prn_strobe = false;
}

void OSD::release_printer()
{
	close_printer_file();
	delete prn_fio;
}



