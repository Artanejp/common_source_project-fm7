/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2021.02.06-

	[ printer ]
*/

#include "printer.h"

void PRINTER::initialize()
{
	fio = new FILEIO();
	emu->get_osd()->open_console(80, 25, create_string(_T("Printer - %s"), _T(DEVICE_NAME)));
	
	register_vline_event(this);
}

void PRINTER::release()
{
	if(fio->IsOpened()) {
		fio->Fclose();
	}
	delete fio;
	emu->get_osd()->close_console();
}

void PRINTER::reset()
{
	if(fio->IsOpened()) {
		fio->Fclose();
	}
	column = htab = 0;
	
	strobe = outdata = 0;
}

/*
$E210: read / write
$E211: read（してからreti) / write ($0193の指すアドレスから転送)
$E212: read (bit操作して$E210にwrite)
*/

void PRINTER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0xe210:
		// C1 -> A1で$E211のデータが出力される？
		if((strobe & 0xe1) == 0xc1 && (data & 0xe1) == 0xa1) {
//			output(outdata);
		}
		strobe = data;
		break;
	case 0xe211:
		outdata = data;
		break;
	}
}

uint32_t PRINTER::read_io8(uint32_t addr)
{
	uint32_t value = 0xff;
	
	switch(addr & 0xffff) {
	case 0xe210:
		// 0x00, 0x80, 0x81 ?
		// bit7: 1=AUTO PRINT ?
		// bit0: PE or BUSY ?
		value = (config.dipswitch & 1) ? 0 : 0x80;
		break;
	case 0xe211:
		value = outdata;
		break;
	case 0xe212:
		value = strobe;
		break;
	}
	return value;
}

void PRINTER::event_vline(int v, int clock)
{
	// ugly patch for printer :-(
	// this code should be called from interrupt handler
#if 0
	0000BDF8  FE0265            ldx  $0265
	0000BDFB  8D0B              bsr  $BE08
	0000BDFD  BC0263            cmpx $0263
	0000BE00  2705              beq  $BE07
	0000BE02  E600              ldb  (x+$00)
	0000BE04  FF0265            stx  $0265
	0000BE07  39                rts  
	0000BE08  08                inx  
	0000BE09  8C0295            cmpx #$0295
	0000BE0C  2603              bne  $BE11
	0000BE0E  CE026E            ldx  #$026E
	0000BE11  39                rts  
#endif
	pair16_t p1, p2;
	p1.read_2bytes_be_from(ram + 0x265);
	p2.read_2bytes_be_from(ram + 0x263);
	p1.w++;
	if(p1.w == 0x295) {
		p1.w = 0x26e;
	}
	if(p1.w != p2.w) {
		uint8_t b = ram[p1.w];
		if(htab) {
			while(column < b) {
				output(0x20);
				column++;
			}
			htab = 0;
		} else if(b >= 0x20 && b < 0x80) {
			output(b);
			column++;
		} else if(b == 0x12) {
			output(0x0d);
			output(0x0a);
			column = 0;
		} else if(b == 0xf0) {
			htab = 1;
		}
		p1.write_2bytes_be_to(ram + 0x265);
	}
}

void PRINTER::output(uint8_t value)
{
	if(config.printer_type == 0) {
		if(!fio->IsOpened()) {
			_TCHAR file_path[_MAX_PATH];
			create_date_file_path(file_path, _MAX_PATH, _T("txt"));
			fio->Fopen(file_path, FILEIO_WRITE_BINARY);
		}
		fio->Fputc(value);
	}
	char temp[2];
	temp[0] = (char)value;
	temp[1] = 0;
	emu->get_osd()->write_console(char_to_tchar(temp), 1);
}

void PRINTER::key_down(int code)
{
	// ugly patch for PAPER FEED
	if(code == 0x91) {
		output(0x0d);
		output(0x0a);
	}
}

void PRINTER::key_up(int code)
{
}

#define STATE_VERSION	1

bool PRINTER::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(column);
	state_fio->StateValue(htab);
	state_fio->StateValue(strobe);
	state_fio->StateValue(outdata);
	return true;
}

