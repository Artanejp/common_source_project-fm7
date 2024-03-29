/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2005.06.10-

	[ i8259 ]
*/

#include "i8259.h"

void I8259::initialize()
{
	DEVICE::initialize();
	__I8259_PC98_HACK = osd->check_feature(_T("I8259_PC98_HACK"));
	for(uint32_t c = 0; c < 4; c++) {
		pic[c].imr = 0xff;
		pic[c].irr = pic[c].isr = pic[c].prio = 0;
		pic[c].icw1 = pic[c].icw2 = pic[c].icw3 = pic[c].icw4 = 0;
		pic[c].ocw3 = 2;
		pic[c].icw2_r = pic[c].icw3_r = pic[c].icw4_r = 0;
		#if 0
		if(__I8259_PC98_HACK) {
			pic[c].icw4 = 1;
		}
		#endif
	}
}

void I8259::reset()
{
	for(int c = 0; c < 4; c++) {
		pic[c].irr_tmp = 0;
		if(pic[c].irr_tmp_id != -1) {
			cancel_event(this, pic[c].irr_tmp_id);
		}
		pic[c].irr_tmp_id = -1;
		#if 0
		if(__I8259_PC98_HACK) {
			pic[c].icw4 = 0x01;
		}
		#endif
	}
}

void I8259::write_io8(uint32_t addr, uint32_t data)
{
	int c = (addr >> 1) % num_chips;

	if(addr & 1) {
		if(pic[c].icw2_r) {
			// icw2
			pic[c].icw2 = data;
			pic[c].icw2_r = 0;
//			out_debug_log("Set ICW2 to %02X\n", data);
		} else if(pic[c].icw3_r) {
			// icw3
			pic[c].icw3 = data;
			pic[c].icw3_r = 0;
		} else if(pic[c].icw4_r) {
			// icw4
			#if 0
			if(__I8259_PC98_HACK) {
				pic[c].icw4 = data | 0x01; // For PC9801, may force to set 8086 mode.
			} else {
				pic[c].icw4 = data;
			}
			#else
			pic[c].icw4 = data;
			pic[c].icw4_r = 0;
			#endif
		} else {
			// ocw1
			uint8_t irr = pic[c].irr;
			for(int i = 0; i < 8; i++) {
				uint8_t bit = 1 << i;
				if((pic[c].irr & bit) && (pic[c].imr & bit) && !(data & bit)) {
					pic[c].irr &= ~bit;
					pic[c].irr_tmp |= bit;
				}
			}
			if(irr != pic[c].irr) {
				if(pic[c].irr_tmp_id != -1) {
					cancel_event(this, pic[c].irr_tmp_id);
				}
				register_event(this, c, 10, false, &pic[c].irr_tmp_id);
			}
			pic[c].imr = data;
		}
	} else {
		if(data & 0x10) {
			// icw1
			pic[c].icw1 = data;
			pic[c].icw2_r = 1;
			pic[c].icw3_r = (data & 2) ? 0 : 1;
			pic[c].icw4_r = data & 1;

			pic[c].irr = 0;
			pic[c].irr_tmp = 0;
			pic[c].isr = 0;
			pic[c].prio = 0;
			pic[c].imr = 0;
			if(!(pic[c].icw1 & 1)) {
				pic[c].icw4 = 0;
			}
			#if 0
			if(__I8259_PC98_HACK) {
				pic[c].icw4 |= 0x01;
			}
			#endif
			pic[c].ocw3 = 0;
		} else if((data & 0x98) == 0x08) {
			// ocw3
			#if 0
			if(__I8259_PC98_HACK) {
				uint8_t ocw3 = pic[c].ocw3;
				if((data & 0x02) == 0) { // OCW3_RR
					data &= ~(0x01); // RIS
					data |= (ocw3 & 0x01);
				}
				if((data & 0x40) == 0) { // ESMM
					data &= ~(0x20); // SMM
					data |= (ocw3 & 0x20);
				}
			}
			#endif
			pic[c].ocw3 = data;
		} else if((data & 0x18) == 0x00) {
			// ocw2
			int n = data & 7;
			uint8_t mask = 1 << n;
			uint8_t level = n;
			#if 0
			if(!(__I8259_PC98_HACK)) {
				if((data & 0x40) == 0) {  // OCW2_SL
					if(pic[c].isr == 0) {
						goto __throughfall;
					}
					level = pic[c].prio;
					while((pic[c].isr & (1 << level)) == 0) {
						level = (level + 1) & 7;
					}
				}
			}
			#endif
			switch(data & 0xe0) {
			case 0x00:
				pic[c].prio = 0;
				break;
			case 0x20:  // EOI
				for(n = 0, mask = (1 << pic[c].prio); n < 8; n++, mask = (mask << 1) | (mask >> 7)) {
					if(pic[c].isr & mask) {
						pic[c].isr &= ~mask;
						break;
					}
				}
				break;
			case 0x40:
				break;
			case 0x60:
				if(pic[c].isr & mask) {
					pic[c].isr &= ~mask;
				}
				break;
			case 0x80:
				#if 0
				if(!(__I8259_PC98_HACK)) {
					pic[c].prio = (pic[c].prio + 1) & 7;
				} else {
					pic[c].prio = (level + 1) & 7;
				}
				#else
				pic[c].prio = (pic[c].prio + 1) & 7;
				#endif
				break;
			case 0xa0:  // EOI
//				if(__I8259_PC98_HACK) {
//					pic[c].prio = (level + 1) & 7;
//				}
				for(n = 0, mask = (1 << pic[c].prio); n < 8; n++, mask = (mask << 1) | (mask >> 7)) {
					if(pic[c].isr & mask) {
						pic[c].isr &= ~mask;
						pic[c].prio = (pic[c].prio + 1) & 7;
						break;
					}
				}
				break;
			case 0xc0:
				#if 0
				if(__I8259_PC98_HACK) {
					pic[c].prio = (level + 1) & 7;
				} else {
					pic[c].prio = n & 7;
				}
				#else
				pic[c].prio = n & 7;
				#endif
				break;
			case 0xe0:
				#if 0
				if(__I8259_PC98_HACK) {
					pic[c].prio = (level + 1) & 7;
				}
				#endif
				if(pic[c].isr & mask) {
					pic[c].isr &= ~mask;
					pic[c].irr &= ~mask;
					pic[c].irr_tmp &= ~mask;
					#if 0
					if(!(__I8259_PC98_HACK)) {
						pic[c].prio = (pic[c].prio + 1) & 7;
					}
					#else
					pic[c].prio = (pic[c].prio + 1) & 7;
					#endif
				}
				break;
			}
		}
	}
__throughfall:
	update_intr();
}

uint32_t I8259::read_io8(uint32_t addr)
{
	int c = (addr >> 1) % num_chips;

	if(addr & 1) {
		return pic[c].imr;
	} else {
		#if 0
		if(__I8259_PC98_HACK) {
			if((pic[c].ocw3 & 1) == 0) {
				return pic[c].irr;
			} else {
				return pic[c].isr;
			}
		}
		#endif
		if(pic[c].ocw3 & 4) {
			// poling command
			if(pic[c].isr & ~pic[c].imr) {
				get_intr_ack();
			}
			for(int i = 0; i < 8; i++) {
				if((1 << i) & pic[c].irr & ~pic[c].imr) {
					return 0x80 | i;
				}
			}
		} else if((pic[c].ocw3 & 3) == 2) {
			return pic[c].irr;
		} else if((pic[c].ocw3 & 3) == 3) {
			return pic[c].isr;// & ~pic[c].imr;
		}
		return 0;
	}
}

void I8259::write_signal(int id, uint32_t data, uint32_t mask)
{
//	if((id & 0x0f) == 0x09) {
//		out_debug_log(_T("CDC INTR %02X"), data & mask);
//	}
	int c = (id >> 3) % num_chips;

	if(data & mask) {
		pic[c].irr |= 1 << (id & 7);
		update_intr();
	} else {
		pic[c].irr &= ~(1 << (id & 7));
		update_intr();
	}
}

void I8259::event_callback(int event_id, int err)
{
	int c = event_id % num_chips;
	uint8_t irr = pic[c].irr;

	pic[c].irr |= pic[c].irr_tmp;
	pic[c].irr_tmp = 0;
	pic[c].irr_tmp_id = -1;

	if(irr != pic[c].irr) {
		update_intr();
	}
}

uint32_t I8259::read_signal(int id)
{
	int c = (id >> 3) % num_chips;
	return (pic[c].irr & (1 << (id & 7))) ? 1 : 0;
}

void I8259::update_intr()
{
	bool intr = false;

	for(uint32_t c = 0; c < num_chips; c++) {
		uint8_t irr = pic[c].irr;
		if((c + 1) < num_chips) {
			// this is master
			if(pic[c + 1].irr & (~pic[c + 1].imr)) {
				// request from slave
				irr |= 1 << (pic[c + 1].icw3 & 7);
			}
		}
		irr &= (~pic[c].imr);
		if(!irr) {
			break;
		}
		if(!(pic[c].ocw3 & 0x20)) {
			irr |= pic[c].isr;
		}
		int level = pic[c].prio;
		uint8_t bit = 1 << level;
		while(!(irr & bit)) {
			level = (level + 1) & 7;
			bit = 1 << level;
		}
		if(((c + 1) < num_chips) && (pic[c].icw3 & bit)) {
			// check slave
			continue;
		}
		if(pic[c].isr & bit) {
			break;
		}

		// interrupt request
		req_chip = c;
		req_level = level;
		req_bit = bit;
		intr = true;
		break;
	}
	#if 0
	if(__I8259_PC98_HACK) {
		// Reset events porting from NP2
		if((req_chip == 0) && (req_level == 0)) {
			for(uint32_t c = 0; c < num_chips; c++) {
				if(pic[c].irr_tmp_id != -1) {
					cancel_event(this, pic[c].irr_tmp_id);
				}
				pic[c].irr_tmp_id = -1;
			}
		}
	}
	#endif
	if(d_cpu) {
		d_cpu->set_intr_line(intr, true, 0);
	}
}

uint32_t I8259::get_intr_ack()
{
	// ack (INTA=L)
	uint32_t vector;

	pic[req_chip].isr |= req_bit;
	pic[req_chip].irr &= ~req_bit;
	if(req_chip > 0) {
		// update isr and irr of master
		uint8_t slave = 1 << (pic[req_chip].icw3 & 7);
		pic[req_chip - 1].isr |= slave;
		pic[req_chip - 1].irr &= ~slave;
	}
	if(pic[req_chip].icw4 & 1) {
		// 8086 mode
		vector = (pic[req_chip].icw2 & 0xf8) | req_level;
	} else {
		// 8080 mode
		uint16_t addr = (uint16_t)pic[req_chip].icw2 << 8;
		if(pic[req_chip].icw1 & 4) {
			addr |= (pic[req_chip].icw1 & 0xe0) | (req_level << 2);
		} else {
			addr |= (pic[req_chip].icw1 & 0xc0) | (req_level << 3);
		}
		vector = 0xcd | (addr << 8);
	}
	if(pic[req_chip].icw4 & 2) {
		// auto eoi
		pic[req_chip].isr &= ~req_bit;
	}
	return vector;
}

#define STATE_VERSION	3

bool I8259::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	for(int i = 0; i < num_chips; i++) {
		state_fio->StateValue(pic[i].imr);
		state_fio->StateValue(pic[i].isr);
		state_fio->StateValue(pic[i].irr);
		state_fio->StateValue(pic[i].irr_tmp);
		state_fio->StateValue(pic[i].prio);
		state_fio->StateValue(pic[i].icw1);
		state_fio->StateValue(pic[i].icw2);
		state_fio->StateValue(pic[i].icw3);
		state_fio->StateValue(pic[i].icw4);
		state_fio->StateValue(pic[i].ocw3);
		state_fio->StateValue(pic[i].icw2_r);
		state_fio->StateValue(pic[i].icw3_r);
		state_fio->StateValue(pic[i].icw4_r);
		state_fio->StateValue(pic[i].irr_tmp_id);
	}
	state_fio->StateValue(req_chip);
	state_fio->StateValue(req_level);
	state_fio->StateValue(req_bit);
 	return true;
}
