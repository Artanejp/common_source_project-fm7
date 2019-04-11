/*
	Skelton for retropc emulator

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.04.11-

	[ ring buffer ]
*/

#pragma once

#include "fifo.h"

class DLL_PREFIX RINGBUFFER : public FIFO
{
public:
	RINGBUFFER(int s, int empty_warn = -1, int fill_warn = -1);
	virtual void write(int val, bool *p_fill_warn = nullptr);
	virtual int read(bool *p_empty_warn = nullptr);
	virtual int read_not_remove(int pt, bool *p_empty_warn = nullptr);
	virtual void write_not_push(int pt, int d, bool *p_fill_warn = nullptr);
};
