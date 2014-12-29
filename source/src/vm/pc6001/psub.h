/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.07.15-

	[ pseudo sub cpu ]
*/

#ifndef _PSUB_H_
#define _PSUB_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FILEIO;

class PSUB : public DEVICE
{
private:
	DEVICE *d_pio, *d_timer;
	
	// data recorder
	FILEIO* fio;
	bool play, rec;
	int CasIntFlag;
	int CasIndex;
	int CasMode;
	int CasBaud;
	int CasEventID;
	uint8 CasData[0x10000];
	
	uint8* key_stat;
	int kbFlagFunc;
	int kbFlagGraph;
	int kbFlagCtrl;
	int kanaMode;
	int katakana;
	int p6key;
	int stick0;
	int StrigIntFlag;
	int StrigEventID;
	void update_keyboard();
	
public:
	PSUB(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~PSUB() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_frame();
	void event_callback(int event_id, int err);
	uint32 intr_ack();
	
	// unique functions
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_context_timer(DEVICE* device)
	{
		d_timer = device;
	}
	bool play_tape(_TCHAR* file_path);
	bool rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted()
	{
		return (play || rec);
	}
};
#endif
