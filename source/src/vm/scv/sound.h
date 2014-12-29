/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ uPD1771C ]
*/

#ifndef _SOUND_H_
#define _SOUND_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SOUND_CLOCK  1522400.0
#define NOISE_CLOCK      760.0
#define SQUARE_CLOCK  174000.0

#define PCM_PERIOD 120000

#define MAX_TONE   24000
#define MAX_NOISE  16000
#define MAX_SQUARE  8000
#define MAX_PCM    20000

#define MAX_PARAM 0x8000

class SOUND : public DEVICE
{
private:
	DEVICE* d_cpu;
	
	// sound gen
	typedef struct {
		int count;
		int diff;
		int period;
		int timbre;
		int volume;
		int output;
		int ptr;
	} channel_t;
	struct channel_t tone;
	struct channel_t noise;
	struct channel_t square1;
	struct channel_t square2;
	struct channel_t square3;
	struct channel_t pcm;
	void clear_channel(channel_t *ch);
	
	int pcm_table[MAX_PARAM * 8];
	uint32 cmd_addr;
	int pcm_len;
	
	int volume_table[32];
	int detune_table[32];
	
	// command buffer
	int param_cnt, param_ptr, register_id;
	uint8 params[MAX_PARAM];
	
	void process_pcm(uint8 data);
	void process_cmd();
	
public:
	SOUND(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SOUND() {}
	
	// common functions
	void reset();
	void write_data8(uint32 addr, uint32 data);
	void write_io8(uint32 addr, uint32 data);
	void event_callback(int event_id, int err);
	void mix(int32* buffer, int cnt);
	
	// unique function
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void init(int rate);
};

#endif

