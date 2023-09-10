#pragma once

#include "device.h"
#include "../../common.h"
#include "towns_common.h"

namespace FMTOWNS {
class TOWNS_VRAM;

class PLANEVRAM : public DEVICE
{
protected:
	DEVICE* d_crtc;
	DEVICE* d_sprite;
	TOWNS_VRAM* d_vram;

	uint8_t mix_reg;             // MMIO 000CH:FF80H
	uint8_t r50_readplane;       // MMIO 000CH:FF81H : BIT 7 and 6.
	uint8_t r50_ramsel;          // MMIO 000CH:FF81H : BIT 3 to 0.
	uint32_t r50_gvramsel;        // MMIO 000CH:FF83H : bit4 (and 3).

public:
	PLANEVRAM(VM_TEMPLATE* parent_vm, EMU_TEMPLATE*parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_crtc = NULL;
		d_sprite = NULL;
		d_vram = NULL;
	}

	void initialize() override;
	void reset() override;

	virtual uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr) override;
	virtual void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_dma_data8w(uint32_t addr, int* wait) override;
	virtual void __FASTCALL write_dma_data8w(uint32_t addr, uint32_t data, int* wait) override;

	virtual uint32_t __FASTCALL read_io8(uint32_t addr) override;
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;

	virtual bool process_state(FILEIO* state_fio, bool loading) override;

	// unique functions
	void set_context_crtc(DEVICE* dev)
	{
		d_crtc = dev;
	}
	void set_context_sprite(DEVICE* dev)
	{
		d_sprite = dev;
	}
	void set_context_vram(TOWNS_VRAM* dev)
	{
		d_vram = dev;
	}

};
}
