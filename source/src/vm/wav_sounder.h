/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.22 -

	[ WAV SOUNDER ]
*/

#ifndef _WAV_SOUNDER_H_
#define _WAV_SOUNDER_H_

#include "device.h"

#define SIG_WAV_SOUNDER_ADD    0
#define SIG_WAV_SOUNDER_PLAY   1
#define SIG_WAV_SOUNDER_MUTE   2
#define SIG_WAV_SOUNDER_CLEAR  3

// SIG_WAV_SOUNDER_ADD  : Add a sounde to mix.
// SIG_WAV_SOUNDER_PLAY : Play/Pause to mix. Pointers of mixing was kept.
// SIG_WAV_SOUNDER_MUTE : Mute / Sound to mix.
// SIG_WAV_SOUNDER_CLEAR : Clear sound table.

class EMU;
class FIFO;

class WAV_SOUNDER : public DEVICE {
protected:
	EMU *p_emu;
	int dst_size;
	int volume_r, volume_l;
	bool play_flag, mute_flag;
	char data_path[_MAX_PATH];
	
	FIFO *queue;
	int16_t *_data;
public:
	WAV_SOUNDER(VM *parent_vm, EMU *parent_emu);
	~WAV_SOUNDER();
	
	void write_signal(int id, uint32_t data, uint32_t mask);
	void mix(int32_t *buffer, int cnt);
	
	const _TCHAR *get_file_name(void);
	
	void set_volume(int ch, int decibel_l, int decibel_r);
	bool load_data(const _TCHAR *path);
	void release();
	void save_state(void *f);
	bool load_state(void *f);

};
#endif
