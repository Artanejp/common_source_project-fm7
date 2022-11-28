/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ i/o trap ]
*/

#include "iotrap.h"
#include "../i8255.h"

void IOTRAP::initialize()
{
	nmi_mask = pasopia = false;
}

void IOTRAP::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		if(pasopia) {
			// trap : 8255-2 Port B, bit 1
			d_pio2->write_signal(SIG_I8255_PORT_B, 2, 2);
			// nmi
			if(!nmi_mask) {
				d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
			}
		}
		break;
	}
}

uint32_t IOTRAP::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		if(pasopia) {
			// trap : 8255-2 Port B, bit 1
			d_pio2->write_signal(SIG_I8255_PORT_B, 2, 2);
			// nmi
			if(!nmi_mask) {
				d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
			}
		}
		return 0xff;
	}
	return 0xff;
}

void IOTRAP::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_IOTRAP_I8255_2_A) {
		// reset nmi : 8255-2 Port B, bit 1+2
		if(data & 1) {
			d_pio2->write_signal(SIG_I8255_PORT_B, 0, 6);
		}
	} else if(id == SIG_IOTRAP_I8255_2_C) {
		nmi_mask = ((data & 0x80) != 0);
		pasopia = ((data & 0x40) != 0);
	}
}

void IOTRAP::do_reset()
{
	// reset : 8255-2 Port B, bit 2
	d_pio2->write_signal(SIG_I8255_PORT_B, 4, 4);
	// nmi
	if(!nmi_mask) {
		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
	}
}

#define STATE_VERSION	1

bool IOTRAP::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(nmi_mask);
	state_fio->StateValue(pasopia);
	return true;
}

