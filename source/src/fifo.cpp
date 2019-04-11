/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2014.12.19-

	[ fifo buffer ]
*/

#include <stdlib.h>
#include <malloc.h> 
#include "fifo.h"
#include "fileio.h"

FIFO::FIFO(int s, int empty_warn, int fill_warn)
{
	size = s;
	//buf = (int*)malloc(size * sizeof(int));
	buf = new int[size];
	
	cnt = rpt = wpt = 0;
	empty_warn_val = empty_warn;
	if(fill_warn <= 0) {
		fill_warn_val = INT_MAX;
	} else {
		fill_warn_val = fill_warn;
	}
}

void FIFO::release()
{
	//free(buf);
	delete[] buf;
}

void FIFO::clear()
{
	cnt = rpt = wpt = 0;
	fill_warn_flag = false;
	if(empty_warn_val > 0) empty_warn_flag = false;
}

void FIFO::write(int val, bool *p_fill_warn)
{
	if(cnt < size) {
		buf[wpt++] = val;
		if(wpt >= size) {
			wpt = 0;
		}
		cnt++;
		if(fill_warn_val < cnt) {
			fill_warn_flag = true;
		} else {
			fill_warn_flag = false;
		}
		if(p_fill_warn != nullptr) {
			*p_fill_warn = fill_warn_flag;
		}
	}
}

int FIFO::read(bool *p_empty_warn)
{
	int val = 0;
	if(cnt) {
		val = buf[rpt++];
		if(rpt >= size) {
			rpt = 0;
		}
		cnt--;
		if(empty_warn_val > cnt) {
			empty_warn_flag = true;
		} else {
			empty_warn_flag = false;
		}
		if(p_empty_warn != nullptr) {
			*p_empty_warn = empty_warn_flag;
		}
	}
	return val;
}

int FIFO::read_not_remove(int pt, bool *p_empty_warn)
{
	if(pt >= 0 && pt < cnt) {
		pt += rpt;
		if(pt >= size) {
			pt -= size;
		}
		if(empty_warn_val > cnt) {
			empty_warn_flag = true;
		} else {
			empty_warn_flag = false;
		}
		if(p_empty_warn != nullptr) {
			*p_empty_warn = empty_warn_flag;
		}
		return buf[pt];
	}
	if(p_empty_warn != nullptr) {
		*p_empty_warn = false;
	}
	return 0;
}

void FIFO::write_not_push(int pt, int d, bool *p_fill_warn)
{
	if(pt >= 0 && pt < cnt) {
		pt += wpt;
		if(pt >= size) {
			pt -= size;
		}
		buf[pt] = d;
		if(fill_warn_val < cnt) {
			fill_warn_flag = true;
		} else {
			fill_warn_flag = false;
		}
		if(p_fill_warn != nullptr) {
			*p_fill_warn = fill_warn_flag;
		}
		return;
	}
	if(p_fill_warn != nullptr) {
		*p_fill_warn = false;
	}
}

int FIFO::count()
{
	return cnt;
}

bool FIFO::full()
{
	return (cnt == size);
}

bool FIFO::empty()
{
	return (cnt == 0);
}

#define STATE_VERSION	1

bool FIFO::process_state(void *f, bool loading)
{
	FILEIO *state_fio = (FILEIO *)f;
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	state_fio->StateValue(size);
	state_fio->StateArray(buf, size * sizeof(int), 1);
	state_fio->StateValue(cnt);
	state_fio->StateValue(rpt);
	state_fio->StateValue(wpt);
	return true;
}

