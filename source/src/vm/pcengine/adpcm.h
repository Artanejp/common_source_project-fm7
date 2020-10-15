/*
	NEC-HE PC Engine Emulator 'ePCEngine'
	SHARP X1twin Emulator 'eX1twin'

	Origin : Ootake (joypad/cdrom)
	       : xpce (psg)
	       : MESS (vdc/vce/vpc/cdrom)
	Author : Takeda.Toshiya
	Date   : 2009.03.11-
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.02.09-  Split from pce.cpp

	[ PC-Engine around ADPCM]
*/

#ifndef _PCE_ADPCM_H_
#define _PCE_ADPCM_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_ADPCM_RESET            0
#define SIG_ADPCM_DATA             1
#define SIG_ADPCM_DMACTRL          2
#define SIG_ADPCM_DMA_ENABLED      3
#define SIG_ADPCM_WRITE_DMA_DATA   4
#define SIG_ADPCM_DO_DMA_TRANSFER  5
#define SIG_ADPCM_PLAY_IN_PROGRESS 6
#define SIG_ADPCM_VCLK             7
#define SIG_ADPCM_STATUS_REG       8
#define SIG_ADPCM_COMMAND          9
#define SIG_ADPCM_PAUSE            10
#define SIG_ADPCM_FADE_IN          11
#define SIG_ADPCM_FADE_OUT         12
#define SIG_ADPCM_ADDR_HI          13
#define SIG_ADPCM_ADDR_LO          14
#define SIG_ADPCM_SET_DIVIDER      15
#define SIG_ADPCM_CMD_REG          16
#define SIG_ADPCM_CLEAR_ACK        17
#define SIG_ADPCM_FORCE_DMA_TRANSFER 18
#define SIG_ADPCM_DMA_RELEASED     19

class MSM5205;

namespace PCEDEV {

class PCE;
	
class ADPCM : public DEVICE
{
protected:
	PCE* d_pce;
	MSM5205* d_msm;

	uint32_t read_ptr;
	uint32_t write_ptr;
	uint32_t read_buf;
	uint32_t write_buf;

	uint8_t msm_data;
	uint32_t msm_ptr;
	uint32_t msm_nibble;
	uint32_t msm_length;
	uint32_t half_addr;
	uint32_t adpcm_length;
	bool adpcm_stream;
	int written_size;

	bool dma_enabled;
	bool dma_connected;
	bool adpcm_paused;
	bool adpcm_repeat;
	
	bool play_in_progress;

	double adpcm_volume;
	
	int adpcm_clock_divider;
	int event_fader;
	int event_ack;

	pair16_t addr_reg;
	uint8_t reg_0b;
	uint8_t reg_0c;
	uint8_t msm_last_cmd; // REG $0D

	uint8_t ram[0x10000];
	
	void __FASTCALL do_vclk(bool flag);
	void msm_init();
	bool __FASTCALL do_dma(uint8_t data);
	void do_cmd(uint8_t cmd);
	void do_play();
	void __FASTCALL do_pause(bool pause);
	void __FASTCALL do_stop(bool do_notify);
	void update_length();
	void __FASTCALL set_ack(int clocks);
	void __FASTCALL clear_ack(int clocks);
	void set_dma_status(bool flag);
	void fade_in(int usec);
	void fade_out(int usec);
	void reset_adpcm();
public:
	ADPCM(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("PC-Engine CD-ROM^2 around ADPCM"));
	}
	~ADPCM() { }
	
	void initialize();
	void reset();
	
	uint32_t __FASTCALL read_signal(int ch);
	void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);
	
	void __FASTCALL event_callback(int id, int err);
	void __FASTCALL mix(int32_t* buffer, int cnt);
	bool process_state(FILEIO* state_fio, bool loading);
	
	void set_context_msm(MSM5205* dev)
	{
		d_msm = dev;
	}

	void set_context_pce(PCE* dev)
	{
		d_pce = dev;
	}

};

}
#endif
