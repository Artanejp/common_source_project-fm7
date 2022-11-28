/*
	NEC PC-8001 Emulator 'ePC-8001'
	NEC PC-8001mkII Emulator 'ePC-8001mkII'
	NEC PC-8001mkIISR Emulator 'ePC-8001mkIISR'
	NEC PC-8801 Emulator 'ePC-8801'
	NEC PC-8801mkII Emulator 'ePC-8801mkII'
	NEC PC-8801MA Emulator 'ePC-8801MA'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2020.12.14-

	[ M88 DiskDrv ]
*/

// ---------------------------------------------------------------------------
//	PC-8801 emulator
//	Copyright (C) cisc 1999.
// ---------------------------------------------------------------------------
//	$Id: diskio.h,v 1.3 1999/10/10 01:38:05 cisc Exp $

#pragma once

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FILEIO;

// ---------------------------------------------------------------------------

class DiskIO : public DEVICE
{
public:
	DiskIO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("M88 DiskDrv"));
	}
	~DiskIO() {}

	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

private:
	void Reset(uint32_t a, uint32_t d);
	void SetCommand(uint32_t a, uint32_t d);
	void SetData(uint32_t a, uint32_t d);
	uint32_t GetStatus(uint32_t a);
	uint32_t GetData(uint32_t a);

	enum Phase
	{
		idlephase, argphase, recvphase, sendphase,
	};
	
	void ProcCommand();
	void ArgPhase(int l);
	void SendPhase(uint8_t* p, int l);
	void RecvPhase(uint8_t* p, int l);
	void IdlePhase();

	void CmdSetFileName();
	void CmdWriteFile();
	void CmdReadFile();
	void CmdGetError();
	void CmdWriteFlush();

	uint8_t* ptr;
	int len;
	
//	FileIO file;
	FILEIO *file;
	int size;
	int length;
	
	Phase phase;
	bool writebuffer;
	uint8_t status;
	uint8_t cmd;
	uint8_t err;
	uint8_t arg[5];
	uint8_t filename[MAX_PATH];
	uint8_t buf[1024];
};
