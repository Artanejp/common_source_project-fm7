//
// É PD7752 flavour voice engine
//
// Copyright (c) 2004 cisc.
// All rights reserved.
//
// This	software is	provided 'as-is', without any express or implied
// warranty.  In no	event will the authors be held liable for any damages
// arising from	the	use	of this	software.
//
// Permission is granted to	anyone to use this software	for	any	purpose,
// including commercial	applications, and to alter it and redistribute it
// freely, subject to the following	restrictions:
//
// 1. The origin of	this software must not be misrepresented; you must not
//  claim	that you wrote the original	software. If you use this software
//  in a product,	an acknowledgment in the product documentation would be
//  appreciated but is not required.
// 2. Altered source versions must be plainly marked as	such, and must not be
//  misrepresented as being the original software.
// 3. This notice may not be removed or	altered	from any source	distribution.
//
// translated into C for Cocoa iP6 by Koichi Nishida 2006
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "upd7752.h"

// internal	macros
#define	I2F(a) (((D7752_FIXED) a) << 16)
#define	F2I(a) ((int)((a) >> 16))

// filter coefficients
typedef	struct {
	D7752_FIXED	f[5];
	D7752_FIXED	b[5];
	D7752_FIXED	amp;
	D7752_FIXED	pitch;
} D7752Coef;

// voice
static D7752Coef Coef;
static int	Y[5][2];
static int	PitchCount;
static int	FrameSize;

// amplitude table
static const int amp_table[16] = {0, 1, 1, 2, 3, 4, 5, 7, 9, 13, 17, 23, 31, 42, 56, 75};

// filter coefficiens (uPD7752 flavour)
static const int iir1[128] = {
	 11424,	11400, 11377, 11331, 11285,	11265, 11195, 11149,
	 11082,	11014, 10922, 10830, 10741,	10629, 10491, 10332,
	 10172,	 9992,	9788,  9560,  9311,	 9037,	8721,  8377,
	  8016,	 7631,	7199,  6720,  6245,	 5721,	5197,  4654,
	-11245,-11200,-11131,-11064,-10995,-10905,-10813,-10700,
	-10585,-10447,-10291,-10108, -9924,	-9722, -9470, -9223,
	 -8928,	-8609, -8247, -7881, -7472,	-7019, -6566, -6068,
	 -5545,	-4999, -4452, -3902, -3363,	-2844, -2316, -1864,
	 11585,	11561, 11561, 11561, 11561,	11540, 11540, 11540,
	 11515,	11515, 11494, 11470, 11470,	11448, 11424, 11400,
	 11377,	11356, 11307, 11285, 11241,	11195, 11149, 11082,
	 11014,	10943, 10874, 10784, 10695,	10583, 10468, 10332,
	 10193,	10013,	9833,  9628,  9399,	 9155,	8876,  8584,
	  8218,	 7857,	7445,  6970,  6472,	 5925,	5314,  4654,
	  3948,	 3178,	2312,  1429,   450,	 -543, -1614, -2729,
	 -3883,	-5066, -6250, -7404, -8500,	-9497,-10359,-11038
};

static const int iir2[64]	= {
	 8192, 8105, 7989, 7844, 7670, 7424, 7158, 6803,
	 6370, 5860, 5252, 4579, 3824, 3057, 2307, 1623,
	 8193, 8100, 7990, 7844, 7672, 7423, 7158, 6802,
	 6371, 5857, 5253, 4576, 3825, 3057, 2309, 1617,
	-6739,-6476,-6141,-5738,-5225,-4604,-3872,-2975,
	-1930, -706,  686, 2224, 3871, 5518, 6992, 8085,
	-6746,-6481,-6140,-5738,-5228,-4602,-3873,-2972,
	-1931, -705,  685, 2228, 3870, 5516, 6993, 8084
};

// start voice synthesis
// parameter: mode
//    b2 F:
//       F=0 10ms/frame
//       F=1 20ms/frame
//    b1-0 S:
//       S=00: NORMAL SPEED
//       S=01: SLOW SPEED
//       S=10: FAST SPEED
// return: error code
int	UPD7752_Start(int mode)
{
	const static int frame_size[8] = {
		100,		 //	10ms, NORMAL
		120,		 //	10ms, SLOW
		 80,		 //	10ms, FAST
		100,		 //	PROHIBITED
		200,		 //	20ms, NORMAL
		240,		 //	20ms, SLOW
		160,		 //	20ms, FAST
		200,		 //	PROHIBITED
	};
	
	// initial filter parameter
	const static int f_default[5] =	{126, 64, 121, 111, 96};
	const static int b_default[5] =	{9,	4, 9, 9, 11};
	int i;
	
	// initialise parameter variable
	FrameSize =	frame_size[mode	& 7];
	
	for(i=0; i<5; i++){
		Y[i][0]	  =	0;
		Y[i][1]	  =	0;
		Coef.f[i] =	I2F(f_default[i]);
		Coef.b[i] =	I2F(b_default[i]);
	}
	PitchCount = 0;
	Coef.amp   = 0;
	Coef.pitch = I2F(30);
	
	return D7752_ERR_SUCCESS;
}

//	get length of voice synthesis for 1 frame
int	GetFrameSize(void)
{
	return FrameSize;
}

// synthesise voice for one frame
// frame: pointer for data store
// return: error code
int	Synth(byte *param, D7752_SAMPLE *frame)
{
	int	vu;
	int	qmag;
	int i, j;
	D7752Coef *curr;
	D7752Coef incr,	next;
	int p;
	
	if (!param || !frame) return D7752_ERR_PARAM;
	curr = &Coef;
	
	// expand parameters to coefficients
	qmag = (param[0] & 4) != 0 ? 1 : 0;
	
	for (i=0; i<5; i++) {
		int b;
		int	f =	(param[i+1]	>> 3) &	31;
		if(f & 16) f -= 32;
		next.f[i] =	curr->f[i] + I2F( f	<< qmag	);
		
		b =	param[i+1] & 7;
		if(b & 4)	b -= 8;
		next.b[i] =	curr->b[i] + I2F( b	<< qmag	);
	}
	
	next.amp = I2F(( param[6] >> 4) & 15);
	
	p =	param[6] & 7;
	if (p &	4)	p -= 8;
	next.pitch = curr->pitch + I2F(p);
	
	// calculate increase for linier correction
	incr.amp   = ( next.amp	  -	curr->amp )	  /	FrameSize;
	incr.pitch = ( next.pitch -	curr->pitch	) /	FrameSize;
	for (i=0; i<5; i++) {
		incr.b[i] =	( next.b[i]	- curr->b[i] ) / FrameSize;
		incr.f[i] =	( next.f[i]	- curr->f[i] ) / FrameSize;
	}
	
	// check if there is impulse noise
	vu	= param[0] & 1 ? 1 : 2;
	vu |= param[6] & 4 ? 3 : 0;
	
	// synthesise
	for (i=0; i<FrameSize; i++) {
		int	y =	0;
		
		// generate impulse
		int	c =	F2I(curr->pitch);
		if (PitchCount > (c	> 0	? c	: 128)) {
			if(vu & 1) y = amp_table[F2I(curr->amp)] * 16 - 1;
			PitchCount = 0;
		}
		PitchCount++;

		// generate noise
		if (vu & 2)
			if(rand() & 1) y += amp_table[F2I(curr->amp)] * 4 - 1;	 //	XXX	ÉmÉCÉYè⁄ç◊ïsñæ

		// mysterious filter
		for (j=0; j<5; j++) {
			int	t;
			t  = Y[j][0] * iir1[ F2I( curr->f[j] ) & 0x7f		  ]	/ 8192;
			y += t		 * iir1[(F2I( curr->b[j] ) * 2 + 1)	& 0x7f]	/ 8192;
			y -= Y[j][1] * iir2[ F2I( curr->b[j] ) & 0x3f		  ]	/ 8192;
			y =	y >	8191 ? 8191	: y	< -8192	? -8192	: y;
			
			Y[j][1]	= Y[j][0];
			Y[j][0]	= y;
		}

		// store data
		*frame++ = y;
		// increase parameter
		curr->amp	+= incr.amp;
		curr->pitch	+= incr.pitch;
		for (j=0; j<5; j++) {
			curr->b[j] += incr.b[j];
			curr->f[j] += incr.f[j];
		}
	}
	
	// shift parameter
	memcpy(curr, &next, sizeof(D7752Coef));
	
	return D7752_ERR_SUCCESS;
}

//
// voice routine by Koichi Nishida 2006
// based on PC6001V voice routine by Mr.Yumitaro
//

/*
	Skelton for retropc emulator

	Author : Takeo.Namiki
	Date   : 2013.12.08-

	[ uPD7752 ]
*/


// write to CoreAudio after converting sampling rate and bit width
void UPD7752::UpConvert(void)
{
	int i;
	// 10kHz -> actual sampling rate
	samples = GetFrameSize() * SOUND_RATE / 10000;
	if (!voicebuf) {
		voicebuf=(unsigned char *)malloc(samples * 10000);
		memset(voicebuf, 0, samples * 10000);
	}
	for(i=0; i<samples; i++) {
		int dat = (int)((double)Fbuf[i*GetFrameSize()/samples]/10.0);
		if (dat > 127) dat = 127;
		if (dat < -128) dat = -128;
		voicebuf[fin++] = dat+128;
	}
	mute = false;
}

// abort voice
void UPD7752::AbortVoice(void)
{
	// stop thread loop
	ThreadLoopStop = 1;
	// cancel remaining parameters
	Pnum = Fnum = PReady = 0;
	// free the frame buffer
	if (Fbuf) {
		free(Fbuf);
		Fbuf = NULL;
	}
	VStat &= ~D7752E_BSY;
}

// cancel voice
void UPD7752::CancelVoice(void)
{
	if (!mute) {
		AbortVoice();
		mute = true;
	}
}

// become 1 when synthesising voice
int UPD7752::VoiceOn(void)
{
	return (mute ? 1:0);
}

// set mode
void UPD7752::VSetMode(byte mode)
{
	// start synthesising
	UPD7752_Start(mode);
	// clear status
	VStat = D7752E_IDL;
}

// set command
void UPD7752::VSetCommand(byte comm)
{
	// if synthesising voice, abort
	AbortVoice();
	switch (comm) {
	case 0x00:		// internal voice
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
		// not implemented
		break;
	case 0xfe: {	// external voice
		// malloc frame buffer
		Fbuf = (D7752_SAMPLE *)malloc(sizeof(D7752_SAMPLE)*GetFrameSize());
		if(!Fbuf) break;
		// make external mode, start parameter request
		VStat = D7752E_BSY | D7752E_EXT | D7752E_REQ;
		break;
	}
	case 0xff:		// stop
		break;
	default:		// invalid
		VStat = D7752E_ERR;
		break;
	}
}

// transfer voice parameter
void UPD7752::VSetData(byte data)
{
	// accept data only when busy
	if ((VStat & D7752E_BSY)&&(VStat & D7752E_REQ)) {
		if (Fnum == 0 || Pnum) {	// first frame?
			// if first frame, set repeat number
			if(Pnum == 0) {
				Fnum = data>>3;
				if (Fnum == 0) { PReady = 1; ThreadLoopStop = 0;}
			}
			// store to parameter buffer
			ParaBuf[Pnum++] = data;
			// if parameter buffer is full, ready!
			if(Pnum == 7) {
				VStat &= ~D7752E_REQ;
				Pnum = 0;
				if(Fnum > 0) Fnum--;
				ThreadLoopStop = 0;
				PReady = 1;
			}
		} else {						// repeat frame?
			int i;
			for(i=1; i<6; i++) ParaBuf[i] = 0;
			ParaBuf[6] = data;
			VStat &= ~D7752E_REQ;
			Pnum = 0;
			Fnum--;
			ThreadLoopStop = 0;			
			PReady = 1;
		}
	}
}

// get status register
int UPD7752::VGetStatus(void)
{
	int ret;
	ret = VStat;
	return ret;
}

void UPD7752::initialize()
{
	mute=true;
	voicebuf=NULL;
	Fbuf = NULL;
	// register event to update the key status
	register_frame_event(this);
	return;
}

void UPD7752::release()
{
	CancelVoice();
	if (voicebuf) free(voicebuf);
	// trash FreeVoice();
	if (Fbuf) free(Fbuf);
	return;
}

void UPD7752::reset()
{
	io_E0H = io_E2H = io_E3H = 0;
	Pnum    = 0;
	Fnum    = 0;
	PReady 	= 0;
	VStat   = D7752E_IDL;
	fin = fout =0;
	mute=true;
	ThreadLoopStop=1;
	return;
}

void UPD7752::write_io8(uint32 addr, uint32 data)
{
	// disk I/O
	uint16 port=(addr & 0x00ff);
	byte Value=(data & 0xff);

	switch(port)
	{
	case 0xE0:
		VSetData(data);
		break;
	case 0xE2:
		VSetMode(data);
		break;
	case 0xE3:
		VSetCommand(data);
		break;
	}
	return;
}

uint32 UPD7752::read_io8(uint32 addr)
{
	uint16 port=(addr & 0x00ff);
	byte Value=0xff;

	switch(port)
	{
	case 0xE0:
		Value=VGetStatus();
		break;
	case 0xE2:
		Value=io_E2H;
		break;
	case 0xE3:
		Value=io_E3H;
		break;
	}
	return Value;
}

// voice thread
void UPD7752::event_frame()
{
	if (ThreadLoopStop) return;
	if (VStat & D7752E_EXT) {	// external voice
		if (PReady) {	// parameter set is complete ?
			// abort if frame number is 0
			if(!(ParaBuf[0]>>3)) {
				AbortVoice();
				mute = true;
			} else {
				// synthesise 1 frame samples
				Synth(ParaBuf, Fbuf);
				// convert sampling rate and write to CoreAudio
				UpConvert();
				// accept next frame parameter
				PReady = 0;
				ThreadLoopStop = 1;
				VStat |= D7752E_REQ;
			}
		} else {
			AbortVoice();		// abort voice
			VStat = D7752E_ERR;
		}
	}
}

void UPD7752::mix(int32* buffer, int cnt)
{
	if (mute && fout == fin) {
		fin = fout =0;
		return;
	}
	for(int i = 0; i < cnt; i++) {
		int32 vol = 0;
		if (fout < fin) {
			vol=voicebuf[fout];
			voicebuf[fout]=0;
			fout++;
		}
		*buffer++ = vol << 4;
	}
}
