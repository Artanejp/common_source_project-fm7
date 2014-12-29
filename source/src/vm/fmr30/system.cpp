/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

	[ system ]
*/

#include "system.h"

void SYSTEM::initialize()
{
	arr = nmistat = 0;
	nmimask = 0xf0;
}

void SYSTEM::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xffff) {
	case 0x46:
		nmistat &= ~data;
		break;
	case 0x47:
		nmimask = data;
		break;
	case 0xff00:
		arr = data;
		break;
	}
}

uint32 SYSTEM::read_io8(uint32 addr)
{
	switch(addr & 0xffff) {
	case 0x18:
		// modem:	no
		// scsi:	yes
		// fd23:	3.5inch
		// ext-ram:	yes (1mb)
		// co-pro	no
		return 0x24;
	case 0x20:
	case 0x21:
		// isrr high/low
		return 0;
	case 0x46:
		return nmistat;
	case 0x47:
		return nmimask;
	case 0xff00:
		return arr;
	}
	return 0xff;
}

