/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : Takeda.Toshiya
	Date   : 2014.05.22-

	[ timer ]
*/

#include "timer.h"
#ifndef _PC6001
#include "memory.h"
#endif

#define EVENT_TIMER	0

void TIMER::initialize()
{
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	memset(sr_vectors, 0, sizeof(sr_vectors));
#endif
}

void TIMER::reset()
{
	IRQ = NewIRQ = 0;
	timer_id = -1;
	
#ifndef _PC6001
	portF3 = 0x00;
	portF4 = 0x02; //???
	portF5 = 0x16;
	portF7 = 0x06;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	portFA = 0x00;
	portFB = 0x00;
	portF6 = 0x7f;
#else
	portF6 = 0x03;
#endif
#endif
}

#ifndef _PC6001
void TIMER::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	case 0xB8:
	case 0xB9:
	case 0xBA:
	case 0xBB:
	case 0xBC:
	case 0xBD:
	case 0xBE:
	case 0xBF:
		sr_vectors[addr & 7] = data;
		break;
#endif
	case 0xF3:
		portF3 = data;
//		d_mem->set_portF3(data);
		break;
	case 0xF4:
		portF4 = data;
		break;
	case 0xF5:
		portF5 = data;
		break;
	case 0xF6:
		portF6 = data;
		break;
	case 0xF7:
		portF7 = data;
		break;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	case 0xFA:
		portFA = data;
		break;
	case 0xFB:
		portFB = data;
		break;
#endif
	}
}

uint32 TIMER::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	case 0xB8:
	case 0xB9:
	case 0xBA:
	case 0xBB:
	case 0xBC:
	case 0xBD:
	case 0xBE:
	case 0xBF:
		return sr_vectors[addr & 7];
#endif
	case 0xF3:
		return portF3;
	case 0xF4:
		return portF4;
	case 0xF5:
		return portF5;
	case 0xF6:
		return portF6;
	case 0xF7:
		return portF7;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	case 0xFA:
		return portFA;
	case 0xFB:
		return portFB;
#endif
	}
	return 0xff;
}
#endif

void TIMER::event_callback(int event_id, int err)
{
	if(event_id == EVENT_TIMER) {
		write_signal(SIG_TIMER_IRQ_TIMER, 1, 1);
		timer_id = -1;
		// register next event
#ifdef _PC6001
		register_event(this, EVENT_TIMER, 2000.0, false, &timer_id);
#else
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
		if(vm->sr_mode) {
			register_event(this, EVENT_TIMER, 2000.0 * (portF6 + 1) / 0x80, false, &timer_id);
		} else
#endif
		register_event(this, EVENT_TIMER, 2000.0 * (portF6 + 1) / 4, false, &timer_id);
#endif
	}
}

void TIMER::set_portB0(uint32 data)
{
	if(data & 1) {
		// stop
		if(timer_id != -1) {
			cancel_event(this, timer_id);
			timer_id = -1;
		}
	} else {
		// start
		if(timer_id == -1) {
#ifdef _PC6001
			// first period is 1msec
			register_event(this, EVENT_TIMER, 1000.0, false, &timer_id);
#else
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
			if(vm->sr_mode) {
				register_event(this, EVENT_TIMER, 2000.0 * (portF6 + 1) / 0x80, false, &timer_id);
			} else
#endif
			register_event(this, EVENT_TIMER, 2000.0 * (portF6 + 1) / 4, false, &timer_id);
#endif
		}
	}
}

void TIMER::write_signal(int id, uint32 data, uint32 mask)
{
	if(data & mask) {
		IRQ |= (1 << id);
	} else {
		IRQ &= ~(1 << id);
	}
	update_intr();
}

uint32 TIMER::intr_ack()
{
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	if(vm->sr_mode) {
		for(int i = 0, bit = 1; i < 8; i++, bit <<= 1) {
			if(NewIRQ & bit) {
				if(portFB & bit) {
					IRQ = NewIRQ = 0;
					return sr_vectors[i];
				}
				break;
			}
		}
	}
#endif
	if(NewIRQ & 0x01) {		// Sub-CPU
		IRQ = NewIRQ = 0;
#ifndef _PC6001
		if(portF3 & 0x08) {
			return portF4;
		}
#endif
		return d_sub->intr_ack();
	}
#ifndef _PC6001
	else if(NewIRQ & 0x02) {	// Joystick
		IRQ = NewIRQ = 0;
		if(portF3 & 0x10) {
			return portF5;
		}
		return 0x16;
	}
#endif
	else if(NewIRQ & 0x04) {	// Timer
		IRQ = NewIRQ = 0;
#ifndef _PC6001
		return portF7;
#else
		return 0x06;
#endif
	}
#ifndef _PC6001
	else if(NewIRQ & 0x08) {	// Voice
		IRQ = NewIRQ = 0;
		return 0x20;
	}
	else if(NewIRQ & 0x10) {	// VRTC
		IRQ = NewIRQ = 0;
		return 0x22;
	}
	else if(NewIRQ & 0x20) {	// RS-232C
		IRQ = NewIRQ = 0;
		return 0x04;
	}
	else if(NewIRQ & 0x40) {	// Printer
		IRQ = NewIRQ = 0;
//		return 0x00;
	}
	else if(NewIRQ & 0x80) {	// Ext Int
		IRQ = NewIRQ = 0;
//		return 0x00;
	}
#endif
	return 0x06;	// dummy
}

void TIMER::intr_reti()
{
	update_intr();
}

void TIMER::update_intr()
{
	NewIRQ = IRQ;
#ifdef _PC6001
	NewIRQ &= 0x05;	// Only Sub-CPU and Timer
#else
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	if(vm->sr_mode) {
		NewIRQ &= ~portFA;
	} else
#endif
	{
		if(portF3 & 0x01) {
			NewIRQ &= ~0x01;
		}
		if(portF3 & 0x02) {
			NewIRQ &= ~0x02;
		}
		if(portF3 & 0x04) {
			NewIRQ &= ~0x04;
		}
	}
#endif
	if(NewIRQ) {
		d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
//	} else {
//		d_cpu->write_signal(SIG_CPU_IRQ, 0, 1);
	}
}

