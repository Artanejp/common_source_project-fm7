#pragma once

#include "../types/scrntype_t.h"

// ToDo: for MSVC, without C++11.
typedef	 union {
	scrntype_t w[8];
} scrntype_vec8_t;
typedef	union {
	scrntype_t w[16];
} scrntype_vec16_t;

typedef  union {
	uint8_t w[8];
} uint8_vec8_t;

typedef union {
	uint16_t w[8];
} uint16_vec8_t;

typedef union {
	uint32_t w[8];
} uint32_vec8_t;

typedef struct {
	uint16_vec8_t plane_table[256];
} _bit_trans_table_t;

typedef struct {
	scrntype_vec8_t plane_table[256];
} _bit_trans_table_scrn_t;

typedef struct {
	scrntype_t* palette; // Must be 2^planes entries. If NULL, assume RGB.
	_bit_trans_table_t* bit_trans_table[16]; // Must be exist >= planes. Must be aligned with sizeof(uint16_vec8_t).
	int xzoom; // 1 - 4?
	bool is_render[16];
	int shift;
	uint8_t* data[16];
	uint32_t baseaddress[16];
	uint32_t voffset[16];
	uint32_t addrmask;  // For global increment.
	uint32_t addrmask2; // For local increment.
	uint32_t begin_pos;
	uint32_t render_width;
} _render_command_data_t;
