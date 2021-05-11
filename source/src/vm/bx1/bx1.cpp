/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2020.08.22-

	[ virtual machine ]
*/

#include "bx1.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../mc6800.h"
#include "../io.h"
#include "../memory.h"
#include "../disk.h"
#include "../mc6843.h"
#include "../mc6844.h"
#include "../noise.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
#include "keyboard.h"
#include "printer.h"

using BX1::DISPLAY;
using BX1::KEYBOARD;
using BX1::PRINTER;

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU_TEMPLATE* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	cpu = new MC6800(this, emu);
	io = new IO(this, emu);
	memory = new MEMORY(this, emu);
	fdc = new MC6843(this, emu);	// HD46503
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	dma = new MC6844(this, emu);	// HD46504
	
	display = new DISPLAY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	printer = new PRINTER(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	
//	fdc->set_context_irq(cpu, SIG_CPU_IRQ, 1);
	fdc->set_context_drq(dma, SIG_MC6844_TX_RQ_0, 1);
	dma->set_context_memory(memory);
	dma->set_context_ch0(fdc);
	dma->set_context_ch1(display);
	
	display->set_context_dma(dma);
	printer->set_context_ram(ram);
	
	// cpu bus
	cpu->set_context_mem(memory);
#ifdef USE_DEBUGGER
	DEBUGGER *debugger = new DEBUGGER(this, emu);
	cpu->set_context_debugger(debugger);
	
//	debugger->add_symbol(0x015c, _T("VRAM_TOP"));
	debugger->add_symbol(0xe121, _T("KEY_DOWN"));
	debugger->add_symbol(0xe122, _T("KEY_UP"));
	debugger->add_symbol(0xe140, _T("DMA[0].ADDR_HI"));
	debugger->add_symbol(0xe141, _T("DMA[0].ADDR_LO"));
	debugger->add_symbol(0xe142, _T("DMA[0].COUNT_HI"));
	debugger->add_symbol(0xe143, _T("DMA[0].COUNT_LO"));
	debugger->add_symbol(0xe144, _T("DMA[1].ADDR_HI"));
	debugger->add_symbol(0xe145, _T("DMA[1].ADDR_LO"));
	debugger->add_symbol(0xe146, _T("DMA[1].COUNT_HI"));
	debugger->add_symbol(0xe147, _T("DMA[1].COUNT_LO"));
	debugger->add_symbol(0xe148, _T("DMA[2].ADDR_HI"));
	debugger->add_symbol(0xe149, _T("DMA[2].ADDR_LO"));
	debugger->add_symbol(0xe14a, _T("DMA[2].COUNT_HI"));
	debugger->add_symbol(0xe14b, _T("DMA[2].COUNT_LO"));
	debugger->add_symbol(0xe14c, _T("DMA[3].ADDR_HI"));
	debugger->add_symbol(0xe14d, _T("DMA[3].ADDR_LO"));
	debugger->add_symbol(0xe14e, _T("DMA[3].COUNT_HI"));
	debugger->add_symbol(0xe14f, _T("DMA[3].COUNT_LO"));
	debugger->add_symbol(0xe150, _T("DMA[0].CH_CTRL"));
	debugger->add_symbol(0xe151, _T("DMA[1].CH_CTRL"));
	debugger->add_symbol(0xe152, _T("DMA[2].CH_CTRL"));
	debugger->add_symbol(0xe153, _T("DMA[3].CH_CTRL"));
	debugger->add_symbol(0xe154, _T("DMA.PRIORITY_CTRL"));
	debugger->add_symbol(0xe155, _T("DMA.INTERRUPT_CTRL"));
	debugger->add_symbol(0xe156, _T("DMA.DATA_CHAIN"));
	debugger->add_symbol(0xe180, _T("FDC.DATA"));
	debugger->add_symbol(0xe181, _T("FDC.CUR_TRACK"));
	debugger->add_symbol(0xe182, _T("FDC.INTSTAT_CMD"));
	debugger->add_symbol(0xe183, _T("FDC.STATA_SETUP"));
	debugger->add_symbol(0xe184, _T("FDC.STATB_SECTOR"));
	debugger->add_symbol(0xe185, _T("FDC.GEN_COUNT"));
	debugger->add_symbol(0xe186, _T("FDC.CRC_CTRL"));
	debugger->add_symbol(0xe187, _T("FDC.LOGIC_TRACK"));
#endif
	
	// memory bus
	memset(ram, 0x00, sizeof(ram));
	memset(bios_9000, 0xff, sizeof(bios_9000));
	memset(bios_f000, 0xff, sizeof(bios_f000));
	
	memory->read_bios(_T("BIOS_9000.ROM"), bios_9000, sizeof(bios_9000));
	memory->read_bios(_T("BIOS_F000.ROM"), bios_f000, sizeof(bios_f000));
	
	memory->set_memory_rw(0x0000, 0x3fff, ram);
	memory->set_memory_r(0x9000, 0xdfff, bios_9000);
	memory->set_memory_mapped_io_rw(0xe000, 0xefff, io);
	memory->set_memory_r(0xf000, 0xffff, bios_f000);
	
	// io bus
	io->set_iomap_range_rw(0xe121, 0xe122, keyboard);
	io->set_iomap_range_rw(0xe140, 0xe156, dma);
	io->set_iomap_range_rw(0xe180, 0xe187, fdc);
	io->set_iomap_range_rw(0xe210, 0xe212, printer); // ?????

//! @note Not remove below comment. 20210511 K.O	
/*
	ram
	0062		画面の桁数？
	0063-0072	画面？

dma[3]	15chから10hバイト

	chain regでdma[3]→dma[1]に設定をコピー

dma[2]	bfd5hから09hバイト	メモリ→

	c7h c7h c7h c7h e2h d9h d9h d9h d9h

dma[1]	15chから10hバイト	メモリ→DISPLAY？

*/


	
	
/*

$E121		ram[$016e]
$E122
$E124
$E128	w

$E189	r
$E18a	w

$E210	rw
$E211	rw
$E212	rw

$E387

$E700

outp(0xe210, 0x03);
a  = inp(0xe211);
a &= 0x1c;
a |= 0x81 or 0xa1;
outp(0xe210, a);
*/

//	io->set_iovalue_single_r(0xe189, 0);
//	io->set_iovalue_single_r(0xe212, 0);

/*
	e121	スイッチ？


	e122	スイッチ？
		bit5 フロッピー読み込み？



UNKNOWN:	00c0d0	IN8	e182 = ff
UNKNOWN:	00b618	IN8	e184 = ff
UNKNOWN:	00b61b	IN8	e180 = ff
UNKNOWN:	00b61e	OUT8	e180,ff
UNKNOWN:	00b667	OUT8	e18c,00

*/

	// initialize all devices
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

// ----------------------------------------------------------------------------
// drive virtual machine
// ----------------------------------------------------------------------------

void VM::reset()
{
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
}

void VM::run()
{
	event->drive();
	cpu->write_signal(SIG_CPU_IRQ, 1, 1);
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
	display->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
}

uint16_t* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
	return event->get_sound_buffer_ptr();
}

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch == 0) {
		fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	if(!repeat) {
		keyboard->key_down(code);
//		printer->key_down(code);
	}
}

void VM::key_up(int code)
{
	keyboard->key_up(code);
//	printer->key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	fdc->open_disk(drv, file_path, bank);
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

uint32_t VM::is_floppy_disk_accessed()
{
	return fdc->read_signal(0);
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

#define STATE_VERSION	1

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		const char *name = typeid(*device).name() + 6; // skip "class "
		int len = (int)strlen(name);
		
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
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}

