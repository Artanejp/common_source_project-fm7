//
// PC-6001/6601 disk I/O
// This file is based on a disk I/O program in C++
// by Mr. Yumitaro and translated into C for Cocoa iP6
// by Koichi NISHIDA 2006
//

/*
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.12.04-

	[ internal floppy drive ]
*/

#include "floppy.h"
#include "../disk.h"

int FLOPPY::Seek88(int drvno, int trackno, int sectno)
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

unsigned char FLOPPY::Getc88(int drvno)
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

int FLOPPY::Putc88(int drvno, unsigned char dat)
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

// push data to data buffer
void FLOPPY::Push(int part, unsigned char data)
{
	if (part > 3) return;
	
	if(Index[part] < 256) Data[part][Index[part]++] = data;
}

// pop data from data buffer
unsigned char FLOPPY::Pop(int part)
{
	if(part > 3) return 0xff;
	
	if(Index[part] > 0) return Data[part][--Index[part]];
	else                return 0xff;
}

// clear data
void FLOPPY::Clear(int i)
{
	Index[i] = 0;
}

// FDC Status
#define FDC_BUSY			(0x10)
#define FDC_READY			(0x00)
#define FDC_NON_DMA			(0x20)
#define FDC_FD2PC			(0x40)
#define FDC_PC2FD			(0x00)
#define FDC_DATA_READY		(0x80)

// Result Status 0
#define ST0_NOT_READY		(0x08)
#define ST0_EQUIP_CHK		(0x10)
#define ST0_SEEK_END		(0x20)
#define ST0_IC_NT			(0x00)
#define ST0_IC_AT			(0x40)
#define ST0_IC_IC			(0x80)
#define ST0_IC_AI			(0xc0)

// Result Status 1
#define ST1_NOT_WRITABLE	(0x02)

// Result Status 2

// Result Status 3
#define ST3_TRACK0			(0x10)
#define ST3_READY			(0x20)
#define ST3_WRITE_PROTECT	(0x40)
#define ST3_FAULT			(0x80)

// initialise
int FLOPPY::DiskInit66(void)
{
	memset( &CmdIn,  0, sizeof( CmdBuffer ) );
	memset( &CmdOut, 0, sizeof( CmdBuffer ) );
	SeekST0 = 0;
	LastCylinder = 0;
	SeekEnd = 0;
	SendSectors  = 0;
	Status = FDC_DATA_READY | FDC_READY | FDC_PC2FD;
	return 1;
}

// push data to status buffer
void FLOPPY::PushStatus(int data)
{
	CmdOut.Data[CmdOut.Index++] = data;
}

// pop data from status buffer
unsigned char FLOPPY::PopStatus()
{
	return CmdOut.Data[--CmdOut.Index];
}

// write to FDC
void FLOPPY::OutFDC(unsigned char data)
{
	const int CmdLength[] = { 0,0,0,3,2,9,9,2,1,0,0,0,0,6,0,3 };
	
	CmdIn.Data[CmdIn.Index++] = data;
	if (CmdLength[CmdIn.Data[0]&0xf] == CmdIn.Index) Exec();
}

// read from FDC
unsigned char FLOPPY::InFDC()
{
	if (CmdOut.Index == 1) Status = FDC_DATA_READY | FDC_PC2FD;
	return PopStatus();
}

// read
void FLOPPY::Read()
{
	int Drv, C, H, R, N;
	int i, j;
	
	Drv = CmdIn.Data[1]&3;		// drive number No.(0-3)
	C   = CmdIn.Data[2];		// cylinder
	H   = CmdIn.Data[3];		// head address
	R   = CmdIn.Data[4];		// sector No.
	N   = CmdIn.Data[5] ? CmdIn.Data[5]*256 : 256;	// sector size
	
	if (disk[Drv]->inserted) {
		// seek
		// double track number(1D->2D)
		Seek88(Drv, C*2+H, R);
		for (i = 0; i < SendSectors; i++) {
			Clear(i);
			for(j=0; j<N; j++)
				Push(i, Getc88(Drv));
		}
	}
	PushStatus(N);	// N
	PushStatus(R);	// R
	PushStatus(H);	// H
	PushStatus(C);	// C
	PushStatus(0);	// st2
	PushStatus(0);	// st1
	PushStatus(disk[Drv]->inserted ? 0 : ST0_NOT_READY);	// st0  bit3 : media not ready
	Status = FDC_DATA_READY | FDC_FD2PC;
}

// Write
void FLOPPY::Write(void)
{
	int Drv, C, H, R, N;
	int i, j;
	
	Drv = CmdIn.Data[1]&3;		// drive No.(0-3)
	C   = CmdIn.Data[2];		// cylinder
	H   = CmdIn.Data[3];		// head address
	R   = CmdIn.Data[4];		// sector No.
	N   = CmdIn.Data[5] ? CmdIn.Data[5]*256 : 256;	// sector size
	
	if (disk[Drv]->inserted) {
		// seek
		// double track number(1D->2D)
		Seek88(Drv, C*2+H, R);
		for (i=0; i<SendSectors; i++) {
			for(j=0; j<0x100; j++)
				Putc88(Drv, Pop(i));	// write data
		}
	}
	
	PushStatus(N);	// N
	PushStatus(R);	// R
	PushStatus(H);	// H
	PushStatus(C);	// C
	PushStatus(0);	// st2
	PushStatus(0);	// st1
	
	PushStatus(disk[Drv]->inserted ? 0 : ST0_NOT_READY);	// st0  bit3 : media not ready
	
	Status = FDC_DATA_READY | FDC_FD2PC;
}

// seek
void FLOPPY::Seek(void)
{
	int Drv,C,H;
	
	Drv = CmdIn.Data[1]&3;		// drive No.(0-3)
	C   = CmdIn.Data[2];		// cylinder
	H   = CmdIn.Data[3];		// head address
	
	if (!disk[Drv]->inserted) {	// disk unmounted ?
		SeekST0      = ST0_IC_AT | ST0_SEEK_END | ST0_NOT_READY | Drv;
		SeekEnd      = 0;
		LastCylinder = 0;
	} else { // seek
		// double number(1D->2D)
		Seek88(Drv, C*2+H, 1);
		SeekST0      = ST0_IC_NT | ST0_SEEK_END | Drv;
		SeekEnd      = 1;
		LastCylinder = C;
	}
}

// sense interrupt status
void FLOPPY::SenseInterruptStatus(void)
{
	if (SeekEnd) {
		SeekEnd = 0;
		PushStatus(LastCylinder);
		PushStatus(SeekST0);
	} else {
		PushStatus(0);
		PushStatus(ST0_IC_IC);
	}
	
	Status = FDC_DATA_READY | FDC_FD2PC;
}

// execute FDC command
void FLOPPY::Exec()
{
	CmdOut.Index = 0;
	switch (CmdIn.Data[0] & 0xf) {
	case 0x03:	// Specify
		break;
	case 0x05:	// Write Data
		Write();
		break;
	case 0x06:	// Read Data
		Read();
		break;
	case 0x08:	// Sense Interrupt Status
		SenseInterruptStatus();
		break;
	case 0x0d:	// Write ID
		// Format is Not Implimented
		break;
	case 0x07:	// Recalibrate
		CmdIn.Data[2] = 0;	// Seek to TRACK0
	case 0x0f:	// Seek
		Seek();
		break;
	default: ;	// Invalid
	}
	CmdIn.Index = 0;
}

// I/O access functions
void FLOPPY::OutB1H_66(unsigned char data) { DIO = data&2 ? 1 : 0; }			// FD mode
void FLOPPY::OutB2H_66(unsigned char data) {}									// FDC INT?
void FLOPPY::OutB3H_66(unsigned char data) {}									// in out of PortB2h
void FLOPPY::OutD0H_66(unsigned char data) { Push(0, data); }					// Buffer
void FLOPPY::OutD1H_66(unsigned char data) { Push(1, data); }					// Buffer
void FLOPPY::OutD2H_66(unsigned char data) { Push(2, data); }					// Buffer
void FLOPPY::OutD3H_66(unsigned char data) { Push(3, data); }					// Buffer
void FLOPPY::OutD6H_66(unsigned char data) {}									// select drive
void FLOPPY::OutD8H_66(unsigned char data) {}									//
void FLOPPY::OutDAH_66(unsigned char data) { SendSectors = ~(data - 0x10); }	// set transfer amount
void FLOPPY::OutDDH_66(unsigned char data) { OutFDC(data); }					// FDC data register
void FLOPPY::OutDEH_66(unsigned char data) {}									// ?
	
unsigned char FLOPPY::InB2H_66() { return 3; }									// FDC INT
unsigned char FLOPPY::InD0H_66() { return Pop(0); }								// Buffer
unsigned char FLOPPY::InD1H_66() { return Pop(1); }								// Buffer
unsigned char FLOPPY::InD2H_66() { return Pop(2); }								// Buffer
unsigned char FLOPPY::InD3H_66() { return Pop(3); }								// Buffer
unsigned char FLOPPY::InD4H_66() { return 0; }									// Mortor(on 0/off 1)
unsigned char FLOPPY::InDCH_66() { return Status; }								// FDC status register
unsigned char FLOPPY::InDDH_66() { return InFDC(); }							// FDC data register

void FLOPPY::initialize()
{
	for(int i = 0; i < 2; i++) {
		disk[i] = new DISK();
	}
	DiskInit66();
}

void FLOPPY::release()
{
	for(int i = 0; i < 2; i++) {
		if(disk[i]) {
			disk[i]->close();
			delete disk[i];
		}
	}
}

void FLOPPY::reset()
{
	io_B1H = 0;
	memset(Index, 0, sizeof(Index));
}

void FLOPPY::write_io8(uint32 addr, uint32 data)
{
	// disk I/O
	uint16 port=(addr & 0x00ff);
	byte Value=(data & 0xff);
	
	switch(port)
	{
	// disk I/O
	case 0xB1:
	case 0xB5:
		io_B1H = Value;
		break;
	case 0xB2:
	case 0xB6:
		OutB2H_66(Value);
		break;
	case 0xB3:
	case 0xB7:
		OutB3H_66(Value);
		break;
	case 0xD0:
		if(io_B1H & 4) {
			d_ext->write_io8(0, data);
		} else {
			OutD0H_66(Value);
		}
		break;
	case 0xD1:
		if(io_B1H & 4) {
			d_ext->write_io8(1, data);
		} else {
			OutD1H_66(Value);
		}
		break;
	case 0xD2:
		if(io_B1H & 4) {
			d_ext->write_io8(2, data);
		} else {
			OutD2H_66(Value);
		}
		break;
	case 0xD3:
		if(io_B1H & 4) {
			d_ext->write_io8(3, data);
		} else {
			OutD3H_66(Value);
		}
		break;
	case 0xD4:
		if(io_B1H & 4) {
			d_ext->write_io8(0, data);
		}
		break;
	case 0xD5:
		if(io_B1H & 4) {
			d_ext->write_io8(1, data);
		}
		break;
	case 0xD6:
		if(io_B1H & 4) {
			d_ext->write_io8(2, data);
		} else {
			OutD6H_66(Value);
		}
		break;
	case 0xD7:
		if(io_B1H & 4) {
			d_ext->write_io8(3, data);
		}
		break;
	case 0xD8:
		OutD8H_66(Value);
		break;
	case 0xDA:
		OutDAH_66(Value);
		break;
	case 0xDD:
		OutDDH_66(Value);
		break;
	case 0xDE:
		OutDEH_66(Value);
		break;
	}
	return;
}

uint32 FLOPPY::read_io8(uint32 addr)
{
	// disk I/O
	uint16 port=(addr & 0x00ff);
	byte Value=0xff;
	
	switch(addr & 0xff) {
	case 0xB2:
	case 0xB6:
		Value=InB2H_66();
		break;
	case 0xD0:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(0);
		} else {
			Value=InD0H_66();
		}
		break;
	case 0xD1:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(1);
		} else {
			Value=InD1H_66();
		}
		break;
	case 0xD2:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(2);
		} else {
			Value=InD2H_66();
		}
		break;
	case 0xD3:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(3);
		} else {
			Value=InD3H_66();
		}
		break;
	case 0xD4:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(0);
		} else {
			Value=InD4H_66();
		}
		break;
	case 0xD5:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(1);
		}
		break;
	case 0xD6:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(2);
		}
		break;
	case 0xD7:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(3);
		}
		break;
	case 0xDC:
		Value=InDCH_66();
		break;
	case 0xDD:
		Value=InDDH_66();
		break;
	}
	return(Value);
}

uint32 FLOPPY::read_signal(int ch)
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

void FLOPPY::open_disk(int drv, _TCHAR path[], int offset)
{
	if(drv < 2) {
		disk[drv]->open(path, offset);
		Seek88(drv, 0, 1);
	}
}

void FLOPPY::close_disk(int drv)
{
	if(drv < 2 && disk[drv]->inserted) {
		disk[drv]->close();
	}
}

bool FLOPPY::disk_inserted(int drv)
{
	if(drv < 2) {
		return disk[drv]->inserted;
	}
	return false;
}
