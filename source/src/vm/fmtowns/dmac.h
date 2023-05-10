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
	bool end_req[4];
	bool end_stat[4];
	bool force_16bit_transfer[4];
	bool is_16bit_transfer[4];
	outputs_t outputs_ube[4];

	int event_dmac_cycle;
	uint8_t div_count;
	// Temporally workaround for SCSI.20200318 K.O
//	bool creg_set[4];
//	bool bcreg_set[4];
	virtual void __FASTCALL inc_dec_ptr_a_byte(const int c, const bool inc) override;
	virtual void __FASTCALL inc_dec_ptr_two_bytes(const int c, const bool inc) override;

	virtual uint32_t __FASTCALL read_16bit_from_device(DEVICE* dev, uint32_t addr, int* wait);
	virtual void __FASTCALL write_16bit_to_device(DEVICE* dev, uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t __FASTCALL read_16bit_from_memory(uint32_t addr, int* wait, bool is_use_debugger);
	virtual void __FASTCALL write_16bit_to_memory(uint32_t addr, uint32_t data, int* wait, bool is_use_debugger);
	virtual bool __FASTCALL do_dma_per_channel(int ch, bool is_use_debugger, bool force_exit);
public:
	TOWNS_DMAC(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : UPD71071(parent_vm, parent_emu)
	{
		set_device_name(_T("FM-Towns uPD71071 DMAC"));
		for(int ch = 0; ch < 4; ch++) {
			force_16bit_transfer[ch] = false;
			is_16bit_transfer[ch] = false;
			initialize_output_signals(&outputs_ube[ch]);
		}
	}
	~TOWNS_DMAC() {}
	// common functions
	virtual void initialize() override;
	virtual void reset() override;
	virtual void do_dma() override;

	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_io8(uint32_t addr) override;
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
};

}
