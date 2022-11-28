/*
	NEC PC-8001 Emulator 'ePC-8001'
	NEC PC-8001mkII Emulator 'ePC-8001mkII'
	NEC PC-8001mkIISR Emulator 'ePC-8001mkIISR'
	NEC PC-8801 Emulator 'ePC-8801'
	NEC PC-8801mkII Emulator 'ePC-8801mkII'
	NEC PC-8801MA Emulator 'ePC-8801MA'

	Author : Takeda.Toshiya
	Date   : 2012.02.16-

	[ virtual machine ]
*/

#include "pc8801.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../i8251.h"
#include "../i8255.h"
#include "../pcm1bit.h"
//#include "../pcpr201.h"
#include "../prnfile.h"
#include "../upd1990a.h"
#if defined(SUPPORT_PC88_OPN1) || defined(SUPPORT_PC88_OPN2)
#include "../ym2203.h"
#endif
#include "../z80.h"

#include "../disk.h"
#include "../noise.h"
#include "../pc80s31k.h"
#include "../upd765a.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#ifdef SUPPORT_PC88_CDROM
#include "../scsi_cdrom.h"
#include "../scsi_host.h"
#endif

#ifdef SUPPORT_PC88_HMB20
#include "../ym2151.h"
#endif

#ifdef SUPPORT_PC88_GSX8800
#include "../ay_3_891x.h"
#endif

#if defined(SUPPORT_PC88_GSX8800) || defined(SUPPORT_PC88_PCG8100)
#include "../i8253.h"
#endif

#ifdef SUPPORT_M88_DISKDRV
#include "diskio.h"
#endif

#include "pc88.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// check configs
#if defined(PC8001_VARIANT)
#if defined(_PC8001)
	if(config.boot_mode != MODE_PC80_N) {
		config.boot_mode = MODE_PC80_N;
	}
	// 8inch floppy drives are not supported
	config.dipswitch &= ~DIPSWITCH_FDD_8INCH;
#elif defined(_PC8001MK2)
	if(config.boot_mode == MODE_PC80_V2) {
		config.boot_mode = MODE_PC80_V1;
	}
#endif
#else
#if !defined(PC8801SR_VARIANT)
	if(config.boot_mode == MODE_PC88_V1H || config.boot_mode == MODE_PC88_V2 || config.boot_mode == MODE_PC88_V2CD) {
		config.boot_mode = MODE_PC88_V1S;
	}
#endif
#if defined(_PC8801MK2) || defined(PC8801SR_VARIANT)
	// 5inch floppy drives are always existing
	config.dipswitch |= DIPSWITCH_FDD_5INCH;
#endif
#endif
	boot_mode = config.boot_mode;
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	
	pc88event = new EVENT(this, emu);
//	pc88event->set_device_name(_T("Event Manager (PC-8801)"));
//	pc88event->set_frames_per_sec(60);
//	pc88event->set_lines_per_frame(260);
	
	pc88 = new PC88(this, emu);
//	pc88->set_context_event_manager(pc88event);
	pc88sio = new I8251(this, emu);
//	pc88sio->set_device_name(_T("8251 SIO (PC-8801)"));
//	pc88sio->set_context_event_manager(pc88event);
	pc88pio = new I8255(this, emu);
//	pc88pio->set_device_name(_T("8255 PIO (PC-8801)"));
//	pc88pio->set_context_event_manager(pc88event);
	pc88pcm = new PCM1BIT(this, emu);
//	pc88pcm->set_device_name(_T("1-Bit PCM Sound (PC-8801)"));
//	pc88pcm->set_context_event_manager(pc88event);
	pc88rtc = new UPD1990A(this, emu);
//	pc88rtc->set_device_name(_T("uPD1990A RTC (PC-8801)"));
//	pc88rtc->set_context_event_manager(pc88event);
	if(config.printer_type == 0) {
		pc88prn = new PRNFILE(this, emu);
//		pc88prn->set_context_event_manager(pc88event);
//	} else if(config.printer_type == 1) {
//		pc88prn = new PCPR201(this, emu);
//		pc88prn->set_context_event_manager(pc88event);
	} else {
		pc88prn = dummy;
	}
	pc88cpu = new Z80(this, emu);
//	pc88cpu->set_context_event_manager(pc88event);
	
	if(config.dipswitch & DIPSWITCH_FDD_5INCH) {
		pc88sub = new PC80S31K(this, emu);
		pc88sub->set_device_name(_T("PC-80S31K (Sub)"));
//		pc88sub->set_context_event_manager(pc88event);
		pc88pio_sub = new I8255(this, emu);
		pc88pio_sub->set_device_name(_T("8255 PIO (Sub)"));
//		pc88pio_sub->set_context_event_manager(pc88event);
		pc88fdc_sub = new UPD765A(this, emu);
		pc88fdc_sub->set_device_name(_T("uPD765A FDC (Sub)"));
//		pc88fdc_sub->set_context_event_manager(pc88event);
		pc88noise_seek = new NOISE(this, emu);
//		pc88noise_seek->set_context_event_manager(pc88event);
		pc88noise_head_down = new NOISE(this, emu);
//		pc88noise_head_down->set_context_event_manager(pc88event);
		pc88noise_head_up = new NOISE(this, emu);
//		pc88noise_head_up->set_context_event_manager(pc88event);
		pc88cpu_sub = new Z80(this, emu);
		pc88cpu_sub->set_device_name(_T("Z80 CPU (Sub)"));
//		pc88cpu_sub->set_context_event_manager(pc88event);
	} else {
		pc88sub = NULL;
		pc88pio_sub = NULL;
		pc88fdc_sub = NULL;
		pc88noise_seek = pc88noise_head_down = pc88noise_head_up = NULL;
		pc88cpu_sub = NULL;
	}
#ifdef SUPPORT_PC88_FDD_8INCH
	if(config.dipswitch & DIPSWITCH_FDD_8INCH) {
		pc88fdc_8inch = new UPD765A(this, emu);
		pc88fdc_8inch->set_device_name(_T("uPD765A FDC (8inch)"));
//		pc88fdc_8inch->set_context_event_manager(pc88event);
		pc88noise_8inch_seek = new NOISE(this, emu);
//		pc88noise_8inch_seek->set_context_event_manager(pc88event);
		pc88noise_8inch_head_down = new NOISE(this, emu);
//		pc88noise_8inch_head_down->set_context_event_manager(pc88event);
		pc88noise_8inch_head_up = new NOISE(this, emu);
//		pc88noise_8inch_head_up->set_context_event_manager(pc88event);
	} else {
		pc88fdc_8inch = NULL;
		pc88noise_8inch_seek = pc88noise_8inch_head_down = pc88noise_8inch_head_up = NULL;
	}
#endif
#ifdef SUPPORT_PC88_CDROM
	pc88scsi_host = new SCSI_HOST(this, emu);
//	pc88scsi_host->set_context_event_manager(pc88event);
	pc88scsi_cdrom = new SCSI_CDROM(this, emu);
//	pc88scsi_cdrom->set_context_event_manager(pc88event);
#endif
#if defined(_PC8801MA)
	// config.sound_type
	// 	0: 44h:OPNA A4h:None		PC-8801FH/MH or later
	// 	1: 44h:OPN  A4h:None		PC-8801mkIISR/TR/FR/MR
	// 	2: 44h:OPN  A4h:OPNA		PC-8801mkIISR/TR/FR/MR + PC-8801-23
	// 	3: 44h:OPN  A4h:OPN		PC-8801mkIISR/TR/FR/MR + PC-8801-11
	// 	4: 44h:OPNA A4h:OPNA		PC-8801FH/MH or later  + PC-8801-24
	// 	5: 44h:OPNA A4h:OPN		PC-8801FH/MH or later  + PC-8801-11
	pc88opn1 = new YM2203(this, emu);
//	pc88opn1->set_context_event_manager(pc88event);
	if(config.sound_type == 0 || config.sound_type == 4 || config.sound_type == 5) {
		pc88opn1->is_ym2608 = true;
		pc88opn1->set_device_name(_T("YM2608 OPNA #1"));
	} else {
		pc88opn1->is_ym2608 = false;
		pc88opn1->set_device_name(_T("YM2203 OPN #1"));
	}
	if(config.sound_type >= 2) {
		pc88opn2 = new YM2203(this, emu);
//		pc88opn2->set_context_event_manager(pc88event);
		if(config.sound_type == 2 || config.sound_type == 4) {
			pc88opn2->is_ym2608 = true;
			pc88opn2->set_device_name(_T("YM2608 OPNA #2"));
		} else if(config.sound_type == 3 || config.sound_type == 5) {
			pc88opn2->is_ym2608 = false;
			pc88opn2->set_device_name(_T("YM2203 OPN #2"));
		}
	} else {
		pc88opn2 = NULL;
	}
#elif defined(_PC8001SR)
	// config.sound_type
	// 	0: 44h:OPN  A4h:None		PC-8001mkIISR
	// 	1: 44h:OPN  A4h:OPN		PC-8001mkIISR + PC-8801-11
	pc88opn1 = new YM2203(this, emu);
//	pc88opn1->set_context_event_manager(pc88event);
	pc88opn1->is_ym2608 = false;
	pc88opn1->set_device_name(_T("YM2203 OPN #1"));
	if(config.sound_type == 1) {
		pc88opn2 = new YM2203(this, emu);
//		pc88opn2->set_context_event_manager(pc88event);
		pc88opn2->is_ym2608 = false;
		pc88opn2->set_device_name(_T("YM2203 OPN #2"));
	} else {
		pc88opn2 = NULL;
	}
#elif defined(_PC8001MK2) || defined(_PC8801) || defined(_PC8801MK2)
	// 	0: 44h:None A4h:None		PC-8001mkII
	// 	1: 44h:None A4h:OPN		PC-8001mkII + PC-8801-11
	if(config.sound_type == 1) {
		pc88opn2 = new YM2203(this, emu);
//		pc88opn2->set_context_event_manager(pc88event);
		pc88opn2->is_ym2608 = false;
		pc88opn2->set_device_name(_T("YM2203 OPN #2"));
	} else {
		pc88opn2 = NULL;
	}
#else
	#if defined(SUPPORT_PC88_OPN1)
		pc88opn1 = new YM2203(this, emu);
//		pc88opn1->set_context_event_manager(pc88event);
		#if defined(SUPPORT_PC88_OPNA)
			pc88opn1->is_ym2608 = true;
			pc88opn1->set_device_name(_T("YM2608 OPNA #1"));
		#else
			pc88opn1->is_ym2608 = false;
			pc88opn1->set_device_name(_T("YM2203 OPN #1"));
		#endif
	#endif
	#if defined(SUPPORT_PC88_OPN2)
		pc88opn2 = new YM2203(this, emu);
//		pc88opn2->set_context_event_manager(pc88event);
		#if defined(SUPPORT_PC88_OPNA)
			pc88opn2->is_ym2608 = true;
			pc88opn2->set_device_name(_T("YM2608 OPNA #2"));
		#else
			pc88opn2->is_ym2608 = false;
			pc88opn2->set_device_name(_T("YM2203 OPN #2"));
		#endif
	#endif
#endif
#ifdef USE_DEBUGGER
#ifdef SUPPORT_PC88_OPN1
	if(pc88opn1 != NULL) {
		pc88opn1->set_context_debugger(new DEBUGGER(this, emu));
	}
#endif
#ifdef SUPPORT_PC88_OPN2
	if(pc88opn2 != NULL) {
		pc88opn2->set_context_debugger(new DEBUGGER(this, emu));
	}
#endif
#endif
#ifdef SUPPORT_PC88_HMB20
	if(config.dipswitch & DIPSWITCH_HMB20) {
		pc88opm = new YM2151(this, emu);
#ifdef USE_DEBUGGER
		pc88opm->set_context_debugger(new DEBUGGER(this, emu));
#endif
		pc88opm->set_device_name(_T("YM2151 OPM (HMB-20)"));
//		pc88opm->set_context_event_manager(pc88event);
	} else {
		pc88opm = NULL;
	}
#endif
#ifdef SUPPORT_PC88_GSX8800
	if(config.dipswitch & DIPSWITCH_GSX8800) {
//		pc88gsx_pit = new I8253(this, emu);
//		pc88gsx_pit->set_device_name(_T("8253 PIT (GSX-8800)"));
//		pc88gsx_pit->set_context_event_manager(pc88event);
		pc88gsx_psg1 = new AY_3_891X(this, emu);
		pc88gsx_psg1->set_device_name(_T("AY-3-8910 PSG #1 (GSX-8800)"));
//		pc88gsx_psg1->set_context_event_manager(pc88event);
		pc88gsx_psg2 = new AY_3_891X(this, emu);
		pc88gsx_psg2->set_device_name(_T("AY-3-8910 PSG #2 (GSX-8800)"));
//		pc88gsx_psg2->set_context_event_manager(pc88event);
		pc88gsx_psg3 = new AY_3_891X(this, emu);
		pc88gsx_psg3->set_device_name(_T("AY-3-8910 PSG #3 (GSX-8800)"));
//		pc88gsx_psg3->set_context_event_manager(pc88event);
		pc88gsx_psg4 = new AY_3_891X(this, emu);
		pc88gsx_psg4->set_device_name(_T("AY-3-8910 PSG #4 (GSX-8800)"));
//		pc88gsx_psg4->set_context_event_manager(pc88event);
#ifdef USE_DEBUGGER
		pc88gsx_psg1->set_context_debugger(new DEBUGGER(this, emu));
		pc88gsx_psg2->set_context_debugger(new DEBUGGER(this, emu));
		pc88gsx_psg3->set_context_debugger(new DEBUGGER(this, emu));
		pc88gsx_psg4->set_context_debugger(new DEBUGGER(this, emu));
#endif
	} else {
//		pc88gsx_pit = NULL;
		pc88gsx_psg1 = pc88gsx_psg2 = pc88gsx_psg3 = pc88gsx_psg4 = NULL;
	}
#endif
#ifdef SUPPORT_PC88_PCG8100
	if(config.dipswitch & DIPSWITCH_PCG8100) {
		pc88pcg_pit = new I8253(this, emu);
		pc88pcg_pit->set_device_name(_T("8253 PIT (PCG-8100)"));
//		pc88pcg_pit->set_context_event_manager(pc88event);
		pc88pcg_pcm1 = new PCM1BIT(this, emu);
		pc88pcg_pcm1->set_device_name(_T("1-Bit PCM Sound (PCG-8100 #1)"));
//		pc88pcg_pcm1->set_context_event_manager(pc88event);
		pc88pcg_pcm2 = new PCM1BIT(this, emu);
		pc88pcg_pcm2->set_device_name(_T("1-Bit PCM Sound (PCG-8100 #2)"));
//		pc88pcg_pcm2->set_context_event_manager(pc88event);
		pc88pcg_pcm3 = new PCM1BIT(this, emu);
		pc88pcg_pcm3->set_device_name(_T("1-Bit PCM Sound (PCG-8100 #3)"));
//		pc88pcg_pcm3->set_context_event_manager(pc88event);
	} else {
		pc88pcg_pit = NULL;
		pc88pcg_pcm1 = pc88pcg_pcm2 = pc88pcg_pcm3 = NULL;
	}
#endif
#ifdef SUPPORT_M88_DISKDRV
	if(config.dipswitch & DIPSWITCH_M88_DISKDRV) {
		pc88diskio = new DiskIO(this, emu);
//		pc88diskio->set_context_event_manager(pc88event);
	} else {
		pc88diskio = NULL;
	}
#endif
	
	// set cpu device contexts
#ifdef SUPPORT_PC88_HIGH_CLOCK
	pc88event->set_context_cpu(pc88cpu, (config.cpu_type == 1) ? 3993624 : 7987248);
#else
	pc88event->set_context_cpu(pc88cpu, 3993624);
#endif
	if(pc88cpu_sub != NULL) {
		pc88event->set_context_cpu(pc88cpu_sub, 3993624);
	}
	
	// set sound device contexts
	pc88event->set_context_sound(pc88pcm);
#ifdef SUPPORT_PC88_CDROM
	pc88event->set_context_sound(pc88scsi_cdrom);
#endif
#ifdef SUPPORT_PC88_OPN1
	if(pc88opn1 != NULL) {
		pc88event->set_context_sound(pc88opn1);
	}
#endif
#ifdef SUPPORT_PC88_OPN2
	if(pc88opn2 != NULL) {
		pc88event->set_context_sound(pc88opn2);
	}
#endif
#ifdef SUPPORT_PC88_HMB20
	if(config.dipswitch & DIPSWITCH_HMB20) {
		pc88event->set_context_sound(pc88opm);
	}
#endif
#ifdef SUPPORT_PC88_GSX8800
	if(config.dipswitch & DIPSWITCH_GSX8800) {
		pc88event->set_context_sound(pc88gsx_psg1);
		pc88event->set_context_sound(pc88gsx_psg2);
		pc88event->set_context_sound(pc88gsx_psg3);
		pc88event->set_context_sound(pc88gsx_psg4);
	}
#endif
#ifdef SUPPORT_PC88_PCG8100
	if(config.dipswitch & DIPSWITCH_PCG8100) {
		pc88event->set_context_sound(pc88pcg_pcm1);
		pc88event->set_context_sound(pc88pcg_pcm2);
		pc88event->set_context_sound(pc88pcg_pcm3);
	}
#endif
	if(config.dipswitch & DIPSWITCH_FDD_5INCH) {
		pc88event->set_context_sound(pc88noise_seek);
		pc88event->set_context_sound(pc88noise_head_down);
		pc88event->set_context_sound(pc88noise_head_up);
	}
#ifdef SUPPORT_PC88_FDD_8INCH
	if(config.dipswitch & DIPSWITCH_FDD_8INCH) {
		pc88event->set_context_sound(pc88noise_8inch_seek);
		pc88event->set_context_sound(pc88noise_8inch_head_down);
		pc88event->set_context_sound(pc88noise_8inch_head_up);
	}
#endif
	
	// set other device contexts
	pc88->set_context_cpu(pc88cpu);
	pc88->set_context_pcm(pc88pcm);
	pc88->set_context_pio(pc88pio);
	pc88->set_context_prn(pc88prn);
	pc88->set_context_rtc(pc88rtc);
	pc88->set_context_sio(pc88sio);
#ifdef SUPPORT_PC88_FDD_8INCH
	if(config.dipswitch & DIPSWITCH_FDD_8INCH) {
		pc88->set_context_fdc_8inch(pc88fdc_8inch);
	}
#endif
#ifdef SUPPORT_PC88_CDROM
	pc88->set_context_scsi_host(pc88scsi_host);
	pc88->set_context_scsi_cdrom(pc88scsi_cdrom);
#endif
#ifdef SUPPORT_PC88_OPN1
	if(pc88opn1 != NULL) {
		pc88->set_context_opn1(pc88opn1);
	}
#endif
#ifdef SUPPORT_PC88_OPN2
	if(pc88opn2 != NULL) {
		pc88->set_context_opn2(pc88opn2);
	}
#endif
#ifdef SUPPORT_PC88_HMB20
	if(config.dipswitch & DIPSWITCH_HMB20) {
		pc88->set_context_opm(pc88opm);
	}
#endif
#ifdef SUPPORT_PC88_GSX8800
	if(config.dipswitch & DIPSWITCH_GSX8800) {
//		pc88->set_context_gsx_pit(pc88gsx_pit);
		pc88->set_context_gsx_psg1(pc88gsx_psg1);
		pc88->set_context_gsx_psg2(pc88gsx_psg2);
		pc88->set_context_gsx_psg3(pc88gsx_psg3);
		pc88->set_context_gsx_psg4(pc88gsx_psg4);
	}
#endif
#ifdef SUPPORT_PC88_PCG8100
	if(config.dipswitch & DIPSWITCH_PCG8100) {
		pc88->set_context_pcg_pit(pc88pcg_pit);
		pc88->set_context_pcg_pcm1(pc88pcg_pcm1);
		pc88->set_context_pcg_pcm2(pc88pcg_pcm2);
		pc88->set_context_pcg_pcm3(pc88pcg_pcm3);
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
	
	if(config.dipswitch & DIPSWITCH_FDD_5INCH) {
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
	}
#ifdef SUPPORT_PC88_FDD_8INCH
	if(config.dipswitch & DIPSWITCH_FDD_8INCH) {
		pc88fdc_8inch->set_context_irq(pc88, SIG_PC88_8INCH_IRQ, 1);
		pc88fdc_8inch->set_context_drq(pc88, SIG_PC88_8INCH_DRQ, 1);
		pc88fdc_8inch->set_context_noise_seek(pc88noise_8inch_seek);
		pc88fdc_8inch->set_context_noise_head_down(pc88noise_8inch_head_down);
		pc88fdc_8inch->set_context_noise_head_up(pc88noise_8inch_head_up);
	}
#endif
#ifdef SUPPORT_PC88_CDROM
	pc88scsi_cdrom->scsi_id = 0;
	pc88scsi_cdrom->set_context_interface(pc88scsi_host);
	pc88scsi_host->set_context_target(pc88scsi_cdrom);
	pc88scsi_host->set_context_drq(pc88, SIG_PC88_SCSI_DRQ, 1);
#endif
#ifdef SUPPORT_PC88_OPN1
	if(pc88opn1 != NULL) {
		pc88opn1->set_context_irq(pc88, SIG_PC88_OPN1_IRQ, 1);
	}
#endif
#ifdef SUPPORT_PC88_OPN2
	if(pc88opn2 != NULL) {
		pc88opn2->set_context_irq(pc88, SIG_PC88_OPN2_IRQ, 1);
	}
#endif
#ifdef SUPPORT_PC88_GSX8800
//	if(config.dipswitch & DIPSWITCH_GSX8800) {
//		pc88gsx_pit->set_context_ch0(pc88, SIG_PC88_GSX_IRQ, 1);
//		pc88gsx_pit->set_context_ch1(pc88gsx_pit, SIG_I8253_CLOCK_2, 1);
//		pc88gsx_pit->set_constant_clock(0, 1996800);
//		pc88gsx_pit->set_constant_clock(1, 1996800);
//	}
#endif
#ifdef SUPPORT_PC88_PCG8100
	if(config.dipswitch & DIPSWITCH_PCG8100) {
		pc88pcg_pit->set_context_ch0(pc88pcg_pcm1, SIG_PCM1BIT_SIGNAL, 1);
		pc88pcg_pit->set_context_ch1(pc88pcg_pcm2, SIG_PCM1BIT_SIGNAL, 1);
		pc88pcg_pit->set_context_ch2(pc88pcg_pcm3, SIG_PCM1BIT_SIGNAL, 1);
		pc88pcg_pit->set_constant_clock(0, 3993624);
		pc88pcg_pit->set_constant_clock(1, 3993624);
		pc88pcg_pit->set_constant_clock(2, 3993624);
	}
#endif
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
#ifdef SUPPORT_PC88_FDD_8INCH
	if(config.dipswitch & DIPSWITCH_FDD_8INCH) {
		pc88fdc_8inch->set_drive_type(0, DRIVE_TYPE_2HD);
		pc88fdc_8inch->set_drive_type(1, DRIVE_TYPE_2HD);
//		pc88fdc_8inch->write_signal(SIG_UPD765A_MOTOR, 1, 1);
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
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
	
	// initial device settings
#ifdef SUPPORT_PC88_OPN1
	if(pc88opn1 != NULL) {
		pc88opn1->set_reg(0x29, 3); // for Misty Blue
	}
#endif
	if(pc88pio_sub != NULL) {
		pc88pio_sub->write_signal(SIG_I8255_PORT_C, 0, 0xff);
	}
	pc88pio->write_signal(SIG_I8255_PORT_C, 0, 0xff);
}

void VM::run()
{
	pc88event->drive();
}

double VM::get_frame_rate()
{
	return pc88event->get_frame_rate();
}

// ----------------------------------------------------------------------------
// debugger
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
DEVICE *VM::get_cpu(int index)
{
	if(index == 0) {
		return pc88cpu;
	} else if(index == 1) {
		return pc88cpu_sub;
	}
	return NULL;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	pc88->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
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
#ifdef SUPPORT_PC88_OPN2
	if(pc88opn2 != NULL) {
		if(pc88opn2->is_ym2608) {
			pc88opn2->initialize_sound(rate, 7987248, samples, 0, 0);
		} else {
			pc88opn2->initialize_sound(rate, 3993624, samples, 0, 0);
		}
	}
#endif
#ifdef SUPPORT_PC88_HMB20
	if(config.dipswitch & DIPSWITCH_HMB20) {
		pc88opm->initialize_sound(rate, 4000000, samples, 0);
	}
#endif
#ifdef SUPPORT_PC88_GSX8800
	if(config.dipswitch & DIPSWITCH_GSX8800) {
		pc88gsx_psg1->initialize_sound(rate, 3993624, samples, 0, 0);
		pc88gsx_psg2->initialize_sound(rate, 3993624, samples, 0, 0);
		pc88gsx_psg3->initialize_sound(rate, 3993624, samples, 0, 0);
		pc88gsx_psg4->initialize_sound(rate, 3993624, samples, 0, 0);
	}
#endif
#ifdef SUPPORT_PC88_PCG8100
	if(config.dipswitch & DIPSWITCH_PCG8100) {
		pc88pcg_pcm1->initialize_sound(rate, 8000);
		pc88pcg_pcm2->initialize_sound(rate, 8000);
		pc88pcg_pcm3->initialize_sound(rate, 8000);
	}
#endif
}

uint16_t* VM::create_sound(int* extra_frames)
{
	return pc88event->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
	return pc88event->get_sound_buffer_ptr();
}

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
#ifdef SUPPORT_PC88_OPN1
	if(ch-- == 0) {
		if(pc88opn1 != NULL) {
			pc88opn1->set_volume(0, decibel_l, decibel_r);
		}
		return;
	}
	if(ch-- == 0) {
		if(pc88opn1 != NULL) {
			pc88opn1->set_volume(1, decibel_l, decibel_r);
		}
		return;
	}
#ifdef SUPPORT_PC88_OPNA
	if(ch-- == 0) {
		if(pc88opn1 != NULL && pc88opn1->is_ym2608) {
			pc88opn1->set_volume(2, decibel_l, decibel_r);
		}
		return;
	}
	if(ch-- == 0) {
		if(pc88opn1 != NULL && pc88opn1->is_ym2608) {
			pc88opn1->set_volume(3, decibel_l, decibel_r);
		}
		return;
	}
#endif
#endif
#ifdef SUPPORT_PC88_OPN2
	if(ch-- == 0) {
		if(pc88opn2 != NULL) {
			pc88opn2->set_volume(0, decibel_l, decibel_r);
		}
		return;
	}
	if(ch-- == 0) {
		if(pc88opn2 != NULL) {
			pc88opn2->set_volume(1, decibel_l, decibel_r);
		}
		return;
	}
#ifdef SUPPORT_PC88_OPNA
	if(ch-- == 0) {
		if(pc88opn2 != NULL && pc88opn2->is_ym2608) {
			pc88opn2->set_volume(2, decibel_l, decibel_r);
		}
		return;
	}
	if(ch-- == 0) {
		if(pc88opn2 != NULL && pc88opn2->is_ym2608) {
			pc88opn2->set_volume(3, decibel_l, decibel_r);
		}
		return;
	}
#endif
#endif
#ifdef SUPPORT_PC88_CDROM
	if(ch-- == 0) {
		pc88scsi_cdrom->set_volume(0, decibel_l, decibel_r);
		return;
	}
#endif
#ifdef SUPPORT_PC88_HMB20
	if(ch-- == 0) {
		if(pc88opm != NULL) {
			pc88opm->set_volume(0, decibel_l, decibel_r);
		}
		return;
	}
#endif
#ifdef SUPPORT_PC88_GSX8800
	if(ch-- == 0) {
		if(pc88gsx_psg1 != NULL) {
			pc88gsx_psg1->set_volume(0, decibel_l, decibel_r);
		}
		if(pc88gsx_psg2 != NULL) {
			pc88gsx_psg2->set_volume(0, decibel_l, decibel_r);
		}
		if(pc88gsx_psg3 != NULL) {
			pc88gsx_psg3->set_volume(0, decibel_l, decibel_r);
		}
		if(pc88gsx_psg4 != NULL) {
			pc88gsx_psg4->set_volume(0, decibel_l, decibel_r);
		}
		return;
	}
#endif
#ifdef SUPPORT_PC88_PCG8100
	if(ch-- == 0) {
		if(pc88pcg_pcm1 != NULL) {
			pc88pcg_pcm1->set_volume(0, decibel_l, decibel_r);
		}
		if(pc88pcg_pcm2 != NULL) {
			pc88pcg_pcm2->set_volume(0, decibel_l, decibel_r);
		}
		if(pc88pcg_pcm3 != NULL) {
			pc88pcg_pcm3->set_volume(0, decibel_l, decibel_r);
		}
		return;
	}
#endif
	if(ch-- == 0) {
		pc88pcm->set_volume(0, decibel_l, decibel_r);
		return;
	}
	if(ch-- == 0) {
		if(pc88noise_seek != NULL) {
			pc88noise_seek->set_volume(0, decibel_l, decibel_r);
		}
		if(pc88noise_head_down != NULL) {
			pc88noise_head_down->set_volume(0, decibel_l, decibel_r);
		}
		if(pc88noise_head_up != NULL) {
			pc88noise_head_up->set_volume(0, decibel_l, decibel_r);
		}
#ifdef SUPPORT_PC88_FDD_8INCH
		if(pc88noise_8inch_seek != NULL) {
			pc88noise_8inch_seek->set_volume(0, decibel_l, decibel_r);
		}
		if(pc88noise_8inch_head_down != NULL) {
			pc88noise_8inch_head_down->set_volume(0, decibel_l, decibel_r);
		}
		if(pc88noise_8inch_head_up != NULL) {
			pc88noise_8inch_head_up->set_volume(0, decibel_l, decibel_r);
		}
#endif
		return;
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	pc88->key_down(code, repeat);
}

void VM::key_up(int code)
{
}

bool VM::get_caps_locked()
{
	return pc88->get_caps_locked();
}

bool VM::get_kana_locked()
{
	return pc88->get_kana_locked();
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

bool VM::is_floppy_disk_connected(int drv)
{
	DISK *handler = get_floppy_disk_handler(drv);
	
	return (handler != NULL);
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
	
	for(int drv = 0; drv < USE_FLOPPY_DISK; drv += 2) {
		UPD765A *controller = get_floppy_disk_controller(drv);
		
		if(controller != NULL) {
			status |= (controller->read_signal(0) & 3) << drv;
		}
	}
	return status;
}

UPD765A *VM::get_floppy_disk_controller(int drv)
{
	if(drv == 0 || drv == 1) {
		return pc88fdc_sub;
	}
#ifdef SUPPORT_PC88_FDD_8INCH
	if(drv == 2 || drv == 3) {
		return pc88fdc_8inch;
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

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	pc88->play_tape(file_path);
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	pc88->rec_tape(file_path);
}

void VM::close_tape(int drv)
{
	pc88->close_tape();
}

bool VM::is_tape_inserted(int drv)
{
	return pc88->is_tape_inserted();
}

#ifdef SUPPORT_PC88_CDROM
void VM::open_compact_disc(int drv, const _TCHAR* file_path)
{
	pc88scsi_cdrom->open(file_path);
}

void VM::close_compact_disc(int drv)
{
	pc88scsi_cdrom->close();
}

bool VM::is_compact_disc_inserted(int drv)
{
	return pc88scsi_cdrom->mounted();
}

uint32_t VM::is_compact_disc_accessed()
{
	return pc88scsi_cdrom->accessed();
}
#endif

bool VM::is_frame_skippable()
{
//	return event->is_frame_skippable();
	return pc88->is_frame_skippable();
}

void VM::update_config()
{
	if(boot_mode != config.boot_mode) {
		// boot mode is changed !!!
		boot_mode = config.boot_mode;
		reset();
	} else {
		for(DEVICE* device = first_device; device; device = device->next_device) {
			device->update_config();
		}
	}
}

#define STATE_VERSION	12

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
	state_fio->StateValue(boot_mode);
	return true;
}

