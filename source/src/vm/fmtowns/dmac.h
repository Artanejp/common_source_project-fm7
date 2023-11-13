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
#define SIG_TOWNS_DMAC_MASK_CH0		8196
#define SIG_TOWNS_DMAC_MASK_CH1		8197
#define SIG_TOWNS_DMAC_MASK_CH2		8198
#define SIG_TOWNS_DMAC_MASK_CH3		8199
// Mainly process from SCSI_HOST:: 's ACK.
//#define SIG_TOWNS_DMAC_ACKREQ_CH0	8200
//#define SIG_TOWNS_DMAC_ACKREQ_CH1	8201
//#define SIG_TOWNS_DMAC_ACKREQ_CH2	8202
//#define SIG_TOWNS_DMAC_ACKREQ_CH3	8203

namespace FMTOWNS {
class TOWNS_DMAC : public UPD71071
{
protected:
	bool dma_wrap;

	bool force_16bit_transfer[4];
	outputs_t outputs_ube[4];
	outputs_t outputs_ack[4];
	outputs_t outputs_towns_tc[4];
	outputs_t outputs_mask_reg;

	bool primary_dmac;

	// Temporally workaround for SCSI.20200318 K.O
	bool address_aligns_16bit[4];
	bool is_16bit_transfer[4];
	bool is_16bit[4];

	bool end_req[4];
	bool end_stat[4];
	double dmac_cycle_us;
	int spent_clocks;
	int clock_multiply;
	uint8_t transfer_ch;

	int event_dmac_cycle;

	inline void __FASTCALL set_ack(int ch, bool val)
	{
		write_signals(&(outputs_ack[ch]), (val) ? 0xffffffff : 0);
	}
	inline void __FASTCALL set_mask_reg(uint8_t val)
	{
		mask = val;
		uint8_t val2 = ~val;
		write_signals(&outputs_mask_reg, val2);
	}
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

	inline void __FASTCALL reset_dma_counter(int ch)
	{
		uint8_t c = ch & 3;
		uint8_t bit = 1 << c;
		if(dma[c].mode & 0x10) {
			// auto initialize
			dma[c].areg = dma[c].bareg;
			dma[c].creg = dma[c].bcreg;
			//mask |= bit;
		} else {
			mask |= bit;
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
		//tc |= bit;	// NOT From MAME 0.246 ;
		if(is_send_tc) {
			write_signals(&outputs_towns_tc[c], 0xffffffff);
		}
		tc |= bit;	// From MAME 0.246 ;
					// TC REGISTER's BIT maybe set after TC line asserted. 20230521 K.O
	}
	bool __FASTCALL check_is_16bit(int ch);

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

	virtual void do_dma_internal();
	virtual int  __FASTCALL do_dma_single(const int ch, const bool is_use_debugger, bool compressed, bool extended, bool& is_terminated, bool& is_single);
	virtual bool __FASTCALL decrement_counter(const int ch, uint8_t mode, uint16_t& counter, bool& is_single);

	void __FASTCALL do_dma_16bit(DEVICE* dev, const uint8_t tr_mode, uint32_t& memory_address, const bool compressed, const bool extended, bool is_use_debugger, int& wait);
	void __FASTCALL do_dma_8bit(DEVICE* dev, const uint8_t tr_mode, uint32_t& memory_address, const bool compressed, const bool extended, bool is_use_debugger, int& wait);
	void check_mask_and_cmd();
	virtual void reset_from_io();

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
		primary_dmac = true;
		clock_multiply = 1;
		event_dmac_cycle = -1;
		initialize_output_signals(&outputs_mask_reg);
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
	void set_dmac_clock(double clock_hz, int ratio);
	void set_primary_dmac(bool is_primary)
	{
		primary_dmac = is_primary;
	}
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
	void set_context_mask_bit(DEVICE* device, int id, uint8_t ch)
	{
		ch = ch & 3;
		uint32_t mask_bit = 1 << ch;
		register_output_signal(&outputs_mask_reg, device, id, mask_bit);
	}
};


}
