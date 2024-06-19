/*
	TOSHIBA T-200/250 Emulator 'eT-250'

	Author : Takeda.Toshiya
	Date   : 2023.12.29-

	[ memory bus ]
*/

#include "membus.h"
#include "../beep.h"
#include "../i8080.h"
#include "../i8279.h"

void MEMBUS::initialize()
{
	MEMORY::initialize();
	
	memset(ram, 0x00, sizeof(ram));
	memset(rom, 0xff, sizeof(rom));
	memset(vpg, 0x00, sizeof(vpg));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("BOOT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
#ifdef _T200
	if(fio->Fopen(create_local_path(_T("T200BOOT.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
#else
	if(fio->Fopen(create_local_path(_T("T250BOOT.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
#endif
	delete fio;
	
	set_memory_rw(0x0000, 0xffff, ram);
	
	mode_reg = data_reg = cmd_reg = fdd_reg = 0;
	key_repeat = key_ctrl = key_shift = key_irq = false;
	prn_ack = prn_busy = false;
	sio_txr = sio_rxr = sio_ci = sio_cts = false;
	stepcnt = false;
	sync_count = cblink = 0;
	
	register_frame_event(this);
}

void MEMBUS::reset()
{
	MEMORY::reset();
	
	mode_reg = 0;
	update_beep();
	update_rom_bank();
	update_vpg_bank();
}

void MEMBUS::write_io8(uint32_t addr, uint32_t data)
{
	uint8_t modified;
	
	switch(addr & 0xff) {
	case 0xc0:
		modified = mode_reg ^ data;
		mode_reg = data;
		if(modified & 0x10) {
			update_beep();
		}
		if(modified & 0x08) {
			update_rom_bank();
		}
		if(modified & 0x04) {
			update_vpg_bank();
		}
		break;
	case 0xc1:
		data_reg = data;
		d_prn->write_signal(SIG_PRINTER_DATA, data, 0xff);
		break;
	case 0xc2:
		modified = cmd_reg ^ data;
		cmd_reg = data;
		if(modified & 0x08) {
			stepcnt = ((data & 0x08) != 0);
			update_irq();
		}
		if((modified & 0x02) && (data & 0x02)) {
			d_prn->write_signal(SIG_PRINTER_STROBE, 1, 1);
			d_prn->write_signal(SIG_PRINTER_STROBE, 0, 1);
		}
		if((modified & 0x01) && (data & 0x01)) {
			d_prn->write_signal(SIG_PRINTER_RESET, 1, 1);
			d_prn->write_signal(SIG_PRINTER_RESET, 0, 1);
		}
		break;
	case 0xc3:
		prn_ack = false;
		update_irq();
		break;
	case 0xf0:
		modified = fdd_reg ^ data;
		fdd_reg = data;
		if((modified & 0x80) && (data & 0x80)) {
			d_fdc->reset();
		}
#ifdef _T200
		d_fdc->write_signal(SIG_UPD765A_MOTOR, data, 0x40);
#endif
		break;
	}
}

uint32_t MEMBUS::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	
	switch(addr & 0xff) {
	case 0xd0:
		if(key_irq) val ^= 0x80;
		if(sio_txr) val ^= 0x40;
		if(sio_rxr) val ^= 0x20;
		if(prn_ack) val ^= 0x10;
		if(stepcnt) val ^= 0x01;
		return val;
	case 0xd1:
		if(key_repeat) val ^= 0x80;
		key_repeat = false;
		if(key_ctrl) val ^= 0x20;
		return val;
	case 0xd2:
		return config.dipswitch & 0xff;
	case 0xd3:
		if(sio_ci) val ^= 0x80;
		if(sio_cts) val ^= 0x40;
		if(!prn_busy) val ^= 0x08;
		return val;
	case 0xd4:
		return data_reg;
	}
	return 0xff;
}

void MEMBUS::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMBUS_KEY_SEL) {
		uint32_t val = 0xff;
/* key matrix ???

	  
  
7
8945
6-123
0.   
!"#$%
QWERT
ASDFG
ZXCVB
&'() 
YUIOP
HJKL+
NM<>?
_\[]*
*/
		d_kbc->write_signal(SIG_I8279_RL, val, 0xff);
	} else if(id == SIG_MEMBUS_KEY_IRQ) {
		key_irq = ((data & mask) != 0);
		update_irq();
	} else if(id == SIG_MEMBUS_PRN_ACK) {
		prn_ack = ((data & mask) == 0);
		update_irq();
	} else if(id == SIG_MEMBUS_SIO_TXR) {
		sio_txr = ((data & mask) != 0);
		update_irq();
	} else if(id == SIG_MEMBUS_SIO_RXR) {
		sio_rxr = ((data & mask) != 0);
		update_irq();
	} else if(id == SIG_MEMBUS_SIO_CI) {
		sio_ci = ((data & mask) != 0);
	} else if(id == SIG_MEMBUS_SIO_CTS) {
		sio_cts = ((data & mask) != 0);
	}
}

void MEMBUS::key_down(int code)
{
	if(code == 0x10) {
		key_shift = true;
		d_kbc->write_signal(SIG_I8279_SHIFT, 1, 1);
	}
	if(code == 0x11) {
		key_ctrl = true;
		d_kbc->write_signal(SIG_I8279_CTRL, 1, 1);
	}
}

void MEMBUS::key_up(int code)
{
	if(code == 0x10) {
		key_shift = false;
		d_kbc->write_signal(SIG_I8279_SHIFT, 0, 1);
	}
	if(code == 0x11) {
		key_ctrl = false;
		d_kbc->write_signal(SIG_I8279_CTRL, 0, 1);
	}
}

void MEMBUS::update_irq()
{
	if(key_irq || prn_ack || sio_txr || sio_rxr || stepcnt) {
		d_cpu->write_signal(SIG_I8085_RST6, 1, 1);
	} else {
		d_cpu->write_signal(SIG_I8085_RST6, 0, 1);
	}
}

void MEMBUS::update_beep()
{
	d_beep->write_signal(SIG_BEEP_ON, mode_reg, 0x10);
}

void MEMBUS::update_rom_bank()
{
	if(mode_reg & 8) {
		set_memory_rw (0x0000, 0x0fff, ram);
	} else {
		set_memory_r  (0x0000, 0x0fff, rom);
		unset_memory_w(0x0000, 0x0fff);
	}
}

void MEMBUS::update_vpg_bank()
{
	if(mode_reg & 4) {
		set_memory_rw(0xf800, 0xffff, vpg);
	} else {
		set_memory_rw(0xf800, 0xffff, ram + 0xf800);
	}
}

void MEMBUS::event_frame()
{
	if(++sync_count == 3) {
		sync_count = 0;
		d_cpu->write_signal(SIG_I8085_RST7, 1, 1);
	}
	cblink = (cblink + 1) & 0x1f;
}

void MEMBUS::draw_screen()
{
	memset(screen, 0, sizeof(screen));
	
	if(mode_reg & 0x29) {
		uint16_t src = ((regs[12] << 8) | regs[13]) & 0x7ff;
		uint16_t cursor = ((regs[14] << 8) | regs[15]) & 0x7ff;
		int hz = (regs[1] <= 80) ? regs[1] : 80;
		int vt = (regs[6] <= 25) ? regs[6] : 25;
		int ht = ((regs[9] <= 9) ? regs[9] : 9) + 1;
		uint8_t bp = regs[10] & 0x60;
		scrntype_t col = RGB_COLOR(0, 255, 0);
		
		for(int y = 0; y < vt; y++) {
			for(int x = 0; x < hz; x++) {
				uint8_t code = ram[0xf800 | src];
				code = ((code & 0x7f) << 1) | (code >> 7);
				
				// draw pattern
				for(int l = 0; l < 8; l++) {
					uint8_t pat = vpg[(code << 3) + l];
					int yy = y * ht + l;
					if(yy >= 200) {
						break;
					}
					scrntype_t* d = &screen[yy][x << 3];
					
					if(pat & 0x80) d[0] = col;
					if(pat & 0x40) d[1] = col;
					if(pat & 0x20) d[2] = col;
					if(pat & 0x10) d[3] = col;
					if(pat & 0x08) d[4] = col;
					if(pat & 0x04) d[5] = col;
					if(pat & 0x02) d[6] = col;
					if(pat & 0x01) d[7] = col;
				}
				// draw cursor
				if(src == cursor) {
					int s = regs[10] & 0x1f;
					int e = regs[11] & 0x1f;
					if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
						for(int l = s; l <= e && l < ht; l++) {
							int yy = y * ht + l;
							if(yy >= 200) {
								break;
							}
							scrntype_t* d = &screen[yy][x << 3];
							d[0] = d[1] = d[2] = d[3] = d[4] = d[5] = d[6] = d[7] = col;
						}
					}
				}
				src = (src + 1) & 0x7ff;
			}
		}
	} else {
		memset(screen, 0, sizeof(screen));
	}
	
	// copy to real screen
	emu->set_vm_screen_lines(200);
	
	for(int y = 0; y < 200; y++) {
		scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0);
		scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1);
		scrntype_t* src = screen[y];
		
		if(config.scan_line) {
			memset(dest1, 0, 640 * sizeof(scrntype_t));
		} else {
			my_memcpy(dest1, src, 640 * sizeof(scrntype_t));
		}
		my_memcpy(dest0, src, 640 * sizeof(scrntype_t));
	}
	emu->screen_skip_line(true);
}

#define STATE_VERSION	1

bool MEMBUS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(!MEMORY::process_state(state_fio, loading)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(vpg, sizeof(vpg), 1);
	state_fio->StateValue(mode_reg);
	state_fio->StateValue(data_reg);
	state_fio->StateValue(cmd_reg);
	state_fio->StateValue(fdd_reg);
	state_fio->StateValue(key_repeat);
	state_fio->StateValue(key_ctrl);
	state_fio->StateValue(key_shift);
	state_fio->StateValue(key_irq);
	state_fio->StateValue(prn_ack);
	state_fio->StateValue(prn_busy);
	state_fio->StateValue(sio_txr);
	state_fio->StateValue(sio_rxr);
	state_fio->StateValue(sio_ci);
	state_fio->StateValue(sio_cts);
	state_fio->StateValue(stepcnt);
	state_fio->StateValue(cblink);
	state_fio->StateValue(sync_count);
	
	// post process
	if(loading) {
		update_irq();
		update_beep();
		update_rom_bank();
		update_vpg_bank();
	}
	return true;
}

