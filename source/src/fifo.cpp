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

