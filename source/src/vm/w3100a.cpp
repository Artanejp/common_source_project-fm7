/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ WIZnet W3100A ]
*/

#include "w3100a.h"
#include "../fileio.h"

void W3100A::initialize()
{
	idm_or = idm_ar0 = idm_ar1 = 0;
	memset(regs, 0, sizeof(regs));
	memset(cx_rw_pr, 0, sizeof(cx_rw_pr));
	memset(cx_rr_pr, 0, sizeof(cx_rr_pr));
	memset(cx_ta_pr, 0, sizeof(cx_ta_pr));
	memset(cx_tw_pr, 0, sizeof(cx_tw_pr));
	memset(cx_tr_pr, 0, sizeof(cx_tr_pr));
}

#define GET_ADDR() { \
	if(idm_or & 2) { \
		raddr = idm_ar0 | (idm_ar1 << 8); \
	} else { \
		raddr = (idm_ar0 << 8) | idm_ar1; \
	} \
}

#define INC_ADDR() { \
	if(idm_or & 1) { \
		raddr++; \
		idm_ar0 = (idm_or & 2) ? (raddr & 0xff) : (raddr >> 8); \
		idm_ar1 = (idm_or & 2) ? (raddr >> 8) : (raddr & 0xff); \
	} \
}

void W3100A::write_io8(uint32 addr, uint32 data)
{
	uint16 raddr;
	
	switch(addr & 3) {
	case 0:
		idm_or = data;
		break;
	case 1:
		idm_ar0 = data;
		break;
	case 2:
		idm_ar1 = data;
		break;
	case 3:
		GET_ADDR();
		regs[raddr] = data;
		process_cmd(raddr, data);
		INC_ADDR();
		break;
	}
}

uint32 W3100A::read_io8(uint32 addr)
{
	uint16 raddr;
	uint32 val;
	
	switch(addr & 3) {
	case 3:
		GET_ADDR();
		val = regs[raddr];
		process_status(raddr);
		INC_ADDR();
		return val;
	}
	return 0xff;
}

void W3100A::process_cmd(uint16 raddr, uint8 data)
{
	uint16 bufsz[4] = { 1024, 2048, 4096, 8192 };
	int ch;
	
	switch(raddr) {
	case 0x00:	// c0_cr
		// sys init (ch.0 only)
		if(data & 1) {
			regs[4] = 1;	// always success
		}
	case 0x01:	// c1_cr
	case 0x02:	// c2_cr
	case 0x03:	// c3_cr
		ch = raddr & 3;
		if(data & 2) {
			// init sock
			uint8 mode = regs[0xa1 + 24 * ch] & 7;
			if(mode == 0) {
				emu->disconnect_socket(ch);
			} else {
				bool result = false, connect = false;
				regs[0xa0 + 24 * ch] = 0;	// SOCK_CLOSE
				if(mode == 1) {
					result = connect = emu->init_socket_tcp(ch);
					is_tcp[ch] = true;
				} else if(mode == 2) {
					result = connect = emu->init_socket_udp(ch);
					if(result) {
						regs[0xa0 + 24 * ch] = 0xf;	// SOCK_UDP
					}
					is_tcp[ch] = false;
				}
				regs[4 + ch] &= ~(2 | 4 | 8);
				regs[4 + ch] |= (result ? 2 : 0) | (connect ? 0 : 8);
				// patch to force buffer is empty
				cx_ta_pr[ch] = cx_tr_pr[ch] = cx_tw_pr[ch];
				cx_rw_pr[ch] = cx_rr_pr[ch] = 0;
			}
		}
		if(data & 4) {
			// connect
			regs[0xa0 + 24 * ch] = 3;	// SOCK_SYNSENT
			uint32 ipaddr = regs[0xa8 + 24 * ch];
			ipaddr |= regs[0xa9 + 24 * ch] << 8;
			ipaddr |= regs[0xaa + 24 * ch] << 16;
			ipaddr |= regs[0xab + 24 * ch] << 24;
			int port = (regs[0xac + 24 * ch] << 8) | regs[0xad + 24 * ch];
			if(!emu->connect_socket(ch, ipaddr, port)) {
				regs[4 + ch] &= ~4;		// established = false
				regs[4 + ch] |= 8;		// closed = true
				regs[0xa0 + 24 * ch] = 0;	// SOCK_CLOSED
			}
		}
		if(data & 8) {
			// listen
			regs[0xa0 + 24 * ch] = 2;	// SOCK_LISTEN
			if(!emu->listen_socket(ch)) {
				regs[4 + ch] &= ~4;		// established = false
				regs[4 + ch] |= 8;		// closed = true
				regs[0xa0 + 24 * ch] = 0;	// SOCK_CLOSED
			}
		}
		if(data & 0x10) {
			// close
			emu->disconnect_socket(ch);
		}
		if(data & 0x20) {
			// send
			send_dst_ptr[ch] = cx_tw_pr[ch];
			uint32 rd_ptr = is_tcp[ch] ? cx_ta_pr[ch] : cx_tr_pr[ch];
			if(rd_ptr != send_dst_ptr[ch]) {
				regs[ch + 4] &= ~0x20;
				if(is_tcp[ch]) {
					emu->send_data_tcp(ch);
				} else {
					uint32 ipaddr = regs[0xa8 + 24 * ch];
					ipaddr |= regs[0xa9 + 24 * ch] << 8;
					ipaddr |= regs[0xaa + 24 * ch] << 16;
					ipaddr |= regs[0xab + 24 * ch] << 24;
					int port = (regs[0xac + 24 * ch] << 8) | regs[0xad + 24 * ch];
					emu->send_data_udp(ch, ipaddr, port);
				}
			} else {
				regs[ch + 4] |= 0x20;	// send ok
			}
		}
		if(data & 0x40) {
			// recv
			recv_dst_ptr[ch] = cx_rr_pr[ch];
			regs[ch + 4] &= ~0x40;	// recv ok
		}
		break;
	case 0x10: case 0x11: case 0x12: case 0x13: ch = 0; goto lbl_cx_rw_pr;
	case 0x1c: case 0x1d: case 0x1e: case 0x1f: ch = 1; goto lbl_cx_rw_pr;
	case 0x28: case 0x29: case 0x2a: case 0x2b: ch = 2; goto lbl_cx_rw_pr;
	case 0x34: case 0x35: case 0x36: case 0x37: ch = 3;
lbl_cx_rw_pr:
		cx_rw_pr[ch]  = regs[0x10 + 12 * ch] << 24;
		cx_rw_pr[ch] |= regs[0x11 + 12 * ch] << 16;
		cx_rw_pr[ch] |= regs[0x12 + 12 * ch] <<  8;
		cx_rw_pr[ch] |= regs[0x13 + 12 * ch] <<  0;
		break;
	case 0x14: case 0x15: case 0x16: case 0x17: ch = 0; goto lbl_cx_rr_pr;
	case 0x20: case 0x21: case 0x22: case 0x23: ch = 1; goto lbl_cx_rr_pr;
	case 0x2c: case 0x2d: case 0x2e: case 0x2f: ch = 2; goto lbl_cx_rr_pr;
	case 0x38: case 0x39: case 0x3a: case 0x3b: ch = 3;
lbl_cx_rr_pr:
		cx_rr_pr[ch]  = regs[0x14 + 12 * ch] << 24;
		cx_rr_pr[ch] |= regs[0x15 + 12 * ch] << 16;
		cx_rr_pr[ch] |= regs[0x16 + 12 * ch] <<  8;
		cx_rr_pr[ch] |= regs[0x17 + 12 * ch] <<  0;
		break;
	case 0x18: case 0x19: case 0x1a: case 0x1b: ch = 0; goto lbl_cx_ta_pr;
	case 0x24: case 0x25: case 0x26: case 0x27: ch = 1; goto lbl_cx_ta_pr;
	case 0x30: case 0x31: case 0x32: case 0x33: ch = 2; goto lbl_cx_ta_pr;
	case 0x3c: case 0x3d: case 0x3e: case 0x3f: ch = 3;
lbl_cx_ta_pr:
		cx_ta_pr[ch]  = regs[0x18 + 12 * ch] << 24;
		cx_ta_pr[ch] |= regs[0x19 + 12 * ch] << 16;
		cx_ta_pr[ch] |= regs[0x1a + 12 * ch] <<  8;
		cx_ta_pr[ch] |= regs[0x1b + 12 * ch] <<  0;
		break;
	case 0x40: case 0x41: case 0x42: case 0x43: ch = 0; goto lbl_cx_tw_pr;
	case 0x4c: case 0x4d: case 0x4e: case 0x4f: ch = 1; goto lbl_cx_tw_pr;
	case 0x58: case 0x59: case 0x5a: case 0x5b: ch = 2; goto lbl_cx_tw_pr;
	case 0x64: case 0x65: case 0x66: case 0x67: ch = 3;
lbl_cx_tw_pr:
		cx_tw_pr[ch]  = regs[0x40 + 12 * ch] << 24;
		cx_tw_pr[ch] |= regs[0x41 + 12 * ch] << 16;
		cx_tw_pr[ch] |= regs[0x42 + 12 * ch] <<  8;
		cx_tw_pr[ch] |= regs[0x43 + 12 * ch] <<  0;
		break;
	case 0x44: case 0x45: case 0x46: case 0x47: ch = 0; goto lbl_cx_tr_pr;
	case 0x50: case 0x51: case 0x52: case 0x53: ch = 1; goto lbl_cx_tr_pr;
	case 0x5c: case 0x5d: case 0x5e: case 0x5f: ch = 2; goto lbl_cx_tr_pr;
	case 0x68: case 0x69: case 0x6a: case 0x6b: ch = 3;
lbl_cx_tr_pr:
		cx_tr_pr[ch]  = regs[0x44 + 12 * ch] << 24;
		cx_tr_pr[ch] |= regs[0x45 + 12 * ch] << 16;
		cx_tr_pr[ch] |= regs[0x46 + 12 * ch] <<  8;
		cx_tr_pr[ch] |= regs[0x47 + 12 * ch] <<  0;
		break;
	case 0x95:	// rmsr
		rx_bufsz[0] = bufsz[(data >> 0) & 3];
		rx_bufsz[1] = bufsz[(data >> 2) & 3];
		rx_bufsz[2] = bufsz[(data >> 4) & 3];
		rx_bufsz[3] = bufsz[(data >> 6) & 3];
		break;
	case 0x96:	// tmsr
		tx_bufsz[0] = bufsz[(data >> 0) & 3];
		tx_bufsz[1] = bufsz[(data >> 2) & 3];
		tx_bufsz[2] = bufsz[(data >> 4) & 3];
		tx_bufsz[3] = bufsz[(data >> 6) & 3];
		break;
	}
}

void W3100A::process_status(uint16 raddr)
{
	int ch;
	
	switch(raddr) {
	case 0x1e0: ch = 0; goto lbl_cx_srw_pr;
	case 0x1e3: ch = 1; goto lbl_cx_srw_pr;
	case 0x1e6: ch = 2; goto lbl_cx_srw_pr;
	case 0x1e9: ch = 3;
lbl_cx_srw_pr:
		regs[0x10 + 12 * ch] = cx_rw_pr[ch] >> 24;
		regs[0x11 + 12 * ch] = cx_rw_pr[ch] >> 16;
		regs[0x12 + 12 * ch] = cx_rw_pr[ch] >>  8;
		regs[0x13 + 12 * ch] = cx_rw_pr[ch] >>  0;
		break;
	case 0x1e1: ch = 0; goto lbl_cx_srr_pr;
	case 0x1e4: ch = 1; goto lbl_cx_srr_pr;
	case 0x1e7: ch = 2; goto lbl_cx_srr_pr;
	case 0x1ea: ch = 3;
lbl_cx_srr_pr:
		regs[0x14 + 12 * ch] = cx_rr_pr[ch] >> 24;
		regs[0x15 + 12 * ch] = cx_rr_pr[ch] >> 16;
		regs[0x16 + 12 * ch] = cx_rr_pr[ch] >>  8;
		regs[0x17 + 12 * ch] = cx_rr_pr[ch] >>  0;
		break;
	case 0x1e2: ch = 0; goto lbl_cx_sta_pr;
	case 0x1e5: ch = 1; goto lbl_cx_sta_pr;
	case 0x1e8: ch = 2; goto lbl_cx_sta_pr;
	case 0x1eb: ch = 3;
lbl_cx_sta_pr:
		regs[0x18 + 12 * ch] = cx_ta_pr[ch] >> 24;
		regs[0x19 + 12 * ch] = cx_ta_pr[ch] >> 16;
		regs[0x1a + 12 * ch] = cx_ta_pr[ch] >>  8;
		regs[0x1b + 12 * ch] = cx_ta_pr[ch] >>  0;
		break;
	case 0x1f0: ch = 0; goto lbl_cx_stw_pr;
	case 0x1f3: ch = 1; goto lbl_cx_stw_pr;
	case 0x1f6: ch = 2; goto lbl_cx_stw_pr;
	case 0x1f9: ch = 3;
lbl_cx_stw_pr:
		regs[0x40 + 12 * ch] = cx_tw_pr[ch] >> 24;
		regs[0x41 + 12 * ch] = cx_tw_pr[ch] >> 16;
		regs[0x42 + 12 * ch] = cx_tw_pr[ch] >>  8;
		regs[0x43 + 12 * ch] = cx_tw_pr[ch] >>  0;
		break;
	case 0x1f1: ch = 0; goto lbl_cx_str_pr;
	case 0x1f4: ch = 1; goto lbl_cx_str_pr;
	case 0x1f7: ch = 2; goto lbl_cx_str_pr;
	case 0x1fa: ch = 3;
lbl_cx_str_pr:
		regs[0x44 + 12 * ch] = cx_tr_pr[ch] >> 24;
		regs[0x45 + 12 * ch] = cx_tr_pr[ch] >> 16;
		regs[0x46 + 12 * ch] = cx_tr_pr[ch] >>  8;
		regs[0x47 + 12 * ch] = cx_tr_pr[ch] >>  0;
		break;
	}
}

void W3100A::connected(int ch)
{
	regs[4 + ch] |= 4;		// established = true
	regs[4 + ch] &= ~8;		// closed = false
	regs[0xa0 + 24 * ch] = 6;	// SOCK_ESTABLISHED
}

void W3100A::disconnected(int ch)
{
	regs[4 + ch] &= ~4;		// established = false
	regs[4 + ch] |= 8;		// closed = true
	if(regs[ch] & 0x40) {		// recv_ok = true
		regs[4 + ch] |= 0x40;
	}
	regs[0xa0 + 24 * ch] = 0;	// SOCK_CLOSED
}

uint8* W3100A::get_sendbuffer(int ch, int* size)
{
	uint32 cx_ta_tr = is_tcp[ch] ? cx_ta_pr[ch] : cx_tr_pr[ch];
	uint32 rd_ptr = cx_ta_tr & (tx_bufsz[ch] - 1);
	uint32 wr_ptr = send_dst_ptr[ch] & (tx_bufsz[ch] - 1);
	
	if(!(regs[ch] & 0x20)) {
		*size = 0;
	} else if(rd_ptr == wr_ptr && cx_ta_tr != send_dst_ptr[ch]) {
		*size = 0;
	} else if(rd_ptr <= wr_ptr) {
		*size = wr_ptr - rd_ptr;
	} else {
		*size = tx_bufsz[ch] - rd_ptr;
	}
	uint32 ofs = 0x4000;
	for(int i = 0; i < ch; i++) {
		ofs += tx_bufsz[i];
	}
	return &regs[ofs + rd_ptr];
}

void W3100A::inc_sendbuffer_ptr(int ch, int size)
{
	if(is_tcp[ch]) {
		cx_ta_pr[ch] += size;
		if(cx_ta_pr[ch] == send_dst_ptr[ch]) {
			regs[ch + 4] |= 0x20;	// send ok
		}
	} else {
		cx_tr_pr[ch] += size;
		if(cx_tr_pr[ch] == send_dst_ptr[ch]) {
			regs[ch + 4] |= 0x20;	// send ok
		}
	}
}

uint8* W3100A::get_recvbuffer0(int ch, int* size0, int* size1)
{
	uint32 wr_ptr = cx_rw_pr[ch] & (rx_bufsz[ch] - 1);
	uint32 rr_ptr = recv_dst_ptr[ch] & (rx_bufsz[ch] - 1);
	
	if(!(regs[ch] & 0x40)) {
		*size0 = *size1 = 0;
	} else if(rr_ptr == wr_ptr && cx_rw_pr[ch] != recv_dst_ptr[ch]) {
		*size0 = *size1 = 0;
	} else if(rr_ptr <= wr_ptr) {
		*size0 = rx_bufsz[ch] - wr_ptr;
		*size1 = rr_ptr;
	} else {
		*size0 = rr_ptr - wr_ptr;
		*size1 = 0;
	}
	uint32 ofs = 0x6000;
	for(int i = 0; i < ch; i++) {
		ofs += rx_bufsz[i];
	}
	return &regs[ofs + wr_ptr];
}

uint8* W3100A::get_recvbuffer1(int ch)
{
	uint32 ofs = 0x6000;
	for(int i = 0; i < ch; i++) {
		ofs += rx_bufsz[i];
	}
	return &regs[ofs];
}

void W3100A::inc_recvbuffer_ptr(int ch, int size)
{
//	uint32 wr_ptr = cx_rw_pr[ch];
//	wr_ptr = (wr_ptr + size) & (rx_bufsz[ch] - 1);
//	cx_rw_pr[ch] = (cx_rw_pr[ch] & ~(rx_bufsz[ch] - 1)) | wr_ptr;
	cx_rw_pr[ch] += size;
//	regs[ch + 4] |= 0x40;	// recv ok
}

#define STATE_VERSION	1

void W3100A::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(idm_or);
	state_fio->FputUint8(idm_ar0);
	state_fio->FputUint8(idm_ar1);
	state_fio->Fwrite(regs, sizeof(regs), 1);
	state_fio->Fwrite(is_tcp, sizeof(is_tcp), 1);
	state_fio->Fwrite(rx_bufsz, sizeof(rx_bufsz), 1);
	state_fio->Fwrite(tx_bufsz, sizeof(tx_bufsz), 1);
	state_fio->Fwrite(cx_rw_pr, sizeof(cx_rw_pr), 1);
	state_fio->Fwrite(cx_rr_pr, sizeof(cx_rr_pr), 1);
	state_fio->Fwrite(cx_ta_pr, sizeof(cx_ta_pr), 1);
	state_fio->Fwrite(cx_tw_pr, sizeof(cx_tw_pr), 1);
	state_fio->Fwrite(cx_tr_pr, sizeof(cx_tr_pr), 1);
	state_fio->Fwrite(send_dst_ptr, sizeof(send_dst_ptr), 1);
	state_fio->Fwrite(recv_dst_ptr, sizeof(recv_dst_ptr), 1);
}

bool W3100A::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	idm_or = state_fio->FgetUint8();
	idm_ar0 = state_fio->FgetUint8();
	idm_ar1 = state_fio->FgetUint8();
	state_fio->Fread(regs, sizeof(regs), 1);
	state_fio->Fread(is_tcp, sizeof(is_tcp), 1);
	state_fio->Fread(rx_bufsz, sizeof(rx_bufsz), 1);
	state_fio->Fread(tx_bufsz, sizeof(tx_bufsz), 1);
	state_fio->Fread(cx_rw_pr, sizeof(cx_rw_pr), 1);
	state_fio->Fread(cx_rr_pr, sizeof(cx_rr_pr), 1);
	state_fio->Fread(cx_ta_pr, sizeof(cx_ta_pr), 1);
	state_fio->Fread(cx_tw_pr, sizeof(cx_tw_pr), 1);
	state_fio->Fread(cx_tr_pr, sizeof(cx_tr_pr), 1);
	state_fio->Fread(send_dst_ptr, sizeof(send_dst_ptr), 1);
	state_fio->Fread(recv_dst_ptr, sizeof(recv_dst_ptr), 1);
	return true;
}

