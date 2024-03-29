/*
	Skelton for retropc emulator

	Origin : MAME HD63484
	Author : Takeda.Toshiya
	Date   : 2009.02.09 -

	[ HD63484 ]
*/

#ifndef _HD63484_H_
#define _HD63484_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

class  DLL_PREFIX HD63484 : public DEVICE
{
private:
	// vram
	uint16_t* vram;
	uint32_t vram_size;

	// fifo
	int fifo_ptr;
	uint16_t fifo[256], readfifo;

	// params
	int ch, vpos;
	uint16_t regs[128], pattern[16];
	int org, org_dpd, rwp;
	uint16_t cl0, cl1, ccmp, edg, mask;
	uint16_t ppy, pzcy, ppx, pzcx, psy, psx, pey, pzy, pex, pzx;
	uint16_t xmin, ymin, xmax, ymax, rwp_dn;
	int16_t cpx, cpy;

	void process_cmd();
	void __FASTCALL doclr16(int opcode, uint16_t fill, int *dst, int _ax, int _ay);
	void __FASTCALL docpy16(int opcode, int src, int *dst, int _ax, int _ay);
	int __FASTCALL org_first_pixel(int _org_dpd);
	void __FASTCALL dot(int x, int y, int opm, uint16_t color);
	int __FASTCALL get_pixel(int x, int y);
	int __FASTCALL get_pixel_ptn(int x, int y);
	void agcpy(int opcode, int src_x, int src_y, int dst_x, int dst_y, int16_t _ax, int16_t _ay);
	void ptn(int opcode, int src_x, int src_y, int16_t _ax, int16_t _ay);
	void line(int16_t sx, int16_t sy, int16_t ex, int16_t ey, int16_t col);
	void paint(int sx, int sy, int col);

public:
	HD63484(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("HD63484 ACRTC"));
	}
	~HD63484() {}

	// common functions
	void initialize() override;
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_io16(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io16(uint32_t addr) override;
	void __FASTCALL write_io16w(uint32_t addr, uint32_t data, int *wait) override;
	uint32_t __FASTCALL read_io16w(uint32_t addr, int *wait) override;
	void event_vline(int v, int clock) override;

	// unique functions
	void set_vram_ptr(uint16_t* ptr, uint32_t size)
	{
		vram = ptr; vram_size = size;
	}
};

#endif
