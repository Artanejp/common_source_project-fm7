/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ device base class ]
*/

#include "common.h"
#include "../vm.h"
#include "../..//emu.h"
#include "device.h"
#if defined(_USE_QT)
#include "../qt/gui/csp_logger.h"
extern CSP_Logger *csp_logger;
#endif

DEVICE::DEVICE(VM* parent_vm, EMU* parent_emu) : vm(parent_vm), emu(parent_emu)
{
	vm = parent_vm;
	emu = parent_emu;
	//osd = NULL;
	osd = emu->get_osd();
	//this->out_debug_log("OSD is %08x", osd);
	memset(this_device_name, 0x00, sizeof(this_device_name));
	strncpy(this_device_name, "Base Device", 128 - 1);
	prev_device = vm->last_device;
	next_device = NULL;
	if(vm->first_device == NULL) {
		// this is the first device
		vm->first_device = this;
		this_device_id = 0;
	} else {
		// this is not the first device
		vm->last_device->next_device = this;
		this_device_id = vm->last_device->this_device_id + 1;
	}
	vm->last_device = this;
	
	// primary event manager
	event_manager = NULL;
}

//DEVICE::~DEVICE(void)
//{
//}

uint32_t DEVICE::read_io8(uint32_t addr)
{
#ifdef IOBUS_RETURN_ADDR
		return (addr & 1 ? addr >> 8 : addr) & 0xff;
#else
		return 0xff;
#endif
}

int DEVICE::get_event_manager_id()
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->this_device_id;
}
void DEVICE::register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->register_event(device, event_id, usec, loop, register_id);
}

void DEVICE::register_event_by_clock(DEVICE* device, int event_id, uint64_t clock, bool loop, int* register_id)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->register_event_by_clock(device, event_id, clock, loop, register_id);
}

void DEVICE::cancel_event(DEVICE* device, int register_id)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->cancel_event(device, register_id);
}
void DEVICE::register_frame_event(DEVICE* device)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->register_frame_event(device);
}
void DEVICE::register_vline_event(DEVICE* device)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->register_vline_event(device);
}
uint32_t DEVICE::get_event_remaining_clock(int register_id)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_event_remaining_clock(register_id);
}

double DEVICE::get_event_remaining_usec(int register_id)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_event_remaining_usec(register_id);
}

uint32_t DEVICE::get_current_clock()
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_current_clock();
}

uint32_t DEVICE::get_passed_clock(uint32_t prev)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_passed_clock(prev);
}

double DEVICE::get_passed_usec(uint32_t prev)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_passed_usec(prev);
}

uint32_t DEVICE::get_passed_clock_since_vline()
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_passed_clock_since_vline();
}

double DEVICE::get_passed_usec_since_vline()
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_passed_usec_since_vline();
}

int DEVICE::get_cur_vline()
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_cur_vline();
}

int DEVICE::get_cur_vline_clocks()
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_cur_vline_clocks();
}

uint32_t DEVICE::get_cpu_pc(int index)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_cpu_pc(index);
}

void DEVICE::request_skip_frames()
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->request_skip_frames();
}

void DEVICE::set_frames_per_sec(double frames)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->set_frames_per_sec(frames);
}

void DEVICE::set_lines_per_frame(int lines)
{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->set_lines_per_frame(lines);
}

int DEVICE::get_lines_per_frame()
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_lines_per_frame();
}

// Force render sound immediately when device's status has changed.
// You must call this after you changing registers (or anything).
// If has problems, try set_realtime_render.
// See mb8877.cpp and ym2203.cpp. 
// -- 20161010 K.O
void DEVICE::touch_sound(void)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->touch_sound();
}
// Force render per 1 sample automatically.
// See pcm1bit.cpp .
// -- 20161010 K.O
void DEVICE::set_realtime_render(DEVICE *device, bool flag)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	if(device != event_manager) event_manager->set_realtime_render(device, flag);
}

void DEVICE::set_device_name(const _TCHAR *format, ...)
{
	if(format != NULL) {
		va_list ap;
		_TCHAR buffer[1024];
		
		va_start(ap, format);
		my_vstprintf_s(buffer, 1024, format, ap);
		va_end(ap);

		my_tcscpy_s(this_device_name, 128, buffer);
#ifdef _USE_QT
		emu->get_osd()->set_vm_node(this_device_id, buffer);
#endif
	}
}

void DEVICE::out_debug_log(const char *fmt, ...)
{
	char strbuf[4096];
	va_list ap;
	
	va_start(ap, fmt);
	vsnprintf(strbuf, 4095, fmt, ap);
	csp_logger->debug_log(CSP_LOG_DEBUG, this_device_id + CSP_LOG_TYPE_VM_DEVICE_0, "%s", strbuf);
	va_end(ap);
}

void DEVICE::force_out_debug_log(const char *fmt, ...)
{
	char strbuf[4096];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(strbuf, 4095, fmt, ap);
	emu->force_out_debug_log("%s", strbuf);
	va_end(ap);
}

// debugger
void *DEVICE::get_debugger()
{
	return NULL;
}
uint32_t DEVICE::get_debug_prog_addr_mask()
{
	return 0;
}

uint32_t DEVICE::get_debug_data_addr_mask()
{
	return 0;
}

void DEVICE::write_debug_data8(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_debug_data8(uint32_t addr)
{
	return 0xff;
}

void DEVICE::write_debug_data16(uint32_t addr, uint32_t data)
{
	write_debug_data8(addr, data & 0xff);
	write_debug_data8(addr + 1, (data >> 8) & 0xff);
}

uint32_t DEVICE::read_debug_data16(uint32_t addr)
{
		uint32_t val = read_debug_data8(addr);
		val |= read_debug_data8(addr + 1) << 8;
		return val;
}

void DEVICE::write_debug_data32(uint32_t addr, uint32_t data)
{
	write_debug_data16(addr, data & 0xffff);
	write_debug_data16(addr + 2, (data >> 16) & 0xffff);
}

uint32_t DEVICE::read_debug_data32(uint32_t addr)
{
	uint32_t val = read_debug_data16(addr);
	val |= read_debug_data16(addr + 2) << 16;
	return val;
}

void DEVICE::write_debug_io8(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_debug_io8(uint32_t addr)
{
	return 0xff;
}
void DEVICE::write_debug_io16(uint32_t addr, uint32_t data)
{
	write_debug_io8(addr, data & 0xff);
	write_debug_io8(addr + 1, (data >> 8) & 0xff);
}

uint32_t DEVICE::read_debug_io16(uint32_t addr)
{
	uint32_t val = read_debug_io8(addr);
	val |= read_debug_io8(addr + 1) << 8;
	return val;
}

void DEVICE::write_debug_io32(uint32_t addr, uint32_t data)
{
	write_debug_io16(addr, data & 0xffff);
	write_debug_io16(addr + 2, (data >> 16) & 0xffff);
}

uint32_t DEVICE::read_debug_io32(uint32_t addr)
{
	uint32_t val = read_debug_io16(addr);
	val |= read_debug_io16(addr + 2) << 16;
	return val;
}

bool DEVICE::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

void DEVICE::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
}

int DEVICE::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	return 0;
}

