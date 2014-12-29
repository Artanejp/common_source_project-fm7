/*
	EPSON HC-80 Emulator 'eHC-80'

	Author : Takeda.Toshiya
	Date   : 2008.03.14 -

	[ i/o ]
*/

#include "io.h"
#include "../beep.h"
#include "../tf20.h"
#include "../../fifo.h"
#include "../../fileio.h"

//#define OUT_CMD_LOG

// interrupt bits
#define BIT_7508	0x01
#define BIT_8251	0x02
#define BIT_CD		0x04
#define BIT_ICF		0x08
#define BIT_OVF		0x10
#define BIT_EXT		0x20

// 6303
#define BIT_OBF		0x01
#define BIT_IBF		0x02
#define BIT_F1		0x08
#define RCD00		0
#define RCD01		1
#define RCD02		2
#define RCD03		3
#define RCD04		11
#define RCD05		12
#define RCD06		13
#define RCD07		41
#define RCD08		42
#define RCD09		43
#define RCD10		44
#define RCD11		45
#define RCD11_1		46
#define RCD12		61
#define RCD13		62
#define RCD14		63
#define RCD15		71

// TF-20
#define DID_FIRST	0x31
#define DS_SEL		0x05
#define SOH		0x01
#define STX		0x02
#define EOT		0x04
#define ACK		0x06

// intelligent ram disk
#define IRAMDISK_WAIT	1
#define IRAMDISK_IN	0
#define IRAMDISK_OUT	1

#define EVENT_FRC	0
#define EVENT_1SEC	1
#define EVENT_6303	2

static const int key_tbl[256] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x56,0x57,0xff,0xff,0xff,0x71,0xff,0xff,
	0xb3,0xb2,0xff,0x10,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,
	0x73,0xff,0xff,0xff,0xff,0x63,0x55,0x65,0x64,0xff,0xff,0xff,0xff,0x80,0x81,0xff,
	0x52,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x50,0x51,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0x66,0x40,0x76,0x30,0x22,0x31,0x32,0x33,0x27,0x34,0x35,0x36,0x42,0x41,0x60,
	0x61,0x20,0x23,0x67,0x24,0x26,0x77,0x21,0x75,0x25,0x74,0xff,0xff,0xff,0xff,0xff,
	0x52,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x50,0x51,0xff,0xff,0xff,0xff,0xff,0xff,
	0x03,0x04,0x05,0x06,0x07,0xff,0xff,0xff,0xff,0xff,0x01,0x02,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x70,0x37,0x43,0x53,0x44,0x45,
	0x62,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x46,0x72,0x47,0x54,0xff,
	0xff,0xff,0x72,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

static const uint8 dot_tbl[8] = {
	0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1
};

void IO::initialize()
{
	// config
	device_type = config.device_type;
	
	// init ram and external ram disk
	memset(ram, 0, sizeof(ram));
	memset(ext, 0, 0x20000);
	memset(ext + 0x20000, 0xff, 0x20000);
	extar = 0;
	extcr = 0;
	
	// load images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(basic, 0x4000, 1);
		memcpy(basic + 0x4000, basic, 0x4000);
		fio->Fread(basic + 0x4000, 0x4000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("UTIL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(util, 0x4000, 1);
		memcpy(util + 0x4000, util, 0x4000);
		fio->Fread(util + 0x4000, 0x4000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("VRAM.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(ram + 0x8000, 0x1800, 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("EXTRAM.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(ext, 0x20000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("INTRAM.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(iramdisk_sectors, sizeof(iramdisk_sectors), 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("EXT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ext + 0x20000, 0x20000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	// init sub cpu
	cmd6303_buf = new FIFO(1024);
	rsp6303_buf = new FIFO(1024);
	tf20_buf = new FIFO(1024);
	
	cmd7508_buf = new FIFO(16);
	rsp7508_buf = new FIFO(16);
	key_buf = new FIFO(7);
	
	// set pallete
	pd = RGB_COLOR(48, 56, 16);
	pb = RGB_COLOR(160, 168, 160);
	
	// init 7508
	emu->get_host_time(&cur_time);
	onesec_intr = alarm_intr = false;
	onesec_intr_enb = alarm_intr_enb = kb_intr_enb = true;
	res_7508 = kb_caps = false;
	
	// register events
	register_frame_event(this);
	register_event_by_clock(this, EVENT_FRC, 0x40000, true, NULL);
	register_event_by_clock(this, EVENT_1SEC, CPU_CLOCKS, true, &register_id);
	register_event_by_clock(this, EVENT_6303, 100, true, NULL);
}

void IO::release()
{
	// save external ram disk
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("VRAM.BIN")), FILEIO_WRITE_BINARY)) {
		fio->Fwrite(ram + 0x8000, 0x1800, 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("EXTRAM.BIN")), FILEIO_WRITE_BINARY)) {
		fio->Fwrite(ext, 0x20000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("INTRAM.BIN")), FILEIO_WRITE_BINARY)) {
		fio->Fwrite(iramdisk_sectors, sizeof(iramdisk_sectors), 1);
		fio->Fclose();
	}
	delete fio;
	
	cmd6303_buf->release();
	delete cmd6303_buf;
	rsp6303_buf->release();
	delete rsp6303_buf;
	tf20_buf->release();
	delete tf20_buf;
	
	cmd7508_buf->release();
	delete cmd7508_buf;
	rsp7508_buf->release();
	delete rsp7508_buf;
	key_buf->release();
	delete key_buf;
}

void IO::reset()
{
	// reset gapnit
	bcr = slbcr = isr = ier = ioctlr = 0;
	icrc = icrb = 0;
	ear = beep = false;
	res_z80 = true;
	key_buf->clear();
	
	// reset 6303
	psr = 0;
	cs_addr = 0x8100;
	gs_addr = 0x9500;
	lcd_on = true;
	scr_mode = 1;
	num_lines = 0;
	flash_block = 0;
	cs_blocks = gs_blocks = 0;
	memset(cs_block, 0, sizeof(cs_block));
	memset(gs_block, 0, sizeof(gs_block));
	memset(udgc, 0, sizeof(udgc));
	wnd_ptr_x = wnd_ptr_y = 0;
	blink = 0;
	
	// reset intelligent ram disk
	iramdisk_count = 0;
	iramdisk_ptr = iramdisk_buf;
}

void IO::sysreset()
{
	// reset 7508
	onesec_intr = alarm_intr = false;
	onesec_intr_enb = alarm_intr_enb = kb_intr_enb = true;
	res_7508 = true;
}

void IO::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_IO_RXRDY) {
		// notify rxrdy is changed from i8251
		if(data & mask) {
			isr |= BIT_8251;
		} else {
			isr &= ~BIT_8251;
		}
		update_intr();
	} else if(id == SIG_IO_BARCODE) {
		// signal from barcode reader
		if(!slbcr) {
			bool next = ((data & mask) != 0);
			if((bcr == 2 && ear && !next) || (bcr == 4 && !ear && next) || (bcr == 6 && ear != next)) {
				icrb = passed_clock(cur_clock) / 4;
				isr |= BIT_ICF;
				update_intr();
			}
			ear = next;
		}
	} else if(id == SIG_IO_TF20) {
		// recv from tf20
		tf20_buf->write(data);
	}
}

void IO::event_frame()
{
	d_beep->write_signal(SIG_BEEP_ON, beep ? 1 : 0, 1);
	beep = false;
	blink++;
}

void IO::event_callback(int event_id, int err)
{
	if(event_id == EVENT_FRC) {
		// FRC overflow event
		cur_clock = current_clock();
		isr |= BIT_OVF;
		update_intr();
	} else if(event_id == EVENT_1SEC) {
		// update rtc
		if(cur_time.initialized) {
			cur_time.increment();
		} else {
			emu->get_host_time(&cur_time);	// resync
			cur_time.initialized = true;
		}
		onesec_intr = true;
		if(onesec_intr_enb) {
			isr |= BIT_7508;
			update_intr();
		}
	} else if(event_id == EVENT_6303) {
		// communicate between z80 and 6303
		if(psr & BIT_OBF) {
			psr &= ~BIT_OBF;
			process_6303();
		}
		if(!rsp6303_buf->empty()) {
			psr |= BIT_IBF;
		}
	}
}

void IO::write_io8(uint32 addr, uint32 data)
{
	//emu->out_debug_log(_T("OUT %2x,%2x\n"), addr & 0xff, data);
	switch(addr & 0xff) {
	case 0x00:
		// CTLR1
		bcr = data & 6;
		d_mem->write_signal(0, data, 1);
		break;
	case 0x01:
		// CMDR
		if(data & 4) {
			isr &= ~BIT_OVF;
			update_intr();
		}
		//if(data & 2) {
		//	rdysio = false;
		//}
		//if(data & 1) {
		//	rdysio = true;
		//}
		break;
	case 0x02:
		// CTLR2
		break;
	case 0x04:
		// IER
		ier = data;
		break;
	case 0x06:
		// SIOR
		send_to_7508(data);
		break;
	case 0x0c:
		// 8251 data write
		d_sio->write_io8(0, data);
		break;
	case 0x0d:
		// 8251 command write
		d_sio->write_io8(1, data);
		break;
	case 0x0e:
		// 6303 send data
		cmd6303_buf->write(data);
		psr |= BIT_OBF;
#ifdef OUT_CMD_LOG
		emu->out_debug_log(_T("%4x\tDAT %2x\n"), get_cpu_pc(0), data);
#endif
		break;
	case 0x0f:
		// 6303 send command
		cmd6303 = data;
		psr |= BIT_OBF;
#ifdef OUT_CMD_LOG
		emu->out_debug_log(_T("\n%4x\tCMD %2x\n"), vm->get_cpu_pc(), data);
#endif
		break;
	case 0x80:
		if(device_type == 1) {
			iramdisk_write_data(data);
		}
		break;
	case 0x81:
		if(device_type == 1) {
			iramdisk_write_cmd(data);
		}
		break;
	case 0x90:
		// EXTAR
		if(device_type == 2) {
			extar = (extar & 0xffff00) | data;
		}
		break;
	case 0x91:
		// EXTAR
		if(device_type == 2) {
			extar = (extar & 0xff00ff) | (data << 8);
		}
		break;
	case 0x92:
		// EXTAR
		if(device_type == 2) {
			extar = (extar & 0x00ffff) | ((data & 7) << 16);
		}
		break;
	case 0x93:
		// EXTOR
		if(device_type == 2) {
			if(extar < 0x20000) {
				ext[extar] = data;
			}
			extar = (extar & 0xffff00) | ((extar + 1) & 0xff);
		}
		break;
	case 0x94:
		// EXTCR
		if(device_type == 2) {
			extcr = data;
		}
		break;
	}
}

uint32 IO::read_io8(uint32 addr)
{
	uint32 val = 0xff;
//	emu->out_debug_log(_T("IN %2x\n"), addr & 0xff);
	
	switch(addr & 0xff) {
	case 0x00:
		// ICRL.C (latch FRC value)
		icrc = passed_clock(cur_clock) / 4;
		return icrc & 0xff;
	case 0x01:
		// ICRH.C
		return (icrc >> 8) & 0xff;
	case 0x02:
		// ICRL.B
		return icrb & 0xff;
	case 0x03:
		// ICRH.B
		isr &= ~BIT_ICF;
		update_intr();
		return (icrb >> 8) & 0xff;
	case 0x04:
		// ISR
		return isr;
	case 0x05:
		// STR
		return 8 | 4 | (ear ? 1 : 0);	// always rdysio=rdy=true
	case 0x06:
		// SIOR
		return rec_from_7508();
	case 0x0c:
		// 8251 data read
		isr &= ~BIT_8251;
		update_intr();
		return d_sio->read_io8(0);
	case 0x0d:
		// 8251 status read
		return d_sio->read_io8(1);
	case 0x0e:
		// 6303 status
		return psr;
	case 0x0f:
		// 6303 recv data
		val = rsp6303_buf->read();
		psr &= ~BIT_IBF;
		if(!rsp6303_buf->empty()) {
			psr &= ~BIT_F1;
		}
#ifdef OUT_CMD_LOG
		emu->out_debug_log(_T("%4x\tRCV %2x\n"), vm->get_cpu_pc(), val);
#endif
		return val;
	case 0x80:
		if(device_type == 1) {
			return iramdisk_read_data();
		}
		return 0xff;
	case 0x81:
		if(device_type == 1) {
			return iramdisk_read_stat();
		}
		return 0xff;
	case 0x93:
		// EXTIR
		if(device_type == 2) {
			if(extar < 0x40000) {
				val = ext[extar];
			}
			extar = (extar & 0xffff00) | ((extar + 1) & 0xff);
			return val;
		}
		return 0xff;
	case 0x94:
		// EXTSR
		if(device_type == 2) {
			return extcr & ~0x80;
		}
		return 0xff;
	}
	return 0xff;
}

uint32 IO::intr_ack()
{
	if(isr & BIT_7508) {
		isr &= ~BIT_7508;
		return 0xf0;
	} else if(isr & BIT_8251) {
		return 0xf2;
	} else if(isr & BIT_CD) {
		return 0xf4;
	} else if(isr & BIT_ICF) {
		return 0xf6;
	} else if(isr & BIT_OVF) {
		return 0xf8;
	} else if(isr & BIT_EXT) {
		return 0xfa;
	}
	// unknown
	return 0xff;
}

void IO::update_intr()
{
	// set int signal
	bool next = ((isr & ier & 0x3f) != 0);
	d_cpu->set_intr_line(next, true, 0);
}

// ----------------------------------------------------------------------------
// 7508
// ----------------------------------------------------------------------------

void IO::send_to_7508(uint8 val)
{
	int res;
	
	// process command
	cmd7508_buf->write(val);
	uint8 cmd = cmd7508_buf->read_not_remove(0);
	
	switch(cmd) {
	case 0x01:
		// power off
		cmd7508_buf->read();
		break;
	case 0x02:
		// status / key
		cmd7508_buf->read();
		if((onesec_intr && onesec_intr_enb) || (alarm_intr && alarm_intr_enb) || res_z80 || res_7508) {
			res = 0xc1;
			res |= (onesec_intr && onesec_intr_enb) ? 0x20 : 0;
			res |= (res_z80 ? 0x10 : 0) | (res_7508 ? 8 : 0);
			res |= (alarm_intr && alarm_intr_enb) ? 2 : 0;
			// clear interrupt
			onesec_intr = alarm_intr = res_z80 = res_7508 = false;
		} else if(key_buf->count()) {
			res = key_buf->read();
		} else {
			res = 0xbf;
		}
		rsp7508_buf->write(res);
		// request next interrupt
		if(key_buf->count() && kb_intr_enb) {
			isr |= BIT_7508;
			update_intr();
		}
		break;
	case 0x03:
		// kb reset
		cmd7508_buf->read();
		key_buf->clear();
		kb_rep_spd1 = 42 | 0x80;
		kb_rep_spd2 = 18 | 0x80;
		kb_intr_enb = true;
		break;
	case 0x04:
		// kb repeat timer 1 set
		if(cmd7508_buf->count() == 2) {
			cmd7508_buf->read();
			kb_rep_spd1 = cmd7508_buf->read();
		}
		break;
	case 0x05:
		// kb repeat off
		cmd7508_buf->read();
		kb_rep_enb = false;
		break;
	case 0x06:
		// kb interrupt off
		cmd7508_buf->read();
		kb_intr_enb = false;
		break;
	case 0x07:
		// clock read
		cmd7508_buf->read();
		rsp7508_buf->write(TO_BCD_HI(cur_time.year));
		rsp7508_buf->write(TO_BCD_LO(cur_time.year));
		rsp7508_buf->write(TO_BCD(cur_time.month));
		rsp7508_buf->write(TO_BCD(cur_time.day));
		rsp7508_buf->write(TO_BCD(cur_time.hour));
		rsp7508_buf->write(TO_BCD(cur_time.minute));
		rsp7508_buf->write(TO_BCD(cur_time.second));
		rsp7508_buf->write(cur_time.day_of_week);
		break;
	case 0x08:
		// power switch read
		cmd7508_buf->read();
		rsp7508_buf->write(1);
		break;
	case 0x09:
		// alarm read
		cmd7508_buf->read();
		rsp7508_buf->write(alarm[0]);
		rsp7508_buf->write(alarm[1]);
		rsp7508_buf->write(alarm[2]);
		rsp7508_buf->write(alarm[3]);
		rsp7508_buf->write(alarm[4]);
		rsp7508_buf->write(alarm[5]);
		break;
	case 0x0a:
		// dip switch read
		cmd7508_buf->read();
		res = 0xf;	// ascii keyboard
		rsp7508_buf->write(res);
		break;
	case 0x0b:
		// set power failure detect voltage
		if(cmd7508_buf->count() == 2) {
			cmd7508_buf->read();
			cmd7508_buf->read();
		}
		break;
	case 0x0c:
		// read buttery voltage
		cmd7508_buf->read();
		rsp7508_buf->write(0xe0);
		break;
	case 0x0d:
		// 1 sec interrupt off
		cmd7508_buf->read();
		onesec_intr_enb = false;
		break;
	case 0x0e:
		// kb clear
		cmd7508_buf->read();
		key_buf->clear();
		break;
	case 0x0f:
		// system reset
		cmd7508_buf->read();
		res_7508 = true;
		break;
	case 0x14:
		// kb repeat timer 2 set
		if(cmd7508_buf->count() == 2) {
			cmd7508_buf->read();
			kb_rep_spd2 = cmd7508_buf->read();
		}
		break;
	case 0x15:
		// kb repeat on
		cmd7508_buf->read();
		kb_rep_enb = true;
		break;
	case 0x16:
		// kb interrupt on
		cmd7508_buf->read();
		kb_intr_enb = true;
		break;
	case 0x17:
		// clock write
		if(cmd7508_buf->count() == 9) {
			cmd7508_buf->read();
			int year10 = cmd7508_buf->read();
			int year1 = cmd7508_buf->read();
			int month = cmd7508_buf->read();
			int day = cmd7508_buf->read();
			int hour = cmd7508_buf->read();
			int minute = cmd7508_buf->read();
			int second = cmd7508_buf->read();
			int day_of_week = cmd7508_buf->read();
			
			if((month & 0x0f) == 0 || (day & 0x0f) == 0) {
				// invalid date
				emu->get_host_time(&cur_time);
			} else {
				bool changed = false;
				if((year10 & 0x0f) != 0x0f && (year1 & 0x0f) != 0x0f) {
					cur_time.year = (year10 & 0x0f) * 10 + (year1 & 0x0f);
					cur_time.update_year();
					changed = true;
				}
				if((month & 0x0f) != 0x0f) {
					cur_time.month = FROM_BCD(month & 0x1f);
					changed = true;
				}
				if((day & 0x0f) != 0x0f) {
					cur_time.day = FROM_BCD(day & 0x3f);
					changed = true;
				}
				if((hour & 0x0f) != 0x0f) {
					cur_time.hour = FROM_BCD(hour & 0x3f);
					changed = true;
				}
				if((minute & 0x0f) != 0x0f) {
					cur_time.minute = FROM_BCD(minute & 0x7f);
					changed = true;
				}
				if((second & 0x0f) != 0x0f) {
					cur_time.second = FROM_BCD(second & 0x7f);
					changed = true;
				}
//				if((day_of_week & 0x0f) != 0x0f) {
//					cur_time.day_of_week = day_of_week & 0x07;
//					changed = true;
//				}
				if(changed) {
					cur_time.update_day_of_week();
					// restart event
					cancel_event(this, register_id);
					register_event_by_clock(this, EVENT_1SEC, CPU_CLOCKS, true, &register_id);
				}
			}
		}
		break;
	case 0x19:
		// alarm set
		if(cmd7508_buf->count() == 7) {
			cmd7508_buf->read();
			alarm[0] = cmd7508_buf->read();
			alarm[1] = cmd7508_buf->read();
			alarm[2] = cmd7508_buf->read();
			alarm[3] = cmd7508_buf->read();
			alarm[4] = cmd7508_buf->read();
			alarm[5] = cmd7508_buf->read();
		}
		break;
	case 0x1b:
		// set full charge voltage
		if(cmd7508_buf->count() == 2) {
			cmd7508_buf->read();
			cmd7508_buf->read();
		}
		break;
	case 0x1c:
		// read temperature
		cmd7508_buf->read();
		rsp7508_buf->write(0x90);
		break;
	case 0x1d:
		// 1 sec interrupt on
		cmd7508_buf->read();
		onesec_intr_enb = true;
		break;
	case 0x24:
		// kb repeat timer 1 read
		cmd7508_buf->read();
		rsp7508_buf->write(kb_rep_spd1);
		break;
	case 0x29:
		// alarm off
		cmd7508_buf->read();
		alarm_intr_enb = false;
		break;
	case 0x2c:
		// read analog jack 1
		cmd7508_buf->read();
		rsp7508_buf->write(0);
		break;
	case 0x34:
		// kb repeat timer 2 read
		cmd7508_buf->read();
		rsp7508_buf->write(kb_rep_spd2);
		break;
	case 0x39:
		// alarm on
		cmd7508_buf->read();
		alarm_intr_enb = true;
		break;
	case 0x3c:
		// read analog jack 2
		cmd7508_buf->read();
		rsp7508_buf->write(0);
		break;
	default:
		// unknown cmd
		cmd7508_buf->read();
		emu->out_debug_log(_T("unknown cmd %2x\n"), cmd);
	}
}

uint8 IO::rec_from_7508()
{
	return rsp7508_buf->read();
}

void IO::key_down(int code)
{
	if(code == 0x14) {
		// toggle caps lock
		kb_caps = !kb_caps;
		update_key(kb_caps ? 0xb4 : 0xa4);
		update_key(kb_caps ? 0xa4 : 0xb4);
	} else {
		update_key(key_tbl[code & 0xff]);
	}
}

void IO::key_up(int code)
{
	if(code == 0x10) {
		update_key(0xa3);	// break shift
	} else if(code == 0x11) {
		update_key(0xa2);	// break ctrl
	}
}

void IO::update_key(int code)
{
	if(code != 0xff) {
		// add to buffer
		if(code == 0x10) {
			// stop key
			key_buf->clear();
			key_buf->write(code);
		} else {
			key_buf->write(code);
		}
		
		// key interrupt
		if(kb_intr_enb || (!kb_intr_enb && code == 0x10)) {
			isr |= BIT_7508;
			update_intr();
		}
	}
}

// ----------------------------------------------------------------------------
// 6303
// ----------------------------------------------------------------------------

void IO::process_6303()
{
	switch(cmd6303) {
	case 0x00:
		// read data
		if(cmd6303_buf->count() == 2) {
			uint16 addr = cmd6303_buf->read() << 8;
			addr |= cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			rsp6303_buf->write(ram[addr]);
			psr |= BIT_F1;
		}
		break;
	case 0x01:
		// write data
		if(cmd6303_buf->count() == 4) {
			uint16 addr = cmd6303_buf->read() << 8;
			addr |= cmd6303_buf->read();
			uint8 val = cmd6303_buf->read();
			uint8 ope = cmd6303_buf->read();
			if(ope == 1) {
				ram[addr] &= val;
			} else if(ope == 2) {
				ram[addr] |= val;
			} else if(ope == 3) {
				ram[addr] ^= val;
			} else {
				ram[addr] = val;
			}
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x02:
		// execute routine
		if(cmd6303_buf->count() == 2) {
			uint16 addr = cmd6303_buf->read() << 8;
			addr |= cmd6303_buf->read();
			// unknown
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x0b:
		// unknown (initialize???)
		rsp6303_buf->write(RCD00);
		psr |= BIT_F1;
		break;
	case 0x10:
		// define screen mode
		if(cmd6303_buf->count() == 16) {
			cs_addr = cmd6303_buf->read() << 8;
			cs_addr |= cmd6303_buf->read();
			gs_addr = cmd6303_buf->read() << 8;
			gs_addr |= cmd6303_buf->read();
			cmd6303_buf->read();
			cmd6303_buf->read();
			cmd6303_buf->read();
			cmd6303_buf->read();
			cmd6303_buf->read();
			cmd6303_buf->read();
			cmd6303_buf->read();
			cmd6303_buf->read();
			uint16 bottom = cmd6303_buf->read() << 8;
			bottom |= cmd6303_buf->read();
			cmd6303_buf->read();
			cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
			// go to character screen mode ???
			scr_mode = 0xff;
			// stop block flashing ???
			flash_block = 0;
			cs_blocks = gs_blocks = 0;
			// clear screen ???
			memset(&ram[cs_addr], 0, bottom - cs_addr);
		}
		break;
	case 0x11:
		// turn on/off lcd
		if(cmd6303_buf->count() == 1) {
			lcd_on = cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x12:
		// select screen
		if(cmd6303_buf->count() == 1) {
			scr_mode = cmd6303_buf->read();
			if(!scr_mode) {
				scr_ptr = cs_addr;
			}
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x13:
		// read screen pointer
		rsp6303_buf->write(RCD00);
		rsp6303_buf->write(scr_ptr >> 8);
		rsp6303_buf->write(scr_ptr & 0xff);
		psr |= BIT_F1;
		break;
	case 0x14:
		// set screen pointer
		if(cmd6303_buf->count() == 2) {
			scr_ptr = cmd6303_buf->read() << 8;
			scr_ptr |= cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
			// stop block flashing ???
			flash_block = 0;
			cs_blocks = gs_blocks = 0;
		}
		break;
	case 0x15:
		// define number of lines
		if(cmd6303_buf->count() == 1) {
			num_lines = cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x16:
		// define cursor mode
		if(cmd6303_buf->count() == 1) {
			curs_mode = cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x17:
		// read cursur position
		rsp6303_buf->write(RCD00);
		rsp6303_buf->write(curs_x);
		rsp6303_buf->write(curs_y);
		psr |= BIT_F1;
		break;
	case 0x18:
		// set cursor position
		if(cmd6303_buf->count() == 2) {
			curs_x = cmd6303_buf->read();
			curs_y = cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x19:
		// start/stop control block flashing
		if(cmd6303_buf->count() == 1) {
			flash_block = cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x1a:
		// clear screen
		if(cmd6303_buf->count() == 4) {
			uint8 scr = cmd6303_buf->read();
			uint8 code = cmd6303_buf->read();
			int sy = cmd6303_buf->read();
			int num = cmd6303_buf->read();
			if(scr) {
				// char screen
				for(int y = 0; y < num; y++) {
					if(sy + y < 64) {
						memset(&ram[cs_addr + (sy + y) * 80], code, 80);
					}
				}
			} else {
				// graph screen
				for(int y = 0; y < num; y++) {
					if(sy + y < 8) {
						memset(&ram[gs_addr + (sy + y) * 60 * 8], code, 60 * 8);
					}
				}
			}
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x1b:
		// read character font
		if(cmd6303_buf->count() == 1) {
			int ofs = cmd6303_buf->read() << 3;
			rsp6303_buf->write(RCD00);
			for(int i = 0; i < 8; i++) {
				rsp6303_buf->write(font[ofs + i]);
			}
			psr |= BIT_F1;
		}
		break;
	case 0x20:
		// define user defined graphic character
		if(cmd6303_buf->count() >= 3) {
			int lx = cmd6303_buf->read_not_remove(1);
			int ly = cmd6303_buf->read_not_remove(2);
			if(cmd6303_buf->count() == lx * ly + 3) {
				uint8 code = cmd6303_buf->read();
				bool pre = (udgc[code][0] && udgc[code][1]);
				for(int i = 0; i < lx * ly + 2; i++) {
					uint8 d = cmd6303_buf->read();
					if(!pre) {
						udgc[code][i] = d;
					}
				}
				if(!code) {
					memset(udgc, 0, sizeof(udgc));
				}
				rsp6303_buf->write(RCD00);
				psr |= BIT_F1;
			}
		}
		break;
	case 0x21:
		// define graphic screen block flashing data
		if(cmd6303_buf->count() >= 1) {
			int cnt = cmd6303_buf->read_not_remove(0);
			if(cmd6303_buf->count() == cnt * 3 + 1) {
				gs_blocks = cmd6303_buf->read();
				for(int i = 0; i < gs_blocks; i++) {
					gs_block[i][0] = cmd6303_buf->read();
					gs_block[i][1] = cmd6303_buf->read();
					gs_block[i][2] = cmd6303_buf->read();
				}
				rsp6303_buf->write(RCD00);
				psr |= BIT_F1;
			}
		}
		break;
	case 0x22:
		// draw character font on graphic screen
		if(cmd6303_buf->count() == 4) {
			int x = cmd6303_buf->read() << 8;
			x |= cmd6303_buf->read();
			int y = cmd6303_buf->read();
			int ofs = cmd6303_buf->read() << 3;
			for(int l = 0; l < 8; l++) {
				uint8 pat = font[ofs + l];
				draw_point(x + 0, y + l, pat & 0x20);
				draw_point(x + 1, y + l, pat & 0x10);
				draw_point(x + 2, y + l, pat & 0x08);
				draw_point(x + 3, y + l, pat & 0x04);
				draw_point(x + 4, y + l, pat & 0x02);
				draw_point(x + 5, y + l, pat & 0x01);
			}
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x23:
		// draw user defined character on graphics screen
		if(cmd6303_buf->count() >= 3) {
			int dx = cmd6303_buf->read();
			int dy = cmd6303_buf->read();
			uint8 code = cmd6303_buf->read();
			int lx = udgc[code][0];
			int ly = udgc[code][1];
			uint8* pat = &udgc[code][2];
			if(lx && ly) {
				for(int y = 0; y < ly; y++) {
					for(int x = 0; x < lx; x++) {
						if(dx + x < 60 && dy + y < 64) {
							ram[gs_addr + (dx + x + (dy + y) * 60)] = *pat++;
						}
					}
				}
			}
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x24:
		// read graphics screen data
		if(cmd6303_buf->count() == 3) {
			int x = cmd6303_buf->read();
			int y = cmd6303_buf->read();
			uint8* src = &ram[gs_addr + (x + y * 60)];
			int cnt = cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			for(int i = 0; i < cnt; i++) {
				rsp6303_buf->write(src[i]);
			}
			psr |= BIT_F1;
		}
		break;
	case 0x25:
		// display data on graphics screen
		if(cmd6303_buf->count() >= 4) {
			int lx = cmd6303_buf->read_not_remove(2);
			int ly = cmd6303_buf->read_not_remove(3);
			if(cmd6303_buf->count() == lx * ly + 5) {
				int dx = cmd6303_buf->read();
				int dy = cmd6303_buf->read();
				lx = cmd6303_buf->read();
				ly = cmd6303_buf->read();
				uint8 ope = cmd6303_buf->read();
				for(int y = 0; y < ly; y++) {
					for(int x = 0; x < lx; x++) {
						uint8 d = cmd6303_buf->read();
						if(dx + x < 60 && dy + y < 64) {
							if(ope == 1) {
								ram[gs_addr + (dx + x + (dy + y) * 60)] &= d;
							} else if(ope == 2) {
								ram[gs_addr + (dx + x + (dy + y) * 60)] |= d;
							} else if(ope == 3) {
								ram[gs_addr + (dx + x + (dy + y) * 60)] ^= d;
							} else {
								ram[gs_addr + (dx + x + (dy + y) * 60)] = d;
							}
						}
					}
				}
				rsp6303_buf->write(RCD00);
				psr |= BIT_F1;
			}
		}
		break;
	case 0x26:
		// move graphics screen block
		if(cmd6303_buf->count() == 6) {
			int sx = cmd6303_buf->read();
			int sy = cmd6303_buf->read();
			int lx = cmd6303_buf->read();
			int ly = cmd6303_buf->read();
			int dx = cmd6303_buf->read();
			int dy = cmd6303_buf->read();
			for(int y = 0; y < ly; y++) {
				for(int x = 0; x < lx; x++) {
					if(sx + x < 60 && sy + y < 64) {
						mov[y][x] = ram[gs_addr + (sx + x + (sy + y) * 60)];
						ram[gs_addr + (sx + x + (sy + y) * 60)] = 0;
					}
				}
			}
			for(int y = 0; y < ly; y++) {
				for(int x = 0; x < lx; x++) {
					if(dx + x < 60 && dy + y < 64) {
						ram[gs_addr + (dx + x + (dy + y) * 60)] = mov[y][x];
					}
				}
			}
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x27:
		// define point
		if(cmd6303_buf->count() == 4) {
			int x = cmd6303_buf->read() << 8;
			x |= cmd6303_buf->read();
			int y = cmd6303_buf->read();
			uint8 ope = cmd6303_buf->read();
			if(ope == 1) {
				draw_point(x, y, 0);
			} else {
				draw_point(x, y, 1);
			}
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x28:
		// read point
		if(cmd6303_buf->count() == 3) {
			int x = cmd6303_buf->read() << 8;
			x |= cmd6303_buf->read();
			int y = cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			rsp6303_buf->write(get_point(x, y));
			psr |= BIT_F1;
		}
		break;
	case 0x29:
		// draw line
		if(cmd6303_buf->count() == 11) {
			int sx = cmd6303_buf->read() << 8;
			sx |= cmd6303_buf->read();
			int sy = cmd6303_buf->read() << 8;
			sy |= cmd6303_buf->read();
			int ex = cmd6303_buf->read() << 8;
			ex |= cmd6303_buf->read();
			int ey = cmd6303_buf->read() << 8;
			ey |= cmd6303_buf->read();
			uint16 ope = cmd6303_buf->read() << 8;
			ope |= cmd6303_buf->read();
			uint8 mode = cmd6303_buf->read();
			if(mode == 1) {
				draw_line(sx, sy, ex, ey, ~ope);
			} else {
				draw_line(sx, sy, ex, ey, ope);
			}
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x30:
		// user defined character
		if(cmd6303_buf->count() == 9) {
			int code = cmd6303_buf->read();
			if(code < 0xe0) {
				for(int i = 0; i < 8; i++) {
					cmd6303_buf->read();
				}
				rsp6303_buf->write(RCD06);
			} else {
				int ofs = code << 3;
				for(int i = 0; i < 8; i++) {
					font[ofs + i] = cmd6303_buf->read();
				}
				rsp6303_buf->write(RCD00);
			}
			psr |= BIT_F1;
		}
		break;
	case 0x31:
		// define character screen block flashing data
		if(cmd6303_buf->count() >= 1) {
			int cnt = cmd6303_buf->read_not_remove(0);
			if(cmd6303_buf->count() == cnt * 3 + 1) {
				cs_blocks = cmd6303_buf->read();
				for(int i = 0; i < cs_blocks; i++) {
					cs_block[i][0] = cmd6303_buf->read();
					cs_block[i][1] = cmd6303_buf->read();
					cs_block[i][2] = cmd6303_buf->read();
				}
				rsp6303_buf->write(RCD00);
				psr |= BIT_F1;
			}
		}
		break;
	case 0x32:
		// read window pointer
		rsp6303_buf->write(RCD00);
		rsp6303_buf->write(wnd_ptr_x);
		rsp6303_buf->write(wnd_ptr_y);
		psr |= BIT_F1;
		break;
	case 0x33:
		// set window pointer
		if(cmd6303_buf->count() == 2) {
			wnd_ptr_x = cmd6303_buf->read();
			wnd_ptr_y = cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x34:
		// read character screen data
		if(cmd6303_buf->count() == 3) {
			int x = cmd6303_buf->read();
			int y = cmd6303_buf->read();
			uint8* src = &ram[cs_addr + (x + y * 80)];
			int cnt = cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			for(int i = 0; i < cnt; i++) {
				rsp6303_buf->write(src[i]);
			}
			psr |= BIT_F1;
		}
		break;
	case 0x35:
		// display data on character screen
		if(cmd6303_buf->count() >= 4) {
			int cnt = cmd6303_buf->read_not_remove(2);
			if(cmd6303_buf->count() == cnt + 3) {
				int x = cmd6303_buf->read();
				int y = cmd6303_buf->read();
				uint8* dest = &ram[cs_addr + (x + y * 80)];
				cnt = cmd6303_buf->read();
				for(int i = 0; i < cnt; i++) {
					dest[i] = cmd6303_buf->read();
				}
				rsp6303_buf->write(RCD00);
				psr |= BIT_F1;
			}
		}
		break;
	case 0x36:
		// move character screen block
		if(cmd6303_buf->count() == 6) {
			int sx = cmd6303_buf->read();
			int sy = cmd6303_buf->read();
			int lx = cmd6303_buf->read();
			int ly = cmd6303_buf->read();
			int dx = cmd6303_buf->read();
			int dy = cmd6303_buf->read();
			for(int y = 0; y < ly; y++) {
				for(int x = 0; x < lx; x++) {
					if(sx + x < 80 && sy + y < 64) {
						mov[y][x] = ram[cs_addr + (sx + x + (sy + y) * 80)];
//						ram[cs_addr + (sx + x + (sy + y) * 80)] = 0;
					}
				}
			}
			for(int y = 0; y < ly; y++) {
				for(int x = 0; x < lx; x++) {
					if(dx + x < 80 && dy + y < 64) {
						ram[cs_addr + (dx + x + (dy + y) * 80)] = mov[y][x];
					}
				}
			}
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x40:
		// read microcassette status
		rsp6303_buf->write(RCD00);
		rsp6303_buf->write(0);
		psr |= BIT_F1;
		break;
	case 0x41:
		// head on
		rsp6303_buf->write(RCD07);
		psr |= BIT_F1;
		break;
	case 0x42:
		// head off
		rsp6303_buf->write(RCD00);
		psr |= BIT_F1;
		break;
	case 0x43:
		// rewind n counts
	case 0x44:
		// fast foward n counts
		if(cmd6303_buf->count() == 2) {
			cmd6303_buf->read();
			cmd6303_buf->read();
			rsp6303_buf->write(RCD07);
			psr |= BIT_F1;
		}
		break;
	case 0x45:
		// rewind
	case 0x47:
		// slow rewind
		rsp6303_buf->write(RCD07);
		psr |= BIT_F1;
		break;
	case 0x46:
		// fast foward
	case 0x48:
		// play
	case 0x49:
		// record
	case 0x4a:
		// stop
		rsp6303_buf->write(RCD00);
		psr |= BIT_F1;
		break;
	case 0x4b:
		// read write protect pin
		rsp6303_buf->write(RCD00);
		rsp6303_buf->write(0);
		psr |= BIT_F1;
		break;
	case 0x4c:
		// read counter
		rsp6303_buf->write(RCD00);
		rsp6303_buf->write(0);
		rsp6303_buf->write(0);
		psr |= BIT_F1;
		break;
	case 0x4d:
		// set counter
		if(cmd6303_buf->count() == 2) {
			cmd6303_buf->read();
			cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x55:
		// set write protect area pointer
		if(cmd6303_buf->count() == 2) {
			cmd6303_buf->read();
			cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x56:
		// reset write protect area pointer
		rsp6303_buf->write(RCD00);
		psr |= BIT_F1;
		break;
	case 0x60:
		// read serial i/o status
		rsp6303_buf->write(RCD00);
		rsp6303_buf->write(0x20 | (tf20_buf->count() ? 0x80 : 0));
		psr |= BIT_F1;
		break;
	case 0x61:
		// set serial port bit rate
		if(cmd6303_buf->count() == 1) {
			cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x62:
		// serial input
		rsp6303_buf->write(RCD00);
		rsp6303_buf->write(tf20_buf->read());
		psr |= BIT_F1;
		break;
	case 0x63:
		// serial output
		if(cmd6303_buf->count() == 1) {
			d_tf20->write_signal(SIGNAL_TF20_SIO, cmd6303_buf->read(), 0xff);
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x64:
		// send data with header
		if(cmd6303_buf->count() >= 6) {
			int cnt = cmd6303_buf->read_not_remove(5);
			if(cmd6303_buf->count() == cnt + 7) {
				int rcv = cmd6303_buf->read();
				int fmt = cmd6303_buf->read();
				int did = cmd6303_buf->read();
				int sid = cmd6303_buf->read();
				int fnc = cmd6303_buf->read();
				int siz = cmd6303_buf->read();
				
				// epsp protocol
				tf20_buf->clear();
				d_tf20->write_signal(SIGNAL_TF20_SIO, DID_FIRST, 0xff);
				d_tf20->write_signal(SIGNAL_TF20_SIO, did, 0xff);
				d_tf20->write_signal(SIGNAL_TF20_SIO, sid, 0xff);
				d_tf20->write_signal(SIGNAL_TF20_SIO, DS_SEL, 0xff);
				tf20_buf->read();	// recv ack
				d_tf20->write_signal(SIGNAL_TF20_SIO, SOH, 0xff);
				d_tf20->write_signal(SIGNAL_TF20_SIO, fmt, 0xff);
				d_tf20->write_signal(SIGNAL_TF20_SIO, did, 0xff);
				d_tf20->write_signal(SIGNAL_TF20_SIO, sid, 0xff);
				d_tf20->write_signal(SIGNAL_TF20_SIO, fnc, 0xff);
				d_tf20->write_signal(SIGNAL_TF20_SIO, siz, 0xff);
				d_tf20->write_signal(SIGNAL_TF20_SIO, 0, 0xff);
				tf20_buf->read();	// recv ack
				d_tf20->write_signal(SIGNAL_TF20_SIO, STX, 0xff);
				for(int i = 0; i < siz + 1; i++) {
					d_tf20->write_signal(SIGNAL_TF20_SIO, cmd6303_buf->read(), 0xff);
				}
				d_tf20->write_signal(SIGNAL_TF20_SIO, 0, 0xff);
				d_tf20->write_signal(SIGNAL_TF20_SIO, 0, 0xff);
				tf20_buf->read();	// recv ack
				d_tf20->write_signal(SIGNAL_TF20_SIO, EOT, 0xff);
				
				rsp6303_buf->write(RCD00);
				if(rcv) {
					rsp6303_buf->write(0);
					tf20_buf->read();
					rsp6303_buf->write(fmt = tf20_buf->read());
					rsp6303_buf->write(did = tf20_buf->read());
					rsp6303_buf->write(sid = tf20_buf->read());
					rsp6303_buf->write(fnc = tf20_buf->read());
					rsp6303_buf->write(siz = tf20_buf->read());
					tf20_buf->read();
					d_tf20->write_signal(SIGNAL_TF20_SIO, ACK, 0xff);	// ack
					tf20_buf->read();
					for(int i = 0; i < siz + 1; i++) {
						rsp6303_buf->write(tf20_buf->read());
					}
					d_tf20->write_signal(SIGNAL_TF20_SIO, ACK, 0xff);	// ack
					d_tf20->write_signal(SIGNAL_TF20_SIO, EOT, 0xff);	// eot
					tf20_buf->clear();
				}
				psr |= BIT_F1;
			}
		}
		break;
	case 0x65:
		// receive data with header
		rsp6303_buf->write(RCD00);
		{
			// epsp protocol
			int fmt, did, sid, fnc, siz;
			rsp6303_buf->write(0);
			tf20_buf->read();
			rsp6303_buf->write(fmt = tf20_buf->read());
			rsp6303_buf->write(did = tf20_buf->read());
			rsp6303_buf->write(sid = tf20_buf->read());
			rsp6303_buf->write(fnc = tf20_buf->read());
			rsp6303_buf->write(siz = tf20_buf->read());
			tf20_buf->read();
			d_tf20->write_signal(SIGNAL_TF20_SIO, ACK, 0xff);	// ack
			tf20_buf->read();
			for(int i = 0; i < siz + 1; i++) {
				rsp6303_buf->write(tf20_buf->read());
			}
			d_tf20->write_signal(SIGNAL_TF20_SIO, ACK, 0xff);	// ack
			d_tf20->write_signal(SIGNAL_TF20_SIO, EOT, 0xff);	// eot
			tf20_buf->clear();
		}
		psr |= BIT_F1;
		break;
	case 0x70:
		// turn on/off prom cupsule power
		if(cmd6303_buf->count() == 1) {
			cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x71:
		// read data
		if(cmd6303_buf->count() == 4) {
			cmd6303_buf->read();
			uint16 addr = cmd6303_buf->read() << 8;
			addr |= cmd6303_buf->read();
			addr ^= 0x4000;
			int cnt = cmd6303_buf->read();
			if(cnt == 0) cnt = 256;
			rsp6303_buf->write(RCD00);
			for(int i = 0; i < cnt; i++) {
				if(addr & 0x8000) {
					rsp6303_buf->write(util[(addr + i) & 0x7fff]);
				} else {
					rsp6303_buf->write(basic[(addr + i) & 0x7fff]);
				}
			}
			psr |= BIT_F1;
		}
		break;
	case 0x72:
		// turn on/off speaker power
		if(cmd6303_buf->count() == 1) {
			cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x73:
		// turn on/off speaker power
		if(cmd6303_buf->count() == 3) {
			cmd6303_buf->read();
			cmd6303_buf->read();
			cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	case 0x74:
		// melody
		if(cmd6303_buf->count() == 3) {
			cmd6303_buf->read();
			cmd6303_buf->read();
			cmd6303_buf->read();
			rsp6303_buf->write(RCD00);
			psr |= BIT_F1;
		}
		break;
	}
}

uint8 IO::get_point(int x, int y)
{
	if(0 <= x && x < 480 && 0 <= y && y < 64) {
		uint8 bit = dot_tbl[x & 7];
		int ofs = y * 60 + (x >> 3);
		return ram[gs_addr + ofs] & bit;
	}
	return 0;
}

void IO::draw_point(int x, int y, uint16 dot)
{
	if(0 <= x && x < 480 && 0 <= y && y < 64) {
		uint8 bit = dot_tbl[x & 7];
		int ofs = y * 60 + (x >> 3);
		if(dot) {
			ram[gs_addr + ofs] |= bit;
		} else {
			ram[gs_addr + ofs] &= ~bit;
		}
	}
}

void IO::draw_line(int sx, int sy, int ex, int ey, uint16 ope)
{
	int next_x = sx, next_y = sy;
	int delta_x = abs(ex - sx) * 2;
	int delta_y = abs(ey - sy) * 2;
	int step_x = (ex < sx) ? -1 : 1;
	int step_y = (ey < sy) ? -1 : 1;
	
	draw_point(sx, sy, ope & 0x8000);
	ope = (ope << 1) | (ope & 0x8000 ? 1 : 0);
	if(delta_x > delta_y) {
		int frac = delta_y - delta_x / 2;
		while(next_x != ex) {
			if(frac >= 0) {
				next_y += step_y;
				frac -= delta_x;
			}
			next_x += step_x;
			frac += delta_y;
			draw_point(next_x, next_y, ope & 0x8000);
			ope = (ope << 1) | (ope & 0x8000 ? 1 : 0);
		}
	} else {
		int frac = delta_x - delta_y / 2;
		while(next_y != ey) {
			if(frac >= 0) {
				next_x += step_x;
				frac -= delta_y;
			}
			next_y += step_y;
			frac += delta_x;
			draw_point(next_x, next_y, ope & 0x8000);
			ope = (ope << 1) | (ope & 0x8000 ? 1 : 0);
		}
	}
	draw_point(ex, ey, ope & 0x8000);
}

// ----------------------------------------------------------------------------
// intelligent ram disk by Mr.Dennis Heynlein
// ----------------------------------------------------------------------------

/*
0x81 (W)	CommandByte c to RAMDisk
0x81 (R)	Statusbyte	
		Bit 0 : Readable DataByte on 0x81 is pending
		Bit 1 : Receive of Data/Command is busy
		Bit 7 and 6 = 0 (ident the RAMdisc)

0x80 (R/W)	DataByte d

Commands:	RESET 		-	input:	c(00)
					output:	d(SWITCHSTATE)		

		READSECTOR	-	input:	c(01) d(TRACK) d(SECTOR)
					output:	d(ERRORSTATE) d(SECTORBYTE)*128
		
		READMEMDIRECT	-	input:	c(02) d(BANK) d(HIGHBYTE) d(LOWBYTE)
					output:	d(ERRORSTATE) d(BYTE)

		WRITESECTOR	-	input:	c(03) d(TRACK) d(SECTOR) d(SECTORBYTE)*128
					output:	d(ERRORSTATE)

		WRITEMEMDIRECT	-	input:	c(04) d(HIGHBYTE) d(LOWBYTE) d(BYTE)
					output:	d(ERRORSTATE)

		INIT_BITMAP	-	input:	c(05)
					output:	d(ERRORSTATE)

ERRORSTATE:	Bit 0 = Ramdiscsize
		Bit 1 = Geometric
		Bit 2 = Writeprotect

HIGHBYTE:	0 - 0xef
LOWBYTE:	0-255
TRACK:		0-14
SECTOR:		0-63
BANK:		1 or 2
*/

void IO::iramdisk_write_data(uint8 val)
{
	if(iramdisk_dest == IRAMDISK_IN && iramdisk_count) {
		*(iramdisk_ptr++) = val;
		iramdisk_count--;
	}
	if(!iramdisk_count) {
		iramdisk_dest = IRAMDISK_OUT;
		iramdisk_ptr = iramdisk_buf;
		int track = iramdisk_buf[0];
		int sector = iramdisk_buf[1];
		
		switch(iramdisk_cmd) {
		case 1: //READSECTOR
			if(track > 14 || sector > 63) {
				iramdisk_buf[0] = 2;
			} else {
				iramdisk_buf[0] = 0;
				for(int t = 0;t < 128; t++) {
					iramdisk_buf[t + 1] = iramdisk_sectors[track][sector][t];
				}
			}
			iramdisk_count = 129; //ERRORCODE + 128 Bytes
			break;
		case 3: //WRITESECTOR
			if(track > 14 || sector > 63) {
				iramdisk_buf[0] = 2;
			} else {
				iramdisk_buf[0] = 0;
				for(int t = 0; t < 128; t++) {
					iramdisk_sectors[track][sector][t] = iramdisk_buf[t+2];
				}
			}
			iramdisk_count = 1; //ERRORCODE
			break;
		case 2: //READMEMDIRECT
			iramdisk_count = 2; //ERRORCODE + 1 Byte
			break;
		case 4: //WRITEMEMDIRECT
			iramdisk_count = 1; //ERRORCODE
			break;
		}
	}
}

void IO::iramdisk_write_cmd(uint8 val)
{
	iramdisk_cmd = val;
	iramdisk_count = 0;
	iramdisk_ptr = iramdisk_buf;
	iramdisk_dest = IRAMDISK_IN;
	
	switch(iramdisk_cmd) {
	case 1:
		iramdisk_count = 2;
		break;
	case 2:
	case 4:
		iramdisk_count = 3;
		break;
	case 3:
		iramdisk_count = 130;
		break;
	default:
		//PROCESS-1-BYTE_CMDs
		iramdisk_count = 1;
		iramdisk_dest = IRAMDISK_OUT;
		if(iramdisk_cmd == 0) {
			iramdisk_buf[0] = 1;	// RESET
		} else {
			iramdisk_buf[0] = 0;	//INIT
		}
	}
}

uint8 IO::iramdisk_read_data()
{
	if(iramdisk_dest == IRAMDISK_OUT) {
		if(iramdisk_count) {
			iramdisk_count--;
			if(!iramdisk_count) {
				iramdisk_dest = IRAMDISK_IN;
			}
			return *(iramdisk_ptr++);
		}
	}
	return 0;
}

uint8 IO::iramdisk_read_stat()
{
	if(iramdisk_dest == IRAMDISK_OUT) {
		return IRAMDISK_WAIT;
	} else {
		return 0;
	}
}

// ----------------------------------------------------------------------------
// video
// ----------------------------------------------------------------------------


void IO::draw_screen()
{
	if(lcd_on) {
		memset(lcd, 0, sizeof(lcd));
		if(scr_mode) {
			// char screen
			uint8* vram = &ram[scr_ptr];
			for(int y = 0; y < (num_lines ? 7 : 8); y++) {
				int py = num_lines ? (y * 9 + 1) : y * 8;
				for(int x = 0; x < 80; x++) {
					int px = x * 6;
					int ofs = vram[y * 80 + x] << 3;
					for(int l = 0; l < 8; l++) {
						uint8 pat = font[ofs + l];
						lcd[py + l][px + 0] = (pat & 0x20) ? 0xff : 0;
						lcd[py + l][px + 1] = (pat & 0x10) ? 0xff : 0;
						lcd[py + l][px + 2] = (pat & 0x08) ? 0xff : 0;
						lcd[py + l][px + 3] = (pat & 0x04) ? 0xff : 0;
						lcd[py + l][px + 4] = (pat & 0x02) ? 0xff : 0;
						lcd[py + l][px + 5] = (pat & 0x01) ? 0xff : 0;
					}
				}
			}
			// block flashing
			if(flash_block) {
				int yofs = (scr_ptr - cs_addr) / 80;
				for(int i = 0; i < cs_blocks; i++) {
					int x = cs_block[i][0];
					int y = cs_block[i][1] - yofs;
					if(0 <= x && x < 80 && 0 <= y && y < 8) {
						int px = x * 6;
						int py = y * 8;
						for(int l = 0; l < 8; l++) {
							lcd[py + l][px + 0] = ~lcd[py + l][px + 0];
							lcd[py + l][px + 1] = ~lcd[py + l][px + 1];
							lcd[py + l][px + 2] = ~lcd[py + l][px + 2];
							lcd[py + l][px + 3] = ~lcd[py + l][px + 3];
							lcd[py + l][px + 4] = ~lcd[py + l][px + 4];
							lcd[py + l][px + 5] = ~lcd[py + l][px + 5];
						}
					}
				}
			}
			// draw cursor
			if(curs_mode & 1) {
				if(!(curs_mode & 2) || (blink & 32)) {
					int px = curs_x * 6;
					int py = curs_y * 8;
					int st = (curs_mode & 4) ? 0 : 7;
					if(px + 6 - 1 < SCREEN_WIDTH) {
						for(int l = st; l < 8 && py + l < SCREEN_HEIGHT; l++) {
							memset(&lcd[py + l][px], 0xff, 6);
						}
					}
				}
			}
		} else {
			// graph screen
			uint8* vram = &ram[gs_addr];
			for(int y = 0; y < 64; y++) {
				for(int x = 0; x < 60; x++) {
					int px = x * 8;
					uint8 pat = *vram++;
					lcd[y][px + 0] = (pat & 0x80) ? 0xff : 0;
					lcd[y][px + 1] = (pat & 0x40) ? 0xff : 0;
					lcd[y][px + 2] = (pat & 0x20) ? 0xff : 0;
					lcd[y][px + 3] = (pat & 0x10) ? 0xff : 0;
					lcd[y][px + 4] = (pat & 0x08) ? 0xff : 0;
					lcd[y][px + 5] = (pat & 0x04) ? 0xff : 0;
					lcd[y][px + 6] = (pat & 0x02) ? 0xff : 0;
					lcd[y][px + 7] = (pat & 0x01) ? 0xff : 0;
				}
			}
			// block flashing
			if(flash_block) {
				for(int i = 0; i < gs_blocks; i++) {
					int x = gs_block[i][0];
					int y = gs_block[i][1];
					if(0 <= x && x < 60 && 0 <= y && y < 8) {
						int px = x * 8;
						int py = y * 8;
						for(int l = 0; l < 8; l++) {
							lcd[py + l][px + 0] = ~lcd[py + l][px + 0];
							lcd[py + l][px + 1] = ~lcd[py + l][px + 1];
							lcd[py + l][px + 2] = ~lcd[py + l][px + 2];
							lcd[py + l][px + 3] = ~lcd[py + l][px + 3];
							lcd[py + l][px + 4] = ~lcd[py + l][px + 4];
							lcd[py + l][px + 5] = ~lcd[py + l][px + 5];
							lcd[py + l][px + 6] = ~lcd[py + l][px + 6];
							lcd[py + l][px + 7] = ~lcd[py + l][px + 7];
						}
					}
				}
			}
		}
		for(int y = 0; y < 64; y++) {
			scrntype* dest = emu->screen_buffer(y);
			for(int x = 0; x < 480; x++) {
				dest[x] = lcd[y][x] ? pd : pb;
			}
		}
	} else {
		for(int y = 0; y < 64; y++) {
			scrntype* dest = emu->screen_buffer(y);
			for(int x = 0; x < 480; x++) {
				dest[x] = pb;
			}
		}
	}
}
