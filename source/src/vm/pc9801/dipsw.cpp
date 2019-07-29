/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.07.29-

	[ dipswitch ]
*/

#include "./dipsw.h"
#include "../i8255.h"
#include "../../config.h"

namespace PC9801 {
// Still ONLY SUPPORT PHYSICAL DIPSWITCH
void DIPSWITCH::initialize()
{
	update_dipswitch();
}

void DIPSWITCH::reset()
{
	update_dipswitch();
	update_ports();
	uint8_t port_c3 = 0x00;
#if defined(SUPPORT_HIRESO)
	if(config.monitor_type == 0) {
		port_c3 |= 0x08; // DIP SW 1-1, 1 = Hiresolution CRT, 0 = Standard CRT
	}
#else
	port_c3 |= 0x08; // MODSW, 1 = Normal Mode, 0 = Hirezo Mode
#endif
#if defined(USE_CPU_TYPE)
	#if defined(HAS_V30_SUB_CPU)
		if(config.cpu_type & 0x02) {// V30 or V33
			port_c3 |= 0x04;
		}
	#endif
#endif
	pio_mouse->write_signal(SIG_I8255_PORT_C, port_c3, 0x0c);

}

void DIPSWITCH::update_dipswitch()
{
	sw1 = 0;
	sw2 = 0;
	sw3 = 0;
#if 1 /* HARDWARE DIP SWITCH */
	sw1 |= (((config.dipswitch & (0xff << 8)) >> 8) & 0xff);
#if 1
	sw1 &= 0xfe;
	sw1 |= ((config.monitor_type == 0) ? 0x01 : 0x00);
#endif
 	sw2 |= (((config.dipswitch & (0xff << 16)) >> 16) & 0xff);
	sw2 = (sw2 & 0x0d) | ((~sw2) & 0xf2); 
	sw3 |= (((config.dipswitch & (0xff << 24)) >> 24) & 0x7f); 
#endif
}

void DIPSWITCH::update_ports()
{
	if(pio_sys != NULL) {
		uint8_t port_a;
		uint8_t port_b = 0x00;
		port_a = sw2;
		pio_sys->write_signal(SIG_I8255_PORT_A, port_a, 0xff);
#if !defined(SUPPORT_HIRESO)
		port_b |= ((sw1 & 0x01) != 0) ? 0x08 : 0x00;
		pio_sys->write_signal(SIG_I8255_PORT_B, port_b, 0x08);
#endif
	}
#if !defined(SUPPORT_HIRESO)
	if(pio_prn != NULL) {
		uint8_t port_b = 0x00;
#if defined(_PC9801)
		//	port_b |= 0x80; // TYP1, 0 = PC-9801 (first)
		//	port_b |= 0x40; // TYP0, 0
#elif defined(_PC9801U)
		port_b |= 0x80; // TYP1, 1 = PC-9801U
		port_b |= 0x40; // TYP0, 1
#else
		port_b |= 0x80; // TYP1, 1 = Other PC-9801 series
	//	port_b |= 0x40; // TYP0, 0
#endif
		port_b |= ((sw1 & (1 << 2)) != 0) ? 0x10 : 0x00; // DIPSW 1-3 
		port_b |= ((sw1 & (1 << 7)) == 0) ? 0x08 : 0x00; // DIPSW 1-8
#if defined(_PC9801VF) || defined(_PC9801U)
		port_b |= 0x01; // VF
#endif
		pio_prn->write_signal(SIG_I8255_PORT_B, port_b, (0x80 | 0x40 | 0x10 | 0x08 | 0x01));
	}
#endif
	if(pio_mouse != NULL) {
		uint8_t port_a = 0x00;
		uint8_t port_b = 0x00;
		uint8_t port_c = 0x00;
#if defined(SUPPORT_HIRESO)
	#if defined(_PC98XA)
		port_b |= (((sw2 & (1 << 7)) == 0) ? 0x80 : 0x00);
		port_b |= (((sw2 & (1 << 6)) == 0) ? 0x40 : 0x00);
		pio_mouse->write_signal(SIG_I8255_PORT_B, port_b, 0xc0);
	#else
		port_b |= (((sw1 & (1 << 3)) == 0) ? 0x80 : 0x00); // SW 1-4
		#if defined(_PC98RL)
		port_b |= (((sw3 & (1 << 5)) == 0) ? 0x40 : 0x00); // SW 3-6
		#endif
		#if defined(_PC98RL) || defined(_PC9821_VARIANTS)
		port_b |= (((sw3 & (1 << 2)) == 0) ? 0x10 : 0x00); // SW 3-3
		#endif
		#if defined(_PC98RL)
		pio_mouse->write_signal(SIG_I8255_PORT_B, port_b, 0xd0);
		#elif defined(_PC9821_VARIANTS)
		pio_mouse->write_signal(SIG_I8255_PORT_B, port_b, 0x90);
		#else
		pio_mouse->write_signal(SIG_I8255_PORT_B, port_b, 0x80);
		#endif
	#endif
		// ToDo: MODSW (PORTC, bit3)
		//port_c |= 0x08;
		// ToDo: DIPSW 3-8
		port_c |= (((sw3 & (1 << 7)) == 0) ? 0x00 : 0x04); // CPUSW (DIPSW 3-8)
		#if defined(_PC98XA)
		// ToDo: DIPSW 1-10, 1-9
		#else
		port_c |= (((sw1 & (1 << 5)) != 0) ? 0x02 : 0x00); // DIPSW 1-6
		port_c |= (((sw1 & (1 << 4)) != 0) ? 0x01 : 0x00); // DIPSW 1-5
		#endif
		pio_mouse->write_signal(SIG_I8255_PORT_C, port_c, 0x0f);
#else
	#if defined(_PC98RL)
		port_b |= (((sw1 & (1 << 3)) != 0) ? 0x80 : 0x00); // SW 1-4
	#else
		port_b |= 0x80;
	#endif
		port_b |= (((sw3 & (1 << 5)) == 0) ? 0x40 : 0x00); // SW 3-6
	#if defined(_PC9801RX2) | defined(_PC9801RX21) |  defined(_PC9801EX) |  defined(_PC9801DX) 
	#if defined(USE_CPU_TYPE)
		if((config.cpu_type & 1) == 1) {
		#if !defined(SUPPORT_HIRESO)
				port_b |= 0x02;
		#else
				port_b |= 0x01;
		#endif
		}
		#if !defined(SUPPORT_HIRESO)
		pio_mouse->write_signal(SIG_I8255_PORT_B, port_b, 0x02);
		#else
		pio_mouse->write_signal(SIG_I8255_PORT_B, port_b, 0x01);
		#endif		
	#endif
	#endif
		pio_mouse->write_signal(SIG_I8255_PORT_B, port_b, 0xc0);
		// ToDo: MODWS (DIPSW 1-1)
	#if defined(_PC98XL) || defined(_PC98XL2) || defined(_PC98RL)
		port_c |= (((sw1 & (1 << 0)) != 0) ? 0x08 : 0x00); // SW 1-1
	#elif defined(_PC9821_VARIANTS)
		port_c |= 0x08;
	#endif
	#if defined(_PC98XA)
		//port_c |= 0x02; //SW 1-10
		//port_c |= 0x01; //SW 1-9
	#else
		port_c |= (((sw1 & (1 << 5)) != 0) ? 0x02 : 0x00); // SW 1-6
		port_c |= (((sw1 & (1 << 4)) != 0) ? 0x01 : 0x00); // SW 1-5
	#endif
		pio_mouse->write_signal(SIG_I8255_PORT_B, port_c, (0x08 | 0x02 | 0x01));
	#if defined(HAS_V30_SUB_CPU) && defined(USE_CPU_TYPE)
		if(config.cpu_type & 0x02) {// V30 or V33
			pio_mouse->write_signal(SIG_I8255_PORT_C, 0x04, 0x04);
		} else {
			pio_mouse->write_signal(SIG_I8255_PORT_C, 0x00, 0x04);
		}
	#endif
#endif
	}		
}

void DIPSWITCH::update_config()
{
	update_dipswitch();
	update_ports();
}

#define STATE_VERSION	1

bool DIPSWITCH::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(sw1);
	state_fio->StateValue(sw2);
	state_fio->StateValue(sw3);
	if(loading) {
		update_ports();
	}
	return true;
}
}
