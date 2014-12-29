/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.17-

	[ fifo buffer ]
*/

#ifndef _FIFO_H_
#define _FIFO_H_

#include "common.h"

class FIFO
{
private:
	int size;
	int* buf;
	int cnt, rpt, wpt;
public:
	FIFO(int s);
	void release();
	void clear();
	void write(int val);
	int read();
	int read_not_remove(int pt);
	int count();
	bool full();
	bool empty();
	void save_state(void *f);
	bool load_state(void *f);
};

#endif

