/*
 * Common source code project -> FM-7/77/AV -> Kanji rom
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *  Feb 11, 2015 : Initial
 */

#include "../device.h"
#include "../mc6809.h"

class KANJIROM: public DEVICE {
private:
	uint8 data_table[0x20000];
	bool read_ok;
	bool class2;
	pair kanjiaddr;
 public:
	KANJIROM(VM *parent_vm, EMU* parent_emu, bool type_2std);
	~KANJIROM();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void release();
	void reset(void);
	bool get_readstat(void);
	
	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);
};
