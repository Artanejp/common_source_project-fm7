/*
	Skelton for retropc emulator

	Origin : MAME V30 core
	Author : Takeda.Toshiya
	Date   : 2020.02.02-

	[ V30 disassembler ]
*/

#ifndef _V30_DASM_H_
#define _V30_DASM_H_

#include "../common.h"

class DEBUGGER;

int v30_dasm(DEBUGGER *debugger, uint8_t *oprom, uint32_t eip, bool emulation_mode, _TCHAR *buffer, size_t buffer_len);

#endif
