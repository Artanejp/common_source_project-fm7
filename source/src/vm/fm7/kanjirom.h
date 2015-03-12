/*
 * Common source code project -> FM-7/77/AV -> Kanji rom
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *  Feb 11, 2015 : Initial
 */


#include "../memory.h"
#include "../mc6809.h"


#undef MEMORY_ADDR_MAX
#undef MEMORY_BANK_SIZE
#define MEMORY_ADDR_MAX  0x20000
#define MEMORY_BANK_SIZE 0x20000

class KANJIROM: public MEMORY {
private:
	uint8 data_table[MEMORY_ADDR_MAX];
	bool read_ok;
	bool class2;
 public:
	KANJIROM(VM *parent_vm, EMU* parent_emu, bool type_2std);
	~KANJIROM();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	bool get_readstat(void);
};
