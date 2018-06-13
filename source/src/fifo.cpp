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

FIFO::FIFO(int s)
{
	size = s;
	buf = (int*)malloc(size * sizeof(int));
	cnt = rpt = wpt = 0;
}

void FIFO::release()
{
	free(buf);
}

void FIFO::clear()
{
	cnt = rpt = wpt = 0;
}

void FIFO::write(int val)
{
	if(cnt < size) {
		buf[wpt++] = val;
		if(wpt >= size) {
			wpt = 0;
		}
		cnt++;
	}
}

int FIFO::read()
{
	int val = 0;
	if(cnt) {
		val = buf[rpt++];
		if(rpt >= size) {
			rpt = 0;
		}
		cnt--;
	}
	return val;
}

int FIFO::read_not_remove(int pt)
{
	if(pt >= 0 && pt < cnt) {
		pt += rpt;
		if(pt >= size) {
			pt -= size;
		}
		return buf[pt];
	}
	return 0;
}

void FIFO::write_not_push(int pt, int d)
{
	if(pt >= 0 && pt < cnt) {
		pt += wpt;
		if(pt >= size) {
			pt -= size;
		}
		buf[pt] = d;
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

#include "./state_data.h"

void FIFO::save_state_helper(csp_state_data_saver *state_saver, uint32_t *sumseed, bool *__stat)
{
	if(state_saver == NULL) return;

	state_saver->save_string_data(_ns, sumseed, strlen(_ns) + 1, __stat);
	state_saver->put_dword(STATE_VERSION, sumseed, __stat);
	
	state_saver->put_int32(size, sumseed, __stat);
	for(int i = 0; i < size; i++) {
		state_saver->put_int32(buf[i], sumseed, __stat);
	}
	state_saver->put_int32(cnt, sumseed, __stat);
	state_saver->put_int32(rpt, sumseed, __stat);
	state_saver->put_int32(wpt, sumseed, __stat);
	
	state_saver->save_string_data(_ne, sumseed, strlen(_ne) + 1, __stat);
	
}

bool FIFO::load_state_helper(csp_state_data_saver *state_saver, uint32_t *sumseed, bool *__stat)
{

	_TCHAR sbuf[128];
	uint32_t tmpvar;
	if(state_saver == NULL) return false;
	
	memset(sbuf, 0x00, sizeof(sbuf));
	state_saver->load_string_data(sbuf, sumseed, strlen(_ns) + 1, __stat);
	tmpvar = state_saver->get_dword(sumseed, __stat);

	if(strncmp(sbuf, _ns, strlen(_ns) + 1) != 0) {
		if(__stat != NULL) *__stat = false;
		return false;
	}
	if(tmpvar != STATE_VERSION) {
		if(__stat != NULL) *__stat = false;
		return false;
	}
	
	size = state_saver->get_int32(sumseed, __stat);
	for(int i = 0; i < size; i++) {
		buf[i] = state_saver->get_int32(sumseed, __stat);
	}
	cnt = state_saver->get_int32(sumseed, __stat);
	rpt = state_saver->get_int32(sumseed, __stat);
	wpt = state_saver->get_int32(sumseed, __stat);
	
	memset(sbuf, 0x00, sizeof(sbuf));
	state_saver->load_string_data(sbuf, sumseed, strlen(_ne) + 1, __stat);
	if(strncmp(_ne, sbuf, strlen(_ne) + 1) != 0) {
		if(__stat != NULL) *__stat = false;
		return false;
	}
	
	if(__stat != NULL) {
		if(*__stat == false) return false;
		//*__stat = true;
	}
	return true;
}


void FIFO::save_state(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;
	
	state_fio->FputUint32(STATE_VERSION);
	
	state_fio->FputInt32(size);
	state_fio->Fwrite(buf, size * sizeof(int), 1);
	state_fio->FputInt32(cnt);
	state_fio->FputInt32(rpt);
	state_fio->FputInt32(wpt);
}

bool FIFO::load_state(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;
	
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != size) {
		return false;
	}
	state_fio->Fread(buf, size * sizeof(int), 1);
	cnt = state_fio->FgetInt32();
	rpt = state_fio->FgetInt32();
	wpt = state_fio->FgetInt32();
	return true;
}

