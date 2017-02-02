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
class KANJIROM: public DEVICE {
private:
	EMU *p_emu;
	uint8_t data_table[0x20000];
	bool read_ok;
	bool class2;
	pair_t kanjiaddr;
public:
	KANJIROM(VM *parent_vm, EMU* parent_emu, bool type_2std);
	~KANJIROM();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void release();
	void reset(void);
	bool get_readstat(void);
	
	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);
};
#endif
