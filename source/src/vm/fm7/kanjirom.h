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
	const _TCHAR *get_device_name()
	{
		if(class2) {
			return _T("FM7_KANJI_CLASS2");
		} else {
			return _T("FM7_KANJI_CLASS1");
		}
	}
	
	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);
};
