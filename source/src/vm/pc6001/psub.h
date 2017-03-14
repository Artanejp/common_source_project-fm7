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
	bool play, rec, is_wav, is_p6t;
	_TCHAR rec_file_path[_MAX_PATH];
	int CasIntFlag;
	int CasIndex;
	int CasRecv;
	int CasMode;
	int CasBaud, FileBaud;
	uint8_t CasData[0x10000];
	int CasLength;
	int CasSkipFlag;
	
//	const uint8_t* key_stat;
	uint8_t key_stat[256];
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
	PSUB(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Pseudo Sub System"));
	}
	~PSUB() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_frame();
	void event_callback(int event_id, int err);
	uint32_t get_intr_ack();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_context_timer(DEVICE* device)
	{
		d_timer = device;
	}
	bool play_tape(const _TCHAR* file_path);
	bool rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted()
	{
		return (play || rec);
	}
	void key_down(int code);
	void key_up(int code);
};
#endif
