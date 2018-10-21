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
	Date   : 2013.07.15-

	[ internal floppy drive ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class DISK;
class NOISE;

class FLOPPY : public DEVICE
{
private:
	DEVICE *d_ext;
	NOISE *d_noise_seek;
//	NOISE* d_noise_head_down;
//	NOISE* d_noise_head_up;
	unsigned char io_B1H;
	
	DISK* disk[2];
	
	int cur_trk[2];
	int cur_sct[2];
	int cur_pos[2];
	bool access[2];
	
	int Seek88(int drvno, int trackno, int sectno);
	unsigned char Getc88(int drvno);
	int Putc88(int drvno, unsigned char dat);
	
	// data buffer (256BytesX4)
	unsigned char Data[4][256];
	int Index[4];
	
	typedef struct {
		unsigned char Data[10];
		int Index;
	} CmdBuffer;
	
	CmdBuffer CmdIn;					// command buffer
	CmdBuffer CmdOut;				// status buffer
	unsigned char SeekST0;			// ST0 when SEEK
	unsigned char LastCylinder;		// last read cylinder
	int SeekEnd;						// complete seek flag
	unsigned char SendSectors;		// amount(100H unit)
	int DIO;							// data direction TRUE: Buffer->CPU FALSE: CPU->Buffer
	unsigned char Status;			// FDC status register
	
	void Push(int part, unsigned char data);
	unsigned char Pop(int part);
	void Clear(int i);
	
	int DiskInit66(void);
	void PushStatus(int data);
	unsigned char PopStatus();
	void OutFDC(unsigned char data);
	unsigned char InFDC();
	void Read();
	void Write(void);
	void Seek(void);
	void SenseInterruptStatus(void);
	void Exec();
	
	void OutB1H_66(unsigned char data);
	void OutB2H_66(unsigned char data);
	void OutB3H_66(unsigned char data);
	void OutD0H_66(unsigned char data);
	void OutD1H_66(unsigned char data);
	void OutD2H_66(unsigned char data);
	void OutD3H_66(unsigned char data);
	void OutD6H_66(unsigned char data);
	void OutD8H_66(unsigned char data);
	void OutDAH_66(unsigned char data);
	void OutDDH_66(unsigned char data);
	void OutDEH_66(unsigned char data);
	
	unsigned char InB2H_66();
	unsigned char InD0H_66();
	unsigned char InD1H_66();
	unsigned char InD2H_66();
	unsigned char InD3H_66();
	unsigned char InD4H_66();
	unsigned char InDCH_66();
	unsigned char InDDH_66();
	
public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_noise_seek = NULL;
//		d_noise_head_down = NULL;
//		d_noise_head_up = NULL;
		set_device_name(_T("Floppy Drive"));
	}
	~FLOPPY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	uint32_t read_signal(int ch);
	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_ext(DEVICE* device)
	{
		d_ext = device;
	}
	void set_context_noise_seek(NOISE* device)
	{
		d_noise_seek = device;
	}
	NOISE* get_context_noise_seek()
	{
		return d_noise_seek;
	}
//	void set_context_noise_head_down(NOISE* device)
//	{
//		d_noise_head_down = device;
//	}
//	NOISE* get_context_noise_head_down()
//	{
//		return d_noise_head_down;
//	}
//	void set_context_noise_head_up(NOISE* device)
//	{
//		d_noise_head_up = device;
//	}
//	NOISE* get_context_noise_head_up()
//	{
//		return d_noise_head_up;
//	}
	DISK* get_disk_handler(int drv)
	{
		return disk[drv];
	}
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool is_disk_inserted(int drv);
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);
};

#endif

