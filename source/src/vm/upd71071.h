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
	DEVICE* d_cpu;
	DEVICE* d_mem;
//#ifdef SINGLE_MODE_DMA
	DEVICE* d_dma;
//#endif
	DEBUGGER *d_debugger;

	bool _SINGLE_MODE_DMA;

	outputs_t outputs_tc[4];
	outputs_t outputs_ube[4]; // If "1" word transfer, "0" byte transfer (OUT)
	outputs_t outputs_ack[4]; // Acknoledge

	struct {
		DEVICE* dev;
		uint32_t areg, bareg;
		uint32_t creg, bcreg; // 20200318 K.O (Temporally workaround for Towns)
		uint8_t mode;
		bool is_16bit;
		bool endreq;
		bool end;
	} dma[4];

	uint8_t b16, selch, base;
	uint16_t cmd, tmp;
	uint8_t req, sreq, mask, tc;
	bool running;

	bool inputs_ube[4];
	bool stats_ube[4];

	// Device accessing functions.
	virtual void __FASTCALL do_dma_verify_8bit(int c, bool extended, bool compressed, int& wait);
	virtual void __FASTCALL do_dma_dev_to_mem_8bit(int c, bool extended, bool compressed, int& wait);
	virtual void __FASTCALL do_dma_mem_to_dev_8bit(int c, bool extended, bool compressed, int& wait);
	virtual void __FASTCALL do_dma_verify_16bit(int c, bool extended, bool compressed, int& wait);
	virtual void __FASTCALL do_dma_dev_to_mem_16bit(int c, bool extended, bool compressed, int& wait);
	virtual void __FASTCALL do_dma_mem_to_dev_16bit(int c, bool extended, bool compressed, int& wait);

	// For DMA sequence.
	virtual bool __FASTCALL do_dma_epilogue(int c);
	virtual bool __FASTCALL do_dma_per_channel(int c);

	virtual void __FASTCALL do_dma_inc_dec_ptr_8bit(int c);
	virtual void __FASTCALL do_dma_inc_dec_ptr_16bit(int c);

	// For manipulating signal PINs.
	inline void __FASTCALL reset_tc(int ch);
	inline void __FASTCALL set_tc(int ch);
	inline void reset_all_tc();

	inline void __FASTCALL reset_dma_ack(int ch);
	inline void __FASTCALL set_dma_ack(int ch);
	inline void __FASTCALL set_ube(int ch);
	inline void __FASTCALL reset_ube(int ch);

	// Wodr manipulating.
	inline uint16_t __FASTCALL manipulate_a_byte_from_word_le(uint16_t src, uint8_t pos, uint8_t data);
	inline uint32_t __FASTCALL manipulate_a_byte_from_dword_le(uint32_t src, uint8_t pos, uint8_t data);

public:
	UPD71071(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		// TIP: if((DEVICE::prev_device == NULL) || (DEVICE::this_device_id == 0)) DEVICE must be DUMMY.
		// And, at this device, should not be FIRST DEVICE. 20170613 Ohta.
		for(int i = 0; i < 4; i++) {
			//dma[i].dev = vm->dummy;
			dma[i].dev = vm->dummy;
		}
		d_cpu = NULL;
		d_mem = vm->dummy;
//#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
//#endif
		d_debugger = NULL;
		_SINGLE_MODE_DMA = false;

		for(int i = 0; i < 4; i++) {
			initialize_output_signals(&outputs_tc[i]);
			initialize_output_signals(&outputs_ube[i]);
			initialize_output_signals(&outputs_ack[i]);
		}
		set_device_name(_T("uPD71071 DMAC"));
	}
	~UPD71071() {}

	// common functions
	virtual void initialize() override;
	virtual void reset() override;

	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_io8(uint32_t addr) override;
	virtual void __FASTCALL write_io16(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_io16(uint32_t addr) override;

	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t _mask) override;
	virtual uint32_t __FASTCALL read_signal(int id) override;
	virtual void __FASTCALL do_dma() override;
	// for debug
	virtual void __FASTCALL write_via_debugger_data8w(uint32_t addr, uint32_t data, int *wait) override;
	virtual uint32_t __FASTCALL read_via_debugger_data8w(uint32_t addr, int *wait) override;
	virtual void __FASTCALL write_via_debugger_data16w(uint32_t addr, uint32_t data, int *wait) override;
	virtual uint32_t __FASTCALL read_via_debugger_data16w(uint32_t addr, int *wait) override;

	bool is_debugger_available() override
	{
		__UNLIKELY_IF(!(__USE_DEBUGGER) || (d_debugger == nullptr)) {
			return false;
		}
		return true;
	}
	void *get_debugger() override
	{
		__UNLIKELY_IF(!(__USE_DEBUGGER) || (d_debugger == nullptr)) {
			return nullptr;
		}
		return d_debugger;
	}
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;
	virtual bool process_state(FILEIO* state_fio, bool loading) override;
	// unique functions
	void set_context_memory(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
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
};

inline uint16_t UPD71071::manipulate_a_byte_from_word_le(uint16_t src, uint8_t pos, uint8_t data)
{
	pair16_t n;
	n.w = src;
	switch(pos) {
	case 0:
		n.b.l = data;
		break;
	case 1:
		n.b.h = data;
		break;
	}
	return n.w;
}

inline uint32_t UPD71071::manipulate_a_byte_from_dword_le(uint32_t src, uint8_t pos, uint8_t data)
{
	pair32_t n;
	n.d = src;
	switch(pos) {
	case 0:
		n.b.l  = data;
		break;
	case 1:
		n.b.h  = data;
		break;
	case 2:
		n.b.h2 = data;
		break;
	case 3:
		n.b.h3 = data;
		break;
	}
	return n.d;
}

inline void UPD71071::reset_all_tc()
{
	for(int i = 0; i < 4; i++) {
		tc = 0;
		write_signals(&(outputs_tc[i]), 0);
	}
}

inline void UPD71071::reset_tc(int ch)
{
	if((ch < 0) || (ch > 3)) return;
	uint8_t bit = (1 << ch);
	uint8_t tc_bak = tc;
	tc &= ~bit;
	/*if(tc != tc_bak) */ write_signals(&(outputs_tc[ch]), 0);
}

inline void UPD71071::set_tc(int ch)
{
	if((ch < 0) || (ch > 3)) return;
	uint8_t bit = (1 << ch);
	uint8_t tc_bak = tc;
	tc |= bit;
	/*if(tc != tc_bak) */write_signals(&(outputs_tc[ch]), 0xffffffff);
}

inline void UPD71071::set_ube(int ch)
{
	bool stat = inputs_ube[ch & 3];
	stat &= dma[ch & 3].is_16bit;
	if(stats_ube[ch & 3] != stat) {
		write_signals(&outputs_ube[ch & 3], (stat) ? 0xffffffff : 0x00000000);
		stats_ube[ch & 3] = stat;
	}
}

inline void UPD71071::reset_ube(int ch)
{
	if(stats_ube[ch &3]) {
		write_signals(&outputs_ube[ch & 3], 0x00000000);
		stats_ube[ch & 3] = false;
	}
}

inline void UPD71071::set_dma_ack(int ch)
{
	write_signals(&outputs_ack[ch & 3], 0xffffffff);
}

inline void UPD71071::reset_dma_ack(int ch)
{
	write_signals(&outputs_ack[ch & 3], 0x00000000);
}


#endif
