/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ device base class ]
*/

#ifndef _DEVICE_H_
#define _DEVICE_H_
/*!
 * @file device.h
 * @brief BASE of all virtual devices.
 * @author Takeda.Toshiya
 * @author Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
 * @date 2006.08.18-
 */

#include <stdarg.h>
#include "vm_template.h"
#include "../emu_template.h"
#if defined(_USE_QT)
#include "osd_base.h"
#include "csp_logger.h"
#define USE_DEVICE_NAME
#endif

#include "./device_params.h"

#if defined(USE_SHARED_DLL)
#include "libcpu_newdev/device.h"

#else
#if defined(_USE_QT)
class CSP_Logger;
extern CSP_Logger *csp_logger;
#endif
/*!
 * @class DEVICE
 * @brief BASE CLASS of all devices.
 */
class DEVICE
{
protected:
	VM_TEMPLATE* vm;		//!< POINTER OF LINKED VIRTUAL MACHINE
	EMU_TEMPLATE* emu;		//!< POINTER OF EMULATION CORE
	OSD_BASE* osd;			//!< POINTER OF OSD (OS DEPENDED part)
#if defined(_USE_QT)
	CSP_Logger *p_logger;	//!< POINTER OF LOGGING LOGGER
#endif
public:
	/*!
	 * @brief Constructor
	 * @param parent_vm pointer of virtual machine.
	 * @param parent_emu pointer of emulation core.
	 */
	DEVICE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : vm(parent_vm), emu(parent_emu)
	{
#if defined(_USE_QT)
		osd = emu->get_osd();
		p_logger = csp_logger;
#else
		osd = NULL;
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
	/*!
	 * @brief Destructor
	 * @todo Will implement real destructor per real classes and below destructor decl. with "virtual".
	 * This makes warning:
	 * "deleting object of polymorphic class type 'DEVICE' which has non-virtual
	 *  destructor might cause undefined behavior [-Wdelete-non-virtual-dtor]".
	 */
	~DEVICE(void) {}
	
	/*!
	 * @brief Initialize a DEVICE.
	 * @note Initializing VM must be after initializing OSD.
	 * @note You may call DEVICE::initialize() FOO::initialize() of inherited class.
	 */ 
	virtual void initialize() {	/* Initializing VM must be after initializing OSD. */ }
	/*!
	 * @brief De-Allocate resources before DESTRUCTION this class.
	 * @note  Normally call before deleting this device automatically.
	 */
	virtual void release() {}
	//!< Sound input functions
	/*!
	 * @brief clear a temporaly sound buffer to zero. 
	 * @param bank bank number of temporaly sound buffer.
	 */
	virtual void clear_sound_in_source(int bank) {
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->clear_sound_in_source(bank);
	}
	/*!
	 * @brief add a temporaly sound buffer
	 * @param rate sampling rate of this buffer.
	 * @param samples samples of this buffer.
	 * @param channels channels of this buffer.
	 * @return bank number of temporaly sound buffer.
	 * @retval -1 failed to add a buffer.
	 * @note this function may be before (or after) initialize().
	 */
	virtual int add_sound_in_source(int rate, int samples, int channels) {
		if(event_manager == NULL) return -1;
		return event_manager->add_sound_in_source(rate, samples, channels);
	}
	/*!
	 * @brief relsease a temporaly sound buffer
	 * @param bank number wish to release.
	 * @return bank number of released.
	 * @retval -1 failed to release a buffer.
	 * @note this function may be before (or after) initialize().
	 */
	virtual int release_sound_in_source(int bank) {
		if(event_manager == NULL) return -1;
		return event_manager->release_sound_in_source(bank);
	}
	
	/*!
	 * @brief check whether a temporaly sound buffer exists.
	 * @param bank number wish to check.
	 * @retval true buffer exists.
	 * @retval false buffer not exists.
	 * @note this function may be before (or after) initialize().
	 */
	virtual bool is_sound_in_source_exists(int bank) {
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->is_sound_in_source_exists(bank);
	}
	/*!
	 * @brief skip data pointer in a sound buffer
	 * @param bank buffer number wish to skip.
	 * @param passed_usec time to skip (by microseconds).
	 * @return incremented pointer value.
	 * @retval 0 maybe not succeed.
	 */
	virtual int __FASTCALL increment_sound_in_passed_data(int bank, double passed_usec) {
		if(event_manager == NULL) {
			return 0;
		}
		return event_manager->increment_sound_in_passed_data(bank, passed_usec);
	}
	/*!
	 * @brief count buffers whether data exists
	 * @return count of buffers that data exists.
	 */
	virtual int get_sound_in_buffers_count() {
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_sound_in_buffers_count();
	}
	
	/*!
	 * @brief get sound samples of a buffer.
	 * @param bank bank number to check samples.
	 * @return samples of a buffer.
	 * @retval 0 if not be used this buffer.
	 */
	virtual int __FASTCALL get_sound_in_samples(int bank) {
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_sound_in_samples(bank);
	}
	/*!
	 * @brief get sound sample rate
	 * @param bank bank number to check.
	 * @return sample rate of this buffer.
	 * @retval 0 if not be used this buffer.
	 */
	virtual int __FASTCALL get_sound_in_rate(int bank) {
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_sound_in_rate(bank);
	}
	/*!
	 * @brief get sound channels
	 * @param bank bank number to check.
	 * @return channels of this buffer.
	 * @retval 0 if not be used this buffer.
	 */
	virtual int __FASTCALL get_sound_in_channels(int bank) {
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_sound_in_channels(bank);
	}
	/*!
	 * @brief get pointer of sound buffer.
	 * @param bank bank number expect to get pointer.
	 * @return pointer of this buffer.
	 * @retval nullptr if not be used this buffer.
	 * @retval NULL if not be used this buffer.
	 * @note this function may be before (or after) initialize().
	 */
	virtual int16_t* __FASTCALL get_sound_in_buf_ptr(int bank) {
		if(event_manager == NULL) return NULL;
		return event_manager->get_sound_in_buf_ptr(bank);
	}
	/*!
	 * @brief write datas to sound buffer.
	 * @param bank bank number expect to write.
	 * @param src sound data pointer wish to write.
	 * @param samples samples wish to write.
	 * @return samples succseed to write
	 * @retval 0 failed to write.
	 */
	virtual int write_sound_in_buffer(int bank, int32_t* src, int samples) {
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->write_sound_in_buffer(bank, src, samples);
		
	}
	/*!
	 * @brief get sound datas from sound buffer (add to destination).
	 * @param bank bank number expect to write.
	 * @param dst destination pointer to get sounds.
	 * @param expect_channels sound channels of destination buffer.
	 * @return samples succseed to get
	 * @retval 0 failed to get.
	 * @note Add sampled values to sample buffer.
	 * @note got value may be clampped -32768 to +32767.
	 * @note this function may be before (or after) initialize().
	 */
	virtual int __FASTCALL get_sound_in_latest_data(int bank, int32_t* dst, int expect_channels) {
		if(event_manager == NULL) return 0;
		return event_manager->get_sound_in_latest_data(bank, dst, expect_channels);
	}
	
	/*!
	 * @brief get sound datas from sound buffer with conversion sample rate (add to destination).
	 * @param bank bank number expect to write.
	 * @param dst destination pointer to get sounds.
	 * @param expect_samples samples of destination buffer.
	 * @param expect_rate sample rate of destination buffer.
	 * @param expect_channels sound channels of destination buffer.
	 * @return samples succseed to get
	 * @retval 0 failed to get.
	 * @note Add sampled values to sample buffer.
	 * @note got value may be clampped -32768 to +32767.
	 * @note this function may be before (or after) initialize().
	 */
	virtual int __FASTCALL get_sound_in_data(int bank, int32_t* dst, int expect_samples, int expect_rate, int expect_channels) {
		if(event_manager == NULL) return -1;
		return event_manager->get_sound_in_data(bank, dst, expect_samples, expect_rate, expect_channels);
	}

	/*!
	 * @brief set parameters sound high pass filter.
	 * @param freq high limit frequency.
	 * @param quality quality parameter of high pass filter.
	 */
	virtual void set_high_pass_filter_freq(int freq, double quality) { } // If freq < 0 disable HPF.
	/*!
	 * @brief set parameters sound low pass filter.
	 * @param freq low limit frequency.
	 * @param quality quality parameter of low pass filter.
	 */
	virtual void set_low_pass_filter_freq(int freq, double quality) { }  // If freq <= 0 disable LPF.

	/*!
	 * @brief update configuration parameters.
	 * @note maybe call after update value(s) of config.foo .
	 */
	virtual void update_config() {}
	/*!
	 * @brief save state values of this device to a file.
	 * @param state_fio pointer of state file i/o.
	 * @note may not use this normally, use process_state() instead of this.
	 */
	virtual void save_state(FILEIO* state_fio) {}
	/*!
	 * @brief load state values of this device from a file.
	 * @param state_fio pointer of state file i/o.
	 * @return loading status
	 * @retval true succeeded to load state values.
	 * @retval false failed to load state values.
	 * @note may not use this normally, use process_state() instead of this.
	 */
	virtual bool load_state(FILEIO* state_fio)
	{
		return true;
	}
	// control
	/*!
	 * @brief reset this device.
	 * @note call within VM::reset() automatically.
	 */
	virtual void reset() {}
	/*!
	 * @brief reset this device with special feature.
	 * @param num type of special feature.
	 * @note call within VM::special_reset() automatically.
	 */
	virtual void special_reset(int num)
	{
		reset();
	}
	/*!
	 * @brief load/save state values of this device from a file.
	 * @param state_fio pointer of state file i/o.
	 * @param loading true for loading , false for saving state values.
	 * @return processing status
	 * @retval true succeeded to load/save state values.
	 * @retval false failed to load/save state values.
	 * @note normally returns true when saving.
	 */
	virtual bool process_state(FILEIO* state_fio, bool loading)
	{
		if(loading) {
			return load_state(state_fio);
		} else {
			save_state(state_fio);
			return true;
		}
	}
	
	/*!
	 * @brief get linear address from segment:offset model.
	 * @param segment number of segment.
	 * @param offset offset address of segment.
	 * @return linear address of calculated.
	 * @note this is used from pseudo-bios (with ix86).
	 * @note segment param must be numeric, not value
	 * @note the virtual bus interface functions for 16/32bit access invite the cpu is little endian.
	 * @note if the cpu is big endian, you need to implement them in the virtual machine memory/io classes.
	 * @deprecated this function will not use from bios
	 */
	virtual uint32_t __FASTCALL translate_address(int segment, uint32_t offset) { return offset; }
	
	//<! memory bus
	/*!
	 * @brief write a 8bit width data to memory .
	 * @param addr address to write.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_data8(uint32_t addr, uint32_t data) {}
	/*!
	 * @brief read a 8bit width data from memory .
	 * @param addr address to read.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_data8(uint32_t addr)
	{
		return 0xff;
	}
	/*!
	 * @brief write a 16bit width data to memory .
	 * @param addr address to write.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_data16(uint32_t addr, uint32_t data)
	{
		write_data8(addr,     (data     ) & 0xff);
		write_data8(addr + 1, (data >> 8) & 0xff);
	}
	/*!
	 * @brief read a 16bit width data from memory .
	 * @param addr address to read.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_data16(uint32_t addr)
	{
		uint32_t val;
		val  = read_data8(addr    );
		val |= read_data8(addr + 1) << 8;
		return val;
	}
	/*!
	 * @brief write a 32bit width data to memory .
	 * @param addr address to write.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_data32(uint32_t addr, uint32_t data)
	{
		if(!(addr & 1)) {
			write_data16(addr,     (data      ) & 0xffff);
			write_data16(addr + 2, (data >> 16) & 0xffff);
		} else {
			write_data8 (addr,     (data      ) & 0x00ff);
			write_data16(addr + 1, (data >>  8) & 0xffff);
			write_data8 (addr + 3, (data >> 24) & 0x00ff);
		}
	}
	/*!
	 * @brief read a 32bit width data from memory .
	 * @param addr address to read.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_data32(uint32_t addr)
	{
		if(!(addr & 1)) {
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
	/*!
	 * @brief write a 8bit width data to memory with wait value.
	 * @param addr address to write.
	 * @param data  value to write.
	 * @param wait pointer to get memory wait value of this address.
	 */
	virtual void __FASTCALL write_data8w(uint32_t addr, uint32_t data, int* wait)
	{
		*wait = 0;
		write_data8(addr, data);
	}
	/*!
	 * @brief read a 8bit width data from memory with wait value.
	 * @param addr address to read.
	 * @param wait pointer to get memory wait value of this address.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_data8w(uint32_t addr, int* wait)
	{
		*wait = 0;
		return read_data8(addr);
	}
	/*!
	 * @brief write a 16bit width data to memory with wait value.
	 * @param addr address to write.
	 * @param data value to write.
	 * @param wait pointer to get memory wait value of this address.
	 */
	virtual void __FASTCALL write_data16w(uint32_t addr, uint32_t data, int* wait)
	{
		int wait_0, wait_1;
		write_data8w(addr,     (data     ) & 0xff, &wait_0);
		write_data8w(addr + 1, (data >> 8) & 0xff, &wait_1);
		*wait = wait_0 + wait_1;
	}
	/*!
	 * @brief read a 16bit width data from memory with wait value.
	 * @param addr address to read.
	 * @param wait pointer to get memory wait value of this address.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_data16w(uint32_t addr, int* wait)
	{
		int wait_0, wait_1;
		uint32_t val;
		val  = read_data8w(addr,     &wait_0);
		val |= read_data8w(addr + 1, &wait_1) << 8;
		*wait = wait_0 + wait_1;
		return val;
	}
	/*!
	 * @brief write a 32bit width data to memory with wait value.
	 * @param addr address to write.
	 * @param data value to write.
	 * @param wait pointer to get memory wait value of this address.
	 */
	virtual void __FASTCALL write_data32w(uint32_t addr, uint32_t data, int* wait)
	{
		if(!(addr & 1)) {
			int wait_0, wait_1;
			write_data16w(addr,     (data      ) & 0xffff, &wait_0);
			write_data16w(addr + 2, (data >> 16) & 0xffff, &wait_1);
			*wait = wait_0 + wait_1;
		} else {
			int wait_0, wait_1, wait_2;
			write_data8w (addr,     (data      ) & 0x00ff, &wait_0);
			write_data16w(addr + 1, (data >>  8) & 0xffff, &wait_1);
			write_data8w (addr + 3, (data >> 24) & 0x00ff, &wait_2);
			*wait = wait_0 + wait_1 + wait_2;
		}
	}
	/*!
	 * @brief read a 32bit width data from memory with wait value.
	 * @param addr address to read.
	 * @param wait pointer to get memory wait value of this address.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_data32w(uint32_t addr, int* wait)
	{
		if(!(addr & 1)) {
			int wait_0, wait_1;
			uint32_t val;
			val  = read_data16w(addr,     &wait_0);
			val |= read_data16w(addr + 2, &wait_1) << 16;
			*wait = wait_0 + wait_1;
			return val;
		} else {
			int wait_0, wait_1, wait_2;
			uint32_t val;
			val  = read_data8w (addr,     &wait_0);
			val |= read_data16w(addr + 1, &wait_1) <<  8;
			val |= read_data8w (addr + 3, &wait_2) << 24;
			*wait = wait_0 + wait_1 + wait_2;
			return val;
		}
	}
	/*!
	 * @brief fetch a opcode from memory
	 * @param addr address to fetch.
	 * @param wait pointer to get memory wait value of this address.
	 * @return fetched value.
	 * @note normally fetced value has 8bit width.
	 */
	virtual uint32_t __FASTCALL fetch_op(uint32_t addr, int *wait)
	{
		return read_data8w(addr, wait);
	}
	/*!
	 * @brief transfer 8bit width data from dma to memory.
	 * @param addr memory address to write.
	 * @param data value to transfer.
	 */
	virtual void __FASTCALL write_dma_data8(uint32_t addr, uint32_t data)
	{
		write_data8(addr, data);
	}
	/*!
	 * @brief transfer 8bit width data from memory to dma.
	 * @param addr memory address to read.
	 * @return value to transfer.
	 */
	virtual uint32_t __FASTCALL read_dma_data8(uint32_t addr)
	{
		return read_data8(addr);
	}
	/*!
	 * @brief transfer 16bit width data from dma to memory.
	 * @param addr memory address to write.
	 * @param data value to transfer.
	 */
	virtual void __FASTCALL write_dma_data16(uint32_t addr, uint32_t data)
	{
		write_data16(addr, data);
	}
	/*!
	 * @brief transfer 16bit width data from memory to dma.
	 * @param addr memory address to read.
	 * @return value to transfer.
	 */
	virtual uint32_t __FASTCALL read_dma_data16(uint32_t addr)
	{
		return read_data16(addr);
	}
	/*!
	 * @brief transfer 32bit width data from dma to memory.
	 * @param addr memory address to write.
	 * @param data value to transfer.
	 */
	virtual void __FASTCALL write_dma_data32(uint32_t addr, uint32_t data)
	{
		write_data32(addr, data);
	}
	/*!
	 * @brief transfer 32bit width data from memory to dma.
	 * @param addr memory address to read.
	 * @return value to transfer.
	 */
	virtual uint32_t __FASTCALL read_dma_data32(uint32_t addr)
	{
		return read_data32(addr);
	}
	/*!
	 * @brief transfer 8bit width data from dma to memory with wait value.
	 * @param addr memory address to write.
	 * @param data value to transfer.
	 * @param wait pointer of got wait value.
	 */
	virtual void __FASTCALL write_dma_data8w(uint32_t addr, uint32_t data, int* wait)
	{
		write_data8w(addr, data, wait);
	}
	/*!
	 * @brief transfer 8bit width data from memory to dma with wait value.
	 * @param addr memory address to transfer.
	 * @param wait pointer of got wait value.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_dma_data8w(uint32_t addr, int* wait)
	{
		return read_data8w(addr, wait);
	}
	/*!
	 * @brief transfer 16bit width data from dma to memory with wait value.
	 * @param addr memory address to write.
	 * @param data value to transfer.
	 * @param wait pointer of got wait value.
	 */
	virtual void __FASTCALL write_dma_data16w(uint32_t addr, uint32_t data, int* wait)
	{
		write_data16w(addr, data, wait);
	}
	/*!
	 * @brief transfer 16bit width data from memory to dma with wait value.
	 * @param addr memory address to transfer.
	 * @param wait pointer of got wait value.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_dma_data16w(uint32_t addr, int* wait)
	{
		return read_data16w(addr, wait);
	}
	/*!
	 * @brief transfer 32bit width data from dma to memory with wait value.
	 * @param addr memory address to write.
	 * @param data value to transfer.
	 * @param wait pointer of got wait value.
	 */
	virtual void __FASTCALL write_dma_data32w(uint32_t addr, uint32_t data, int* wait)
	{
		write_data32w(addr, data, wait);
	}
	/*!
	 * @brief transfer 32bit width data from memory to dma with wait value.
	 * @param addr memory address to transfer.
	 * @param wait pointer of got wait value.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_dma_data32w(uint32_t addr, int* wait)
	{
		return read_data32w(addr, wait);
	}
	
	//<! i/o bus
	/*!
	 * @brief write 8bit width data to I/O bus.
	 * @param addr address of I/O port.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data) {}
	/*!
	 * @brief read 8bit width data from I/O bus.
	 * @param addr address of I/O port.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_io8(uint32_t addr)
	{
#ifdef IOBUS_RETURN_ADDR
		return (addr & 1 ? addr >> 8 : addr) & 0xff;
#else
		return 0xff;
#endif
	}
	/*!
	 * @brief write 16bit width data to I/O bus.
	 * @param addr address of I/O port.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_io16(uint32_t addr, uint32_t data)
	{
		write_io8(addr    , (data     ) & 0xff);
		write_io8(addr + 1, (data >> 8) & 0xff);
	}
	/*!
	 * @brief read 16bit width data from I/O bus.
	 * @param addr address of I/O port.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_io16(uint32_t addr)
	{
		uint32_t val;
		val  = read_io8(addr    );
		val |= read_io8(addr + 1) << 8;
		return val;
	}
	/*!
	 * @brief write 32bit width data to I/O bus.
	 * @param addr address of I/O port.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_io32(uint32_t addr, uint32_t data)
	{
		if(!(addr & 1)) {
			write_io16(addr,     (data      ) & 0xffff);
			write_io16(addr + 2, (data >> 16) & 0xffff);
		} else {
			write_io8 (addr,     (data      ) & 0x00ff);
			write_io16(addr + 1, (data >>  8) & 0xffff);
			write_io8 (addr + 3, (data >> 24) & 0x00ff);
		}
	}
	/*!
	 * @brief read 32bit width data from I/O bus.
	 * @param addr address of I/O port.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_io32(uint32_t addr)
	{
		if(!(addr & 1)) {
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
	/*!
	 * @brief write 8bit width data to I/O bus with wait.
	 * @param addr address of I/O port.
	 * @param wait pointer to get wait value.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_io8w(uint32_t addr, uint32_t data, int* wait)
	{
		*wait = 0;
		write_io8(addr, data);
	}
	/*!
	 * @brief read 8bit width data from I/O bus with wait.
	 * @param addr address of I/O port.
	 * @param wait pointer to get wait value.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_io8w(uint32_t addr, int* wait)
	{
		*wait = 0;
		return read_io8(addr);
	}
	/*!
	 * @brief write 16bit width data to I/O bus with wait.
	 * @param addr address of I/O port.
	 * @param wait pointer to get wait value.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_io16w(uint32_t addr, uint32_t data, int* wait)
	{
		int wait_0, wait_1;
		write_io8w(addr,     (data     ) & 0xff, &wait_0);
		write_io8w(addr + 1, (data >> 8) & 0xff, &wait_1);
		*wait = wait_0 + wait_1;
	}
	/*!
	 * @brief read 16bit width data from I/O bus with wait.
	 * @param addr address of I/O port.
	 * @param wait pointer to get wait value.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_io16w(uint32_t addr, int* wait)
	{
		int wait_0, wait_1;
		uint32_t val;
		val  = read_io8w(addr,     &wait_0);
		val |= read_io8w(addr + 1, &wait_1) << 8;
		*wait = wait_0 + wait_1;
		return val;
	}
	/*!
	 * @brief write 32bit width data to I/O bus with wait.
	 * @param addr address of I/O port.
	 * @param wait pointer to get wait value.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_io32w(uint32_t addr, uint32_t data, int* wait)
	{
		if(!(addr & 1)) {
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
	/*!
	 * @brief read 32bit width data from I/O bus with wait.
	 * @param addr address of I/O port.
	 * @param wait pointer to get wait value.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_io32w(uint32_t addr, int* wait)
	{
		if(!(addr & 1)) {
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
	/*!
	 * @brief transfer 8bit width data from dma to I/O bus.
	 * @param addr I/O address to write.
	 * @param data value to transfer.
	 */
	virtual void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data)
	{
		write_io8(addr, data);
	}
	/*!
	 * @brief transfer 8bit width data from I/O bus to dma.
	 * @param addr I/O address to read.
	 * @return value to transfer.
	 */
	virtual uint32_t __FASTCALL read_dma_io8(uint32_t addr)
	{
		return read_io8(addr);
	}
	/*!
	 * @brief transfer 16bit width data from dma to I/O bus.
	 * @param addr I/O address to write.
	 * @param data value to transfer.
	 */
	virtual void __FASTCALL write_dma_io16(uint32_t addr, uint32_t data)
	{
		write_io16(addr, data);
	}
	/*!
	 * @brief transfer 16bit width data from I/O bus to dma.
	 * @param addr I/O address to read.
	 * @return value to transfer.
	 */
	virtual uint32_t __FASTCALL read_dma_io16(uint32_t addr)
	{
		return read_io16(addr);
	}
	/*!
	 * @brief transfer 32bit width data from dma to I/O bus.
	 * @param addr I/O address to write.
	 * @param data value to transfer.
	 */
	virtual void __FASTCALL write_dma_io32(uint32_t addr, uint32_t data)
	{
		write_io32(addr, data);
	}
	/*!
	 * @brief transfer 32bit width data from I/O bus to dma.
	 * @param addr I/O address to read.
	 * @return value to transfer.
	 */
	virtual uint32_t __FASTCALL read_dma_io32(uint32_t addr)
	{
		return read_io32(addr);
	}
	virtual void __FASTCALL write_dma_io8w(uint32_t addr, uint32_t data, int* wait)
	{
		write_io8w(addr, data, wait);
	}
	virtual uint32_t __FASTCALL read_dma_io8w(uint32_t addr, int* wait)
	{
		return read_io8w(addr, wait);
	}
	virtual void __FASTCALL write_dma_io16w(uint32_t addr, uint32_t data, int* wait)
	{
		write_io16w(addr, data, wait);
	}
	virtual uint32_t __FASTCALL read_dma_io16w(uint32_t addr, int* wait)
	{
		return read_io16w(addr, wait);
	}
	virtual void __FASTCALL write_dma_io32w(uint32_t addr, uint32_t data, int* wait)
	{
		write_io32w(addr, data, wait);
	}
	virtual uint32_t __FASTCALL read_dma_io32w(uint32_t addr, int* wait)
	{
		return read_io32w(addr, wait);
	}
	
	// memory mapped i/o
	virtual void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data)
	{
		write_io8(addr, data);
	}
	virtual uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr)
	{
		return read_io8(addr);
	}
	virtual void __FASTCALL write_memory_mapped_io16(uint32_t addr, uint32_t data)
	{
		write_memory_mapped_io8(addr,     (data     ) & 0xff);
		write_memory_mapped_io8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32_t __FASTCALL read_memory_mapped_io16(uint32_t addr)
	{
		uint32_t val;
		val =  read_memory_mapped_io8(addr    );
		val |= read_memory_mapped_io8(addr + 1) << 8;
		return val;
	}
	virtual void __FASTCALL write_memory_mapped_io32(uint32_t addr, uint32_t data)
	{
		if(!(addr & 1)) {
			write_memory_mapped_io16(addr,     (data      ) & 0xffff);
			write_memory_mapped_io16(addr + 2, (data >> 16) & 0xffff);
		} else {
			write_memory_mapped_io8 (addr,     (data      ) & 0x00ff);
			write_memory_mapped_io16(addr + 1, (data >>  8) & 0xffff);
			write_memory_mapped_io8 (addr + 3, (data >> 24) & 0x00ff);
		}
	}
	virtual uint32_t __FASTCALL read_memory_mapped_io32(uint32_t addr)
	{
		if(!(addr & 1)) {
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
	virtual void __FASTCALL write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait)
	{
		*wait = 0;
		write_memory_mapped_io8(addr, data);
	}
	virtual uint32_t __FASTCALL read_memory_mapped_io8w(uint32_t addr, int* wait)
	{
		*wait = 0;
		return read_memory_mapped_io8(addr);
	}
	virtual void __FASTCALL write_memory_mapped_io16w(uint32_t addr, uint32_t data, int* wait)
	{
		int wait_0, wait_1;
		write_memory_mapped_io8w(addr,     (data     ) & 0xff, &wait_0);
		write_memory_mapped_io8w(addr + 1, (data >> 8) & 0xff, &wait_1);
		*wait = wait_0 + wait_1;
	}
	virtual uint32_t __FASTCALL  read_memory_mapped_io16w(uint32_t addr, int* wait)
	{
		int wait_0, wait_1;
		uint32_t val;
		val  = read_memory_mapped_io8w(addr,     &wait_0);
		val |= read_memory_mapped_io8w(addr + 1, &wait_1) << 8;
		*wait = wait_0 + wait_1;
		return val;
	}
	virtual void __FASTCALL write_memory_mapped_io32w(uint32_t addr, uint32_t data, int* wait)
	{
		if(!(addr & 1)) {
			int wait_0, wait_1;
			write_memory_mapped_io16w(addr,     (data      ) & 0xffff, &wait_0);
			write_memory_mapped_io16w(addr + 2, (data >> 16) & 0xffff, &wait_1);
			*wait = wait_0 + wait_1;
		} else {
			int wait_0, wait_1, wait_2;
			write_memory_mapped_io8w (addr,     (data      ) & 0x00ff, &wait_0);
			write_memory_mapped_io16w(addr + 1, (data >>  8) & 0xffff, &wait_1);
			write_memory_mapped_io8w (addr + 3, (data >> 24) & 0x00ff, &wait_2);
			*wait = wait_0 + wait_1 + wait_2;
		}
	}
	virtual uint32_t __FASTCALL read_memory_mapped_io32w(uint32_t addr, int* wait)
	{
		if(!(addr & 1)) {
			int wait_0, wait_1;
			uint32_t val;
			val  = read_memory_mapped_io16w(addr,     &wait_0);
			val |= read_memory_mapped_io16w(addr + 2, &wait_1) << 16;
			*wait = wait_0 + wait_1;
			return val;
		} else {
			int wait_0, wait_1, wait_2;
			uint32_t val;
			val  = read_memory_mapped_io8w (addr,     &wait_0);
			val |= read_memory_mapped_io16w(addr + 1, &wait_1) <<  8;
			val |= read_memory_mapped_io8w (addr + 3, &wait_2) << 24;
			*wait = wait_0 + wait_1 + wait_2;
			return val;
		}
	}
	
	//<! device to device
	/*!
	 * @struct output_t
	 * @brief abstruction of output port.
	 */
	typedef struct {
		DEVICE *device;	//<! pointer of target device class
		int id;			//<! signal number to use
		uint32_t mask;	//<! value mask of this signal
		int shift;		//<! shifting value to write.
	} output_t;
	
	/*!
	 * @struct outputs_t
	 * @brief multiple output port.
	 */
	typedef struct {
		int count;					//<! count of registered devices
		output_t item[MAX_OUTPUT];	//<! output ports
	} outputs_t;
	/*!
	 * @brief initialize multiple output port
	 * @param items pointer of multiple output port.
	 * @note must call before using a multiple output port.
	 * @note items must be pointer, not be multiple output port.
	 */ 
	virtual void initialize_output_signals(outputs_t *items)
	{
		items->count = 0;
	}
	/*!
	 * @brief register a port to multiple output port
	 * @param items pointer of multiple output port.
	 * @param device pointer of target device (to write signal value).
	 * @param id signal number for this port.
	 * @param mask bit mask for this port.
	 * @param shift bit shft value of this port.
	 * @note must call before using a multiple output port.
	 * @note items must be pointer, not be multiple output port.
	 */
	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask, int shift)
	{
		int c = items->count++;
		items->item[c].device = device;
		items->item[c].id = id;
		items->item[c].mask = mask;
		items->item[c].shift = shift;
	}
	/*!
	 * @brief register a port to multiple output port without bit shift
	 * @param items pointer of multiple output port.
	 * @param device pointer of target device (to write signal value).
	 * @param id signal number for this port.
	 * @param mask bit mask for this port.
	 * @note must call before using a multiple output port.
	 * @note items must be pointer, not be multiple output port.
	 */
	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask)
	{
		int c = items->count++;
		items->item[c].device = device;
		items->item[c].id = id;
		items->item[c].mask = mask;
		items->item[c].shift = 0;
	}
	/*!
	 * @brief write data to multiple output port
	 * @param items pointer of multiple output port.
	 * @param data value to write.
	 * @note items must be pointer, not be multiple output port.
	 * @note useful for batch writing signals.
	 */
	virtual void __FASTCALL write_signals(outputs_t *items, uint32_t data)
	{
		for(int i = 0; i < items->count; i++) {
			output_t *item = &items->item[i];
			int shift = item->shift;
			uint32_t val = (shift < 0) ? (data >> (-shift)) : (data << shift);
			uint32_t mask = (shift < 0) ? (item->mask >> (-shift)) : (item->mask << shift);
			item->device->write_signal(item->id, val, mask);
		}
	};
	/*!
	 * @brief update bitmask to specified device at multiple output port
	 * @param items pointer of multiple output port.
	 * @param device target device to update bitmask.
	 * @param mask bitmask to update.
	 * @note items must be pointer, not be multiple output port.
	 */
	virtual void update_signal_mask(outputs_t *items, DEVICE *device, uint32_t mask)
	{
		if(items == NULL) return;
		int c = items->count;
		if(c <= 0) return;
		if(c >= MAX_OUTPUT) c = MAX_OUTPUT - 1;
		// if (ARG:device == NULL) apply to all devices.
		for(int i = 0; i < c; i++) {
			if((device == NULL) || (device == items->item[i].device)) {
				items->item[i].mask = mask;
			}
		}
	}
	/*!
	 * @brief write signal to this device (normally from another device or VM).
	 * @param id signal number
	 * @param data value to write
	 * @param mask bitmask to write
	 */
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) {}
	/*!
	 * @brief inspect signal from this device (normally to another device or VM).
	 * @param id signal number to inspect
	 * @return inspected value
	 */
	virtual uint32_t __FASTCALL read_signal(int ch)
	{
		return 0;
	}
	
	//<! z80 daisy chain
	/*!
	 * @brief set target device for INTR signal
	 * @param device pointer of target device
	 * @param bit bitmask for target device.
	 * @note this may use for Z80 variants.
	 */
	virtual void set_context_intr(DEVICE* device, uint32_t bit) {}
	/*!
	 * @brief register child (slave) device to this device.	
	 * @param device pointer of child device.
	 */
	virtual void set_context_child(DEVICE* device) {}
	
	/*!
	 * @brief get  child (slave) device of this device.	
	 * @return pointer of child device.
	 * @retval nullptr child not exists.
	 * @retval NULL child not exists.
	 */
	virtual DEVICE *get_context_child()
	{
		return NULL;
	}
	
	/*!
	 * @brief send interrupt (IEI) signal from device to device
	 * @param val interrupt status value
	 */
	virtual void __FASTCALL set_intr_iei(bool val) {}
	
	/*!
	 * @brief send interrupt signal from device to cpu
	 * @param line signal value
	 * @param pending pending to update signal until next period.
	 * @param bit interrupt bit to update.
	 */
	virtual void __FASTCALL set_intr_line(bool line, bool pending, uint32_t bit) {}
	
	// interrupt cpu to device
	/*!
	 * @brief get acknowledge value from this device.
	 * @return acknowledge value (normally level of interrupt)
	 * @note this is used from PIC and ix86 variant CPUs.
	 */
	virtual uint32_t get_intr_ack()
	{
		return 0xff;
	}
	/*!
	 * @brief update interrupt signal (to target devices)
	 */
	virtual void update_intr() {}
	/*!
	 * @brief notify RETI signal to this device from CPU.
	 */
	virtual void notify_intr_reti() {}
	/*!
	 * @brief notify EI signal to this device from CPU.
	 */
	virtual void notify_intr_ei() {}
	
	/*!
	 * @brief do a DMA transfer cycle.
	 */
	virtual void __FASTCALL do_dma() { }
	
	//<! cpu
	/*!
	 * @brief run instruction(s) of CPU
	 * @param clock clocks count to run.
	 * @return spent clocks to run instruction(s).
	 * @note if set clock to -1, run a one instruction.
	 */
	virtual int __FASTCALL run(int clock)
	{
		// when clock == -1, run one opecode
		return (clock == -1 ? 1 : clock);
	}
	/*!
	 * @brief add extra clocks value to CPU
	 * @param clock value to add.
	 * @note normally use for waiting.
	 */
	virtual void __FASTCALL set_extra_clock(int clock) {}
	/*!
	 * @brief get extra clocks 
	 * @return extra clocks
	 */
	virtual int get_extra_clock()
	{
		return 0;
	}
	/*!
	 * @brief get program counter of CPU
	 * @return value of program counter
	 * @note normally this value contains previous instruction address.
	 */
	virtual uint32_t get_pc()
	{
		return 0;
	}
	/*!
	 * @brief get next program counter of CPU
	 * @return value of program counter
	 * @note normally this value contains current instruction address.
	 */
	virtual uint32_t get_next_pc()
	{
		return 0;
	}
	
	//<! bios
	/*!
	 * @brief hook of pseudo bios for i86 (16bit) variants
	 * @param PC program counter value from CPU
	 * @param regs array of general purpose registes
	 * @param sregs array of segment registers
	 * @param ZeroFlag pointer to get ZeroFlag
	 * @param CarryFlag pointer to get CarryFlag
	 * @param cycles pointer to get spent clock cycles
	 * @param total_cycles pointer to calculate clock cycles (of this period).
	 * @retval true if use pseudo bios, not process native CPU instructions more.
	 */
	virtual bool bios_call_far_i86(uint32_t PC, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
	{
		return false;
	}
	/*!
	 * @brief hook of INTxx pseudo bios for i86 (16bit) variants
	 * @param intnum number of software interrupt 
	 * @param regs array of general purpose registes
	 * @param sregs array of segment registers
	 * @param ZeroFlag pointer to get ZeroFlag
	 * @param CarryFlag pointer to get CarryFlag
	 * @param cycles pointer to get spent clock cycles
	 * @param total_cycles pointer to calculate clock cycles (of this period).
	 * @retval true if use pseudo bios, not process native CPU instructions more.
	 */
	virtual bool bios_int_i86(int intnum, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
	{
		return false;
	}
	/*!
	 * @brief hook of pseudo bios for i386 (32bit) variants
	 * @param PC program counter value from CPU
	 * @param regs array of general purpose registes
	 * @param sregs array of segment registers
	 * @param ZeroFlag pointer to get ZeroFlag
	 * @param CarryFlag pointer to get CarryFlag
	 * @param cycles pointer to get spent clock cycles
	 * @param total_cycles pointer to calculate clock cycles (of this period).
	 * @retval true if use pseudo bios, not process native CPU instructions more.
	 */
	virtual bool bios_call_far_ia32(uint32_t PC, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
	{
		return false;
	}
	/*!
	 * @brief hook of INTxx pseudo bios for i386 (32bit) variants
	 * @param intnum number of software interrupt 
	 * @param regs array of general purpose registes
	 * @param sregs array of segment registers
	 * @param ZeroFlag pointer to get ZeroFlag
	 * @param CarryFlag pointer to get CarryFlag
	 * @param cycles pointer to get spent clock cycles
	 * @param total_cycles pointer to calculate clock cycles (of this period).
	 * @retval true if use pseudo bios, not process native CPU instructions more.
	 */
	virtual bool bios_int_ia32(int intnum, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles)
	{
		return false;
	}
	/*!
	 * @brief hook of pseudo bios for Z80 variants
	 * @param PC value of PC
	 * @param af pointer of AF register
	 * @param bc pointer of BC register
	 * @param de pointer of DE register
	 * @param hl pointer of HL register
	 * @param ix pointer of IX register
	 * @param iy pointer of IY register
	 * @param iff1 pointer of IFF1 intrerrupt flags
	 * @retval true if use pseudo bios, not process native CPU instructions more.
	 */
	virtual bool bios_ret_z80(uint16_t PC, pair32_t* af, pair32_t* bc, pair32_t* de, pair32_t* hl, pair32_t* ix, pair32_t* iy, uint8_t* iff1)	{
		return false;
	}
	virtual bool __FASTCALL address_translate(int space, int intention, uint64_t &taddress) { return true; /* If not present, always succeeded.*/ }
	// misc
	/*!
	 * @brief get name of this device
	 * @return pointer of name
	 */
	const _TCHAR *get_device_name(void)
	{
		return (const _TCHAR *)this_device_name;
	}
   
	// event manager
	DEVICE* event_manager;
	/*!
	 * @brief set event manager of this device
	 * @param device pointer if event manager.
	 */
	virtual void set_context_event_manager(DEVICE* device)
	{
		event_manager = device;
	}
	/*!
	 * @brief get device id of event manager
	 * @return device id of event manager
	 */
	virtual int get_event_manager_id()
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->this_device_id;
	}
	/*!
	 * @brief get cpu clocks for event manager
	 * @return cpu clocks value (by HZ).
	 */
	virtual uint32_t get_event_clocks()
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_event_clocks();
	}
	/*!
	 * @brief check target is primary CPU of this event manager
	 * @param device target device to check
	 * @retval true taget is primary CPU.
	 */
	virtual bool is_primary_cpu(DEVICE* device)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->is_primary_cpu(device);
	}
	/*!
	 * @brief get cpu clocks of target device
	 * @param device target device
	 * @return cpu clocks value (by HZ).
	 * @retval CPU_CLOCKS when target is *not* CPU 
	 */
	virtual uint32_t get_cpu_clocks(DEVICE* device)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_cpu_clocks(device);
	}
	/*!
	 * @brief process extra events by clocks.
	 * @param clock clocks period for primary CPU.
	 * @note this is called from primary cpu while running one opecode
	*/
	virtual void __FASTCALL update_extra_event(int clock)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->update_extra_event(clock);
	}
	/*!
	 * @brief register a event of device to event manager by microseconds.
	 * @param device target device (normally this)
	 * @param event_id event id number to register
	 * @param usec delay time by microseconds
	 * @param loop true if repeat to call event
	 * @param register_id pointer of registered id
	 * @note works even register_id is nullptr
	 */
	virtual void register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->register_event(device, event_id, usec, loop, register_id);
	}
	/*!
	 * @brief register a event of device to event manager by CPU clocks.
	 * @param device target device (normally this)
	 * @param event_id event id number to register
	 * @param clock delay time by clocks
	 * @param loop true if repeat to call event
	 * @param register_id pointer of registered id
	 * @note works even register_id is nullptr
	 */
	virtual void register_event_by_clock(DEVICE* device, int event_id, uint64_t clock, bool loop, int* register_id)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->register_event_by_clock(device, event_id, clock, loop, register_id);
	}
	/*!
	 * @brief unregister a event
	 * @param device target device to unregister a event, normally this
	 * @param register_id event id expect to unregister
	 */
	virtual void cancel_event(DEVICE* device, int register_id)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->cancel_event(device, register_id);
	}
	/*!
	 * @brief unregister a event and clear event variable
	 * @param dev target device to unregister a event, normally this
	 * @param evid event id variable to unregister
	 */
	virtual void clear_event(DEVICE* dev, int& evid)
	{
		if(evid > -1) {
			cancel_event(dev, evid);
		}
		evid = -1;
	}
	/*!
	 * @brief unregister a event, then register a event
	 * @param device target device (normally this)
	 * @param event_num event id number to register
	 * @param usec delay time by microseconds
	 * @param loop true if repeat to call event
	 * @param evid event variable
	 */
	virtual void force_register_event(DEVICE* dev, int event_num, double usec, bool loop, int& evid)
	{
		clear_event(dev, evid);
		register_event(dev, event_num, usec, loop, &evid);
	}
	/*!
	 * @brief unregister a event, then register a event by clocks
	 * @param device target device (normally this)
	 * @param event_num event id number to register
	 * @param clock delay time by cpu clocks
	 * @param loop true if repeat to call event
	 * @param evid event variable
	 */
	virtual void force_register_event_by_clock(DEVICE* dev, int event_num, uint64_t clock, bool loop, int& evid)
	{
		clear_event(dev, evid);
		register_event_by_clock(dev, event_num, clock, loop, &evid);
	}

	// Register a EVENT to evid , if evid slot isn't used.
	/*!
	 * @brief register a event of device to event manager if not regsiterred by microseconds.
	 * @param device target device (normally this)
	 * @param event_num event id number to register
	 * @param usec delay time by mycroseconds
	 * @param loop true if repeat to call event
	 * @param evid registered id variable
	 */
	virtual void check_and_update_event(DEVICE* dev, int event_num, double usec, bool loop, int& evid)
	{
		if(evid > -1) return;
		register_event(dev, event_num, usec, loop, &evid);
	}
	/*!
	 * @brief register a event of device to event manager if not regsiterred by CPU clocks.
	 * @param device target device (normally this)
	 * @param event_num event id number to register
	 * @param clock delay time by clocks
	 * @param loop true if repeat to call event
	 * @param evid registered id variable
	 */
	virtual void check_and_update_event_by_clock(DEVICE* dev, int event_num, uint64_t clock, bool loop, int& evid)
	{
		if(evid > -1) return;
		register_event_by_clock(dev, event_num, clock, loop, &evid);
	}

	/*!
	 * @brief register a device for event at start of frame
	 * @param device target device, normally this
	 */
	virtual void register_frame_event(DEVICE* device)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->register_frame_event(device);
	}
	/*!
	 * @brief register a device for event at every start of vertical line
	 * @param device target device, normally this
	 */
	virtual void register_vline_event(DEVICE* device)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->register_vline_event(device);
	}
	virtual uint32_t __FASTCALL get_event_remaining_clock(int register_id)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_event_remaining_clock(register_id);
	}
	virtual double __FASTCALL get_event_remaining_usec(int register_id)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_event_remaining_usec(register_id);
	}
	virtual uint32_t get_current_clock()
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_current_clock();
	}
	virtual uint32_t __FASTCALL get_passed_clock(uint32_t prev)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_passed_clock(prev);
	}
	virtual double __FASTCALL get_passed_usec(uint32_t prev)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_passed_usec(prev);
	}
	virtual uint32_t get_passed_clock_since_vline()
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_passed_clock_since_vline();
	}
	virtual double get_passed_usec_since_vline()
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_passed_usec_since_vline();
	}
	/*!
	 * @brief get current vertical line position
	 * @return vertical line position
	 * @retval 0 if start of frame or out of frame.
	 */
	virtual int get_cur_vline()
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_cur_vline();
	}
	virtual int get_cur_vline_clocks()
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_cur_vline_clocks();
	}
	virtual uint32_t __FASTCALL get_cpu_pc(int index)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_cpu_pc(index);
	}
	virtual uint64_t get_current_clock_uint64()
	{ 
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_current_clock_uint64();
	}
	virtual uint32_t __FASTCALL get_cpu_clock(int index)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_cpu_clock(index);
	}
	virtual void request_skip_frames()
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->request_skip_frames();
	}
	virtual void set_frames_per_sec(double frames)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->set_frames_per_sec(frames);
	}
	virtual void set_lines_per_frame(int lines)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->set_lines_per_frame(lines);
	}
	virtual int get_lines_per_frame()
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		return event_manager->get_lines_per_frame();
	}
	/*!
	  @brief Force render sound immediately when device's status has changed.
	  @note You must call this after you changing registers (or enything).
	  @note If has problems, try set_realtime_render.
	  @note See mb8877.cpp and ym2203.cpp. 
	  @note -- 20161010 K.O
	*/
	virtual void touch_sound(void)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		event_manager->touch_sound();
	}
	/*!
	  @brief set to render per 1 sample automatically.
	  @param device pointer of target device
	  @param flag force to render per 1 sample.
	  @note See pcm1bit.cpp.
	  @note -- 20161010 K.O
	*/
	virtual void set_realtime_render(DEVICE* device, bool flag = true)
	{
		if(event_manager == NULL) {
			event_manager = vm->first_device->next_device;
		}
		if(device != event_manager) event_manager->set_realtime_render(device, flag);
	}
	/*!
	  @brief set to render per 1 sample automatically for this device.
	  @param flag force to render per 1 sample.
	*/
	virtual void set_realtime_render(bool flag)
	{
		set_realtime_render(this, flag);
	}
	virtual void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame) {}
	
	// event callback
	virtual void __FASTCALL event_callback(int event_id, int err) {}
	virtual void event_pre_frame() {}	// this event is to update timing settings
	virtual void event_frame() {}
	virtual void event_vline(int v, int clock) {}
	virtual void event_hsync(int v, int h, int clock) {}
	
	// sound
	virtual void __FASTCALL mix(int32_t* buffer, int cnt) {}
	virtual void set_volume(int ch, int decibel_l, int decibel_r) {} // +1 equals +0.5dB (same as fmgen)
	/*!
	  @brief set name of this device
	  @param format name of this device (printf style)
	*/
	virtual void set_device_name(const _TCHAR *format, ...)
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
/*
	These functions are used for debugging non-cpu device
	Insert debugger between standard read/write functions and these functions for checking breakpoints

	void DEVICE::write_data8(uint32_t addr, uint32_t data)
	{
		if(debugger != NULL && debugger->now_device_debugging) {
			// debugger->mem = this;
			// debugger->mem->write_via_debugger_data8(addr, data)
			debugger->write_via_debugger_data8(addr, data);
		} else {
			this->write_via_debugger_data8(addr, data);
		}
	}
	void DEVICE::write_via_debugger_data8(uint32_t addr, uint32_t data)
	{
		// write memory
	}
*/
	virtual void __FASTCALL write_via_debugger_data8(uint32_t addr, uint32_t data) {}
	virtual uint32_t __FASTCALL read_via_debugger_data8(uint32_t addr)
	{
		return 0xff;
	}
	virtual void __FASTCALL write_via_debugger_data16(uint32_t addr, uint32_t data) {}
	virtual uint32_t __FASTCALL read_via_debugger_data16(uint32_t addr)
	{
		return 0xffff;
	}
	virtual void __FASTCALL write_via_debugger_data32(uint32_t addr, uint32_t data) {}
	virtual uint32_t __FASTCALL read_via_debugger_data32(uint32_t addr)
	{
		return 0xffffffff;
	}
	virtual void __FASTCALL write_via_debugger_data8w(uint32_t addr, uint32_t data, int* wait) {}
	virtual uint32_t __FASTCALL read_via_debugger_data8w(uint32_t addr, int* wait)
	{
		return 0xff;
	}
	virtual void __FASTCALL write_via_debugger_data16w(uint32_t addr, uint32_t data, int* wait) {}
	virtual uint32_t __FASTCALL read_via_debugger_data16w(uint32_t addr, int* wait)
	{
		return 0xffff;
	}
	virtual void __FASTCALL write_via_debugger_data32w(uint32_t addr, uint32_t data, int* wait) {}
	virtual uint32_t __FASTCALL read_via_debugger_data32w(uint32_t addr, int* wait)
	{
		return 0xffffffff;
	}
	virtual void __FASTCALL write_via_debugger_io8(uint32_t addr, uint32_t data) {}
	virtual uint32_t __FASTCALL read_via_debugger_io8(uint32_t addr)
	{
		return 0xff;
	}
	virtual void __FASTCALL write_via_debugger_io16(uint32_t addr, uint32_t data) {}
	virtual uint32_t __FASTCALL read_via_debugger_io16(uint32_t addr)
	{
		return 0xffff;
	}
	virtual void __FASTCALL write_via_debugger_io32(uint32_t addr, uint32_t data) {}
	virtual uint32_t __FASTCALL read_via_debugger_io32(uint32_t addr)
	{
		return 0xffffffff;
	}
	virtual void __FASTCALL write_via_debugger_io8w(uint32_t addr, uint32_t data, int* wait) {}
	virtual uint32_t __FASTCALL read_via_debugger_io8w(uint32_t addr, int* wait)
	{
		return 0xff;
	}
	virtual void __FASTCALL write_via_debugger_io16w(uint32_t addr, uint32_t data, int* wait) {}
	virtual uint32_t __FASTCALL read_via_debugger_io16w(uint32_t addr, int* wait)
	{
		return 0xffff;
	}
	virtual void __FASTCALL write_via_debugger_io32w(uint32_t addr, uint32_t data, int* wait) {}
	virtual uint32_t __FASTCALL read_via_debugger_io32w(uint32_t addr, int* wait)
	{
		return 0xffffffff;
	}
	/*!
	 @brief logging a message
	 @param fmt message to log, same as printf().
	*/
	virtual void out_debug_log(const char *fmt, ...)
	{
		char strbuf[4096];
		va_list ap;

		va_start(ap, fmt);
		vsnprintf(strbuf, 4095, fmt, ap);
		emu->out_debug_log("%s", strbuf);
		va_end(ap);
	}
	/*!
	 @brief logging a message with switch
	 @param logging logging if true, not logging if false.
	 @param fmt message to log, same as printf().
	*/
	virtual void DEVICE::out_debug_log_with_switch(bool logging, const char *fmt, ...)
	{
		if(!(logging)) return;
#if defined(_USE_QT)
		if(p_logger == NULL) return;
		char strbuf[4096];
		va_list ap;
		
		va_start(ap, fmt);
		vsnprintf(strbuf, 4095, fmt, ap);
		p_logger->debug_log(CSP_LOG_DEBUG, this_device_id + CSP_LOG_TYPE_VM_DEVICE_0, "%s", strbuf);
		va_end(ap);
#else
		char strbuf[4096];
		va_list ap;
		
		va_start(ap, fmt);
		vsnprintf(strbuf, 4095, fmt, ap);
		emu->out_debug_log("%s", strbuf);
		va_end(ap);
#endif
	}
	/*!
	 @brief force to log a message
	 @param fmt message to log, same as printf().
	*/
	virtual void force_out_debug_log(const char *fmt, ...)
	{
		char strbuf[4096];
		va_list ap;

		va_start(ap, fmt);
		vsnprintf(strbuf, 4095, fmt, ap);
		emu->force_out_debug_log("%s", strbuf);
		va_end(ap);
	}

#ifdef USE_DEBUGGER
	// debugger
	/*!
	 @brief check whether this is CPU variants
	 @return status whether this is CPU
	*/
	virtual bool is_cpu()
	{
		return false;
	}
	/*!
	 @brief check whether this is debugger
	 @return status whether this is debugger
	*/
	virtual bool is_debugger()
	{
		return false;
	}
	/*!
	 @brief check this has child debugger
	 @return status this has child debugger
	*/
	virtual bool is_debugger_available()
	{
		return false;
	}
	/*!
	  @brief get child debugger of this
	  @return pointer of debigger of this
	  @retval nullptr if don't have debugger
	  @retval NULL if don't have debugger
	*/
	virtual void *get_debugger()
	{
		return NULL;
	}
	virtual uint32_t get_debug_prog_addr_mask()
	{
		return 0;
	}
	virtual uint32_t get_debug_data_addr_mask()
	{
		return 0;
	}
	virtual uint64_t get_debug_data_addr_space()
	{
		// override this function when memory space is not (2 << n)
		return (uint64_t)get_debug_data_addr_mask() + 1;
	}
	virtual void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data)
	{
//		write_data8(addr, data);
	}
	virtual uint32_t __FASTCALL read_debug_data8(uint32_t addr)
	{
//		return read_data8(addr);
		return 0xff;
	}
	virtual void __FASTCALL write_debug_data16(uint32_t addr, uint32_t data)
	{
		write_debug_data8(addr, data & 0xff);
		write_debug_data8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32_t __FASTCALL read_debug_data16(uint32_t addr)
	{
		uint32_t val = read_debug_data8(addr);
		val |= read_debug_data8(addr + 1) << 8;
		return val;
	}
	virtual void __FASTCALL write_debug_data32(uint32_t addr, uint32_t data)
	{
		write_debug_data16(addr, data & 0xffff);
		write_debug_data16(addr + 2, (data >> 16) & 0xffff);
	}
	virtual uint32_t __FASTCALL read_debug_data32(uint32_t addr)
	{
		uint32_t val = read_debug_data16(addr);
		val |= read_debug_data16(addr + 2) << 16;
		return val;
	}
	virtual void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data)
	{
//		write_io8(addr, data);
	}
	virtual uint32_t __FASTCALL read_debug_io8(uint32_t addr)
	{
//		return read_io8(addr);
		return 0xff;
	}
	virtual void __FASTCALL write_debug_io16(uint32_t addr, uint32_t data)
	{
		write_debug_io8(addr, data & 0xff);
		write_debug_io8(addr + 1, (data >> 8) & 0xff);
	}
	virtual uint32_t __FASTCALL read_debug_io16(uint32_t addr)
	{
		uint32_t val = read_debug_io8(addr);
		val |= read_debug_io8(addr + 1) << 8;
		return val;
	}
	virtual void __FASTCALL write_debug_io32(uint32_t addr, uint32_t data)
	{
		write_debug_io16(addr, data & 0xffff);
		write_debug_io16(addr + 2, (data >> 16) & 0xffff);
	}
	virtual uint32_t __FASTCALL read_debug_io32(uint32_t addr)
	{
		uint32_t val = read_debug_io16(addr);
		val |= read_debug_io16(addr + 2) << 16;
		return val;
	}
	virtual bool write_debug_reg(const _TCHAR *reg, uint32_t data)
	{
		return false;
	}
	virtual uint32_t __FASTCALL read_debug_reg(const _TCHAR *reg)
	{
		return 0;
	}
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
	{
		return false;
	}
	virtual bool get_debug_regs_description(_TCHAR *buffer, size_t buffer_len)
	{
		return false;
	}
	virtual int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
	{
		return debug_dasm_with_userdata(pc, buffer, buffer_len, 0);
	}
	virtual int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0)
	{
		return 0;
	}
	virtual bool debug_rewind_call_trace(uint32_t pc, int &size, _TCHAR* buffer, size_t buffer_len, uint64_t userdata = 0)
	{
		size = 0;
		return false;
	}
#endif
	_TCHAR this_device_name[128];	//<! name of this device
	
	DEVICE* prev_device;	//<! previous device pointer of this device chain
	DEVICE* next_device;	//<! next device pointer of this device chain
	int this_device_id;		//<! ID of this device 
};
#endif // USE_SHARED_DLL

#endif
