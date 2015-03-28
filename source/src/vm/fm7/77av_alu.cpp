/*
 * FM77AV/FM16β ALU [77av_alu.cpp]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Mar 28, 2015 : Initial
 *
 */

#include "77av_alu.h"


void FMALU::do_pset(uint32 addr)
{
	uint32 raddr = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? addr & 0x7fff : addr & 0x3fff;
	uint8  planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	uint8  rdata[4];

	if(planes >= 4) planes = 4;
	read_common(raddr, rdata, planes);
