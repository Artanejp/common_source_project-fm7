/*
 * Common source code project -> FM-7/77/AV -> Kanji rom
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *  Feb 11, 2015 : Initial
 */

#ifndef _FM7_KANJIROM_H_
#define _FM7_KANJIROM_H_

#include "./fm7.h"
#include "fm7_common.h"
#include "../device.h"

class EMU;
class VM;

namespace FM7 {

class KANJIROM: public DEVICE {
private:
	uint8_t data_table[0x20000];
	bool read_ok;
	bool class2;
	pair32_t kanjiaddr;

public:
	KANJIROM(VM_TEMPLATE* parent_vm, EMU* parent_emu, bool type_2std);
	~KANJIROM();
	void __FASTCALL write_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_data8(uint32_t addr);
	void release();
	void reset(void);
	bool get_readstat(void);

	bool process_state(FILEIO *state_fio, bool loading);
};

}
#endif
