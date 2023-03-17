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
#include <memory>

#include "../common.h"
#include "../fileio.h"

#if defined(_USE_QT)
#define USE_DEVICE_NAME
#endif

#include "../emu_template.h"
#include "vm_template.h"

#include "./device_params.h"

class CSP_Logger;
class OSD_BASE;

/*!
 * @class DEVICE
 * @brief BASE CLASS of all devices.
 */
class DLL_PREFIX DEVICE
{
protected:
	bool __IOBUS_RETURN_ADDR;
	bool __USE_DEBUGGER;
	VM_TEMPLATE* vm;		//!< POINTER OF LINKED VIRTUAL MACHINE
	EMU_TEMPLATE* emu;		//!< POINTER OF EMULATION CORE
	OSD_BASE* osd;			//!< POINTER OF OSD (OS DEPENDED part)
	std::shared_ptr<CSP_Logger> p_logger;	//!< POINTER OF LOGGING LOGGER

public:
	/*!
	 * @brief Constructor
	 * @param parent_vm pointer of virtual machine.
	 * @param parent_emu pointer of emulation core.
	 */
	DEVICE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu);
	/*!
	 * @brief Destructor
	 * @todo Will implement real destructor per real classes and below destructor decl. with "virtual".
	 * This makes warning:
	 * "deleting object of polymorphic class type 'DEVICE' which has non-virtual
	 *  destructor might cause undefined behavior [-Wdelete-non-virtual-dtor]".
	 */
	~DEVICE(void);

	/*!
	 * @brief Initialize a DEVICE.
	 * @note Initializing VM must be after initializing OSD.
	 * @note You may call DEVICE::initialize() FOO::initialize() of inherited class.
	 */
	virtual void initialize();
	/*!
	 * @brief De-Allocate resources before DESTRUCTION this class.
	 * @note  Normally call before deleting this device automatically.
	 */
	virtual void release();
	//!< Sound input functions
	/*!
	 * @brief clear a temporaly sound buffer to zero.
	 * @param bank bank number of temporaly sound buffer.
	 */
	virtual void clear_sound_in_source(int bank);
	/*!
	 * @brief add a temporaly sound buffer
	 * @param rate sampling rate of this buffer.
	 * @param samples samples of this buffer.
	 * @param channels channels of this buffer.
	 * @return bank number of temporaly sound buffer.
	 * @retval -1 failed to add a buffer.
	 * @note this function may be before (or after) initialize().
	 */
	virtual int add_sound_in_source(int rate, int samples, int channels);
	/*!
	 * @brief relsease a temporaly sound buffer
	 * @param bank number wish to release.
	 * @return bank number of released.
	 * @retval -1 failed to release a buffer.
	 * @note this function may be before (or after) initialize().
	 */
	virtual int release_sound_in_source(int bank);

	/*!
	 * @brief check whether a temporaly sound buffer exists.
	 * @param bank number wish to check.
	 * @retval true buffer exists.
	 * @retval false buffer not exists.
	 * @note this function may be before (or after) initialize().
	 */
	virtual bool is_sound_in_source_exists(int bank);
	/*!
	 * @brief skip data pointer in a sound buffer
	 * @param bank buffer number wish to skip.
	 * @param passed_usec time to skip (by microseconds).
	 * @return incremented pointer value.
	 * @retval 0 maybe not succeed.
	 */
	virtual int __FASTCALL increment_sound_in_passed_data(int bank, double passed_usec);
	/*!
	 * @brief count buffers whether data exists
	 * @return count of buffers that data exists.
	 */
	virtual int get_sound_in_buffers_count();
	/*!
	 * @brief get sound samples of a buffer.
	 * @param bank bank number to check samples.
	 * @return samples of a buffer.
	 * @retval 0 if not be used this buffer.
	 */
	virtual int __FASTCALL get_sound_in_samples(int bank);
	/*!
	 * @brief get sound sample rate
	 * @param bank bank number to check.
	 * @return sample rate of this buffer.
	 * @retval 0 if not be used this buffer.
	 */
	virtual int __FASTCALL get_sound_in_rate(int bank);
	/*!
	 * @brief get sound channels
	 * @param bank bank number to check.
	 * @return channels of this buffer.
	 * @retval 0 if not be used this buffer.
	 */
	virtual int __FASTCALL get_sound_in_channels(int bank);
	/*!
	 * @brief get pointer of sound buffer.
	 * @param bank bank number expect to get pointer.
	 * @return pointer of this buffer.
	 * @retval nullptr if not be used this buffer.
	 * @retval NULL if not be used this buffer.
	 * @note this function may be before (or after) initialize().
	 */
	virtual int16_t* get_sound_in_buf_ptr(int bank);
	/*!
	 * @brief write datas to sound buffer.
	 * @param bank bank number expect to write.
	 * @param src sound data pointer wish to write.
	 * @param samples samples wish to write.
	 * @return samples succseed to write
	 * @retval 0 failed to write.
	 */
	virtual int write_sound_in_buffer(int bank, int32_t* src, int samples);
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
	virtual int __FASTCALL get_sound_in_latest_data(int bank, int32_t* dst, int expect_channels);
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
	virtual int __FASTCALL get_sound_in_data(int bank, int32_t* dst, int expect_samples, int expect_rate, int expect_channels);
	/*!
	 * @brief set parameters sound high pass filter.
	 * @param freq high limit frequency.
	 * @param quality quality parameter of high pass filter.
	 */
	virtual void set_high_pass_filter_freq(int freq, double quality); //<! If freq < 0 disable HPF.
	/*!
	 * @brief set parameters sound low pass filter.
	 * @param freq low limit frequency.
	 * @param quality quality parameter of low pass filter.
	 */
	virtual void set_low_pass_filter_freq(int freq, double quality); //<! If freq <= 0 disable LPF.

	/*!
	 * @brief update configuration parameters.
	 * @note maybe call after update value(s) of config.foo .
	 */
	virtual void update_config();
	/*!
	 * @brief save state values of this device to a file.
	 * @param state_fio pointer of state file i/o.
	 * @note may not use this normally, use process_state(state_fio, false) instead of this.
	 */
	virtual void save_state(FILEIO* state_fio);
	/*!
	 * @brief load state values of this device from a file.
	 * @param state_fio pointer of state file i/o.
	 * @return loading status
	 * @retval true succeeded to load state values.
	 * @retval false failed to load state values.
	 * @note may not use this normally, use process_state(state_fio, true) instead of this.
	 */
	virtual bool load_state(FILEIO* state_fio);
	// control
	/*!
	 * @brief reset this device.
	 * @note call within VM::reset() automatically.
	 */
	virtual void reset();
	/*!
	 * @brief reset this device with special feature.
	 * @param num type of special feature.
	 * @note call within VM::special_reset() automatically.
	 */
	virtual void special_reset(int num);
	/*!
	 * @brief load/save state values of this device from a file.
	 * @param state_fio pointer of state file i/o.
	 * @param loading true for loading , false for saving state values.
	 * @return processing status
	 * @retval true succeeded to load/save state values.
	 * @retval false failed to load/save state values.
	 * @note normally returns true when saving.
	 */
	virtual bool process_state(FILEIO* state_fio, bool loading);

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
	virtual uint32_t __FASTCALL translate_address(int segment, uint32_t offset);

	//<! memory bus
	/*!
	 * @brief write a 8bit width data to memory .
	 * @param addr address to write.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_data8(uint32_t addr, uint32_t data);
	/*!
	 * @brief read a 8bit width data from memory .
	 * @param addr address to read.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_data8(uint32_t addr);
	/*!
	 * @brief write a 16bit width data to memory .
	 * @param addr address to write.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_data16(uint32_t addr, uint32_t data);
	/*!
	 * @brief read a 16bit width data from memory .
	 * @param addr address to read.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_data16(uint32_t addr);
	/*!
	 * @brief write a 32bit width data to memory .
	 * @param addr address to write.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_data32(uint32_t addr, uint32_t data);
	/*!
	 * @brief read a 32bit width data from memory .
	 * @param addr address to read.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_data32(uint32_t addr);
	/*!
	 * @brief write a 8bit width data to memory with wait value.
	 * @param addr address to write.
	 * @param data  value to write.
	 * @param wait pointer to get memory wait value of this address.
	 */
	virtual void __FASTCALL write_data8w(uint32_t addr, uint32_t data, int* wait);
	/*!
	 * @brief read a 8bit width data from memory with wait value.
	 * @param addr address to read.
	 * @param wait pointer to get memory wait value of this address.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_data8w(uint32_t addr, int* wait);
	/*!
	 * @brief write a 16bit width data to memory with wait value.
	 * @param addr address to write.
	 * @param data value to write.
	 * @param wait pointer to get memory wait value of this address.
	 */
	virtual void __FASTCALL write_data16w(uint32_t addr, uint32_t data, int* wait);
	/*!
	 * @brief read a 16bit width data from memory with wait value.
	 * @param addr address to read.
	 * @param wait pointer to get memory wait value of this address.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_data16w(uint32_t addr, int* wait);
	/*!
	 * @brief write a 32bit width data to memory with wait value.
	 * @param addr address to write.
	 * @param data value to write.
	 * @param wait pointer to get memory wait value of this address.
	 */
	virtual void __FASTCALL write_data32w(uint32_t addr, uint32_t data, int* wait);
	/*!
	 * @brief read a 32bit width data from memory with wait value.
	 * @param addr address to read.
	 * @param wait pointer to get memory wait value of this address.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_data32w(uint32_t addr, int* wait);
	/*!
	 * @brief fetch a opcode from memory
	 * @param addr address to fetch.
	 * @param wait pointer to get memory wait value of this address.
	 * @return fetched value.
	 * @note normally fetced value has 8bit width.
	 */
	virtual uint32_t __FASTCALL fetch_op(uint32_t addr, int *wait);
	/*!
	 * @brief transfer 8bit width data from dma to memory.
	 * @param addr memory address to write.
	 * @param data value to transfer.
	 */
	virtual void __FASTCALL write_dma_data8(uint32_t addr, uint32_t data);
	/*!
	 * @brief transfer 8bit width data from memory to dma.
	 * @param addr memory address to read.
	 * @return value to transfer.
	 */
	virtual uint32_t __FASTCALL read_dma_data8(uint32_t addr);
	/*!
	 * @brief transfer 16bit width data from dma to memory.
	 * @param addr memory address to write.
	 * @param data value to transfer.
	 */
	virtual void __FASTCALL write_dma_data16(uint32_t addr, uint32_t data);
	/*!
	 * @brief transfer 16bit width data from memory to dma.
	 * @param addr memory address to read.
	 * @return value to transfer.
	 */
	virtual uint32_t __FASTCALL read_dma_data16(uint32_t addr);
	/*!
	 * @brief transfer 32bit width data from dma to memory.
	 * @param addr memory address to write.
	 * @param data value to transfer.
	 */
	virtual void __FASTCALL write_dma_data32(uint32_t addr, uint32_t data);
	/*!
	 * @brief transfer 32bit width data from memory to dma.
	 * @param addr memory address to read.
	 * @return value to transfer.
	 */
	virtual uint32_t __FASTCALL read_dma_data32(uint32_t addr);
	/*!
	 * @brief transfer 8bit width data from dma to memory with wait value.
	 * @param addr memory address to write.
	 * @param data value to transfer.
	 * @param wait pointer of got wait value.
	 */
	virtual void __FASTCALL write_dma_data8w(uint32_t addr, uint32_t data, int* wait);
	/*!
	 * @brief transfer 8bit width data from memory to dma with wait value.
	 * @param addr memory address to transfer.
	 * @param wait pointer of got wait value.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_dma_data8w(uint32_t addr, int* wait);
	/*!
	 * @brief transfer 16bit width data from dma to memory with wait value.
	 * @param addr memory address to write.
	 * @param data value to transfer.
	 * @param wait pointer of got wait value.
	 */
	virtual void __FASTCALL write_dma_data16w(uint32_t addr, uint32_t data, int* wait);
	/*!
	 * @brief transfer 16bit width data from memory to dma with wait value.
	 * @param addr memory address to transfer.
	 * @param wait pointer of got wait value.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_dma_data16w(uint32_t addr, int* wait);
	/*!
	 * @brief transfer 32bit width data from dma to memory with wait value.
	 * @param addr memory address to write.
	 * @param data value to transfer.
	 * @param wait pointer of got wait value.
	 */
	virtual void __FASTCALL write_dma_data32w(uint32_t addr, uint32_t data, int* wait);
	/*!
	 * @brief transfer 32bit width data from memory to dma with wait value.
	 * @param addr memory address to transfer.
	 * @param wait pointer of got wait value.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_dma_data32w(uint32_t addr, int* wait);

	//<! i/o bus
	/*!
	 * @brief write 8bit width data to I/O bus.
	 * @param addr address of I/O port.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	/*!
	 * @brief read 8bit width data from I/O bus.
	 * @param addr address of I/O port.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	/*!
	 * @brief write 16bit width data to I/O bus.
	 * @param addr address of I/O port.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_io16(uint32_t addr, uint32_t data);
	/*!
	 * @brief read 16bit width data from I/O bus.
	 * @param addr address of I/O port.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_io16(uint32_t addr);
	/*!
	 * @brief write 32bit width data to I/O bus.
	 * @param addr address of I/O port.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_io32(uint32_t addr, uint32_t data);
	/*!
	 * @brief read 32bit width data from I/O bus.
	 * @param addr address of I/O port.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_io32(uint32_t addr);
	/*!
	 * @brief write 8bit width data to I/O bus with wait.
	 * @param addr address of I/O port.
	 * @param wait pointer to get wait value.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_io8w(uint32_t addr, uint32_t data, int* wait);
	/*!
	 * @brief read 8bit width data from I/O bus with wait.
	 * @param addr address of I/O port.
	 * @param wait pointer to get wait value.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_io8w(uint32_t addr, int* wait);
	/*!
	 * @brief write 16bit width data to I/O bus with wait.
	 * @param addr address of I/O port.
	 * @param wait pointer to get wait value.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_io16w(uint32_t addr, uint32_t data, int* wait);
	/*!
	 * @brief read 16bit width data from I/O bus with wait.
	 * @param addr address of I/O port.
	 * @param wait pointer to get wait value.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_io16w(uint32_t addr, int* wait);
	/*!
	 * @brief write 32bit width data to I/O bus with wait.
	 * @param addr address of I/O port.
	 * @param wait pointer to get wait value.
	 * @param data value to write.
	 */
	virtual void __FASTCALL write_io32w(uint32_t addr, uint32_t data, int* wait);
	/*!
	 * @brief read 32bit width data from I/O bus with wait.
	 * @param addr address of I/O port.
	 * @param wait pointer to get wait value.
	 * @return read value.
	 */
	virtual uint32_t __FASTCALL read_io32w(uint32_t addr, int* wait);
	/*!
	 * @brief transfer 8bit width data from dma to I/O bus.
	 * @param addr I/O address to write.
	 * @param data value to transfer.
	 */
	virtual void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data);
	/*!
	 * @brief transfer 8bit width data from I/O bus to dma.
	 * @param addr I/O address to read.
	 * @return value to transfer.
	 */
	virtual uint32_t __FASTCALL read_dma_io8(uint32_t addr);
	/*!
	 * @brief transfer 16bit width data from dma to I/O bus.
	 * @param addr I/O address to write.
	 * @param data value to transfer.
	 */
	virtual void __FASTCALL write_dma_io16(uint32_t addr, uint32_t data);
	/*!
	 * @brief transfer 16bit width data from I/O bus to dma.
	 * @param addr I/O address to read.
	 * @return value to transfer.
	 */
	virtual uint32_t __FASTCALL read_dma_io16(uint32_t addr);
	/*!
	 * @brief transfer 32bit width data from dma to I/O bus.
	 * @param addr I/O address to write.
	 * @param data value to transfer.
	 */
	virtual void __FASTCALL write_dma_io32(uint32_t addr, uint32_t data);
	/*!
	 * @brief transfer 32bit width data from I/O bus to dma.
	 * @param addr I/O address to read.
	 * @return value to transfer.
	 */
	virtual uint32_t __FASTCALL read_dma_io32(uint32_t addr);
	/*!
	 * @brief transfer 8bit width data from dma to I/O bus with wait.
	 * @param addr I/O address to write.
	 * @param data value to transfer.
	 * @param wait pointer for wait result value.
	 */
	virtual void __FASTCALL write_dma_io8w(uint32_t addr, uint32_t data, int* wait);
	/*!
	 * @brief transfer 8bit width data from I/O bus to dma with wait.
	 * @param addr I/O address to write.
	 * @param wait pointer for wait result value.
	 * @return read value from I/O.
	 */
	virtual uint32_t __FASTCALL read_dma_io8w(uint32_t addr, int* wait);
	/*!
	 * @brief transfer 16bit width data from dma to I/O bus with wait.
	 * @param addr I/O address to write.
	 * @param data value to transfer.
	 * @param wait pointer for wait result value.
	 */
	virtual void __FASTCALL write_dma_io16w(uint32_t addr, uint32_t data, int* wait);
	/*!
	 * @brief transfer 16bit width data from I/O bus to dma with wait.
	 * @param addr I/O address to write.
	 * @param wait pointer for wait result value.
	 * @return read value from I/O.
	 */
	virtual uint32_t __FASTCALL read_dma_io16w(uint32_t addr, int* wait);
	/*!
	 * @brief transfer 32bit width data from dma to I/O bus with wait.
	 * @param addr I/O address to write.
	 * @param data value to transfer.
	 * @param wait pointer for wait result value.
	 */
	virtual void __FASTCALL write_dma_io32w(uint32_t addr, uint32_t data, int* wait);
	/*!
	 * @brief transfer 32bit width data from I/O bus to dma with wait.
	 * @param addr I/O address to write.
	 * @param wait pointer for wait result value.
	 * @return read value from I/O.
	 */
	virtual uint32_t __FASTCALL read_dma_io32w(uint32_t addr, int* wait);

	// memory mapped i/o
	/*!
	  @brief write 8bit width data to this device's MMIO (or MEMORY).
	  @param addr address of MMIO. normally global address.
	  @param data data to write to MMIO.
	  @note This method is called from another device, normally MEMORY BUS.
	*/
	virtual void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data);
	/*!
	  @brief read 8bit width data from this device's MMIO (or MEMORY).
	  @param addr address of MMIO. normally global address.
	  @return data data to read from MMIO.
	  @note This method is called from another device, normally MEMORY BUS.
	*/
	virtual uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr);
	/*!
	  @brief write 16bit width data to this device's MMIO (or MEMORY).
	  @param addr address of MMIO. normally global address.
	  @param data data to write to MMIO.
	  @note This method is called from another device, normally MEMORY BUS.
	*/
	virtual void __FASTCALL write_memory_mapped_io16(uint32_t addr, uint32_t data);
	/*!
	  @brief read 16bit width data from this device's MMIO (or MEMORY).
	  @param addr address of MMIO. normally global address.
	  @return data data to read from MMIO.
	  @note This method is called from another device, normally MEMORY BUS.
	*/
	virtual uint32_t __FASTCALL read_memory_mapped_io16(uint32_t addr);
	/*!
	  @brief write 32bit width data to this device's MMIO (or MEMORY).
	  @param addr address of MMIO. normally global address.
	  @param data data to write to MMIO.
	  @note This method is called from another device, normally MEMORY BUS.
	*/
	virtual void __FASTCALL write_memory_mapped_io32(uint32_t addr, uint32_t data);
	/*!
	  @brief read 32bit width data from this device's MMIO (or MEMORY).
	  @param addr address of MMIO. normally global address.
	  @return data data to read from MMIO.
	  @note This method is called from another device, normally MEMORY BUS.
	*/
	virtual uint32_t __FASTCALL read_memory_mapped_io32(uint32_t addr);
	/*!
	  @brief write 8bit width data to this device's MMIO (or MEMORY) with wait.
	  @param addr address of MMIO. normally global address.
	  @param data data to write to MMIO.
	  @param wait pointer for wait result.
	  @note This method is called from another device, normally MEMORY BUS.
	*/
	virtual void __FASTCALL write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait);
	/*!
	  @brief read 8bit width data from this device's MMIO (or MEMORY) with wait.
	  @param addr address of MMIO. normally global address.
	  @param wait pointer for wait result.
	  @return data data to read from MMIO.
	  @note This method is called from another device, normally MEMORY BUS.
	*/
	virtual uint32_t __FASTCALL read_memory_mapped_io8w(uint32_t addr, int* wait);
	/*!
	  @brief write 16bit width data to this device's MMIO (or MEMORY) with wait.
	  @param addr address of MMIO. normally global address.
	  @param data data to write to MMIO.
	  @param wait pointer for wait result.
	  @note This method is called from another device, normally MEMORY BUS.
	*/
	virtual void __FASTCALL write_memory_mapped_io16w(uint32_t addr, uint32_t data, int* wait);
	/*!
	  @brief read 16bit width data from this device's MMIO (or MEMORY) with wait.
	  @param addr address of MMIO. normally global address.
	  @param wait pointer for wait result.
	  @return data data to read from MMIO.
	  @note This method is called from another device, normally MEMORY BUS.
	*/
	virtual uint32_t __FASTCALL  read_memory_mapped_io16w(uint32_t addr, int* wait);
	/*!
	  @brief write 32bit width data to this device's MMIO (or MEMORY) with wait.
	  @param addr address of MMIO. normally global address.
	  @param data data to write to MMIO.
	  @param wait pointer for wait result.
	  @note This method is called from another device, normally MEMORY BUS.
	*/
	virtual void __FASTCALL write_memory_mapped_io32w(uint32_t addr, uint32_t data, int* wait);
	/*!
	  @brief read 32bit width data from this device's MMIO (or MEMORY) with wait.
	  @param addr address of MMIO. normally global address.
	  @param wait pointer for wait result.
	  @return data data to read from MMIO.
	  @note This method is called from another device, normally MEMORY BUS.
	*/
	virtual uint32_t __FASTCALL read_memory_mapped_io32w(uint32_t addr, int* wait);

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
	virtual void initialize_output_signals(outputs_t *items);
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
	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask, int shift);
	/*!
	 * @brief register a port to multiple output port without bit shift
	 * @param items pointer of multiple output port.
	 * @param device pointer of target device (to write signal value).
	 * @param id signal number for this port.
	 * @param mask bit mask for this port.
	 * @note must call before using a multiple output port.
	 * @note items must be pointer, not be multiple output port.
	 */
	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask);
	/*!
	 * @brief write data to multiple output port
	 * @param items pointer of multiple output port.
	 * @param data value to write.
	 * @note items must be pointer, not be multiple output port.
	 * @note useful for batch writing signals.
	 */
	virtual void __FASTCALL write_signals(outputs_t *items, uint32_t data);
	/*!
	 * @brief update bitmask to specified device at multiple output port
	 * @param items pointer of multiple output port.
	 * @param device target device to update bitmask.
	 * @param mask bitmask to update.
	 * @note items must be pointer, not be multiple output port.
	 */
	virtual void update_signal_mask(outputs_t *items, DEVICE *device, uint32_t mask);
	/*!
	 * @brief write signal to this device (normally from another device or VM).
	 * @param id signal number
	 * @param data value to write
	 * @param mask bitmask to write
	 */
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	/*!
	 * @brief inspect signal from this device (normally to another device or VM).
	 * @param id signal number to inspect
	 * @return inspected value
	 * @retval 0 default value.
	 */
	virtual uint32_t __FASTCALL read_signal(int ch);

	//<! z80 daisy chain
	/*!
	 * @brief set target device for INTR signal
	 * @param device pointer of target device
	 * @param bit bitmask for target device.
	 * @note this may use for Z80 variants.
	 */
	virtual void set_context_intr(DEVICE* device, uint32_t bit);
	/*!
	 * @brief register child (slave) device to this device.
	 * @param device pointer of child device.
	 */
	virtual void set_context_child(DEVICE* device);

	/*!
	 * @brief get  child (slave) device of this device.
	 * @return pointer of child device.
	 * @retval nullptr child not exists.
	 * @retval NULL child not exists.
	 */
	virtual DEVICE *get_context_child();

	/*!
	 * @brief send interrupt (IEI) signal from device to device
	 * @param val interrupt status value
	 */
	virtual void __FASTCALL set_intr_iei(bool val);

	/*!
	 * @brief send interrupt signal from device to cpu
	 * @param line signal value
	 * @param pending pending to update signal until next period.
	 * @param bit interrupt bit to update.
	 */
	virtual void __FASTCALL set_intr_line(bool line, bool pending, uint32_t bit);

	// interrupt cpu to device
	/*!
	  @brief get acknowledge value from this device.
	  @return acknowledge value (normally level of interrupt)
	  @retval 0xff default value
	  @note this is used from PIC and ix86 variant CPUs.
	 */
	virtual uint32_t get_intr_ack();
	/*!
	  @brief update interrupt signals (to target devices)
	 */
	virtual void update_intr();
	/*!
	  @brief notify RETI signal to this device from CPU.
	 */
	virtual void notify_intr_reti();
	/*!
	  @brief notify EI signal to this device from CPU.
	 */
	virtual void notify_intr_ei();

	/*!
	  @brief do a DMA transfer cycle.
	 */
	virtual void __FASTCALL do_dma();

	//<! cpu
	/*!
	  @brief run instruction(s) of CPU
	  @param clock clocks count to run.
	  @return spent clocks to run instruction(s).
	  @note if set clock to -1, run a one instruction.
	 */
	virtual int __FASTCALL run(int clock);
	/*!
	  @brief add extra clocks value to CPU
	  @param clock value to add.
	  @note normally use for waiting.
	 */
	virtual void __FASTCALL set_extra_clock(int clock);
	/*!
	  @brief get extra clocks
	  @return extra clocks
	  @retval 0 default value.
	 */
	virtual int get_extra_clock();
	/*!
	  @brief get program counter of CPU
	  @return value of program counter
	  @retval 0x00000000 default value.
	  @note normally this value contains previous instruction address.
	 */
	virtual uint32_t get_pc();
	/*!
	 * @brief get next program counter of CPU
	 * @return value of program counter
	 * @note normally this value contains current instruction address.
	 */
	virtual uint32_t get_next_pc();

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
	virtual bool bios_call_far_i86(uint32_t PC, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles);
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
	virtual bool bios_int_i86(int intnum, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles);
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
	virtual bool bios_call_far_ia32(uint32_t PC, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles);
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
	virtual bool bios_int_ia32(int intnum, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles);
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
	virtual bool bios_ret_z80(uint16_t PC, pair32_t* af, pair32_t* bc, pair32_t* de, pair32_t* hl, pair32_t* ix, pair32_t* iy, uint8_t* iff1);
	/*!
	  @deprecated this function already is not used any components.
	  @return translate status
	  @retval true if succeeded.
	  @note If not present, always succeeded.
	*/
	virtual bool __FASTCALL address_translate_for_bios(int space, int intention, uint64_t &taddress);
	// misc

	// event manager
	DEVICE* event_manager; //<! event manager for this device (normally EVENT::).
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
	virtual int get_event_manager_id();
	/*!
	 * @brief get cpu clocks for event manager
	 * @return cpu clocks value (by HZ).
	 */
	virtual uint32_t get_event_clocks();
	/*!
	 * @brief check target is primary CPU of this event manager
	 * @param device target device to check
	 * @retval true taget is primary CPU.
	 */
	virtual bool is_primary_cpu(DEVICE* device);
	/*!
	 * @brief get cpu clocks of target device
	 * @param device target device
	 * @return cpu clocks value (by HZ).
	 * @retval CPU_CLOCKS when target is *not* CPU
	 */
	virtual uint32_t __FASTCALL get_cpu_clocks(DEVICE* device);
	/*!
	 * @brief process extra events by clocks.
	 * @param clock clocks period for primary CPU.
	 * @note this is called from primary cpu while running one opecode
	*/
	virtual void __FASTCALL update_event_in_opecode(int clock);
	/*!
	 * @brief register a event of device to event manager by microseconds.
	 * @param device target device (normally this)
	 * @param event_id event id number to register
	 * @param usec delay time by microseconds
	 * @param loop true if repeat to call event
	 * @param register_id pointer of registered id
	 * @note works even register_id is nullptr
	 */
	virtual void register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id);
	/*!
	 * @brief register a event of device to event manager by CPU clocks.
	 * @param device target device (normally this)
	 * @param event_id event id number to register
	 * @param clock delay time by clocks
	 * @param loop true if repeat to call event
	 * @param register_id pointer of registered id
	 * @note works even register_id is nullptr
	 */
	virtual void register_event_by_clock(DEVICE* device, int event_id, uint64_t clock, bool loop, int* register_id);
	/*!
	 * @brief unregister a event
	 * @param device target device to unregister a event, normally this
	 * @param register_id event id expect to unregister
	 */
	virtual void cancel_event(DEVICE* device, int register_id);
	/*!
	 * @brief unregister a event and clear event variable
	 * @param dev target device to unregister a event, normally this
	 * @param evid event id variable to unregister
	 */
	virtual void clear_event(DEVICE* dev, int& evid);
	/*!
	 * @brief unregister a event, then register a event
	 * @param device target device (normally this)
	 * @param event_num event id number to register
	 * @param usec delay time by microseconds
	 * @param loop true if repeat to call event
	 * @param evid event variable
	 */
	virtual void force_register_event(DEVICE* dev, int event_num, double usec, bool loop, int& evid);
	/*!
	 * @brief unregister a event, then register a event by clocks
	 * @param device target device (normally this)
	 * @param event_num event id number to register
	 * @param clock delay time by cpu clocks
	 * @param loop true if repeat to call event
	 * @param evid event variable
	 */
	virtual void force_register_event_by_clock(DEVICE* dev, int event_num, uint64_t clock, bool loop, int& evid);

	// Register a EVENT to evid , if evid slot isn't used.
	/*!
	 * @brief register a event of device to event manager if not regsiterred by microseconds.
	 * @param device target device (normally this)
	 * @param event_num event id number to register
	 * @param usec delay time by mycroseconds
	 * @param loop true if repeat to call event
	 * @param evid registered id variable
	 */
	virtual void check_and_update_event(DEVICE* dev, int event_num, double usec, bool loop, int& evid);
	/*!
	 * @brief register a event of device to event manager if not regsiterred by CPU clocks.
	 * @param device target device (normally this)
	 * @param event_num event id number to register
	 * @param clock delay time by clocks
	 * @param loop true if repeat to call event
	 * @param evid registered id variable
	 */
	virtual void check_and_update_event_by_clock(DEVICE* dev, int event_num, uint64_t clock, bool loop, int& evid);

	/*!
	  @brief register a device for event at start of frame
	  @param device target device, normally this
	  @note call device->event_frame() and device->event_pre_frame() from event manager every frame period.
	 */
	virtual void register_frame_event(DEVICE* device);
	/*!
	  @brief register a device for event at every start of vertical line
	  @param device target device, normally this
	  @note call device->event_vline() from event manager every vertical line period.
	 */
	virtual void register_vline_event(DEVICE* device);
	/*!
	  @brief inspect scheduled remain clocks until happened a event.
	  @param register_id event id what wish to check remain.
	  @return remain clock count until happen this event.
	  @retval 0 already happened or evenr_id dont' exists.
	 */
	virtual uint32_t __FASTCALL get_event_remaining_clock(int register_id);
	/*!
	  @brief inspect scheduled remain uSecs until happened a event.
	  @param register_id event id what wish to check remain.
	  @return remain time by uSecs until happen this event.
	  @retval 0.0 already happened or evenr_id dont' exists.
	 */
	virtual double __FASTCALL get_event_remaining_usec(int register_id);
	/*!
	 @brief get current clock count with unsigned 32bit value
	 @return current clock count value
	 @note this return value is 32bit width, sometimes overwrap.
	*/
	virtual uint32_t get_current_clock();
	/*!
	 @brief get passed over clock counts from previous clock count.
	 @return passed clock counts
	 @note this function culculated by 32bit width inetrnally, maybe wrong result when overwrapping.
	*/
	virtual uint32_t __FASTCALL get_passed_clock(uint32_t prev);
	/*!
	 @brief get passed over uSecs from previous clock count.
	 @return passed time (by uSec).
	 @note this function culculated by 32bit width internally , maybe wrong result when overwrapping.
	*/
	virtual double __FASTCALL get_passed_usec(uint32_t prev);
	/*!
	 @brief calculate clocks from recent vertical line period
	 @return clock count value
	 @note this function culculated by 32bit width, maybe wrong result when overwrapping.
	*/
	virtual uint32_t get_passed_clock_since_vline();
	/*!
	 @brief calculate time (by uSec) from recent vertical line period
	 @return passed time by uSec
	 @note this function culculated by 32bit width, maybe wrong result when overwrapping.
	*/
	virtual double get_passed_usec_since_vline();
	/*!
	 * @brief get current vertical line position
	 * @return vertical line position
	 * @retval 0 if start of frame or out of frame.
	 */
	virtual int get_cur_vline();
	/*!
	  @brief get related clock count of this vertical line.
	  @return relative clock value to next vertical line (or end of this frame).
	*/
	virtual int get_cur_vline_clocks();
	/*!
	  @brief get program counter value of a CPU
	  @param index CPU index
	  @return PC value.
	  @note if CPU[index] don't exists, will make undefined behavior.
	*/
	virtual uint32_t __FASTCALL get_cpu_pc(int index);
	/*!
	 @brief get current clock count with unsigned 64bit value
	 @return current clock count value
	 @note this return value is 64bit width, sometimes overwrap.
	*/
	virtual uint64_t get_current_clock_uint64();
	/*!
	 @brief inspect CPU clock value by Hz.
	 @param index CPU number to inspect.
	 @return clock value (by Hz).
	 @retval 0 maybe this CPU don't exists.
	*/
	virtual uint32_t __FASTCALL get_cpu_clock(int index);
	/*!
	  @brief request to skip next frame.
	  @note this is for "FRAME SKIP" feature .
	*/
	virtual void request_skip_frames();
	/*!
	  @brief set frame rate.
	  @param frames display frames per one second.
	*/
	virtual void set_frames_per_sec(double frames);
	/*!
	  @brief set vertical lines in a frame.
	  @param lines vertical lines in a frame.
	  @note this setting will effect after next frame period, not effect immediately.
	  @note calculate time doesn't care VBLANK after last line.
	*/
	virtual void set_lines_per_frame(int lines);
	/*!
	  @brief get number of vertical lines of this frame.
	  @return number of vertical lines.
	*/
	virtual int get_lines_per_frame();
	/*!
	  @brief Force render sound immediately when device's status has changed.
	  @note You must call this after you changing registers (or enything).
	  @note If has problems, try set_realtime_render.
	  @note See mb8877.cpp and ym2203.cpp.
	  @note -- 20161010 K.O
	*/
	virtual void touch_sound(void);
	/*!
	  @brief set to render per 1 sample automatically.
	  @param device pointer of target device
	  @param flag force to render per 1 sample.
	  @note See pcm1bit.cpp.
	  @note -- 20161010 K.O
	*/
	virtual void set_realtime_render(DEVICE* device, bool flag = true);
	/*!
	  @brief set to render per 1 sample automatically for this device.
	  @param flag force to render per 1 sample.
	*/
	virtual void set_realtime_render(bool flag)
	{
		set_realtime_render(this, flag);
	}
	/*!
	  @brief update timing factors for thi device and frame.
	  @param new_clocks clocks of this device expect to update.
	  @param new_frames_per_sec frame rate expect to update.
	  @param new_lines_per_frame number of vertical lines expect to update.
	  @note any parameters has no means, as of DEVICE definitions.
	*/
	virtual void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);

	//<! event callback
	/*!
	  @brief event handler of this device
	  @param event_id called id to process.
	  @param err local value
	  @note normally, err is not used.
	*/
	virtual void __FASTCALL event_callback(int event_id, int err);
	/*!
	  @brief fixed handler before processing a frame.
	  @note must call register_frame_event(this) before using (normally initialize()).
	*/
	virtual void event_pre_frame();
	/*!
	  @brief fixed handler processing a top of frame.
	  @note must call register_frame_event(this) before using (normally initialize()).
	*/
	virtual void event_frame();
	/*!
	  @brief fixed handler per begin of vertical lines.
	  @param v position of vertical line.
	  @param clock relative clocks
	  @note must call register_vline_event(this) before using (normally initialize()).
	*/
	virtual void event_vline(int v, int clock);
	/*!
	  @brief fixed handler per a pixel (HSYNC).
	  @param v position of vertical line.
	  @param h pixel offset position of this vertical line.
	  @param clock relative clocks
	  @note this still not be used.
	*/
	virtual void __FASTCALL event_hsync(int v, int h, int clock);

	// sound
	/*!
	  @brief render sound samples
	  @param buffer target sound buffer.
	  @cnt sound sample counts
	  @note data adds to buffer value already exists.
	  @note Must clamp to -32768 to +32767 insize of this function.
	*/
	virtual void __FASTCALL mix(int32_t* buffer, int cnt);
	/*!
	  @brief set render sound volume by decibel
	  @param ch local channel to set volume
	  @param decibel_l left volume by 0.5 decibel
	  @param decivel_r right volume by 0.5 decibel
	  @note +1 equals +0.5dB (same as fmgen)
	*/
	virtual void set_volume(int ch, int decibel_l, int decibel_r);

	/*!
	  @brief get render sound volume values
	  @param ch local channel to set volume
	  @param decibel_l left volume value by 0.5 decibel
	  @param decivel_r right volume value by 0.5 decibel
	  @note normally replies 0.
	  @note +1 equals +0.5dB (same as fmgen)
	*/
	virtual void get_volume(int ch, int& decibel_l, int& decibel_r);
	/*!
	  @brief set name of this device
	  @param format name of this device (printf style)
	*/
	virtual void set_device_name(const _TCHAR *format, ...);
	/*!
	  @brief write 8bit data to memory from debugger.
	  @param addr target address to write.
	  @param data data to write.
	*/
	virtual void __FASTCALL write_via_debugger_data8(uint32_t addr, uint32_t data);
	/*!
	  @brief read 8bit data to debugger from memory.
	  @param addr target address to read.
	  @return data data from memory.
	*/
	virtual uint32_t __FASTCALL read_via_debugger_data8(uint32_t addr);
	/*!
	  @brief write 16bit data to memory from debugger.
	  @param addr target address to write.
	  @param data data to write.
	*/
	virtual void __FASTCALL write_via_debugger_data16(uint32_t addr, uint32_t data);
	/*!
	  @brief read 16bit data to debugger from memory.
	  @param addr target address to read.
	  @return data data from memory.
	*/
	virtual uint32_t __FASTCALL read_via_debugger_data16(uint32_t addr);
	/*!
	  @brief write 32bit data to memory from debugger.
	  @param addr target address to write.
	  @param data data to write.
	*/
	virtual void __FASTCALL write_via_debugger_data32(uint32_t addr, uint32_t data);
	/*!
	  @brief read 32bit data to debugger from memory.
	  @param addr target address to read.
	  @return data data from memory.
	*/
	virtual uint32_t __FASTCALL read_via_debugger_data32(uint32_t addr);
	/*!
	  @brief write 8bit data to memory from debugger with wait.
	  @param addr target address to write.
	  @param data data to write.
	  @param wait pointer of wait result value.
	*/
	virtual void __FASTCALL write_via_debugger_data8w(uint32_t addr, uint32_t data, int* wait);
	/*!
	  @brief read 8bit data to debugger from memory with wait.
	  @param addr target address to read.
	  @param wait pointer of wait result value.
	  @return data data from memory.
	*/
	virtual uint32_t __FASTCALL read_via_debugger_data8w(uint32_t addr, int* wait);
	/*!
	  @brief write 16bit data to memory from debugger with wait.
	  @param addr target address to write.
	  @param data data to write.
	  @param wait pointer of wait result value.
	*/
	virtual void __FASTCALL write_via_debugger_data16w(uint32_t addr, uint32_t data, int* wait);
	/*!
	  @brief read 16bit data to debugger from memory with wait.
	  @param addr target address to read.
	  @param wait pointer of wait result value.
	  @return data data from memory.
	*/
	virtual uint32_t __FASTCALL read_via_debugger_data16w(uint32_t addr, int* wait);
	/*!
	  @brief write 32bit data to memory from debugger with wait.
	  @param addr target address to write.
	  @param data data to write.
	  @param wait pointer of wait result value.
	*/
	virtual void __FASTCALL write_via_debugger_data32w(uint32_t addr, uint32_t data, int* wait);
	/*!
	  @brief read 32bit data to debugger from memory with wait.
	  @param addr target address to read.
	  @param wait pointer of wait result value.
	  @return data data from memory.
	*/
	virtual uint32_t __FASTCALL read_via_debugger_data32w(uint32_t addr, int* wait);
	/*!
	  @brief write 8bit data to I/O bus from debugger.
	  @param addr target address to write.
	  @param data data to write.
	*/
	virtual void __FASTCALL write_via_debugger_io8(uint32_t addr, uint32_t data);
	/*!
	  @brief read 8bit data to debugger from I/O bus.
	  @param addr target address to read.
	  @return data data from memory.
	*/
	virtual uint32_t __FASTCALL read_via_debugger_io8(uint32_t addr);
	/*!
	  @brief write 16bit data to I/O bus from debugger.
	  @param addr target address to write.
	  @param data data to write.
	*/
	virtual void __FASTCALL write_via_debugger_io16(uint32_t addr, uint32_t data);
	/*!
	  @brief read 16bit data to debugger from I/O bus.
	  @param addr target address to read.
	  @return data data from memory.
	*/
	virtual uint32_t __FASTCALL read_via_debugger_io16(uint32_t addr);
	/*!
	  @brief write 32bit data to I/O bus from debugger with wait.
	  @param addr target address to write.
	  @param data data to write.
	*/
	virtual void __FASTCALL write_via_debugger_io32(uint32_t addr, uint32_t data);
	/*!
	  @brief read 32bit data to debugger from I/O bus.
	  @param addr target address to read.
	  @return data data from memory.
	*/
	virtual uint32_t __FASTCALL read_via_debugger_io32(uint32_t addr);
	/*!
	  @brief write 8bit data to I/O bus from debugger with wait.
	  @param addr target address to write.
	  @param data data to write.
	  @param wait pointer of wait result value.
	*/
	virtual void __FASTCALL write_via_debugger_io8w(uint32_t addr, uint32_t data, int* wait);
	/*!
	  @brief read 8bit data to debugger from I/O bus with wait.
	  @param addr target address to read.
	  @param wait pointer of wait result value.
	  @return data data from memory.
	*/
	virtual uint32_t __FASTCALL read_via_debugger_io8w(uint32_t addr, int* wait);
	/*!
	  @brief write 16bit data to I/O bus from debugger with wait.
	  @param addr target address to write.
	  @param data data to write.
	  @param wait pointer of wait result value.
	*/
	virtual void __FASTCALL write_via_debugger_io16w(uint32_t addr, uint32_t data, int* wait);
	/*!
	  @brief read 16bit data to debugger from I/O bus with wait.
	  @param addr target address to read.
	  @param wait pointer of wait result value.
	  @return data data from memory.
	*/
	virtual uint32_t __FASTCALL read_via_debugger_io16w(uint32_t addr, int* wait);
	/*!
	  @brief write 32bit data to I/O bus from debugger with wait.
	  @param addr target address to write.
	  @param data data to write.
	  @param wait pointer of wait result value.
	*/
	virtual void __FASTCALL write_via_debugger_io32w(uint32_t addr, uint32_t data, int* wait);
	/*!
	  @brief read 32bit data to debugger from I/O bus with wait.
	  @param addr target address to read.
	  @param wait pointer of wait result value.
	  @return data data from memory.
	*/
	virtual uint32_t __FASTCALL read_via_debugger_io32w(uint32_t addr, int* wait);
	/*!
	 @brief logging a message
	 @param fmt message to log, same as printf().
	*/
	virtual void out_debug_log(const char *fmt, ...);
	/*!
	 @brief logging a message with switch
	 @param logging logging if true, not logging if false.
	 @param fmt message to log, same as printf().
	*/
	virtual void out_debug_log_with_switch(bool logging, const char *fmt, ...);
	/*!
	 @brief force to log a message
	 @param fmt message to log, same as printf().
	*/
	virtual void force_out_debug_log(const char *fmt, ...);

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
	/*!
	  @brief get address mask of program address space for debugger.
	*/
	virtual uint32_t get_debug_prog_addr_mask()
	{
		return 0;
	}
	/*!
	  @brief get address mask of data address space for debugger.
	*/
	virtual uint32_t get_debug_data_addr_mask()
	{
		return 0;
	}
	/*!
	  @brief get size of address space for debugger.
	*/
	virtual uint64_t get_debug_data_addr_space()
	{
		// override this function when memory space is not (2 << n)
		return (uint64_t)get_debug_data_addr_mask() + 1;
	}
	/*!
	  @brief write 8bit data from debugger to this device (mostly memory bus).
	  @param addr address to write
	  @param data data to write
	*/
	virtual void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	/*!
	  @brief inspect 8bit data from this device (mostly internal memory) to debugger.
	  @param addr address to inspect
	  @return read data.
	*/
	virtual uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	/*!
	  @brief write 16bit data from debugger to this device (mostly memory bus).
	  @param addr address to write
	  @param data data to write
	*/
	virtual void __FASTCALL write_debug_data16(uint32_t addr, uint32_t data);
	/*!
	  @brief inspect 16bit data from this device (mostly internal memory) to debugger.
	  @param addr address to inspect
	  @return read data.
	*/
	virtual uint32_t __FASTCALL read_debug_data16(uint32_t addr);
	/*!
	  @brief write 32bit data from debugger to this device (mostly memory bus).
	  @param addr address to write
	  @param data data to write
	*/
	virtual void __FASTCALL write_debug_data32(uint32_t addr, uint32_t data);
	/*!
	  @brief inspect 32bit data from this device (mostly internal memory) to debugger.
	  @param addr address to inspect
	  @return read data.
	*/
	virtual uint32_t __FASTCALL read_debug_data32(uint32_t addr);
	/*!
	  @brief write 8bit data from debugger to this device (mostly I/O port).
	  @param addr address to write
	  @param data data to write
	*/
	virtual void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data);
	/*!
	  @brief inspect 8bit data from this device (mostly I/O port) to debugger.
	  @param addr address to inspect
	  @return read data.
	*/
	virtual uint32_t __FASTCALL read_debug_io8(uint32_t addr);
	/*!
	  @brief write 16bit data from debugger to this device (mostly I/O port).
	  @param addr address to write
	  @param data data to write
	*/
	virtual void __FASTCALL write_debug_io16(uint32_t addr, uint32_t data);
	/*!
	  @brief inspect 16bit data from this device (mostly I/O port) to debugger.
	  @param addr address to inspect
	  @return read data.
	*/
	virtual uint32_t __FASTCALL read_debug_io16(uint32_t addr);
	/*!
	  @brief write 32bit data from debugger to this device (mostly I/O port).
	  @param addr address to write
	  @param data data to write
	*/
	virtual void __FASTCALL write_debug_io32(uint32_t addr, uint32_t data);
	/*!
	  @brief inspect 32bit data from this device (mostly I/O port) to debugger.
	  @param addr address to inspect
	  @return read data.
	*/
	virtual uint32_t __FASTCALL read_debug_io32(uint32_t addr);
	/*!
	  @brief modify an internal register from debugger
	  @param reg name of register.
	  @param data value expect to modify.
	  @return modify result.
	  @retval true modifying succeeded.
	  @retval false modifying falied.
	*/
	virtual bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	/*!
	  @brief read value from a register.
	  @param reg name of register.
	  @return register value.
	  @retval 0 maybe register has not found (or has found and value is zero).
	*/
	virtual uint32_t __FASTCALL read_debug_reg(const _TCHAR *reg);
	/*!
	  @brief dump registers value information to string
	  @param buffer pointer of result string.
	  @param buffer_len length of buffer.
	  @return dump results
	  @retval true succeed to dump.
	  @retval false failed to dump, has not any result.
	*/
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	/*!
	 @brief get descriptions of registers.
	 @param buffer pointer of result string.
	 @param buffer_len length of buffer.
	 @return results.
	 @retval true this device has description of registers.
	 @retval false this device doesn't have descpriction of registers.
	*/
	virtual bool get_debug_regs_description(_TCHAR *buffer, size_t buffer_len);
	/*!
	  @brief disassemble one instruction.
	  @param pc address expect to disassemble.
	  @param buffer pointer of result string.
	  @param buffer_len length of buffer.
	  @return bytes of instructions to disassemble.
	*/
	virtual int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
	/*!
	  @brief disassemble one instruction with attached data.
	  @param pc address expect to disassemble.
	  @param buffer pointer of result string.
	  @param buffer_len length of buffer.
	  @param userdata attached data of this position.
	  @return bytes of instructions to disassemble.
	  @note this is useful to trace.
	*/
	virtual int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
	/*!
	  @brief dump call trace
	  @param pc address expect to trace.
	  @param size maximum trace depth, this will be modified to trace really.
	  @param buffer pointer of result string
	  @param buffer_len length of buffer
	  @param userdata attached data, useful to trace.
	  @return status to dump
	*/
	virtual bool debug_rewind_call_trace(uint32_t pc, int &size, _TCHAR* buffer, size_t buffer_len, uint64_t userdata = 0);

	/*!
	 * @brief get name of this device
	 * @return pointer of name
	 */
	const _TCHAR *get_device_name(void)
	{
		return (const _TCHAR *)this_device_name;
	}
	/*!
	  @brief query shared library version string
	  @return address of queried string.
	  @retval enpty-string is not shared library.
	*/
	const _TCHAR *get_lib_common_vm_version(void);

	_TCHAR this_device_name[128];	//<! name of this device

	DEVICE* prev_device;	//<! previous device pointer of this device chain
	DEVICE* next_device;	//<! next device pointer of this device chain
	int this_device_id;		//<! ID of this device
};

#endif
