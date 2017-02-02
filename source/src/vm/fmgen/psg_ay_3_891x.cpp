// ---------------------------------------------------------------------------
//	PSG Sound Implementation
//	Copyright (C) cisc 1997, 1999.
// ---------------------------------------------------------------------------
//	$Id: psg.cpp,v 1.10 2002/05/15 21:38:01 cisc Exp $

#include "headers.h"
#include "misc.h"
#include "./psg_ay_3_891x.h"
// for AY-3-8190/8192
//#include "../vm.h"

#include "../../fileio.h"

PSG_AY_3_891X::PSG_AY_3_891X() : PSG()
{
}

PSG_AY_3_891X::~PSG_AY_3_891X()
{
}

bool PSG_AY_3_891X::Init(uint c, uint r)
{
	//clock = c;
	//psgrate = r;
	SetClock(c, r);
	return true;
}

void PSG_AY_3_891X::SetVolume(int volume_l, int volume_r)
{
	double base_l = 0x4000 / 3.0 * pow(10.0, volume_l / 40.0);
	double base_r = 0x4000 / 3.0 * pow(10.0, volume_r / 40.0);
//#if defined(HAS_AY_3_8910) || defined(HAS_AY_3_8912)
	// AY-3-8190/8192 (PSG): 16step
	for (int i=31; i>=3; i-=2)
	{
		EmitTableL[i] = EmitTableL[i-1] = int(base_l);
		EmitTableR[i] = EmitTableR[i-1] = int(base_r);
		base_l /= 1.189207115;
		base_l /= 1.189207115;
		base_r /= 1.189207115;
		base_r /= 1.189207115;
	}
//#endif
	EmitTableL[1] = 0;
	EmitTableL[0] = 0;
	EmitTableR[1] = 0;
	EmitTableR[0] = 0;
	MakeEnvelopTable();

	SetChannelMask(~mask);
}
