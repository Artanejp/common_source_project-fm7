/*
	FUJITSU FM16beta Emulator 'eFM16beta'

	Author : Takeda.Toshiya
	Date   : 2017.12.28-

	[ main system ]
*/

#include "main.h"
#include "sub.h"
#include "keyboard.h"
#include "../disk.h"
#include "../i8237.h"
#include "../i8259.h"
#ifdef HAS_I286
#include "../i286.h"
#endif
#include "../mb8877.h"
#include "../msm58321.h"
#include "../pcm1bit.h"

void MAIN::initialize()
{
	MEMORY::initialize();
	
	memset(ram, 0x00, sizeof(ram));
	memset(rom, 0xff, sizeof(rom));
	direct = 0;
	
	read_bios(_T("IPL.ROM"), rom, sizeof(rom));
	
	set_memory_rw(0x00000, 0xfbfff, ram);
	set_memory_r(0xfc000, 0xfffff, rom);
	
	// main-sub
//	sub_busy = false;
	
	// 1mb fdd
	sidereg_2hd = 0;	// side 0
	drvreg_2hd = 0;		// drive #0, motor on
	drq_2hd = false;
	d_fdc_2hd->write_signal(SIG_MB8877_MOTOR, 1, 1);
	d_fdc_2hd->write_signal(SIG_MB8877_DRIVEREG, 0, 3);
	
	// 320kb fdd
	sidereg_2d = 0;		// side 0
	drvreg_2d = 0x80;	// drive #0, motor on
	drq_2d = false;
	d_fdc_2d->write_signal(SIG_MB8877_MOTOR, 1, 1);
	d_fdc_2d->write_signal(SIG_MB8877_DRIVEREG, 0, 3);
	
	// rtc
	rtc_data = 0;
	
	// irq
	irq_enb = ext_irq_enb = 0;
	irq0_tx = irq0_rx = irq0_syn = irq1 = irq2 = irq3 = irq4 = irq5 = irq6 = irq7 = irq8 = irq9 = false;
	firq0 = firq1 = firq2 = firq3 = false;
	int0 = int1 = int2 = int3 = int4 = int5 = int6 = int7 = false;
}

void MAIN::release()
{
	MEMORY::release();
}

void MAIN::reset()
{
	MEMORY::reset();
	
#ifdef HAS_I286
	d_dma->set_address_mask(0x000fffff);
	d_cpu->set_address_mask(0x000fffff);
	rst = 0x00;
#endif
	
	
	d_pcm->write_signal(SIG_PCM1BIT_ON, 0, 0);
}

void MAIN::write_io8(uint32_t addr, uint32_t data)
{
	uint8_t change;
	
	switch(addr) {
	case 0x0020:
		d_dma->write_signal(SIG_I8237_BANK0, data, 0xff);
		break;
	case 0x0021:
		d_dma->write_signal(SIG_I8237_BANK1, data, 0xff);
		break;
	case 0x0022:
		d_dma->write_signal(SIG_I8237_BANK2, data, 0xff);
		break;
	case 0x0023:
		d_dma->write_signal(SIG_I8237_BANK3, data, 0xff);
		break;


#ifdef HAS_I286
	case 0x0060:
		if((data & 0xc0) == 0x40) {
			d_cpu->reset();
			rst |= 0x01;
		}
		if(data & 0x01) {
			d_dma->set_address_mask(0x00ffffff);
			d_cpu->set_address_mask(0x00ffffff);
		} else {
			d_dma->set_address_mask(0x000fffff);
			d_cpu->set_address_mask(0x000fffff);
		}
		break;
#endif
	case 0xfd02:
		change = irq_enb ^ data;
		irq_enb = data;
		if(change & 0x01) update_int7();	// Printer
		if(change & 0x0e) update_int4();	// RS-232C
		if(change & 0x10) update_int0();	// PTM
		if(change & 0x20) update_int6();	// 320KB FDD
		if(change & 0x80) update_int1();	// Keyboard
		break;
	case 0xfd03:
		d_pcm->write_signal(SIG_PCM1BIT_ON, data, 0x80);
		break;

	case 0xfd04:
		d_sub->write_signal(SIG_SUB_MAINACK, 1, 1);
		break;

	case 0xfd05:
		d_sub->write_signal(SIG_SUB_HALT, data, 0x80);
		d_sub->write_signal(SIG_SUB_CANCEL, data, 0x40);
		break;

	case 0xfd0f:
		change = direct ^ data;
		direct = data;
		if(change & 0x80) {
			if(direct & 0x80) {
				set_memory_mapped_io_rw(0xc0000, 0xcffff, d_sub);
			} else {
				set_memory_rw(0xc0000, 0xcffff, ram + 0xc0000);
			}
		}
		d_keyboard->write_signal(SIG_KEYBOARD_INSLED, data, 0x02);
		break;
	case 0xfd10:
		d_rtc->write_signal(SIG_MSM58321_DATA, data, 0x0f);
		break;
	case 0xfd11:
		d_rtc->write_signal(SIG_MSM58321_CS, data, 0x80);
		d_rtc->write_signal(SIG_MSM58321_READ, data, 0x04);
		d_rtc->write_signal(SIG_MSM58321_WRITE, data, 0x02);
		d_rtc->write_signal(SIG_MSM58321_ADDR_WRITE, data, 0x01);
		break;

	case 0xfd1c:
		d_fdc_2d->write_signal(SIG_MB8877_SIDEREG, data, 0x01);
		sidereg_2d = data;
		break;
	case 0xfd1d:
		d_fdc_2d->write_signal(SIG_MB8877_MOTOR, data, 0x80);
		d_fdc_2d->write_signal(SIG_MB8877_DRIVEREG, data, 0x03);
		drvreg_2d = data;
		break;


	case 0xfd2c:
		change = ext_irq_enb ^ data;
		ext_irq_enb = data;
		if(change & 0x08) update_int2();	// 1MB FDD
		break;

	case 0xfd34:
		for(int i = 0; i < 4; i++) {
			d_fdc_2hd->set_drive_mfm(i, ((data & 0x40) != 0));
		}
		d_fdc_2hd->write_signal(SIG_MB8877_SIDEREG, data, 0x01);
		sidereg_2hd = data;
		break;
	case 0xfd35:
		d_fdc_2hd->write_signal(SIG_MB8877_MOTOR, ~data, 0x80);
		d_fdc_2hd->write_signal(SIG_MB8877_DRIVEREG, data, 0x03);
		drvreg_2hd = data;
		break;

	}
}

uint32_t MAIN::read_io8(uint32_t addr)
{
	switch(addr) {
#ifdef HAS_I286
	case 0x0060:
		{
			uint8_t val = rst | (d_cpu->get_shutdown_flag() << 1);
			rst = 0;
			d_cpu->set_shutdown_flag(0);
			return val;
		}
#endif
	case 0xfd03:
		return (irq8 ? 0x01 : 0) | (irq7 ? 0x02 : 0) | (irq4 ? 0x08 : 0) | (irq3 ? 0x10 : 0) | (irq2 ? 0x20 : 0) | (irq1 ? 0x40 : 0) | (irq0_tx || irq0_rx || irq0_syn ? 0x80 : 0);


	case 0xfd04:
		return (firq0 ? 0x01 : 0) | (firq1 ? 0x02 : 0) | (firq2 ? 0x80 : 0);

	case 0xfd05:
		return (sub_busy ? 0x80 : 0);

	case 0xfd0f:
		return direct;

	case 0xfd10:
		return rtc_data;

	case 0xfd1c:
		return sidereg_2d;
	case 0xfd1d:
		return drvreg_2d;
	case 0xfd1f:
		return (irq4 ? 0x40 : 0) | (drq_2d ? 0x80 : 0);

	case 0xfd2c:
		return (irq6 ? 0x04 : 0) | (irq5 ? 0x40 : 0);

	case 0xfd34:
		return sidereg_2hd;
	case 0xfd35:
		return drvreg_2hd;
	case 0xfd36:
		{
			uint8_t val = 0x40; // DSNS=1
			if(d_fdc_2hd->read_signal(SIG_MB8877_MOTOR)) {
				for(int i = 0; i < 4; i++) {
					if(d_fdc_2hd->is_disk_inserted(i)) {
						val |= 1 << i;
					}
				}
			}
			DISK *disk = d_fdc_2hd->get_disk_handler(drvreg_2hd & 3);
			if(disk->get_rpm() == 300) {
				val |= 0x20;
			}
			if(disk->inserted && disk->two_side) {
				val |= 0x80;
			}
			return val;
		}
	case 0xfd37:
		return (irq5 ? 0x40 : 0) | (drq_2hd ? 0x80 : 0);

//	case 0xfda0:

	}
	return 0xff;
}

void MAIN::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MAIN_IRQ0_TX) {
		irq0_tx = ((data & mask) != 0);
		update_int4();
	} else if(id == SIG_MAIN_IRQ0_RX) {
		irq0_rx = ((data & mask) != 0);
		update_int4();
	} else if(id == SIG_MAIN_IRQ0_SYN) {
		irq0_syn = ((data & mask) != 0);
		update_int4();
	} else if(id == SIG_MAIN_IRQ1) {
		if(!(irq_enb & 0x80)) {
			d_sub->write_signal(SIG_SUB_KEY, data, mask);
		}
		irq1 = ((data & mask) != 0);
		update_int1();
	} else if(id == SIG_MAIN_IRQ2) {
		irq2 = ((data & mask) != 0);
		update_int5();
	} else if(id == SIG_MAIN_IRQ3) {
		irq3 = ((data & mask) != 0); // dma
//		update_int*();
	} else if(id == SIG_MAIN_IRQ4) {
		irq4 = ((data & mask) != 0);
		update_int6();
	} else if(id == SIG_MAIN_IRQ5) {
		irq5 = ((data & mask) != 0);
		update_int2();
	} else if(id == SIG_MAIN_IRQ6) {
		irq6 = ((data & mask) != 0);
		update_int2();
	} else if(id == SIG_MAIN_IRQ7) {
		irq7 = ((data & mask) != 0);
		update_int6();
	} else if(id == SIG_MAIN_IRQ8) {
		irq8 = ((data & mask) != 0);
		update_int0();
	} else if(id == SIG_MAIN_IRQ9) {
		irq9 = ((data & mask) != 0);
		update_int5();
	} else if(id == SIG_MAIN_FIRQ0) {
		firq0 = ((data & mask) != 0);
		update_int3();
	} else if(id == SIG_MAIN_FIRQ1) {
		firq1 = ((data & mask) != 0);
		update_int1();
	} else if(id == SIG_MAIN_FIRQ2) {
		firq2 = ((data & mask) != 0);
		update_int3();
	} else if(id == SIG_MAIN_FIRQ3) {
		firq3 = ((data & mask) != 0);
		update_int3();
	} else if(id == SIG_MAIN_SUB_BUSY) {
		sub_busy = ((data & mask) != 0);

this->out_debug_log(_T("SUB -> MAIN: SUB BUSY = %d\n"), sub_busy);

	} else if(id == SIG_MAIN_DRQ_2HD) {
		drq_2hd = ((data & mask) != 0);
	} else if(id == SIG_MAIN_DRQ_2D) {
		drq_2d = ((data & mask) != 0);
	} else if(id == SIG_MAIN_RTC_DATA) {
		rtc_data = (data & mask) | (rtc_data & ~mask);
	} else if(id == SIG_MAIN_RTC_BUSY) {
		rtc_data = (data & mask) | (rtc_data & ~mask);
	}
}

/*
IRQ8			-> INT0
	IRQ8: タイマー
IRQ1 + FIRQ1		-> INT1
	IRQ1: キーボード
	FIRQ1: BREAKキー
IRQ5 + IRQ6		-> INT2
	IRQ5: IMBフロッピィディスク
	IRQ6: ハードディスク
FIRQ0 + FIRQ2 + FIRQ3	-> INT3
	FIRQ0: SUBアテンション
	FIRQ2: 拡張
	FIRQ3: ユーザ用
IRQ0			-> INT4
	IRQ0: RS-232C
IRQ2 + IRQ9 + INTNDP	-> INT5
	IRQ2: 拡張
	IRQ9: ユーザ用
IRQ4			-> INT6
	IRQ4: 320KBフロッピィディスク
IRQ7			-> INT7
	IRQ7: プリンタ
*/

void MAIN::update_int0()
{
//	bool prev = int0;
	int0 = (irq8 && (irq_enb & 0x10));
//	if(prev != int0) {
		d_pic->write_signal(SIG_I8259_IR0, int0 ? 1 : 0, 1);
//	}
}

void MAIN::update_int1()
{
//	bool prev = int1;
	int1 = (irq1 && (irq_enb & 0x80)) || firq1;
//	if(prev != int1) {
		d_pic->write_signal(SIG_I8259_IR1, int1 ? 1 : 0, 1);
//	}
}

void MAIN::update_int2()
{
//	bool prev = int2;
	int2 = (irq5 && (ext_irq_enb & 0x08)) || irq6;
//	if(prev != int2) {
		d_pic->write_signal(SIG_I8259_IR2, int2 ? 1 : 0, 1);
//	}
}

void MAIN::update_int3()
{
//	bool prev = int3;
	int3 = firq0 || firq2 || firq3;
//	if(prev != int3) {
		d_pic->write_signal(SIG_I8259_IR3, int3 ? 1 : 0, 1);
//	}
}

void MAIN::update_int4()
{
//	bool prev = int4;
	int4 = (irq0_tx && (irq_enb & 0x02)) || (irq0_rx && (irq_enb & 0x04)) || (irq0_syn && (irq_enb & 0x08));
//	if(prev != int4) {
		d_pic->write_signal(SIG_I8259_IR4, int4 ? 1 : 0, 1);
//	}
}

void MAIN::update_int5()
{
//	bool prev = int5;
	int5 = irq2 || irq9;
//	if(prev != int5) {
		d_pic->write_signal(SIG_I8259_IR5, int5 ? 1 : 0, 1);
//	}
}

void MAIN::update_int6()
{
//	bool prev = int6;
	int6 = (irq6 && (irq_enb & 0x20));
//	if(prev != int6) {
		d_pic->write_signal(SIG_I8259_IR6, int6 ? 1 : 0, 1);
//	}
}

void MAIN::update_int7()
{
//	bool prev = int7;
	int7 = irq7 && (irq_enb & 0x01);
//	if(prev != int7) {
		d_pic->write_signal(SIG_I8259_IR7, int7 ? 1 : 0, 1);
//	}
}

#define STATE_VERSION	1

bool MAIN::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return true;
}

