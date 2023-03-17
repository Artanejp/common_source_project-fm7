/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ device base class ]
*/

#include "../common.h"

#if !defined(_USE_QT)
#include "../vm.h"
#include "../../emu.h"
#endif

#include "./device.h"
#include <memory>


DEVICE::DEVICE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : vm(parent_vm), emu(parent_emu)
{
	__IOBUS_RETURN_ADDR = false;
	__USE_DEBUGGER = false;
#if defined(_USE_QT)
	osd = emu->get_osd();
	p_logger = osd->get_logger();
#else
	osd = NULL;
	p_logger = NULL;
#endif
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

DEVICE::~DEVICE(void)
{
}

void DEVICE::initialize()
{
#if defined(_USE_QT)
	if(osd == nullptr) {
	#if defined(IOBUS_RETURN_ADDR)
		__IOBUS_RETURN_ADDR = true;
	#endif
	#if defined(USE_DEBUGGER)
		__USE_DEBUGGER = true;
	#endif
	} else {
		__IOBUS_RETURN_ADDR = osd->check_feature(_T("IOBUS_RETURN_ADDR"));
		__USE_DEBUGGER = osd->check_feature("USE_DEBUGGER");
	}
#elif defined(IOBUS_RETURN_ADDR)
		__IOBUS_RETURN_ADDR = true;
#endif
	/* Initializing VM must be after initializing OSD. */
}

void DEVICE::release()
{
}

void DEVICE::reset()
{
}

void DEVICE::special_reset(int num)
{
	reset();
}


uint32_t DEVICE::translate_address(int segment, uint32_t offset)
{
	return offset;
}

void DEVICE::write_data8(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_data8(uint32_t addr)
{
	return 0xff;
}

void DEVICE::write_data16(uint32_t addr, uint32_t data)
{
	write_data8(addr,     (data     ) & 0xff);
	write_data8(addr + 1, (data >> 8) & 0xff);
}

uint32_t DEVICE::read_data16(uint32_t addr)
{
	uint32_t val;
	val  = read_data8(addr    );
	val |= read_data8(addr + 1) << 8;
	return val;
}

void DEVICE::write_data32(uint32_t addr, uint32_t data)
{
	__LIKELY_IF(!(addr & 1)) {
		write_data16(addr,     (data      ) & 0xffff);
		write_data16(addr + 2, (data >> 16) & 0xffff);
	} else {
		write_data8 (addr,     (data      ) & 0x00ff);
		write_data16(addr + 1, (data >>  8) & 0xffff);
		write_data8 (addr + 3, (data >> 24) & 0x00ff);
	}
}

uint32_t DEVICE::read_data32(uint32_t addr)
{
	__LIKELY_IF(!(addr & 1)) {
		uint32_t val;
		val  = read_data16(addr    );
		val |= read_data16(addr + 2) << 16;
		return val;
	} else {
		uint32_t val;
		val  = read_data8 (addr    );
		val |= read_data16(addr + 1) <<  8;
		val |= read_data8 (addr + 3) << 24;
		return val;
	}
}

void DEVICE::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
	write_data8(addr, data);
}

uint32_t DEVICE::read_data8w(uint32_t addr, int* wait)
{
	*wait = 0;
	return read_data8(addr);
}
void DEVICE::write_data16w(uint32_t addr, uint32_t data, int* wait)
{
	int wait_l = 0, wait_h = 0;
	write_data8w(addr,     (data     ) & 0xff, &wait_l);
	write_data8w(addr + 1, (data >> 8) & 0xff, &wait_h);
	*wait = wait_l + wait_h;
}

uint32_t DEVICE::read_data16w(uint32_t addr, int* wait)
{
	int wait_l = 0, wait_h = 0;
	uint32_t val;
	val  = read_data8w(addr,     &wait_l);
	val |= read_data8w(addr + 1, &wait_h) << 8;
	*wait = wait_l + wait_h;
	return val;
}

void DEVICE::write_data32w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(!(addr & 1)) {
		int wait_l = 0, wait_h = 0;
		write_data16w(addr,     (data      ) & 0xffff, &wait_l);
		write_data16w(addr + 2, (data >> 16) & 0xffff, &wait_h);
		*wait = wait_l + wait_h;
	} else {
		int wait_l = 0, wait_m = 0, wait_h = 0;
		write_data8w (addr,     (data      ) & 0x00ff, &wait_l);
		write_data16w(addr + 1, (data >>  8) & 0xffff, &wait_m);
		write_data8w (addr + 3, (data >> 24) & 0x00ff, &wait_h);
		*wait = wait_l + wait_m + wait_h;
	}
}

uint32_t DEVICE::read_data32w(uint32_t addr, int* wait)
{
	__LIKELY_IF(!(addr & 1)) {
		int wait_l = 0, wait_h = 0;
		uint32_t val;
		val  = read_data16w(addr,     &wait_l);
		val |= read_data16w(addr + 2, &wait_h) << 16;
		*wait = wait_l + wait_h;
		return val;
	} else {
		int wait_l = 0, wait_m = 0, wait_h = 0;
		uint32_t val;
		val  = read_data8w (addr,     &wait_l);
		val |= read_data16w(addr + 1, &wait_m) <<  8;
		val |= read_data8w (addr + 3, &wait_h) << 24;
		*wait = wait_l + wait_m + wait_h;
		return val;
	}
}

uint32_t DEVICE::fetch_op(uint32_t addr, int *wait)
{
	return read_data8w(addr, wait);
}

void DEVICE::write_dma_data8(uint32_t addr, uint32_t data)
{
	write_data8(addr, data);
}

uint32_t DEVICE::read_dma_data8(uint32_t addr)
{
	return read_data8(addr);
}

void DEVICE::write_dma_data16(uint32_t addr, uint32_t data)
{
	write_data16(addr, data);
}

uint32_t DEVICE::read_dma_data16(uint32_t addr)
{
	return read_data16(addr);
}
void DEVICE::write_dma_data32(uint32_t addr, uint32_t data)
{
	write_data32(addr, data);
}

uint32_t DEVICE::read_dma_data32(uint32_t addr)
{
	return read_data32(addr);
}


void DEVICE::write_dma_data8w(uint32_t addr, uint32_t data, int* wait)
{
	write_data8w(addr, data, wait);
}

uint32_t DEVICE::read_dma_data8w(uint32_t addr, int* wait)
{
	return read_data8w(addr, wait);
}

void DEVICE::write_dma_data16w(uint32_t addr, uint32_t data, int* wait)
{
	write_data16w(addr, data, wait);
}

uint32_t DEVICE::read_dma_data16w(uint32_t addr, int* wait)
{
	return read_data16w(addr, wait);
}

void DEVICE::write_dma_data32w(uint32_t addr, uint32_t data, int* wait)
{
	write_data32w(addr, data, wait);
}

uint32_t DEVICE::read_dma_data32w(uint32_t addr, int* wait)
{
	return read_data32w(addr, wait);
}

//<! i/o bus

void DEVICE::write_io8(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_io8(uint32_t addr)
{
	__UNLIKELY_IF(__IOBUS_RETURN_ADDR) {
		return (addr & 1 ? addr >> 8 : addr) & 0xff;
	} else {
		return 0xff;
	}
}

void DEVICE::write_io16(uint32_t addr, uint32_t data)
{
	write_io8(addr    , (data     ) & 0xff);
	write_io8(addr + 1, (data >> 8) & 0xff);
}

uint32_t DEVICE::read_io16(uint32_t addr)
{
	uint32_t val;
	val  = read_io8(addr    );
	val |= read_io8(addr + 1) << 8;
	return val;
}

void DEVICE::write_io32(uint32_t addr, uint32_t data)
{
	__LIKELY_IF(!(addr & 1)) {
		write_io16(addr,     (data      ) & 0xffff);
		write_io16(addr + 2, (data >> 16) & 0xffff);
	} else {
		write_io8 (addr,     (data      ) & 0x00ff);
		write_io16(addr + 1, (data >>  8) & 0xffff);
		write_io8 (addr + 3, (data >> 24) & 0x00ff);
	}
}

uint32_t DEVICE::read_io32(uint32_t addr)
{
	__LIKELY_IF(!(addr & 1)) {
		uint32_t val;
		val  = read_io16(addr    );
		val |= read_io16(addr + 2) << 16;
		return val;
	} else {
		uint32_t val;
		val  = read_io8 (addr    );
		val |= read_io16(addr + 1) <<  8;
		val |= read_io8 (addr + 3) << 24;
		return val;
	}
}

void DEVICE::write_io8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
	write_io8(addr, data);
}

uint32_t DEVICE::read_io8w(uint32_t addr, int* wait)
{
	*wait = 0;
	return read_io8(addr);
}

void DEVICE::write_io16w(uint32_t addr, uint32_t data, int* wait)
{
	int wait_0, wait_1;
	write_io8w(addr,     (data     ) & 0xff, &wait_0);
	write_io8w(addr + 1, (data >> 8) & 0xff, &wait_1);
	*wait = wait_0 + wait_1;
}

uint32_t DEVICE::read_io16w(uint32_t addr, int* wait)
{
	int wait_0, wait_1;
	uint32_t val;
	val  = read_io8w(addr,     &wait_0);
	val |= read_io8w(addr + 1, &wait_1) << 8;
	*wait = wait_0 + wait_1;
	return val;
}

void DEVICE::write_io32w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(!(addr & 1)) {
		int wait_0, wait_1;
		write_io16w(addr,     (data      ) & 0xffff, &wait_0);
		write_io16w(addr + 2, (data >> 16) & 0xffff, &wait_1);
		*wait = wait_0 + wait_1;
	} else {
		int wait_0, wait_1, wait_2;
		write_io8w (addr,     (data      ) & 0x00ff, &wait_0);
		write_io16w(addr + 1, (data >>  8) & 0xffff, &wait_1);
		write_io8w (addr + 3, (data >> 24) & 0x00ff, &wait_2);
		*wait = wait_0 + wait_1 + wait_2;
	}
}

uint32_t DEVICE::read_io32w(uint32_t addr, int* wait)
{
	__LIKELY_IF(!(addr & 1)) {
		int wait_0, wait_1;
		uint32_t val;
		val  = read_io16w(addr,     &wait_0);
		val |= read_io16w(addr + 2, &wait_1) << 16;
		*wait = wait_0 + wait_1;
		return val;
	} else {
		int wait_0, wait_1, wait_2;
		uint32_t val;
		val  = read_io8w (addr,     &wait_0);
		val |= read_io16w(addr + 1, &wait_1) <<  8;
		val |= read_io8w (addr + 3, &wait_2) << 24;
		*wait = wait_0 + wait_1 + wait_2;
		return val;
	}
}

void DEVICE::write_dma_io8(uint32_t addr, uint32_t data)
{
	write_io8(addr, data);
}

uint32_t DEVICE::read_dma_io8(uint32_t addr)
{
	return read_io8(addr);
}

void DEVICE::write_dma_io16(uint32_t addr, uint32_t data)
{
	write_io16(addr, data);
}

uint32_t DEVICE::read_dma_io16(uint32_t addr)
{
	return read_io16(addr);
}

void DEVICE::write_dma_io32(uint32_t addr, uint32_t data)
{
	write_io32(addr, data);
}

uint32_t DEVICE::read_dma_io32(uint32_t addr)
{
	return read_io32(addr);
}

void DEVICE::write_dma_io8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
	write_dma_io8(addr, data);
}

uint32_t DEVICE::read_dma_io8w(uint32_t addr, int* wait)
{
	*wait = 0;
	return read_dma_io8(addr);
}

void DEVICE::write_dma_io16w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
	write_dma_io16(addr, data);
}

uint32_t DEVICE::read_dma_io16w(uint32_t addr, int* wait)
{
	*wait = 0;
	return read_dma_io16(addr);
}

void DEVICE::write_dma_io32w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
	write_dma_io32(addr, data);
}

uint32_t DEVICE::read_dma_io32w(uint32_t addr, int* wait)
{
	*wait = 0;
	return read_dma_io32(addr);
}

// memory mapped i/o
void DEVICE::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	write_io8(addr, data);
}

uint32_t DEVICE::read_memory_mapped_io8(uint32_t addr)
{
	return read_io8(addr);
}

void DEVICE::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	write_memory_mapped_io8(addr,     (data     ) & 0xff);
	write_memory_mapped_io8(addr + 1, (data >> 8) & 0xff);
}

uint32_t DEVICE::read_memory_mapped_io16(uint32_t addr)
{
	uint32_t val;
	val =  read_memory_mapped_io8(addr    );
	val |= read_memory_mapped_io8(addr + 1) << 8;
	return val;
}

void DEVICE::write_memory_mapped_io32(uint32_t addr, uint32_t data)
{
	__LIKELY_IF(!(addr & 1)) {
		write_memory_mapped_io16(addr,     (data      ) & 0xffff);
		write_memory_mapped_io16(addr + 2, (data >> 16) & 0xffff);
	} else {
		write_memory_mapped_io8 (addr,     (data      ) & 0x00ff);
		write_memory_mapped_io16(addr + 1, (data >>  8) & 0xffff);
		write_memory_mapped_io8 (addr + 3, (data >> 24) & 0x00ff);
	}
}

uint32_t DEVICE::read_memory_mapped_io32(uint32_t addr)
{
	__LIKELY_IF(!(addr & 1)) {
		uint32_t val;
		val  = read_memory_mapped_io16(addr    );
		val |= read_memory_mapped_io16(addr + 2) << 16;
		return val;
	} else {
		uint32_t val;
		val  = read_memory_mapped_io8 (addr    );
		val |= read_memory_mapped_io16(addr + 1) <<  8;
		val |= read_memory_mapped_io8 (addr + 3) << 24;
		return val;
	}
}

void DEVICE::write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
	write_memory_mapped_io8(addr, data);
}

uint32_t DEVICE::read_memory_mapped_io8w(uint32_t addr, int* wait)
{
	*wait = 0;
	return read_memory_mapped_io8(addr);
}

void DEVICE::write_memory_mapped_io16w(uint32_t addr, uint32_t data, int* wait)
{
	int wait_l = 0, wait_h = 0;
	write_memory_mapped_io8w(addr,     (data     ) & 0xff, &wait_l);
	write_memory_mapped_io8w(addr + 1, (data >> 8) & 0xff, &wait_h);
	*wait = wait_l + wait_h;
}

uint32_t DEVICE::read_memory_mapped_io16w(uint32_t addr, int* wait)
{
	int wait_l = 0, wait_h = 0;
	uint32_t val;
	val  = read_memory_mapped_io8w(addr,     &wait_l);
	val |= read_memory_mapped_io8w(addr + 1, &wait_h) << 8;
	*wait = wait_l + wait_h;
	return val;
}

void DEVICE::write_memory_mapped_io32w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(!(addr & 1)) {
		int wait_l = 0, wait_h = 0;
		write_memory_mapped_io16w(addr,     (data      ) & 0xffff, &wait_l);
		write_memory_mapped_io16w(addr + 2, (data >> 16) & 0xffff, &wait_h);
		*wait = wait_l + wait_h;
	} else {
		int wait_l = 0, wait_m = 0, wait_h = 0;
		write_memory_mapped_io8w (addr,     (data      ) & 0x00ff, &wait_l);
		write_memory_mapped_io16w(addr + 1, (data >>  8) & 0xffff, &wait_m);
		write_memory_mapped_io8w (addr + 3, (data >> 24) & 0x00ff, &wait_h);
		*wait = wait_l + wait_m + wait_h;
	}
}

uint32_t DEVICE::read_memory_mapped_io32w(uint32_t addr, int* wait)
{
	__LIKELY_IF(!(addr & 1)) {
		int wait_l = 0, wait_h = 0;
		uint32_t val;
		val  = read_memory_mapped_io16w(addr,     &wait_l);
		val |= read_memory_mapped_io16w(addr + 2, &wait_h) << 16;
		*wait = wait_l + wait_h;
		return val;
	} else {
		int wait_l = 0, wait_m = 0, wait_h = 0;
		uint32_t val;
		val  = read_memory_mapped_io8w (addr,     &wait_l);
		val |= read_memory_mapped_io16w(addr + 1, &wait_m) <<  8;
		val |= read_memory_mapped_io8w (addr + 3, &wait_h) << 24;
		*wait = wait_l + wait_m + wait_h;
		return val;
	}
}

//<! device to device
void DEVICE::initialize_output_signals(outputs_t *items)
{
	items->count = 0;
}

void DEVICE::register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask, int shift)
{
	int c = items->count++;
	items->item[c].device = device;
	items->item[c].id = id;
	items->item[c].mask = mask;
	items->item[c].shift = shift;
}

void DEVICE::register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask)
{
	int c = items->count++;
	items->item[c].device = device;
	items->item[c].id = id;
	items->item[c].mask = mask;
	items->item[c].shift = 0;
}


void DEVICE::write_signals(outputs_t *items, uint32_t data)
{
	for(int i = 0; i < items->count; i++) {
		output_t *item = &items->item[i];
		int shift = item->shift;
		uint32_t val = (shift < 0) ? (data >> (-shift)) : (data << shift);
		uint32_t mask = (shift < 0) ? (item->mask >> (-shift)) : (item->mask << shift);
		item->device->write_signal(item->id, val, mask);
	}
}

void DEVICE::update_signal_mask(outputs_t *items, DEVICE *device, uint32_t mask)
{
	__UNLIKELY_IF(items == NULL) return;
	int c = items->count;
	__UNLIKELY_IF(c <= 0) return;
	__LIKELY_IF(c >= MAX_OUTPUT) c = MAX_OUTPUT - 1;
	// if (ARG:device == NULL) apply to all devices.
	for(int i = 0; i < c; i++) {
		__UNLIKELY_IF((device == NULL) || (device == items->item[i].device)) {
			items->item[i].mask = mask;
		}
	}
}

void DEVICE::write_signal(int id, uint32_t data, uint32_t mask)
{
}

uint32_t DEVICE::read_signal(int ch)
{
	return 0;
}

//<! z80 daisy chain
void DEVICE::set_context_intr(DEVICE* device, uint32_t bit)
{
}

void DEVICE::set_context_child(DEVICE* device)
{
}

DEVICE *DEVICE::get_context_child()
{
	return NULL;
}

void DEVICE::set_intr_iei(bool val)
{
}

void DEVICE::set_intr_line(bool line, bool pending, uint32_t bit)
{
}

// interrupt cpu to device
uint32_t DEVICE::get_intr_ack()
{
	return 0xff;
}

void DEVICE::update_intr()
{

}

void DEVICE::notify_intr_reti()
{
}

void DEVICE::notify_intr_ei()
{
}

void DEVICE::do_dma()
{
}

//<! cpu
int DEVICE::run(int clock)
{
	// when clock == -1, run one opecode
	return (clock == -1 ? 1 : clock);
}

void DEVICE::set_extra_clock(int clock)
{
}

int DEVICE::get_extra_clock()
{
	return 0;
}

uint32_t DEVICE::get_pc()
{
	return 0;
}

uint32_t DEVICE::get_next_pc()
{
	return 0;
}

//<! bios
bool DEVICE::bios_call_far_i86(uint32_t PC, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
{
	return false;
}
bool DEVICE::bios_int_i86(int intnum, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
{
	return false;
}
bool DEVICE::bios_call_far_ia32(uint32_t PC, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
{
	return false;
}
bool DEVICE::bios_int_ia32(int intnum, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
{
	return false;
}
bool DEVICE::bios_ret_z80(uint16_t PC, pair32_t* af, pair32_t* bc, pair32_t* de, pair32_t* hl, pair32_t* ix, pair32_t* iy, uint8_t* iff1)
{
	return false;
}



// Sound input functions

void DEVICE::clear_sound_in_source(int bank)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->clear_sound_in_source(bank);
}



// this function may be before (or after) initialize().
int DEVICE::add_sound_in_source(int rate, int samples, int channels)
{
	__UNLIKELY_IF(event_manager == NULL) return -1;
	return event_manager->add_sound_in_source(rate, samples, channels);
}

// this function may be before (or after) initialize().
int DEVICE::release_sound_in_source(int bank)
{
	__UNLIKELY_IF(event_manager == NULL) return -1;
	return event_manager->release_sound_in_source(bank);
}

bool DEVICE::is_sound_in_source_exists(int bank)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->is_sound_in_source_exists(bank);
}

int DEVICE::increment_sound_in_passed_data(int bank, double passed_usec)
{
	__UNLIKELY_IF(event_manager == NULL) {
		return 0;
	}
	return event_manager->increment_sound_in_passed_data(bank, passed_usec);
}

int DEVICE::get_sound_in_buffers_count()
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_sound_in_buffers_count();
}

int DEVICE::get_sound_in_samples(int bank)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_sound_in_samples(bank);
}

int DEVICE::get_sound_in_rate(int bank)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_sound_in_rate(bank);
}

int DEVICE::get_sound_in_channels(int bank)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_sound_in_channels(bank);
}

// this function may be before (or after) initialize().
int16_t* DEVICE::get_sound_in_buf_ptr(int bank)
{
	__UNLIKELY_IF(event_manager == NULL) return NULL;
	return event_manager->get_sound_in_buf_ptr(bank);
}

int DEVICE::write_sound_in_buffer(int bank, int32_t* src, int samples)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->write_sound_in_buffer(bank, src, samples);
}

// Add sampled values to sample buffer;value may be -32768 to +32767.
// this function may be before (or after) initialize().
int DEVICE::get_sound_in_latest_data(int bank, int32_t* dst, int expect_channels)
{
	__UNLIKELY_IF(event_manager == NULL) return 0;
	return event_manager->get_sound_in_latest_data(bank, dst, expect_channels);
}

int DEVICE::get_sound_in_data(int bank, int32_t* dst, int expect_samples, int expect_rate, int expect_channels)
{
	__UNLIKELY_IF(event_manager == NULL) return -1;
	return event_manager->get_sound_in_data(bank, dst, expect_samples, expect_rate, expect_channels);
}

void DEVICE::set_high_pass_filter_freq(int freq, double quality)
{
}

void DEVICE::set_low_pass_filter_freq(int freq, double quality)
{
}

int DEVICE::get_event_manager_id()
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->this_device_id;
}

uint32_t DEVICE::get_event_clocks()
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_event_clocks();
}

bool DEVICE::is_primary_cpu(DEVICE* device)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->is_primary_cpu(device);
}

uint32_t DEVICE::get_cpu_clocks(DEVICE* device)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_cpu_clocks(device);
}

void DEVICE::update_event_in_opecode(int clock)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->update_event_in_opecode(clock);
}

void DEVICE::register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->register_event(device, event_id, usec, loop, register_id);
}

void DEVICE::register_event_by_clock(DEVICE* device, int event_id, uint64_t clock, bool loop, int* register_id)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->register_event_by_clock(device, event_id, clock, loop, register_id);
}

void DEVICE::cancel_event(DEVICE* device, int register_id)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->cancel_event(device, register_id);
}

// Clear and DE-Register EVENT at slot evid.
void DEVICE::clear_event(DEVICE* dev, int& evid)
{
	__LIKELY_IF(evid > -1) {
		cancel_event(dev, evid);
	}
	evid = -1;
}

// Register a EVENT to evid (and update evid) , even if evid's slot is used.
void DEVICE::force_register_event(DEVICE* dev, int event_num, double usec, bool loop, int& evid)
{
	clear_event(dev, evid);
	register_event(dev, event_num, usec, loop, &evid);
}

void DEVICE::force_register_event_by_clock(DEVICE* dev, int event_num, uint64_t clock, bool loop, int& evid)
{
	clear_event(dev, evid);
	register_event_by_clock(dev, event_num, clock, loop, &evid);
}

// Register a EVENT to evid , if evid slot isn't used.
void DEVICE::check_and_update_event(DEVICE* dev, int event_num, double usec, bool loop, int& evid)
{
	__UNLIKELY_IF(evid > -1) return;
	register_event(dev, event_num, usec, loop, &evid);
}

void DEVICE::check_and_update_event_by_clock(DEVICE* dev, int event_num, uint64_t clock, bool loop, int& evid)
{
	__UNLIKELY_IF(evid > -1) return;
	register_event_by_clock(dev, event_num, clock, loop, &evid);
}


void DEVICE::register_frame_event(DEVICE* device)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->register_frame_event(device);
}
void DEVICE::register_vline_event(DEVICE* device)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->register_vline_event(device);
}

uint32_t DEVICE::get_event_remaining_clock(int register_id)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_event_remaining_clock(register_id);
}

double DEVICE::get_event_remaining_usec(int register_id)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_event_remaining_usec(register_id);
}

uint32_t DEVICE::get_current_clock()
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_current_clock();
}

uint32_t DEVICE::get_passed_clock(uint32_t prev)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_passed_clock(prev);
}

double DEVICE::get_passed_usec(uint32_t prev)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_passed_usec(prev);
}

uint32_t DEVICE::get_passed_clock_since_vline()
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_passed_clock_since_vline();
}

double DEVICE::get_passed_usec_since_vline()
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_passed_usec_since_vline();
}

int DEVICE::get_cur_vline()
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_cur_vline();
}

int DEVICE::get_cur_vline_clocks()
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_cur_vline_clocks();
}

uint32_t DEVICE::get_cpu_pc(int index)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_cpu_pc(index);
}

uint64_t DEVICE::get_current_clock_uint64()
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_current_clock_uint64();
}

uint32_t DEVICE::get_cpu_clock(int index)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_cpu_clock(index);
}

void DEVICE::request_skip_frames()
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->request_skip_frames();
}

void DEVICE::set_frames_per_sec(double frames)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->set_frames_per_sec(frames);
}

void DEVICE::set_lines_per_frame(int lines)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->set_lines_per_frame(lines);
}

int DEVICE::get_lines_per_frame()
{
	__UNLIKELY_IF(event_manager == NULL) {
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
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->touch_sound();
}
// Force render per 1 sample automatically.
// See pcm1bit.cpp .
// -- 20161010 K.O
void DEVICE::set_realtime_render(DEVICE *device, bool flag)
{
	__UNLIKELY_IF(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	__LIKELY_IF(device != event_manager) event_manager->set_realtime_render(device, flag);
}

void DEVICE::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
}

//!< event callback
void DEVICE::event_callback(int event_id, int err)
{
}

void DEVICE::event_pre_frame()
{
	// this event is to update timing settings
}

void DEVICE::event_frame()
{
	// this event is to update timing settings
}

void DEVICE::event_vline(int v, int clock)
{
}

void DEVICE::event_hsync(int v, int h, int clock)
{
}

void DEVICE::mix(int32_t* buffer, int cnt)
{
}

void DEVICE::set_volume(int ch, int decibel_l, int decibel_r)
{
	// +1 equals +0.5dB (same as fmgen)
}

void DEVICE::get_volume(int ch, int &decibel_l, int &decibel_r)
{
	decibel_l = 0;
	decibel_r = 0;
}

void DEVICE::set_device_name(const _TCHAR *format, ...)
{
	__LIKELY_IF(format != NULL) {
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
#if defined(_USE_QT)
 	__UNLIKELY_IF(osd == nullptr) return;
  	char strbuf[4096] = {0};
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(strbuf, 4095, fmt, ap);
	va_end(ap);
	osd->debug_log(CSP_LOG_DEBUG, this_device_id + CSP_LOG_TYPE_VM_DEVICE_0,  strbuf);
#else
	char strbuf[4096] = {0};
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(strbuf, 4095, fmt, ap);
	emu->out_debug_log("%s", strbuf);
	va_end(ap);
#endif
}

void DEVICE::out_debug_log_with_switch(bool logging, const char *fmt, ...)
{
	__UNLIKELY_IF(!(logging)) return;
#if defined(_USE_QT)
	__UNLIKELY_IF(osd == nullptr) return;
   	char strbuf[4096] = {0};
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(strbuf, 4095, fmt, ap);
	va_end(ap);
	osd->debug_log(CSP_LOG_DEBUG, this_device_id + CSP_LOG_TYPE_VM_DEVICE_0, strbuf);

#else
	char strbuf[4096];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(strbuf, 4095, fmt, ap);
	emu->out_debug_log("%s", strbuf);
	va_end(ap);
#endif
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

void DEVICE::write_debug_data8(uint32_t addr, uint32_t data)
{
//		write_data8(addr, data);
}

uint32_t DEVICE::read_debug_data8(uint32_t addr)
{
//		return read_data8(addr);
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
//		write_io8(addr, data);
}

uint32_t DEVICE::read_debug_io8(uint32_t addr)
{
//		return read_io8(addr);
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

uint32_t DEVICE::read_debug_reg(const _TCHAR *reg)
{
	return 0;
}

bool DEVICE::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	return false;
}

bool DEVICE::get_debug_regs_description(_TCHAR *buffer, size_t buffer_len)
{
	return false;
}

int DEVICE::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	return debug_dasm_with_userdata(pc, buffer, buffer_len, 0);
}

int DEVICE::debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata)
{
	return 0;
}

bool DEVICE::debug_rewind_call_trace(uint32_t pc, int &size, _TCHAR* buffer, size_t buffer_len, uint64_t userdata)
{
		size = 0;
		return false;
}

void DEVICE::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_via_debugger_data8(uint32_t addr)
{
	return 0xff;
}

void DEVICE::write_via_debugger_data16(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_via_debugger_data16(uint32_t addr)
{
	return 0xffff;
}

void DEVICE::write_via_debugger_data32(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_via_debugger_data32(uint32_t addr)
{
	return 0xffffffff;
}

void DEVICE::write_via_debugger_data8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
}

uint32_t DEVICE::read_via_debugger_data8w(uint32_t addr, int* wait)
{
	*wait = 0;
	return 0xff;
}

void DEVICE::write_via_debugger_data16w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
}

uint32_t DEVICE::read_via_debugger_data16w(uint32_t addr, int* wait)
{
	*wait = 0;
	return 0xffff;
}

void DEVICE::write_via_debugger_data32w(uint32_t addr, uint32_t data, int* wait)
{
}

uint32_t DEVICE::read_via_debugger_data32w(uint32_t addr, int* wait)
{
	*wait = 0;
	return 0xffffffff;
}

void DEVICE::write_via_debugger_io8(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_via_debugger_io8(uint32_t addr)
{
	return 0xff;
}

void DEVICE::write_via_debugger_io16(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_via_debugger_io16(uint32_t addr)
{
	return 0xffff;
}

void DEVICE::write_via_debugger_io32(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_via_debugger_io32(uint32_t addr)
{
	return 0xffffffff;
}

void DEVICE::write_via_debugger_io8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
//	write_io8w(addr, data, wait);
}

uint32_t DEVICE::read_via_debugger_io8w(uint32_t addr, int* wait)
{
	// return read_io8w(addr, wait);
	*wait = 0;
	return 0xff;
}

void DEVICE::write_via_debugger_io16w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
}

uint32_t DEVICE::read_via_debugger_io16w(uint32_t addr, int* wait)
{
	*wait = 0;
	return 0xffff;
}
void DEVICE::write_via_debugger_io32w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
}

uint32_t DEVICE::read_via_debugger_io32w(uint32_t addr, int* wait)
{
	*wait = 0;
	return 0xffffffff;
}

bool DEVICE::address_translate_for_bios(int space, int intention, uint64_t &taddress)
{
	return true; // If don't present address translation, translation succeed.
}

const _TCHAR *DEVICE::get_lib_common_vm_version(void)
{
#if defined(__LIBRARY_NAME)
	return (const _TCHAR *)__LIBRARY_NAME;
#else
	return (const _TCHAR *)"\0";
#endif
}


void DEVICE::update_config()
{
}

void DEVICE::save_state(FILEIO* state_fio)
{
}

bool DEVICE::load_state(FILEIO* state_fio)
{
	return true;
}

bool DEVICE::process_state(FILEIO* state_fio, bool loading)
{
	if(loading) {
		return load_state(state_fio);
	} else {
		save_state(state_fio);
		return true;
	}
}
