/*
	FUJITSU FM-Towns Emulator 'eFMTowns'

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ virtual machine ]
	History:
		2016-12-28 Copy from eFMR-50.
*/

#include "fmtowns.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

//#include "../hd46505.h"
#include "../i8251.h"
#include "../i8253.h"
#include "../i8259.h"

#include "../i386_np21.h"
//#include "../i386.h"

#include "../io.h"
#include "../mb8877.h"
#include "../msm58321.h"
#include "../noise.h"
#include "../pcm1bit.h"
#include "../harddisk.h"
#include "../scsi_hdd.h"
#include "../scsi_host.h"
//#include "./towns_scsi_host.h"
#include "../upd71071.h"

#include "./cdrom.h"
#include "./cmos.h"
#include "./crtc.h"
#include "./dictionary.h"
#include "./dmac.h"
#include "./towns_memory.h"
#include "./sprite.h"
#include "./sysrom.h"
#include "./vram.h"

// Electric Volume
#include "mb87078.h"
//YM-2612 "OPN2"
//#include "../ym2612.h"
//RF5C68 PCM
#include "rf5c68.h"
//AD7820 ADC
#include "ad7820kr.h"
#include "ym2612.h"
// 80387?

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "./adpcm.h"
//#include "./cdc.h"
#include "./floppy.h"
#include "./fontroms.h"
#include "./joystick.h"
#include "./joypad_2btn.h"
#include "./joypad_6btn.h"
#include "./keyboard.h"
#include "./mouse.h"
#include "./msdosrom.h"
#include "./scsi.h"
#include "./serialrom.h"
#include "./timer.h"
#include "./iccard.h"

#include "./planevram.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------
using FMTOWNS::ADPCM;
//using FMTOWNS::CDC;
using FMTOWNS::CMOS;
using FMTOWNS::DICTIONARY;
using FMTOWNS::FLOPPY;
using FMTOWNS::FONT_ROMS;
using FMTOWNS::JOYSTICK;
using FMTOWNS::JOYPAD_2BTN;
using FMTOWNS::JOYPAD_6BTN;

using FMTOWNS::KEYBOARD;
using FMTOWNS::MOUSE;
using FMTOWNS::MSDOSROM;
using FMTOWNS::SCSI;
using FMTOWNS::SERIAL_ROM;
using FMTOWNS::SYSROM;
using FMTOWNS::TIMER;
using FMTOWNS::TOWNS_ICCARD;

using FMTOWNS::TOWNS_CDROM;
using FMTOWNS::TOWNS_CRTC;
using FMTOWNS::TOWNS_DMAC;
using FMTOWNS::TOWNS_MEMORY;
//using FMTOWNS::TOWNS_SCSI_HOST;
using FMTOWNS::TOWNS_SPRITE;
using FMTOWNS::TOWNS_VRAM;
using FMTOWNS::PLANEVRAM;


VM::VM(EMU_TEMPLATE* parent_emu) : VM_TEMPLATE(parent_emu)
{
/*
	Machine ID & CPU ID

	FMR-50FD/HD/LT	0xF8
	FMR-50FX/HX	0xE0
	FMR-50SFX/SHX	0xE8
	FMR-50LT	0xF8
	FMR-50NBX	0x28
	FMR-50NB	0x60
	FMR-50NE/T	0x08
	FMR-CARD	0x70

	80286		0x00
	80386		0x01
	80386SX		0x03
	80486		0x02
*/

	// create devices
	//first_device = last_device = nullptr;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
#if defined(_USE_QT)
	dummy->set_device_name(_T("1st Dummy"));
	event->set_device_name(_T("EVENT"));
#endif

	cpu = new I386(this, emu);
#if defined(_USE_QT)
  #if defined(HAS_I386)
	cpu->set_device_name(_T("CPU(i386)"));
  #elif defined(HAS_I486)
	cpu->set_device_name(_T("CPU(i486)"));
  #elif defined(HAS_PENTIUM)
	cpu->set_device_name(_T("CPU(Pentium)"));
  #endif
#endif

	io = new IO(this, emu);
	io->space = _IO_SPACE;
	io->bus_width = _IO_BUS_WIDTH;

	crtc = new TOWNS_CRTC(this, emu);
	cdrom = new TOWNS_CDROM(this, emu);

	memory = new TOWNS_MEMORY(this, emu);
	//memory->space = _MEMORY_SPACE;
	//memory->bank_size = _MEMORY_BANK_SIZE;
	//memory->bus_width = _MEMORY_BUS_WIDTH;

	vram = new TOWNS_VRAM(this, emu);
	sprite = new TOWNS_SPRITE(this, emu);
	sysrom = new SYSROM(this, emu);
	msdosrom = new MSDOSROM(this, emu);
	fontrom = new FONT_ROMS(this, emu);
	dictionary = new DICTIONARY(this, emu);
	cmos     = new CMOS(this, emu);
#ifdef USE_DEBUGGER
	cmos->set_context_debugger(new DEBUGGER(this, emu));
#endif

#if defined(HAS_20PIX_FONTS)
	fontrom_20pix = new FONT_ROM_20PIX(this, emu);
#endif
	serialrom = new SERIAL_ROM(this, emu);

	adpcm = new ADPCM(this, emu);
//	mixer = new MIXER(this, emu); // Pseudo mixer.

	planevram = new PLANEVRAM(this, emu);

	adc = new AD7820KR(this, emu);
	rf5c68 = new RF5C68(this, emu);
	e_volumes[0] = new MB87078(this, emu);
	e_volumes[1] = new MB87078(this, emu);

	sio = new I8251(this, emu);
	pit0 = new I8253(this, emu);
	pit0->device_model = INTEL_8253;

	pit1 = new I8253(this, emu);
	pit1->device_model = INTEL_8253;

	pic = new I8259(this, emu);
	pic->num_chips = 2;
	fdc = new MB8877(this, emu);
	rtc = new MSM58321(this, emu);
	beep = new PCM1BIT(this, emu);
	opn2 = new YM2612(this, emu);

	seek_sound = new NOISE(this, emu);
	head_up_sound = new NOISE(this, emu);
	head_down_sound = new NOISE(this, emu);

//	scsi_host = new TOWNS_SCSI_HOST(this, emu);
	scsi_host = new SCSI_HOST(this, emu);

	for(int i = 0; i < 7; i++) {
		scsi_hdd[i] = nullptr;
	}
#if defined(USE_HARD_DISK)
	for(int i = 0; i < USE_HARD_DISK; i++) {
		scsi_hdd[i] = new SCSI_HDD(this, emu);
		scsi_hdd[i]->set_device_name(_T("SCSI Hard Disk Drive #%d"), i + 1);
		scsi_hdd[i]->scsi_id = i ;
		scsi_hdd[i]->set_disk_handler(0, new HARDDISK(emu));
		scsi_hdd[i]->set_context_interface(scsi_host);
		my_sprintf_s(scsi_hdd[i]->vendor_id, 9, "FUJITSU");
		my_sprintf_s(scsi_hdd[i]->product_id, 17, "SCSI-HDD");
		scsi_host->set_context_target(scsi_hdd[i]);
	}
#endif
	dma = new TOWNS_DMAC(this, emu);
	extra_dma = new TOWNS_DMAC(this, emu);

	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	joystick = new JOYSTICK(this, emu);
	scsi = new SCSI(this, emu);
	timer = new TIMER(this, emu);

	iccard1 = new TOWNS_ICCARD(this, emu);
#if 0
	iccard2 = new TOWNS_ICCARD(this, emu);
#else
	iccard2 = nullptr;
#endif
	for(int i = 0; i < 2; i++) {
		joypad_2btn[i] = new JOYPAD_2BTN(this, emu);
		joypad_6btn[i] = new JOYPAD_6BTN(this, emu);
		mouse[i] = new MOUSE(this, emu);
	}

	uint16_t machine_id = 0x0100; // FM-Towns1
	uint16_t cpu_id = 0x0001;     // i386DX
	uint32_t cpu_clock = 16000 * 1000; // 16MHz
#if defined(_FMTOWNS1_2ND_GEN)
	machine_id = 0x0200;   // 1F/2F/1H/2H
#elif defined(_FMTOWNS_UX_VARIANTS)
	machine_id = 0x0300;   // UX10/20/40
	cpu_id = 0x0003;       // i386SX
#elif defined(_FMTOWNS1_3RD_GEN)
	machine_id = 0x0400;  // 10F/20F.40H/80H
#elif defined(_FMTOWNS2_CX_VARIANTS)
	machine_id = 0x0500;  // CX10/20/30/40
#elif defined(_FMTOWNS_UG_VARIANTS)
	machine_id = 0x0600;  // UG10/20/40/80
	cpu_id = 0x0003;      // i386SX
	cpu_clock = 20000 * 1000; // 20MHz
#elif defined(_FMTOWNS_HR_VARIANTS)
	machine_id = 0x0700;
	cpu_id = 0x0002;      // i486SX
	cpu_clock = 20000 * 1000; // 20MHz
#elif defined(_FMTOWNS_HG_VARIANTS)
	machine_id = 0x0800;
	cpu_clock = 20000 * 1000; // 20MHz
#elif defined(_FMTOWNS_SG_VARIANTS)
	machine_id = 0x0800; // OK?
#elif defined(_FMTOWNS_SR_VARIANTS)
	machine_id = 0x0700; // OK?
	cpu_id = 0x0002;      // i486SX
	cpu_clock = 20000 * 1000; // 20MHz
#elif defined(_FMTOWNS_UR_VARIANTS)
	machine_id = 0x0900;  // UR10/20/40/80
	cpu_id = 0x0002;      // i486DX
	cpu_clock = 20000 * 1000; // ToDo: Correct frequency.
#elif defined(_FMTOWNS_MA_VARIANTS)
	machine_id = 0x0b00; // OK?
	cpu_id = 0x0002;      // i486SX
	cpu_clock = 33000 * 1000; // 33MHz
#elif defined(_FMTOWNS_ME_VARIANTS)
	machine_id = 0x0d00; // OK?
	cpu_id = 0x0002;      // i486SX
	cpu_clock = 25000 * 1000; // 25MHz
#elif defined(_FMTOWNS_MF_VARIANTS)
	machine_id = 0x0f00; // OK?
	cpu_id = 0x0002;      // i486SX
	cpu_clock = 33000 * 1000; // 33MHz
#elif defined(_FMTOWNS_MX_VARIANTS)
	machine_id = 0x0c00; // OK?
	cpu_id = 0x0002;      // i486DX (With FPU?)
	cpu_clock = 66000 * 1000; // 66MHz
#elif defined(_FMTOWNS_HC_VARIANTS)
	// 20210227 K.O
	// From FMTowns::MachineID()  of TSUGARU,
	// git 83d4ec2309ac9fcbb8c01f26061ff0d49c5321e4.
	machine_id = 0x1100; // OK?
	cpu_id = 0x0002;      // Pentium (With FPU?)
	cpu_clock = 50000 * 1000; // ToDo: Correctness frequency.
#else
	// ToDo: Pentium Model (After HB).

#endif

	event->set_frames_per_sec(FRAMES_PER_SEC);
	event->set_lines_per_frame(LINES_PER_FRAME);

	set_machine_type(machine_id, cpu_id);
	// set contexts
	event->set_context_cpu(cpu, cpu_clock);

	adc_in_ch = -1;
	line_in_ch = -1;
	modem_in_ch = -1;
	mic_in_ch = -1;

	// Use pseudo mixer instead of event.Due to using ADC.
	// Temporally not use mixer.
	event->set_context_sound(beep);
	event->set_context_sound(opn2);
	event->set_context_sound(rf5c68);
	event->set_context_sound(cdrom);

	fdc->set_context_noise_seek(seek_sound);
	fdc->set_context_noise_head_down(head_down_sound);
	fdc->set_context_noise_head_up(head_up_sound);
	event->set_context_sound(seek_sound);
	event->set_context_sound(head_down_sound);
	event->set_context_sound(head_up_sound);

#ifdef USE_DEBUGGER
	pit0->set_context_debugger(new DEBUGGER(this, emu));
	pit1->set_context_debugger(new DEBUGGER(this, emu));
#endif
	pit0->set_context_ch0(timer, SIG_TIMER_CH0, 1);
	pit0->set_context_ch1(timer, SIG_TIMER_CH1, 1);
	pit0->set_context_ch2(beep,  SIG_PCM1BIT_SIGNAL, 1);
	pit0->set_constant_clock(0, 307200);
	pit0->set_constant_clock(1, 307200);
	pit0->set_constant_clock(2, 307200);
	pit1->set_constant_clock(0, 1229900);
	pit1->set_constant_clock(1, 1229900);
	pit1->set_constant_clock(2, 1229900);
	pic->set_context_cpu(cpu);
	//pic->set_context_cpu(memory);

	fdc->set_context_irq(floppy, SIG_FLOPPY_IRQ, 1);
	//fdc->set_context_irq(dma, SIG_TOWNS_DMAC_EOT_CH0, 1);

	rtc->set_context_data(timer, SIG_TIMER_RTC, 0x0f, 0);
	rtc->set_context_busy(timer, SIG_TIMER_RTC_BUSY, 0x80);
	scsi_host->set_context_irq(scsi, SIG_SCSI_IRQ, 1);
	scsi_host->set_context_drq(scsi, SIG_SCSI_DRQ, 1);
	scsi_host->set_context_drq(keyboard, SIG_KEYBOARD_BOOTSEQ_END, 1);
	//scsi_host->set_context_ack(dma, SIG_TOWNS_DMAC_ACKREQ_CH1, 0xffffffff);

#ifdef USE_DEBUGGER
	dma->set_context_debugger(new DEBUGGER(this, emu));
	extra_dma->set_context_debugger(new DEBUGGER(this, emu));
#endif
	// Note: DMAC may set wait value to CPU. 20230409 K.O
	dma->set_context_cpu(NULL);
	//dma->set_context_cpu(cpu);
	dma->set_context_memory(memory);
	dma->set_primary_dmac(true);
	// BASE CLOCK is 2MHz * 8.
	//dma->set_dmac_clock(((double)cpu_clock) / 8.0, 8);
	dma->set_dmac_clock(16.0e6, 8);
	dma->set_context_ch0(fdc);
	// This is workaround for FM-Towns's SCSI.
	dma->set_force_16bit_transfer(1, false);
	dma->set_context_ch1(scsi_host);
	//dma->set_context_ch2(printer);
	dma->set_context_ch3(cdrom);
	dma->set_context_mask_bit(cdrom, SIG_TOWNS_CDROM_DMAMASK, 3);

	//extra_dma->set_context_cpu(cpu);
	extra_dma->set_context_cpu(NULL);
	extra_dma->set_context_memory(memory);
	extra_dma->set_primary_dmac(false);
	// BASE CLOCK is 2MHz * 8.
	//extra_dma->set_dmac_clock(((double)cpu_clock) / 8.0, 8);
	extra_dma->set_dmac_clock(16.0e6, 8);

	//dma->set_context_tc1(scsi, SIG_SCSI_EOT, 0xffffffff);
	dma->set_context_tc3(cdrom, SIG_TOWNS_CDROM_DMAINT, 0xffffffff);

	dma->set_context_ube(1, scsi_host, SIG_SCSI_16BIT_BUS, 0x02);
	//dma->set_context_ack(1, scsi_host, SIG_SCSI_ACK, 0xffffffff);
	//dma->set_context_ack(3, cdrom, SIG_TOWNS_CDROM_DMAACK, 0xffffffff);
	dma->set_context_child_dma(extra_dma);

	floppy->set_context_fdc(fdc);

	sprite->set_context_vram(vram);
//	sprite->set_context_font(fontrom);
//	sprite->set_context_crtc(crtc);
#ifdef USE_DEBUGGER
	sprite->set_context_debugger(new DEBUGGER(this, emu));
#endif

	planevram->set_context_vram(vram);
	planevram->set_context_sprite(sprite);
	planevram->set_context_crtc(crtc);

	crtc->set_context_sprite(sprite);
	crtc->set_context_vram(vram);
	crtc->set_context_font(fontrom);

//	e_volumes[0]->set_context_device(0, line_in, 0,
//									 MB87078_TYPE_MASK_LEFT);
//	e_volumes[0]->set_context_device(1, line_in, 0,
//									 MB87078_TYPE_MASK_RIGHT);
	e_volumes[1]->set_context_device(0, cdrom, 0,
									 MB87078_TYPE_SET_LEFT, SIG_TOWNS_CDROM_MUTE_L,
									 0xffffffff,
									 0xffffffff,
									 false
		);
	e_volumes[1]->set_context_device(1, cdrom, 0,
									 MB87078_TYPE_SET_RIGHT, SIG_TOWNS_CDROM_MUTE_R,
									 0xffffffff,
									 0xffffffff,
									 false
		);

	memory->set_context_cpu(cpu);
	memory->set_context_dmac(dma);
	memory->set_context_vram(vram);
	memory->set_context_planevram(planevram);
	memory->set_context_crtc(crtc);
	memory->set_context_system_rom(sysrom);
	memory->set_context_msdos(msdosrom);
	memory->set_context_dictionary(dictionary);
	memory->set_context_cmos(cmos);
	memory->set_context_font_rom(fontrom);
	memory->set_context_timer(timer);
	memory->set_context_serial_rom(serialrom);
	memory->set_context_sprite(sprite);
	memory->set_context_pcm(adpcm);
	memory->set_context_iccard(iccard1, 0);
	memory->set_context_iccard(iccard2, 1);

	adpcm->set_context_opn2(opn2);
	adpcm->set_context_rf5c68(rf5c68);
	adpcm->set_context_adc(adc);

	rf5c68->set_context_interrupt_boundary(adpcm, SIG_ADPCM_WRITE_INTERRUPT, 0xffffffff);
#ifdef USE_DEBUGGER
	rf5c68->set_context_debugger(new DEBUGGER(this, emu));
#endif
	opn2->set_context_irq(adpcm, SIG_ADPCM_OPX_INTR, 0xffffffff);

	adc->set_sample_rate(19200);
	adc->set_sound_bank(-1);
	adc->set_context_interrupt(adpcm, SIG_ADPCM_ADC_INTR, 0xffffffff);

	scsi->set_context_dma(dma);
	scsi->set_context_host(scsi_host);
	scsi->set_context_pic(pic);
	timer->set_context_pcm(beep);
	timer->set_context_rtc(rtc);
	//timer->set_context_halt_line(cpu, SIG_CPU_HALTREQ, 0xffffffff);
	timer->set_context_halt_line(cpu, SIG_CPU_BUSREQ, 0xffffffff);

	for(int i = 0; i < 2; i++) {
		// type =
		// 0: Towns PAD 2buttons
		// 1: Towns PAD 6buttons
		// 2: Towns MOUSE
		// 3: Analog Pad (reserved)
		// 4: Libble Rabble stick (reserved)
		joystick->set_context_joystick(i, joypad_2btn[i]);
		joystick->set_context_joystick(i, joypad_6btn[i]);
		joystick->set_context_joystick(i, mouse[i]);
	}
#ifdef USE_DEBUGGER
	joystick->set_context_debugger(new DEBUGGER(this, emu));
#endif
	// ToDo: Selective by config.
	for(int i = 0; i < 2; i++) {
		joypad_2btn[i]->set_context_pad_num(i);
		joypad_2btn[i]->set_context_parent_port(i, joystick, 0, 0xff);
		joypad_2btn[i]->set_negative_logic(true);
		joypad_2btn[i]->set_enable(true);
	}
	for(int i = 0; i < 2; i++) {
		joypad_6btn[i]->set_context_pad_num(i);
		joypad_6btn[i]->set_context_parent_port(i, joystick, 0, 0xff);
		joypad_6btn[i]->set_negative_logic(true);
		joypad_6btn[i]->set_enable(false);
	}
	for(int i = 0; i < 2; i++) {
		mouse[i]->set_context_pad_num(i);
		mouse[i]->set_context_parent_port(i, joystick, 0, 0xff);
		mouse[i]->set_negative_logic(true);
		mouse[i]->set_enable(false);
	}
	joystick->set_using_pad(0, -1);
	joystick->set_using_pad(1, -1);

	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
#ifdef SINGLE_MODE_DMA
	cpu->set_context_dma(dma);
#endif
	cpu->set_context_bios(nullptr);
	cpu->set_context_extreset(memory, SIG_FMTOWNS_NOTIFY_RESET, 0xffffffff);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	// Interrupts
	// IRQ0  : TIMER
	// IRQ1  : KEYBOARD
	// IRQ2  : USART (ToDo)
	// IRQ3  : EXTRA USART (ToDo)
	// IRQ4  : EXTRA I/O (Maybe not implement)
	// IRQ5  : EXTRA I/O (Maybe not implement)
	// IRQ6  : FDC
	// IRQ7  : Deisy chain (to IRQ8 - 15)
	timer->set_context_intr_line(pic, SIG_I8259_CHIP0 | SIG_I8259_IR0, 0xffffffff);
	keyboard->set_context_intr_line(pic, SIG_I8259_CHIP0 | SIG_I8259_IR1, 0xffffffff);
	floppy->set_context_intr_line(pic, SIG_I8259_CHIP0 | SIG_I8259_IR6, 0xffffffff);

	// IRQ8  : SCSI (-> scsi.cpp)
	// IRQ9  : CDC/CDROM
	// IRQ10 : EXTRA I/O (Maybe not implement)
	// IRQ11 : VSYNC
	// IRQ12 : PRINTER (ToDo)
	// IRQ13 : ADPCM AND OPN2 (Route to adpcm.cpp)
	// IRQ14 : EXTRA I/O (Maybe not implement)
	// IRQ15 : RESERVED.
	cdrom->set_context_mpuint_line(pic, SIG_I8259_CHIP1 | SIG_I8259_IR1, 0xffffffff);
	crtc->set_context_vsync(pic, SIG_I8259_CHIP1 | SIG_I8259_IR3, 0xffffffff);
	crtc->set_context_vsync(sprite, SIG_TOWNS_SPRITE_VSYNC, 0xffffffff);

	adpcm->set_context_intr_line(pic, SIG_I8259_CHIP1 | SIG_I8259_IR5, 0xffffffff);

	// DMA0  : FDC/DRQ
	// DMA1  : SCSI (-> scsi.cpp)
	// DMA2  : PRINTER (ToDo)
	// DMA3  : CDC/CDROM
	// EXTRA DMA0 : EXTRA SLOT (Maybe not implement)
	// EXTRA DMA1 : Reserved
	// EXTRA DMA2 : Reserved
	// EXTRA DMA3 : Reserved
	fdc->set_context_drq(dma, SIG_UPD71071_CH0, 1);
	fdc->set_context_drq(keyboard, SIG_KEYBOARD_BOOTSEQ_END, 1);
	cdrom->set_context_drq_line(dma, SIG_UPD71071_CH3, 0xff);
	cdrom->set_context_drq_line(keyboard, SIG_KEYBOARD_BOOTSEQ_END, 1);

	// NMI0 : KEYBOARD (RAS)
	// NMI1 : Extra SLOT (Maybe not implement)
	keyboard->set_context_nmi_line(memory, SIG_CPU_NMI, 0xffffffff);

	cdrom->set_context_dmac(dma);
	// For Debugging, will remove 20200822 K.O
	cdrom->set_context_cpu(cpu);
	//cdrom->set_context_eot_line(dma, SIG_TOWNS_DMAC_EOT_CH3, 0xffffffff);

	// i/o bus
	io->set_iowait_range_rw(0x0000, 0xffff, 6); // ToDo: May variable wait.

	io->set_iomap_alias_rw (0x0000, pic, I8259_ADDR_CHIP0 | 0);
	io->set_iomap_alias_rw (0x0002, pic, I8259_ADDR_CHIP0 | 1);
	io->set_iomap_alias_rw (0x0010, pic, I8259_ADDR_CHIP1 | 0);
	io->set_iomap_alias_rw (0x0012, pic, I8259_ADDR_CHIP1 | 1);

	io->set_iomap_single_rw(0x0020, memory); // RESET REASON / POWER CONTROL (by software)
	io->set_iomap_single_rw(0x0022, memory); // POWER CONTROL
	io->set_iomap_single_rw(0x0024, memory); // MISC3 / DMA WRAP (AFTER MA/MX/ME)
	io->set_iomap_single_r (0x0025, memory); // MISC4
	io->set_iomap_range_r  (0x0026, 0x0027, timer);  // Freerun counter
	io->set_iomap_single_rw(0x0028, memory);         // NMI MASK

	io->set_iomap_range_r  (0x0030, 0x0031, memory);	// cpu id / machine id
	io->set_iomap_single_rw(0x0032, memory);	// serial rom (routed from memory)
	io->set_iomap_single_r (0x0034, scsi);	// ENABLE/ UNABLE to WORD DMA for SCSI

	io->set_iomap_alias_rw(0x0040, pit0, 0);
	io->set_iomap_alias_rw(0x0042, pit0, 1);
	io->set_iomap_alias_rw(0x0044, pit0, 2);
	io->set_iomap_alias_rw(0x0046, pit0, 3);
	io->set_iomap_alias_rw(0x0050, pit1, 0);
	io->set_iomap_alias_rw(0x0052, pit1, 1);
	io->set_iomap_alias_rw(0x0054, pit1, 2);
	io->set_iomap_alias_rw(0x0056, pit1, 3);

	io->set_iomap_single_rw(0x0060, timer); // Beep and interrupts register
	io->set_iomap_single_rw(0x0068, timer); // Interval timer register2 : CONTROL (after Towns 10F).
	io->set_iomap_range_rw (0x006a, 0x006b, timer); // Interval timer register2 : DATA (after Towns 10F).
	io->set_iomap_single_rw(0x006c, timer); // 1uS wait register (after Towns 10F).

	io->set_iomap_single_rw(0x0070, timer); // RTC DATA
	io->set_iomap_single_w (0x0080, timer); // RTC COMMAND

	io->set_iomap_range_rw (0x00a0, 0x00af, dma);
	io->set_iomap_range_rw (0x00b0, 0x00bf, extra_dma);

	io->set_iomap_single_rw(0x00c0, memory);   // CACHE CONTROLLER   (after HR, i486)
	io->set_iomap_single_rw(0x00c2, memory);   // CACHE DIAGNNOSTICS (after HR, i486)

	io->set_iomap_alias_rw (0x0200, fdc, 0);  // STATUS/COMMAND
	io->set_iomap_alias_rw (0x0202, fdc, 1);  // TRACK
	io->set_iomap_alias_rw (0x0204, fdc, 2);  // SECTOR
	io->set_iomap_alias_rw (0x0206, fdc, 3);  // DATA
	io->set_iomap_single_rw(0x0208, floppy);  // DRIVE STATUS / DRIVE CONTROL
	io->set_iomap_single_rw(0x020c, floppy);  // DRIVE SELECT
	io->set_iomap_single_r (0x020d, floppy);  // FDDVEXT (after HG/HR).
	io->set_iomap_single_rw(0x020e, floppy);  // Towns drive SW

	io->set_iomap_single_r (0x0400, memory); // RESOLUTION
	io->set_iomap_single_rw(0x0402, memory); // RESERVED (??)
	io->set_iomap_single_rw(0x0404, memory); // SYSTEM STATUS
//	io->set_iomap_range_rw (0x0406, 0x043f, memory); // Reserved

	io->set_iomap_single_rw(0x0440, crtc); // CRTC ADDRESS INDEX
	io->set_iomap_range_rw (0x0442, 0x0443, crtc); // CRTC DATA
	io->set_iomap_single_rw(0x0448, crtc); // VIDEO OUT ADDRESS INDEX
	io->set_iomap_single_rw(0x044a, crtc); // VIDEO OUT DATA
	io->set_iomap_single_r (0x044c, crtc); // DIGITAL PALLETTE STATUS, SPRITE STATUS


	io->set_iomap_alias_rw(0x0450, sprite, 0); // SPRITE
	io->set_iomap_alias_rw(0x0452, sprite, 2); // SPRITE

	io->set_iomap_single_rw(0x0458, vram);         // VRAM ACCESS CONTROLLER (ADDRESS)
	io->set_iomap_range_rw (0x045a, 0x045b, vram); // VRAM ACCESS CONTROLLER (DATA)

	//io->set_iomap_single_r (0x0470, crtc); // HIGH RESOLUTION (after MX)
	//io->set_iomap_single_r (0x0471, vram); // VRAM CAPACITY  (after MX, by MBytes)
	//io->set_iomap_range_rw (0x0472, 0x0473, crtc); // VIDEO OUT REGS ADDRESS (after MX, by MBytes)
	//io->set_iomap_range_rw (0x0474, 0x0477, crtc); // VIDEO OUT REGS DATA (after MX, by MBytes)

	io->set_iomap_single_rw(0x0480, memory); //  MEMORY REGISTER
	io->set_iomap_single_rw(0x0484, dictionary); // Dictionary BANK

	io->set_iomap_alias_r(0x48a, iccard1, 0); //
	//io->set_iomap_alias_rw(0x490, memory_card); // After Towns2
	//io->set_iomap_alias_rw(0x491, memory_card); // After Towns2

	//io->set_iomap_single_r(0x4b0, cdrom); // CDROM functions (after MX)
	io->set_iomap_single_rw(0x04c0, cdrom); // CDROM: MASTER STATUS/MASTER CONTROL
	io->set_iomap_single_rw(0x04c2, cdrom); // CDROM: STATUS(FIFO) / COMMAND
	io->set_iomap_single_rw(0x04c4, cdrom); // CDROM: DATA (BUFFER~ / PARAMETER (FIFO)
	io->set_iomap_single_w (0x04c6, cdrom); // CDROM TRANSFER CONTROL
	io->set_iomap_single_rw(0x04c8, cdrom); // CDROM CACHE CONTROL (after HR)
	io->set_iomap_range_r  (0x04cc, 0x04cd, cdrom); // CDROM SUBCODE DATA
	// PAD, Sound

	io->set_iomap_single_r (0x04d0, joystick); // Pad1
	io->set_iomap_single_r (0x04d2, joystick); // Pad 2
	io->set_iomap_single_rw(0x04d5, adpcm); // mute
	io->set_iomap_single_w (0x04d6, joystick); // Pad out

	// OPN2(YM2612)
	io->set_iomap_alias_rw(0x04d8, opn2, 0); // STATUS(R)/Addrreg 0(W)
	io->set_iomap_alias_w (0x04da, opn2, 1);  // Datareg 0(W)
	io->set_iomap_alias_w (0x04dc, opn2, 2);  // Addrreg 1(W)
	io->set_iomap_alias_w (0x04de, opn2, 3);  // Datareg 1(W)
	// Electrical volume
	io->set_iomap_alias_rw(0x04e0, e_volumes[0], 0);
	io->set_iomap_alias_rw(0x04e1, e_volumes[0], 1);
	io->set_iomap_alias_rw(0x04e2, e_volumes[1], 0);
	io->set_iomap_alias_rw(0x04e3, e_volumes[1], 1);

	// ADPCM
	io->set_iomap_single_r (0x04e7, adpcm); // A/D SAMPLING DATA REG
	io->set_iomap_single_rw(0x04e8, adpcm); // A/D SAMPLING FLAG
	io->set_iomap_single_rw(0x04e9, adpcm); // OPN2/PCM INTERRUPT (INT13) REGISTER
	io->set_iomap_single_rw(0x04ea, adpcm); // PCM INTERRUPT MASK
	io->set_iomap_single_r (0x04eb, adpcm); // PCM INTERRUPT STATUS
	io->set_iomap_single_w (0x04ec, adpcm); // PCM LED/MUTE
	io->set_iomap_range_w (0x04f0, 0x04f8, adpcm); // PCM CONTROL REGS (WO?)

	//io->set_iomap_single_rw(0x510, newpcm); // PCM BANK (after MX)
	//io->set_iomap_single_rw(0x511, newpcm); // PCM DMA STATUS(after MX)
	//io->set_iomap_range_rw (0x512, 0x0513, newpcm); // PCM DMA COUNTER(after MX)
	//io->set_iomap_range_rw (0x514, 0x0517, newpcm); // PCM DMA ADDRESS(after MX)
	//io->set_iomap_single_rw(0x518, newpcm); // PCM CLOCK (after MX)
	//io->set_iomap_single_rw(0x519, newpcm); // PCM MODE (after MX)
	//io->set_iomap_single_rw(0x51a, newpcm); // PCM SYSTEM CONTROL (after MX)
	//io->set_iomap_single_rw(0x51b, newpcm); // PCM BUFFER STATUS/CONTROL (after MX)
	//io->set_iomap_single_rw(0x51c, newpcm); // PCM REC/PLAY (after MX)
	//io->set_iomap_single_rw(0x51d, newpcm); // PCM PEAK MONITOR / TRIGGER (after MX)
	//io->set_iomap_range_rw (0x51e, 0x051f, newpcm); // PCM LEVEL / SOFTWARE DATA (after MX)

	io->set_iomap_single_rw(0x05c0, memory); // NMI MASK
	io->set_iomap_single_r (0x05c2, memory);  // NMI STATUS
	io->set_iomap_alias_r  (0x05c8, sprite, 8); // TVRAM EMULATION
	io->set_iomap_single_w (0x05ca, crtc); // VSYNC INTERRUPT

	io->set_iomap_single_rw(0x05e0, memory); // Hidden MEMORY WAIT REGISTER from AB.COM (Towns 1/2)
	io->set_iomap_single_rw(0x05e2, memory); // Hidden MEMORY WAIT REGISTER from AB.COM (After Towns 1H/1F/2H/2F )
	io->set_iomap_single_rw(0x05e6, memory); // Hidden VRAM WAIT REGISTER from TSUGARU (Maybe after Towns 10H/10F/20H/20F )
	io->set_iomap_single_r (0x05e8, memory); // RAM capacity register.(later Towns1H/2H/1F/2F).
	io->set_iomap_single_rw(0x05ec, memory); // RAM Wait register , ofcially after Towns2, but exists after Towns1H.
	io->set_iomap_single_r (0x05ed, memory); // Maximum clock register (After HR/HG).
	io->set_iomap_single_rw(0x05ee, vram);   // VRAM CACHE CONTROLLER (After HR, i486)

	io->set_iomap_single_rw(0x0600, keyboard);
	io->set_iomap_single_rw(0x0602, keyboard);
	io->set_iomap_single_rw(0x0604, keyboard);
	//io->set_iomap_single_r (0x606, keyboard); // BUFFER FULL (for TUNER)

	//io->set_iomap_single_rw(0x0800, printer);
	//io->set_iomap_single_rw(0x0802, printer);
	//io->set_iomap_single_rw(0x0804, printer);

	io->set_iomap_alias_rw (0x0a00, sio, 0);
	io->set_iomap_alias_rw (0x0a02, sio, 1);
//	io->set_iomap_single_r (0x0a04, serial);
//	io->set_iomap_single_r (0x0a06, serial);
//	io->set_iomap_single_w (0x0a08, serial);
//	io->set_iomap_single_rw(0x0a0a, modem);
//	io->set_iomap_single_rw(0x0a0c, uart_fifo); // USART HAVE FIFO (after MA)
//	io->set_iomap_single_rw(0x0a0d, uart_fifo); // USART FIFO STATUS (after MA)
//	io->set_iomap_single_rw(0x0a0e, uart_fifo); // USART FIFO STATUS (after MA)

	io->set_iomap_single_rw(0x0c30, scsi);
	io->set_iomap_single_rw(0x0c32, scsi);
	io->set_iomap_single_r (0x0c34, scsi);

	// ToDo: Implement debugging I/Os to 2000h - 2FFFh.

	for(uint32_t addr = 0x3000; addr < 0x4000; addr += 2) {
		io->set_iomap_single_rw (addr, cmos); // CMOS
	}
	io->set_iomap_range_rw (0xfd90, 0xfd91, crtc);	// PALETTE INDEX
	io->set_iomap_range_rw (0xfd92, 0xfd93, crtc);	// PALETTE DATA BLUE
	io->set_iomap_range_rw (0xfd94, 0xfd95, crtc);	// PALETTE DATA RED
	io->set_iomap_single_rw(0xfd96, crtc);	// PALETTE DATA GREEN
	io->set_iomap_range_rw (0xfd98, 0xfd9f, crtc);	// DIGITAL PALETTE REGISTERS(EMULATED)

	io->set_iomap_single_rw(0xfda0, crtc);	// CRTC: VSYNC, HSYNC / OUTPUT CONTROL
	io->set_iomap_single_r (0xfda2, crtc);	// CRTC OUT (after UG)
	io->set_iomap_single_rw(0xfda4, memory);	// CRTC: READ COMPATIBLE (after UG)

	io->set_iomap_range_rw (0xff81, 0xff83, planevram);	// MMIO changed from Tsugaru.
	io->set_iomap_single_r (0xff84, planevram);	// MMIO
	io->set_iomap_single_r (0xff86, planevram);	// MMIO
	io->set_iomap_single_rw(0xff88, memory);	// MMIO

	io->set_iomap_range_rw (0xff94, 0xff99, memory);	// MMIO
	io->set_iomap_range_r  (0xff9c, 0xff9d, memory);	// MMIO
	io->set_iomap_single_rw(0xff9e, memory);	// MMIO
	io->set_iomap_single_rw(0xffa0, planevram);	// MMIO

	// Vram allocation may be before initialize().
	// initialize all devices
#if defined(__GIT_REPO_VERSION)
	set_git_repo_version(__GIT_REPO_VERSION);
#endif
	// ToDo : Use config framework
	int exram_size = config.current_ram_size;
	if(exram_size < 1) {
		switch((machine_id & 0xff00) >> 8) {
		case 0x00:
		case 0x01: // Towns 1/2 : Not Supported.
			exram_size = 2; // UP TO 5MB.
			break;
		case 0x03: // TOWNS2 UX
		case 0x06: // TOWNS2 UG
			exram_size = 4; // UP TO 9MB.
			break;
		case 0x02: // TOWNS 2F/2H
		case 0x04: // TOWNS 10F/10H/20F/20H
			exram_size = 6; // MAYBE UP TO 7MB.
			break;
		case 0x05: // TOWNS II CX
		case 0x08: // TOWNS II HG
			exram_size = 8; // UP TO 15MB.
			break;
		case 0x07: // Towns II HR
		case 0x09: // Towns II UR
			exram_size = 24; // UP TO 31MB.
			break;
		default:
			exram_size = 31; // UP TO 127MB.
			break;
		}
	}
	if(exram_size < MIN_RAM_SIZE) {
		exram_size = MIN_RAM_SIZE;
	}

	memory->set_extra_ram_size(exram_size);

#if defined(WITH_I386SX)
	cpu->device_model = INTEL_80386;
#elif defined(WITH_I486SX)
	cpu->device_model = INTEL_I486SX;
#elif defined(WITH_I486DX)
	cpu->device_model = INTEL_I486DX;
#elif defined(WITH_PENTIUM)
	cpu->device_model = INTEL_PENTIUM;
#else
	// I386
	cpu->device_model = INTEL_80386;
#endif

	initialize_devices();
//	cpu->set_address_mask(0xffffffff);
}

VM::~VM()
{
	// delete all devices
	release_devices();
}

void VM::set_machine_type(uint16_t machine_id, uint16_t cpu_id)
{
	if(memory != nullptr) {
		memory->set_cpu_id(cpu_id);
		memory->set_machine_id(machine_id);
	}
	if(crtc != nullptr) {
		crtc->set_cpu_id(cpu_id);
		crtc->set_machine_id(machine_id);
	}
	if(timer != nullptr) {
		timer->set_cpu_id(cpu_id);
		timer->set_machine_id(machine_id);
	}
	if(cdrom != nullptr) {
		cdrom->set_cpu_id(cpu_id);
		cdrom->set_machine_id(machine_id);
	}
	if(scsi != nullptr) {
		scsi->set_cpu_id(cpu_id);
		scsi->set_machine_id(machine_id);
	}
	if(serialrom != nullptr) {
		serialrom->set_cpu_id(cpu_id);
		serialrom->set_machine_id(machine_id);
	}
	if(floppy != nullptr) {
		floppy->set_cpu_id(cpu_id);
		floppy->set_machine_id(machine_id);
	}
#if defined(HAS_20PIX_FONTS)
	if(fontrom_20pix != nullptr) {
		fontrom_20pix->set_cpu_id(cpu_id);
		fontrom_20pix->set_machine_id(machine_id);
	}
#endif
	if(vram != nullptr) {
		vram->set_cpu_id(cpu_id);
		vram->set_machine_id(machine_id);
	}

}


// ----------------------------------------------------------------------------
// drive virtual machine
// ----------------------------------------------------------------------------

void VM::reset()
{
	// reset all devices
	boot_seq = false;
	VM_TEMPLATE::reset();
//	cpu->set_address_mask(0xffffffff);
}

void VM::special_reset(int num)
{
	boot_seq = true;

	// reset all devices
	VM_TEMPLATE::reset();
	//	cpu->set_address_mask(0xffffffff);
	__LIKELY_IF(keyboard != nullptr) {
		keyboard->special_reset(num);
	}
}

void VM::run()
{
	__LIKELY_IF(event != nullptr) {
		event->drive();
	}
}
void VM::process_boot_sequence(uint32_t val)
{
	if(boot_seq) {
		if(val != 0) {
			if(keyboard != nullptr) {
				keyboard->write_signal(SIG_KEYBOARD_BOOTSEQ_END, 0xffffffff, 0xffffffff);
			}
			boot_seq = false;
		}
	}
}

// ----------------------------------------------------------------------------
// debugger
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
DEVICE *VM::get_cpu(int index)
{
	if(index == 0) {
		return cpu;
	}
	return nullptr;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	__LIKELY_IF(crtc != nullptr) {
		crtc->draw_screen();
	}
}


// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
//	emu->lock_vm();
	// init sound manager
	event->initialize_sound(rate, samples);

	// init sound gen
	beep->initialize_sound(rate, 8000);

	// init OPN2
	#if 0
	// MASTER CLOCK MAYBE 600KHz * 12 = 7200KHz .
	// From FM-Towns Technical Databook (Rev.2), Page 201
	// opn2->initialize_sound(rate, (int)(600.0e3 * 12.0) , samples, 0.0, 0.0);
	#else
	// But...20230330 K.O:
	// From src/ym2612/ym2612.h of Tsugaru below quote.
	// Thanks to YS-11 San.
	//
	// What exactly is the master clock given to YM2612 of TOWNS?
	// FM Towns Technical Databook tells internal clock is 600KHz on page 201.
	// However, after calibrating the tone to match real Towns, it must be 690KHz.  Is it correct?
	// Master clock must be the internal clock times pre scaler.  But, we don't know the default pre scalar value.
	// Let's say it is 3.  Then, 690KHz times 3=2070KHz.  Sounds reasonable.
	// But, now FM77AV's YM2203C uses master clock frequency of 1228.8KHz.
	// And after calibration we know the ratio between the two.
	// From there, 1228.8*1698/1038=1999.46.  2MHz.  Makes more sense.
	opn2->initialize_sound(rate, (int)(((1228.8e3 * 1698.0) / 1038.0) * 4.0) , samples, 0.0, 0.0);
	#endif
	// init PCM
	rf5c68->initialize_sound(rate, samples);

	// add_sound_in_source() must add after per initialize_sound().
	adc_in_ch = event->add_sound_in_source(rate, samples, 2);
	adc->set_sample_rate(19200);
	adc->set_sound_bank(adc_in_ch);
#if 0
	mixer->set_context_out_line(adc_in_ch);
	mixer->set_context_sample_out(adc_in_ch, rate, samples); // Must be 2ch.
	// ToDo: Check recording sample rate & channels.
	mic_in_ch = event->add_sound_in_source(rate, samples, 2);
	mixer->set_context_mic_in(mic_in_ch, rate, samples);

	line_in_ch = event->add_sound_in_source(rate, samples, 2);
	mixer->set_context_line_in(line_in_ch, rate, samples);
#endif
//	emu->unlock_vm();
}

uint16_t* VM::create_sound(int* extra_frames)
{
	__LIKELY_IF(event != nullptr) {
		return event->create_sound(extra_frames);
	}
	return VM_TEMPLATE::create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
	__LIKELY_IF(event != nullptr) {
		return event->get_sound_buffer_ptr();
	}
	return VM_TEMPLATE::get_sound_buffer_ptr();
}

void VM::clear_sound_in()
{
	__UNLIKELY_IF(event == nullptr) return;

	event->clear_sound_in_source(adc_in_ch);
	event->clear_sound_in_source(mic_in_ch);
	event->clear_sound_in_source(line_in_ch);
	return;
}

int VM::get_sound_in_data(int ch, int32_t* dst, int expect_samples, int expect_rate, int expect_channels)
{
	if(dst == nullptr) return 0;
	if(expect_samples <= 0) return 0;
	int n_ch = -1;
	switch(ch) {
	case 0x00:
		n_ch = line_in_ch;
		break;
	case 0x01:
		n_ch = mic_in_ch;
		break;
	case 0x100:
		n_ch = adc_in_ch;
		break;
	}
	if(n_ch < 0) return 0;
	int samples = 0;
	if(event != nullptr) {
		samples = event->get_sound_in_data(n_ch, dst, expect_samples, expect_rate, expect_channels);
	}
	return samples;
}

// Write to event's buffer
int VM::sound_in(int ch, int32_t* src, int samples)
{
	if(ch < 0) return 0;
	if(ch >= 2) return 0;
	int n_ch = -1;
	switch(ch) {
	case 0x100:  // ADC in from MIXER, not connected.
		break;
	case 0x00: // LINE
		n_ch = line_in_ch;
		break;
	case 0x01: // MIC
		n_ch = mic_in_ch;
		break;
	}
	if(n_ch < 0) return 0;

	int ss = 0;
	if(event != nullptr) {
		emu->lock_vm();
		ss =  event->write_sound_in_buffer(n_ch, src, samples);
		emu->unlock_vm();
	}
	return ss;
}

#if defined(USE_HARD_DISK)
void VM::open_hard_disk(int drv, const _TCHAR* file_path)
{
	if((drv < USE_HARD_DISK) && (drv < 8) && (drv >= 0)) {
		if(scsi_hdd[drv] != nullptr) {
			scsi_hdd[drv]->open(0, file_path, 512);
		}
	}
}

void VM::close_hard_disk(int drv)
{
	if((drv < USE_HARD_DISK) && (drv < 8) && (drv >= 0)) {
		if(scsi_hdd[drv] != nullptr) {
			scsi_hdd[drv]->close(0);
		}
	}
}

bool VM::is_hard_disk_inserted(int drv)
{
	if((drv < USE_HARD_DISK) && (drv < 8) && (drv >= 0)) {
		if(scsi_hdd[drv] != nullptr) {
			return scsi_hdd[drv]->mounted(0);
		}
	}
	return false;
}

uint32_t VM::is_hard_disk_accessed()
{
	uint32_t status = 0;

	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
		if(scsi_hdd[drv] != nullptr) {
			if(scsi_hdd[drv]->accessed(0)) {
				status |= 1 << drv;
			}
		}
	}
	process_boot_sequence(status);
	return status;
}
#endif // USE_HARD_DISK

void VM::open_compact_disc(int drv, const _TCHAR* file_path)
{
	if(cdrom != nullptr) {
		cdrom->open(file_path);
	}
}

void VM::close_compact_disc(int drv)
{
	if(cdrom != nullptr) {
		cdrom->close();
	}
}

bool VM::is_compact_disc_inserted(int drv)
{
	if(cdrom == nullptr) return false;
	return cdrom->mounted();
}

uint32_t VM::is_compact_disc_accessed()
{
	uint32_t status = cdrom->accessed();
	process_boot_sequence(status);
	return status;
}

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
#ifndef HAS_LINEIN_SOUND
//	if(ch >= 7) ch++;
#endif
#ifndef HAS_MIC_SOUND
//	if(ch >= 8) ch++;
#endif
#ifndef HAS_MODEM_SOUND
//	if(ch >= 9) ch++;
#endif
#ifndef HAS_2ND_ADPCM
//	if(ch >= 10) ch++;
#endif
	if(ch == 0) { // BEEP
		if(beep != nullptr) {
			beep->set_volume(0, decibel_l, decibel_r);
		}
	}
	else if(ch == 1) { // CD-ROM
		if(e_volumes[1] != nullptr) {
			e_volumes[1]->set_volumes(0, decibel_l, 1, decibel_r);
		} else if(cdrom != nullptr) {
			cdrom->set_volume(0, decibel_l, decibel_r);
		}
	}
	else if(ch == 2) { // OPN2
		if(opn2 != nullptr) {
			opn2->set_volume(0, decibel_l, decibel_r);
		}
	}
	else if(ch == 3) { // ADPCM
		if(rf5c68 != nullptr) {
			rf5c68->set_volume(0, decibel_l, decibel_r);
		}
	}
	else if(ch == 4) { // SEEK, HEAD UP / DOWN
		if(seek_sound != nullptr) {
			seek_sound->set_volume(0, decibel_l, decibel_r);
		}
		if(head_up_sound != nullptr) {
			head_up_sound->set_volume(0, decibel_l, decibel_r);
		}
		if(head_down_sound != nullptr) {
			head_down_sound->set_volume(0, decibel_l, decibel_r);
		}
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	__LIKELY_IF(keyboard != nullptr) {
		keyboard->key_down(code);
	}
}

void VM::key_up(int code)
{
	__LIKELY_IF(keyboard != nullptr) {
		keyboard->key_up(code);
	}
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------
void VM::open_cart(int drv, const _TCHAR* file_path)
{
	switch(drv) {
	case 0:
		if(iccard1 != nullptr) {
			iccard1->open_cart(file_path);
		}
		break;
	case 1:
		if(iccard2 != nullptr) {
			iccard2->open_cart(file_path);
		}
		break;
	}
}

void VM::close_cart(int drv)
{
	switch(drv) {
	case 0:
		if(iccard1 != nullptr) {
			iccard1->close_cart();
		}
		break;
	case 1:
		if(iccard2 != nullptr) {
			iccard2->close_cart();
		}
		break;
	}
}

bool VM::is_cart_inserted(int drv)
{
	switch(drv) {
	case 0:
		if(iccard1 != nullptr) {
			return iccard1->is_cart_inserted();
		}
		break;
	case 1:
		if(iccard2 != nullptr) {
			return iccard2->is_cart_inserted();
		}
		break;
	}
	return false;
}

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{

	if(fdc != nullptr) {
		fdc->open_disk(drv, file_path, bank);
		floppy->change_disk(drv);
	}
}

void VM::close_floppy_disk(int drv)
{
	if(fdc != nullptr) {
		fdc->close_disk(drv);
	//	floppy->change_disk(drv);
	}
}
uint32_t VM::is_floppy_disk_accessed()
{
	uint32_t val = fdc->read_signal(0);
	process_boot_sequence(val);
	return val;
}

bool VM::is_floppy_disk_inserted(int drv)
{
	if(fdc != nullptr) {
		return fdc->is_disk_inserted(drv);
	}
	return VM_TEMPLATE::is_floppy_disk_inserted(drv);
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
	__LIKELY_IF(fdc != nullptr) {
		fdc->is_disk_protected(drv, value);
	}
}

bool VM::is_floppy_disk_protected(int drv)
{
	__LIKELY_IF(fdc != nullptr) {
		return fdc->is_disk_protected(drv);
	}
	return VM_TEMPLATE::is_floppy_disk_protected(drv);
}

bool VM::is_frame_skippable()
{
	__LIKELY_IF(event != nullptr) {
		return event->is_frame_skippable();
	}
	return VM_TEMPLATE::is_frame_skippable();
}


double VM::get_current_usec()
{
	__LIKELY_IF(event != nullptr) {
		return event->get_current_usec();
	}
	return VM_TEMPLATE::get_current_usec();
}

uint64_t VM::get_current_clock_uint64()
{
	__LIKELY_IF(event != nullptr) {
		return event->get_current_clock_uint64();
	}
	return VM_TEMPLATE::get_current_clock_uint64();
}

#define STATE_VERSION	4

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!(VM_TEMPLATE::process_state_core(state_fio, loading, STATE_VERSION))) {
		return false;
	}

	// Machine specified.
	state_fio->StateValue(boot_seq);

	if(loading) {
		update_config();
	}
	return true;
}
