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
	if(buf != NULL) {
		delete[] buf;
	}
}

void FIFO::clear()
{
	cnt = rpt = wpt = 0;
	fill_warn_flag = false;
	if(empty_warn_val > 0) empty_warn_flag = false;
}

void FIFO::write(int val, bool *p_fill_warn)
{
	__UNLIKELY_IF(buf == NULL) {
		__UNLIKELY_IF(p_fill_warn != NULL) {
			*p_fill_warn = true;
		}
		return;
	}
	__LIKELY_IF(cnt < size) {
		buf[wpt++] = val;
		__UNLIKELY_IF(wpt >= size) {
			wpt = 0;
		}
		cnt++;
		__UNLIKELY_IF(fill_warn_val < cnt) {
			fill_warn_flag = true;
		} else {
			fill_warn_flag = false;
		}
	}
	__UNLIKELY_IF(p_fill_warn != nullptr) {
		*p_fill_warn = fill_warn_flag;
	}
}

int FIFO::read(bool *p_empty_warn)
{
	int val = 0;
	__UNLIKELY_IF(buf == NULL) {
		__UNLIKELY_IF(p_empty_warn != NULL) {
			*p_empty_warn = true;
		}
		return val;
	}
	__LIKELY_IF(cnt > 0) {
		val = buf[rpt++];
		__UNLIKELY_IF(rpt >= size) {
			rpt = 0;
		}
		cnt--;
		__UNLIKELY_IF(cnt <= 0) {
			cnt = 0;
			rpt = 0;
			wpt = 0;
		}
		__UNLIKELY_IF(empty_warn_val > cnt) {
			empty_warn_flag = true;
		} else {
			empty_warn_flag = false;
		}
	}
	__UNLIKELY_IF(p_empty_warn != nullptr) {
		*p_empty_warn = empty_warn_flag;
	}
	return val;
}

int FIFO::read_not_remove(int pt, bool *p_empty_warn)
{
	__UNLIKELY_IF(buf == NULL) {
		__UNLIKELY_IF(p_empty_warn != NULL) {
			*p_empty_warn = true;
		}
		return 0;
	}
	__UNLIKELY_IF(p_empty_warn != nullptr) {
		*p_empty_warn = empty_warn_flag;
	}
	__LIKELY_IF(pt >= 0 && pt < cnt) {
		pt += rpt;
		if(pt >= size) {
			pt -= size;
		}
		return buf[pt];
	}
	return 0;
}

void FIFO::write_not_push(int pt, int d, bool *p_fill_warn)
{
	__UNLIKELY_IF(buf == NULL) {
		__UNLIKELY_IF(p_fill_warn != NULL) {
			*p_fill_warn = true;
		}
		return;
	}
	__UNLIKELY_IF(p_fill_warn != nullptr) {
		*p_fill_warn = fill_warn_flag;
	}
	__LIKELY_IF(pt >= 0 && pt < cnt) {
		pt += wpt;
		if(pt >= size) {
			pt -= size;
		}
		buf[pt] = d;
		return;
	}
}

int FIFO::count()
{
	return cnt;
}

int FIFO::fifo_size()
{
	return size;
}

bool FIFO::full()
{
	return (cnt == size);
}

int FIFO::left()
{
	int val = size - cnt;
	__UNLIKELY_IF(val < 0) val = 0;
	__UNLIKELY_IF(val > size) val = size;
	return val;
}

bool FIFO::empty()
{
	return (cnt <= 0);
}

#define STATE_VERSION	1

bool FIFO::process_state(void *f, bool loading)
{
	FILEIO *state_fio = (FILEIO *)f;

	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	state_fio->StateValue(size);
	if(buf != NULL) {
		state_fio->StateArray(buf, size * sizeof(int), 1);
	}
	state_fio->StateValue(cnt);
	state_fio->StateValue(rpt);
	state_fio->StateValue(wpt);
	return true;
}
