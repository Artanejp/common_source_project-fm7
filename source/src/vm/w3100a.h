/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ WIZnet W3100A ]
*/

#ifndef _W3100A_H_
#define _W3100A_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class W3100A : public DEVICE
{
private:
	uint8_t idm_or, idm_ar0, idm_ar1;
	uint8_t regs[0x10000];
	bool is_tcp[4];
	uint16_t rx_bufsz[4], tx_bufsz[4];
	uint32_t cx_rw_pr[4], cx_rr_pr[4];
	uint32_t cx_ta_pr[4], cx_tw_pr[4], cx_tr_pr[4];
	uint32_t send_dst_ptr[4], recv_dst_ptr[4];
	
	void process_cmd(uint16_t raddr, uint8_t data);
	void process_status(uint16_t addr);
	
public:
	W3100A(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("W3100A TCP/IP"));
	}
	~W3100A() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void notify_connected(int ch);
	void notify_disconnected(int ch);
	uint8_t* get_send_buffer(int ch, int* size);
	void inc_send_buffer_ptr(int ch, int size);
	uint8_t* get_recv_buffer0(int ch, int* size0, int* size1);
	uint8_t* get_recv_buffer1(int ch);
	void inc_recv_buffer_ptr(int ch, int size);
};

#endif
