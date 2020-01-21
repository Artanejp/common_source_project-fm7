/*
	FUJITSU FM-Towns Emulator 'eFMR-60'

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 216.12.28 -

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

#include "../i386.h"

#include "../io.h"
#include "../mb8877.h"
#include "../msm58321.h"
#include "../noise.h"
#include "../pcm1bit.h"
#include "../scsi_hdd.h"
#include "../scsi_host.h"
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
#include "./cdc.h"
#include "./floppy.h"
#include "./fontroms.h"
#include "./keyboard.h"
#include "./msdosrom.h"
#include "./scsi.h"
#include "./serialrom.h"
#include "./timer.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------
using FMTOWNS::ADPCM;
using FMTOWNS::CDC;
using FMTOWNS::DICTIONARY;
using FMTOWNS::FLOPPY;
using FMTOWNS::FONT_ROMS;
using FMTOWNS::KEYBOARD;
using FMTOWNS::MSDOSROM;
using FMTOWNS::SCSI;
using FMTOWNS::SERIAL_ROM;
using FMTOWNS::SYSROM;
using FMTOWNS::TIMER;

using FMTOWNS::TOWNS_CDROM;
using FMTOWNS::TOWNS_CRTC;
using FMTOWNS::TOWNS_DMAC;
using FMTOWNS::TOWNS_MEMORY;
using FMTOWNS::TOWNS_SPRITE;
using FMTOWNS::TOWNS_VRAM;


VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
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
	cdc  = new CDC(this, emu);
	cdc_scsi = new SCSI_HOST(this, emu);
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
	
	scsi_host = new SCSI_HOST(this, emu);
	scsi_host->set_device_name(_T("SCSI HOST"));
	for(int i = 0; i < 7; i++) {
		if(FILEIO::IsFileExisting(create_local_path(_T("SCSI%d.DAT"), i))) {
			SCSI_HDD* scsi_hdd = new SCSI_HDD(this, emu);
#if defined(_USE_QT)
			char d_name[64] = {0};
			snprintf(d_name, 64, "SCSI DISK #%d", i + 1);
			scsi_hdd->set_device_name(d_name);
#endif			
			scsi_hdd->scsi_id = i;
			scsi_hdd->set_context_interface(scsi_host);
			scsi_host->set_context_target(scsi_hdd);
		}
	}
	dma = new UPD71071(this, emu);
	extra_dma = new UPD71071(this, emu);

	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	scsi = new SCSI(this, emu);
	timer = new TIMER(this, emu);
	
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
	cdc_mix_ch = mixer->set_context_sound(cdc);
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

	
/*	pic	0	timer
		1	keyboard
		2	rs-232c
		3	ex rs-232c
		4	(option)
		5	(option)
		6	floppy drive or dma ???
		7	(slave)
		8	scsi
		9	cd-rom controller
		10	(option)
		11	crtc vsync
		12	printer
		13	sound (OPN2 + ADPCM)
		14	(option)
		15	(reserve)
	nmi 0   keyboard (RAS)
        1   extend slot
	dma	0	floppy drive
		1	hard drive
		2	printer
		3	cd-rom controller
    dma 4   extend slot
        5   (reserve)
        6   (reserve)
        7   (reserve)


*/
	
	
	
	pit0->set_context_ch0(timer, SIG_TIMER_CH0, 1);
	pit0->set_context_ch1(timer, SIG_TIMER_CH1, 1);
	pit0->set_context_ch2(beep, SIG_PCM1BIT_SIGNAL, 1);
	pit0->set_constant_clock(0, 307200);
	pit0->set_constant_clock(1, 307200);
	pit0->set_constant_clock(2, 307200);
	pit1->set_constant_clock(1, 1228800);
	pic->set_context_cpu(cpu);
	fdc->set_context_drq(dma, SIG_UPD71071_CH0, 1);
	fdc->set_context_irq(floppy, SIG_FLOPPY_IRQ, 1);
	rtc->set_context_data(timer, SIG_TIMER_RTC, 0x0f, 0);
	rtc->set_context_busy(timer, SIG_TIMER_RTC, 0x80);
	scsi_host->set_context_irq(scsi, SIG_SCSI_IRQ, 1);
	scsi_host->set_context_drq(scsi, SIG_SCSI_DRQ, 1);
	dma->set_context_memory(memory);
	dma->set_context_ch0(fdc);
	dma->set_context_ch1(scsi_host);
	//dma->set_context_ch2(printer);
	dma->set_context_ch3(cdc);
	dma->set_context_child_dma(extra_dma);
	
	floppy->set_context_fdc(fdc);
	floppy->set_context_pic(pic);
	keyboard->set_context_pic(pic);
	
	sprite->set_context_vram(vram);
	vram->set_context_sprite(sprite);
	vram->set_context_crtc(crtc);

	//e_volume[0]->set_context_ch0(line_in, MB87878_VOLUME_LEFT);
	//e_volume[0]->set_context_ch1(line_in, MB87878_VOLUME_RIGHT);
	//e_volume[0]->set_context_ch2(NULL, MB87878_VOLUME_LEFT);
	//e_volume[0]->set_context_ch3(NULL, MB87878_VOLUME_RIGHT);
//	e_volume[1]->set_context_ch0(cdc, MB87878_VOLUME_LEFT);
//	e_volume[1]->set_context_ch1(cdc, MB87878_VOLUME_RIGHT);
	//e_volume[1]->set_context_ch2(mic, MB87878_VOLUME_LEFT | MB87878_VOLUME_RIGHT);
	//e_volume[1]->set_context_ch3(modem, MB87878_VOLUME_LEFT | MB87878_VOLUME_RIGHT);
	
	memory->set_context_vram(vram);
	memory->set_context_system_rom(sysrom);
	memory->set_context_msdos(msdosrom);
	memory->set_context_dictionary(dictionary);
	memory->set_context_font_rom(fontrom);
	memory->set_context_beep(beep);
	memory->set_context_serial_rom(serialrom);
	memory->set_context_sprite(sprite);
	memory->set_machine_id(machine_id);
	memory->set_cpu_id(cpu_id);
	memory->set_context_cpu(cpu);

	cdc->set_context_cdrom(cdrom);
	cdc->set_context_scsi_host(cdc_scsi);
	cdc->set_context_dmareq_line(dma, SIG_UPD71071_CH3, 0xff);
//	cdc->set_context_pic(pic, SIG_I8259_CHIP1 | SIG_I8259_IR1);
	
	crtc->set_context_vsync(pic, SIG_I8259_CHIP1 | SIG_I8259_IR3, 0xffffffff); // VSYNC
	adpcm->set_context_opn2(opn2);
	adpcm->set_context_rf5c68(rf5c68);
	adpcm->set_context_adc(adc);
	adpcm->set_context_pic(pic);
	adpcm->set_context_intr_line(pic, SIG_I8259_CHIP1 | SIG_I8259_IR5, 0xffffffff); // ADPCM AND OPN2

	rf5c68->set_context_interrupt_boundary(adpcm, SIG_ADPCM_WRITE_INTERRUPT, 0xffffffff);
	opn2->set_context_irq(adpcm, SIG_ADPCM_OPX_INTR, 0xffffffff);
	
	adc->set_sample_rate(19200);
	adc->set_sound_bank(-1);
	adc->set_context_interrupt(adpcm, SIG_ADPCM_ADC_INTR, 0xffffffff); 
	
	scsi->set_context_dma(dma);
	scsi->set_context_pic(pic);
	scsi->set_context_host(scsi_host);
	timer->set_context_pcm(beep);
	timer->set_context_pic(pic);
	timer->set_context_rtc(rtc);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
	cpu->set_context_dma(dma);


#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_alias_rw(0x00, pic, I8259_ADDR_CHIP0 | 0);
	io->set_iomap_alias_rw(0x02, pic, I8259_ADDR_CHIP0 | 1);
	io->set_iomap_alias_rw(0x10, pic, I8259_ADDR_CHIP1 | 0);
	io->set_iomap_alias_rw(0x12, pic, I8259_ADDR_CHIP1 | 1);
	io->set_iomap_single_rw(0x20, memory);	// reset
	io->set_iomap_single_r(0x21, memory);	// cpu misc
	io->set_iomap_single_w(0x22, memory);	// power
	//io->set_iomap_single_rw(0x24, memory);	// dma
	io->set_iomap_single_r(0x25, memory);	// cpu_misc4 (after Towns2)
	//io->set_iomap_single_r(0x26, timer);
	//io->set_iomap_single_r(0x27, timer);
	io->set_iomap_single_r(0x28, memory);   // NMI MASK (after Towns2)
	io->set_iomap_single_r(0x30, memory);	// cpu id
	io->set_iomap_single_r(0x31, memory);	// cpu id
	
	io->set_iomap_single_rw(0x32, serialrom);	// serial rom

	io->set_iomap_alias_rw(0x40, pit0, 0);
	io->set_iomap_alias_rw(0x42, pit0, 1);
	io->set_iomap_alias_rw(0x44, pit0, 2);
	io->set_iomap_alias_rw(0x46, pit0, 3);
	io->set_iomap_alias_rw(0x50, pit1, 0);
	io->set_iomap_alias_rw(0x52, pit1, 1);
	io->set_iomap_alias_rw(0x54, pit1, 2);
	io->set_iomap_alias_rw(0x56, pit1, 3);
	
	io->set_iomap_single_rw(0x60, timer);
	io->set_iomap_single_rw(0x68, timer); // Interval timer register2 (after Towns 10F).
	io->set_iomap_single_rw(0x6a, timer); // Interval timer register2 (after Towns 10F).
	io->set_iomap_single_rw(0x6b, timer); // Interval timer register2 (after Towns 10F).
	io->set_iomap_single_rw(0x6c, memory); // 1uS wait register (after Towns 10F).
	
	io->set_iomap_single_rw(0x70, timer);
	io->set_iomap_single_w(0x80, timer);
	
	io->set_iomap_range_rw(0xa0, 0xaf, dma);
	io->set_iomap_range_rw(0xb0, 0xbf, extra_dma);
	
	io->set_iomap_alias_rw(0x200, fdc, 0);
	io->set_iomap_alias_rw(0x202, fdc, 1);
	io->set_iomap_alias_rw(0x204, fdc, 2);
	io->set_iomap_alias_rw(0x206, fdc, 3);
	io->set_iomap_single_rw(0x208, floppy);
	io->set_iomap_single_rw(0x20c, floppy);
	io->set_iomap_single_rw(0x20e, floppy); // Towns drive SW
	
	io->set_iomap_single_rw(0x400, memory);	// System Status
	//io->set_iomap_single_rw(0x402, memory);
	io->set_iomap_single_rw(0x404, memory);	// System status
	
	io->set_iomap_range_rw(0x440, 0x443, crtc); // CRTC
	io->set_iomap_range_rw(0x448, 0x44c, vram); // 
	io->set_iomap_single_rw(0x450, sprite); //
	io->set_iomap_single_rw(0x452, sprite); //
	
	io->set_iomap_range_rw(0x458, 0x45f, vram); // CRTC
	
	io->set_iomap_single_rw(0x480, memory); //
	io->set_iomap_single_rw(0x484, memory); // Dictionary
	//io->set_iomap_alias_r(0x48a, memory_card, 0); //
	//io->set_iomap_alias_rw(0x490, memory_card); // After Towns2
	//io->set_iomap_alias_rw(0x491, memory_card); // After Towns2
	
	io->set_iomap_range_rw(0x4c0, 0x4cf, cdc); // CDROM
	// PAD, Sound
#if 0
	io->set_iomap_alias_r(0x4d0, pad, 0); // Pad1
	io->set_iomap_alias_r(0x4d2, pad, 1); // Pad 2
	io->set_iomap_alias_rw(0x4d5, adpcm, 0); // mute 
	io->set_iomap_alias_w(0x4d6, pad, 3); // Pad out
#else
	io->set_iomap_alias_rw(0x4d5, adpcm, 0); // mute 
#endif	
	// OPN2(YM2612)
	io->set_iomap_alias_rw(0x4d8, opn2, 0); // STATUS(R)/Addrreg 0(W)
	io->set_iomap_alias_w(0x4da, opn2, 1);  // Datareg 0(W)
	io->set_iomap_alias_w(0x4dc, opn2, 2);  // Addrreg 1(W)
	io->set_iomap_alias_w(0x4de, opn2, 3);  // Datareg 1(W)
	// Electrical volume
//	io->set_iomap_alias_rw(0x4e0, e_volume[0], 0);
//	io->set_iomap_alias_rw(0x4e1, e_volume[0], 1);
//	io->set_iomap_alias_rw(0x4e2, e_volume[1], 0);
//	io->set_iomap_alias_rw(0x4e3, e_volume[1], 1);

	// ADPCM
	io->set_iomap_range_w(0x4e7, 0x4ff, adpcm); // 

	io->set_iomap_single_rw(0x5c0, memory); // NMI MASK
	io->set_iomap_single_r(0x5c2, memory);  // NMI STATUS
	io->set_iomap_single_r(0x5c8, vram); // TVRAM EMULATION
	io->set_iomap_single_w(0x5ca, vram); // VSYNC INTERRUPT
	
	io->set_iomap_single_r(0x5e8, memory); // RAM capacity register.(later Towns1H/2H/1F/2F).
	io->set_iomap_single_r(0x5ec, memory); // RAM Wait register , ofcially after Towns2, but exists after Towns1H.
	
	io->set_iomap_single_rw(0x600, keyboard);
	io->set_iomap_single_rw(0x602, keyboard);
	io->set_iomap_single_rw(0x604, keyboard);
	//io->set_iomap_single_r(0x606, keyboard); // BufFul (After Towns2)

	//io->set_iomap_single_rw(0x800, printer);
	//io->set_iomap_single_rw(0x802, printer);
	//io->set_iomap_single_rw(0x804, printer);
	
	io->set_iomap_alias_rw(0xa00, sio, 0);
	io->set_iomap_alias_rw(0xa02, sio, 1);
//	io->set_iomap_single_r(0xa04, serial);
//	io->set_iomap_single_r(0xa06, serial);
//	io->set_iomap_single_w(0xa08, serial);
//	io->set_iomap_single_rw(0xa0a, modem);
	
	io->set_iomap_single_rw(0xc30, scsi);
	io->set_iomap_single_rw(0xc32, scsi);

	
	io->set_iomap_range_rw(0x3000, 0x3fff, memory); // CMOS
	io->set_iomap_range_rw(0xfd90, 0xfda0, vram);	// Palette and CRTC

	// Vram allocation may be before initialize().
	/*
	bool alloc_failed = false;
	for(int bank = 0; bank < 2; bank++) {
		if(alloc_failed) break;
		for(int layer = 0; layer < 2; layer++) {
			d_renderbuffer[bank][layer] = NULL;
			renderbuffer_size[bank][layer] = 0;
			
			uint32_t initial_width = 640;
			uint32_t initial_height = 480;
			uint32_t initial_stride = 640;
			uint32_t __size = initial_stride * initial_height * sizeof(scrntype_t);
			scrntype_t *p = (scrntype_t*)malloc(__size);
			if(p == NULL) {
				alloc_faled = true;
				break;
			} else {
				memset(p, 0x00, __size);
				renderbuffer_size[bank][layer] = __size;
				d_renderbuffer[bank][layer] = p;
//				d_vram->set_context_renderbuffer(p, layer, bank, width, height, stride);
			}
		}
	}
	if(alloc_failed) {
		for(int bank = 0; bank < 2; bank++) {
			for(int layer = 0; layer < 2; layer++) {
				renderbuffer_size[bank][layer] = 0;
				if(d_renderbuffer[bank][layer] != NULL) {
					free((void *)(d_renderbuffer[bank][layer]));
				}
				d_renderbuffer[bank][layer] = NULL;
				d_vram->set_context_renderbuffer(NULL, layer, bank, 0, 0, 0);
			}
		}
	}
	*/
	// initialize all devices
#if defined(__GIT_REPO_VERSION)
	strncpy(_git_revision, __GIT_REPO_VERSION, sizeof(_git_revision) - 1);
#endif
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
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

void VM::set_machine_type(uint16_t machine_id, uint16_t cpu_id)
{
	if(memory != NULL) {
		memory->set_cpu_id(cpu_id);
		memory->set_machine_id(machine_id);
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
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
	// temporary fix...
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
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
	return fdc->read_signal(0);
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
	opn2->initialize_sound(rate, 1.0e6 / (16.0 / (384.0 * 2.0)), samples, 0.0, 0.0);

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

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	fdc->open_disk(drv, file_path, bank);
	floppy->change_disk(drv);
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

	
	return true;
}

