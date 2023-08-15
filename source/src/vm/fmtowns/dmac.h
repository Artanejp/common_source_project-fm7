#pragma once

#include "../upd71071.h"

// Using original signal using after 1 << 12.
#define SIG_TOWNS_DMAC_ADDR_REG		4096
#define SIG_TOWNS_DMAC_WRAP			4100
#define SIG_TOWNS_DMAC_ADDR_MASK	4104
#define SIG_TOWNS_DMAC_EOT_CH0		8192
#define SIG_TOWNS_DMAC_EOT_CH1		8193
#define SIG_TOWNS_DMAC_EOT_CH2		8194
#define SIG_TOWNS_DMAC_EOT_CH3		8195

namespace FMTOWNS {
class TOWNS_DMAC : public UPD71071
{
protected:
	bool dma_wrap;
	bool force_16bit_transfer[4];
	outputs_t outputs_ube[4];
	outputs_t outputs_ack[4];
	outputs_t outputs_towns_tc[4];

	uint8_t div_count;
	// Temporally workaround for SCSI.20200318 K.O
	bool address_aligns_16bit[4];
	bool is_16bit_transfer[4];
	bool is_16bit[4];

	bool is_started[4];
	bool end_req[4];
	bool end_stat[4];
	double dmac_cycle_us;
	int event_dmac_cycle;

	virtual void __FASTCALL call_dma(int ch);
	inline void __FASTCALL set_ack(int ch, bool val)
	{
		write_signals(&(outputs_ack[ch]), (val) ? 0xffffffff : 0);
	}
	void check_start_condition();
	constexpr bool check_address_16bit_bus_changed(int ch)
	{
		bool __is_align_bak = address_aligns_16bit[ch];
		address_aligns_16bit[ch] = ((dma[ch].areg & 0x00000001) == 0);
		__UNLIKELY_IF(__is_align_bak != address_aligns_16bit[ch]) {
			calc_transfer_status(ch);
			return true;
		}
		return false;
	}
	constexpr void __FASTCALL set_ube_line(int ch)
	{
		write_signals(&(outputs_ack[ch]), (is_16bit[ch]) ? 0xffffffff : 0);
	}
	virtual inline void check_running()
	{
		__UNLIKELY_IF((event_dmac_cycle < 0) || (_SINGLE_MODE_DMA)) {
			return;
		}
		for(int ch = 0; ch < 4; ch++) {
			__UNLIKELY_IF(is_started[ch]) {
				return;
			}
		}
		// OK. All channels are stopped.
		cancel_event(this, event_dmac_cycle);
		event_dmac_cycle = -1;
		return;
	}

	inline void __FASTCALL reset_dma_counter(int ch)
	{
		uint8_t c = ch & 3;
		uint8_t bit = 1 << c;
		if(dma[c].mode & 0x10) {
			// auto initialize
			dma[c].areg = dma[c].bareg;
			dma[c].creg = dma[c].bcreg;
		} else {
			mask |= bit;
			is_started[c] = false;
		}
	}
	inline void __FASTCALL calc_transfer_status(int ch)
	{
		address_aligns_16bit[ch] = ((dma[ch].areg & 0x00000001) == 0);
		is_16bit_transfer[ch] = (((dma[ch].mode & 0x01) == 1)
									&& (b16)
									&& (address_aligns_16bit[selch]));
		is_16bit[ch] = (is_16bit_transfer[ch] || force_16bit_transfer[ch]);
	}
	constexpr void do_end_sequence(int c, bool is_send_tc)
	{
		c = c & 3;
		const uint8_t bit = 1 << c;
		reset_dma_counter(c);
		req &= ~bit;
		sreq &= ~bit;
		running = false;
		end_req[c] = false;
		check_running();

		tc |= bit;	// NOT From MAME 0.246 ;
		if(is_send_tc) {
			write_signals(&outputs_towns_tc[c], 0xffffffff);
		}
		//tc |= bit;	// From MAME 0.246 ;
					// TC REGISTER's BIT maybe set after TC line asserted. 20230521 K.O
	}
	virtual void __FASTCALL inc_dec_ptr_a_byte(uint32_t& addr, const bool inc) override;
	virtual void __FASTCALL inc_dec_ptr_two_bytes(uint32_t& addr, const bool inc) override;

	virtual uint32_t __FASTCALL read_16bit_from_device(DEVICE* dev, uint32_t addr, int* wait);
	virtual void __FASTCALL write_16bit_to_device(DEVICE* dev, uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t __FASTCALL read_16bit_from_memory(uint32_t addr, int* wait, bool is_use_debugger);
	virtual void __FASTCALL write_16bit_to_memory(uint32_t addr, uint32_t data, int* wait, bool is_use_debugger);

	virtual uint32_t __FASTCALL read_8bit_from_device(DEVICE* dev, uint32_t addr, int* wait);
	virtual void __FASTCALL write_8bit_to_device(DEVICE* dev, uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t __FASTCALL read_8bit_from_memory(uint32_t addr, int* wait, bool is_use_debugger);
	virtual void __FASTCALL write_8bit_to_memory(uint32_t addr, uint32_t data, int* wait, bool is_use_debugger);

	virtual bool __FASTCALL do_dma_per_channel(int ch, bool is_use_debugger, bool force_exit);
	virtual void do_dma_internal();
	void __FASTCALL do_dma_16bit(DEVICE* dev, const uint8_t tr_mode, uint32_t& memory_address, const bool compressed, const bool extended, bool is_use_debugger, int& wait);
	void __FASTCALL do_dma_8bit(DEVICE* dev, const uint8_t tr_mode, uint32_t& memory_address, const bool compressed, const bool extended, bool is_use_debugger, int& wait);

public:
	TOWNS_DMAC(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : UPD71071(parent_vm, parent_emu)
	{
		dmac_cycle_us = 1.0 / 4.0; // Default is 4MHz.
		set_device_name(_T("FM-Towns uPD71071 DMAC"));
		for(int ch = 0; ch < 4; ch++) {
			force_16bit_transfer[ch] = false;
			is_16bit_transfer[ch] = false;
			is_16bit[ch] = false;
			initialize_output_signals(&outputs_ube[ch]);
			initialize_output_signals(&outputs_ack[ch]);
			initialize_output_signals(&outputs_towns_tc[ch]);
		}
	}
	~TOWNS_DMAC() {}
	// common functions
	virtual void initialize() override;
	virtual void reset() override;
	virtual void do_dma() override;

	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	virtual void __FASTCALL write_io16(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_io8(uint32_t addr) override;
	virtual uint32_t __FASTCALL read_io16(uint32_t addr) override;
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t _mask) override;
	virtual uint32_t __FASTCALL read_signal(int id) override;
	virtual void __FASTCALL event_callback(int id, int err) override;
	virtual bool process_state(FILEIO* state_fio, bool loading) override;

	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;

	// Unique functions
	// This is workaround for FM-Towns's SCSI.
	void set_force_16bit_transfer(int ch, bool is_force)
	{
		force_16bit_transfer[ch & 3] = is_force;
	}
	void set_context_ube(int ch, DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ube[ch & 3], device, id, mask);
	}
	void set_context_ack(int ch, DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ack[ch & 3], device, id, mask);
	}
	void set_context_tc0(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_towns_tc[0], device, id, mask);
	}
	void set_context_tc1(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_towns_tc[1], device, id, mask);
	}
	void set_context_tc2(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_towns_tc[2], device, id, mask);
	}
	void set_context_tc3(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_towns_tc[3], device, id, mask);
	}
};


}
