/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2010.05.12 -

	[ bios ]
*/

#include "bios.h"
#include "../upd765a.h"
#include "../disk.h"

// regs
#define AL	regs8[0]
#define AH	regs8[1]

bool BIOS::bios_int_i86(int intnum, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	static const int check_cmds[16] = {1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0};
	uint8_t *regs8 = (uint8_t *)regs;
	
	if(intnum == 0x1b && check_cmds[AH & 0x0f]) {
		switch(AL & 0xf0) {
		case 0x10:
		case 0x70:
			// 2DD
			if(d_fdc->is_disk_inserted(AL & 0x03) && d_fdc->get_media_type(AL & 0x03) != MEDIA_TYPE_2DD) {
				AH = 0xe0;
				*CarryFlag = 1;
				return true;
			}
			break;
		case 0x90:
		case 0xf0:
			// 2HD
			if(d_fdc->is_disk_inserted(AL & 0x03) && d_fdc->get_media_type(AL & 0x03) != MEDIA_TYPE_2HD) {
				AH = 0xe0;
				*CarryFlag = 1;
				return true;
			}
			break;
		}
	}
	return false;
}
