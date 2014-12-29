//
// PC-6001/6601 disk I/O
// This file is based on a disk I/O program in C++
// by Mr. Yumitaro and translated into C for Cocoa iP6
// by Koichi NISHIDA 2006
//

/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Origin : tanam
	Date   : 2014.05.21-

	[ PC-6031 ]
*/

#include "pc6031.h"
#include "disk.h"

int PC6031::Seek88(int drvno, int trackno, int sectno)
{
	if(drvno < 2) {
		cur_trk[drvno] = trackno;
		cur_sct[drvno] = sectno;
		cur_pos[drvno] = 0;
		
		if(disk[drvno]->get_track(trackno >> 1, trackno & 1)) {
			for(int i = 0; i < disk[drvno]->sector_num; i++) {
				if(disk[drvno]->get_sector(trackno >> 1, 0/*trackno & 1*/, i)) {
					if(disk[drvno]->id[2] == sectno) {
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

unsigned char PC6031::Getc88(int drvno)
{
	if(drvno < 2 && disk[drvno]->sector != NULL) {
		if(cur_pos[drvno] >= disk[drvno]->sector_size) {
			cur_sct[drvno]++;
			if(!Seek88(drvno, cur_trk[drvno], cur_sct[drvno])) {
//				cur_trk[drvno]++;
				cur_trk[drvno] += 2;
				cur_sct[drvno] = 1;
				if(!Seek88(drvno, cur_trk[drvno], cur_sct[drvno])) {
					return 0xff;
				}
			}
		}
		access[drvno] = true;
		return disk[drvno]->sector[cur_pos[drvno]++];
	}
	return 0xff;
}

int PC6031::Putc88(int drvno, unsigned char dat)
{
	if(drvno < 2 && disk[drvno]->sector != NULL) {
		if(cur_pos[drvno] >= disk[drvno]->sector_size) {
			cur_sct[drvno]++;
			if(!Seek88(drvno, cur_trk[drvno], cur_sct[drvno])) {
//				cur_trk[drvno]++;
				cur_trk[drvno] += 2;
				cur_sct[drvno] = 1;
				if(!Seek88(drvno, cur_trk[drvno], cur_sct[drvno])) {
					return 0xff;
				}
			}
		}
		access[drvno] = true;
		disk[drvno]->sector[cur_pos[drvno]++] = dat;
		return 1;
	}
	return 0;
}

// command
enum FddCommand
{
	INIT				= 0x00,
	WRITE_DATA			= 0x01,
	READ_DATA			= 0x02,
	SEND_DATA			= 0x03,
	COPY				= 0x04,
	FORMAT				= 0x05,
	SEND_RESULT_STATUS	= 0x06,
	SEND_DRIVE_STATUS	= 0x07,
	TRANSMIT			= 0x11,
	RECEIVE				= 0x12,
	LOAD				= 0x14,
	SAVE				= 0x15,
	WAIT				= 0xff,	// waiting state
	EndofFdcCmd
};

// data input (port D0H)
unsigned char PC6031::FddIn60()
{
	unsigned char ret;

	if (mdisk.DAV) {		// if data is valid
		if (mdisk.step == 6) {
			mdisk.retdat = Getc88(mdisk.drv);
			if(--mdisk.size == 0) mdisk.step = 0;
		}
		mdisk.DAC = 1;
		ret = mdisk.retdat;
	} else {			// if data is not valid
		ret = 0xff;
	}
	return ret;
}

// data/command output (port D1H)
void PC6031::FddOut60(unsigned char dat)
{
	if (mdisk.command == WAIT) {	// when command
		mdisk.command = dat;
		switch (mdisk.command) {
		case INIT:					// 00h init
			break;
		case WRITE_DATA:			// 01h write data
			mdisk.step = 1;
			break;
		case READ_DATA:				// 02h read data
			mdisk.step = 1;
			break;
		case SEND_DATA:				// 03h send data
			mdisk.step = 6;
			break;
		case COPY:					// 04h copy
			break;
		case FORMAT:				// 05h format
			break;
		case SEND_RESULT_STATUS:	// 06h send result status
			mdisk.retdat = 0x40;
			break;
		case SEND_DRIVE_STATUS:		// 07h send drive status
			mdisk.retdat |= 0x0a;
			break;
		case TRANSMIT:				// 11h transnmit
			break;
		case RECEIVE:				// 12h receive
			break;
		case LOAD:					// 14h load
			break;
		case SAVE:					// 15h save
			break;
		}
	} else {					// when data
		switch (mdisk.command) {
		case WRITE_DATA:			// 01h write data
			switch (mdisk.step) {
			case 1:	// 01h:block number
				mdisk.blk = dat;
				mdisk.size = mdisk.blk*256;
				mdisk.step++;
				break;
			case 2:	// 02h:drive number - 1
				mdisk.drv = dat;
				mdisk.step++;
				break;
			case 3:	// 03h:track number
				mdisk.trk = dat;
				mdisk.step++;
				break;
			case 4:	// 04h:sector number
				mdisk.sct = dat;
				// double track number(1D->2D)
				Seek88(mdisk.drv, mdisk.trk*2, mdisk.sct);
				mdisk.step++;
				break;
			case 5:	// 05h:write data
				Putc88(mdisk.drv, dat);
				if( --mdisk.size == 0 ){
					mdisk.step = 0;
				}
				break;
			}
			break;
		case READ_DATA:				// 02h read data
			switch (mdisk.step) {
			case 1:	// 01h:block number
				mdisk.blk = dat;
				mdisk.size = mdisk.blk*256;
				mdisk.step++;
				break;
			case 2:	// 02h:drive number-1
				mdisk.drv = dat;
				mdisk.step++;
				break;
			case 3:	// 03h:track number
				mdisk.trk = dat;
				mdisk.step++;
				break;
			case 4:	// 04h:sector number
				mdisk.sct = dat;
				// double track number(1D->2D)
				Seek88(mdisk.drv, mdisk.trk*2, mdisk.sct);
				mdisk.step = 0;
				break;
			}
		}
	}
}

// control input from disk unit (port D2H)
unsigned char PC6031::FddCntIn60(void)
{
	if (((old_D2H & 0x01) ^ mdisk.DAV) || mdisk.RFD && mdisk.DAV) {
		mdisk.DAC = mdisk.DAV;
	} else if (mdisk.ATN) {
		mdisk.RFD = 1;
		mdisk.command = WAIT;
	} else if (mdisk.DAC) {
		mdisk.DAV = 0;
	} else if (mdisk.RFD) {
		mdisk.DAV = 1;
	}	
	old_D2H = io_D2H;
	io_D2H = 0xf0 | 0x08 /* (mdisk.ATN<<3) */ | (mdisk.DAC<<2) | (mdisk.RFD<<1) | mdisk.DAV;
	return (io_D2H);
}

// control output to disk unit (port D3H)
void PC6031::FddCntOut60(unsigned char dat)
{
	// 8255 basic behavior
	if (!(dat&0x80)) {		// check msb
							// ignore when 1
		switch ((dat>>1)&0x07) {
		case 7:	// bit7 ATN
			mdisk.ATN = dat&1;
			break;
		case 6:	// bit6 DAC
			mdisk.DAC = dat&1;
			break;
		case 5:	// bit5 RFD
			mdisk.RFD = dat&1;
			break;
		case 4:	// bit4 DAV
			mdisk.DAV = dat&1;
			break;
		}
		io_D2H = 0xf0 | 0x08 /* (mdisk.ATN<<3) */ | (mdisk.DAC<<2) | (mdisk.RFD<<1) | mdisk.DAV;
	}
}

// I/O access functions
void PC6031::OutD1H_60(unsigned char data) { io_D1H = data; FddOut60(io_D1H); }
void PC6031::OutD2H_60(unsigned char data) {
	mdisk.ATN = (data & 0x80) >> 7;
	mdisk.DAC = (data & 0x40) >> 6;
	mdisk.RFD = (data & 0x20) >> 5;
	mdisk.DAV = (data & 0x10) >> 4;
	io_D2H = 0xf0 | 0x08 /* (mdisk.ATN<<3) */ | (mdisk.DAC<<2) | (mdisk.RFD<<1) | mdisk.DAV;
}
void PC6031::OutD3H_60(unsigned char data) { io_D3H = data; FddCntOut60(io_D3H); }

unsigned char PC6031::InD0H_60() { return FddIn60(); }
unsigned char PC6031::InD1H_60() { return io_D1H; }
unsigned char PC6031::InD2H_60() { io_D2H = FddCntIn60(); return io_D2H; }
unsigned char PC6031::InD3H_60() { return io_D3H; }

void PC6031::initialize()
{
	for(int i = 0; i < 2; i++) {
		disk[i] = new DISK();
	}
	DrvNum = 1;
	memset(&mdisk, 0, sizeof(DISK60));
	mdisk.command = WAIT;		// received command
	mdisk.retdat  = 0xff;		// data from port D0H
	io_D1H = 0;
	io_D2H = 0xf0 | 0x08 /* (mdisk.ATN<<3) */ | (mdisk.DAC<<2) | (mdisk.RFD<<1) | mdisk.DAV;
	io_D3H = 0;
	old_D2H = 0;
}

void PC6031::release()
{
	for(int i = 0; i < 2; i++) {
		if(disk[i]) {
			delete disk[i];
		}
	}
}

void PC6031::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 3) {
	case 1:
		OutD1H_60(data);
		break;
	case 2:
		OutD2H_60(data);
		break;
	case 3:
		OutD3H_60(data);
		break;
	}
}

uint32 PC6031::read_io8(uint32 addr)
{
	switch(addr & 3) {
	case 0:
		return InD0H_60(); break;
	case 1:
		return InD1H_60(); break;
	case 2:
		return InD2H_60(); break;
	case 3:
		return InD3H_60(); break;
	}
	return 0xff;
}

uint32 PC6031::read_signal(int ch)
{
	// get access status
	uint32 stat = 0;
	for(int drv = 0; drv < 2; drv++) {
		if(access[drv]) {
			stat |= 1 << drv;
		}
		access[drv] = false;
	}
	return stat;
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void PC6031::open_disk(int drv, _TCHAR path[], int offset)
{
	if(drv < 2) {
		disk[drv]->open(path, offset);
		Seek88(drv, 0, 1);
	}
}

void PC6031::close_disk(int drv)
{
	if(drv < 2 && disk[drv]->inserted) {
		disk[drv]->close();
	}
}

bool PC6031::disk_inserted(int drv)
{
	if(drv < 2) {
		return disk[drv]->inserted;
	}
	return false;
}

bool PC6031::disk_ejected(int drv)
{
	if(drv < 2) {
		return disk[drv]->ejected;
	}
	return false;
}

