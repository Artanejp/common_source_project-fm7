/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2014.09.02-

	[ debugger ]
*/

#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#ifdef USE_DEBUGGER

#define MAX_BREAK_POINTS	8

typedef struct {
	struct {
		uint32 addr, mask;
		int status;	// 0 = none, 1 = enabled, other = disabled
	} table[MAX_BREAK_POINTS], stored[MAX_BREAK_POINTS];
	bool hit;
	uint32 hit_addr;
} break_point_t;

class DEBUGGER : public DEVICE
{
private:
	DEVICE *d_mem, *d_io;
	
	void check_mem_break_points(break_point_t *bp, uint32 addr, int length)
	{
		for(int i = 0; i < MAX_BREAK_POINTS; i++) {
			if(bp->table[i].status == 1) {
				if(addr >= bp->table[i].addr && addr < bp->table[i].addr + length) {
					bp->hit = now_suspended = true;
					bp->hit_addr = bp->table[i].addr;
					break;
				}
			}
		}
	}
	void check_io_break_points(break_point_t *bp, uint32 addr)
	{
		for(int i = 0; i < MAX_BREAK_POINTS; i++) {
			if(bp->table[i].status == 1) {
				if((addr & bp->table[i].mask) == (bp->table[i].addr & bp->table[i].mask)) {
					bp->hit = now_suspended = true;
					bp->hit_addr = addr;
					break;
				}
			}
		}
	}
public:
	DEBUGGER(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(&bp, 0, sizeof(bp));
		memset(&rbp, 0, sizeof(rbp));
		memset(&wbp, 0, sizeof(wbp));
		memset(&ibp, 0, sizeof(ibp));
		memset(&obp, 0, sizeof(obp));
		_tcscpy_s(file_path, _MAX_PATH, _T("debug.bin"));
		now_debugging = now_going = now_suspended = false;
	}
	~DEBUGGER() {}
	
	// common functions
	void write_data8(uint32 addr, uint32 data)
	{
		check_mem_break_points(&wbp, addr, 1);
		d_mem->write_data8(addr, data);
	}
	uint32 read_data8(uint32 addr)
	{
		check_mem_break_points(&rbp, addr, 1);
		return d_mem->read_data8(addr);
	}
	void write_data16(uint32 addr, uint32 data)
	{
		check_mem_break_points(&wbp, addr, 2);
		d_mem->write_data16(addr, data);
	}
	uint32 read_data16(uint32 addr)
	{
		check_mem_break_points(&rbp, addr, 2);
		return d_mem->read_data16(addr);
	}
	void write_data32(uint32 addr, uint32 data)
	{
		check_mem_break_points(&wbp, addr, 4);
		d_mem->write_data32(addr, data);
	}
	uint32 read_data32(uint32 addr)
	{
		check_mem_break_points(&rbp, addr, 4);
		return d_mem->read_data32(addr);
	}
	void write_data8w(uint32 addr, uint32 data, int* wait)
	{
		check_mem_break_points(&wbp, addr, 1);
		d_mem->write_data8w(addr, data, wait);
	}
	uint32 read_data8w(uint32 addr, int* wait)
	{
		check_mem_break_points(&rbp, addr, 1);
		return d_mem->read_data8w(addr, wait);
	}
	void write_data16w(uint32 addr, uint32 data, int* wait)
	{
		check_mem_break_points(&wbp, addr, 2);
		d_mem->write_data16w(addr, data, wait);
	}
	uint32 read_data16w(uint32 addr, int* wait)
	{
		check_mem_break_points(&rbp, addr, 2);
		return d_mem->read_data16w(addr, wait);
	}
	void write_data32w(uint32 addr, uint32 data, int* wait)
	{
		check_mem_break_points(&wbp, addr, 4);
		d_mem->write_data32w(addr, data, wait);
	}
	uint32 read_data32w(uint32 addr, int* wait)
	{
		check_mem_break_points(&rbp, addr, 4);
		return d_mem->read_data32w(addr, wait);
	}
	uint32 fetch_op(uint32 addr, int *wait)
	{
		check_mem_break_points(&rbp, addr, 1);
		return d_mem->fetch_op(addr, wait);
	}
	void write_io8(uint32 addr, uint32 data)
	{
		check_io_break_points(&obp, addr);
		d_io->write_io8(addr, data);
	}
	uint32 read_io8(uint32 addr)
	{
		check_io_break_points(&ibp, addr);
		return d_io->read_io8(addr);
	}
	void write_io16(uint32 addr, uint32 data)
	{
		check_io_break_points(&obp, addr);
		d_io->write_io16(addr, data);
	}
	uint32 read_io16(uint32 addr)
	{
		check_io_break_points(&ibp, addr);
		return d_io->read_io16(addr);
	}
	void write_io32(uint32 addr, uint32 data)
	{
		check_io_break_points(&obp, addr);
		d_io->write_io32(addr, data);
	}
	uint32 read_io32(uint32 addr)
	{
		check_io_break_points(&ibp, addr);
		return d_io->read_io32(addr);
	}
	void write_io8w(uint32 addr, uint32 data, int* wait)
	{
		check_io_break_points(&obp, addr);
		d_io->write_io8w(addr, data, wait);
	}
	uint32 read_io8w(uint32 addr, int* wait)
	{
		check_io_break_points(&ibp, addr);
		return d_io->read_io8w(addr, wait);
	}
	void write_io16w(uint32 addr, uint32 data, int* wait)
	{
		check_io_break_points(&obp, addr);
		d_io->write_io16w(addr, data, wait);
	}
	uint32 read_io16w(uint32 addr, int* wait)
	{
		check_io_break_points(&ibp, addr);
		return d_io->read_io16w(addr, wait);
	}
	void write_io32w(uint32 addr, uint32 data, int* wait)
	{
		check_io_break_points(&obp, addr);
		d_io->write_io32w(addr, data, wait);
	}
	uint32 read_io32w(uint32 addr, int* wait)
	{
		check_io_break_points(&ibp, addr);
		return d_io->read_io32w(addr, wait);
	}
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void check_break_points(uint32 addr)
	{
		check_mem_break_points(&bp, addr, 1);
	}
	void store_break_points()
	{
		memcpy( bp.stored,  bp.table, sizeof( bp.table));
		memcpy(rbp.stored, rbp.table, sizeof(rbp.table));
		memcpy(wbp.stored, wbp.table, sizeof(wbp.table));
		memcpy(ibp.stored, ibp.table, sizeof(ibp.table));
		memcpy(obp.stored, obp.table, sizeof(obp.table));
		memset( bp.table, 0, sizeof( bp.table));
		memset(rbp.table, 0, sizeof(rbp.table));
		memset(wbp.table, 0, sizeof(wbp.table));
		memset(ibp.table, 0, sizeof(ibp.table));
		memset(obp.table, 0, sizeof(obp.table));
	}
	void restore_break_points()
	{
		memcpy( bp.table,  bp.stored, sizeof( bp.table));
		memcpy(rbp.table, rbp.stored, sizeof(rbp.table));
		memcpy(wbp.table, wbp.stored, sizeof(wbp.table));
		memcpy(ibp.table, ibp.stored, sizeof(ibp.table));
		memcpy(obp.table, obp.stored, sizeof(obp.table));
	}
	bool hit()
	{
		return (bp.hit || rbp.hit || wbp.hit || ibp.hit || obp.hit);
	}
	break_point_t bp, rbp, wbp, ibp, obp;
	_TCHAR file_path[_MAX_PATH];
	bool now_debugging, now_going, now_suspended;
};

#endif
#endif

