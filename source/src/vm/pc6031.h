//
// PC-6001/6601 disk I/O
// This file is based on a disk I/O program in C++
// by Mr. Yumitaro and translated into C for Cocoa iP6
// by Koichi NISHIDA 2006
//

/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Origin : tanam
	Date   : 2014.05.21-

	[ PC-6031 ]
*/

#ifndef _PC6031_H_
#define _PC6031_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

//#if defined(USE_SOUND_FILES)
#define PC6031_SND_TBL_MAX 256
#ifndef SIG_SOUNDER_MUTE
#define SIG_SOUNDER_MUTE    	(65536 + 0)
#endif
#ifndef SIG_SOUNDER_RELOAD
#define SIG_SOUNDER_RELOAD    	(65536 + 32)
#endif
#ifndef SIG_SOUNDER_ADD
#define SIG_SOUNDER_ADD     	(65536 + 64)
#endif

#define PC6031_SND_TYPE_SEEK 0
#define PC6031_SND_TYPE_HEAD 1
//#endif
class DISK;
class VM;
class EMU;
class PC6031 : public DEVICE
{
private:
	DISK* disk[2];
//#if defined(USE_SOUND_FILES)
	int seek_event_id[2];
	int seek_track_num[2];
//#endif
	int cur_trk[2];
	int cur_sct[2];
	int cur_pos[2];
	bool access[2];
	
	int Seek88(int drvno, int trackno, int sectno);
	unsigned char Getc88(int drvno);
	int Putc88(int drvno, unsigned char dat);
	
	typedef struct {
		int ATN;				// attention
		int DAC;				// data accepted
		int RFD;				// ready for data
		int DAV;				// data valid
		int command;			// received command
		int step;				// status for waiting parameter
		int blk;				// block number
		int drv;				// drive number - 1
		int trk;				// track number
		int sct;				// sector number
		int size;				// byte number to process
		unsigned char retdat;	// return from port D0H
	} DISK60;
	
	DISK60 mdisk;
	unsigned char io_D1H;
	unsigned char io_D2H, old_D2H;
	unsigned char io_D3H;
	int DrvNum;
	
	unsigned char FddIn60();
	void FddOut60(unsigned char dat);
	unsigned char FddCntIn60(void);
	void FddCntOut60(unsigned char dat);
	
	void OutD1H_60(unsigned char data);
	void OutD2H_60(unsigned char data);
	void OutD3H_60(unsigned char data);
	
	unsigned char InD0H_60();
	unsigned char InD1H_60();
	unsigned char InD2H_60();
	unsigned char InD3H_60();
	
//#if defined(USE_SOUND_FILES)
protected:
	_TCHAR snd_seek_name[512];
	_TCHAR snd_head_name[512];
	int snd_seek_mix_tbl[PC6031_SND_TBL_MAX];
	int snd_head_mix_tbl[PC6031_SND_TBL_MAX];
	int16_t *snd_seek_data; // Read only
	int16_t *snd_head_data; // Read only
	int snd_seek_samples_size;
	int snd_head_samples_size;
	bool snd_mute;
	int snd_level_l, snd_level_r;
	virtual void mix_main(int32_t *dst, int count, int16_t *src, int *table, int samples);
	void add_sound(int type);
//#endif
public:
	PC6031(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("PSEUDO PC-6031 FDD"));
//#if defined(USE_SOUND_FILES)
		seek_event_id[0] = seek_event_id[1] = -1;
		seek_track_num[0] = seek_track_num[1] = 0;
		for(int i = 0; i < PC6031_SND_TBL_MAX; i++) {
			snd_seek_mix_tbl[i] = -1;
			snd_head_mix_tbl[i] = -1;
		}
		snd_seek_data = snd_head_data = NULL;
		snd_seek_samples_size = snd_head_samples_size = 0;
		snd_mute = false;
		snd_level_l = snd_level_r = decibel_to_volume(0);
		memset(snd_seek_name, 0x00, sizeof(snd_seek_name));
		memset(snd_head_name, 0x00, sizeof(snd_head_name));
//#endif
	}
	~PC6031() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	uint32_t read_signal(int ch);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	void event_callback(int event_id, int err);
	
	// unique functions
	DISK* get_disk_handler(int drv)
	{
		return disk[drv];
	}
//#if defined(USE_SOUND_FILES)
	// Around SOUND. 20161004 K.O
	bool load_sound_data(int type, const _TCHAR *pathname);
	void release_sound_data(int type);
	bool reload_sound_data(int type);
	
	void mix(int32_t *buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
//#endif
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool is_disk_inserted(int drv);
	bool disk_ejected(int drv);
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);
};

#endif

