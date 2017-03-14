//
// PC-6001/6601 disk I/O
// This file is based on a disk I/O program in C++
// by Mr. Yumitaro and translated into C for Cocoa iP6
// by Koichi NISHIDA 2006
//

/*
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.07.15-

	[ internal floppy drive ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#if defined(USE_SOUND_FILES)
#define FLOPPY_SND_TBL_MAX 256
#ifndef SIG_SOUNDER_MUTE
#define SIG_SOUNDER_MUTE    	(65536 + 0)
#endif
#ifndef SIG_SOUNDER_RELOAD
#define SIG_SOUNDER_RELOAD    	(65536 + 32)
#endif
#ifndef SIG_SOUNDER_ADD
#define SIG_SOUNDER_ADD     	(65536 + 64)
#endif

#define FLOPPY_SND_TYPE_SEEK 0
#define FLOPPY_SND_TYPE_HEAD 1
#endif
class DISK;
class FLOPPY : public DEVICE
{
private:
	DEVICE *d_ext;
	unsigned char io_B1H;
	
	DISK* disk[2];
	int cur_trk[2];
	int cur_sct[2];
	int cur_pos[2];
	bool access[2];
	
	int Seek88(int drvno, int trackno, int sectno);
	unsigned char Getc88(int drvno);
	int Putc88(int drvno, unsigned char dat);
	
	// data buffer (256BytesX4)
	unsigned char Data[4][256];
	int Index[4];
	
	typedef struct {
		unsigned char Data[10];
		int Index;
	} CmdBuffer;
	
	CmdBuffer CmdIn;					// command buffer
	CmdBuffer CmdOut;				// status buffer
	unsigned char SeekST0;			// ST0 when SEEK
	unsigned char LastCylinder;		// last read cylinder
	int SeekEnd;						// complete seek flag
	unsigned char SendSectors;		// amount(100H unit)
	int DIO;							// data direction TRUE: Buffer->CPU FALSE: CPU->Buffer
	unsigned char Status;			// FDC status register
#if defined(USE_SOUND_FILES)
	int seek_track_num[2];
	int seek_event_id[2];
#endif
	void Push(int part, unsigned char data);
	unsigned char Pop(int part);
	void Clear(int i);
	
	int DiskInit66(void);
	void PushStatus(int data);
	unsigned char PopStatus();
	void OutFDC(unsigned char data);
	unsigned char InFDC();
	void Read();
	void Write(void);
	void Seek(void);
	void SenseInterruptStatus(void);
	void Exec();
	
	void OutB1H_66(unsigned char data);
	void OutB2H_66(unsigned char data);
	void OutB3H_66(unsigned char data);
	void OutD0H_66(unsigned char data);
	void OutD1H_66(unsigned char data);
	void OutD2H_66(unsigned char data);
	void OutD3H_66(unsigned char data);
	void OutD6H_66(unsigned char data);
	void OutD8H_66(unsigned char data);
	void OutDAH_66(unsigned char data);
	void OutDDH_66(unsigned char data);
	void OutDEH_66(unsigned char data);
	
	unsigned char InB2H_66();
	unsigned char InD0H_66();
	unsigned char InD1H_66();
	unsigned char InD2H_66();
	unsigned char InD3H_66();
	unsigned char InD4H_66();
	unsigned char InDCH_66();
	unsigned char InDDH_66();
	
#if defined(USE_SOUND_FILES)
protected:
	_TCHAR snd_seek_name[512];
	_TCHAR snd_head_name[512];
	int snd_seek_mix_tbl[FLOPPY_SND_TBL_MAX];
	int snd_head_mix_tbl[FLOPPY_SND_TBL_MAX];
	int16_t *snd_seek_data; // Read only
	int16_t *snd_head_data; // Read only
	int snd_seek_samples_size;
	int snd_head_samples_size;
	bool snd_mute;
	int snd_level_l, snd_level_r;
	virtual void mix_main(int32_t *dst, int count, int16_t *src, int *table, int samples);
	void add_sound(int type);
#endif
public:
	FLOPPY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#if defined(USE_SOUND_FILES)
		seek_track_num[0] = seek_track_num[1] = 0;
		cur_trk[0] = cur_trk[1] = 0;
		seek_event_id[0] = seek_event_id[1] = -1;
		for(int i = 0; i < FLOPPY_SND_TBL_MAX; i++) {
			snd_seek_mix_tbl[i] = -1;
			snd_head_mix_tbl[i] = -1;
		}
		snd_seek_data = snd_head_data = NULL;
		snd_seek_samples_size = snd_head_samples_size = 0;
		snd_mute = false;
		snd_level_l = snd_level_r = decibel_to_volume(0);
		memset(snd_seek_name, 0x00, sizeof(snd_seek_name));
		memset(snd_head_name, 0x00, sizeof(snd_head_name));
#endif
		set_device_name(_T("Floppy Drive"));
	}
	~FLOPPY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_callback(int event_id, int err);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	uint32_t read_signal(int ch);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_ext(DEVICE* device)
	{
		d_ext = device;
	}
	DISK* get_disk_handler(int drv)
	{
		return disk[drv];
	}
#if defined(USE_SOUND_FILES)
	// Around SOUND. 20161004 K.O
	bool load_sound_data(int type, const _TCHAR *pathname);
	void release_sound_data(int type);
	bool reload_sound_data(int type);
	
	void mix(int32_t *buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
#endif
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool is_disk_inserted(int drv);
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);
};

#endif

