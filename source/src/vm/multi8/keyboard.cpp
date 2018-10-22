/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.15 -

	[ keyboard ]
*/

#include "keyboard.h"

// f9 (78)=hard copy
// f10(79)=break
// f11(7a)=erase line
namespace MULTI8 {

static const uint8_t matrix_normal[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x08,0x09,0x00,0x00, 0x00,0x0d,0x00,0x00,	// 00
	0x00,0x00,0x00,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x1b, 0x00,0x00,0x00,0x00,	// 10
	0x20,0x00,0x00,0x00, 0x0b,0x1d,0x1e,0x1c, 0x1f,0x00,0x00,0x00, 0x00,0x12,0x7f,0x00,	// 20
	0x30,0x31,0x32,0x33, 0x34,0x35,0x36,0x37, 0x38,0x39,0x00,0x00, 0x00,0x00,0x00,0x00,	// 30
	0x00,0x41,0x42,0x43, 0x44,0x45,0x46,0x47, 0x48,0x49,0x4a,0x4b, 0x4c,0x4d,0x4e,0x4f,	// 40
	0x50,0x51,0x52,0x53, 0x54,0x55,0x56,0x57, 0x58,0x59,0x5a,0x00, 0x00,0x00,0x00,0x00,	// 50
	0x30,0x31,0x32,0x33, 0x34,0x35,0x36,0x37, 0x38,0x39,0x00,0x2b, 0x00,0x00,0x2e,0x00,	// 60
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x10,0x03,0x15,0x00, 0x00,0x00,0x00,0x00,	// 70
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 80
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 90
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// a0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x3a,0x3b, 0x2c,0x2d,0x2e,0x2f,	// b0
	0x40,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// c0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x5b, 0x5c,0x5d,0x5e,0x00,	// d0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// e0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00	// f0
};
static const uint8_t matrix_shift[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x08,0x09,0x00,0x00, 0x00,0x0d,0x00,0x00,	// 00
	0x00,0x00,0x00,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x1b, 0x00,0x00,0x00,0x00,	// 10
	0x20,0x00,0x00,0x00, 0x0b,0x1d,0x1e,0x1c, 0x1f,0x00,0x00,0x00, 0x00,0x12,0x7f,0x00,	// 20
	0x00,0x21,0x22,0x23, 0x24,0x25,0x26,0x27, 0x28,0x29,0x00,0x00, 0x00,0x00,0x00,0x00,	// 30
	0x00,0x61,0x62,0x63, 0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b, 0x6c,0x6d,0x6e,0x6f,	// 40
	0x70,0x71,0x72,0x73, 0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0x00, 0x00,0x00,0x00,0x00,	// 50
	0x30,0x31,0x32,0x33, 0x34,0x35,0x36,0x37, 0x38,0x39,0x00,0x2b, 0x00,0x00,0x2e,0x00,	// 60
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x10,0x03,0x15,0x00, 0x00,0x00,0x00,0x00,	// 70
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 80
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 90
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// a0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x2a,0x2b, 0x3c,0x3d,0x3e,0x3f,	// b0
	0x60,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// c0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x7b, 0x7c,0x7d,0x7e,0x00,	// d0
	0x00,0x00,0x5f,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// e0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00	// f0
};
static const uint8_t matrix_ctrl[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 00
	0x00,0x00,0x00,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 10
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 20
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 30
	0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07, 0x08,0x09,0x0a,0x0b, 0x0c,0x0d,0x0e,0x0f,	// 40
	0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17, 0x18,0x19,0x1a,0x00, 0x00,0x00,0x00,0x00,	// 50
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 60
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x10,0x03,0x15,0x00, 0x00,0x00,0x00,0x00,	// 70
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 80
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 90
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// a0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// b0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// c0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// d0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// e0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00	// f0
};
static const uint8_t matrix_graph[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x08,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,	// 00
	0x00,0x00,0x00,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x1b, 0x00,0x00,0x00,0x00,	// 10
	0x20,0x00,0x00,0x00, 0x0b,0x1d,0x1e,0x1c, 0x1f,0x00,0x00,0x00, 0x00,0x12,0x7f,0x00,	// 20
	0xf7,0xf8,0xf9,0xfa, 0xfb,0xf2,0xf3,0xf4, 0xf5,0xf6,0x00,0x00, 0x00,0x00,0x00,0x00,	// 30
	0x00,0xec,0x84,0x82, 0x9f,0x9d,0xe6,0xe7, 0xf0,0xe8,0xea,0xeb, 0x8e,0x86,0x85,0xe9,	// 40
	0x8d,0xed,0xe4,0x9e, 0xe5,0xef,0x83,0x9c, 0x81,0xee,0x80,0x00, 0x00,0x00,0x00,0x00,	// 50
	0x30,0x31,0x32,0x33, 0x34,0x35,0x36,0x37, 0x38,0x39,0x00,0x2b, 0x00,0x00,0x2e,0x00,	// 60
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x10,0x03,0x15,0x00, 0x00,0x00,0x00,0x00,	// 70
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 80
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 90
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// a0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x94,0x89, 0x87,0x8c,0x88,0x97,	// b0
	0x8a,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// c0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0xe1, 0xf1,0xe2,0x8b,0x00,	// d0
	0x00,0x00,0xe3,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// e0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00	// f0
};
static const uint8_t matrix_kana[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x08,0x09,0x00,0x00, 0x00,0x0d,0x00,0x00,	// 00
	0x00,0x00,0x00,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x1b, 0x00,0x00,0x00,0x00,	// 10
	0x20,0x00,0x00,0x00, 0x0b,0x1d,0x1e,0x1c, 0x1f,0x00,0x00,0x00, 0x00,0x12,0x7f,0x00,	// 20
	0xdc,0xc7,0xcc,0xb1, 0xb3,0xb4,0xb5,0xd4, 0xd5,0xd6,0x00,0x00, 0x00,0x00,0x00,0x00,	// 30
	0x00,0xc1,0xba,0xbf, 0xbc,0xb2,0xca,0xb7, 0xb8,0xc6,0xcf,0xc9, 0xd8,0xd3,0xd0,0xd7,	// 40
	0xbe,0xc0,0xbd,0xc4, 0xb6,0xc5,0xcb,0xc3, 0xbb,0xdd,0xc2,0x00, 0x00,0x00,0x00,0x00,	// 50
	0x30,0x31,0x32,0x33, 0x34,0x35,0x36,0x37, 0x38,0x39,0x00,0x2b, 0x00,0x00,0x2e,0x00,	// 60
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x10,0x03,0x15,0x00, 0x00,0x00,0x00,0x00,	// 70
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 80
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 90
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// a0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0xb9,0xda, 0xc8,0xce,0xd9,0xd2,	// b0
	0xde,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// c0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0xdf, 0xb0,0xd1,0xcd,0x00,	// d0
	0x00,0x00,0xdb,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// e0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00	// f0
};
static const uint8_t matrix_shiftkana[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 00
	0x00,0x00,0x00,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 10
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 20
	0xa6,0x00,0x00,0xa7, 0xa9,0xaa,0xab,0xac, 0xad,0xae,0x00,0x00, 0x00,0x00,0x00,0x00,	// 30
	0x00,0x00,0x00,0x00, 0x00,0xa8,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 40
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0xaf,0x00, 0x00,0x00,0x00,0x00,	// 50
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 60
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x10,0x03,0x15,0x00, 0x00,0x00,0x00,0x00,	// 70
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 80
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// 90
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// a0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0xa4,0x00,0xa1,0xa5,	// b0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// c0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0xa2, 0x00,0xa3,0x00,0x00,	// d0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,	// e0
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00	// f0
};

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
	register_frame_event(this);
}

void KEYBOARD::reset()
{
	caps = caps_prev = false;
	graph = graph_prev = false;
	kana = kana_prev = false;
	init = 1;
	code = code_prev = 0;
	stat = 0x8;
}

void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x00:
		break;
	case 0x01:
		break;
	}
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0x00:
		if(init == 1) {
			init = 2;
			return 3;
		}
		if(code) {
			code_prev = code;
		}
		stat &= 0xfe;
		return code_prev;
	case 0x01:
		if(init == 1) {
			return 1;
		} else if(init == 2) {
			init = 3;
			return 1;
		} else if(init == 3) {
			init = 0;
			return 0;
		}
		return stat;
	}
	return 0xff;
}

void KEYBOARD::event_frame()
{
	bool shift = (key_stat[0x10] != 0);
	bool ctrl = (key_stat[0x11] != 0);
	caps = (key_stat[0x14] && !caps_prev) ? !caps : caps;
	graph = (key_stat[0x12] && !graph_prev) ? !graph : graph;
	kana = (key_stat[0x15] && !kana_prev) ? !kana : kana;
	bool function = false;
	
	caps_prev = (key_stat[0x14] != 0);
	graph_prev = (key_stat[0x12] != 0);
	kana_prev = (key_stat[0x15] != 0);
	
	uint8_t next_stat, next_code = 0;
	
	if(key_stat[0x70]) {
		next_code = 0;
		function = true;
	} else if(key_stat[0x71]) {
		next_code = 1;
		function = true;
	} else if(key_stat[0x72]) {
		next_code = 2;
		function = true;
	} else if(key_stat[0x73]) {
		next_code = 3;
		function = true;
	} else {
		const uint8_t* matrix = matrix_normal;
		if(ctrl) {
			matrix = matrix_ctrl;
		} else if(graph) {
			matrix = matrix_graph;
		} else if(kana && shift) {
			matrix = matrix_shiftkana;
		} else if(kana && !shift) {
			matrix = matrix_kana;
		} else if(shift) {
			matrix = matrix_shift;
		}
		for(int i = 0; i < 256; i++) {
			if(key_stat[i]) {
				next_code = matrix[i];
			}
			if(next_code) {
				break;
			}
		}
		if(caps) {
			if('a' <= next_code && next_code <= 'z') {
				next_code -= 0x20;
			} else if('A' <= next_code && next_code <= 'Z') {
				next_code += 0x20;
			}
		}
	}
	bool press = (next_code || function);
	next_stat = (shift ? 0x80 : 0) | (function ? 0x40 : 0) | (press ? 0 : 0x08);
	
	if(next_code != code && press) {
		next_stat |= 0x01;
	}
	code = next_code;
	stat = next_stat;
}

#define STATE_VERSION	1

bool KEYBOARD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateInt32(init);
	state_fio->StateUint8(code);
	state_fio->StateUint8(code_prev);
	state_fio->StateUint8(stat);
	state_fio->StateBool(caps);
	state_fio->StateBool(caps_prev);
	state_fio->StateBool(graph);
	state_fio->StateBool(graph_prev);
	state_fio->StateBool(kana);
	state_fio->StateBool(kana_prev);
	return true;
}

}
