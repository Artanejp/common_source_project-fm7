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
#if defined(HAS_I386) || defined(HAS_I486SX) || defined(HAS_I486DX)
#include "../i386_np21.h"
//#include "../i386.h"
#elif defined(HAS_I286)
//#include "../i286_np21.h"
#include "../i286.h"
#else
#include "../i86.h"
#endif
#if defined(HAS_SUB_V30)
#include "../i86.h" // V30
#endif
#include "../io.h"
#include "../ls244.h"
#include "../memory.h"
#include "../midi.h"
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
#endif
#if defined(SUPPORT_SCSI_IF)
#include "scsi.h"
#endif
#if defined(SUPPORT_IDE_IF)
#include "ide.h"
#endif
#include "serial.h"

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
#ifdef SUPPORT_PC88_JAST
#include "../pcm8bit.h"
#endif
#ifdef SUPPORT_M88_DISKDRV
#include "../pc8801/diskio.h"
#endif
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
	int v30_clocks = 9984060;
#if defined(PIT_CLOCK_8MHZ)
	pit_clock_8mhz = true;
#else
	pit_clock_8mhz = false;
#endif
#if defined(_PC9801E)
	if(config.cpu_type == 1) {
		// 8MHz -> 5MHz
		cpu_clocks = 4992030;
		pit_clock_8mhz = false;
	}
#elif defined(_PC9801VM) || defined(_PC98DO) || defined(_PC98DOPLUS) || defined(_PC9801VX) || defined(_PC98XL)
	if(config.cpu_type == 1 || config.cpu_type == 3) {
		// 10MHz/16MHz -> 8MHz
		cpu_clocks = v30_clocks = 7987248;
		pit_clock_8mhz = true;
	}
#elif defined(_PC9801RA) || defined(_PC98RL)
	if(config.cpu_type == 1 || config.cpu_type == 3) {
		// 20MHz -> 16MHz
		cpu_clocks = 15974496;
		v30_clocks = 7987248;
		pit_clock_8mhz = true;
	}
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
	pic->num_chips = 2;
#if defined(HAS_I86)
	cpu = new I86(this, emu);
	cpu->device_model = INTEL_8086;
#elif defined(HAS_V30)
	cpu = new I86(this, emu);
	cpu->device_model = NEC_V30;
#elif defined(HAS_I286)
	cpu = new I286(this, emu);
//	cpu->device_model = INTEL_80286;
#elif defined(HAS_I386)
	cpu = new I386(this, emu);
	cpu->device_model = INTEL_80386;
#elif defined(HAS_I486SX)
	cpu = new I386(this, emu);
	cpu->device_model = INTEL_I486SX;
#elif defined(HAS_I486DX)
	cpu = new I386(this, emu);
	cpu->device_model = INTEL_I486DX;
#endif
#if defined(HAS_SUB_V30)
	v30 = new I86(this, emu);
	v30->device_model = NEC_V30;
#endif
	io = new IO(this, emu);
	io->space = 0x10000;
	io->bus_width = 16;
	rtcreg = new LS244(this, emu);
	rtcreg->set_device_name(_T("74LS244 (RTC)"));
//	memory = new MEMORY(this, emu);
	memory = new MEMBUS(this, emu);
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	memory->space = 0x1000000; // 16MB
#else
	memory->space = 0x0100000; // 1MB
#endif
#if defined(SUPPORT_32BIT_DATABUS)
	memory->bus_width = 32;
#else
	memory->bus_width = 16;
#endif
	memory->bank_size = 0x800;
	not_busy = new NOT(this, emu);
	not_busy->set_device_name(_T("NOT Gate (Printer Busy)"));
#if defined(HAS_I86) || defined(HAS_V30)
	not_prn = new NOT(this, emu);
	not_prn->set_device_name(_T("NOT Gate (Printer IRQ)"));
#endif
	rtc = new UPD1990A(this, emu);
#if defined(SUPPORT_2HD_FDD_IF)
#if /*defined(_PC9801) ||*/ defined(_PC9801E)
	if((config.dipswitch & DIPSWITCH_2HD) && FILEIO::IsFileExisting(create_local_path(_T("2HDIF.ROM")))) {
#endif
		fdc_2hd = new UPD765A(this, emu);
		fdc_2hd->set_device_name(_T("uPD765A FDC (2HD I/F)"));
#if /*defined(_PC9801) ||*/ defined(_PC9801E)
	} else {
		fdc_2hd = NULL;
	}
#endif
#endif
#if defined(SUPPORT_2DD_FDD_IF)
#if defined(_PC9801) || defined(_PC9801E)
	if((config.dipswitch & DIPSWITCH_2DD) && FILEIO::IsFileExisting(create_local_path(_T("2DDIF.ROM")))) {
#endif
		fdc_2dd = new UPD765A(this, emu);
		fdc_2dd->set_device_name(_T("uPD765A FDC (2DD I/F)"));
#if defined(_PC9801) || defined(_PC9801E)
	} else {
		fdc_2dd = NULL;
	}
#endif
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
	serial = new SERIAL(this, emu);
#if defined(SUPPORT_SASI_IF)
	sasi = new SASI(this, emu);
#endif
#if defined(SUPPORT_SCSI_IF)
	scsi = new SCSI(this, emu);
#endif
#if defined(SUPPORT_IDE_IF)
	ide = new IDE(this, emu);
#endif
	
#if defined(SUPPORT_320KB_FDD_IF)
	// 320kb fdd drives
	if((config.dipswitch & DIPSWITCH_2D) && (FILEIO::IsFileExisting(create_local_path(_T("DISK.ROM"))) || FILEIO::IsFileExisting(create_local_path(_T("PC88.ROM"))))) {
		pio_sub = new I8255(this, emu);
		pio_sub->set_device_name(_T("8255 PIO (320KB FDD)"));
		pc80s31k = new PC80S31K(this, emu);
		pc80s31k->set_device_name(_T("PC-80S31K (320KB FDD)"));
		fdc_sub = new UPD765A(this, emu);
		fdc_sub->set_device_name(_T("uPD765A FDC (320KB FDD)"));
		cpu_sub = new Z80(this, emu);
		cpu_sub->set_device_name(_T("Z80 CPU (320KB FDD)"));
	} else {
		pio_sub = NULL;
		pc80s31k = NULL;
		fdc_sub = NULL;
		cpu_sub = NULL;
	}
#endif
	
	/* IRQ	0  PIT
		1  KEYBOARD
		2  CRTV
		3  (INT0)
		4  RS-232C
		5  (INT1)
		6  (INT2) MOUSE (HIRESO)
		7  SLAVE PIC
		8  PRINTER (8086,V30) or FPU
		9  (INT3) PC-9801-27 (SASI), PC-9801-55 (SCSI), or IDE
		10 (INT41) FDC (640KB I/F)
		11 (INT42) FDC (1MB I/F)
		12 (INT5) PC-9801-26(K)/86 or PC-9801-14
		13 (INT6) MOUSE (STANDARD)
		14 
		15 (RESERVED)
	*/
	
	// set cpu device contexts
	event->set_context_cpu(cpu, cpu_clocks);
#if defined(HAS_SUB_V30)
	event->set_context_cpu(v30, v30_clocks);
#endif
#if defined(SUPPORT_320KB_FDD_IF)
	if(cpu_sub) {
		event->set_context_cpu(cpu_sub, 4000000);
	}
#endif
	
	// set sound device contexts
	event->set_context_sound(beep);
	if(sound_type == 0 || sound_type == 1) {
		event->set_context_sound(opn);
#if defined(SUPPORT_PC98_OPNA) && defined(SUPPORT_PC98_86PCM)
		event->set_context_sound(fmsound);
#endif
	} else if(sound_type == 2 || sound_type == 3) {
		event->set_context_sound(tms3631);
	}
	event->set_context_sound(noise_seek);
	event->set_context_sound(noise_head_down);
	event->set_context_sound(noise_head_up);
	
	// set other device contexts
#if (defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)) && defined(HAS_SUB_V30)
	dma->set_context_cpu(cpureg);
#else
	dma->set_context_cpu(cpu);
#endif
	dma->set_context_memory(memory);
	// dma ch.0: sasi
	// dma ch.1: memory refresh
#if defined(SUPPORT_2HD_FDD_IF)
	if(fdc_2hd) {
		dma->set_context_ch2(fdc_2hd);
		dma->set_context_tc2(fdc_2hd, SIG_UPD765A_TC, 1);
	}
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	if(fdc_2dd) {
		dma->set_context_ch3(fdc_2dd);
		dma->set_context_tc3(fdc_2dd, SIG_UPD765A_TC, 1);
	}
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
	sio_rs->set_context_rxrdy(serial, SIG_SERIAL_RXR, 1);
	sio_rs->set_context_txempty(serial, SIG_SERIAL_TXE, 1);
	sio_rs->set_context_txrdy(serial, SIG_SERIAL_TXR, 1);
	if(config.serial_type == 2) {
		MIDI *midi = new MIDI(this, emu);
		sio_rs->set_context_out(midi, SIG_MIDI_OUT);
		midi->set_context_in(sio_rs, SIG_I8251_RECV, 0xff);
	}
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
//	pio_sys->set_context_port_b(pio_sys, SIG_I8255_PORT_C, 0x10, +3); // SHUT0
//	pio_sys->set_context_port_b(pio_sys, SIG_I8255_PORT_C, 0x08, +2); // SHUT1
	pio_sys->set_context_port_c(pio_sys, SIG_I8255_PORT_B, 0x80, -3); // SHUT0
	pio_sys->set_context_port_c(pio_sys, SIG_I8255_PORT_B, 0x20, -2); // SHUT1
#endif
	// sysport port.c bit6: printer strobe
#if defined(SUPPORT_OLD_BUZZER)
	pio_sys->set_context_port_c(beep, SIG_BEEP_MUTE, 0x08, 0);
#else
	pio_sys->set_context_port_c(beep, SIG_PCM1BIT_MUTE, 0x08, 0);
#endif
	pio_sys->set_context_port_c(serial, SIG_SERIAL_PORT_C, 0x07, 0);
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
#if (defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)) && defined(HAS_SUB_V30)
	pic->set_context_cpu(cpureg);
#else
	pic->set_context_cpu(cpu);
#endif
	rtc->set_context_dout(pio_sys, SIG_I8255_PORT_B, 1);
	
	if(sound_type == 0 || sound_type == 1) {
		opn->set_context_irq(pic, SIG_I8259_CHIP1 | SIG_I8259_IR4, 1);
		opn->set_context_port_b(joystick, SIG_JOYSTICK_SELECT, 0xc0, 0);
		fmsound->set_context_opn(opn);
#if defined(SUPPORT_PC98_OPNA) && defined(SUPPORT_PC98_86PCM)
		fmsound->set_context_pic(pic);
#endif
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
#if defined(HAS_SUB_V30)
	cpureg->set_context_v30(v30);
	cpureg->set_context_pio(pio_prn);
	cpureg->cpu_mode = (config.cpu_type == 2 || config.cpu_type == 3);
#endif
#endif
	display->set_context_pic(pic);
	display->set_context_gdc_chr(gdc_chr);
	display->set_context_gdc_gfx(gdc_gfx);
	dmareg->set_context_dma(dma);
	keyboard->set_context_sio(sio_kbd);
	memory->set_context_display(display);
	mouse->set_context_pic(pic);
	mouse->set_context_pio(pio_mouse);
	serial->set_context_pic(pic);
	
#if defined(SUPPORT_2HD_FDD_IF)
	if(fdc_2hd) {
		fdc_2hd->set_context_irq(floppy, SIG_FLOPPY_2HD_IRQ, 1);
		fdc_2hd->set_context_drq(floppy, SIG_FLOPPY_2HD_DRQ, 1);
		fdc_2hd->set_context_noise_seek(noise_seek);
		fdc_2hd->set_context_noise_head_down(noise_head_down);
		fdc_2hd->set_context_noise_head_up(noise_head_up);
		fdc_2hd->raise_irq_when_media_changed = true;
	}
	floppy->set_context_fdc_2hd(fdc_2hd);
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	if(fdc_2dd) {
		fdc_2dd->set_context_irq(floppy, SIG_FLOPPY_2DD_IRQ, 1);
		fdc_2dd->set_context_drq(floppy, SIG_FLOPPY_2DD_DRQ, 1);
		fdc_2dd->set_context_noise_seek(noise_seek);
		fdc_2dd->set_context_noise_head_down(noise_head_down);
		fdc_2dd->set_context_noise_head_up(noise_head_up);
		fdc_2dd->raise_irq_when_media_changed = true;
	}
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
//	sasi_host->set_context_irq(sasi, SIG_SASI_IRQ, 1);
//	sasi_host->set_context_drq(sasi, SIG_SASI_DRQ, 1);
	sasi_host->set_context_bsy(sasi, SIG_SASI_BSY, 1);
	sasi_host->set_context_cd (sasi, SIG_SASI_CXD, 1);
	sasi_host->set_context_io (sasi, SIG_SASI_IXO, 1);
	sasi_host->set_context_msg(sasi, SIG_SASI_MSG, 1);
	sasi_host->set_context_req(sasi, SIG_SASI_REQ, 1);
	sasi_host->set_context_ack(sasi, SIG_SASI_ACK, 1);
#ifdef _PC98XA
//	dma->set_context_ch3(sasi_host);
	dma->set_context_ch3(sasi);
	dma->set_context_tc3(sasi, SIG_SASI_TC, 1);
#else
//	dma->set_context_ch0(sasi_host);
	dma->set_context_ch0(sasi);
	dma->set_context_tc0(sasi, SIG_SASI_TC, 1);
#endif
	sasi->set_context_host(sasi_host);
	sasi->set_context_hdd(sasi_hdd);
	sasi->set_context_dma(dma);
	sasi->set_context_pic(pic);
#if !defined(SUPPORT_HIRESO)
	sasi->set_context_pio(pio_sys);
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
//	sio_cmt->set_context_txempty(cmt, SIG_CMT_TXEMP, 1);
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
#if defined(HAS_SUB_V30)
	v30->set_context_mem(memory);
	v30->set_context_io(io);
	v30->set_context_intr(pic);
#ifdef SINGLE_MODE_DMA
	v30->set_context_dma(dma);
#endif
#ifdef USE_DEBUGGER
	v30->set_context_debugger(new DEBUGGER(this, emu));
#endif
#endif
	
#if defined(SUPPORT_320KB_FDD_IF)
	// 320kb fdd drives
	if(pc80s31k && pio_sub && fdc_sub && cpu_sub) {
		pc80s31k->set_context_cpu(cpu_sub);
		pc80s31k->set_context_fdc(fdc_sub);
		pc80s31k->set_context_pio(pio_sub);
		pio_fdd->set_context_port_a(pio_sub, SIG_I8255_PORT_B, 0xff, 0);
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
	}
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
	
	io->set_iomap_alias_rw(0x0031, pio_sys, 0);
	io->set_iomap_alias_rw(0x0033, pio_sys, 1);
	io->set_iomap_alias_rw(0x0035, pio_sys, 2);
	io->set_iomap_alias_w (0x0037, pio_sys, 3);
	
	io->set_iomap_alias_rw(0x0040, pio_prn, 0);
	io->set_iomap_alias_rw(0x0042, pio_prn, 1);
	io->set_iomap_alias_rw(0x0044, pio_prn, 2);
	io->set_iomap_alias_w (0x0046, pio_prn, 3);
	
	io->set_iomap_alias_rw(0x0041, sio_kbd, 0);
	io->set_iomap_alias_rw(0x0043, sio_kbd, 1);
	
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	io->set_iomap_single_w(0x0050, cpureg);
	io->set_iomap_single_w(0x0052, cpureg);
#endif
	
#if defined(SUPPORT_320KB_FDD_IF)
	io->set_iomap_alias_rw(0x0051, pio_fdd, 0);
	io->set_iomap_alias_rw(0x0053, pio_fdd, 1);
	io->set_iomap_alias_rw(0x0055, pio_fdd, 2);
	io->set_iomap_alias_w (0x0057, pio_fdd, 3);
#endif
	
	io->set_iomap_alias_rw(0x0060, gdc_chr, 0);
	io->set_iomap_alias_rw(0x0062, gdc_chr, 1);
	
	io->set_iomap_single_w(0x0064, display);
	io->set_iomap_single_w(0x0068, display);
#if defined(SUPPORT_16_COLORS)
	io->set_iomap_single_w(0x006a, display);
#endif
#if !defined(SUPPORT_HIRESO)
	io->set_iomap_single_w(0x006c, display);
	io->set_iomap_single_w(0x006e, display);
#endif
	io->set_iomap_single_w(0x0070, display);
	io->set_iomap_single_w(0x0072, display);
	io->set_iomap_single_w(0x0074, display);
	io->set_iomap_single_w(0x0076, display);
	io->set_iomap_single_w(0x0078, display);
	io->set_iomap_single_w(0x007a, display);
#if defined(SUPPORT_GRCG)
#if !defined(SUPPORT_HIRESO)
	io->set_iomap_single_w(0x007c, display);
	io->set_iomap_single_w(0x007e, display);
#else
	io->set_iomap_single_w(0x00a4, display);
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
	
//	io->set_iomap_single_w(0x00a1, display);
//	io->set_iomap_single_w(0x00a3, display);
//	io->set_iomap_single_w(0x00a5, display);
	io->set_iomap_single_rw(0x00a1, display);
	io->set_iomap_single_rw(0x00a3, display);
	io->set_iomap_single_rw(0x00a5, display);
	io->set_iomap_single_rw(0x00a9, display);
#if defined(SUPPORT_EGC)
	io->set_iomap_range_rw(0x04a0, 0x04af, display);
#endif
	
	io->set_iomap_alias_rw(0x0071, pit, 0);
	io->set_iomap_alias_rw(0x0073, pit, 1);
	io->set_iomap_alias_rw(0x0075, pit, 2);
	io->set_iomap_alias_w (0x0077, pit, 3);
	
#if defined(SUPPORT_2HD_FDD_IF)
	if(fdc_2hd) {
#endif
		io->set_iomap_single_rw(0x0090, floppy);
		io->set_iomap_single_rw(0x0092, floppy);
		io->set_iomap_single_rw(0x0094, floppy);
		io->set_iomap_single_rw(0x0096, floppy);
#if defined(SUPPORT_2HD_FDD_IF)
	}
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
#if !defined(SUPPORT_HIRESO)
	io->set_iomap_single_rw(0x00be, floppy);
#else
//#if !defined(_PC98XA) && !defined(_PC98XL)
	io->set_iomap_single_w (0x00be, floppy);
//#endif
#endif
#endif
#if !defined(SUPPORT_HIRESO)
#if defined(SUPPORT_2DD_FDD_IF)
	if(fdc_2dd) {
#endif
		io->set_iomap_single_rw(0x00c8, floppy);
		io->set_iomap_single_rw(0x00ca, floppy);
		io->set_iomap_single_rw(0x00cc, floppy);
		io->set_iomap_single_rw(0x00ce, floppy);
#if defined(SUPPORT_2DD_FDD_IF)
	}
#endif
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
#if defined(SUPPORT_32BIT_ADDRESS)
	io->set_iomap_single_rw(0x00f6, cpureg);
#endif
	
	if(sound_type == 0 || sound_type == 1) {
		io->set_iomap_single_rw(0x0188, fmsound);
		io->set_iomap_single_rw(0x018a, fmsound);
#if defined(SUPPORT_PC98_OPNA)
		io->set_iomap_single_rw(0x018c, fmsound);
		io->set_iomap_single_rw(0x018e, fmsound);
		io->set_iomap_single_rw(0xa460, fmsound);
#if defined(SUPPORT_PC98_86PCM)
		io->set_iomap_single_rw(0xa462, fmsound); // dummy
		io->set_iomap_single_rw(0xa464, fmsound); // dummy
		io->set_iomap_single_rw(0xa466, fmsound);
		io->set_iomap_single_rw(0xa468, fmsound);
		io->set_iomap_single_rw(0xa46a, fmsound);
		io->set_iomap_single_rw(0xa46c, fmsound);
		io->set_iomap_single_rw(0xa66e, fmsound);
#endif
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
	
//#if !defined(SUPPORT_HIRESO)
//	io->set_iovalue_single_r(0x0431, 0x04); // bit2: 1 = Normal mode, 0 = Hireso mode
//#else
//	io->set_iovalue_single_r(0x0431, 0x00);
//#endif
#if defined(SUPPORT_ITF_ROM)
	io->set_iomap_single_w(0x043d, memory);
#endif
#if !defined(SUPPORT_HIRESO)
	io->set_iomap_single_w(0x043f, memory);
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
#if defined(SUPPORT_32BIT_ADDRESS)
	io->set_iomap_single_w(0x053d, memory);
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
	
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	// create devices
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
#ifdef SUPPORT_PC88_OPN1
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
#endif
	pc88cpu = new Z80(this, emu);
	pc88cpu->set_device_name(_T("Z80 CPU (PC-8801)"));
	pc88cpu->set_context_event_manager(pc88event);
	
	if(config.printer_type == 0) {
		pc88prn = new PRNFILE(this, emu);
		pc88prn->set_context_event_manager(pc88event);
//	} else if(config.printer_type == 1) {
//		pc88prn = new PCPR201(this, emu);
//		pc88prn->set_context_event_manager(pc88event);
#ifdef SUPPORT_PC88_JAST
	} else if(config.printer_type == 2) {
		pc88prn = new PCM8BIT(this, emu);
		pc88prn->set_context_event_manager(pc88event);
#endif
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
	
#ifdef SUPPORT_M88_DISKDRV
	if(config.dipswitch & DIPSWITCH_M88_DISKDRV) {
		pc88diskio = new DiskIO(this, emu);
		pc88diskio->set_context_event_manager(pc88event);
	} else {
		pc88diskio = NULL;
	}
#endif
	
	// set cpu device contexts
	pc88event->set_context_cpu(pc88cpu, (config.cpu_type == 1) ? 3993624 : 7987248);
	pc88event->set_context_cpu(pc88cpu_sub, 3993624);
	
	// set sound device contexts
	pc88event->set_context_sound(pc88pcm);
#ifdef SUPPORT_PC88_OPN1
	if(pc88opn1 != NULL) {
		pc88event->set_context_sound(pc88opn1);
	}
#endif
#ifdef SUPPORT_PC88_JAST
	if(config.printer_type == 2) {
		pc88event->set_context_sound(pc88prn);
	}
#endif
	pc88event->set_context_sound(pc88noise_seek);
	pc88event->set_context_sound(pc88noise_head_down);
	pc88event->set_context_sound(pc88noise_head_up);
	
	// set other device contexts
	pc88->set_context_cpu(pc88cpu);
	pc88->set_context_pcm(pc88pcm);
	pc88->set_context_pio(pc88pio);
	pc88->set_context_prn(pc88prn);
	pc88->set_context_rtc(pc88rtc);
	pc88->set_context_sio(pc88sio);
#ifdef SUPPORT_PC88_OPN1
	if(pc88opn1 != NULL) {
		pc88->set_context_opn1(pc88opn1);
	}
#endif
#ifdef SUPPORT_M88_DISKDRV
	if(config.dipswitch & DIPSWITCH_M88_DISKDRV) {
		pc88->set_context_diskio(pc88diskio);
	}
#endif
	pc88cpu->set_context_mem(pc88);
	pc88cpu->set_context_io(pc88);
	pc88cpu->set_context_intr(pc88);
#ifdef USE_DEBUGGER
	pc88cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	pc88sio->set_context_rxrdy(pc88, SIG_PC88_USART_IRQ, 1);
	pc88sio->set_context_out(pc88, SIG_PC88_USART_OUT);
#ifdef SUPPORT_PC88_OPN1
	if(pc88opn1 != NULL) {
		pc88opn1->set_context_irq(pc88, SIG_PC88_OPN1_IRQ, 1);
	}
#endif
	
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
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
#if defined(_PC9801)
		// 8086-5MHz
		memory->set_wait_rw(0x000000, 0x0fffff, 0); // RAM/ROM
		io->set_iowait_range_rw(0x0000, 0xffff, 1);
#elif defined(_PC9801E)
	if(config.cpu_type == 0) {
		// 8086-8MHz
		memory->set_wait_rw(0x000000, 0x0fffff, 1); // RAM/ROM
		io->set_iowait_range_rw(0x0000, 0xffff, 2);
	} else {
		// 8086-5MHz
		memory->set_wait_rw(0x000000, 0x0fffff, 0); // RAM/ROM
		io->set_iowait_range_rw(0x0000, 0xffff, 1);
	}
#elif defined(_PC9801U) || defined(_PC9801VF)
		// V30-8MHz
		memory->set_wait_rw(0x000000, 0x0fffff, 1); // RAM/ROM
		io->set_iowait_range_rw(0x0000, 0xffff, 2);
#elif defined(_PC9801VM) || defined(_PC98DO)
	if(config.cpu_type == 0) {
		// V30-10MHz
		memory->set_wait_rw(0x000000, 0x0fffff, 1); // RAM/ROM
		io->set_iowait_range_rw(0x0000, 0xffff, 3);
	} else {
		// V30-8MHz
		memory->set_wait_rw(0x000000, 0x0fffff, 1); // RAM/ROM
		io->set_iowait_range_rw(0x0000, 0xffff, 2);
	}
#elif defined(_PC9801VX)
	if(config.cpu_type == 0) {
		// 80286-10MHz
		memory->set_wait_rw(0x000000, 0x0bffff, 0); // RAM
		memory->set_wait_rw(0x0c0000, 0x0dffff, 5); // ROM (0C,0D)
		memory->set_wait_rw(0x0e0000, 0x0e7fff, 0); // RAM
		memory->set_wait_rw(0x0e8000, 0x0fffff, 0); // ROM (STD)
		memory->set_wait_rw(0x100000, 0xffffff, 1); // RAM (EXP)
		io->set_iowait_range_rw(0x0000, 0xffff, 4);
	} else if(config.cpu_type == 1) {
		// 80286-8MHz
		memory->set_wait_rw(0x000000, 0x0bffff, 0); // RAM
		memory->set_wait_rw(0x0c0000, 0x0dffff, 4); // ROM (0C,0D)
		memory->set_wait_rw(0x0e0000, 0x0e7fff, 0); // RAM
		memory->set_wait_rw(0x0e8000, 0x0fffff, 0); // ROM (STD)
		memory->set_wait_rw(0x100000, 0xffffff, 1); // RAM (EXP)
		io->set_iowait_range_rw(0x0000, 0xffff, 3);
	} else if(config.cpu_type == 2) {
		// V30-10MHz
		memory->set_wait_rw(0x000000, 0x0bffff, 1); // RAM
		memory->set_wait_rw(0x0c0000, 0x0dffff, 3); // ROM (0C,0D)
		memory->set_wait_rw(0x0e0000, 0x0e7fff, 1); // RAM
		memory->set_wait_rw(0x0e8000, 0x0fffff, 1); // ROM (STD)
		io->set_iowait_range_rw(0x0000, 0xffff, 3);
	} else {
		// V30-8MHz
		memory->set_wait_rw(0x000000, 0x0bffff, 1); // RAM
		memory->set_wait_rw(0x0c0000, 0x0dffff, 2); // ROM (0C,0D)
		memory->set_wait_rw(0x0e0000, 0x0e7fff, 1); // RAM
		memory->set_wait_rw(0x0e8000, 0x0fffff, 1); // ROM (STD)
		io->set_iowait_range_rw(0x0000, 0xffff, 2);
	}
#elif defined(_PC9801RA)
	if(config.cpu_type == 0) {
		// 80386-20MHz
		memory->set_wait_rw(0x000000, 0x0bffff,  0); // RAM
		memory->set_wait_rw(0x0c0000, 0x0dffff, 12); // ROM (0C,0D)
		memory->set_wait_rw(0x0e0000, 0x0e7fff,  0); // RAM
		memory->set_wait_rw(0x0e8000, 0x0fffff,  0); // ROM (STD)
		memory->set_wait_rw(0x100000, 0xffffff,  0); // RAM (EXP)
		io->set_iowait_range_rw(0x0000, 0xffff, 10);
	} else if(config.cpu_type == 1) {
		// 80386-16MHz
		memory->set_wait_rw(0x000000, 0x0bffff,  0); // RAM
		memory->set_wait_rw(0x0c0000, 0x0dffff, 10); // ROM (0C,0D)
		memory->set_wait_rw(0x0e0000, 0x0e7fff,  0); // RAM
		memory->set_wait_rw(0x0e8000, 0x0fffff,  0); // ROM (STD)
		memory->set_wait_rw(0x100000, 0xffffff,  1); // RAM (EXP)
		io->set_iowait_range_rw(0x0000, 0xffff,  8);
	} else if(config.cpu_type == 2) {
		// V30-10MHz
		memory->set_wait_rw(0x000000, 0x0bffff, 1); // RAM
		memory->set_wait_rw(0x0c0000, 0x0dffff, 3); // ROM (0C,0D)
		memory->set_wait_rw(0x0e0000, 0x0e7fff, 1); // RAM
		memory->set_wait_rw(0x0e8000, 0x0fffff, 1); // ROM (STD)
		io->set_iowait_range_rw(0x0000, 0xffff, 3);
	} else {
		// V30-8MHz
		memory->set_wait_rw(0x000000, 0x0bffff, 1); // RAM
		memory->set_wait_rw(0x0c0000, 0x0dffff, 2); // ROM (0C,0D)
		memory->set_wait_rw(0x0e0000, 0x0e7fff, 1); // RAM
		memory->set_wait_rw(0x0e8000, 0x0fffff, 1); // ROM (STD)
		io->set_iowait_range_rw(0x0000, 0xffff, 2);
	}
#elif defined(_PC98XA)
		// 80286-8MHz
		memory->set_wait_rw(0x000000, 0xffffff, 1); // RAM/ROM
		io->set_iowait_range_rw(0x0000, 0xffff, 3);
#elif defined(_PC98XL)
	if(config.cpu_type == 0) {
		// 80286-10MHz
		memory->set_wait_rw(0x000000, 0xffffff, 1); // RAM/ROM
		io->set_iowait_range_rw(0x0000, 0xffff, 4); // I/O
	} else if(config.cpu_type == 1) {
		// 80286-8MHz
		memory->set_wait_rw(0x000000, 0xffffff, 1); // RAM/ROM
		io->set_iowait_range_rw(0x0000, 0xffff, 3); // I/O
	}
#elif defined(_PC98XL2)
		// 80386-16MHz
		memory->set_wait_rw(0x000000, 0xffffff,  1); // RAM/ROM
		io->set_iowait_range_rw(0x0000, 0xffff, 10); // I/O
#elif defined(_PC98RL)
	if(config.cpu_type == 0) {
		// 80386-20MHz
		memory->set_wait_rw(0x000000, 0xffffff,  0); // RAM/ROM
		io->set_iowait_range_rw(0x0000, 0xffff, 10); // I/O
	} else {
		// 80386-16MHz
		memory->set_wait_rw(0x000000, 0x0fffff,  0); // RAM/ROM
		memory->set_wait_rw(0x100000, 0xffffff,  1); // RAM (EXP)
		io->set_iowait_range_rw(0x0000, 0xffff,  8); // I/O
	}
#endif
#if defined(HAS_SUB_V30)
	if(config.cpu_type == 2 || config.cpu_type == 3) {
		cpu->write_signal(SIG_CPU_BUSREQ,  1, 1);
	} else {
		v30->write_signal(SIG_CPU_BUSREQ,  1, 1);
	}
#endif
#if defined(_PC9801) || defined(_PC9801E)
	if(fdc_2hd) {
		fdc_2hd->get_disk_handler(0)->drive_num = 0;
		fdc_2hd->get_disk_handler(1)->drive_num = 1;
	}
	if(fdc_2dd) {
		fdc_2dd->get_disk_handler(0)->drive_num = 2;
		fdc_2dd->get_disk_handler(1)->drive_num = 3;
	}
	if(fdc_sub) {
		fdc_sub->get_disk_handler(0)->drive_num = 4;
		fdc_sub->get_disk_handler(1)->drive_num = 5;
	}
#elif defined(_PC9801VF) || defined(_PC9801U)
	if(fdc_2dd) {
		fdc_2dd->get_disk_handler(0)->drive_num = 0;
		fdc_2dd->get_disk_handler(1)->drive_num = 1;
	}
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

void VM::reset()
{
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
#endif
	
	// initial device settings
	uint8_t port_a, port_b, port_c;
	
	port_a  = 0xf0; // Clear mouse buttons and counters
	port_b  = 0x00;
#if defined(_PC98XA)
	port_b |= (uint8_t)((RAM_SIZE - 0x100000) / 0x40000);
#else
#if defined(SUPPORT_HIRESO)
	port_b |= 0x80; // DIP SW 1-4, 1 = External FDD #3/#4, 0 = #1/#2
#endif
	port_b |= 0x40; // DIP SW 3-6, 1 = Enable internal RAM 80000h-9FFFFh
#if defined(_PC98RL)
	port_b |= 0x10; // DIP SW 3-3, 1 = DMA ch.0 for SASI-HDD
#endif
#if defined(USE_CPU_TYPE)
	if(config.cpu_type == 1 || config.cpu_type == 3) {
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
//	port_c |= 0x08; // MODSW, 1 = Normal Mode, 0 = Hireso Mode
#endif
#if defined(HAS_SUB_V30)
	if(config.cpu_type == 2 || config.cpu_type == 3) {
		port_c |= 0x04; // DIP SW 3-8, 1 = V30, 0 = 80x86
	}
#endif
	pio_mouse->write_signal(SIG_I8255_PORT_A, port_a, 0xff);
	pio_mouse->write_signal(SIG_I8255_PORT_B, port_b, 0xff);
	pio_mouse->write_signal(SIG_I8255_PORT_C, port_c, 0xff);
	
#if 1
	port_a  = ~(config.dipswitch >> 16) & 0xff;
#else
	port_a  = 0x00;
	port_a |= 0x80; // DIP SW 2-8, 1 = GDC 2.5MHz, 0 = GDC 5MHz
	port_a |= 0x40; // DIP SW 2-7, 1 = Do not control FD motor
	port_a |= 0x20; // DIP SW 2-6, 1 = Enable internal HD
//	port_a |= 0x10; // DIP SW 2-5, 1 = Initialize memory switch
//	port_a |= 0x08; // DIP SW 2-4, 1 = 20 lines, 0 = 25 lines
//	port_a |= 0x04; // DIP SW 2-3, 1 = 40 columns, 0 = 80 columns
	port_a |= 0x02; // DIP SW 2-2, 1 = BASIC mode, 0 = Terminal mode
	port_a |= 0x01; // DIP SW 2-1, 1 = Normal mode, 0 = LT mode
#endif
	port_b  = 0x00;
	port_b |= 0x80; // RS-232C CI#, 1 = OFF
	if(config.serial_type == SERIAL_TYPE_DEFAULT) {
		// no device connected
		port_b |= 0x40; // RS-232C CS#, 1 = OFF
		port_b |= 0x20; // RS-232C CD#, 1 = OFF
	}
#if !defined(SUPPORT_HIRESO)
//	port_b |= 0x10; // INT3, 1 = Active, 0 = Inactive
	if(config.monitor_type == 0) {
		port_b |= 0x08; // DIP SW 1-1, 1 = Hiresolution CRT, 0 = Standard CRT
	}
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
	
	port_b  = 0x00;
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
	if(pit_clock_8mhz) {
		port_b |= 0x20; // MOD, 1 = System clock 8MHz, 0 = 5/10MHz
	}
	port_b |= 0x10; // DIP SW 1-3, 1 = Don't use LCD
#if !defined(SUPPORT_16_COLORS)
	port_b |= 0x08; // DIP SW 1-8, 1 = Standard graphic mode, 0 = Enhanced graphic mode
#endif
	port_b |= 0x04; // Printer BUSY#, 1 = Inactive, 0 = Active (BUSY)
#if !defined(SUPPORT_HIRESO)
#if (defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)) && defined(HAS_SUB_V30)
	if(cpureg->cpu_mode) {
		port_b |= 0x02; // CPUT, 1 = V30/V33, 0 = 80x86
	}
#elif defined(HAS_V30) || defined(HAS_V33)
	port_b |= 0x02; // CPUT, 1 = V30/V33, 0 = 80x86
#endif
#endif
#if defined(_PC9801VF) || defined(_PC9801U)
	port_b |= 0x01; // VF, 1 = PC-9801VF/U
#endif
	pio_prn->write_signal(SIG_I8255_PORT_B, port_b, 0xff);
	
#if defined(SUPPORT_320KB_FDD_IF)
	pio_fdd->write_signal(SIG_I8255_PORT_A, 0xff, 0xff);
	pio_fdd->write_signal(SIG_I8255_PORT_B, 0xff, 0xff);
	pio_fdd->write_signal(SIG_I8255_PORT_C, 0xff, 0xff);
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	if(fdc_2dd) {
		fdc_2dd->write_signal(SIG_UPD765A_FREADY, 1, 1);	// 2DD FDC RDY is pulluped
	}
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
#ifdef SUPPORT_PC88_OPN1
	if(pc88opn1 != NULL) {
		pc88opn1->set_reg(0x29, 3); // for Misty Blue
	}
#endif
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
#if (defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)) && defined(HAS_SUB_V30)
		if(cpureg->cpu_mode) {
			return NULL;
		}
#endif
		return cpu;
	} else if(index == 1) {
#if (defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)) && defined(HAS_SUB_V30)
		if(cpureg->cpu_mode) {
			return v30;
		}
		return NULL;
#elif defined(SUPPORT_320KB_FDD_IF)
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
#if defined(SUPPORT_PC98_OPNA) && defined(SUPPORT_PC98_86PCM)
			fmsound->initialize_sound(rate, samples, 8000);
#endif
		} else {
			opn->initialize_sound(rate, 3993624, samples, 0, 0);
		}
	} else if(sound_type == 2 || sound_type == 3) {
		tms3631->initialize_sound(rate, 8000);
	}
	
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	// init sound manager
	pc88event->initialize_sound(rate, samples);
	
	// init sound gen
	pc88pcm->initialize_sound(rate, 8000);
#ifdef SUPPORT_PC88_OPN1
	if(pc88opn1 != NULL) {
		if(pc88opn1->is_ym2608) {
			pc88opn1->initialize_sound(rate, 7987248, samples, 0, 0);
		} else {
			pc88opn1->initialize_sound(rate, 3993624, samples, 0, 0);
		}
	}
#endif
#ifdef SUPPORT_PC88_JAST
	if(config.printer_type == 2) {
		PCM8BIT *pcm8 = (PCM8BIT *)pc88prn;
		pcm8->initialize_sound(rate, 32000);
	}
#endif
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
#if defined(SUPPORT_PC98_86PCM)
	} else if(ch-- == 0) {
		if(sound_type == 0 || sound_type == 1) {
			fmsound->set_volume(0, decibel_l, decibel_r);
		}
#endif
#endif
	} else if(ch-- == 0) {
		if(sound_type == 2 || sound_type == 3) {
			tms3631->set_volume(0, decibel_l, decibel_r);
		}
	} else if(ch-- == 0) {
		beep->set_volume(0, decibel_l, decibel_r);
#if defined(_PC98DO) || defined(_PC98DOPLUS)
#ifdef SUPPORT_PC88_OPN1
	} else if(ch-- == 0) {
		if(pc88opn1 != NULL) {
			pc88opn1->set_volume(0, decibel_l, decibel_r);
		}
	} else if(ch-- == 0) {
		if(pc88opn1 != NULL) {
			pc88opn1->set_volume(1, decibel_l, decibel_r);
		}
#if defined(SUPPORT_PC88_OPNA)
	} else if(ch-- == 0) {
		if(pc88opn1 != NULL) {
			pc88opn1->set_volume(2, decibel_l, decibel_r);
		}
	} else if(ch-- == 0) {
		if(pc88opn1 != NULL) {
			pc88opn1->set_volume(3, decibel_l, decibel_r);
		}
#endif
#endif
#ifdef SUPPORT_PC88_JAST
	} else if(ch-- == 0) {
		if(config.printer_type == 2) {
			PCM8BIT *pcm8 = (PCM8BIT *)pc88prn;
			pcm8->set_volume(0, decibel_l, decibel_r);
		}
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

#if defined(_PC9801) || defined(_PC9801E)
bool VM::is_floppy_disk_connected(int drv)
{
	DISK *handler = get_floppy_disk_handler(drv);
	
	return (handler != NULL);
}
#endif

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

#define STATE_VERSION	20

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		const _TCHAR *name = char_to_tchar(typeid(*device).name() + 6); // skip "class "
		int len = (int)_tcslen(name);
		
		if(!state_fio->StateCheckInt32(len)) {
			return false;
		}
		if(!state_fio->StateCheckBuffer(name, len, 1)) {
			return false;
		}
		if(!device->process_state(state_fio, loading)) {
			return false;
		}
	}
	state_fio->StateValue(pit_clock_8mhz);
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	state_fio->StateValue(boot_mode);
#endif
	state_fio->StateValue(sound_type);
	return true;
}

