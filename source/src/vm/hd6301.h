#ifndef _HD6301_H_ 
#define _HD6301_H_

#include "mc6801.h"
#include "device.h"

class DEBUGGER;
class FIFO;
//#endif
class HD6301 : public MC6801
{
protected:
#define XX 5
	const uint8_t cycles[256] = {
//#elif defined(HAS_HD6301)
		XX, 1,XX,XX, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1,XX,XX,XX,XX, 1, 1, 2, 2, 4, 1,XX,XX,XX,XX,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		1, 1, 3, 3, 1, 1, 4, 4, 4, 5, 1,10, 5, 7, 9,12,
		1,XX,XX, 1, 1,XX, 1, 1, 1, 1, 1,XX, 1, 1,XX, 1,
		1,XX,XX, 1, 1,XX, 1, 1, 1, 1, 1,XX, 1, 1,XX, 1,
		6, 7, 7, 6, 6, 7, 6, 6, 6, 6, 6, 5, 6, 4, 3, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 4, 3, 5,
		2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 5, 3, 3,
		3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 5, 4, 4,
		4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
		4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5,
		2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3,XX, 3, 3,
		3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
		4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
		4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5
//#elif defined(HAS_MB8861)
	};
#undef XX	
	uint16_t latch09;

	void __FASTCALL mc6801_io_w(uint32_t offset, uint32_t data) override;
	void __FASTCALL insn(uint8_t code) override;
	
	void __FASTCALL undoc1();
	void __FASTCALL undoc2();
	void __FASTCALL xgdx();

	void __FASTCALL slp();
	void __FASTCALL aim_ix();
	void __FASTCALL oim_ix();
	void __FASTCALL eim_ix();
	void __FASTCALL tim_ix();
	void __FASTCALL aim_di();
	void __FASTCALL oim_di();
	void __FASTCALL eim_di();
	void __FASTCALL tim_di();

	void __FASTCALL illegal() override;
		
public:
	HD6301(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MC6801(parent_vm, parent_emu)
	{
		set_device_name(_T("HD6301 MPU"));
	}
	~HD6301() {}
	bool process_state(FILEIO* state_fio, bool loading);

	int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0) override;

};
#endif
