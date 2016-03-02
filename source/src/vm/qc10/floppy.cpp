/*
	EPSON QC-10 Emulator 'eQC-10'

	Author : Takeda.Toshiya
	Date   : 2008.02.18 -

	[ floppy ]
*/

#include "floppy.h"
#include "memory.h"
#include "../upd765a.h"

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	d_fdc->write_signal(SIG_UPD765A_MOTOR, 1, 1);
	d_mem->write_signal(SIG_MEMORY_MOTOR, 1, 1);
}

