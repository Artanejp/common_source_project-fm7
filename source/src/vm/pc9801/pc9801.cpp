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

	Author : Takeda.Toshiya
	Date   : 2010.09.15-

	[ virtual machine ]
*/

#include "pc9801.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#if defined(SUPPORT_OLD_BUZZER)
#include "../beep.h"
#endif
#include "../disk.h"
#if defined(USE_HARD_DISK)
#include "../harddisk.h"
#endif
#include "../i8237.h"
#include "../i8251.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../i8259.h"
#if defined(HAS_I386) || defined(HAS_I486)
#include "../i386.h"
#elif defined(HAS_I86) || defined(HAS_V30)
#include "../i286.h"
#else
#include "../i286.h"
#endif
#include "../io.h"
#include "../ls244.h"
#include "../memory.h"
#include "../noise.h"
#include "../not.h"
#if !defined(SUPPORT_OLD_BUZZER)
#include "../pcm1bit.h"
#endif
//#include "../pcpr201.h"
#include "../prnfile.h"
#if defined(SUPPORT_SASI_IF) || defined(SUPPORT_SCSI_IF)
#include "../scsi_hdd.h"
#include "../scsi_host.h"
#endif
#include "../tms3631.h"
#include "../upd1990a.h"
#include "../upd7220.h"
#include "../upd765a.h"
#include "../ym2203.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
#include "cpureg.h"
#endif
#include "display.h"
#include "dmareg.h"
#include "floppy.h"
#include "fmsound.h"
#include "joystick.h"
#include "keyboard.h"
#include "membus.h"
#include "mouse.h"
#if defined(SUPPORT_SASI_IF)
#include "sasi.h"
#include "sasi_bios.h"
#endif
#if defined(SUPPORT_SCSI_IF)
#include "scsi.h"
#endif
#if defined(SUPPORT_IDE_IF)
#include "ide.h"
#endif

#if defined(SUPPORT_CMT_IF)
using PC9801::CMT;
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
using PC9801::CPUREG;
#endif
using PC9801::DISPLAY;
using PC9801::DMAREG;
using PC9801::FLOPPY;
using PC9801::FMSOUND;
using PC9801::JOYSTICK;
using PC9801::KEYBOARD;
using PC9801::MEMBUS;
using PC9801::MOUSE;
#if defined(SUPPORT_SASI_IF)
using PC9801::SASI;
using PC9801::BIOS;
#endif
#if defined(SUPPORT_SCSI_IF)
using PC9801::SCSI;
#endif
#if defined(SUPPORT_IDE_IF)
using PC9801::IDE;
#endif

#if defined(SUPPORT_320KB_FDD_IF)
#include "../pc80s31k.h"
#include "../z80.h"
#endif
#if defined(SUPPORT_CMT_IF)
#include "cmt.h"
#endif

#if defined(_PC98DO) || defined(_PC98DOPLUS)
#include "../pc80s31k.h"
#include "../z80.h"
#include "../pc8801/pc88.h"
#endif

#if defined(_PC98DO) || defined(_PC98DOPLUS)
using PC88DEV::PC88;
#endif
// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// check configs
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	boot_mode = config.boot_mode;
#endif
	int cpu_clocks = CPU_CLOCKS;
#if defined(PIT_CLOCK_8MHZ)
	pit_clock_8mhz = true;
#else
	pit_clock_8mhz = false;
#endif
	int pit_clocks = pit_clock_8mhz ? 1996812 : 2457600;
	
	sound_type = config.sound_type;
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
#if defined(SUPPORT_OLD_BUZZER)
	beep = new BEEP(this, emu);
#else
	beep = new PCM1BIT(this, emu);
#endif
	dma = new I8237(this, emu);
#ifdef USE_DEBUGGER
	dma->set_context_debugger(new DEBUGGER(this, emu));
#endif
#if defined(SUPPORT_CMT_IF)
	sio_cmt = new I8251(this, emu);		// for cmt
	sio_cmt->set_device_name(_T("8251 SIO (CMT)"));
#endif
	sio_rs = new I8251(this, emu);		// for rs232c
	sio_rs->set_device_name(_T("8251 SIO (RS-232C)"));
	sio_kbd = new I8251(this, emu);		// for keyboard
	sio_kbd->set_device_name(_T("8251 SIO (Keyboard)"));
	pit = new I8253(this, emu);
#if defined(SUPPORT_320KB_FDD_IF)
	pio_fdd = new I8255(this, emu);		// for 320kb fdd i/f
	pio_fdd->set_device_name(_T("8255 PIO (320KB I/F)"));
#endif
	pio_mouse = new I8255(this, emu);	// for mouse
	pio_mouse->set_device_name(_T("8255 PIO (Mouse)"));
	pio_sys = new I8255(this, emu);		// for system port
	pio_sys->set_device_name(_T("8255 PIO (System)"));
	pio_prn = new I8255(this, emu);		// for printer
	pio_prn->set_device_name(_T("8255 PIO (Printer)"));
	pic = new I8259(this, emu);
#if defined(HAS_I386) || defined(HAS_I486)
	cpu = new I386(this, emu); // 80386, 80486
#elif defined(HAS_I86) || defined(HAS_V30)
	cpu = new I286(this, emu);// 8086, V30, 80286
#else
	cpu = new I286(this, emu);
#endif	
#if defined(HAS_I86)
	cpu->set_device_name(_T("CPU(i8086)"));
#elif defined(HAS_I386)
	cpu->set_device_name(_T("CPU(i386)"));
#elif defined(HAS_I486)
	cpu->set_device_name(_T("CPU(i486)"));
#elif defined(HAS_PENTIUM)
	cpu->set_device_name(_T("CPU(Pentium)"));
#elif defined(HAS_V33A)
	cpu->set_device_name(_T("CPU(V33A)"));
#elif defined(HAS_V30)
	cpu->set_device_name(_T("CPU(V30)"));
#else
	cpu->set_device_name(_T("CPU(i286)"));
#endif
	io = new IO(this, emu);
	rtcreg = new LS244(this, emu);
	rtcreg->set_device_name(_T("74LS244 (RTC)"));
	//memory = new MEMORY(this, emu);
	memory = new MEMBUS(this, emu);
	not_busy = new NOT(this, emu);
	not_busy->set_device_name(_T("NOT Gate (Printer Busy)"));
#if defined(HAS_I86) || defined(HAS_V30)
	not_prn = new NOT(this, emu);
	not_prn->set_device_name(_T("NOT Gate (Printer IRQ)"));
#endif
	rtc = new UPD1990A(this, emu);
#if defined(SUPPORT_2HD_FDD_IF)
	fdc_2hd = new UPD765A(this, emu);
	fdc_2hd->set_device_name(_T("uPD765A FDC (2HD I/F)"));
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	fdc_2dd = new UPD765A(this, emu);
	fdc_2dd->set_device_name(_T("uPD765A FDC (2DD I/F)"));
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	fdc = new UPD765A(this, emu);
	fdc->set_device_name(_T("uPD765A FDC (2HD/2DD I/F)"));
#endif
	noise_seek = new NOISE(this, emu);
	noise_head_down = new NOISE(this, emu);
	noise_head_up = new NOISE(this, emu);
#if defined(SUPPORT_SASI_IF)
	sasi_host = new SCSI_HOST(this, emu);
	sasi_hdd = new SASI_HDD(this, emu);
	sasi_hdd->set_device_name(_T("SASI Hard Disk Drive"));
	sasi_hdd->scsi_id = 0;
	sasi_hdd->bytes_per_sec = 625 * 1024; // 625KB/s
	for(int i = 0; i < USE_HARD_DISK; i++) {
		sasi_hdd->set_disk_handler(i, new HARDDISK(emu));
	}
	sasi_hdd->set_context_interface(sasi_host);
	sasi_host->set_context_target(sasi_hdd);
#endif
#if defined(SUPPORT_SCSI_IF)
	scsi_host = new SCSI_HOST(this, emu);
	for(int i = 0; i < USE_HARD_DISK; i++) {
		scsi_hdd[i] = new SCSI_HDD(this, emu);
		scsi_hdd[i]->set_device_name(_T("SCSI Hard Disk Drive #%d"), i + 1);
		scsi_hdd[i]->scsi_id = i;
		scsi_hdd[i]->set_disk_handler(0, new HARDDISK(emu));
		scsi_hdd[i]->set_context_interface(scsi_host);
		scsi_host->set_context_target(scsi_hdd[i]);
	}
#endif
	gdc_chr = new UPD7220(this, emu);
	gdc_chr->set_device_name(_T("uPD7220 GDC (Character)"));
	gdc_gfx = new UPD7220(this, emu);
	gdc_gfx->set_device_name(_T("uPD7220 GDC (Graphics)"));
	
	if(sound_type == 0 || sound_type == 1) {
		opn = new YM2203(this, emu);
#ifdef USE_DEBUGGER
		opn->set_context_debugger(new DEBUGGER(this, emu));
#endif
#ifdef SUPPORT_PC98_OPNA
		opn->set_device_name(_T("YM2608 OPNA (PC-9801-86)"));
		opn->is_ym2608 = true;
#else
		opn->set_device_name(_T("YM2203 OPN (PC-9801-26)"));
		opn->is_ym2608 = false;
#endif
		fmsound = new FMSOUND(this, emu);
		joystick = new JOYSTICK(this, emu);
	} else if(sound_type == 2 || sound_type == 3) {
		tms3631 = new TMS3631(this, emu);
		tms3631->set_device_name(_T("TMS3631 SSG (PC-9801-14)"));
		pit_14 = new I8253(this, emu);
		pit_14->set_device_name(_T("8253 PIT (PC-9801-14)"));
		pio_14 = new I8255(this, emu);
		pio_14->set_device_name(_T("8255 PIO (PC-9801-14)"));
		maskreg_14 = new LS244(this, emu);
		maskreg_14->set_device_name(_T("74LS244 (PC-9801-14)"));
	}
	if(config.printer_type == 0) {
		printer = new PRNFILE(this, emu);
//	} else if(config.printer_type == 1) {
//		printer = new PCPR201(this, emu);
	} else {
		printer = dummy;
	}
	
#if defined(SUPPORT_CMT_IF)
	cmt = new CMT(this, emu);
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	cpureg = new CPUREG(this, emu);
#endif
	display = new DISPLAY(this, emu);
	dmareg = new DMAREG(this, emu);
	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	mouse = new MOUSE(this, emu);
#if defined(SUPPORT_SASI_IF)
	sasi = new SASI(this, emu);
	sasi_bios = new BIOS(this, emu);
#endif
#if defined(SUPPORT_SCSI_IF)
	scsi = new SCSI(this, emu);
#endif
#if defined(SUPPORT_IDE_IF)
	ide = new IDE(this, emu);
#endif
	
#if defined(SUPPORT_320KB_FDD_IF)
	// 320kb fdd drives
	pio_sub = new I8255(this, emu);
	pio_sub->set_device_name(_T("8255 PIO (320KB FDD)"));
	pc80s31k = new PC80S31K(this, emu);
	pc80s31k->set_device_name(_T("PC-80S31K (320KB FDD)"));
	fdc_sub = new UPD765A(this, emu);
	fdc_sub->set_device_name(_T("uPD765A FDC (320KB FDD)"));
	cpu_sub = new Z80(this, emu);
	cpu_sub->set_device_name(_T("Z80 CPU (320KB FDD)"));
#endif
	
	/* IRQ	0  PIT
		1  KEYBOARD
		2  CRTV
		3  (INT0)
		4  RS-232C
		5  (INT1)
		6  (INT2)
		7  SLAVE PIC
		8  PRINTER
		9  (INT3) PC-9801-27 (SASI), PC-9801-55 (SCSI), or IDE
		10 (INT41) FDC (640KB I/F)
		11 (INT42) FDC (1MB I/F)
		12 (INT5) PC-9801-26(K) or PC-9801-14
		13 (INT6) MOUSE
		14 
		15 (RESERVED)
	*/
	
	// set contexts
	
#if defined(HAS_I386) || defined(HAS_I486)
	cpu->set_context_extreset(cpureg, SIG_CPUREG_RESET, 0xffffffff);
#endif
	event->set_context_cpu(cpu, cpu_clocks);
#if defined(SUPPORT_320KB_FDD_IF)
	event->set_context_cpu(cpu_sub, 4000000);
#endif
	event->set_context_sound(beep);
	if(sound_type == 0 || sound_type == 1) {
		event->set_context_sound(opn);
		event->set_context_sound(fmsound);
	} else if(sound_type == 2 || sound_type == 3) {
		event->set_context_sound(tms3631);
	}
	event->set_context_sound(noise_seek);
	event->set_context_sound(noise_head_down);
	event->set_context_sound(noise_head_up);
	
	dma->set_context_memory(memory);
	// dma ch.0: sasi
	// dma ch.1: memory refresh
#if defined(SUPPORT_2HD_FDD_IF)
	dma->set_context_ch2(fdc_2hd);
	dma->set_context_tc2(fdc_2hd, SIG_UPD765A_TC, 1);
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	dma->set_context_ch3(fdc_2dd);
	dma->set_context_tc3(fdc_2dd, SIG_UPD765A_TC, 1);
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
#if !defined(SUPPORT_HIRESO)
	dma->set_context_ch2(fdc);
	dma->set_context_ch3(fdc);
	dma->set_context_tc2(fdc, SIG_UPD765A_TC, 1);
	dma->set_context_tc3(fdc, SIG_UPD765A_TC, 1);
#else
	dma->set_context_ch1(fdc);
	dma->set_context_tc1(fdc, SIG_UPD765A_TC, 1);
#endif
#endif
//	sio_rs->set_context_rxrdy(pic, SIG_I8259_CHIP0 | SIG_I8259_IR4, 1);
	sio_kbd->set_context_rxrdy(pic, SIG_I8259_CHIP0 | SIG_I8259_IR1, 1);
	pit->set_context_ch0(pic, SIG_I8259_CHIP0 | SIG_I8259_IR0, 1);
#if defined(SUPPORT_OLD_BUZZER)
	// pit ch.1: memory refresh
#else
	// pit ch.1: buzzer
	pit->set_context_ch1(beep, SIG_PCM1BIT_SIGNAL, 1);
#endif
	// pit ch.2: rs-232c
	pit->set_constant_clock(0, pit_clocks);
	pit->set_constant_clock(1, pit_clocks);
	pit->set_constant_clock(2, pit_clocks);
	pio_mouse->set_context_port_c(mouse, SIG_MOUSE_PORT_C, 0xf0, 0);
#if defined(SUPPORT_HIRESO)
	// sysport port.c bit7,5: sysport port.b bit4,3
	pio_sys->set_context_port_c(pio_sys, SIG_I8255_PORT_B, 0x80, -3); // SHUT0
	pio_sys->set_context_port_c(pio_sys, SIG_I8255_PORT_B, 0x20, -2); // SHUT1
#endif
	// sysport port.c bit6: printer strobe
#if defined(SUPPORT_OLD_BUZZER)
	pio_sys->set_context_port_c(beep, SIG_BEEP_MUTE, 0x08, 0);
#else
	pio_sys->set_context_port_c(beep, SIG_PCM1BIT_MUTE, 0x08, 0);
#endif
	// sysport port.c bit2: enable txrdy interrupt
	// sysport port.c bit1: enable txempty interrupt
	// sysport port.c bit0: enable rxrdy interrupt
	pio_prn->set_context_port_a(printer, SIG_PRINTER_DATA, 0xff, 0);
	pio_prn->set_context_port_c(printer, SIG_PRINTER_STROBE, 0x80, 0);
	if(config.printer_type == 0) {
		PRNFILE *prnfile = (PRNFILE *)printer;
		prnfile->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
//	} else if(config.printer_type == 1) {
//		PRNFILE *pcpr201 = (PCPR201 *)printer;
//		pcpr201->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
	}
	not_busy->set_context_out(pio_prn, SIG_I8255_PORT_B, 4);
#if defined(HAS_I86) || defined(HAS_V30)
	pio_prn->set_context_port_c(not_prn, SIG_NOT_INPUT, 8, 0);
	not_prn->set_context_out(pic, SIG_I8259_CHIP1 | SIG_I8259_IR0, 1);
#endif
	rtcreg->set_context_output(rtc, SIG_UPD1990A_CMD, 0x07, 0);
	rtcreg->set_context_output(rtc, SIG_UPD1990A_DIN, 0x20, 0);
	rtcreg->set_context_output(rtc, SIG_UPD1990A_STB, 0x08, 0);
	rtcreg->set_context_output(rtc, SIG_UPD1990A_CLK, 0x10, 0);
	pic->set_context_cpu(cpu);
	rtc->set_context_dout(pio_sys, SIG_I8255_PORT_B, 1);
	
	if(sound_type == 0 || sound_type == 1) {
		opn->set_context_irq(pic, SIG_I8259_CHIP1 | SIG_I8259_IR4, 1);
		opn->set_context_port_b(joystick, SIG_JOYSTICK_SELECT, 0xc0, 0);
		fmsound->set_context_opn(opn);
		fmsound->set_context_pcm_int(pic, SIG_I8259_CHIP1 | SIG_I8259_IR4, 1); // OK?
		joystick->set_context_opn(opn);

	} else if(sound_type == 2 || sound_type == 3) {
		pio_14->set_context_port_a(tms3631, SIG_TMS3631_ENVELOP1, 0xff, 0);
		pio_14->set_context_port_b(tms3631, SIG_TMS3631_ENVELOP2, 0xff, 0);
		pio_14->set_context_port_c(tms3631, SIG_TMS3631_DATAREG, 0xff, 0);
		maskreg_14->set_context_output(tms3631, SIG_TMS3631_MASKREG, 0xff, 0);
		pit_14->set_constant_clock(2, 1996800 / 8);
		pit_14->set_context_ch2(pic, SIG_I8259_CHIP1 | SIG_I8259_IR4, 1);
	}
	
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	cpureg->set_context_cpu(cpu);
	cpureg->set_context_membus(memory);
	cpureg->set_context_piosys(pio_sys);
#endif
	display->set_context_pic(pic);
	display->set_context_gdc_chr(gdc_chr, gdc_chr->get_ra());
	display->set_context_gdc_gfx(gdc_gfx, gdc_gfx->get_ra(), gdc_gfx->get_cs());
	display->set_context_gdc_freq(pio_sys, SIG_I8255_PORT_A, 0x80);
	//pio_sys->set_context_port_b(display, SIG_DISPLAY98_HIGH_RESOLUTION, 0x08, 0); // SHUT1
	//display->set_context_pio_prn(pio_prn);
	
	dmareg->set_context_dma(dma);
	keyboard->set_context_sio(sio_kbd);
	memory->set_context_display(display);
	mouse->set_context_pic(pic);
	mouse->set_context_pio(pio_mouse);
	
#if defined(SUPPORT_2HD_FDD_IF)
	fdc_2hd->set_context_irq(floppy, SIG_FLOPPY_2HD_IRQ, 1);
	fdc_2hd->set_context_drq(floppy, SIG_FLOPPY_2HD_DRQ, 1);
	fdc_2hd->set_context_noise_seek(noise_seek);
	fdc_2hd->set_context_noise_head_down(noise_head_down);
	fdc_2hd->set_context_noise_head_up(noise_head_up);
	fdc_2hd->raise_irq_when_media_changed = true;
	floppy->set_context_fdc_2hd(fdc_2hd);
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	fdc_2dd->set_context_irq(floppy, SIG_FLOPPY_2DD_IRQ, 1);
	fdc_2dd->set_context_drq(floppy, SIG_FLOPPY_2DD_DRQ, 1);
	fdc_2dd->set_context_noise_seek(noise_seek);
	fdc_2dd->set_context_noise_head_down(noise_head_down);
	fdc_2dd->set_context_noise_head_up(noise_head_up);
	fdc_2dd->raise_irq_when_media_changed = true;
	floppy->set_context_fdc_2dd(fdc_2dd);
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	fdc->set_context_irq(floppy, SIG_FLOPPY_IRQ, 1);
	fdc->set_context_drq(floppy, SIG_FLOPPY_DRQ, 1);
	fdc->set_context_noise_seek(noise_seek);
	fdc->set_context_noise_head_down(noise_head_down);
	fdc->set_context_noise_head_up(noise_head_up);
	fdc->raise_irq_when_media_changed = true;
	floppy->set_context_fdc(fdc);
#endif
	floppy->set_context_dma(dma);
	floppy->set_context_pic(pic);
	
#if defined(SUPPORT_SASI_IF)
#if !defined(SUPPORT_HIRESO)
	sasi_host->set_context_irq(pio_sys, SIG_I8255_PORT_B, 0x10);
#endif
	sasi_host->set_context_irq(sasi, SIG_SASI_IRQ, 1);
	sasi_host->set_context_drq(sasi, SIG_SASI_DRQ, 1);
#ifdef _PC98XA
	dma->set_context_ch3(sasi_host);
	dma->set_context_tc3(sasi, SIG_SASI_TC, 1);
#else
	dma->set_context_ch0(sasi_host);
	dma->set_context_tc0(sasi, SIG_SASI_TC, 1);
#endif
	sasi->set_context_host(sasi_host);
	sasi->set_context_hdd(sasi_hdd);
	sasi->set_context_dma(dma);
	sasi->set_context_pic(pic);
#if 1
	sasi_bios->set_context_sasi(sasi);
	sasi_bios->set_context_memory(memory);
	sasi_bios->set_context_cpu(cpu);
	sasi_bios->set_context_pic(pic);
	sasi_bios->set_context_cpureg(cpureg);
	
	cpu->set_context_bios(sasi_bios);
#endif
#endif
#if defined(SUPPORT_SCSI_IF)
	dma->set_context_ch0(scsi);
	scsi->set_context_dma(dma);
	scsi->set_context_pic(pic);
#endif
#if defined(SUPPORT_IDE_IF)
	dma->set_context_ch0(ide);
	ide->set_context_dma(dma);
	ide->set_context_pic(pic);
#endif
	
#if defined(SUPPORT_CMT_IF)
	sio_cmt->set_context_out(cmt, SIG_CMT_OUT);
//	sio_cmt->set_context_txrdy(cmt, SIG_CMT_TXRDY, 1);
//	sio_cmt->set_context_rxrdy(cmt, SIG_CMT_RXRDY, 1);
//	sio_cmt->set_context_txe(cmt, SIG_CMT_TXEMP, 1);
	cmt->set_context_sio(sio_cmt);
#endif
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
#ifdef SINGLE_MODE_DMA
	cpu->set_context_dma(dma);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
#if defined(SUPPORT_320KB_FDD_IF)
	// 320kb fdd drives
	pc80s31k->set_context_cpu(cpu_sub);
	pc80s31k->set_context_fdc(fdc_sub);
	pc80s31k->set_context_pio(pio_sub);
	pio_fdd->set_context_port_a(pio_sub, SIG_I8255_PORT_A, 0xff, 0);
	pio_fdd->set_context_port_b(pio_sub, SIG_I8255_PORT_A, 0xff, 0);
	pio_fdd->set_context_port_c(pio_sub, SIG_I8255_PORT_C, 0x0f, 4);
	pio_fdd->set_context_port_c(pio_sub, SIG_I8255_PORT_C, 0xf0, -4);
	pio_fdd->clear_ports_by_cmdreg = true;
	pio_sub->set_context_port_a(pio_fdd, SIG_I8255_PORT_B, 0xff, 0);
	pio_sub->set_context_port_b(pio_fdd, SIG_I8255_PORT_A, 0xff, 0);
	pio_sub->set_context_port_c(pio_fdd, SIG_I8255_PORT_C, 0x0f, 4);
	pio_sub->set_context_port_c(pio_fdd, SIG_I8255_PORT_C, 0xf0, -4);
	pio_sub->clear_ports_by_cmdreg = true;
	fdc_sub->set_context_irq(cpu_sub, SIG_CPU_IRQ, 1);
	fdc_sub->set_context_noise_seek(noise_seek);
	fdc_sub->set_context_noise_head_down(noise_head_down);
	fdc_sub->set_context_noise_head_up(noise_head_up);
	cpu_sub->set_context_mem(pc80s31k);
	cpu_sub->set_context_io(pc80s31k);
	cpu_sub->set_context_intr(pc80s31k);
#ifdef USE_DEBUGGER
	cpu_sub->set_context_debugger(new DEBUGGER(this, emu));
#endif
#endif
	
	// i/o bus
	io->set_iomap_alias_rw(0x0000, pic, 0);
	io->set_iomap_alias_rw(0x0002, pic, 1);
	io->set_iomap_alias_rw(0x0008, pic, 2);
	io->set_iomap_alias_rw(0x000a, pic, 3);
	
	io->set_iomap_alias_rw(0x0001, dma, 0x00);
	io->set_iomap_alias_rw(0x0003, dma, 0x01);
	io->set_iomap_alias_rw(0x0005, dma, 0x02);
	io->set_iomap_alias_rw(0x0007, dma, 0x03);
	io->set_iomap_alias_rw(0x0009, dma, 0x04);
	io->set_iomap_alias_rw(0x000b, dma, 0x05);
	io->set_iomap_alias_rw(0x000d, dma, 0x06);
	io->set_iomap_alias_rw(0x000f, dma, 0x07);
	io->set_iomap_alias_rw(0x0011, dma, 0x08);
	io->set_iomap_alias_w (0x0013, dma, 0x09);
	io->set_iomap_alias_w (0x0015, dma, 0x0a);
	io->set_iomap_alias_w (0x0017, dma, 0x0b);
	io->set_iomap_alias_w (0x0019, dma, 0x0c);
	io->set_iomap_alias_rw(0x001b, dma, 0x0d);
	io->set_iomap_alias_w (0x001d, dma, 0x0e);
	io->set_iomap_alias_w (0x001f, dma, 0x0f);
	io->set_iomap_single_w(0x0021, dmareg);
	io->set_iomap_single_w(0x0023, dmareg);
	io->set_iomap_single_w(0x0025, dmareg);
	io->set_iomap_single_w(0x0027, dmareg);
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	io->set_iomap_single_w(0x0029, dmareg);
#endif
	
#if defined(SUPPORT_32BIT_ADDRESS)
	io->set_iomap_single_w(0x0e05, dmareg);
	io->set_iomap_single_w(0x0e07, dmareg);
	io->set_iomap_single_w(0x0e09, dmareg);
	io->set_iomap_single_w(0x0e0b, dmareg);
#endif
	
	io->set_iomap_single_w(0x0020, rtcreg);
	
	io->set_iomap_alias_rw(0x0030, sio_rs, 0);
	io->set_iomap_alias_rw(0x0032, sio_rs, 1);
	
#if !(defined(_PC9821AP) || defined(_PC9821AS) || defined(_PC9821AE) || defined(_PC98H) || defined(_PC98LT_VARIANTS))
	io->set_iomap_alias_r(0x0031, pio_sys, 0);
#else
	io->set_iomap_alias_rw(0x0031, pio_sys, 0);
#endif
//	io->set_iomap_alias_rw(0x0033, pio_sys, 1);
	io->set_iomap_alias_r(0x0033, pio_sys, 1); // PORTB should be read only.
	io->set_iomap_alias_rw(0x0035, pio_sys, 2);
	io->set_iomap_alias_w (0x0037, pio_sys, 3);
	
	io->set_iomap_alias_rw(0x0040, pio_prn, 0);
	io->set_iomap_alias_r (0x0042, pio_prn, 1);
	io->set_iomap_alias_rw(0x0044, pio_prn, 2);
	io->set_iomap_alias_w (0x0046, pio_prn, 3);
	
	io->set_iomap_alias_rw(0x0041, sio_kbd, 0);
	io->set_iomap_alias_rw(0x0043, sio_kbd, 1);
	
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	io->set_iomap_single_w(0x0050, cpureg);
	io->set_iomap_single_w(0x0052, cpureg);

	// ToDo: Before PC9801VM.
	io->set_iomap_single_r(0x005c, cpureg);
	io->set_iomap_single_r(0x005d, cpureg);
	io->set_iomap_single_r(0x005e, cpureg);
	io->set_iomap_single_rw(0x005f, cpureg);

#endif
	
#if defined(SUPPORT_320KB_FDD_IF)
	io->set_iomap_alias_rw(0x0051, pio_fdd, 0);
	io->set_iomap_alias_rw(0x0053, pio_fdd, 1);
	io->set_iomap_alias_rw(0x0055, pio_fdd, 2);
	io->set_iomap_alias_w (0x0057, pio_fdd, 3);
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	io->set_iomap_range_r(0x005c, 0x005f, cpureg); // TimeStamp
#endif
	
	io->set_iomap_alias_rw(0x0060, gdc_chr, 0);
	io->set_iomap_alias_rw(0x0062, gdc_chr, 1);
	
	io->set_iomap_single_w(0x0064, display);
	io->set_iomap_single_w(0x0068, display);
#if defined(SUPPORT_16_COLORS)
	io->set_iomap_single_w(0x006a, display);
#endif
	io->set_iomap_single_w(0x006c, display);
	io->set_iomap_single_w(0x006e, display);
	
	io->set_iomap_single_w(0x0070, display);
	io->set_iomap_single_w(0x0072, display);
	io->set_iomap_single_w(0x0074, display);
	io->set_iomap_single_w(0x0076, display);
	io->set_iomap_single_w(0x0078, display);
	io->set_iomap_single_w(0x007a, display);
#if defined(SUPPORT_GRCG)
#if !defined(SUPPORT_HIRESO)
	io->set_iomap_single_rw(0x007c, display);
	io->set_iomap_single_w(0x007e, display);
#else
	io->set_iomap_single_rw(0x00a4, display);
	io->set_iomap_single_w(0x00a6, display);
#endif
#endif
	
#if defined(SUPPORT_SASI_IF)
	io->set_iomap_single_rw(0x0080, sasi);
	io->set_iomap_single_rw(0x0082, sasi);
#endif
#if defined(SUPPORT_SCSI_IF)
	io->set_iomap_single_rw(0x0cc0, scsi);
	io->set_iomap_single_rw(0x0cc2, scsi);
	io->set_iomap_single_rw(0x0cc4, scsi);
#endif
#if defined(SUPPORT_IDE_IF)
	//Q: MAME MAPPING has 0430h and 0432h as IDE control registers.20190408 K.O
	//Q: MAME MAPPING has 0740h to 074fh as IDE control registers.2019516 K.O
	io->set_iomap_single_rw(0x0640, ide);
	io->set_iomap_single_rw(0x0642, ide);
	io->set_iomap_single_rw(0x0644, ide);
	io->set_iomap_single_rw(0x0646, ide);
	io->set_iomap_single_rw(0x0648, ide);
	io->set_iomap_single_rw(0x064a, ide);
	io->set_iomap_single_rw(0x064c, ide);
	io->set_iomap_single_rw(0x064e, ide);
	io->set_iomap_single_rw(0x074c, ide);
	io->set_iomap_single_rw(0x074e, ide);
#endif
	
	io->set_iomap_alias_rw(0x00a0, gdc_gfx, 0);
	io->set_iomap_alias_rw(0x00a2, gdc_gfx, 1);
	
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
	io->set_iomap_single_rw(0x00a4, display);
	io->set_iomap_single_rw(0x00a6, display);
#endif
	io->set_iomap_single_rw(0x00a8, display);
	io->set_iomap_single_rw(0x00aa, display);
	io->set_iomap_single_rw(0x00ac, display);
	io->set_iomap_single_rw(0x00ae, display);
	
	io->set_iomap_single_rw(0x00a1, display);
	io->set_iomap_single_rw(0x00a3, display);
	io->set_iomap_single_rw(0x00a5, display);
	io->set_iomap_single_rw(0x00a9, display);
#if defined(SUPPORT_EGC)
	io->set_iomap_range_rw(0x04a0, 0x04af, display); // EGC REGS
#endif
	
	io->set_iomap_alias_rw(0x0071, pit, 0);
	io->set_iomap_alias_rw(0x0073, pit, 1);
	io->set_iomap_alias_rw(0x0075, pit, 2);
	io->set_iomap_alias_w (0x0077, pit, 3);
	
	io->set_iomap_single_rw(0x0090, floppy);
	io->set_iomap_single_rw(0x0092, floppy);
	io->set_iomap_single_rw(0x0094, floppy);
	io->set_iomap_single_rw(0x0096, floppy);
#if defined(SUPPORT_2HD_2DD_FDD_IF)
#if !defined(SUPPORT_HIRESO)
	io->set_iomap_single_rw(0x00bc, floppy);
	io->set_iomap_single_rw(0x00be, floppy);
#else
//#if !defined(_PC98XA) && !defined(_PC98XL)
	io->set_iomap_single_w (0x00be, floppy);
//#endif
#endif
#endif
#if !defined(SUPPORT_HIRESO)
	io->set_iomap_single_rw(0x00c8, floppy);
	io->set_iomap_single_rw(0x00ca, floppy);
	io->set_iomap_single_rw(0x00cc, floppy);
//	io->set_iomap_single_rw(0x00ce, floppy); // OK? 20190516 K.O
#endif
	
#if defined(SUPPORT_CMT_IF)
	io->set_iomap_alias_rw(0x0091, sio_cmt, 0);
	io->set_iomap_alias_rw(0x0093, sio_cmt, 1);
	io->set_iomap_single_w(0x0095, cmt);
	io->set_iomap_single_w(0x0097, cmt);
#endif
	
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	io->set_iomap_single_rw(0x00f0, cpureg);
	io->set_iomap_single_rw(0x00f2, cpureg);
#endif
#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM)
	io->set_iomap_single_rw(0x00f6, cpureg);
#endif
	
	if(sound_type == 0 || sound_type == 1) {
		io->set_iomap_single_rw(0x0188, fmsound);
		io->set_iomap_single_rw(0x018a, fmsound);
#ifdef SUPPORT_PC98_OPNA
		io->set_iomap_single_rw(0x018c, fmsound);
		io->set_iomap_single_rw(0x018e, fmsound);
		io->set_iomap_single_rw(0xa460, fmsound);
		io->set_iomap_single_rw(0xa460, fmsound);
		io->set_iomap_single_rw(0xa466, fmsound);
		io->set_iomap_single_rw(0xa468, fmsound);
		io->set_iomap_single_rw(0xa46a, fmsound);
		io->set_iomap_single_rw(0xa46c, fmsound);
		//io->set_iomap_single_rw(0xa46e, fmsound);
#endif
	} else if(sound_type == 2 || sound_type == 3) {
		io->set_iomap_alias_rw(0x0088, pio_14, 0);
		io->set_iomap_alias_rw(0x008a, pio_14, 1);
		io->set_iomap_alias_rw(0x008c, pio_14, 2);
		io->set_iomap_alias_w(0x008e, pio_14, 3);
		io->set_iovalue_single_r(0x008e, 0x08);
		io->set_iomap_single_w(0x0188, maskreg_14);
		io->set_iomap_single_w(0x018a, maskreg_14);
		io->set_iomap_alias_rw(0x018c, pit_14, 2);
		io->set_iomap_alias_w(0x018e, pit_14, 3);
//		io->set_iovalue_single_r(0x018e, 0x00); // INT0
//		io->set_iovalue_single_r(0x018e, 0x40); // INT41
		io->set_iovalue_single_r(0x018e, 0x80); // INT5
//		io->set_iovalue_single_r(0x018e, 0xc0); // INT6
	}
	
#if defined(SUPPORT_ITF_ROM)
	io->set_iomap_single_w(0x043d, memory);
#endif
#if !defined(SUPPORT_HIRESO)
	io->set_iomap_single_w(0x043f, memory); // ITF
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
#if !defined(_PC98XA)
	io->set_iomap_single_rw(0x0439, memory);
#endif
#if !defined(SUPPORT_HIRESO)
	io->set_iomap_single_rw(0x0461, memory);
	io->set_iomap_single_rw(0x0463, memory);
#else
	io->set_iomap_single_rw(0x0091, memory);
	io->set_iomap_single_rw(0x0093, memory);
#endif
	io->set_iomap_single_r(0x0567, memory);
#endif

#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM)
	io->set_iomap_single_w(0x053d, memory);
#endif
#if (defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)) && defined(SUPPORT_NEC_EMS)
	io->set_iomap_single_w(0x08e9, memory);
#endif	
	
#if !(defined(_PC9801) || defined(_PC9801E) || defined(SUPPORT_HIRESO))
	io->set_iomap_alias_rw(0x3fd9, pit, 0);
	io->set_iomap_alias_rw(0x3fdb, pit, 1);
	io->set_iomap_alias_rw(0x3fdd, pit, 2);
	io->set_iomap_alias_w (0x3fdf, pit, 3);
#endif
	
#if !defined(SUPPORT_HIRESO)
	io->set_iomap_alias_rw(0x7fd9, pio_mouse, 0);
	io->set_iomap_alias_rw(0x7fdb, pio_mouse, 1);
	io->set_iomap_alias_rw(0x7fdd, pio_mouse, 2);
	io->set_iomap_alias_w (0x7fdf, pio_mouse, 3);
	io->set_iomap_single_rw(0xbfdb, mouse);
#else
	io->set_iomap_alias_rw(0x0061, pio_mouse, 0);
	io->set_iomap_alias_rw(0x0063, pio_mouse, 1);
	io->set_iomap_alias_rw(0x0065, pio_mouse, 2);
	io->set_iomap_alias_rw(0x0067, pio_mouse, 3);
#endif
	// ToDo: MIDI@0xe0d0 - e0d3
	
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	pc88event = new EVENT(this, emu);
	pc88event->set_device_name(_T("Event Manager (PC-8801)"));
	pc88event->set_frames_per_sec(60);
	pc88event->set_lines_per_frame(260);
	
	pc88 = new PC88(this, emu);
	pc88->set_context_event_manager(pc88event);
	pc88sio = new I8251(this, emu);
	pc88sio->set_device_name(_T("8251 SIO (PC-8801)"));
	pc88sio->set_context_event_manager(pc88event);
	pc88pio = new I8255(this, emu);
	pc88pio->set_device_name(_T("8255 PIO (PC-8801)"));
	pc88pio->set_context_event_manager(pc88event);
	pc88pcm = new PCM1BIT(this, emu);
	pc88pcm->set_device_name(_T("1-Bit PCM Sound (PC-8801)"));
	pc88pcm->set_context_event_manager(pc88event);
	pc88rtc = new UPD1990A(this, emu);
	pc88rtc->set_device_name(_T("uPD1990A RTC (PC-8801)"));
	pc88rtc->set_context_event_manager(pc88event);
	pc88opn1 = new YM2203(this, emu);
#ifdef USE_DEBUGGER
	pc88opn1->set_context_debugger(new DEBUGGER(this, emu));
#endif
#ifdef SUPPORT_PC88_OPNA
	pc88opn1->set_device_name(_T("YM2608 OPNA (PC-8801)"));
	pc88opn1->is_ym2608 = true;
#else
	pc88opn1->set_device_name(_T("YM2203 OPN (PC-8801)"));
	pc88opn1->is_ym2608 = false;
#endif
	pc88opn1->set_context_event_manager(pc88event);
	pc88cpu = new Z80(this, emu);
	pc88cpu->set_device_name(_T("Z80 CPU (PC-8801)"));
	pc88cpu->set_context_event_manager(pc88event);
	
	if(config.printer_type == 0) {
		pc88prn = new PRNFILE(this, emu);
		pc88prn->set_context_event_manager(pc88event);
//	} else if(config.printer_type == 1) {
//		pc88prn = new PCPR201(this, emu);
//		pc88prn->set_context_event_manager(pc88event);
	} else {
		pc88prn = dummy;
	}
	
	pc88sub = new PC80S31K(this, emu);
	pc88sub->set_device_name(_T("PC-80S31K (PC-8801 Sub)"));
	pc88sub->set_context_event_manager(pc88event);
	pc88pio_sub = new I8255(this, emu);
	pc88pio_sub->set_device_name(_T("8255 PIO (PC-8801 Sub)"));
	pc88pio_sub->set_context_event_manager(pc88event);
	pc88fdc_sub = new UPD765A(this, emu);
	pc88fdc_sub->set_device_name(_T("uPD765A FDC (PC-8801 Sub)"));
	pc88fdc_sub->set_context_event_manager(pc88event);
	pc88noise_seek = new NOISE(this, emu);
	pc88noise_seek->set_context_event_manager(pc88event);
	pc88noise_head_down = new NOISE(this, emu);
	pc88noise_head_down->set_context_event_manager(pc88event);
	pc88noise_head_up = new NOISE(this, emu);
	pc88noise_head_up->set_context_event_manager(pc88event);
	pc88cpu_sub = new Z80(this, emu);
	pc88cpu_sub->set_device_name(_T("Z80 CPU (PC-8801 Sub)"));
	pc88cpu_sub->set_context_event_manager(pc88event);
	
	pc88event->set_context_cpu(pc88cpu, (config.cpu_type == 1) ? 3993624 : 7987248);
	pc88event->set_context_cpu(pc88cpu_sub, 3993624);
	pc88event->set_context_sound(pc88opn1);
	pc88event->set_context_sound(pc88pcm);
	pc88event->set_context_sound(pc88noise_seek);
	pc88event->set_context_sound(pc88noise_head_down);
	pc88event->set_context_sound(pc88noise_head_up);
	
	pc88->set_context_cpu(pc88cpu);
	pc88->set_context_opn1(pc88opn1);
	pc88->set_context_pcm(pc88pcm);
	pc88->set_context_pio(pc88pio);
	pc88->set_context_prn(pc88prn);
	pc88->set_context_rtc(pc88rtc);
	pc88->set_context_sio(pc88sio);
	pc88cpu->set_context_mem(pc88);
	pc88cpu->set_context_io(pc88);
	pc88cpu->set_context_intr(pc88);
#ifdef USE_DEBUGGER
	pc88cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	pc88opn1->set_context_irq(pc88, SIG_PC88_OPN1_IRQ, 1);
	pc88sio->set_context_rxrdy(pc88, SIG_PC88_USART_IRQ, 1);
	pc88sio->set_context_out(pc88, SIG_PC88_USART_OUT);
	
	pc88sub->set_context_cpu(pc88cpu_sub);
	pc88sub->set_context_fdc(pc88fdc_sub);
	pc88sub->set_context_pio(pc88pio_sub);
	pc88pio->set_context_port_a(pc88pio_sub, SIG_I8255_PORT_B, 0xff, 0);
	pc88pio->set_context_port_b(pc88pio_sub, SIG_I8255_PORT_A, 0xff, 0);
	pc88pio->set_context_port_c(pc88pio_sub, SIG_I8255_PORT_C, 0x0f, 4);
	pc88pio->set_context_port_c(pc88pio_sub, SIG_I8255_PORT_C, 0xf0, -4);
	pc88pio->clear_ports_by_cmdreg = true;
	pc88pio_sub->set_context_port_a(pc88pio, SIG_I8255_PORT_B, 0xff, 0);
	pc88pio_sub->set_context_port_b(pc88pio, SIG_I8255_PORT_A, 0xff, 0);
	pc88pio_sub->set_context_port_c(pc88pio, SIG_I8255_PORT_C, 0x0f, 4);
	pc88pio_sub->set_context_port_c(pc88pio, SIG_I8255_PORT_C, 0xf0, -4);
	pc88pio_sub->clear_ports_by_cmdreg = true;
	pc88fdc_sub->set_context_irq(pc88cpu_sub, SIG_CPU_IRQ, 1);
	pc88fdc_sub->set_context_noise_seek(pc88noise_seek);
	pc88fdc_sub->set_context_noise_head_down(pc88noise_head_down);
	pc88fdc_sub->set_context_noise_head_up(pc88noise_head_up);
	pc88cpu_sub->set_context_mem(pc88sub);
	pc88cpu_sub->set_context_io(pc88sub);
	pc88cpu_sub->set_context_intr(pc88sub);
#ifdef USE_DEBUGGER
	pc88cpu_sub->set_context_debugger(new DEBUGGER(this, emu));
#endif
#endif

	// initialize all devices
#if defined(__GIT_REPO_VERSION)
	strncpy(_git_revision, __GIT_REPO_VERSION, sizeof(_git_revision) - 1);
#endif
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	set_cpu_clock_with_switch((config.cpu_type != 0) ? 1 : 0);

#if defined(_PC9801) || defined(_PC9801E)
	fdc_2hd->get_disk_handler(0)->drive_num = 0;
	fdc_2hd->get_disk_handler(1)->drive_num = 1;
	fdc_2dd->get_disk_handler(0)->drive_num = 2;
	fdc_2dd->get_disk_handler(1)->drive_num = 3;
	fdc_sub->get_disk_handler(0)->drive_num = 4;
	fdc_sub->get_disk_handler(1)->drive_num = 5;
#elif defined(_PC9801VF) || defined(_PC9801U)
	fdc_2dd->get_disk_handler(0)->drive_num = 0;
	fdc_2dd->get_disk_handler(1)->drive_num = 1;
#elif defined(_PC98DO) || defined(_PC98DOPLUS)
	fdc->get_disk_handler(0)->drive_num = 0;
	fdc->get_disk_handler(1)->drive_num = 1;
	pc88fdc_sub->get_disk_handler(0)->drive_num = 2;
	pc88fdc_sub->get_disk_handler(1)->drive_num = 3;
#else
	fdc->get_disk_handler(0)->drive_num = 0;
	fdc->get_disk_handler(1)->drive_num = 1;
#endif
#if defined(USE_HARD_DISK)
	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
		if(!(config.last_hard_disk_path[drv][0] != _T('\0') && FILEIO::IsFileExisting(config.last_hard_disk_path[drv]))) {
#if defined(SUPPORT_SASI_IF)
			create_local_path(config.last_hard_disk_path[drv], _MAX_PATH, _T("SASI%d.DAT"), drv);
#endif
#if defined(SUPPORT_SCSI_IF)
			create_local_path(config.last_hard_disk_path[drv], _MAX_PATH, _T("SCSI%d.DAT"), drv);
#endif
#if defined(SUPPORT_IDE_IF)
			create_local_path(config.last_hard_disk_path[drv], _MAX_PATH, _T("IDE%d.DAT"), drv);
#endif
		}
	}
#endif
}

VM::~VM()
{
	// delete all devices
	for(DEVICE* device = first_device; device;) {
		DEVICE *next_device = device->next_device;
		device->release();
		delete device;
		device = next_device;
	}
}

DEVICE* VM::get_device(int id)
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(device->this_device_id == id) {
			return device;
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// drive virtual machine
// ----------------------------------------------------------------------------
void VM::set_cpu_clock_with_switch(int speed_type)
{
	uint32_t cpu_clocks = CPU_CLOCKS;
#if defined(_PC9801E)
	if(speed_type != 0) {
		// 8MHz -> 5MHz
		cpu_clocks = 4992030;
		pit_clock_8mhz = false;
	} else {
		pit_clock_8mhz = true;
	}
#elif defined(_PC9801VM) || defined(_PC98DO) || defined(_PC98DOPLUS) || defined(_PC9801VX) || defined(_PC98XL)
	if(speed_type != 0) {
		// 10MHz/16MHz -> 8MHz
		cpu_clocks = 7987248;
		pit_clock_8mhz = true;
	} else {
		pit_clock_8mhz = false;
	}
#elif defined(_PC9801RA) || defined(_PC98RL)
	if(speed_type != 0) {
		// 20MHz -> 16MHz
		cpu_clocks = 15974496;
		pit_clock_8mhz = true;
	} else {
		pit_clock_8mhz = false;
	}		
#endif
	uint32_t waitfactor;
	if(CPU_CLOCKS > cpu_clocks) {
		waitfactor = (uint32_t)(65536.0 * ((1.0 - (double)cpu_clocks / (double)CPU_CLOCKS)));
		//out_debug_log(_T("CLOCK=%d WAIT FACTOR=%d"), cpu_clocks, waitfactor);
	} else {
		waitfactor = 0;
		//out_debug_log(_T("CLOCK=%d WAIT FACTOR=%d"), cpu_clocks, waitfactor);
	}
	cpu->write_signal(SIG_CPU_WAIT_FACTOR, waitfactor, 0xffffffff);
	
	uint8_t prn_port_b = pio_prn->read_signal(SIG_I8255_PORT_B);
	prn_port_b &= (uint8_t)(~0x20);
	if(pit_clock_8mhz) {
		prn_port_b |= 0x20; // MOD, 1 = System clock 8MHz, 0 = 5/10MHz
	}
	pio_prn->write_signal(SIG_I8255_PORT_B, prn_port_b, 0xff);
	int pit_clocks = pit_clock_8mhz ? 1996812 : 2457600;
	// pit ch.2: rs-232c
	pit->set_constant_clock(0, pit_clocks);
	pit->set_constant_clock(1, pit_clocks);
	pit->set_constant_clock(2, pit_clocks);
}

void VM::set_wait(int dispmode, int clock)
{
	// by PC-9800 Technical Data Book (HARDWARE), ASCII, 1993.
	int io_wait       = 8;  // I/O
	int slotmem_wait  = 4;  // Extra RAM on extra bus.
	int exmem_wait    = 1;  // Extra RAM on extra memory slot.
	int exboards_wait = 8; // BANK 0C, 0D
	int introm_wait   = 0;  // INTERNAL ROM (BIOS, ITF)
	int bank08_wait   = 2/*8*/;
	int intram_wait   = 0;
	int gvram_wait    = intram_wait;
	int cpuclock      = 8000000;
	// TODO: INTA
#if defined(_PC9801RA) || defined(_PC9801RL)
	// PC-9801RA21
	intram_wait = 0;
	if(clock == 0) { // FAST CLOCK (20MHz)
		cpuclock = 20000000;
#if defined(_SUPPORT_HIRESO)		
		if(dispmode != 0) { // Low RESO
			exboards_wait = 12;
		} else {
			introm_wait = 6;
		}
#else
		exboards_wait = 12;
#endif
		io_wait = 10;
	} else {
		cpuclock = 16000000;
#if defined(_SUPPORT_HIRESO)		
		if(dispmode != 0) { // Low RESO
			exboards_wait = 10;
		} else {
			introm_wait = 4;
		}
#else
		exboards_wait = 10;
#endif
		io_wait = 8;
	}
	gvram_wait = intram_wait + 1; // OK?
	
#elif defined(_PC98XL2)
	// ToDo: V30
	cpuclock = 16000000;
	if(dispmode == 0) {
		intram_wait = 1;
		bank08_wait = 1;
		io_wait = 10;
		introm_wait = 6;
		slotmem_wait  = 6;
		exmem_wait    = 1;
		exboards_wait = 4;
		// inta_wait = 14;
	} else { // Normal
		intram_wait = 1;
		bank08_wait = 1;
		io_wait = 10;
		introm_wait = 1;
		slotmem_wait  = 6;
		exmem_wait    = 1;
		exboards_wait = 12;
		// inta_wait = 14;
	}
	gvram_wait = intram_wait + 1; // OK?
	
#elif defined(_PC98XL)
	if(dispmode == 0) {
		intram_wait = 1;
		bank08_wait = 1;
		io_wait = 3;
		introm_wait = 1;
		slotmem_wait  = 1;
		exmem_wait    = 1;
		exboards_wait = 4;
		cpuclock = 8000000;
		if(clock == 0) { // FAST CLOCK (10MHz)
			io_wait += 1;
			exboards_wait = 1;
			cpuclock = 10000000;
		}
		// inta_wait = 5;
	} else { // Normal
		intram_wait = 0;
		bank08_wait = 0;
		io_wait = 3;
		introm_wait = 0;
		slotmem_wait  = 0;
		exmem_wait    = 1;
		exboards_wait = 4;
		cpuclock = (clock == 0) ? 10000000 : 8000000;
	}
	gvram_wait = intram_wait + 1; // OK?
	
#elif defined(_PC9801VM21) || defined(_PC9801VX)
	// ToDo: V30
	// They are for 80286.
	if(clock == 0) { // FAST CLOCK (10MHz)
		cpuclock = 10000000;
		intram_wait = 0;
		bank08_wait = 5;
		io_wait = 4;
		introm_wait = 0;
		slotmem_wait  = 1;
		exmem_wait    = 1;
		exboards_wait = 5;
	} else { // SLOW (8MHz)
		cpuclock = 8000000;
		intram_wait = 0;
		io_wait = 3;
		bank08_wait = 4;
		introm_wait = 0;
		slotmem_wait  = 1;
		exmem_wait    = 1;
		exboards_wait = 4;
		// inta_wait = 5;
	}		
	gvram_wait = intram_wait + 1; // OK?
#elif defined(_PC9801U) || defined(_PC9801VF) || defined(_PC9801VM) || defined(_PC9801UV)
	if(clock == 0) { // FAST CLOCK (10MHz)
		cpuclock = 10000000;
		intram_wait = 1;
		io_wait = 3;
	} else { // SLOW (8MHz)
		cpuclock = 8000000;
		intram_wait = 0;
		io_wait = 2;
	}		
	slotmem_wait  = intram_wait;
	exmem_wait    = intram_wait;
	exboards_wait = intram_wait;
	introm_wait   = intram_wait;
	bank08_wait   = intram_wait;
	gvram_wait    = intram_wait; // OK?
#elif defined(_PC98XA)
	cpuclock = 8000000;
	intram_wait = 1;
	io_wait = 3;
	// inta_wait = 2;
	slotmem_wait  = intram_wait;
	exmem_wait    = intram_wait;
	exboards_wait = intram_wait;
	introm_wait   = intram_wait;
	bank08_wait   = intram_wait;
	gvram_wait    = intram_wait; // OK?
#elif defined(_PC9801E) || defined(_PC9801F) || defined(_PC9801M) || defined(_PC9801)
	// ToDo: Others.
	if(clock == 0) { // FAST CLOCK (8MHz)
		cpuclock = 8000000;
		intram_wait = 1;
		io_wait = 2;
	} else {
		cpuclock = 5000000;
		intram_wait = 0;
		io_wait = 1;
	}		
	slotmem_wait  = intram_wait;
	exmem_wait    = intram_wait;
	exboards_wait = intram_wait;
	introm_wait   = intram_wait;
	bank08_wait   = intram_wait;
	gvram_wait    = intram_wait; // OK?
#endif
	memory->write_signal(SIG_INTRAM_WAIT, intram_wait, 0xff);
	memory->write_signal(SIG_BANK08_WAIT, bank08_wait, 0xff);
	memory->write_signal(SIG_EXMEM_WAIT, exmem_wait, 0xff);
	memory->write_signal(SIG_SLOTMEM_WAIT, slotmem_wait, 0xff);
	memory->write_signal(SIG_EXBOARDS_WAIT, exboards_wait, 0xff);
	memory->write_signal(SIG_INTROM_WAIT, introm_wait, 0xff);
	// IO
	io->set_iowait_range_r(0x0000, 0xffff, io_wait);
	io->set_iowait_range_w(0x0000, 0xffff, io_wait);
	// ToDo: Wait factor
	int waitval;
	waitval = (int)round(((double)cpuclock) / (1.0e6 / 1.6));
	if(waitval < 1) waitval = 0;
	memory->write_signal(SIG_TVRAM_WAIT, waitval, 0xfffff); // OK?
	memory->write_signal(SIG_GVRAM_WAIT, gvram_wait, 0xfffff); // OK?
//	memory->write_signal(SIG_GVRAM_WAIT, waitval, 0xfffff); // OK?
}	

void VM::reset()
{
	// Set resolution before resetting.
	// initial device settings
	uint8_t port_a, port_b, port_c, port_b2;
#if defined(USE_MONITOR_TYPE) /*&& defined(SUPPORT_HIRESO)*/
#if !defined(SUPPORT_HIRESO)
	io->set_iovalue_single_r(0x0467, 0xfd); // Detect high-reso.
//	io->set_iovalue_single_r(0x0ca0, 0xff); // Detect high-reso.
#endif
	if(config.monitor_type == 0) {
#if defined(SUPPORT_HIRESO)
//		io->set_iovalue_single_r(0x0431, 0x00);
#else
//		io->set_iovalue_single_r(0x0431, 0x04);
#endif
		gdc_gfx->set_horiz_freq(24830);
		gdc_chr->set_horiz_freq(24830);
		
	} else { // WIP
//		io->set_iovalue_single_r(0x0431, 0x04); // bit2: 1 = Normal mode, 0 = Hireso mode
		gdc_gfx->set_horiz_freq(15750);
		gdc_chr->set_horiz_freq(15750);
	}
#else
//	io->set_iovalue_single_r(0x0431, 0x04);
#endif
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
#endif
	set_cpu_clock_with_switch((config.cpu_type != 0) ? 1 : 0);
	
#if defined(USE_MONITOR_TYPE)
	set_wait(config.monitor_type, config.cpu_type);
#else
	set_wait(0, config.cpu_type);
#endif	
	port_a  = 0x00;
//	port_a |= 0x80; // DIP SW 2-8, 1 = GDC 2.5MHz, 0 = GDC 5MHz
	port_a |= 0x40; // DIP SW 2-7, 1 = Do not control FD motor
	port_a |= 0x20; // DIP SW 2-6, 1 = Enable internal HD
//	port_a |= 0x10; // DIP SW 2-5, 1 = Initialize emory switch
//	port_a |= 0x08; // DIP SW 2-4, 1 = 20 lines, 0 = 25 lines
//	port_a |= 0x04; // DIP SW 2-3, 1 = 40 columns, 0 = 80 columns
	port_a |= 0x02; // DIP SW 2-2, 1 = BASIC mode, 0 = Terminal mode
	port_a |= 0x01; // DIP SW 2-1, 1 = Normal mode, 0 = LT mode
	if((config.dipswitch & (1 << DIPSWITCH_POSITION_GDC_FAST)) == 0) {
		port_a = port_a | 0x80;
	}
	if((config.dipswitch & (1 << DIPSWITCH_POSITION_NOINIT_MEMSW)) != 0) {
		port_a = port_a | 0x10;
	}
	gdc_gfx->set_clock_freq(((port_a & 0x80) != 0) ? (2500 * 1000) : (5000 * 1000));
	gdc_chr->set_clock_freq(((port_a & 0x80) != 0) ? (2500 * 1000) : (5000 * 1000));
	
	port_b  = 0x00;
	port_b |= 0x80; // RS-232C CI#, 1 = OFF
	port_b |= 0x40; // RS-232C CS#, 1 = OFF
	port_b |= 0x20; // RS-232C CD#, 1 = OFF
#if !defined(SUPPORT_HIRESO)
//	port_b |= 0x10; // INT3, 1 = Active, 0 = Inactive
	port_b |= (config.monitor_type == 0) ? 0x08 : 0x00; // DIP SW 1-1, 1 = Hiresolution CRT, 0 = Standard CRT
#else
	port_b |= 0x10; // SHUT0
	port_b |= 0x08; // SHUT1
#endif
//	port_b |= 0x04; // IMCK, 1 = Parity error occurs in internal RAM
//	port_b |= 0x02; // EMCK, 1 = Parity error occurs in external RAM
//	port_b |= 0x01; // CDAT
	
	port_c  = 0x00;
	port_c |= 0x80; // SHUT0
	port_c |= 0x40; // PSTBM, 1 = Mask printer's PSTB#
	port_c |= 0x20; // SHUT1
	port_c |= 0x10; // MCHKEN, 1 = Enable parity check for RAM
	port_c |= 0x08; // BUZ, 1 = Stop buzzer
	port_c |= 0x04; // TXRE, 1 = Enable IRQ from RS-232C 8251A TXRDY
	port_c |= 0x02; // TXEE, 1 = Enable IRQ from RS-232C 8251A TXEMPTY
	port_c |= 0x01; // RXRE, 1 = Enable IRQ from RS-232C 8251A RXRDY
	pio_sys->write_signal(SIG_I8255_PORT_A, port_a, 0xff);
	pio_sys->write_signal(SIG_I8255_PORT_B, port_b, 0xff);
	pio_sys->write_signal(SIG_I8255_PORT_C, port_c, 0xff);
	
	port_b2  = 0x00;
#if defined(_PC9801)
//	port_b |= 0x80; // TYP1, 0 = PC-9801 (first)
//	port_b |= 0x40; // TYP0, 0
#elif defined(_PC9801U)
	port_b2 |= 0x80; // TYP1, 1 = PC-9801U
	port_b2 |= 0x40; // TYP0, 1
#else
	port_b2 |= 0x80; // TYP1, 1 = Other PC-9801 series
//	port_b2 |= 0x40; // TYP0, 0
#endif
	if(pit_clock_8mhz) {
		port_b2 |= 0x20; // MOD, 1 = System clock 8MHz, 0 = 5/10MHz
	}
	port_b2 |= 0x10; // DIP SW 1-3, 1 = Don't use LCD
#if !defined(SUPPORT_16_COLORS) || defined(SUPPORT_EGC)
	#if defined(SUPPORT_EGC)
	if((config.dipswitch & (1 << DIPSWITCH_POSITION_EGC)) == 0) {
		port_b2 = port_b2 | 0x08;
	}
	#else
	port_b2 |= 0x08; // DIP SW 1-8, 1 = Standard graphic mode, 0 = Enhanced graphic mode
	#endif
#endif
	port_b2 |= 0x04; // Printer BUSY#, 1 = Inactive, 0 = Active (BUSY)
#if defined(HAS_V30) || defined(HAS_V33)
	port_b2 |= 0x02; // CPUT, 1 = V30/V33, 0 = 80x86
#endif
#if defined(_PC9801VF) || defined(_PC9801U)
	port_b2 |= 0x01; // VF, 1 = PC-9801VF/U
#endif
	pio_prn->write_signal(SIG_I8255_PORT_B, port_b2, 0xff);
	
	port_a  = 0xf0; // Clear mouse buttons and counters
	port_b  = 0x00;
#if defined(_PC98XA)
	port_b |= (uint8_t)((RAM_SIZE - 0x100000) / 0x40000);
#else
#if defined(SUPPORT_HIRESO)
	port_b |= 0x80; // DIP SW 1-4, 1 = External FDD #3/#4, 0 = #1/#2
#endif
	port_b |= ((config.dipswitch & (1 << DIPSWITCH_POSITION_RAM512K)) == 0) ? 0x40 : 0x00; // DIP SW 3-6, 1 = Enable internal RAM 80000h-9FFFFh
#if defined(_PC98RL)
	port_b |= 0x10; // DIP SW 3-3, 1 = DMA ch.0 for SASI-HDD
#endif
#if defined(USE_CPU_TYPE)
	if(config.cpu_type != 0) {
#if !defined(SUPPORT_HIRESO)
		port_b |= 0x02; // SPDSW, 1 = 10MHz, 0 = 12MHz
#else
		port_b |= 0x01; // SPDSW, 1 = 8/16MHz, 0 = 10/20MHz
#endif
	}
#endif
#endif
	port_c  = 0x00;
#if defined(SUPPORT_HIRESO)
	port_c |= (config.monitor_type == 0) ? 0x08 : 0x00; // DIP SW 1-1, 1 = Hiresolution CRT, 0 = Standard CRT
//	port_c |= 0x08; // MODSW, 1 = Normal Mode, 0 = Hirezo Mode
#endif
#if defined(HAS_V30) || defined(HAS_V33)
	port_c |= 0x04; // DIP SW 3-8, 1 = V30, 0 = 80x86
#endif
	pio_mouse->write_signal(SIG_I8255_PORT_A, port_a, 0xff);
	pio_mouse->write_signal(SIG_I8255_PORT_B, port_b, 0xff);
	pio_mouse->write_signal(SIG_I8255_PORT_C, port_c, 0xff);

#if defined(SUPPORT_320KB_FDD_IF)
	pio_fdd->write_signal(SIG_I8255_PORT_A, 0xff, 0xff);
	pio_fdd->write_signal(SIG_I8255_PORT_B, 0xff, 0xff);
	pio_fdd->write_signal(SIG_I8255_PORT_C, 0xff, 0xff);
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	fdc_2dd->write_signal(SIG_UPD765A_FREADY, 1, 1);	// 2DD FDC RDY is pulluped
#endif
	
	if(sound_type == 0 || sound_type == 1) {
//		opn->write_signal(SIG_YM2203_PORT_A, 0x3f, 0xff);	// PC-9801-26(K) INT0
//		opn->write_signal(SIG_YM2203_PORT_A, 0xbf, 0xff);	// PC-9801-26(K) INT41
		opn->write_signal(SIG_YM2203_PORT_A, 0xff, 0xff);	// PC-9801-26(K) INT5
//		opn->write_signal(SIG_YM2203_PORT_A, 0x7f, 0xff);	// PC-9801-26(K) INT6
	}
	
#if defined(SUPPORT_OLD_BUZZER)
	beep->write_signal(SIG_BEEP_ON, 1, 1);
	beep->write_signal(SIG_BEEP_MUTE, 1, 1);
#else
	beep->write_signal(SIG_PCM1BIT_ON, 1, 1);
	beep->write_signal(SIG_PCM1BIT_MUTE, 1, 1);
#endif
	
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	pc88opn1->set_reg(0x29, 3); // for Misty Blue
	pc88pio->write_signal(SIG_I8255_PORT_C, 0, 0xff);
	pc88pio_sub->write_signal(SIG_I8255_PORT_C, 0, 0xff);
#endif

}

void VM::run()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		pc88event->drive();
	} else
#endif
	event->drive();
}

double VM::get_frame_rate()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(config.boot_mode != 0) {
		return pc88event->get_frame_rate();
	} else
#endif
	return event->get_frame_rate();
}

// ----------------------------------------------------------------------------
// debugger
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
DEVICE *VM::get_cpu(int index)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(index == 0 && boot_mode == 0) {
		return cpu;
	} else if(index == 1 && boot_mode != 0) {
		return pc88cpu;
	} else if(index == 2 && boot_mode != 0) {
		return pc88cpu_sub;
	}
#else
	if(index == 0) {
		return cpu;
#if defined(SUPPORT_320KB_FDD_IF)
	} else if(index == 1) {
		return cpu_sub;
#endif
	}
#endif
	return NULL;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		pc88->draw_screen();
	} else
#endif
	display->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
#if defined(SUPPORT_OLD_BUZZER)
	beep->initialize_sound(rate, 2400, 8000);
#else
	beep->initialize_sound(rate, 8000);
#endif
	if(sound_type == 0 || sound_type == 1) {
		if(opn->is_ym2608) {
			opn->initialize_sound(rate, 7987248, samples, 0, 0);
		} else {
			opn->initialize_sound(rate, 3993624, samples, 0, 0);
		}
		fmsound->initialize_sound(rate, samples);
	} else if(sound_type == 2 || sound_type == 3) {
		tms3631->initialize_sound(rate, 8000);
	}
	
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	// init sound manager
	pc88event->initialize_sound(rate, samples);
	
	// init sound gen
	if(pc88opn1->is_ym2608) {
		pc88opn1->initialize_sound(rate, 7987248, samples, 0, 0);
	} else {
		pc88opn1->initialize_sound(rate, 3993624, samples, 0, 0);
	}
	pc88pcm->initialize_sound(rate, 8000);
#endif
}

uint16_t* VM::create_sound(int* extra_frames)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		return pc88event->create_sound(extra_frames);
	} else
#endif
	return event->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		return pc88event->get_sound_buffer_ptr();
	} else
#endif
	return event->get_sound_buffer_ptr();
}

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch-- == 0) {
		if(sound_type == 0 || sound_type == 1) {
			opn->set_volume(0, decibel_l, decibel_r);
		}
	} else if(ch-- == 0) {
		if(sound_type == 0 || sound_type == 1) {
			opn->set_volume(1, decibel_l, decibel_r);
		}
#if defined(SUPPORT_PC98_OPNA)
	} else if(ch-- == 0) {
		if(sound_type == 0 || sound_type == 1) {
			opn->set_volume(2, decibel_l, decibel_r);
		}
	} else if(ch-- == 0) {
		if(sound_type == 0 || sound_type == 1) {
			opn->set_volume(3, decibel_l, decibel_r);
		}
	} else if(ch-- == 0) {
		if(sound_type == 0 || sound_type == 1) {
			fmsound->set_volume(0, decibel_l, decibel_r);
		}
#endif
	} else if(ch-- == 0) {
		if(sound_type == 2 || sound_type == 3) {
			tms3631->set_volume(0, decibel_l, decibel_r);
		}
	} else if(ch-- == 0) {
		beep->set_volume(0, decibel_l, decibel_r);
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	} else if(ch-- == 0) {
		pc88opn1->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		pc88opn1->set_volume(1, decibel_l, decibel_r);
#if defined(SUPPORT_PC88_OPNA)
	} else if(ch-- == 0) {
		pc88opn1->set_volume(2, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		pc88opn1->set_volume(3, decibel_l, decibel_r);
#endif
	} else if(ch-- == 0) {
		pc88pcm->set_volume(0, decibel_l, decibel_r);
#endif
	} else if(ch-- == 0) {
		noise_seek->set_volume(0, decibel_l, decibel_r);
		noise_head_down->set_volume(0, decibel_l, decibel_r);
		noise_head_up->set_volume(0, decibel_l, decibel_r);
#if defined(_PC98DO) || defined(_PC98DOPLUS)
		pc88noise_seek->set_volume(0, decibel_l, decibel_r);
		pc88noise_head_down->set_volume(0, decibel_l, decibel_r);
		pc88noise_head_up->set_volume(0, decibel_l, decibel_r);
#endif
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		pc88->key_down(code, repeat);
	} else
#endif
	keyboard->key_down(code, repeat);
}

void VM::key_up(int code)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
//		pc88->key_up(code);
	} else
#endif
	keyboard->key_up(code);
}

bool VM::get_caps_locked()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		return pc88->get_caps_locked();
	} else
#endif
	return keyboard->get_caps_locked();
}

bool VM::get_kana_locked()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		return pc88->get_kana_locked();
	} else
#endif
	return keyboard->get_kana_locked();
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	UPD765A *controller = get_floppy_disk_controller(drv);
	
	if(controller != NULL) {
		controller->open_disk(drv & 1, file_path, bank);
	}
}

void VM::close_floppy_disk(int drv)
{
	UPD765A *controller = get_floppy_disk_controller(drv);
	
	if(controller != NULL) {
		controller->close_disk(drv & 1);
	}
}

bool VM::is_floppy_disk_inserted(int drv)
{
	DISK *handler = get_floppy_disk_handler(drv);
	
	if(handler != NULL) {
		return handler->inserted;
	}
	return false;
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
	DISK *handler = get_floppy_disk_handler(drv);
	
	if(handler != NULL) {
		handler->write_protected = value;
	}
}

bool VM::is_floppy_disk_protected(int drv)
{
	DISK *handler = get_floppy_disk_handler(drv);
	
	if(handler != NULL) {
		return handler->write_protected;
	}
	return false;
}

uint32_t VM::is_floppy_disk_accessed()
{
	uint32_t status = 0;
	
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
		status = pc88fdc_sub->read_signal(0) & 3;
	} else {
		status = fdc->read_signal(0) & 3;
	}
#else
	for(int drv = 0; drv < USE_FLOPPY_DISK; drv += 2) {
		UPD765A *controller = get_floppy_disk_controller(drv);
		
		if(controller != NULL) {
			status |= (controller->read_signal(0) & 3) << drv;
		}
	}
#endif
	return status;
}

UPD765A *VM::get_floppy_disk_controller(int drv)
{
#if defined(_PC9801) || defined(_PC9801E)
	if(drv == 0 || drv == 1) {
		return fdc_2hd;
	} else if(drv == 2 || drv == 3) {
		return fdc_2dd;
	} else if(drv == 4 || drv == 5) {
		return fdc_sub;
	}
#elif defined(_PC9801VF) || defined(_PC9801U)
	if(drv == 0 || drv == 1) {
		return fdc_2dd;
	}
#elif defined(_PC98DO) || defined(_PC98DOPLUS)
	if(drv == 0 || drv == 1) {
		return fdc;
	} else if(drv == 2 || drv == 3) {
		return pc88fdc_sub;
	}
#else
	if(drv == 0 || drv == 1) {
		return fdc;
	}
#endif
	return NULL;
}

DISK *VM::get_floppy_disk_handler(int drv)
{
	UPD765A *controller = get_floppy_disk_controller(drv);
	
	if(controller != NULL) {
		return controller->get_disk_handler(drv & 1);
	}
	return NULL;
}

#if defined(USE_HARD_DISK)
void VM::open_hard_disk(int drv, const _TCHAR* file_path)
{
	if(drv < USE_HARD_DISK) {
#if defined(SUPPORT_SASI_IF)
		sasi_hdd->open(drv, file_path, 256);
#endif
#if defined(SUPPORT_SCSI_IF)
		scsi_hdd[drv]->open(0, file_path, 512);
#endif
#if defined(SUPPORT_IDE_IF)
		ide_hdd[drv]->open(0, file_path, 512);
#endif
	}
}

void VM::close_hard_disk(int drv)
{
	if(drv < USE_HARD_DISK) {
#if defined(SUPPORT_SASI_IF)
		sasi_hdd->close(drv);
#endif
#if defined(SUPPORT_SCSI_IF)
		scsi_hdd[drv]->close(0);
#endif
#if defined(SUPPORT_IDE_IF)
		ide_hdd[drv]->close(0);
#endif
	}
}

bool VM::is_hard_disk_inserted(int drv)
{
	if(drv < USE_HARD_DISK) {
#if defined(SUPPORT_SASI_IF)
		return sasi_hdd->mounted(drv);
#endif
#if defined(SUPPORT_SCSI_IF)
		return scsi_hdd[drv]->mounted(0);
#endif
#if defined(SUPPORT_IDE_IF)
		return ide_hdd[drv]->mounted(0);
#endif
	}
	return false;
}

uint32_t VM::is_hard_disk_accessed()
{
	uint32_t status = 0;
	
	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
#if defined(SUPPORT_SASI_IF)
		if(sasi_hdd->accessed(drv)) {
			status |= 1 << drv;
		}
#endif
#if defined(SUPPORT_SCSI_IF)
		if(scsi_hdd[drv]->accessed(0)) {
			status |= 1 << drv;
		}
#endif
#if defined(SUPPORT_IDE_IF)
		if(ide_hdd[drv]->accessed(0)) {
			status |= 1 << drv;
		}
#endif
	}
	return status;
}
#endif

#if defined(USE_TAPE)
void VM::play_tape(int drv, const _TCHAR* file_path)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	pc88->play_tape(file_path);
#else
	cmt->play_tape(file_path);
#endif
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	pc88->rec_tape(file_path);
#else
	cmt->rec_tape(file_path);
#endif
}

void VM::close_tape(int drv)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	pc88->close_tape();
#else
	cmt->close_tape();
#endif
}

bool VM::is_tape_inserted(int drv)
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	return pc88->is_tape_inserted();
#else
	return cmt->is_tape_inserted();
#endif
}
#endif

bool VM::is_frame_skippable()
{
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != 0) {
//		return pc88event->is_frame_skippable();
		return pc88->is_frame_skippable();
	} else
#endif
	return event->is_frame_skippable();
}


void VM::update_config()
{
	set_cpu_clock_with_switch((config.cpu_type != 0) ? 1 : 0);
#if defined(USE_MONITOR_TYPE)
	set_wait(config.monitor_type, config.cpu_type);
#else
	set_wait(0, config.cpu_type);
#endif	
	{
		uint8_t mouse_port_b = pio_mouse->read_signal(SIG_I8255_PORT_B);
		mouse_port_b = mouse_port_b & ~0x40;
		if((config.dipswitch & (1 << DIPSWITCH_POSITION_RAM512K)) == 0) {
			mouse_port_b = mouse_port_b | 0x40;
		}
		pio_mouse->write_signal(SIG_I8255_PORT_B, mouse_port_b, 0xff);
	}
	{
		uint8_t sys_port_a = pio_sys->read_signal(SIG_I8255_PORT_A);
		//sys_port_a = sys_port_a & ~0x80;
		//if((config.dipswitch & (1 << DIPSWITCH_POSITION_GDC_FAST)) != 0) {
		//	sys_port_a = sys_port_a | 0x80;
		//}
		sys_port_a = sys_port_a & ~0x10;
		if((config.dipswitch & (1 << DIPSWITCH_POSITION_NOINIT_MEMSW)) != 0) {
			sys_port_a = sys_port_a | 0x10;
		}
		pio_sys->write_signal(SIG_I8255_PORT_A, sys_port_a, 0xff);
	}

	{
		uint8_t prn_port_b = pio_prn->read_signal(SIG_I8255_PORT_B);
#if defined(SUPPORT_EGC)
		prn_port_b = prn_port_b & ~0x08;
		if((config.dipswitch & (1 << DIPSWITCH_POSITION_EGC)) == 0) {
			prn_port_b = prn_port_b | 0x08;
		}
#endif
		pio_prn->write_signal(SIG_I8255_PORT_B, prn_port_b, 0xff);
	}
	
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	if(boot_mode != config.boot_mode) {
		// boot mode is changed !!!
		boot_mode = config.boot_mode;
		reset();
	} else
#endif
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

#define STATE_VERSION	17

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
 	for(DEVICE* device = first_device; device; device = device->next_device) {
		// Note: typeid(foo).name is fixed by recent ABI.Not dec 6.
 		// const char *name = typeid(*device).name();
		//       But, using get_device_name() instead of typeid(foo).name() 20181008 K.O
		const char *name = device->get_device_name();
		int len = strlen(name);
		if(!state_fio->StateCheckInt32(len)) {
			if(loading) {
				printf("Class name len Error: DEVID=%d EXPECT=%s\n", device->this_device_id, name);
			}
			return false;
		}
		if(!state_fio->StateCheckBuffer(name, len, 1)) {
			if(loading) {
				printf("Class name Error: DEVID=%d EXPECT=%s\n", device->this_device_id, name);
			}
 			return false;
 		}
		if(!device->process_state(state_fio, loading)) {
			if(loading) {
				printf("Data loading Error: DEVID=%d\n", device->this_device_id);
			}
 			return false;
 		}
 	}
	state_fio->StateValue(pit_clock_8mhz);
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	state_fio->StateValue(boot_mode);
#endif
	state_fio->StateValue(sound_type);
	if(loading) {
		set_cpu_clock_with_switch((config.cpu_type != 0) ? 1 : 0);
#if defined(USE_MONITOR_TYPE)
		set_wait(config.monitor_type, config.cpu_type);
#else
		set_wait(0, config.cpu_type);
#endif	
	}
 	return true;
}
