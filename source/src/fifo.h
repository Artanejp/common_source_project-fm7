/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.17-

	[ fifo buffer ]
*/

#ifndef _FIFO_H_
#define _FIFO_H_

#include "common.h"

class csp_state_data_saver;
class DLL_PREFIX FIFO
{
private:
	int size;
	int* buf;
	int cnt, rpt, wpt;

	const _TCHAR *_ns = "FIFO::BEGIN";
	const _TCHAR *_ne = "FIFO::END";
public:
	FIFO(int s);
	void release();
	void clear();
	void write(int val);
	int read();
	int read_not_remove(int pt);
	void write_not_push(int pt, int d);
	int count();
	bool full();
	bool empty();
	void save_state(void *f);
	bool load_state(void *f);

	void save_state_helper(csp_state_data_saver *state_saver, uint32_t *sumseed, bool *__stat);
	bool load_state_helper(csp_state_data_saver *state_saver, uint32_t *sumseed, bool *__stat);
	
};

#endif

