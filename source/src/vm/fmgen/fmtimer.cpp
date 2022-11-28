// ---------------------------------------------------------------------------
//	FM sound generator common timer module
//	Copyright (C) cisc 1998, 2000.
// ---------------------------------------------------------------------------
//	$Id: fmtimer.cpp,v 1.1 2000/09/08 13:45:56 cisc Exp $

#include "headers.h"
#include "fmtimer.h"

#include "../../fileio.h"

using namespace FM;

// ---------------------------------------------------------------------------
//	タイマー制御
//
void Timer::SetTimerControl(uint data)
{
	uint tmp = regtc ^ data;
	regtc = uint8(data);
	
	if (data & 0x10) 
		ResetStatus(1);
	if (data & 0x20) 
		ResetStatus(2);

	if (tmp & 0x01)
		timera_count = (data & 1) ? timera * prescaler : 0;
	if (tmp & 0x02)
		timerb_count = (data & 2) ? timerb * prescaler : 0;
}

// ---------------------------------------------------------------------------
//	タイマーA 周期設定
//
void Timer::SetTimerA(uint addr, uint data)
{
	uint tmp;
	regta[addr & 1] = uint8(data);
	tmp = (regta[0] << 2) + (regta[1] & 3);
	timera = 1024-tmp;
}

// ---------------------------------------------------------------------------
//	タイマーB 周期設定
//
void Timer::SetTimerB(uint data)
{
	timerb = (256-data) << 4;
}

// ---------------------------------------------------------------------------
//	タイマー時間処理
//
bool Timer::Count(int32 clock)
{
	bool event = false;

	if (timera_count)
	{
		timera_count -= clock;
		if (timera_count <= 0)
		{
			event = true;
			TimerA();

			while (timera_count <= 0)
				timera_count += timera * prescaler;
			
			if (regtc & 4)
				SetStatus(1);
		}
	}
	if (timerb_count)
	{
		timerb_count -= clock;
		if (timerb_count <= 0)
		{
			event = true;
			while (timerb_count <= 0)
				timerb_count += timerb * prescaler;
			
			if (regtc & 8)
				SetStatus(2);
		}
	}
	return event;
}

// ---------------------------------------------------------------------------
//	次にタイマーが発生するまでの時間を求める
//
int32 Timer::GetNextEvent()
{
	if(timera_count > 0 && timerb_count > 0) {
		return (timera_count < timerb_count ? timera_count : timerb_count);
	} else if(timera_count > 0) {
		return timera_count;
	} else if (timerb_count > 0) {
		return timerb_count;
	}
	return 0;
}

// ---------------------------------------------------------------------------
//	タイマー基準値設定
//
void Timer::SetTimerPrescaler(int32 p)
{
	prescaler = p;
}

// ---------------------------------------------------------------------------
//	ステートセーブ
//
#define TIMER_STATE_VERSION	1

bool Timer::ProcessState(void *f, bool loading)
{
	FILEIO *state_fio = (FILEIO *)f;
	
	if(!state_fio->StateCheckUint32(TIMER_STATE_VERSION)) {
		return false;
	}
	state_fio->StateValue(status);
	state_fio->StateValue(regtc);
	state_fio->StateArray(regta, sizeof(regta), 1);
	state_fio->StateValue(timera);
	state_fio->StateValue(timera_count);
	state_fio->StateValue(timerb);
	state_fio->StateValue(timerb_count);
	state_fio->StateValue(prescaler);
	return true;
}

