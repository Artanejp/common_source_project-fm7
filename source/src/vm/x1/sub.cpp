/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2013.05.01-

	[ sub system ]
*/

#include "sub.h"
#include "../datarec.h"
#include "../i8255.h"
#include "../mcs48.h"
#include "../upd1990a.h"

/*
SUB CPU
	P10	--> uPD1990AC CO
	P11	--> uPD1990AC C1
	P12	<-- RESET
	P13	--> uPD1990AC STB
	P14	--> uDP1990AC DIN
	P15	--> uPD1990AC CLK
	P16	--> 8255 A0
	P17	--> 8255 A1
	
	P20	--> MOTOR
	P21	--> PLAY
	P22	--> FF
	P23	--> REW
	P24	--> STOP
	P25	--> STAND-BY LED
	P26	--> IRQ
	P27	--> TV REMOTE
	
	DB0-7	<-> 8255 DATA
	
	T0	<-- TV POWER ON/OFF
	T1	<-- uPD1990AC DOUT
	
	INT	<-- KEY DATA

SUB 8255
	PA0-7	<-> 0x1900
	
	PB0	<-- TAPE END(L)
	PB1	<-- CASSET INSERTED(H)
	PB2	<-- REC PROTECT(H)
	PB3	<-- NC
	PB4	<-- EJECT SW(L)
	PB5	<-- APSS
	PB6	<-- ACK(L)
	PB7	<-- OBF(L)
	
	PC0	--> CMT EJECT
	PC1	--> BREAK(L)
	PC2	--> CMT WRITE LED
	PC3	--> NC
	PC4	<-- STB(L)
	PC5	--> IBF(H)
	PC6	<-- ACK(L)
	PC7	--> OBF(L)
*/

void SUB::reset()
{
	p1_out = p2_out = portc = 0xff;
	p1_in = 0;
	p2_in = 4;
	
	intr = obf = false;
	iei = true;
	
	close_tape();
}

#define SET_STB(v) { \
	d_pio->write_signal(SIG_I8255_PORT_C, (v) ? 0xff : 0, 0x10); \
}

#define SET_ACK(v) { \
	d_pio->write_signal(SIG_I8255_PORT_B, (v) ? 0xff : 0, 0x40); \
	d_pio->write_signal(SIG_I8255_PORT_C, (v) ? 0xff : 0, 0x40); \
}

void SUB::write_io8(uint32_t addr, uint32_t data)
{
	// FIXME: this function is referred from both 80c48 and z80
	if((addr & 0xff00) == 0x1900) {
		// for z80
//		this->out_debug_log(_T("Z80 -> PA=%2x\n"), data);
		d_pio->write_signal(SIG_I8255_PORT_A, data, 0xff);
		SET_STB(true);
		SET_STB(false);
		SET_STB(true);
	} else {
		// for 80c48
		switch(addr) {
		case MCS48_PORT_P1:
			d_rtc->write_signal(SIG_UPD1990A_C0,  data, 0x01);
			d_rtc->write_signal(SIG_UPD1990A_C1,  data, 0x02);
			d_rtc->write_signal(SIG_UPD1990A_STB, data, 0x08);
			d_rtc->write_signal(SIG_UPD1990A_DIN, data, 0x10);
			d_rtc->write_signal(SIG_UPD1990A_CLK, data, 0x20);
			p1_out = data;
			break;
		case MCS48_PORT_P2:
			if((p2_out & 0x02) && !(data & 0x02)) {
				d_drec->set_ff_rew(0);
				d_drec->set_remote(true);
			}
			if(!(p2_out & 0x02) && (data & 0x02)) {
				if(rom_crc32 == CRC32_MSM80C49_262) {
					d_drec->set_remote(false);
				}
			}
			if((p2_out & 0x04) && !(data & 0x04)) {
				d_drec->set_ff_rew(1);
				d_drec->set_remote(true);
			}
			if((p2_out & 0x08) && !(data & 0x08)) {
				d_drec->set_ff_rew(-1);
				d_drec->set_remote(true);
			}
			if(!(p2_out & 0x10) && (data & 0x10)) {
				d_drec->set_remote(false);
			}
			intr = ((data & 0x40) == 0);
			update_intr();
			p2_out = data;
			break;
		default:
			d_pio->write_io8(p1_out >> 6, data);
			break;
		}
	}
}

uint32_t SUB::read_io8(uint32_t addr)
{
	// FIXME: this function is referred from both 80c48 and z80
	if((addr & 0xff00) == 0x1900) {
		// for z80
		uint32_t value = d_pio->read_signal(SIG_I8255_PORT_A);
//		this->out_debug_log(_T("Z80 <- PA=%2x\n"), value);
		// don't check iei status (thanks Mr.Sato)
//		if(iei) {
			SET_ACK(true);
			SET_ACK(false);
			SET_ACK(true);
//		}
		return value;
	} else {
		// for 80c48
		switch(addr) {
		case MCS48_PORT_P1:
			{
				uint32_t value = p1_in & p1_out;
				p1_in &= ~4;	// P12
				return value | 4;
			}
		case MCS48_PORT_P2:
			return p2_in & p2_out;
		case MCS48_PORT_T0:
			return 1;	// FIXME: always tv on
		case MCS48_PORT_T1:
			return d_rtc->read_signal(0);
		default:
			return d_pio->read_io8(p1_out >> 6);
		}
	}
	return 0xff;
}

void SUB::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_SUB_PIO_PORT_C) {
		if((portc & 0x01) && !(data & 0x01)) {
			emu->close_tape(0);
		}
		obf = ((data & 0x80) == 0);
		portc = data;
		update_intr();
	} else if(id == SIG_SUB_TAPE_END) {
		tape_eot = ((data & mask) != 0);
		update_tape();
	} else if(id == SIG_SUB_TAPE_APSS) {
		tape_apss = ((data & mask) != 0);
		update_tape();
	}
}

void SUB::play_tape(bool value)
{
	tape_play = value;
	tape_rec = tape_eot = tape_apss = false;
	update_tape();
}

void SUB::rec_tape(bool value)
{
	tape_rec = value;
	tape_play = tape_eot = tape_apss = false;
	update_tape();
}

void SUB::close_tape()
{
	tape_play = tape_rec = tape_eot = tape_apss = false;
	update_tape();
}

void SUB::update_tape()
{
	if(rom_crc32 != CRC32_MSM80C49_277) {
		uint32_t value = 0x10;
//		if(!(tape_play && tape_eot)) {
		if((tape_play || tape_rec) && !tape_eot) {
			value |= 0x01;	// tape end
		}
		if(tape_play || tape_rec) {
			value |= 0x02;	// cassette inserted
		}
		if(tape_rec) {
			value |= 0x04;	// rec protected
		}
		if(rom_crc32 == CRC32_MSM80C49_262) {
			value ^= 0x06;	// X1F/G or X1turbo
		}
		if(tape_play && tape_apss) {
			value |= 0x20;
		}
		d_pio->write_signal(SIG_I8255_PORT_B, value, 0x3f);
	} else {
		// X1turbo without CMT
		d_pio->write_signal(SIG_I8255_PORT_B, 0x3f, 0x3f);
	}
}

// z80 daisy cahin

void SUB::set_intr_iei(bool val)
{
	if(iei != val) {
		iei = val;
		update_intr();
	}
}

uint32_t SUB::get_intr_ack()
{
	intr = false;
	return read_io8(0x1900);
}

void SUB::notify_intr_reti()
{
	// NOTE: some software uses RET, not RETI ???
}

void SUB::update_intr()
{
	if(intr && obf && iei) {
		d_cpu->set_intr_line(true, true, intr_bit);
	} else {
		d_cpu->set_intr_line(false, true, intr_bit);
	}
}

#define STATE_VERSION	1

bool SUB::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(p1_out);
	state_fio->StateValue(p1_in);
	state_fio->StateValue(p2_out);
	state_fio->StateValue(p2_in);
	state_fio->StateValue(portc);
	state_fio->StateValue(tape_play);
	state_fio->StateValue(tape_rec);
	state_fio->StateValue(tape_eot);
	state_fio->StateValue(tape_apss);
	state_fio->StateValue(intr);
	state_fio->StateValue(obf);
	state_fio->StateValue(iei);
	state_fio->StateValue(intr_bit);
	return true;
}

