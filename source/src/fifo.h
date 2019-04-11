/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.17-

	[ fifo buffer ]
*/

#ifndef _FIFO_H_
#define _FIFO_H_

#include "common.h"

class DLL_PREFIX FIFO
{
protected:
	int size;
	int* buf;
	int cnt, rpt, wpt;
	int empty_warn_val, fill_warn_val;
	bool empty_warn_flag, fill_warn_flag;
public:
	FIFO(int s, int empty_warn = -1, int fill_warn = -1);
	virtual void release();
	virtual void clear();
	virtual void write(int val, bool *p_fill_warn = nullptr);
	virtual int read(bool *p_empty_warn = nullptr);
	virtual int read_not_remove(int pt, bool *p_empty_warn = nullptr);
	virtual void write_not_push(int pt, int d, bool *p_fill_warn = nullptr);
	virtual int count();
	virtual bool full();
	virtual bool empty();
	virtual bool is_fill_warn() { return fill_warn_flag; }
	virtual bool is_fill_warn_or_empty_warn() { return ((fill_warn_flag) || (empty_warn_flag)); }
	virtual bool is_empty_warn() { return empty_warn_flag; }
	virtual void modify_fill_warn_val(int n) { fill_warn_val = n; }
	virtual void modify_empty_warn_val(int n) { empty_warn_val = n; }
	virtual void clear_fill_warn() { fill_warn_flag = false; }
	virtual void clear_empty_warn() { empty_warn_flag = false; }
	virtual bool process_state(void *f, bool loading);
};

#endif

