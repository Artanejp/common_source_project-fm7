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
#include "./towns_scsi_host.h"
#include "../upd71071.h"

#include "towns_cdrom.h"
#include "towns_crtc.h"
#include "towns_dictionary.h"
#include "towns_dmac.h"
#include "towns_memory.h"
#include "towns_sprite.h"
#include "towns_sysrom.h"
#include "towns_vram.h"

// Electric Volume
//#include "mb87078.h"
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
#include "./keyboard.h"
#include "./msdosrom.h"
#include "./scsi.h"
#include "./serialrom.h"
#include "./timer.h"
#include "./iccard.h"

#include "./towns_planevram.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------
using FMTOWNS::ADPCM;
//using FMTOWNS::CDC;
using FMTOWNS::DICTIONARY;
using FMTOWNS::FLOPPY;
using FMTOWNS::FONT_ROMS;
using FMTOWNS::JOYSTICK;
using FMTOWNS::KEYBOARD;
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
using FMTOWNS::TOWNS_SCSI_HOST;
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
	first_device = last_device = NULL;
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
	
	crtc = new TOWNS_CRTC(this, emu);
	cdrom = new TOWNS_CDROM(this, emu);

	memory = new TOWNS_MEMORY(this, emu);
	vram = new TOWNS_VRAM(this, emu);
	sprite = new TOWNS_SPRITE(this, emu);
	sysrom = new SYSROM(this, emu);
	msdosrom = new MSDOSROM(this, emu);
	fontrom = new FONT_ROMS(this, emu);
	dictionary = new DICTIONARY(this, emu);
#if defined(HAS_20PIX_FONTS)
	fontrom_20pix = new FONT_ROM_20PIX(this, emu);
#endif
	serialrom = new SERIAL_ROM(this, emu);

	adpcm = new ADPCM(this, emu);
//	mixer = new MIXER(this, emu); // Pseudo mixer.

	planevram = new PLANEVRAM(this, emu);
	
	adc = new AD7820KR(this, emu);
	rf5c68 = new RF5C68(this, emu);
//	e_volume[0] = new MB87878(this, emu);
//	e_volume[1] = new MB87878(this, emu);
	
	sio = new I8251(this, emu);
	pit0 = new I8253(this, emu);
	pit1 = new I8253(this, emu);
	pic = new I8259(this, emu);
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
		scsi_hdd[i] = NULL;
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
	iccard2 = NULL;
#endif
	
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
#elif defined(_FMTOWNS_MX_VARIANTS)
	machine_id = 0x0c00; // OK?
	cpu_id = 0x0002;      // i486DX (With FPU?)
	cpu_clock = 66000 * 1000; // 66MHz
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
#if 0
	line_mix_ch = -1;
	modem_mix_ch = -1;
	mic_mix_ch = -1;
	//line_mix_ch = mixer->set_context_sound(line_in);
	//modem_mix_ch = mixer->set_context_sound(modem_in);
	//mic_mix_ch = mixer->set_context_sound(mic_in);
	beep_mix_ch = mixer->set_context_sound(beep);
	pcm_mix_ch  = mixer->set_context_sound(rf5c68);
	opn2_mix_ch = mixer->set_context_sound(opn2);
	cdc_mix_ch = mixer->set_context_sound(cdrom);
	mixer->set_interpolate_filter_freq(pcm_mix_ch, 4000); // channel, freq; disable if freq <= 0.
	event->set_context_sound(mixer);
#else
	// Temporally not use mixer.
	event->set_context_sound(beep);
	event->set_context_sound(opn2);
	event->set_context_sound(rf5c68);
	event->set_context_sound(cdrom);
#endif
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
//	pic->set_context_cpu(cpu);
	pic->set_context_cpu(memory);
	fdc->set_context_irq(floppy, SIG_FLOPPY_IRQ, 1);
	rtc->set_context_data(timer, SIG_TIMER_RTC, 0x0f, 0);
	rtc->set_context_busy(timer, SIG_TIMER_RTC_BUSY, 0x80);
	scsi_host->set_context_irq(scsi, SIG_SCSI_IRQ, 1);
	scsi_host->set_context_drq(scsi, SIG_SCSI_DRQ, 1);
	dma->set_context_memory(memory);
	dma->set_context_ch0(fdc);
	dma->set_context_ch1(scsi_host);
	//dma->set_context_ch2(printer);
	dma->set_context_ch3(cdrom);
	dma->set_context_tc1(scsi, SIG_SCSI_EOT, 0xffffffff);
	dma->set_context_tc3(cdrom, SIG_TOWNS_CDROM_DMAINT, 0xffffffff);
	dma->set_context_ube1(scsi_host, SIG_SCSI_16BIT_BUS, 0x02);
	
	dma->set_context_child_dma(extra_dma);
	
	floppy->set_context_fdc(fdc);
	
	sprite->set_context_vram(vram);	
	sprite->set_context_font(fontrom);
	sprite->set_context_crtc(crtc);
#ifdef USE_DEBUGGER
	sprite->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	vram->set_context_sprite(sprite);
	vram->set_context_crtc(crtc);
	
	planevram->set_context_vram(vram);
	planevram->set_context_sprite(sprite);
	planevram->set_context_crtc(crtc);
	
	crtc->set_context_sprite(sprite);
	crtc->set_context_vram(vram);
	crtc->set_context_font(fontrom);

	//e_volume[0]->set_context_ch0(line_in, MB87878_VOLUME_LEFT);
	//e_volume[0]->set_context_ch1(line_in, MB87878_VOLUME_RIGHT);
	//e_volume[0]->set_context_ch2(NULL, MB87878_VOLUME_LEFT);
	//e_volume[0]->set_context_ch3(NULL, MB87878_VOLUME_RIGHT);
//	e_volume[1]->set_context_ch0(cdrom, MB87878_VOLUME_LEFT);
//	e_volume[1]->set_context_ch1(cdrom, MB87878_VOLUME_RIGHT);
	//e_volume[1]->set_context_ch2(mic, MB87878_VOLUME_LEFT | MB87878_VOLUME_RIGHT);
	//e_volume[1]->set_context_ch3(modem, MB87878_VOLUME_LEFT | MB87878_VOLUME_RIGHT);
	
	memory->set_context_cpu(cpu);
	memory->set_context_dmac(dma);
	memory->set_context_vram(vram);
	memory->set_context_planevram(planevram);
	memory->set_context_crtc(crtc);
	memory->set_context_system_rom(sysrom);
	memory->set_context_msdos(msdosrom);
	memory->set_context_dictionary(dictionary);
	memory->set_context_font_rom(fontrom);
	memory->set_context_beep(beep);
	memory->set_context_serial_rom(serialrom);
	memory->set_context_sprite(sprite);
	memory->set_context_pcm(rf5c68);
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
	timer->set_context_halt_line(cpu, SIG_CPU_HALTREQ, 0xffffffff);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
	cpu->set_context_dma(dma);
	cpu->set_context_bios(NULL);
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
	cdrom->set_context_drq_line(dma, SIG_UPD71071_CH3, 0xff);

	// NMI0 : KEYBOARD (RAS)
	// NMI1 : Extra SLOT (Maybe not implement)
	keyboard->set_context_nmi_line(memory, SIG_CPU_NMI, 0xffffffff);

	cdrom->set_context_dmac(dma);
	// For Debugging, will remove 20200822 K.O
	cdrom->set_context_cpu(cpu);

	// i/o bus
	io->set_iowait_range_rw(0x0000, 0xffff, 6); // ToDo: May variable wait.
		
	io->set_iomap_alias_rw (0x0000, pic, I8259_ADDR_CHIP0 | 0);
	io->set_iomap_alias_rw (0x0002, pic, I8259_ADDR_CHIP0 | 1);
	io->set_iomap_alias_rw (0x0010, pic, I8259_ADDR_CHIP1 | 0);
	io->set_iomap_alias_rw (0x0012, pic, I8259_ADDR_CHIP1 | 1);
	
	io->set_iomap_range_rw (0x0020, 0x0025, memory);
	io->set_iomap_range_rw (0x0026, 0x0027, timer);  // Freerun counter
	io->set_iomap_single_rw(0x0028, memory);
	
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
	io->set_iomap_single_rw(0x0068, timer); // Interval timer register2 (after Towns 10F).
	io->set_iomap_single_rw(0x006a, timer); // Interval timer register2 (after Towns 10F).
	io->set_iomap_single_rw(0x006b, timer); // Interval timer register2 (after Towns 10F).
	io->set_iomap_single_rw(0x006c, timer); // 1uS wait register (after Towns 10F).
	
	io->set_iomap_single_rw(0x0070, timer); // RTC DATA
	io->set_iomap_single_w (0x0080, timer); // RTC COMMAND
	
	io->set_iomap_range_rw (0x00a0, 0x00af, dma);
	io->set_iomap_range_rw (0x00b0, 0x00bf, extra_dma);
	
	io->set_iomap_alias_rw (0x0200, fdc, 0);  // STATUS/COMMAND
	io->set_iomap_alias_rw (0x0202, fdc, 1);  // TRACK
	io->set_iomap_alias_rw (0x0204, fdc, 2);  // SECTOR
	io->set_iomap_alias_rw (0x0206, fdc, 3);  // DATA
	io->set_iomap_single_rw(0x0208, floppy);  // DRIVE STATUS / DRIVE CONTROL
	io->set_iomap_single_rw(0x020c, floppy);  // DRIVE SELECT
	io->set_iomap_single_rw(0x020e, floppy);  // Towns drive SW
	
	io->set_iomap_range_rw (0x0400, 0x0404, memory); // System Status
	io->set_iomap_range_rw (0x0406, 0x403f, memory); // Reserved
	
	io->set_iomap_range_rw(0x0440, 0x0443, crtc); // CRTC
	io->set_iomap_range_rw(0x0448, 0x044f, crtc); // VIDEO OUT (CRTC)
	
	io->set_iomap_range_rw(0x0450, 0x0452, sprite); // SPRITE
	
	io->set_iomap_single_rw(0x0458, vram);         // VRAM ACCESS CONTROLLER (ADDRESS)
	io->set_iomap_range_rw (0x045a, 0x045f, vram); // VRAM ACCESS CONTROLLER (DATA)
	
	io->set_iomap_single_rw(0x0480, memory); //  MEMORY REGISTER
	io->set_iomap_single_rw(0x0484, dictionary); // Dictionary
	
	io->set_iomap_alias_r(0x48a, iccard1, 0); //
	//io->set_iomap_alias_rw(0x490, memory_card); // After Towns2
	//io->set_iomap_alias_rw(0x491, memory_card); // After Towns2
	
	io->set_iomap_range_rw(0x04c0, 0x04cf, cdrom); // CDROM
	// PAD, Sound
#if 1
	io->set_iomap_single_r(0x04d0, joystick); // Pad1
	io->set_iomap_single_r(0x04d2, joystick); // Pad 2
	io->set_iomap_single_w(0x04d6, joystick); // Pad out
#endif	
	io->set_iomap_single_rw(0x04d5, adpcm); // mute 
	// OPN2(YM2612)
	io->set_iomap_alias_rw(0x04d8, opn2, 0); // STATUS(R)/Addrreg 0(W)
	io->set_iomap_alias_w (0x04da, opn2, 1);  // Datareg 0(W)
	io->set_iomap_alias_w (0x04dc, opn2, 2);  // Addrreg 1(W)
	io->set_iomap_alias_w (0x04de, opn2, 3);  // Datareg 1(W)
	// Electrical volume
//	io->set_iomap_alias_rw(0x04e0, e_volume[0], 0);
//	io->set_iomap_alias_rw(0x04e1, e_volume[0], 1);
//	io->set_iomap_alias_rw(0x04e2, e_volume[1], 0);
//	io->set_iomap_alias_rw(0x04e3, e_volume[1], 1);

	// ADPCM
	io->set_iomap_range_rw(0x04e7, 0x04ef, adpcm); // A/D SAMPLING DATA REG 
	io->set_iomap_range_rw(0x04f0, 0x04f8, rf5c68); // A/D SAMPLING DATA REG 

	io->set_iomap_single_rw(0x05c0, memory); // NMI MASK
	io->set_iomap_single_r (0x05c2, memory);  // NMI STATUS
	io->set_iomap_single_r (0x05c8, sprite); // TVRAM EMULATION
	io->set_iomap_single_w (0x05ca, crtc); // VSYNC INTERRUPT
	if(machine_id < 0x0200) {
		io->set_iomap_single_rw(0x05e0, memory); //  MEMORY WAIT REGISTER ffrom AB.COM 
	} else {
		io->set_iomap_single_rw(0x05e2, memory); // MEMORY WAIT REGISTER ffrom AB.COM 
	}
	io->set_iomap_single_rw(0x05e8, memory); // RAM capacity register.(later Towns1H/2H/1F/2F).
	io->set_iomap_single_rw(0x05ec, memory); // RAM Wait register , ofcially after Towns2, but exists after Towns1H.
	io->set_iomap_single_r (0x05ed, memory); // RAM Wait register , ofcially after Towns2, but exists after Towns1H.
	
	io->set_iomap_single_rw(0x0600, keyboard);
	io->set_iomap_single_rw(0x0602, keyboard);
	io->set_iomap_single_rw(0x0604, keyboard);

	//io->set_iomap_single_rw(0x0800, printer);
	//io->set_iomap_single_rw(0x0802, printer);
	//io->set_iomap_single_rw(0x0804, printer);
	
	io->set_iomap_alias_rw (0x0a00, sio, 0);
	io->set_iomap_alias_rw (0x0a02, sio, 1);
//	io->set_iomap_single_r (0x0a04, serial);
//	io->set_iomap_single_r (0x0a06, serial);
//	io->set_iomap_single_w (0x0a08, serial);
//	io->set_iomap_single_rw(0x0a0a, modem);
	
	io->set_iomap_single_rw(0x0c30, scsi);
	io->set_iomap_single_rw(0x0c32, scsi);

	io->set_iomap_range_rw (0x3000, 0x3fff, dictionary); // CMOS
	
	io->set_iomap_range_rw (0xfd90, 0xfda2, crtc);	// Palette and CRTC
	io->set_iomap_single_rw(0xfda4, memory);	// memory
	
	io->set_iomap_range_rw (0xff80, 0xff83, planevram);	// MMIO
	io->set_iomap_single_r (0xff84, planevram);	// MMIO
	io->set_iomap_single_rw(0xff86, planevram);	// MMIO
	io->set_iomap_single_rw(0xff88, memory);	// MMIO
	io->set_iomap_range_rw (0xff94, 0xff99, memory);	// MMIO
	io->set_iomap_range_r  (0xff9c, 0xff9d, memory);	// MMIO
	io->set_iomap_single_rw(0xff9e, memory);	// MMIO
	io->set_iomap_single_rw(0xffa0, planevram);	// MMIO
	
//	io->set_iomap_range_w (0xff94, 0xff95, fontrom);
//	io->set_iomap_range_r (0xff96, 0xff97, fontrom);

	// Vram allocation may be before initialize().
	// initialize all devices
#if defined(__GIT_REPO_VERSION)
	strncpy(_git_revision, __GIT_REPO_VERSION, sizeof(_git_revision) - 1);
#endif
	// ToDo : Use config framework
	int exram_size = config.current_ram_size;
	if(exram_size < 1) {
		if(machine_id < 0x0200) { // Model1 - 2H
			exram_size = 6;
		} else if(machine_id == 0x0500) { // CX
			exram_size = 15;
		} else if(machine_id < 0x0700) {  // 10F,20H
			exram_size = 8;
		} else if(machine_id == 0x0800) { // HG
			exram_size = 15;
		} else { 
			exram_size = 31;
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

	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
//	cpu->set_address_mask(0xffffffff);
}

VM::~VM()
{
	// delete all devices
	for(DEVICE* device = first_device; device;) {
		DEVICE *next_device = device->next_device;
//		printf("DEVID=%d\n", device->this_device_id);
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

void VM::set_machine_type(uint16_t machine_id, uint16_t cpu_id)
{
	if(memory != NULL) {
		memory->set_cpu_id(cpu_id);
		memory->set_machine_id(machine_id);
	}
	if(crtc != NULL) {
		crtc->set_cpu_id(cpu_id);
		crtc->set_machine_id(machine_id);
	}
	if(timer != NULL) {
		timer->set_cpu_id(cpu_id);
		timer->set_machine_id(machine_id);
	}
	if(cdrom != NULL) {
		cdrom->set_cpu_id(cpu_id);
		cdrom->set_machine_id(machine_id);
	}
	if(scsi != NULL) {
		scsi->set_cpu_id(cpu_id);
		scsi->set_machine_id(machine_id);
	}
	if(serialrom != NULL) {
		serialrom->set_cpu_id(cpu_id);
		serialrom->set_machine_id(machine_id);
	}
#if defined(HAS_20PIX_FONTS)
	if(fontrom_20pix != NULL) {
		fontrom_20pix->set_cpu_id(cpu_id);
		fontrom_20pix->set_machine_id(machine_id);
	}
#endif
	
}		


// ----------------------------------------------------------------------------
// drive virtual machine
// ----------------------------------------------------------------------------

void VM::reset()
{
	// reset all devices
	boot_seq = true;
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
//	cpu->set_address_mask(0xffffffff);
}

void VM::special_reset(int num)
{
	// reset all devices
	boot_seq = true;
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
	keyboard->special_reset(num);

//	cpu->set_address_mask(0xffffffff);
}

void VM::run()
{
	event->drive();
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
	return NULL;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	crtc->draw_screen();
}

uint32_t VM::is_floppy_disk_accessed()
{
	uint32_t val = fdc->read_signal(0);
	if(boot_seq) {
		if(val != 0) {
			keyboard->write_signal(SIG_KEYBOARD_BOOTSEQ_END, 0xffffffff, 0xffffffff);
			boot_seq = false;
		}
	}
	return val;
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	emu->lock_vm();
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	beep->initialize_sound(rate, 8000);

	// init OPN2
	// MASTER CLOCK MAYBE 600KHz * 12 = 7200KHz .
	// From FM-Towns Technical Databook (Rev.2), Page 201
	opn2->initialize_sound(rate, (int)(600.0e3 * 12.0) , samples, 0.0, 0.0); 
	//opn2->initialize_sound(rate, (int)(8000.0e3) , samples, 0.0, 0.0); 
	//opn2->initialize_sound(rate, (int)(600.0e3 * 6.0) , samples, 0.0, 0.0); 

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
	emu->unlock_vm();
}

uint16_t* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
	return event->get_sound_buffer_ptr();
}

void VM::clear_sound_in()
{
	event->clear_sound_in_source(adc_in_ch);
	event->clear_sound_in_source(mic_in_ch);
	event->clear_sound_in_source(line_in_ch);
	return;
}

int VM::get_sound_in_data(int ch, int32_t* dst, int expect_samples, int expect_rate, int expect_channels)
{
	if(dst == NULL) return 0;
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
	int samples = event->get_sound_in_data(n_ch, dst, expect_samples, expect_rate, expect_channels);
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
	{
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
		if(scsi_hdd[drv] != NULL) {
			scsi_hdd[drv]->open(0, file_path, 512);
		}
	}
}

void VM::close_hard_disk(int drv)
{
	if((drv < USE_HARD_DISK) && (drv < 8) && (drv >= 0)) {
		if(scsi_hdd[drv] != NULL) {
			scsi_hdd[drv]->close(0);
		}
	}
}

bool VM::is_hard_disk_inserted(int drv)
{
	if((drv < USE_HARD_DISK) && (drv < 8) && (drv >= 0)) {
		if(scsi_hdd[drv] != NULL) {
			return scsi_hdd[drv]->mounted(0);
		}
	}
	return false;
}

uint32_t VM::is_hard_disk_accessed()
{
	uint32_t status = 0;
	
	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
		if(scsi_hdd[drv] != NULL) {
			if(scsi_hdd[drv]->accessed(0)) {
				status |= 1 << drv;
			}
		}
	}
	if(boot_seq) {
		if(status != 0) {
			keyboard->write_signal(SIG_KEYBOARD_BOOTSEQ_END, 0xffffffff, 0xffffffff);
			boot_seq = false;
		}
	}
	return status;
}
#endif // USE_HARD_DISK

void VM::open_compact_disc(int drv, const _TCHAR* file_path)
{
	cdrom->open(file_path);
}

void VM::close_compact_disc(int drv)
{
	cdrom->close();
}

bool VM::is_compact_disc_inserted(int drv)
{
	return cdrom->mounted();
}

uint32_t VM::is_compact_disc_accessed()
{
	uint32_t status = cdrom->accessed();
	if(boot_seq) {
		if(status != 0) {
			keyboard->write_signal(SIG_KEYBOARD_BOOTSEQ_END, 0xffffffff, 0xffffffff);
			boot_seq = false;
		}
	}
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
#if 0	
	if(ch == 0) { // BEEP
		mixer->set_volume(beep_mix_ch, decibel_l, decibel_r);
	}
	else if(ch == 1) { // CD-ROM
		//e_volume[1]->set_volume(0, decibel_l);
		//e_volume[1]->set_volume(1, decibel_r);
		mixer->set_volume(cdc_mix_ch, decibel_l, decibel_r);
	}
	else if(ch == 2) { // OPN2
		mixer->set_volume(opn2_mix_ch, decibel_l, decibel_r);
	}
	else if(ch == 3) { // ADPCM
		mixer->set_volume(pcm_mix_ch, decibel_l, decibel_r);
	}
	else if(ch == 4) { // LINE IN
		//mixer->set_volume(line_mix_ch, decibel_l, decibel_r);
	} 
	else if(ch == 5) { // MIC
		//mic->set_volume(0, (decibel_l + decibel_r) / 2);
	} 
	else if(ch == 6) { // MODEM
		//modem->set_volume(0, (decibel_l + decibel_r) / 2);
	}
#ifdef HAS_2ND_ADPCM
	else if(ch == 7) { // ADPCM
		adpcm2->set_volume(0, decibel_l, decibel_r);
	}
#endif
	else if(ch == 8) { // FDD
		fdc->set_volume(0, decibel_l, decibel_r);
	}
	else if(ch == 9) { // HDD(ToDo)
		fdc->set_volume(0, decibel_l, decibel_r);
	}	
#else
	if(ch == 0) { // BEEP
		beep->set_volume(0, decibel_l, decibel_r);
	}
	else if(ch == 1) { // CD-ROM
		cdrom->set_volume(0, decibel_l, decibel_r);
	}	
	else if(ch == 2) { // OPN2
		opn2->set_volume(0, decibel_l, decibel_r);
	}
	else if(ch == 3) { // ADPCM
		rf5c68->set_volume(0, decibel_l, decibel_r);
	}
	else if(ch == 4) { // SEEK, HEAD UP / DOWN
		seek_sound->set_volume(0, decibel_l, decibel_r);
		head_up_sound->set_volume(0, decibel_l, decibel_r);
		head_down_sound->set_volume(0, decibel_l, decibel_r);
	}
	
#endif
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	keyboard->key_down(code);
}

void VM::key_up(int code)
{
	keyboard->key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------
void VM::open_cart(int drv, const _TCHAR* file_path)
{
	switch(drv) {
	case 0:
		if(iccard1 != NULL) {
			iccard1->open_cart(file_path);
		}
		break;
	case 1:
		if(iccard2 != NULL) {
			iccard2->open_cart(file_path);
		}
		break;
	}
}

void VM::close_cart(int drv)
{
	switch(drv) {
	case 0:
		if(iccard1 != NULL) {
			iccard1->close_cart();
		}
		break;
	case 1:
		if(iccard2 != NULL) {
			iccard2->close_cart();
		}
		break;
	}		
}

bool VM::is_cart_inserted(int drv)
{
	switch(drv) {
	case 0:
		if(iccard1 != NULL) {
			return iccard1->is_cart_inserted();
		}
		break;
	case 1:
		if(iccard2 != NULL) {
			return iccard2->is_cart_inserted();
		}
		break;
	}
	return false;
}

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	fdc->open_disk(drv, file_path, bank);
//	floppy->change_disk(drv);
}

void VM::close_floppy_disk(int drv)
{
	fdc->close_disk(drv);
}

bool VM::is_floppy_disk_inserted(int drv)
{
	return fdc->is_disk_inserted(drv);
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
	fdc->is_disk_protected(drv, value);
}

bool VM::is_floppy_disk_protected(int drv)
{
	return fdc->is_disk_protected(drv);
}

bool VM::is_frame_skippable()
{
	return event->is_frame_skippable();
}

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

double VM::get_current_usec()
{
	if(event == NULL) return 0.0;
	return event->get_current_usec();
}

uint64_t VM::get_current_clock_uint64()
{
		if(event == NULL) return (uint64_t)0;
		return event->get_current_clock_uint64();
}

#define STATE_VERSION	3

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
 	for(DEVICE* device = first_device; device; device = device->next_device) {
		// Note: typeid(foo).name is fixed by recent ABI.Not decr. 6.
 		// const char *name = typeid(*device).name();
		//       But, using get_device_name() instead of typeid(foo).name() 20181008 K.O
		const char *name = device->get_device_name();
		int len = strlen(name);
		if(!state_fio->StateCheckInt32(len)) {
			return false;
		}
		if(!state_fio->StateCheckBuffer(name, len, 1)) {
 			return false;
 		}
		if(!device->process_state(state_fio, loading)) {
			if(loading) {
				printf("Data loading Error: DEVID=%d\n", device->this_device_id);
			}
 			return false;
 		}
 	}
	// Machine specified.
	state_fio->StateValue(beep_mix_ch);
	state_fio->StateValue(cdc_mix_ch);
	state_fio->StateValue(opn2_mix_ch);
	state_fio->StateValue(pcm_mix_ch);
	state_fio->StateValue(line_mix_ch);
	state_fio->StateValue(modem_mix_ch);
	state_fio->StateValue(mic_mix_ch);

	state_fio->StateValue(boot_seq);
	
	return true;
}

