/*
	Systems Formulate BUBCOM80 Emulator 'eBUBCOM80'

	Author : Takeda.Toshiya
	Date   : 2018.05.08-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_DMAC_CH0	0
#define SIG_DISPLAY_DMAC_CH1	1
#define SIG_DISPLAY_DMAC_CH2	2
#define SIG_DISPLAY_DMAC_CH3	3

class Z80;

namespace BUBCOM80 {
typedef struct crtc_t {
	struct {
		int rate, counter;
		uint8_t cursor, attrib;
	} blink;
	struct {
		int type, mode;
		int x, y;
	} cursor;
	struct {
		uint8_t data;
		int num;
		uint8_t expand[200][80];
	} attrib;
	struct {
		uint8_t expand[200][80];
	} text;
	int width, height;
	int char_height;
	bool skip_line;
	int vretrace;
	bool timing_changed;
	uint8_t buffer[120 * 200];
	int buffer_ptr;
	uint8_t cmd;
	int cmd_ptr;
	uint8_t mode, reverse, intr_mask, status;
	bool vblank;
	
	void reset();
	void __FASTCALL write_cmd(uint8_t data);
	void __FASTCALL write_param(uint8_t data);
	uint32_t __FASTCALL read_param();
	uint32_t __FASTCALL read_status();
	void start();
	void finish();
	void write_buffer(uint8_t data);
	uint8_t read_buffer(int ofs);
	void update_blink();
	void expand_buffer();
	void set_attrib(uint8_t code);
} crtc_t;

typedef struct dmac_t {
	struct {
		pair32_t addr, count;
		uint8_t mode;
		int nbytes;
		DEVICE *io;
		bool running;
	} ch[4];
	uint8_t mode, status;
	bool high_low;
	DEVICE *mem;
	
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void start(int c);
	void __FASTCALL run(int c);
	void finish(int c);
} dmac_t;

class DISPLAY : public DEVICE
{
private:
	Z80 *d_cpu;
	DEVICE *d_cmt, *d_pcm, *d_prn;
	
	uint8_t sg_pattern[0x800];
	uint8_t font[0x800];
	uint8_t vram[0x10000];
	
	int busreq_clocks;
	bool color;
	bool width40;
	uint8_t mode;
	
	crtc_t crtc;
	dmac_t dmac;
	
	scrntype_t palette_text_pc[8];
	scrntype_t palette_graph_pc[8];
	uint8_t text[200][640];
	uint8_t graph[200][640];
	
	void update_timing_local();
	void draw_text();
	void draw_graph();
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 4; i++) {
			dmac.ch[i].io = parent_vm->dummy;
		}
		dmac.mem = parent_vm->dummy;
		
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize() override;
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data) override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	void __FASTCALL event_callback(int event_id, int err) override;
	void event_frame() override;
	void event_vline(int v, int clock) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique functions
	void set_context_cpu(Z80* device)
	{
		d_cpu = device;
	}
	void set_context_cmt(DEVICE* device)
	{
		d_cmt = device;
	}
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void set_context_prn(DEVICE* device)
	{
		d_prn = device;
	}
	void set_context_dmac_mem(DEVICE* device)
	{
		dmac.mem = device;
	}
	void set_context_dmac_ch0(DEVICE* device)
	{
		dmac.ch[0].io = device;
	}
	void set_context_dmac_ch2(DEVICE* device)
	{
		dmac.ch[2].io = device;
	}
	void draw_screen();
};
}

#endif
