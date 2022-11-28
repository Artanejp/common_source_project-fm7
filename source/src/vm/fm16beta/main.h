/*
	FUJITSU FM16beta Emulator 'eFM16beta'

	Author : Takeda.Toshiya
	Date   : 2017.12.28-

	[ main system ]
*/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "../memory.h"

#define SIG_MAIN_IRQ0_TX	0	// RS-232C
#define SIG_MAIN_IRQ0_RX	1	// RS-232C
#define SIG_MAIN_IRQ0_SYN	2	// RS-232C
#define SIG_MAIN_IRQ1		3	// Keyboard
#define SIG_MAIN_IRQ2		4	// Expantion
#define SIG_MAIN_IRQ3		5	// DMA Controller
#define SIG_MAIN_IRQ4		6	// 320KB Floppy Disk
#define SIG_MAIN_IRQ5		7	// 1MB Floppy Disk
#define SIG_MAIN_IRQ6		8	// Hard Disk
#define SIG_MAIN_IRQ7		9	// Printer
#define SIG_MAIN_IRQ8		10	// PTM
#define SIG_MAIN_IRQ9		11	// User
#define SIG_MAIN_FIRQ0		12	// Sub system attention
#define SIG_MAIN_FIRQ1		13	// Break
#define SIG_MAIN_FIRQ2		14	// Expantion
#define SIG_MAIN_FIRQ3		15	// User

#define SIG_MAIN_SUB_BUSY	16

#define SIG_MAIN_DRQ_2HD	17
#define SIG_MAIN_DRQ_2D		18

#define SIG_MAIN_RTC_DATA	19
#define SIG_MAIN_RTC_BUSY	20

#ifdef HAS_I286
class I286;
#endif
class I8237;
class MB8877;

class MAIN : public MEMORY
{
private:
#ifdef HAS_I286
	I286 *d_cpu;
	uint8_t rst;
#endif
	I8237 *d_dma;
	DEVICE *d_pic;
	DEVICE *d_pcm;
	DEVICE *d_keyboard;

	// memory
	uint8_t ram[0xfc000];
	uint8_t rom[0x04000];
	uint8_t direct;
	
	// main-sub
	DEVICE *d_sub;
	bool sub_busy;
	
	// 1mb fdd
	MB8877 *d_fdc_2hd;
	uint8_t sidereg_2hd, drvreg_2hd;
	bool drq_2hd;
	
	// 320kb fdd
	MB8877 *d_fdc_2d;
	uint8_t sidereg_2d, drvreg_2d;
	bool drq_2d;
	
	// rtc
	DEVICE *d_rtc;
	uint8_t rtc_data;
	
	// irq
	uint8_t irq_enb, ext_irq_enb;
	bool irq0_tx, irq0_rx, irq0_syn, irq1, irq2, irq3, irq4, irq5, irq6, irq7, irq8, irq9;
	bool firq0, firq1, firq2, firq3;
	bool int0, int1, int2, int3, int4, int5, int6, int7;
	
	void update_int0();
	void update_int1();
	void update_int2();
	void update_int3();
	void update_int4();
	void update_int5();
	void update_int6();
	void update_int7();
	
public:
	MAIN(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Main System"));
	}
	~MAIN() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
#ifdef HAS_I286
	void set_context_cpu(I286* device)
	{
		d_cpu = device;
	}
#endif
	void set_context_dma(I8237* device)
	{
		d_dma = device;
	}
	void set_context_fdc_2hd(MB8877* device)
	{
		d_fdc_2hd = device;
	}
	void set_context_fdc_2d(MB8877* device)
	{
		d_fdc_2d = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
	void set_context_sub(DEVICE* device)
	{
		d_sub = device;
	}
	void set_context_keyboard(DEVICE* device)
	{
		d_keyboard = device;
	}
};

#endif
