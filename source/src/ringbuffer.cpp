/*
	Skelton for retropc emulator

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.04.11-

	[ ring buffer ]
*/


#include "ringbuffer.h"

RINGBUFFER::RINGBUFFER(int s, int empty_warn, int fill_warn) : FIFO(s, empty_warn, fill_warn)
{
	
}

void RINGBUFFER::write(int val, bool *p_fill_warn)
{
	buf[wpt++] = val;
	if(wpt >= size) {
		wpt = 0;
	}
	cnt++;
	if(cnt >= size) cnt = size;
	if(fill_warn_val < cnt) {
		fill_warn_flag = true;
	} else {
		fill_warn_flag = false;
	}
	if(p_fill_warn != nullptr) {
		*p_fill_warn = fill_warn_flag;
	}
}

int RINGBUFFER::read(bool *p_empty_warn)
{
	int val = 0;
	if(cnt > 0) {
		val = buf[rpt++];
		if(rpt >= size) {
			rpt = 0;
		}
		cnt--;
		if(cnt < 0) cnt = 0;
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

int RINGBUFFER::read_not_remove(int pt, bool *p_empty_warn)
{
	if(pt >= 0 && pt < size) {
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

void RINGBUFFER::write_not_push(int pt, int d, bool *p_fill_warn)
{
	if(pt >= 0 && pt < size) {
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

