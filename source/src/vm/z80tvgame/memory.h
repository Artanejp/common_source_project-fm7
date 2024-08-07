/*
	Homebrew Z80 TV GAME SYSTEM Emulator 'eZ80TVGAME'

	Author : Takeda.Toshiya
	Date   : 2015.04.28-

	[ memory ]
*/

// http://w01.tp1.jp/~a571632211/z80tvgame/index.html

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace Z80TVGAME {

class MEMORY : public DEVICE
{
private:
	// memory
	uint8_t rom[0x8000];
	uint8_t ram[0x6000];
	uint8_t wdmy[0x1000];
	uint8_t rdmy[0x1000];
	uint8_t* wbank[16];
	uint8_t* rbank[16];
	
	bool inserted;

	__DECL_ALIGNED(32) _bit_trans_table_scrn_t pixel_trans_table;
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize() override;
	void reset() override;
	void __FASTCALL write_data8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_data8(uint32_t addr) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique functions
	void open_cart(const _TCHAR* file_path);
	void close_cart();
	bool is_cart_inserted()
	{
		return inserted;
	}
	void draw_screen();
};

}
#endif

