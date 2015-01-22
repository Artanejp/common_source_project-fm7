/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.02.28 -

	[ EPSON TF-20 ]
*/

#include "tf20.h"
#include "disk.h"

/*
	This is based on vfloppy 1.4 by:

	Justin Mitchell (madmitch@discordia.org.uk) and friends.
	Fred Jan Kraan (fjkraan@xs4all.nl)
*/

#define PHASE_IDLE	0
#define PHASE_SELECT	1
#define PHASE_FUNC	2
#define PHASE_HEAD	3
#define PHASE_DATA	4
#define PHASE_EXEC	5
#define PHASE_RESULT	6
#define PHASE_END	7

#define DID_FIRST	0x31
#define DID_SECOND	0x32
#define DS_SEL		0x05

#define NUL		0x00
#define SOH		0x01
#define STX		0x02
#define ETX		0x03
#define EOT		0x04
#define ENQ		0x05
#define ACK		0x06
#define NAK		0x15
#define US		0x31

#define FNC_RESET_P	0x0d
#define FNC_RESET_M	0x0e
#define FNC_READ	0x77
#define FNC_WRITE	0x78
#define FNC_WRITEHST	0x79
#define FNC_COPY	0x7a
#define FNC_FORMAT	0x7c

#define ERR_SUCCESS	0x00
#define ERR_READ	0xfa
#define ERR_WRITE	0xfb
#define ERR_DRIVE	0xfc
#define ERR_PROTECTED	0xfd
#define ERR_UNKNOWN	0xfe

void TF20::initialize()
{
	// initialize d88 handler
	for(int i = 0; i < MAX_DRIVE; i++) {
		disk[i] = new DISK();
	}
}

void TF20::release()
{
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(disk[i]) {
			disk[i]->close();
			delete disk[i];
		}
	}
}

void TF20::reset()
{
	phase = PHASE_IDLE;
	buflen = 0;
}

#define REPLY(val) d_sio->write_signal(did_sio, val, 0xff)

void TF20::write_signal(int id, uint32 data, uint32 mask)
{
	switch(phase) {
	case PHASE_IDLE:
		if(data == EOT) {
			break;
		}
		if(data == ENQ) {
			REPLY(ACK);
			break;
		}
		if(data != DID_FIRST) {
			REPLY(NAK);
			break;
		}
		phase = PHASE_SELECT;
		buflen = 0;
		break;
		
	case PHASE_SELECT:
		bufr[buflen++] = data;
		if(buflen < 3) {
			break;
		}
		if(bufr[0] != DID_FIRST && bufr[0] != DID_SECOND) {
			phase = PHASE_IDLE;
			break;
		}
		if(bufr[2] != DS_SEL) {
			REPLY(NAK);
			phase = PHASE_IDLE;
			break;
		}
		REPLY(ACK);
		phase = PHASE_FUNC;
		break;
		
	case PHASE_FUNC:
		if(data == EOT) {
			phase = PHASE_IDLE;
			break;
		}
		if(data == ENQ) {
			REPLY(ACK);
			break;
		}
		if(data != SOH) {
			REPLY(NAK);
			phase = PHASE_IDLE;
			break;
		}
		phase = PHASE_HEAD;
		buflen = 0;
		break;
		
	case PHASE_HEAD:
		bufr[buflen++] = data;
		if(buflen < 6) {
			break;
		}
		REPLY(ACK);
		phase = PHASE_DATA;
		break;
		
	case PHASE_DATA:
		bufr[buflen++] = data;
		if(buflen < (6 + bufr[4] + 4)) {
			break;
		}
		if(bufr[6] != STX) {
			REPLY(NAK);
			phase = PHASE_IDLE;
			break;
		}
		REPLY(ACK);
		phase = PHASE_EXEC;
		break;
		
	case PHASE_EXEC:
		if(data != EOT) {
			REPLY(NAK);
			phase = PHASE_IDLE;
			break;
		}
		if(!process_cmd()) {
			REPLY(NAK);
			phase = PHASE_IDLE;
			break;
		}
		for(int i = 0; i < 7; i++) {
			REPLY(bufs[i]);
		}
		phase = PHASE_RESULT;
		break;
		
	case PHASE_RESULT:
		if(data == ENQ) {
			REPLY(ACK);
			phase = PHASE_FUNC;
			break;
		}
		if(data != ACK) {
			phase = PHASE_FUNC;
			break;
		}
		for(int i = 7; i < buflen; i++) {
			REPLY(bufs[i]);
		}
		phase = PHASE_END;
		break;
		
	case PHASE_END:
		if(data == ENQ) {
			REPLY(ACK);
		} else if(data == ACK) {
			REPLY(EOT);
		}
		phase = PHASE_FUNC;
		break;
	}
}

#define SET_HEAD(size) { \
	bufs[0] = 1; \
	bufs[1] = 1; \
	bufs[2] = bufr[2]; \
	bufs[3] = bufr[1]; \
	bufs[4] = bufr[3]; \
	bufs[5] = size; \
	uint8 sum = 0; \
	for(int s = 0; s < 6; s++) \
		sum += bufs[s]; \
	bufs[6] = 256 - sum; \
	bufs[7] = 2; \
	buflen = 8; \
}
#define SET_DATA(v) bufs[buflen++] = (v)
#define SET_CODE(v) { \
	bufs[buflen++] = (v); \
	bufs[buflen++] = 3; \
	uint8 sum = 0; \
	for(int s = 7; s < buflen; s++) \
		sum += bufs[s]; \
	bufs[buflen++] = 256 - sum; \
}

bool TF20::process_cmd()
{
	int drv, trk, sec, dst;
	uint8 *sctr, *sctw;
	
	switch(bufr[3]) {
	case FNC_RESET_P:
	case FNC_RESET_M:
		SET_HEAD(0);
		SET_CODE(ERR_SUCCESS);
		return true;
		
	case FNC_READ:
		drv = (bufr[1] == DID_FIRST) ? 0 : (bufr[1] == DID_SECOND) ? 2 : 4;
		drv += bufr[7] - 1;
		trk = bufr[8];
		sec = bufr[9];
		if(!disk_inserted(drv)) {
			// drive error
			SET_HEAD(0x80);
			for(int i = 0; i < 128; i++) {
				SET_DATA(0xff);
			}
			SET_CODE(ERR_DRIVE);
			return true;
		}
		if((sctr = get_sector(drv, trk, sec)) == NULL) {
			// read error
			SET_HEAD(0x80);
			for(int i = 0; i < 128; i++) {
				SET_DATA(0xff);
			}
			SET_CODE(ERR_READ);
			return true;
		}
		SET_HEAD(0x80);
		for(int i = 0; i < 128; i++) {
			SET_DATA(sctr[i]);
		}
		SET_CODE(ERR_SUCCESS);
		return true;
		
	case FNC_WRITE:
		drv = (bufr[1] == DID_FIRST) ? 0 : (bufr[1] == DID_SECOND) ? 2 : 4;
		drv += bufr[7] - 1;
		trk = bufr[8];
		sec = bufr[9];
		if(!disk_inserted(drv)) {
			// drive error
			SET_HEAD(0);
			SET_CODE(ERR_DRIVE);
			return true;
		}
		if(disk_protected(drv)) {
			// write protect
			SET_HEAD(0);
			SET_CODE(ERR_PROTECTED);
			return true;
		}
		if((sctw = get_sector(drv, trk, sec)) == NULL) {
			// write error
			SET_HEAD(0);
			SET_CODE(ERR_WRITE);
			return true;
		}
		// dont care write type
		for(int i = 0; i < 128; i++) {
			sctw[i] = bufr[11 + i];
		}
		SET_HEAD(0);
		SET_CODE(ERR_SUCCESS);
		return true;
		
	case FNC_WRITEHST:
		SET_HEAD(0);
		SET_CODE(ERR_SUCCESS);
		return true;
		
	case FNC_COPY:
		drv = (bufr[1] == DID_FIRST) ? 0 : (bufr[1] == DID_SECOND) ? 2 : 4;
		drv += bufr[7] - 1;
		dst = (drv & ~1) | (~drv & 1);
		if(!disk_inserted(drv)) {
			// drive error
			SET_HEAD(0);
			SET_CODE(ERR_DRIVE);
			return true;
		}
		if(!disk_inserted(dst)) {
			// drive error
			SET_HEAD(0);
			SET_CODE(ERR_DRIVE);
			return true;
		}
		if(disk_protected(dst)) {
			// write protect
			SET_HEAD(0);
			SET_CODE(ERR_PROTECTED);
			return true;
		}
		for(trk = 0; trk < 40; trk++) {
			for(sec = 1; sec <= 64; sec++) {
				if((sctr = get_sector(drv, trk, sec)) == NULL) {
					// read error
					SET_HEAD(0);
					SET_CODE(ERR_READ);
					return true;
				}
				if((sctw = get_sector(dst, trk, sec)) == NULL) {
					// write error
					SET_HEAD(0);
					SET_CODE(ERR_WRITE);
					return true;
				}
				memcpy(sctw, sctr, 128);
			}
			SET_HEAD(2);
			SET_DATA(trk == 39 ? 0xff : 0);		// high-order
			SET_DATA(trk == 39 ? 0xff : trk);	// low-order
			SET_CODE(ERR_SUCCESS);
		}
		return true;
		
	case FNC_FORMAT:
		drv = (bufr[1] == DID_FIRST) ? 0 : (bufr[1] == DID_SECOND) ? 2 : 4;
		drv += bufr[7] - 1;
		if(!disk_inserted(drv)) {
			// drive error
			SET_HEAD(0);
			SET_CODE(ERR_DRIVE);
			return true;
		}
		if(disk_protected(drv)) {
			// write protect
			SET_HEAD(0);
			SET_CODE(ERR_PROTECTED);
			return true;
		}
		for(trk = 0; trk < 40; trk++) {
			for(sec = 1; sec <= 64; sec++) {
				if((sctw = get_sector(drv, trk, sec)) == NULL) {
					// write error
					SET_HEAD(0);
					SET_CODE(ERR_WRITE);
					return true;
				}
				memset(sctw, 0xe5, 128);
			}
			SET_HEAD(2);
			SET_DATA(trk == 39 ? 0xff : 0);		// high-order
			SET_DATA(trk == 39 ? 0xff : trk);	// low-order
			SET_CODE(ERR_SUCCESS);
		}
		return true;
	}
	// unknown command
	return false;
}

bool TF20::disk_protected(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->write_protected;
	}
	return false;
}

uint8* TF20::get_sector(int drv, int trk, int sec)
{
	// logical : trk = 0-39, sec = 1-64, secsize = 128bytes
	int total = trk * 64 + sec - 1;
	int half = total & 1;
	total >>= 1;
	int phys_sec = total & 15;
	total >>= 4;
	int phys_side = total & 1;
	int phys_trk = total >> 1;
	
	if(!disk_inserted(drv)) {
		return NULL;
	}
	if(!disk[drv]->get_sector(phys_trk, phys_side, phys_sec)) {
		return NULL;
	}
	return disk[drv]->sector + (half ? 128 : 0);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void TF20::open_disk(int drv, _TCHAR path[], int offset)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->open(path, offset);
	}
}

void TF20::close_disk(int drv)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->close();
	}
}

bool TF20::disk_inserted(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->inserted;
	}
	return false;
}

