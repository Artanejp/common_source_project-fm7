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

	void mc6801_io_w(uint32_t offset, uint32_t data) override;
	void insn(uint8_t code) override;
	
	void undoc1();
	void undoc2();
	void xgdx();

	void slp();
	void aim_ix();
	void oim_ix();
	void eim_ix();
	void tim_ix();
	void aim_di();
	void oim_di();
	void eim_di();
	void tim_di();

	void illegal() override;
		
public:
	HD6301(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MC6801(parent_vm, parent_emu)
	{
		set_device_name(_T("HD6301 MPU"));
	}
	~HD6301() {}
	bool process_state(FILEIO* state_fio, bool loading);

	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len) override;

};
#endif
