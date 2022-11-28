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
//	$Id: diskio.cpp,v 1.2 1999/09/25 03:13:51 cisc Exp $

#include "diskio.h"

// ---------------------------------------------------------------------------
//	Init
//

void DiskIO::Reset(uint32_t a, uint32_t d)
{
	writebuffer = false;
	filename[0] = 0;
	IdlePhase();
}

// ---------------------------------------------------------------------------
//	コマンド
//
void DiskIO::SetCommand(uint32_t a, uint32_t d)
{
	if (d != 0x84 || !writebuffer)
		file->Fclose();
	phase = idlephase;
	cmd = d;
//	LOG1("\n[%.2x]", d);
	status |= 1;
	ProcCommand();
}

// ---------------------------------------------------------------------------
//	ステータス
//
uint32_t DiskIO::GetStatus(uint32_t a)
{
	return status;
}

// ---------------------------------------------------------------------------
//	データセット
//
void DiskIO::SetData(uint32_t a, uint32_t d)
{
	if (phase == recvphase || phase == argphase)
	{
		*ptr++ = d;
		if (--len <= 0)
		{
			status &= ~2;
			ProcCommand();
		}
	}
}

// ---------------------------------------------------------------------------
//	データげっと
//
uint32_t DiskIO::GetData(uint32_t a)
{
	uint32_t r = 0xff;
	if (phase == sendphase)
	{
		r = *ptr++;
		if (--len <= 0)
		{
			status &= ~(2|4);
			ProcCommand();
		}
	}
	return r;
}

// ---------------------------------------------------------------------------
//
//
void DiskIO::SendPhase(uint8_t* p, int l)
{
	ptr = p, len = l;
	phase = sendphase;
	status |= 2 | 4;
}

void DiskIO::ArgPhase(int l)
{
	ptr = arg, len = l;
	phase = argphase;
}

// ---------------------------------------------------------------------------
//
//
void DiskIO::RecvPhase(uint8_t* p, int l)
{
	ptr = p, len = l;
	phase = recvphase;
	status |= 2;
}

// ---------------------------------------------------------------------------
//
//
void DiskIO::IdlePhase()
{
	phase = idlephase;
	status = 0;
	file->Fclose();
}

// ---------------------------------------------------------------------------
//
//
void DiskIO::ProcCommand()
{
	switch (cmd)
	{
	case 0x80:	CmdSetFileName();	break;
	case 0x81:	CmdReadFile();		break;
	case 0x82:	CmdWriteFile();		break;
	case 0x83:	CmdGetError();		break;
	case 0x84:	CmdWriteFlush();	break;
	default:	IdlePhase();		break;
	}
}

// ---------------------------------------------------------------------------

void DiskIO::CmdSetFileName()
{
	switch (phase)
	{
	case idlephase:
//		LOG0("SetFileName ");
		ArgPhase(1);
		break;

	case argphase:
		if (arg[0])
		{
			RecvPhase(filename, arg[0]);
			err = 0;
			break;
		}
		err = 56;
	case recvphase:
		filename[arg[0]] = 0;
//		LOG1("Path=%s", filename);
		IdlePhase();
		break;
	}
}

// ---------------------------------------------------------------------------

void DiskIO::CmdReadFile()
{
	switch (phase)
	{
	case idlephase:
		writebuffer = false;
//		LOG1("ReadFile(%s) - ", filename);
		if (file->Fopen(create_absolute_path(char_to_tchar((char*) filename)), FILEIO_READ_BINARY))
		{
			file->Fseek(0, FILEIO_SEEK_END);
			size = min(0xffff, file->Ftell());
			file->Fseek(0, FILEIO_SEEK_SET);
			buf[0] = size & 0xff;
			buf[1] = (size >> 8) & 0xff;
//			LOG1("%d bytes  ", size);
			SendPhase(buf, 2);
		}
		else
		{
//			LOG0("failed");
			err = 53;
			IdlePhase();
		}
		break;

	case sendphase:
		if (size > 0)
		{
			int b = min(1024, size);
			size -= b;
			if (file->Fread(buf, b, 1))
			{
				SendPhase(buf, b);
				break;
			}
			err = 64;
		}

//		LOG0("success");
		IdlePhase();
		err = 0;
		break;
	}
}

// ---------------------------------------------------------------------------

void DiskIO::CmdWriteFile()
{
	switch (phase)
	{
	case idlephase:
		writebuffer = true;
//		LOG1("WriteFile(%s) - ", filename);
		if (file->Fopen(create_absolute_path(char_to_tchar((char*) filename)), FILEIO_WRITE_BINARY))
			ArgPhase(2);
		else
		{
//			LOG0("failed");
			IdlePhase(), err = 60;
		}
		break;

	case argphase:
		size = arg[0] + arg[1] * 256;
		if (size > 0)
		{
//			LOG0("%d bytes ");
			length = min(1024, size);
			size -= length;
			RecvPhase(buf, length);
		}
		else
		{
//			LOG0("success");
			IdlePhase(), err = 0;
		}
		break;

	case recvphase:
		if (!file->Fwrite(buf, length, 1))
		{	
//			LOG0("write error");
			IdlePhase(), err = 61;
		}
		if (size > 0)
		{
			length = min(1024, size);
			size -= length;
			RecvPhase(buf, length);
		}
		else
			ArgPhase(2);
		break;
	}
}

void DiskIO::CmdWriteFlush()
{
	switch (phase)
	{
	case idlephase:
//		LOG0("WriteFlush ");
		if (writebuffer)
		{
//			if (length-len > 0)
//				LOG1("%d bytes\n", length - len);
			file->Fwrite(buf, length-len, 1);
			writebuffer = false;
		}
		else
		{
//			LOG0("failed\n");
			err = 51;
		}
		IdlePhase();
		break;
	}
}

// ---------------------------------------------------------------------------

void DiskIO::CmdGetError()
{
	switch (phase)
	{
	case idlephase:
		buf[0] = err;
		SendPhase(buf, 1);
		break;
		
	case sendphase:
		IdlePhase();
		break;
	}
}

// ---------------------------------------------------------------------------
//	Common Source Code Project
//

void DiskIO::initialize()
{
	file = new FILEIO();
}

void DiskIO::release()
{
	delete file;
}

void DiskIO::reset()
{
	this->Reset(0, 0);
}

void DiskIO::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xd0:
		this->SetCommand(addr, data & 0xff);
		break;
	case 0xd1:
		this->SetData(addr, data & 0xff);
		break;
	}
}

uint32_t DiskIO::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xd0:
		return this->GetStatus(addr) & 0xff;
	case 0xd1:
		return this->GetData(addr) & 0xff;
	}
	return 0xff;
}
