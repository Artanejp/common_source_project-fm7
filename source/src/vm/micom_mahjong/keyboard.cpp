/*
	MICOM MAHJONG Emulator 'eMuCom Mahjong'

	Author : Hiroaki GOTO as GORRY
	Date   : 2020.07.20 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "memory.h"

static const int key_map[][3] = {
	{ 0x31, 2, 0}, // KEY  1     = 1[FULL]
	{ 0x32, 2, 1}, // KEY  2     = 2[FULL]
	{ 0x33, 2, 2}, // KEY  3     = 3[FULL]
	{ 0x34, 2, 3}, // KEY  4     = 4[FULL]
	{ 0x35, 2, 4}, // KEY  5     = 5[FULL]
	{ 0x36, 2, 5}, // KEY  6     = 6[FULL]
	{ 0x37, 2, 6}, // KEY  7     = 7[FULL]
	{ 0x38, 2, 7}, // KEY  8     = 8[FULL]
	{ 0x39, 1, 2}, // KEY  9     = 9[FULL]
	{ 0x30, 1, 3}, // KEY 10     = 0[FULL]
	{ 0xbd, 1, 4}, // KEY 11     = -=[JP][US]
	{ 0x61, 1, 4}, // KEY 11     = 1[10KEY]
	{ 0xde, 1, 5}, // KEY 12     = ^~[JP]
	{ 0xbb, 1, 5}, // KEY 12     = =+[US]
	{ 0x62, 1, 5}, // KEY 12     = 2[10KEY]
	{ 0xdc, 1, 6}, // KEY 13     = \|[JP]
	{ 0x08, 1, 6}, // KEY 13     = Backspace
	{ 0x63, 1, 6}, // KEY 13     = 3[10KEY]
	{ 0x0d, 1, 7}, // KEY ツモ   = Enter
	{ 0x20, 1, 7}, // KEY ツモ   = Space
	{ 0x5a, 0, 2}, // KEY ポン   = Z
	{ 0x58, 0, 3}, // KEY チー   = X
	{ 0x43, 0, 4}, // KEY カン   = C
	{ 0x56, 0, 5}, // KEY リーチ = V
	{ 0x41, 0, 6}, // KEY ロン   = A
	{ 0x70, 0, 2}, // KEY ポン   = [F1]
	{ 0x71, 0, 3}, // KEY チー   = [F2]
	{ 0x72, 0, 4}, // KEY カン   = [F3]
	{ 0x73, 0, 5}, // KEY リーチ = [F4]
	{ 0x74, 0, 6}, // KEY ロン   = [F5]
	{ 0x00, 0, 0}
};

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
	column = 0;
	
	// register event
	register_frame_event(this);
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (data) {
	  default:
		break;
	  case 0xfe:
		column = 2;
		break;
	  case 0xfd:
		column = 1;
		break;
	  case 0xfb:
		column = 0;
		break;
	}
	update_key();
}

void KEYBOARD::event_frame()
{
	update_key();
}

void KEYBOARD::update_key()
{
	uint8_t stat = 0xff;
	int i = 0;
	while (!0) {
		if (key_map[i][0] == 0) break;
		if (key_map[i][1] == column) {
			if (key_stat[key_map[i][0]]) {
				stat &= ~(1 << key_map[i][2]);
			}
		}
		i++;
	}
	d_memory->write_signal(SIG_MEMORY_KEYDATA, stat, 0xff);
}

#define STATE_VERSION	1

bool KEYBOARD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(column);
	return true;
}

