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
	uint8 idm_or, idm_ar0, idm_ar1;
	uint8 regs[0x10000];
	bool is_tcp[4];
	uint16 rx_bufsz[4], tx_bufsz[4];
	uint32 cx_rw_pr[4], cx_rr_pr[4];
	uint32 cx_ta_pr[4], cx_tw_pr[4], cx_tr_pr[4];
	uint32 send_dst_ptr[4], recv_dst_ptr[4];
	
	void process_cmd(uint16 raddr, uint8 data);
	void process_status(uint16 addr);
	
public:
	W3100A(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~W3100A() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void connected(int ch);
	void disconnected(int ch);
	uint8* get_sendbuffer(int ch, int* size);
	void inc_sendbuffer_ptr(int ch, int size);
	uint8* get_recvbuffer0(int ch, int* size0, int* size1);
	uint8* get_recvbuffer1(int ch);
	void inc_recvbuffer_ptr(int ch, int size);
};

#endif
