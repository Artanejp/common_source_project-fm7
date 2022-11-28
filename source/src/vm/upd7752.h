// [OUT]
//  E0H : transfer voice parameter
//  E1H : ?
//  E2H : set mode : 0 0 0 0 F S1 S0
//    F     : frame period      0: 10ms/frame
//                              1: 20ms/frame
//    S1,S0 : synthesis speed  00: NORMAL SPEED
//                             01: SLOW SPEED
//                             10: FAST SPEED
//                             11: prohibit

//  E3H : set command
//    internal voice : 0  0 P5 P4 P3 P2 P1 P0
//			           P5-P0 : select voice kind(0-63)
//    stop           : 1  1  1  1  1  1  1  1
//    external voice : 1  1  1  1  1  1  1  0

// [IN]
//  E0H : status register : BSY REQ ~INT/EXT ERR 0 0 0 0
//    BSY      : synthesizing voice 1:busy 0:stop
//    REQ      : voice parameter 1:input request 0:prohibit
//    ~INT/EXT : message data 1:external 0:internal
//    ERR      : error flag
//  E1H : ?
//  E2H : written PortE2 ?
//  E3H : written PortE3 ?

/*
	Skelton for retropc emulator

	Author : Takeo.Namiki
	Date   : 2013.12.08-

	[ uPD7752 ]
*/

#ifndef _UPD7752_H_
#define _UPD7752_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

typedef	int	D7752_SAMPLE;
typedef	int	D7752_FIXED;

#define	D7752_ERR_SUCCESS		(0)
#define	D7752_ERR_PARAM			(-1)

#define	D7752_ERR_DEVICE_MODE	(-2)
#define	D7752_ERR_MEMORY		(-3)
#define	D7752_ERR_BUFFER_FULL	(-4)
#define	D7752_ERR_BUFFER_EMPTY	(-5)

#define	D7752E_BSY	(0x80)	// b7 BSY -	1 when synthesising voice
#define	D7752E_REQ	(0x40)	// b6 REQ -	1 when there is a space in the parameter buffer
#define	D7752E_EXT	(0x20)	// b5 INT/EXT
#define	D7752E_ERR	(0x10)	// b4 ERR -	1 when error
#define	D7752E_IDL	(0x00)	// waiting

class UPD7752 : public DEVICE
{
private:
	bool mute;
	int ThreadLoopStop;

	// I/O ports
	uint8_t io_E0H;
	uint8_t io_E2H;
	uint8_t io_E3H;

	int VStat;					// status

	// parameter buffer
	uint8_t ParaBuf[7];			// parameter buffer
	uint8_t Pnum;					// parameter number
	int Fnum;					// repeat frame number
	int PReady;					// complete setting parameter

	int FbufLength;
	D7752_SAMPLE *Fbuf;			// frame buffer pointer (10kHz 1frame)
	unsigned char *voicebuf;
	int fin;
	int fout;

	void UpConvert(void);
	void AbortVoice(void);
	void CancelVoice(void);
	int VoiceOn(void);
	void VSetMode(uint8_t mode);
	void VSetCommand(uint8_t comm);
	void VSetData(uint8_t data);
	int VGetStatus(void);
	
	// filter coefficients
	typedef	struct {
		D7752_FIXED	f[5];
		D7752_FIXED	b[5];
		D7752_FIXED	amp;
		D7752_FIXED	pitch;
	} D7752Coef;
	
	// voice
	D7752Coef Coef;
	int Y[5][2];
	int PitchCount;
	int FrameSize;
	int SOUND_RATE;
	
	int UPD7752_Start(int mode);
	int GetFrameSize(void);
	int Synth(uint8_t *param, D7752_SAMPLE *frame);
	
	int volume_l, volume_r;
	
public:
	UPD7752(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
		set_device_name(_T("uPD7752 Voice Synthesizer"));
	}
	~UPD7752() {}
	
	// common functions
	void initialize();
	void event_frame();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void initialize_sound(int rate)
	{
		SOUND_RATE = rate;
	}
};

#endif
