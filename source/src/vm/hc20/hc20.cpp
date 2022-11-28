/*
	EPSON HC-20 Emulator 'eHC-20'

	Author : Takeda.Toshiya
	Date   : 2011.05.23-

	[ virtual machine ]
*/

#include "hc20.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../beep.h"
#include "../disk.h"
#include "../hd146818p.h"
#include "../i8255.h"
#include "../mc6800.h"
#include "../noise.h"
#include "../tf20.h"
#include "../upd765a.h"
#include "../z80.h"
#include "../z80sio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "memory.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	beep = new BEEP(this, emu);
	rtc = new HD146818P(this, emu);
	cpu = new MC6800(this, emu);
	
	tf20 = new TF20(this, emu);
	pio_tf20 = new I8255(this, emu);
	fdc_tf20 = new UPD765A(this, emu);
	fdc_tf20->set_context_noise_seek(new NOISE(this, emu));
	fdc_tf20->set_context_noise_head_down(new NOISE(this, emu));
	fdc_tf20->set_context_noise_head_up(new NOISE(this, emu));
	cpu_tf20 = new Z80(this, emu);
	sio_tf20 = new Z80SIO(this, emu);
	
	memory = new MEMORY(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_cpu(cpu_tf20, 4000000);
	event->set_context_sound(beep);
	event->set_context_sound(fdc_tf20->get_context_noise_seek());
	event->set_context_sound(fdc_tf20->get_context_noise_head_down());
	event->set_context_sound(fdc_tf20->get_context_noise_head_up());
	
/*
	memory:
		0002	in	---	port1 (cpu)
		0003	in/out	---	port2 (cpu)
		0006		---	port3 (cpu)
		0007		---	port4 (cpu)
		0020	out	bit0-7	key scan line
		0022	in	bit0-7	key scan result (lo)
		0026	out	bit0-2	selection of lcd driver (0,1-6)
				bit3	output selection for lcd driver (0=data 1=command)
				bit4	key input interrupt mask (0=Mask)
				bit5	pout (serial control line)
				bit6	shift/load select for rom cartridge (0=load 1=shift)
				bit7	clock for rom cartridge
		0028	in	bit0-1	key scan result (hi)
				bit6	power switch interrupt flag (0=active)
				bit7	busy signal of lcd controller (0=busy)
		002a	out	bit0-7	output data to lcd controller
			in	---	serial clock to lcd controller
		002b	in	---	serial clock to lcd controller
		002c	in/out	---	used for interrupt mask setting in sleep mode
		0030	in/out	---	select expansion unit rom (bank1)
		0032	in/out	---	select internal rom (bank0)
		0033	in/out	---	select internal rom (bank0)
		003c	in	---	XXX: unknown

	port1:
		p10	in	dsr (RS-232C)
		p11	in	cts (RS-232C)
		p12	in	error status of slave mcu (P34)
		p13	in	external interrupt flag(0=active)
		p14	in	battery voltage interrupt flag (0=active)
		p15	in	key input inerrupt flag (0=active)
		p16	in	pin (serial control line)
		p17	in	counter status of microcassete / rom data / plug-in option

	port 2:
		p20	in	barcode input signal (1=mark 0=space)
		p21	out	txd (RS-232C)
		p22	out	selection for CPU serial communication (0=slave 1=serial)
*/	
	cpu->set_context_port2(memory, SIG_MEMORY_PORT_2, 0xff, 0);
	cpu->set_context_port3(memory, SIG_MEMORY_PORT_3, 0xff, 0);
	cpu->set_context_port4(memory, SIG_MEMORY_PORT_4, 0xff, 0);
	cpu->set_context_sio(memory, SIG_MEMORY_SIO_MAIN);
	rtc->set_context_intr(memory, SIG_MEMORY_RTC_IRQ, 1);
	
	memory->set_context_beep(beep);
	memory->set_context_cpu(cpu);
	memory->set_context_rtc(rtc);
	memory->set_context_sio_tf20(sio_tf20);
	
	// cpu bus
	cpu->set_context_mem(memory);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// tf-20
	tf20->set_context_cpu(cpu_tf20);
	tf20->set_context_fdc(fdc_tf20);
	tf20->set_context_pio(pio_tf20);
	tf20->set_context_sio(sio_tf20);
	cpu_tf20->set_context_mem(tf20);
	cpu_tf20->set_context_io(tf20);
	cpu_tf20->set_context_intr(tf20);
#ifdef USE_DEBUGGER
	cpu_tf20->set_context_debugger(new DEBUGGER(this, emu));
#endif
	fdc_tf20->set_context_irq(cpu_tf20, SIG_CPU_IRQ, 1);
	sio_tf20->set_context_send(0, memory, SIG_MEMORY_SIO_TF20);
	sio_tf20->set_tx_clock(0, 4915200 / 8);	// 4.9152MHz / 8 (38.4kbps)
	sio_tf20->set_rx_clock(0, 4915200 / 8);	// baud-rate can be changed by jumper pin
	sio_tf20->set_tx_clock(1, 4915200 / 8);
	sio_tf20->set_rx_clock(1, 4915200 / 8);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc_tf20->set_drive_type(i, DRIVE_TYPE_2D);
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
	cpu->write_signal(SIG_MC6801_PORT_1, 0x78, 0xff);
	cpu->write_signal(SIG_MC6801_PORT_2, 0x9e, 0xff);
}

void VM::notify_power_off()
{
//	this->out_debug_log(T("--- POWER OFF ---\n"));
	memory->notify_power_off();
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
	} else if(index == 1) {
		return cpu_tf20;
	}
	return NULL;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	memory->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	beep->initialize_sound(rate, 1000, 8000);
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
		beep->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		fdc_tf20->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc_tf20->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc_tf20->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	memory->key_down(code);
}

void VM::key_up(int code)
{
	memory->key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	fdc_tf20->open_disk(drv, file_path, bank);
}

void VM::close_floppy_disk(int drv)
{
	fdc_tf20->close_disk(drv);
}

bool VM::is_floppy_disk_inserted(int drv)
{
	return fdc_tf20->is_disk_inserted(drv);
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
	fdc_tf20->is_disk_protected(drv, value);
}

bool VM::is_floppy_disk_protected(int drv)
{
	return fdc_tf20->is_disk_protected(drv);
}

uint32_t VM::is_floppy_disk_accessed()
{
	return fdc_tf20->read_signal(0);
}

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	memory->play_tape(file_path);
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	memory->rec_tape(file_path);
}

void VM::close_tape(int drv)
{
	memory->close_tape();
}

bool VM::is_tape_inserted(int drv)
{
	return memory->is_tape_inserted();
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
	return true;
}

