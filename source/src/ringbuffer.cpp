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
	__UNLIKELY_IF(wpt >= size) {
		wpt = 0;
	}
	cnt++;
	__UNLIKELY_IF(cnt >= size) {
		cnt = size;
	}
	__UNLIKELY_IF(fill_warn_val < cnt) {
		fill_warn_flag = true;
	} else {
		fill_warn_flag = false;
	}
	__UNLIKELY_IF(p_fill_warn != nullptr) {
		*p_fill_warn = fill_warn_flag;
	}
}

int RINGBUFFER::read(bool *p_empty_warn)
{
	int val = 0;
	__LIKELY_IF(cnt > 0) {
		val = buf[rpt++];
		__UNLIKELY_IF(rpt >= size) {
			rpt = 0;
		}
		cnt--;
		__UNLIKELY_IF(cnt <= 0) {
			cnt = 0;
			wpt = 0;
			rpt = 0;
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

int RINGBUFFER::read_not_remove(int pt, bool *p_empty_warn)
{
	__UNLIKELY_IF(p_empty_warn != nullptr) {
		*p_empty_warn = empty_warn_flag;
	}
	__LIKELY_IF(pt >= 0 && pt < size) {
		pt += rpt;
		__UNLIKELY_IF(pt >= size) {
			pt -= size;
		}
		return buf[pt];
	}
	return 0;
}

void RINGBUFFER::write_not_push(int pt, int d, bool *p_fill_warn)
{
	__UNLIKELY_IF(p_fill_warn != nullptr) {
		*p_fill_warn = fill_warn_flag;
	}
	__LIKELY_IF(pt >= 0 && pt < size) {
		pt += wpt;
		__UNLIKELY_IF(pt >= size) {
			pt -= size;
		}
		buf[pt] = d;
		return;
	}
}
