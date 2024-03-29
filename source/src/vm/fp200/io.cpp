/*
	CASIO FP-200 Emulator 'eFP-200'

	Author : Takeda.Toshiya
	Date   : 2013.03.21-

	[ io ]
*/

#include "io.h"
#include "../datarec.h"
#include "../i8080.h"
#include "../../types/util_sound.h"

namespace FP200 {
	
#define EVENT_CMT_READY	0
#define EVENT_CMT_CLOCK	1

#define CMT_MODE_REC	1
#define CMT_MODE_PLAY	3

#define CMT_RECORDING	(cmt_selected && cmt_mode == CMT_MODE_REC && cmt_rec)
#define CMT_PLAYING	(cmt_selected && cmt_mode == CMT_MODE_PLAY)

#define CMT_SAMPLE_RATE	48000

// based on elisa font (http://hp.vector.co.jp/authors/VA002310/index.htm
static const uint8_t elisa_font[2048] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x08, 0x04, 0x02, 0x7f, 0x02, 0x04, 0x08, 0x00, 0x08, 0x10, 0x20, 0x7f, 0x20, 0x10, 0x08, 0x00,
	0x08, 0x1c, 0x2a, 0x49, 0x08, 0x08, 0x08, 0x00, 0x08, 0x08, 0x08, 0x49, 0x2a, 0x1c, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08,
	0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x12, 0x7f, 0x24, 0x24, 0xfe, 0x48, 0x48,
	0x08, 0x1c, 0x2a, 0x1c, 0x0a, 0x2a, 0x1c, 0x08, 0x61, 0x92, 0x94, 0x68, 0x16, 0x29, 0x49, 0x86,
	0x30, 0x48, 0x48, 0x32, 0x4a, 0x44, 0x3a, 0x00, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x03, 0x04, 0x08, 0x08, 0x08, 0x08, 0x04, 0x03, 0xc0, 0x20, 0x10, 0x10, 0x10, 0x10, 0x20, 0xc0,
	0x08, 0x2a, 0x1c, 0x08, 0x1c, 0x2a, 0x08, 0x00, 0x08, 0x08, 0x08, 0x7f, 0x08, 0x08, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x20, 0x40, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00,
	0x3c, 0x42, 0x46, 0x5a, 0x62, 0x42, 0x3c, 0x00, 0x08, 0x18, 0x08, 0x08, 0x08, 0x08, 0x1c, 0x00,
	0x3c, 0x42, 0x02, 0x0c, 0x10, 0x20, 0x7e, 0x00, 0x3c, 0x42, 0x02, 0x3c, 0x02, 0x42, 0x3c, 0x00,
	0x04, 0x0c, 0x14, 0x24, 0x44, 0x7e, 0x04, 0x00, 0x7e, 0x40, 0x40, 0x7c, 0x02, 0x02, 0x7c, 0x00,
	0x3c, 0x42, 0x40, 0x7c, 0x42, 0x42, 0x3c, 0x00, 0x7e, 0x02, 0x04, 0x08, 0x10, 0x10, 0x10, 0x00,
	0x3c, 0x42, 0x42, 0x3c, 0x42, 0x42, 0x3c, 0x00, 0x3c, 0x42, 0x42, 0x3e, 0x02, 0x42, 0x3c, 0x00,
	0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x08, 0x10,
	0x03, 0x0c, 0x30, 0xc0, 0x30, 0x0c, 0x03, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00,
	0xc0, 0x30, 0x0c, 0x03, 0x0c, 0x30, 0xc0, 0x00, 0x1c, 0x22, 0x22, 0x04, 0x08, 0x08, 0x00, 0x08,
	0x1c, 0x22, 0x4d, 0x55, 0x47, 0x5f, 0x20, 0x1e, 0x18, 0x24, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x00,
	0x7c, 0x42, 0x42, 0x7c, 0x42, 0x42, 0x7c, 0x00, 0x1c, 0x22, 0x40, 0x40, 0x40, 0x22, 0x1c, 0x00,
	0x78, 0x24, 0x22, 0x22, 0x22, 0x24, 0x78, 0x00, 0x7e, 0x40, 0x40, 0x7c, 0x40, 0x40, 0x7e, 0x00,
	0x7e, 0x40, 0x40, 0x7c, 0x40, 0x40, 0x40, 0x00, 0x1c, 0x22, 0x40, 0x47, 0x42, 0x26, 0x1a, 0x00,
	0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x00, 0x1c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1c, 0x00,
	0x0e, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00, 0x42, 0x44, 0x48, 0x50, 0x68, 0x44, 0x42, 0x00,
	0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7e, 0x00, 0x41, 0x63, 0x55, 0x49, 0x41, 0x41, 0x41, 0x00,
	0x42, 0x62, 0x52, 0x4a, 0x46, 0x42, 0x42, 0x00, 0x18, 0x24, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00,
	0x7c, 0x42, 0x42, 0x7c, 0x40, 0x40, 0x40, 0x00, 0x18, 0x24, 0x42, 0x42, 0x4a, 0x24, 0x1a, 0x00,
	0x7c, 0x42, 0x42, 0x7c, 0x48, 0x44, 0x42, 0x00, 0x3c, 0x42, 0x40, 0x3c, 0x02, 0x42, 0x3c, 0x00,
	0x7f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00,
	0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08, 0x00, 0x41, 0x49, 0x49, 0x55, 0x55, 0x22, 0x22, 0x00,
	0x41, 0x22, 0x14, 0x08, 0x14, 0x22, 0x41, 0x00, 0x41, 0x22, 0x14, 0x08, 0x08, 0x08, 0x08, 0x00,
	0x7e, 0x02, 0x04, 0x08, 0x10, 0x20, 0x7e, 0x00, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07,
	0x41, 0x22, 0x14, 0x7f, 0x08, 0x7f, 0x08, 0x08, 0xe0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xe0,
	0x08, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
	0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x02, 0x1e, 0x22, 0x1e, 0x00,
	0x20, 0x20, 0x20, 0x3c, 0x22, 0x22, 0x3c, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x20, 0x22, 0x1c, 0x00,
	0x02, 0x02, 0x02, 0x1e, 0x22, 0x22, 0x1e, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x3e, 0x20, 0x1e, 0x00,
	0x0c, 0x12, 0x10, 0x3c, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x1e, 0x02, 0x3c,
	0x20, 0x20, 0x3c, 0x22, 0x22, 0x22, 0x22, 0x00, 0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x1c, 0x00,
	0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x48, 0x30, 0x20, 0x20, 0x24, 0x28, 0x30, 0x28, 0x24, 0x00,
	0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x3c, 0x2a, 0x2a, 0x2a, 0x2a, 0x00,
	0x00, 0x00, 0x3c, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x1c, 0x00,
	0x00, 0x00, 0x3c, 0x22, 0x22, 0x3c, 0x20, 0x20, 0x00, 0x00, 0x1e, 0x22, 0x22, 0x1e, 0x02, 0x02,
	0x00, 0x00, 0x2e, 0x30, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x1e, 0x20, 0x1c, 0x02, 0x3c, 0x00,
	0x00, 0x10, 0x3c, 0x10, 0x10, 0x10, 0x0c, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x1e, 0x00,
	0x00, 0x00, 0x22, 0x22, 0x14, 0x14, 0x08, 0x00, 0x00, 0x00, 0x22, 0x2a, 0x2a, 0x14, 0x14, 0x00,
	0x00, 0x00, 0x22, 0x14, 0x08, 0x14, 0x22, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x1e, 0x02, 0x3c,
	0x00, 0x00, 0x3e, 0x04, 0x08, 0x10, 0x3e, 0x00, 0x03, 0x04, 0x04, 0x08, 0x04, 0x04, 0x04, 0x03,
	0x08, 0x08, 0x08, 0x00, 0x08, 0x08, 0x08, 0x00, 0xc0, 0x20, 0x20, 0x10, 0x20, 0x20, 0x20, 0xc0,
	0x00, 0x00, 0x30, 0x49, 0x06, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x10, 0x08, 0x08, 0x00, 0x08,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
	0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
	0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0x08, 0x08, 0x08, 0x08, 0xff, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0xf8, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08,
	0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x00, 0x00, 0x00, 0x00, 0x0f, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x0f, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0xf8, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x07, 0x08, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0xc0, 0x20, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x20, 0xc0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xa0, 0x40,
	0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0xf0,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x20, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
	0x00, 0x7e, 0x02, 0x7e, 0x02, 0x04, 0x08, 0x30, 0x00, 0x00, 0x00, 0x3e, 0x0a, 0x0c, 0x08, 0x10,
	0x00, 0x00, 0x00, 0x04, 0x08, 0x18, 0x28, 0x08, 0x00, 0x00, 0x00, 0x08, 0x3e, 0x22, 0x02, 0x0c,
	0x00, 0x00, 0x00, 0x00, 0x1c, 0x08, 0x08, 0x3e, 0x00, 0x00, 0x00, 0x04, 0x3e, 0x0c, 0x14, 0x24,
	0x00, 0x00, 0x00, 0x10, 0x3e, 0x12, 0x14, 0x10, 0x00, 0x00, 0x00, 0x00, 0x38, 0x08, 0x08, 0x7c,
	0x00, 0x00, 0x00, 0x3c, 0x04, 0x3c, 0x04, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x52, 0x52, 0x04, 0x18,
	0x00, 0x00, 0x00, 0x40, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x01, 0x0a, 0x0c, 0x08, 0x08, 0x10,
	0x00, 0x02, 0x04, 0x08, 0x18, 0x28, 0x48, 0x08, 0x08, 0x08, 0x7e, 0x42, 0x42, 0x02, 0x04, 0x18,
	0x00, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x7f, 0x04, 0x04, 0x7f, 0x04, 0x0c, 0x14, 0x24, 0x44,
	0x10, 0x10, 0x7e, 0x12, 0x12, 0x22, 0x22, 0x44, 0x10, 0x10, 0x7f, 0x08, 0x7f, 0x08, 0x04, 0x04,
	0x20, 0x3e, 0x22, 0x42, 0x02, 0x04, 0x04, 0x18, 0x20, 0x20, 0x3f, 0x24, 0x44, 0x04, 0x08, 0x30,
	0x00, 0x00, 0x7e, 0x02, 0x02, 0x02, 0x02, 0x7e, 0x00, 0x24, 0xff, 0x24, 0x24, 0x04, 0x04, 0x18,
	0x00, 0x40, 0x20, 0x42, 0x22, 0x04, 0x08, 0x70, 0x00, 0x7e, 0x02, 0x04, 0x08, 0x14, 0x22, 0x41,
	0x20, 0x20, 0x7e, 0x22, 0x24, 0x28, 0x20, 0x1e, 0x00, 0x41, 0x41, 0x21, 0x02, 0x02, 0x04, 0x18,
	0x10, 0x1e, 0x12, 0x32, 0x4e, 0x04, 0x08, 0x70, 0x06, 0x38, 0x08, 0x7f, 0x08, 0x08, 0x08, 0x30,
	0x00, 0x52, 0x52, 0x02, 0x04, 0x04, 0x08, 0x30, 0x3e, 0x00, 0x00, 0x7f, 0x08, 0x08, 0x08, 0x30,
	0x20, 0x20, 0x20, 0x38, 0x24, 0x22, 0x20, 0x20, 0x04, 0x04, 0x7f, 0x04, 0x04, 0x04, 0x08, 0x30,
	0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x7e, 0x02, 0x22, 0x14, 0x0c, 0x12, 0x60,
	0x08, 0x08, 0x7f, 0x01, 0x06, 0x1a, 0x69, 0x08, 0x00, 0x02, 0x02, 0x02, 0x04, 0x04, 0x08, 0x30,
	0x00, 0x00, 0x24, 0x24, 0x22, 0x42, 0x81, 0x00, 0x00, 0x40, 0x40, 0x7e, 0x40, 0x40, 0x40, 0x3e,
	0x00, 0x00, 0x7e, 0x02, 0x02, 0x04, 0x08, 0x30, 0x00, 0x00, 0x18, 0x24, 0x42, 0x01, 0x00, 0x00,
	0x08, 0x08, 0x7f, 0x08, 0x08, 0x2a, 0x49, 0x08, 0x00, 0x7e, 0x02, 0x02, 0x24, 0x18, 0x08, 0x04,
	0x00, 0x18, 0x06, 0x18, 0x06, 0x00, 0x30, 0x0e, 0x00, 0x08, 0x08, 0x10, 0x10, 0x24, 0x42, 0x7d,
	0x00, 0x01, 0x01, 0x12, 0x0a, 0x04, 0x0a, 0x30, 0x00, 0x7e, 0x10, 0x10, 0x7f, 0x10, 0x10, 0x1e,
	0x10, 0x10, 0xff, 0x11, 0x12, 0x14, 0x10, 0x10, 0x00, 0x00, 0x3c, 0x04, 0x04, 0x04, 0x04, 0x7e,
	0x00, 0x7e, 0x02, 0x02, 0x7e, 0x02, 0x02, 0x7e, 0x00, 0x3c, 0x00, 0x7e, 0x02, 0x02, 0x04, 0x38,
	0x00, 0x22, 0x22, 0x22, 0x22, 0x02, 0x04, 0x18, 0x00, 0x28, 0x28, 0x28, 0x28, 0x29, 0x2a, 0x4c,
	0x00, 0x20, 0x20, 0x20, 0x22, 0x22, 0x24, 0x38, 0x00, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7e,
	0x00, 0x7e, 0x42, 0x42, 0x02, 0x04, 0x08, 0x30, 0x00, 0x40, 0x20, 0x02, 0x02, 0x04, 0x08, 0x70,
	0xa0, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xa0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x0f, 0x08, 0x08,
	0x08, 0x08, 0xff, 0x08, 0x08, 0xff, 0x08, 0x08, 0x08, 0x08, 0xf8, 0x08, 0x08, 0xf8, 0x08, 0x08,
	0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff,
	0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80,
	0x08, 0x1c, 0x3e, 0x7f, 0x7f, 0x1c, 0x3e, 0x00, 0x36, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c, 0x08, 0x00,
	0x08, 0x1c, 0x3e, 0x7f, 0x3e, 0x1c, 0x08, 0x00, 0x1c, 0x1c, 0x6b, 0x7f, 0x6b, 0x08, 0x1c, 0x00,
	0x1c, 0x3e, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c, 0x00, 0x1c, 0x22, 0x41, 0x41, 0x41, 0x22, 0x1c, 0x00,
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
	0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81, 0x7f, 0x49, 0x49, 0x49, 0x7f, 0x41, 0x41, 0x43,
	0x20, 0x3e, 0x48, 0xbe, 0x28, 0xff, 0x08, 0x08, 0x3f, 0x21, 0x3f, 0x21, 0x3f, 0x21, 0x41, 0x83,
	0x7e, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x7e, 0x04, 0xee, 0xa4, 0xff, 0xa2, 0xbf, 0xe2, 0x0a,
	0x18, 0x04, 0x42, 0xbd, 0x14, 0x14, 0x24, 0x4c, 0x24, 0xc4, 0x4e, 0xed, 0x55, 0xe6, 0xc4, 0x58,
	0x7f, 0x00, 0x7f, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08, 0x7f, 0x08, 0x7f, 0x49, 0x49, 0x4b, 0x08,
	0xff, 0x80, 0xa4, 0x94, 0x88, 0x94, 0xe0, 0xff, 0xf8, 0xaf, 0xfa, 0xaa, 0xaa, 0xfa, 0x02, 0x06,
	0x22, 0xf2, 0x2f, 0x72, 0x6a, 0xa2, 0x22, 0x26, 0x08, 0x08, 0x08, 0x08, 0x08, 0x14, 0x22, 0xc1,
	0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0x00, 0x00, 0x10, 0x08, 0x04, 0x04, 0x00, 0x00,
};

static const uint8_t key_table[10][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x37, 0x55, 0x4a, 0x4d},
	{0x00, 0x00, 0x00, 0x00, 0x38, 0x49, 0x4b, 0xbf},
	{0x00, 0x00, 0x00, 0x00, 0x39, 0x4f, 0x4c, 0xbe},
	{0x00, 0x00, 0x00, 0x00, 0x30, 0x50, 0xbc, 0xbb},
	{0x15, 0x2e, 0x24, 0x70, 0x31, 0x51, 0x41, 0x5a},
	{0x13, 0xdc, 0x27, 0x71, 0x32, 0x57, 0x53, 0x58},
	{0x20, 0xde, 0x25, 0x72, 0x33, 0x45, 0x44, 0x43},
	{0xc0, 0xbd, 0x28, 0x73, 0x34, 0x52, 0x46, 0x56},
	{0xdb, 0xba, 0x26, 0x74, 0x35, 0x54, 0x47, 0x42},
	{0x0d, 0xdd, 0x14, 0xe2, 0x36, 0x59, 0x48, 0x4e}
};

void IO::initialize()
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	} else {
		memcpy(font, elisa_font, sizeof(font));
	}
	delete fio;
	
	sod = false;
	
	cmt_fio = new FILEIO();
	cmt_rec = cmt_is_wav = false;
	cmt_bufcnt = 0;
	
	memset(&b16_1, 0, sizeof(b16_1));
	memset(&b16_2, 0, sizeof(b16_2));
	memset(&g21_1, 0, sizeof(g21_1));
	memset(&g21_2, 0, sizeof(g21_2));
	memset(&c15, 0, sizeof(c15));
	memset(&c16, 0, sizeof(c16));
	memset(&f21, 0, sizeof(f21));
	
	cmt_clock = 0;
	b16_1.in_s = b16_1.in_r = true;
	b16_2.in_s = b16_2.in_r = true;
	g21_1.in_s = true;
	g21_2.in_s = g21_2.in_r = true;
	g21_1.in_d = true;	// f21:q5 and f21:q6 are low
	c15.in_b = true;	// 300baud
	c15.in_c = false;	// load clock
	c15.in_s = false;
	update_cmt();
	
	register_event(this, EVENT_CMT_READY, 1000000.0 / 300.0, true, NULL); // 300baud
	register_event(this, EVENT_CMT_CLOCK, 1000000.0 / 76800.0, true, NULL);
	
	key_stat = emu->get_key_buffer();
}

void IO::release()
{
	close_tape();
	delete cmt_fio;
}

void IO::reset()
{
	mode_basic = (config.boot_mode == 0);
	
	memset(lcd, 0, sizeof(lcd));
	lcd[0].cursor = lcd[1].cursor = -1;
	lcd_status = lcd_addr = 0;
	lcd_text = true;
	
	close_tape();
	cmt_selected = false;
	cmt_mode = 0;
	cmt_play_ready = cmt_play_signal = cmt_rec_ready = false;
	
	key_column = 0;
	update_sid();
}

#define REVERSE(v) (((v) & 0x80) >> 7) | (((v) & 0x40) >> 5) | (((v) & 0x20) >> 3) | (((v) & 0x10) >> 1) | (((v) & 0x08) << 1) | (((v) & 0x04) << 3) | (((v) & 0x02) << 5) | (((v) & 0x01) << 7)

void IO::write_io8(uint32_t addr, uint32_t data)
{
#ifdef _IO_DEBUG_LOG
	this->out_debug_log(_T("%06x\tSOD=%d\tOUT8\t%04x, %02x\n"), get_cpu_pc(0), sod ? 1 : 0, addr & 0xff, data & 0xff);
#endif
	
	if(sod) {
		switch(addr & 0xff) {
		case 0x01:
		case 0x02:
			if(lcd_status == 0xb0) {
				if(data == 0x40) {
					lcd_text = false;
				} else if(data == 0x50) {
					lcd[addr & 1].offset = lcd_addr & 0x3ff;
				} else {
					lcd_text = true;
				}
			} else if(lcd_status == 0x10) {
				if(lcd_text) {
					for(int l = 0; l < 8; l++) {
						lcd[addr & 1].ram[(lcd_addr + 16 * l) & 0x3ff] = REVERSE(font[data * 8 + l]);
					}
				} else {
					lcd[addr & 1].ram[lcd_addr & 0x3ff] = data;
				}
			}
			break;
		case 0x08:
			if((lcd_status = data & 0xf0) == 0xb0) {
				lcd_addr = (lcd_addr & 0xff) | (data << 8);
			}
			break;
		case 0x09:
			lcd_addr = (lcd_addr & 0xff00) | data;
			break;
		case 0x10:
			cmt_selected = ((data & 2) == 0);
			d_drec->write_signal(SIG_DATAREC_REMOTE, CMT_MODE_PLAY ? 1 : 0, 1);
			break;
		case 0x21:
			key_column = data & 0x0f;
			update_sid();
			break;
		case 0x40:
			if(CMT_RECORDING) {
				if(data & 1) {
					// 1: 2.4KHz * 8
					for(int i = 0; i < 8; i++) {
						cmt_write_buffer(0xff, CMT_SAMPLE_RATE / 2400 / 2);
						cmt_write_buffer(0x00, CMT_SAMPLE_RATE / 2400 / 2);
					}
				} else {
					// 0: 1.2KHz * 4
					for(int i = 0; i < 4; i++) {
						cmt_write_buffer(0xff, CMT_SAMPLE_RATE / 1200 / 2);
						cmt_write_buffer(0x00, CMT_SAMPLE_RATE / 1200 / 2);
					}
				}
			}
			break;
		case 0x43:
			cmt_mode = data & 7;
			d_drec->write_signal(SIG_DATAREC_REMOTE, CMT_MODE_PLAY ? 1 : 0, 1);
			break;
		}
	} else {
		switch(addr & 0xf0) {
		case 0x10:
			d_rtc->write_io8(addr, data);
			break;
		}
	}
}

uint32_t IO::read_io8(uint32_t addr)
{
	uint32_t value = 0xff;
	
	if(sod) {
		switch(addr & 0xff) {
		case 0x01:
		case 0x02:
			value = lcd[addr & 1].ram[lcd_addr & 0x3ff];
			break;
		case 0x08:
			value = ((lcd_addr >> 8) & 0x0f) | lcd_status;
			break;
		case 0x09:
			value = lcd_addr & 0xff;
			break;
		case 0x10:
			value = 0;
			break;
		case 0x20:
			value = 0;
			if(key_column < 10) {
				for(int i = 0; i < 8; i++) {
					if(key_stat[key_table[key_column][i]]) {
						value |= (1 << i);
					}
				}
			}
			value = value;
			break;
		case 0x40:
			value = (CMT_PLAYING && cmt_play_signal) ? 0x08 : 0;
			break;
		case 0x41:
			value = ((CMT_PLAYING && cmt_play_ready) || (CMT_RECORDING && cmt_rec_ready)) ? 0x10 : 0;
			cmt_play_ready = cmt_rec_ready = false;
			break;
		}
	} else {
		switch(addr & 0xf0) {
		case 0x10:
			value = d_rtc->read_io8(addr);
			break;
		case 0x40:
			value = 0; // no floppy drives
			break;
		}
	}
#ifdef _IO_DEBUG_LOG
	this->out_debug_log(_T("%06x\tSOD=%d\tIN8\t%04x = %02x\n"), get_cpu_pc(0), sod ? 1 : 0, addr & 0xff, value);
#endif
	return value;
}

void IO::write_io8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 2;
	write_io8(addr, data);
}

uint32_t IO::read_io8w(uint32_t addr, int* wait)
{
	*wait = 2;
	return read_io8(addr);
}

void IO::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_IO_SOD) {
		sod = ((data & mask) != 0);
	} else if(id == SIG_IO_CMT) {
		b16_1.in_d = g21_1.in_ck = c16.in_a = ((data & mask) != 0);
		update_cmt();
	}
}

#define BIT_2400HZ	0x10
#define BIT_1200HZ	0x20
#define BIT_300HZ	0x80

void IO::event_callback(int event_id, int err)
{
	if(event_id == EVENT_CMT_READY) {
		if(CMT_RECORDING) {
			if(!cmt_rec_ready) {
				request_skip_frames();
			}
			cmt_rec_ready = true;
		}
	} else if(event_id == EVENT_CMT_CLOCK) {
		c15.in_d4 = c15.in_d5 = ((cmt_clock & BIT_1200HZ) != 0);
		c15.in_d6 = c15.in_d7 = ((cmt_clock & BIT_300HZ ) != 0);
		
		// 76.8KHz: L->H
		b16_1.in_ck = b16_2.in_ck = g21_2.in_ck = false;
		f21.in_ck = !(g21_1.in_d && false);
		update_cmt();
		b16_1.in_ck = b16_2.in_ck = g21_2.in_ck = true;
		f21.in_ck = !(g21_1.in_d && true);
		update_cmt();
		
		cmt_clock++;
	}
}

void IO::key_down(int code)
{
	if(code >= 0x10 && code <= 0x13) {
		update_sid();
	} else {
		d_cpu->write_signal(SIG_I8085_RST7, 1, 1);
	}
}

void IO::key_up()
{
	update_sid();
}

void IO::update_sid()
{
	switch(key_column) {
	case 5:
		d_cpu->write_signal(SIG_I8085_SID, mode_basic ? 1 : 0, 1);
		break;
	case 6:
		d_cpu->write_signal(SIG_I8085_SID, key_stat[0x10] ? 0 : 1, 1);	// shift
		break;
	case 7:
		d_cpu->write_signal(SIG_I8085_SID, key_stat[0x13] ? 0 : 1, 1);	// break
		break;
	case 8:
		d_cpu->write_signal(SIG_I8085_SID, key_stat[0x12] ? 0 : 1, 1);	// graph
		break;
	case 9:
		d_cpu->write_signal(SIG_I8085_SID, key_stat[0x11] ? 0 : 1, 1);	// ctrl
		break;
	default:
		d_cpu->write_signal(SIG_I8085_SID, 1, 1);
		break;
	}
}

void IO::cmt_write_buffer(uint8_t value, int samples)
{
	if(cmt_is_wav) {
		for(int i = 0; i < samples; i++) {
			cmt_buffer[cmt_bufcnt++] = value;
			if(cmt_bufcnt == sizeof(cmt_buffer)) {
				cmt_fio->Fwrite(cmt_buffer, sizeof(cmt_buffer), 1);
				cmt_bufcnt = 0;
			}
		}
	} else {
		value = (value > 128) ? 0x80 : 0;
		while(samples > 0) {
			int s = min(samples, 0x7f);
			samples -= s;
			cmt_buffer[cmt_bufcnt++] = value | s;
			if(cmt_bufcnt == sizeof(cmt_buffer)) {
				cmt_fio->Fwrite(cmt_buffer, sizeof(cmt_buffer), 1);
				cmt_bufcnt = 0;
			}
		}
	}
}

void IO::rec_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(cmt_fio->Fopen(file_path, FILEIO_READ_WRITE_NEW_BINARY)) {
		my_tcscpy_s(cmt_rec_file_path, _MAX_PATH, file_path);
		if(check_file_extension(file_path, _T(".wav"))) {
			write_dummy_wav_header((void *)cmt_fio);
			cmt_is_wav = true;
		}
		cmt_bufcnt = 0;
		cmt_rec = true;
	}
}

void IO::close_tape()
{
	// close file
	if(cmt_fio->IsOpened()) {
		if(cmt_rec) {
			if(cmt_bufcnt) {
				cmt_fio->Fwrite(cmt_buffer, cmt_bufcnt, 1);
			}
			if(cmt_is_wav) {
				uint32_t length = cmt_fio->Ftell();
				
				wav_header_t wav_header;
				wav_chunk_t wav_chunk;
				if(set_wav_header(&wav_header, &wav_chunk, 1, CMT_SAMPLE_RATE, 8, length)) {
					cmt_fio->Fseek(0, FILEIO_SEEK_SET);
					cmt_fio->Fwrite(&wav_header, sizeof(wav_header), 1);
					cmt_fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
				}
			}
		}
		cmt_fio->Fclose();
	}
	cmt_rec = cmt_is_wav = false;
	cmt_bufcnt = 0;
}

void IO::update_cmt()
{
	bool prev_load_clock = c15.out_y;
	
	b16_1.update();
	b16_2.update();
	f21.update();
	g21_1.update();
	g21_2.update();
	c16.update();
	
	b16_2.in_d = b16_1.out_q;
	f21.in_clr = (b16_1.out_q && b16_2.out_nq);
	g21_1.in_d = g21_1.in_r = c15.in_d0 = !(f21.out_q5 && f21.out_q6);
	g21_2.in_d = g21_1.out_q;
	c16.in_rc1 = c16.in_rc2 = (g21_1.out_q != g21_2.out_q); // xor
	c15.in_a = (g21_1.out_q && g21_2.out_q);
	c16.in_b = c16.out_qa;
	c15.in_d1 = !c16.out_qa;
	c15.in_d2 = c16.out_qb;
	c15.in_d3 = c16.out_qc;
	
	c15.update();
	
	if(!prev_load_clock && c15.out_y) {
		cmt_play_ready = true;
		cmt_play_signal = c15.in_a;
	}
}

void IO::draw_screen()
{
	// render screen
	for(int y = 0; y < 8; y++) {
		for(int x = 0; x < 20; x++) {
			int addr = y * 0x80 + (x % 10) + lcd[x < 10].offset;
			for(int l = 0; l < 8; l++) {
				uint8_t pat = lcd[x < 10].ram[addr & 0x3ff];
				addr += 0x10;
				uint8_t* d = &screen[y * 8 + l][x * 8];
				
				d[0] = pat & 0x01;
				d[1] = pat & 0x02;
				d[2] = pat & 0x04;
				d[3] = pat & 0x08;
				d[4] = pat & 0x10;
				d[5] = pat & 0x20;
				d[6] = pat & 0x40;
				d[7] = pat & 0x80;
			}
		}
	}
	
	// copy to real screen
	scrntype_t cd = RGB_COLOR(48, 56, 16);
	scrntype_t cb = RGB_COLOR(160, 168, 160);
	for(int y = 0; y < 64; y++) {
		scrntype_t* dst = emu->get_screen_buffer(y);
		uint8_t* src = screen[y];
		
		for(int x = 0; x < 160; x++) {
			dst[x] = src[x] ? cd : cb;
		}
	}
}

#define STATE_VERSION	3

void process_state_ls74(ls74_t* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->in_d);
	state_fio->StateValue(val->in_ck);
	state_fio->StateValue(val->in_s);
	state_fio->StateValue(val->in_r);
	state_fio->StateValue(val->out_q);
	state_fio->StateValue(val->out_nq);
	state_fio->StateValue(val->tmp_ck);
}

void process_state_ls151(ls151_t* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->in_d0);
	state_fio->StateValue(val->in_d1);
	state_fio->StateValue(val->in_d2);
	state_fio->StateValue(val->in_d3);
	state_fio->StateValue(val->in_d4);
	state_fio->StateValue(val->in_d5);
	state_fio->StateValue(val->in_d6);
	state_fio->StateValue(val->in_d7);
	state_fio->StateValue(val->in_a);
	state_fio->StateValue(val->in_b);
	state_fio->StateValue(val->in_c);
	state_fio->StateValue(val->in_s);
	state_fio->StateValue(val->out_y);
	state_fio->StateValue(val->out_ny);
}

void process_state_ls93(ls93_t* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->in_a);
	state_fio->StateValue(val->in_b);
	state_fio->StateValue(val->in_rc1);
	state_fio->StateValue(val->in_rc2);
	state_fio->StateValue(val->out_qa);
	state_fio->StateValue(val->out_qb);
	state_fio->StateValue(val->out_qc);
	state_fio->StateValue(val->tmp_a);
	state_fio->StateValue(val->tmp_b);
	state_fio->StateValue(val->counter_a);
	state_fio->StateValue(val->counter_b);
}

void process_state_tc4024bp(tc4024bp_t* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->in_ck);
	state_fio->StateValue(val->in_clr);
	state_fio->StateValue(val->out_q5);
	state_fio->StateValue(val->out_q6);
	state_fio->StateValue(val->tmp_ck);
	state_fio->StateValue(val->counter);
}

bool IO::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	// pre process
	if(loading) {
		close_tape();
	}
	for(int i = 0; i < array_length(lcd); i++) {
		state_fio->StateArray(lcd[i].ram, sizeof(lcd[i].ram), 1);
		state_fio->StateValue(lcd[i].offset);
		state_fio->StateValue(lcd[i].cursor);
	}
	state_fio->StateValue(lcd_status);
	state_fio->StateValue(lcd_addr);
	state_fio->StateValue(lcd_text);
	state_fio->StateValue(cmt_selected);
	state_fio->StateValue(cmt_mode);
	state_fio->StateValue(cmt_play_ready);
	state_fio->StateValue(cmt_play_signal);
	state_fio->StateValue(cmt_rec_ready);
	state_fio->StateValue(cmt_rec);
	state_fio->StateValue(cmt_is_wav);
	state_fio->StateArray(cmt_rec_file_path, sizeof(cmt_rec_file_path), 1);
	if(loading) {
		int length_tmp = state_fio->FgetInt32_LE();
		if(cmt_rec) {
			cmt_fio->Fopen(cmt_rec_file_path, FILEIO_READ_WRITE_NEW_BINARY);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				state_fio->Fread(buffer_tmp, length_rw, 1);
				if(cmt_fio->IsOpened()) {
					cmt_fio->Fwrite(buffer_tmp, length_rw, 1);
				}
				length_tmp -= length_rw;
			}
		}
	} else {
		if(cmt_rec && cmt_fio->IsOpened()) {
			int length_tmp = (int)cmt_fio->Ftell();
			cmt_fio->Fseek(0, FILEIO_SEEK_SET);
			state_fio->FputInt32_LE(length_tmp);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				cmt_fio->Fread(buffer_tmp, length_rw, 1);
				state_fio->Fwrite(buffer_tmp, length_rw, 1);
				length_tmp -= length_rw;
			}
		} else {
			state_fio->FputInt32_LE(0);
		}
	}
	state_fio->StateValue(cmt_bufcnt);
	state_fio->StateArray(cmt_buffer, cmt_bufcnt, 1);
	state_fio->StateValue(cmt_clock);
	process_state_ls74(&b16_1, state_fio);
	process_state_ls74(&b16_2, state_fio);
	process_state_ls74(&g21_1, state_fio);
	process_state_ls74(&g21_2, state_fio);
	process_state_ls151(&c15, state_fio);
	process_state_ls93(&c16, state_fio);
	process_state_tc4024bp(&f21, state_fio);
	state_fio->StateValue(key_column);
	return true;
}

}
