/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ uPD71071 ]
*/

#ifndef _UPD71071_H_
#define _UPD71071_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_UPD71071_CH0				0
#define SIG_UPD71071_CH1				1
#define SIG_UPD71071_CH2				2
#define SIG_UPD71071_CH3				3
 /* UBE: INDICATE TARGET DEVICE HAS 16bit capability YES=1 NO=0*/
#define SIG_UPD71071_UBE_CH0			4
#define SIG_UPD71071_UBE_CH1			5
#define SIG_UPD71071_UBE_CH2			6
#define SIG_UPD71071_UBE_CH3			7
#define SIG_UPD71071_EOT_CH0			8
#define SIG_UPD71071_EOT_CH1			9
#define SIG_UPD71071_EOT_CH2			10
#define SIG_UPD71071_EOT_CH3			11
#define SIG_UPD71071_IS_TRANSFERING		16 /* 16 - 19 */
#define SIG_UPD71071_IS_16BITS_TRANSFER	20 /* 20 - 23 */
#define SIG_UPD71071_CREG				24 /* 24 - 27 */
#define SIG_UPD71071_BCREG				28 /* 28 - 31 */
#define SIG_UPD71071_AREG				32 /* 32 - 35 */
#define SIG_UPD71071_BAREG				36 /* 36 - 39 */

class DEBUGGER;
class  DLL_PREFIX UPD71071 : public DEVICE
{
protected:
	DEVICE* d_mem;
//#ifdef SINGLE_MODE_DMA
	DEVICE* d_dma;
//#endif
	DEBUGGER *d_debugger;
	outputs_t outputs_tc[4];
	outputs_t outputs_ube[4]; // If "1" word transfer, "0" byte transfer (OUT)
	outputs_t outputs_ack[4]; // Acknoledge

	struct {
		DEVICE* dev;
		uint32_t areg, bareg;
		uint32_t creg, bcreg; // 20200318 K.O (Temporally workaround for Towns)
		uint8_t mode;
	} dma[4];

	uint8_t b16, selch, base;
	uint16_t cmd;
	uint32_t tmp; // For TOWNS
	uint8_t req, sreq, mask, tc;
	bool memory_width_is_16bit[4];
	bool device_width_is_16bit[4];
	bool extend_to_32bit[4];
	bool _SINGLE_MODE_DMA;

	void __FASTCALL set_ube(int ch);
	void __FASTCALL reset_ube(int ch);
	void __FASTCALL set_dma_ack(int ch);
	void __FASTCALL reset_dma_ack(int ch);

	inline uint16_t __FASTCALL read_from_memory_8bit(int ch, int *wait, bool __debugging);
	inline uint16_t __FASTCALL read_from_memory_16bit(int ch, int *wait, bool __debugging);
	inline void __FASTCALL write_to_memory_8bit(int ch, uint16_t data, int *wait, bool __debugging);
	inline void __FASTCALL write_to_memory_16bit(int ch, uint16_t data, int *wait, bool __debugging);

	inline uint16_t __FASTCALL read_from_io_8bit(int ch, int *wait);
	inline uint16_t __FASTCALL read_from_io_16bit(int ch, int *wait);
	inline void __FASTCALL write_to_io_8bit(int ch, uint16_t data, int *wait);
	inline void __FASTCALL write_to_io_16bit(int ch, uint16_t data, int *wait);

	uint32_t __FASTCALL dec_memory_ptr(int ch, uint32_t& addr, bool is_16bit_mode);
	uint32_t __FASTCALL inc_memory_ptr(int ch, uint32_t& addr, bool is_16bit_mode);
public:
	UPD71071(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		// TIP: if((DEVICE::prev_device == NULL) || (DEVICE::this_device_id == 0)) DEVICE must be DUMMY.
		// And, at this device, should not be FIRST DEVICE. 20170613 Ohta.
		DEVICE *__dev = this;
		while((__dev->prev_device != NULL) && (__dev->this_device_id > 0)) {
			__dev = __dev->prev_device;
		}
		for(int i = 0; i < 4; i++) {
			//dma[i].dev = vm->dummy;
			dma[i].dev = __dev;
		}
		d_mem = __dev;
//#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
//#endif
		d_debugger = NULL;
		_SINGLE_MODE_DMA = false;
		for(int i = 0; i < 4; i++) {
			initialize_output_signals(&outputs_tc[i]);
			initialize_output_signals(&outputs_ack[i]);
			initialize_output_signals(&outputs_ube[i]);
			memory_width_is_16bit[c] = false;
			device_width_is_16bit[c] = false;
			extend_to_32bit[c] = false;
		}
		set_device_name(_T("uPD71071 DMAC"));
	}
	~UPD71071() {}

	// common functions
	virtual void initialize() override;
	virtual void reset() override;
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data)  override;
	virtual uint32_t __FASTCALL read_io8(uint32_t addr) override;

	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t _mask) override;
	virtual void __FASTCALL do_dma() override;
	// for debug
	virtual void __FASTCALL write_via_debugger_data8w(uint32_t addr, uint32_t data, int *wait) override;
	virtual uint32_t __FASTCALL read_via_debugger_data8w(uint32_t addr, int *wait) override;
	virtual void __FASTCALL write_via_debugger_data16w(uint32_t addr, uint32_t data, int *wait) override;
	virtual uint32_t __FASTCALL read_via_debugger_data16w(uint32_t addr, int *wait) override;
	bool is_debugger_available() override
	{
		return true;
	}
	void *get_debugger() override
	{
		return d_debugger;
	}
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;
	virtual bool process_state(FILEIO* state_fio, bool loading) override;
	// unique functions
	void set_context_memory(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_ch0(DEVICE* device)
	{
		dma[0].dev = device;
	}
	void set_context_ch1(DEVICE* device)
	{
		dma[1].dev = device;
	}
	void set_context_ch2(DEVICE* device)
	{
		dma[2].dev = device;
	}
	void set_context_ch3(DEVICE* device)
	{
		dma[3].dev = device;
	}
//#ifdef SINGLE_MODE_DMA
	void set_context_child_dma(DEVICE* device)
	{
		d_dma = device;
	}
//#endif
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
	void set_context_tc0(DEVICE* device, int id, uint32_t _mask)
	{
		register_output_signal(&outputs_tc[0], device, id, _mask);
	}
	void set_context_tc1(DEVICE* device, int id, uint32_t _mask)
	{
		register_output_signal(&outputs_tc[1], device, id, _mask);
	}
	void set_context_tc2(DEVICE* device, int id, uint32_t _mask)
	{
		register_output_signal(&outputs_tc[2], device, id, _mask);
	}
	void set_context_tc3(DEVICE* device, int id, uint32_t _mask)
	{
		register_output_signal(&outputs_tc[3], device, id, _mask);
	}
	void set_context_ack0(DEVICE* device, int id, uint32_t _mask)
	{
		register_output_signal(&outputs_ack[0], device, id, _mask);
	}
	void set_context_ack1(DEVICE* device, int id, uint32_t _mask)
	{
		register_output_signal(&outputs_ack[1], device, id, _mask);
	}
	void set_context_ack2(DEVICE* device, int id, uint32_t _mask)
	{
		register_output_signal(&outputs_ack[2], device, id, _mask);
	}
	void set_context_ack3(DEVICE* device, int id, uint32_t _mask)
	{
		register_output_signal(&outputs_ack[3], device, id, _mask);
	}

	void set_context_ube0(DEVICE* device, int id, uint32_t _mask)
	{
		register_output_signal(&outputs_ube[0], device, id, _mask);
	}
	void set_context_ube1(DEVICE* device, int id, uint32_t _mask)
	{
		register_output_signal(&outputs_ube[1], device, id, _mask);
	}
	void set_context_ube2(DEVICE* device, int id, uint32_t _mask)
	{
		register_output_signal(&outputs_ube[2], device, id, _mask);
	}
	void set_context_ube3(DEVICE* device, int id, uint32_t _mask)
	{
		register_output_signal(&outputs_ube[3], device, id, _mask);
	}

	// 20230318 K.Ohta
	// These set bus width to 8bit (false) or 16bit (true).
	// May be useful for SCSI controller of FM Towns.
	void set_memory_width_is_16bit(int ch, bool flag)
	{
	   memory_width_is_16bit[ch & 3] = flag;
	}
	void set_device_width_is_16bit(int ch, bool flag)
	{
	   device_width_is_16bit[ch & 3] = flag;
	}
	// This is for FM Towns.
	// FM Towns implements extended variant as DMAC.
	void set_dma_extend_to_32bit(int ch, bool flag)
	{
	   extend_to_32bit[ch & 3] = flag;
	}
};

inline uint16_t UPD71071::read_from_memory_8bit(int ch, int *wait, bool __debugging)
{
	uint16_t val;
	const uint32_t _addr_mask = (extend_to_32bit) ? 0xffffffff : 0x00ffffff;
	const uint32_t addr = dma[ch].areg & _addr_mask;
	__UNLIKELY_IF(__debugging) {
		val = d_debugger->read_via_debugger_data8w(addr, wait);
	} else {
//#endif
		val = this->read_via_debugger_data8w(addr, wait);
	}
	val = val & 0x00ff;
	return val;
}

inline uint16_t UPD71071::read_from_memory_16bit(int ch, int *wait, bool __debugging)
{
	uint16_t val;
	ch = ch & 3;
	const uint32_t _addr_mask = (extend_to_32bit) ? 0xffffffff : 0x00ffffff;
	const uint32_t addr = dma[ch].areg & _addr_mask;
	if(memory_width_is_16bit[ch]) {
		__UNLIKELY_IF(__debugging) {
			val = d_debugger->read_via_debugger_data8w(addr, wait);
		} else {
//#endif
			val = this->read_via_debugger_data8w(addr, wait);
		}
	} else {
		// 8bit * 2
		uint16_t h, l;
		int wait_l = 0, wait_h = 0;
		const uint32_t addr2 = (addr + 1) & _addr_mask;
		__UNLIKELY_IF(__debugging) {
			l = d_debugger->read_via_debugger_data8w(addr, &wait_l);
			h = d_debugger->read_via_debugger_data8w(addr2, &wait_h);
//#endif
		} else {
			l = this->read_via_debugger_data8w(addr, &wait_l);
			h = this->read_via_debugger_data8w(addr2, &wait_h);
		}
		val = ((h & 0xff) << 8) | (l & 0xff);
		*wait = wait_h + wait_l;
	}
	return val;
}

inline void UPD71071::write_to_memory_8bit(int ch, uint16_t data, int *wait, bool __debugging)
{
	const uint32_t _addr_mask = (extend_to_32bit) ? 0xffffffff : 0x00ffffff;
	const uint32_t addr = dma[ch].areg & _addr_mask;
	__UNLIKELY_IF(__debugging) {
		d_debugger->write_via_debugger_data8w(addr, data & 0xff, wait);
	} else {
//#endif
		this->write_via_debugger_data8w(addr, data & 0xff, wait);
	}
}

inline void UPD71071::write_to_memory_16bit(int ch, uint16_t data, int *wait, bool __debugging)
{
	ch = ch & 3;
	const uint32_t _addr_mask = (extend_to_32bit) ? 0xffffffff : 0x00ffffff;
	const uint32_t addr = dma[ch].areg & _addr_mask;
	if(memory_width_is_16bit[ch]) {
		__UNLIKELY_IF(__debugging) {
			d_debugger->write_via_debugger_data8w(addr, data, wait);
		} else {
//#endif
			this->write_via_debugger_data8w(addr, data, wait);
		}
	} else {
		// 8bit * 2
		uint16_t h, l;
		int wait_l = 0, wait_h = 0;
		const uint32_t addr2 = (addr + 1) & _addr_mask;
		h = (data >> 8) & 0xff;
		l = data & 0xff;
		__UNLIKELY_IF(__debugging) {
			d_debugger->write_via_debugger_data8w(addr, l, &wait_l);
			d_debugger->write_via_debugger_data8w(addr2, h, &wait_h);
//#endif
		} else {
			this->write_via_debugger_data8w(addr, l, &wait_l);
			this->write_via_debugger_data8w(addr2, h, &wait_h);
		}
		*wait = wait_h + wait_l;
	}
	return val;
}
inline uint16_t UPD71071::read_from_io_8bit(int ch, int *wait)
{
	// ToDo: 16bit transfer bus, maybe set only tmpreg (at odd count).
	ch = ch & 3;
	uint16_t val;
	val = dma[ch].dev->read_dma_io8w(0, wait) & 0xff;
	return val;
}

inline uint16_t UPD71071::read_from_io_16bit(int ch, int *wait)
{
	ch = ch & 3;
	if(io_width_is_16bit[ch]) {
		uint16_t val;
		val = dma[ch].dev->read_dma_io16w(0, wait);
		return val;
	} else {
		uint16_t h, l;
		int wait_h = 0, wait_l = 0;
		l = dma[ch].dev->read_dma_io8w(0, &wait_l) & 0xff;
		tmp = (tmp >> 8) | (l << 8);
		h = dma[ch].dev->read_dma_io8w(0, &wait_h) & 0xff;
		tmp = (tmp >> 8) | (h << 8);
		*wait = wait_h + wait_l;
		return (h << 8) | l;
	}
}

inline uint16_t UPD71071::write_to_io_8bit(int ch, uint16_t data, int *wait)
{
	// ToDo: 16bit transfer bus, maybe set only tmpreg (at odd count).
	ch = ch & 3;
	dma[ch].dev->write_dma_io8w(0, data, wait);
}
inline uint16_t UPD71071::write_to_io_16bit(int ch, uint16_t data, int *wait)
{
	ch = ch & 3;
	if(io_width_is_16bit[ch]) {
		dma[ch].dev->write_dma_io16w(0, data, wait);
	} else {
		uint16_t h, l;
		int wait_h = 0, wait_l = 0;
		h = (data >> 8) & 0xff;
		l = data & 0xff;
		dma[ch].dev->write_dma_io8w(0, l, &wait_l);
		dma[ch].dev->write_dma_io8w(0, h, &wait_h);
		*wait = wait_h + wait_l;
	}
}

uint32_t UPD71071::inc_memory_ptr(int ch, uint32_t& addr, bool is_16bit_mode)
{
	ch = ch & 3;
	const uint32_t _addr_mask = (extend_to_32bit) ? 0xffffffff : 0x00ffffff;
	const uint32_t val = (is_16bit_mode) ? 2 : 1;
	addr = (addr + val) & _addr_mask;
	return addr;
}

uint32_t UPD71071::dec_memory_ptr(int ch, uint32_t& addr, bool is_16bit_mode)
{
	ch = ch & 3;
	const uint32_t _addr_mask = (extend_to_32bit) ? 0xffffffff : 0x00ffffff;
	const uint32_t val = (is_16bit_mode) ? 2 : 1;
	addr = (addr + val) & _addr_mask;
	return addr;
}
#endif
