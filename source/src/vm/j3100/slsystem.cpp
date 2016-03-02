/*
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

	[ system ]
*/

#include "slsystem.h"

void SYSTEM::initialize()
{
	nmi_mask = 0;
}

/*
e3	out	word	ff5a
e4	in	byte	??
e5	out	word	865a/ff5a/d55a
e6	in	byte	??
e8	out	byte	5a
e9	out	byte	bf/9f
ea	out	byte	5a
eb	out	byte	e0
*/

void SYSTEM::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0xa0:
		nmi_mask = data;
		break;
	}
}

uint32_t SYSTEM::read_io8(uint32_t addr)
{
	return 0xff;
}

