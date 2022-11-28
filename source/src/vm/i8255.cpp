/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.06.01-

	[ i8255 ]
*/

#include "i8255.h"

// mode1 input
#define BIT_IBF_A	0x20	// PC5
#define BIT_STB_A	0x10	// PC4
#define BIT_STB_B	0x04	// PC2
#define BIT_IBF_B	0x02	// PC1

#define BIT_OBF_A	0x80	// PC7
#define BIT_ACK_A	0x40	// PC6
#define BIT_ACK_B	0x04	// PC2
#define BIT_OBF_B	0x02	// PC1

#define BIT_INTR_A	0x08	// PC3
#define BIT_INTR_B	0x01	// PC0

void I8255::reset()
{
	for(int i = 0; i < 3; i++) {
		port[i].rmask = 0xff;
		port[i].first = true;
		port[i].mode = 0;
	}
}

void I8255::write_io8(uint32_t addr, uint32_t data)
{
	int ch = addr & 3;
	
	switch(ch) {
	case 0:
	case 1:
	case 2:
		if(port[ch].wreg != data || port[ch].first) {
			write_signals(&port[ch].outputs, port[ch].wreg = data);
			port[ch].first = false;
		}
#ifndef I8255_AUTO_HAND_SHAKE
		if(ch == 0) {
			if(port[0].mode == 1 || port[0].mode == 2) {
				uint32_t val = port[2].wreg & ~BIT_OBF_A;
				if(port[2].wreg & BIT_ACK_A) {
					val &= ~BIT_INTR_A;
				}
				write_io8(2, val);
			}
		} else if(ch == 1) {
			if(port[1].mode == 1) {
				uint32_t val = port[2].wreg & ~BIT_OBF_B;
				if(port[2].wreg & BIT_ACK_B) {
					val &= ~BIT_INTR_B;
				}
				write_io8(2, val);
			}
		}
#endif
		break;
	case 3:
		if(data & 0x80) {
			port[0].mode = (data & 0x40) ? 2 : ((data >> 5) & 1);
			port[0].rmask = (port[0].mode == 2) ? 0xff : (data & 0x10) ? 0xff : 0;
			port[1].mode = (data >> 2) & 1;
			port[1].rmask = (data & 2) ? 0xff : 0;
			port[2].rmask = ((data & 8) ? 0xf0 : 0) | ((data & 1) ? 0xf : 0);
			// clear ports
			if(clear_ports_by_cmdreg) {
				write_io8(0, 0);
				write_io8(1, 0);
				write_io8(2, 0);
			}
			// setup control signals
			if(port[0].mode != 0 || port[1].mode != 0) {
				uint32_t val = port[2].wreg;
				if(port[0].mode == 1 || port[0].mode == 2) {
					val &= ~BIT_IBF_A;
					val |= BIT_OBF_A;
					val &= ~BIT_INTR_A;
				}
				if(port[1].mode == 1) {
					if(port[1].rmask == 0xff) {
						val &= ~BIT_IBF_B;
					} else {
						val |= BIT_OBF_B;
					}
					val &= ~BIT_INTR_B;
				}
				write_io8(2, val);
			}
		} else {
			uint32_t val = port[2].wreg;
			int bit = (data >> 1) & 7;
			if(data & 1) {
				val |= 1 << bit;
			} else {
				val &= ~(1 << bit);
			}
			write_io8(2, val);
		}
		break;
	}
}

uint32_t I8255::read_io8(uint32_t addr)
{
	int ch = addr & 3;
	
	switch(ch) {
	case 0:
	case 1:
	case 2:
		if(ch == 0) {
			if(port[0].mode == 1 || port[0].mode == 2) {
				uint32_t val = port[2].wreg & ~BIT_IBF_A;
				if(port[2].wreg & BIT_STB_A) {
					val &= ~BIT_INTR_A;
				}
				write_io8(2, val);
			}
		} else if(ch == 1) {
			if(port[1].mode == 1) {
				uint32_t val = port[2].wreg & ~BIT_IBF_B;
				if(port[2].wreg & BIT_STB_B) {
					val &= ~BIT_INTR_B;
				}
				write_io8(2, val);
			}
		}
		return (port[ch].rreg & port[ch].rmask) | (port[ch].wreg & ~port[ch].rmask);
	}
	return 0xff;
}

void I8255::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_I8255_PORT_A:
		port[0].rreg = (port[0].rreg & ~mask) | (data & mask);
#ifdef I8255_AUTO_HAND_SHAKE
		if(port[0].mode == 1 || port[0].mode == 2) {
			uint32_t val = port[2].wreg | BIT_IBF_A;
			if(port[2].wreg & BIT_STB_A) {
				val |= BIT_INTR_A;
			}
			write_io8(2, val);
		}
#endif
		break;
	case SIG_I8255_PORT_B:
		port[1].rreg = (port[1].rreg & ~mask) | (data & mask);
#ifdef I8255_AUTO_HAND_SHAKE
		if(port[1].mode == 1) {
			uint32_t val = port[2].wreg | BIT_IBF_B;
			if(port[2].wreg & BIT_STB_B) {
				val |= BIT_INTR_B;
			}
			write_io8(2, val);
		}
#endif
		break;
	case SIG_I8255_PORT_C:
#ifndef I8255_AUTO_HAND_SHAKE
		if(port[0].mode == 1 || port[0].mode == 2) {
			if(mask & BIT_STB_A) {
				if((port[2].rreg & BIT_STB_A) && !(data & BIT_STB_A)) {
					write_io8(2, port[2].wreg | BIT_IBF_A);
				} else if(!(port[2].rreg & BIT_STB_A) && (data & BIT_STB_A)) {
					if(port[2].wreg & BIT_STB_A) {
						write_io8(2, port[2].wreg | BIT_INTR_A);
					}
				}
			}
			if(mask & BIT_ACK_A) {
				if((port[2].rreg & BIT_ACK_A) && !(data & BIT_ACK_A)) {
					write_io8(2, port[2].wreg | BIT_OBF_A);
				} else if(!(port[2].rreg & BIT_ACK_A) && (data & BIT_ACK_A)) {
					if(port[2].wreg & BIT_ACK_A) {
						write_io8(2, port[2].wreg | BIT_INTR_A);
					}
				}
			}
		}
		if(port[1].mode == 1) {
			if(port[1].rmask == 0xff) {
				if(mask & BIT_STB_B) {
					if((port[2].rreg & BIT_STB_B) && !(data & BIT_STB_B)) {
						write_io8(2, port[2].wreg | BIT_IBF_B);
					} else if(!(port[2].rreg & BIT_STB_B) && (data & BIT_STB_B)) {
						if(port[2].wreg & BIT_STB_B) {
							write_io8(2, port[2].wreg | BIT_INTR_B);
						}
					}
				}
			} else {
				if(mask & BIT_ACK_B) {
					if((port[2].rreg & BIT_ACK_B) && !(data & BIT_ACK_B)) {
						write_io8(2, port[2].wreg | BIT_OBF_B);
					} else if(!(port[2].rreg & BIT_ACK_B) && (data & BIT_ACK_B)) {
						if(port[2].wreg & BIT_ACK_B) {
							write_io8(2, port[2].wreg | BIT_INTR_B);
						}
					}
				}
			}
		}
#endif
		port[2].rreg = (port[2].rreg & ~mask) | (data & mask);
		break;
	}
}

uint32_t I8255::read_signal(int id)
{
	switch(id) {
	case SIG_I8255_PORT_A:
		return port[0].wreg;
	case SIG_I8255_PORT_B:
		return port[1].wreg;
	case SIG_I8255_PORT_C:
		return port[2].wreg;
	}
	return 0;
}

#define STATE_VERSION	1

bool I8255::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < 3; i++) {
		state_fio->StateValue(port[i].wreg);
		state_fio->StateValue(port[i].rreg);
		state_fio->StateValue(port[i].rmask);
		state_fio->StateValue(port[i].mode);
		state_fio->StateValue(port[i].first);
	}
	return true;
}

