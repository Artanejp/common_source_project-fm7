/*
	Skelton for retropc emulator

	Origin : MAME 0.171 MSM5205 Core
	Author : Takeda.Toshiya
	Date   : 2016.03.07-

	[ MSM5205 ]
*/

// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef _MSM5205_H_
#define _MSM5205_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

/* an interface for the MSM5205 and similar chips */

/* prescaler selector defines   */
/* MSM5205 default master clock is 384KHz */
#define MSM5205_S96_3B 0     /* prescaler 1/96(4KHz) , data 3bit */
#define MSM5205_S48_3B 1     /* prescaler 1/48(8KHz) , data 3bit */
#define MSM5205_S64_3B 2     /* prescaler 1/64(6KHz) , data 3bit */
#define MSM5205_SEX_3B 3     /* VCLK slave mode      , data 3bit */
#define MSM5205_S96_4B 4     /* prescaler 1/96(4KHz) , data 4bit */
#define MSM5205_S48_4B 5     /* prescaler 1/48(8KHz) , data 4bit */
#define MSM5205_S64_4B 6     /* prescaler 1/64(6KHz) , data 4bit */
#define MSM5205_SEX_4B 7     /* VCLK slave mode      , data 4bit */

/* MSM6585 default master clock is 640KHz */
#define MSM6585_S160  (4+8)  /* prescaler 1/160(4KHz), data 4bit */
#define MSM6585_S40   (5+8)  /* prescaler 1/40(16KHz), data 4bit */
#define MSM6585_S80   (6+8)  /* prescaler 1/80 (8KHz), data 4bit */
#define MSM6585_S20   (7+8)  /* prescaler 1/20(32KHz), data 4bit */

class MSM5205 : public DEVICE
{
private:
	int32_t m_mod_clock;          /* clock rate                   */
	int m_timer;
	int32_t m_data;               /* next adpcm data              */
	int32_t m_vclk;               /* vclk signal (external mode)  */
	int32_t m_reset;              /* reset pin signal             */
	int32_t m_prescaler;          /* prescaler selector S1 and S2 */
	int32_t m_bitwidth;           /* bit width selector -3B/4B    */
	int32_t m_signal;             /* current ADPCM signal         */
	int32_t m_step;               /* current ADPCM step           */
	int m_diff_lookup[49*16];
	int m_select;
	outputs_t m_vclk_cb;
	
	void compute_tables();
	
	int volume_m;
	int volume_l, volume_r;
	
public:
	MSM5205(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&m_vclk_cb);
		volume_m = 1024;
		volume_l = volume_r = 1024;
		set_device_name(_T("MSM5205 ADPCM"));
	}
	~MSM5205() {}
	
	// common functions
	void initialize();
	void reset();
	void event_callback(int event_id, int err);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void initialize_sound(int clock, int mode)
	{
		m_mod_clock = clock;
		m_select = mode;
	}
	void set_context_vclk(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&m_vclk_cb, device, id, mask);
	}
	void reset_w(int reset);
	void data_w(int data);
	void vclk_w(int vclk);
	void playmode_w(int select);
	void set_volume(int volume);
	void change_clock_w(int32_t clock);
};

#endif

