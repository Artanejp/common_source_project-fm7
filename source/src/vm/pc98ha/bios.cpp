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

namespace PC98HA {

// regs
#define AL	regs8[0]
#define AH	regs8[1]

#define CALC_CYCLES(a,b,e)									  \
	{														  \
		__LIKELY_IF(a != NULL) {							  \
			*a -= e;										  \
		}													  \
		__LIKELY_IF(b != NULL) {							  \
			*b += (uint64_t)e;								  \
		}													  \
	}

	
bool BIOS::bios_int_i86(int intnum, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
{
	static const int check_cmds[16] = {1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0};
	uint8_t *regs8 = (uint8_t *)regs;
	int elapsed_cycles = 200; // TBD.
	if(intnum == 0x1b && check_cmds[AH & 0x0f]) {
		switch(AL & 0xf0) {
		case 0x10:
		case 0x70:
			// 2DD
			// ToDo: This will be FIX cycle value.
			CALC_CYCLES(cycles, total_cycles, elapsed_cycles); 
			if(d_fdc->is_disk_inserted(AL & 0x03) && d_fdc->get_media_type(AL & 0x03) != MEDIA_TYPE_2DD) {
				AH = 0xe0;
				*CarryFlag = 1;
				return true;
			}
			break;
		case 0x90:
		case 0xf0:
			// 2HD
			// ToDo: This will be FIX cycle value.
			CALC_CYCLES(cycles, total_cycles, elapsed_cycles); 
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

}
