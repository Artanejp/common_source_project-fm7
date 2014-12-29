/*
	Skelton for retropc emulator

	Origin : MAME HD63484
	Author : Takeda.Toshiya
	Date   : 2009.02.09 -

	[ HD63484 ]
*/

#ifndef _HD63484_H_
#define _HD63484_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class HD63484 : public DEVICE
{
private:
	// vram
	uint16* vram;
	uint32 vram_size;
	
	// fifo
	int fifo_ptr;
	uint16 fifo[256], readfifo;
	
	// params
	int ch, vpos;
	uint16 regs[128], pattern[16];
	int org, org_dpd, rwp;
	uint16 cl0, cl1, ccmp, edg, mask;
	uint16 ppy, pzcy, ppx, pzcx, psy, psx, pey, pzy, pex, pzx;
	uint16 xmin, ymin, xmax, ymax, rwp_dn;
	int16 cpx, cpy;
	
	void process_cmd();
	void doclr16(int opcode, uint16 fill, int *dst, int _ax, int _ay);
	void docpy16(int opcode, int src, int *dst, int _ax, int _ay);
	int org_first_pixel(int _org_dpd);
	void dot(int x, int y, int opm, uint16 color);
	int get_pixel(int x, int y);
	int get_pixel_ptn(int x, int y);
	void agcpy(int opcode, int src_x, int src_y, int dst_x, int dst_y, int16 _ax, int16 _ay);
	void ptn(int opcode, int src_x, int src_y, int16 _ax, int16 _ay);
	void line(int16 sx, int16 sy, int16 ex, int16 ey, int16 col);
	void paint(int sx, int sy, int col);
	
public:
	HD63484(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~HD63484() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_io16(uint32 addr, uint32 data);
	uint32 read_io16(uint32 addr);
	void event_vline(int v, int clock);
	
	// unique functions
	void set_vram_ptr(uint16* ptr, uint32 size)
	{
		vram = ptr; vram_size = size;
	}
};

#endif
