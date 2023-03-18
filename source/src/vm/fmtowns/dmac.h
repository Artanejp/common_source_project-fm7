#pragma once

#include "../upd71071.h"

// Using original signal using after 1 << 12.
#define SIG_TOWNS_DMAC_ADDR_REG     4096
#define SIG_TOWNS_DMAC_WRAP_REG     4100
#define SIG_TOWNS_DMAC_ADDR_MASK    4104

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

namespace FMTOWNS {
class TOWNS_DMAC : public UPD71071
{
protected:
	uint8_t dma_wrap_reg;
	uint32_t dma_addr_mask;

	// Temporally workaround for SCSI.20200318 K.O
	bool creg_set[4];
	bool bcreg_set[4];
	virtual void __FASTCALL do_dma_inc_dec_ptr_8bit(int c);
	virtual void __FASTCALL do_dma_inc_dec_ptr_16bit(int c);
	virtual bool __FASTCALL do_dma_epilogue(int c);

	virtual void __FASTCALL do_dma_dev_to_mem_8bit(int c);
	virtual void __FASTCALL do_dma_mem_to_dev_8bit(int c);
	virtual void __FASTCALL do_dma_dev_to_mem_16bit(int c);
	virtual void __FASTCALL do_dma_mem_to_dev_16bit(int c);

public:
	TOWNS_DMAC(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : UPD71071(parent_vm, parent_emu)
	{
		set_device_name(_T("FM-Towns uPD71071 DMAC"));
	}
	~TOWNS_DMAC() {}
	// common functions
	virtual void initialize();
	virtual void reset();

	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	virtual void __FASTCALL write_io16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_io16(uint32_t addr);

	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t _mask);
	virtual uint32_t __FASTCALL read_signal(int id);

	virtual bool process_state(FILEIO* state_fio, bool loading);

	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
};

}
