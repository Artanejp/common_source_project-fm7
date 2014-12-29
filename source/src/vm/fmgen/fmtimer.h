// ---------------------------------------------------------------------------
//	FM sound generator common timer module
//	Copyright (C) cisc 1998, 2000.
// ---------------------------------------------------------------------------
//	$Id: fmtimer.h,v 1.2 2003/04/22 13:12:53 cisc Exp $

#ifndef FM_TIMER_H
#define FM_TIMER_H

#include "types.h"

// ---------------------------------------------------------------------------

namespace FM
{
	class Timer
	{
	public:
		void	Reset();
		bool	Count(int32 clock);
		int32	GetNextEvent();
	
	protected:
		virtual void SetStatus(uint bit) = 0;
		virtual void ResetStatus(uint bit) = 0;

		void	SetTimerPrescaler(int32 p);
		void	SetTimerA(uint addr, uint data);
		void	SetTimerB(uint data);
		void	SetTimerControl(uint data);
		
		void SaveState(void *f);
		bool LoadState(void *f);
		
		uint8	status;
		uint8	regtc;
	
	private:
		virtual void TimerA() {}
		uint8	regta[2];
		
		int32	timera, timera_count;
		int32	timerb, timerb_count;
		int32	prescaler;
	};

// ---------------------------------------------------------------------------
//	èâä˙âª
//
inline void Timer::Reset()
{
	timera_count = 0;
	timerb_count = 0;
}

} // namespace FM

#endif // FM_TIMER_H
