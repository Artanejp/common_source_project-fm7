/*
	EPS TRN Junior Emulator 'eTRNJunior'

	Author : Takeda.Toshiya
	Date   : 2022.07.02-

	[ memory bus ]
*/

#include "membus.h"
#include "../tmpz84c015.h"

uint32_t MEMBUS::fetch_op(uint32_t addr, int *wait)
{
	d_cpudev->write_signal(SIG_TMPZ84C015_CTC_TRIG_3, 1, 1);
	d_cpudev->write_signal(SIG_TMPZ84C015_CTC_TRIG_3, 0, 1);
	*wait = 0;
	return MEMORY::read_data8(addr);
}
