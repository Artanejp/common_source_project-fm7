/*
	CANON X-07 Emulator 'eX-07'

	Origin : J.Brigaud
	Author : Takeda.Toshiya
	Date   : 2007.12.26 -

	[ i/o ]
*/

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4996 )
#endif

#include <math.h>
#include "io.h"
#include "../beep.h"
#include "../z80.h"
#include "../../fifo.h"

#define EVENT_BEEP	0
#define EVENT_CMT	1
#define EVENT_1SEC	2

static const uint8_t sub_cmd_len[0x47] = {
	1,	// 00	Unknown
	1,	// 01	TimeCall
	1,	// 02	Stick
	1,	// 03	Strig
	1,	// 04	Strig1
	3,	// 05	RamRead
	4,	// 06	RamWrite
	3,	// 07	ScrollSet
	1,	// 08	ScrollExet
	2,	// 09	LineClear
	9,	// 0a	TimeSet
	1,	// 0b	CalcDay
	9,	// 0c	AlarmSet
	1,	// 0d	BuzzerOff
	1,	// 0e	BuzzerOn
	2,	// 0f	TrfLine
	3,	// 10	TestPoint
	3,	// 11	Pset
	3,	// 12	Preset
	3,	// 13	Peor
	5,	// 14	Line
	4,	// 15	Circle
	0x82,	// 16	UDKWrite
	2,	// 17	UDKRead
	1,	// 18	UDKOn
	1,	// 19	UDKOff
	10,	// 1a	UDCWrite
	2,	// 1b	UDCRead
	1,	// 1c	UDCInt
	0x81,	// 1d	StartPgmWrite
	0x81,	// 1e	SPWriteCont
	1,	// 1f	SPOn
	1,	// 20	SPOff
	1,	// 21	StartPgmRead
	1,	// 22	OnStat
	1,	// 23	OFFReq
	4,	// 24	Locate
	1,	// 25	CursOn
	1,	// 26	CursOff
	3,	// 27	TestKey
	2,	// 28	TestChr
	1,	// 29	InitSec
	2,	// 2a	InitDate
	1,	// 2b	ScrOff
	1,	// 2c	ScrOn
	1,	// 2d	KeyBufferClear
	1,	// 2e	ClsScr
	1,	// 2f	Home
	1,	// 30	OutUdkOn
	1,	// 31	OutUDKOff
	1,	// 32	RepateKeyOn
	1,	// 33	RepateKeyOff
	1,	// 34	UDK=KANA
	0x82,	// 35	UdkContWrite
	1,	// 36	AlarmRead
	1,	// 37	BuzzZero
	1,	// 38	ClickOff
	1,	// 39	ClickOn
	1,	// 3a	LocateClose
	1,	// 3b	KeybOn
	1,	// 3c	KeybOff
	1,	// 3d	ExecStartPgm
	1,	// 3e	UnexecStartPgm
	1,	// 3f	Sleep
	1,	// 40	UDKInit
	9,	// 41	OutUDC
	1,	// 42	ReadCar
	3,	// 43	ScanR
	3,	// 44	ScanL
	1,	// 45	TimeChk
	1	// 46	AlmChk
};

static const char *udk_ini[12] = {
	"tim?TIME$^",
	"cldCLOAD\"",
	"locLOCATE ",
	"lstLIST ",
	"runRUN^",
	"",
	"dat?DATE$^",
	"csvCSAVE\"",
	"prtPRINT ",
	"slpSLEEP",
	"cntCONT^",
	""
};

static const int udk_ofs[12] = {
	0, 42, 84, 126, 168, 210, 256, 298, 340, 382, 424, 466
};

static const int udk_size[12] = {
	42, 42, 42, 42, 42, 46, 42, 42, 42, 42, 42, 46
};

static const int key_tbl[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00, 0x0b,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x12,0x16,0x00,
	0x30,0x31,0x32,0x33, 0x34,0x35,0x36,0x37, 0x38,0x39,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x41,0x42,0x43, 0x44,0x45,0x46,0x47, 0x48,0x49,0x4a,0x4b, 0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53, 0x54,0x55,0x56,0x57, 0x58,0x59,0x5a,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x3a,0x3b, 0x2c,0x2d,0x2e,0x2f,
	0x40,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x5b, 0x5c,0x5d,0x5e,0x00,
	0x00,0x00,0x5c,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

static const int key_tbl_s[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00, 0x0c,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x12,0x16,0x00,
	0x00,0x21,0x22,0x23, 0x24,0x25,0x26,0x27, 0x28,0x29,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x61,0x62,0x63, 0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b, 0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73, 0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x2a,0x2b, 0x3c,0x3d,0x3e,0x3f,
	0x60,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x7b, 0x7c,0x7d,0x7e,0x00,
	0x00,0x00,0x5f,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

static const int key_tbl_k[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00, 0x0b,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x12,0x16,0x00,
	0xdc,0xc7,0xcc,0xb1, 0xb3,0xb4,0xb5,0xd4, 0xd5,0xd6,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0xc1,0xba,0xbf, 0xbc,0xb2,0xca,0xb7, 0xb8,0xc6,0xcf,0xc9, 0xd8,0xd3,0xd0,0xd7,
	0xbe,0xc0,0xbd,0xc4, 0xb6,0xc5,0xcb,0xc3, 0xbb,0xdd,0xc2,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0xb9,0xda, 0xc8,0xce,0xd9,0xd2,
	0xde,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0xdf, 0xb0,0xd1,0xcd,0x00,
	0x00,0x00,0xdb,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

static const int key_tbl_ks[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00, 0x0c,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x12,0x16,0x00,
	0xa6,0x00,0x00,0xa7, 0xa9,0xaa,0xab,0xac, 0xad,0xae,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0xa8,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0xaf,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0xa4,0x00,0xa1,0xa5,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0xa2, 0x00,0xa3,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

static const int key_tbl_g[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00, 0x0c,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x12,0x16,0x00,
	0x8a,0xe9,0x90,0x91, 0x92,0x93,0xec,0xe0, 0xf2,0xf1,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x88,0xed,0xe4, 0xef,0x99,0xfd,0x9d, 0xfe,0xf9,0xe5,0x9b, 0xf4,0xf5,0x89,0x9e,
	0xf7,0x8b,0xf6,0x9f, 0x97,0x94,0x95,0xfb, 0x98,0x96,0xe1,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x81,0x82, 0x9c,0xf0,0x9a,0x80,
	0xe7,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x84, 0x00,0x85,0xfc,0x00,
	0x00,0x00,0x83,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

static const int key_tbl_c[256] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x0d,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00, 0x0b,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x12,0x16,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07, 0x08,0x09,0x0a,0x0b, 0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17, 0x18,0x19,0x1a,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

//memo: how to request the display size changing
//emu->set_vm_screen_size(TV_SCREEN_WIDTH, TV_SCREEN_HEIGHT, -1, -1, -1, -1);
//emu->set_vm_screen_size(SCREEN_WIDTH, SCREEN_HEIGHT, -1, -1, -1, -1);

void IO::initialize()
{
	// load font
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	// init timer
	get_host_time(&cur_time);
	register_event(this, EVENT_1SEC, 1000000, true, &register_id_1sec);
	
	// init fifo
	key_buf = new FIFO(20);
	cmd_buf = new FIFO(256);
	rsp_buf = new FIFO(256);
	
	// wram
	memset(wram, 0, 0x200);
	for(int i = 0; i < 12; i++) {
		strcpy((char*)wram + udk_ofs[i], udk_ini[i]);
	}
	for(int i = 0; i < 0x200; i++) {
		if(wram[i] == '^') {
			wram[i] = 13;
		}
	}
	memcpy(udc, font, sizeof(udc));
	
	// cmt
	cmt_fio = new FILEIO();
	cmt_play = cmt_rec = false;
	
	// video
	register_frame_event(this);
	register_vline_event(this);
}

void IO::release()
{
	close_tape();
	delete cmt_fio;
	
	key_buf->release();
	delete key_buf;
	cmd_buf->release();
	delete cmd_buf;
	rsp_buf->release();
	delete rsp_buf;
}

void IO::reset()
{
	// registers
	memset(rregs, 0, sizeof(rregs));
	memset(wregs, 0, sizeof(wregs));
	
	// t6834
	cmd_buf->clear();
	rsp_buf->clear();
	sub_int = 0;
	
	// keyboard
	key_buf->clear();
	ctrl = shift = kana = graph = brk = false;
	stick = 0x30;
	strig = strig1 = 0xff;
	
	// data recorder
	close_tape();
	cmt_mode = false;
	
	// video
	memset(lcd, 0, sizeof(lcd));
	locate_on = cursor_on = udk_on = false;
	locate_x = locate_y = cursor_x = cursor_y = cursor_blink = 0;
	scroll_min = 0;
	scroll_max = 4;
	
	// beep
	register_id_beep = -1;
}

void IO::event_callback(int event_id, int err)
{
	if(event_id == EVENT_BEEP) {
		register_id_beep = -1;
		d_beep->write_signal(SIG_BEEP_ON, 0, 0);
		rregs[4] = (wregs[4] &= ~2);
	} else if(event_id == EVENT_CMT) {
		sub_int |= 2;
		update_intr();
	} else if(event_id == EVENT_1SEC) {
		if(cur_time.initialized) {
			cur_time.increment();
		} else {
			get_host_time(&cur_time);	// resync
			cur_time.initialized = true;
		}
	}
}

void IO::event_frame()
{
	cursor_blink++;
}

void IO::event_vline(int v, int clock)
{
	vblank = !(v < 192);
}

void IO::write_io8(uint32_t addr, uint32_t data)
{
//	this->out_debug_log(_T("OUT\t%4x, %2x\n"), addr, data);
	switch(addr & 0xff) {
	case 0x80:
		font_code = data;
		break;
	case 0xf0:
//		d_mem->write_signal(0, data, 0xff);
	case 0xf1:
	case 0xf2:
	case 0xf3:
	case 0xf6:
	case 0xf7:
		wregs[addr & 7] = data;
		break;
	case 0xf4:
		rregs[4] = wregs[4] = data;
		// cmt
		cmt_mode = ((data & 0x0c) == 8);
//		if(cmt_mode && (wregs[5] & 4)) {
//			recv_from_cmt();
//		}
		// beep
		if((data & 0x0e) == 0x0e) {
			double freq = 192000.0 / (wregs[2] | (wregs[3] << 8));
			d_beep->set_frequency(freq);
			d_beep->write_signal(SIG_BEEP_ON, 1, 1);
			// temporary patch: register the event to stop
			int intv = ram[0x450] * 50000;
			if(register_id_beep != -1) {
				cancel_event(this, register_id_beep);
			}
			register_event(this, EVENT_BEEP, intv, false, &register_id_beep);
		} else {
			d_beep->write_signal(SIG_BEEP_ON, 0, 1);
		}
		break;
	case 0xf5:
		wregs[5] = data;
		if(data & 1) {
			ack_from_sub();
		}
		if(data & 2) {
			send_to_sub();
		}
		if(data & 4) {
			recv_from_cmt();
		}
		if(data & 8) {
			send_to_cmt();
		}
//		if(data & 0x20) {
//			print(prt_data);
//		}
		break;
	}
}

uint32_t IO::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	
	switch(addr & 0xff) {
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87:
	case 0x88:
	case 0x89:
	case 0x8a:
	case 0x8b:
	case 0x8c:
		val = ((addr & 0x0f) < 8) ? udc[(font_code << 3) | (addr & 7)] : 0;
		break;
	case 0x90:
		val =  vblank ? 0x80 : 0;
		break;
	case 0xf0:
	case 0xf1:
	case 0xf3:
	case 0xf4:
	case 0xf5:
	case 0xf7:
		val = rregs[addr & 7];
		break;
	case 0xf2:
		if(wregs[5] & 4) {
			rregs[2] |= 2;
		} else {
			rregs[2] &= 0xfd;
		}
		val = rregs[2] | 2;
		break;
	case 0xf6:
		if(cmt_mode) {
			rregs[6] |= 5;
		}
		val = rregs[6];
		break;
	}
//	this->out_debug_log(_T("IN\t%4x = %2x\n", addr, val);
	return val;
}

void IO::update_intr()
{
	if(!key_buf->empty()) {
		rregs[0] = 0;
		rregs[1] = key_buf->read();
		rregs[2] |= 1;
		d_cpu->write_signal(SIG_NSC800_RSTA, 1, 1);
	} else if(brk) {
		rregs[0] = 0x80;
		rregs[1] = 5;
		rregs[2] |= 1;
		brk = false;
		d_cpu->write_signal(SIG_NSC800_RSTA, 1, 1);
	} else if(sub_int & 1) {
		recv_from_sub();
		sub_int &= ~1;
		d_cpu->write_signal(SIG_NSC800_RSTA, 1, 1);
	} else if(sub_int & 2) {
		sub_int &= ~2;
		d_cpu->write_signal(SIG_NSC800_RSTB, 1, 1);
	}
}

// ----------------------------------------------------------------------------
// video
// ----------------------------------------------------------------------------

void IO::draw_screen()
{
	scrntype_t cd = RGB_COLOR(48, 56, 16);
	scrntype_t cb = RGB_COLOR(160, 168, 160);
	
	for(int y = 0; y < 4; y++) {
		int py = y * 8;
		for(int x = 0; x < 20; x++) {
			int px = x * 6;
			if(cursor_on && (cursor_blink & 0x20) && cursor_x == x && cursor_y == y) {
				for(int l = 0; l < 8; l++) {
					scrntype_t* dest = emu->get_screen_buffer(py + l);
					dest += px;
					dest[0] = dest[1] = dest[2] = dest[3] = dest[4] = dest[5] = (l < 7) ? cb : cd;
				}
			} else {
				for(int l = 0; l < 8; l++) {
					uint8_t* src = &lcd[py + l][px];
					scrntype_t* dest = emu->get_screen_buffer(py + l);
					dest += px;
					dest[0] = src[0] ? cd : cb;
					dest[1] = src[1] ? cd : cb;
					dest[2] = src[2] ? cd : cb;
					dest[3] = src[3] ? cd : cb;
					dest[4] = src[4] ? cd : cb;
					dest[5] = src[5] ? cd : cb;
				}
			}
		}
	}
}

void IO::draw_font(int x, int y, uint8_t code)
{
	if(x < 20 && y < 4) {
		int px = x * 6;
		int py = y * 8;
		int ofs = code << 3;
		for(int l = 0; l < 8; l++) {
			uint8_t pat = udc[ofs + l];
			lcd[py + l][px + 0] = (pat & 0x80) ? 0xff : 0;
			lcd[py + l][px + 1] = (pat & 0x40) ? 0xff : 0;
			lcd[py + l][px + 2] = (pat & 0x20) ? 0xff : 0;
			lcd[py + l][px + 3] = (pat & 0x10) ? 0xff : 0;
			lcd[py + l][px + 4] = (pat & 0x08) ? 0xff : 0;
			lcd[py + l][px + 5] = (pat & 0x04) ? 0xff : 0;
		}
	}
}

void IO::draw_udk()
{
	for(int i = 0, x = 0; i < 5; i++) {
		int ofs = udk_ofs[i + (shift ? 6 : 0)];
		draw_font(x++, 3, 0x83);
		for(int j = 0; j < 3; j++) {
			draw_font(x++, 3, wram[ofs++]);
		}
	}
}

#define draw_point(x, y, c) { if((x) < 120 && (y) < 32) lcd[y][x] = c; }

void IO::draw_line(int sx, int sy, int ex, int ey)
{
	int next_x = sx, next_y = sy;
	int delta_x = abs(ex - sx) * 2;
	int delta_y = abs(ey - sy) * 2;
	int step_x = (ex < sx) ? -1 : 1;
	int step_y = (ey < sy) ? -1 : 1;
	
	if(delta_x > delta_y) {
		int frac = delta_y - delta_x / 2;
		while(next_x != ex) {
			if(frac >= 0) {
				next_y += step_y;
				frac -= delta_x;
			}
			next_x += step_x;
			frac += delta_y;
			draw_point(next_x, next_y, 0xff);
		}
	} else {
		int frac = delta_x - delta_y / 2;
		while(next_y != ey) {
			if(frac >= 0) {
				next_x += step_x;
				frac -= delta_y;
			}
			next_y += step_y;
			frac += delta_x;
			draw_point(next_x, next_y, 0xff);
		}
	}
	draw_point(sx, sy, 0xff);
	draw_point(ex, ey, 0xff);
}

void IO::draw_circle(int x, int y, int r)
{
#if 0
	// high accuracy
	double xlim = sqrt((double)(r * r) / 2);
	
	for(int cx = 0, cy = r; cx <= xlim ; cx++) {
		double d1 = (cx * cx + cy * cy) - r * r;
		double d2 = (cx * cx + (cy - 1) * (cy - 1)) - r * r;
		if(abs(d1) > abs(d2)) {
			cy--;
		}
		draw_point(cx + x, cy + y, 0xff);
		draw_point(cx + x, -cy + y, 0xff);
		draw_point(-cx + x, cy + y, 0xff);
		draw_point(-cx + x, -cy + y, 0xff);
		draw_point(cy + x, cx + y, 0xff);
		draw_point(cy + x, -cx + y, 0xff);
		draw_point(-cy + x, cx + y, 0xff);
		draw_point(-cy + x, -cx + y, 0xff);
	}
#else
	// high speed
	int cx = 0, cy = r;
	int d = 2 - 2 * r;
	
	draw_point(cx + x, cy + y, 0xff);
	draw_point(cx + x, -cy + y, 0xff);
	draw_point(cy + x, cx + y, 0xff);
	draw_point(-cy + x, cx + y, 0xff);
	while(1) {
		if(d > -cy) {
			cy--;
			d += 1 - 2 * cy;
		}
		if(d <= cx) {
			cx++;
			d += 1 + 2 * cx;
		}
		if(!cy) {
			return;
		}
		draw_point(cx + x, cy + y, 0xff);
		draw_point(-cx + x, cy + y, 0xff);
		draw_point(-cx + x, -cy + y, 0xff);
		draw_point(cx + x, -cy + y, 0xff);
	}
#endif
}

void IO::line_clear(int y)
{
	if(y < 4) {
		for(int l = y * 8; l < (y + 1) * 8; l++) {
			memset(lcd[l], 0, 120);
		}
	}
}

void IO::scroll()
{
	if(scroll_min <= scroll_max && scroll_max <= 4) {
		for(int l = scroll_min * 8; l < (scroll_max - 1) * 8; l++) {
			memcpy(lcd[l], lcd[l + 8], 120);
		}
		for(int l = (scroll_max - 1) * 8; l < scroll_max * 8; l++) {
			memset(lcd[l], 0, 120);
		}
	}
}

// ----------------------------------------------------------------------------
// keyboard
// ----------------------------------------------------------------------------

void IO::key_down(int code)
{
	int fctn, ptr;
	
	switch(code) {
	case 0x10:
		shift = true;
		if(udk_on) {
			draw_udk();
		}
		break;
	case 0x11:
		ctrl = true;
		break;
	case 0x12:
		if(graph) {
			graph = kana = false;
		} else {
			graph = true;
		}
		break;
	case 0x13:
		brk = true;
		update_intr();
		break;
	case 0x15:
		if(kana) {
			graph = kana = false;
		} else {
			kana = true;
		}
		break;
	case 0x20:
		strig1 = 0;
		goto strig_key;
	case 0x25:	// left
		stick = 0x37;
		code = 0x1d;
		goto strig_key;
	case 0x26:	// up
		stick = 0x31;
		code = 0x1e;
		goto strig_key;
	case 0x27:	// right
		stick = 0x32;
		code = 0x1c;
		goto strig_key;
	case 0x28:	// down
		stick = 0x36;
		code = 0x1f;
strig_key:
		if(!key_buf->full()) {
			key_buf->write(code);
			update_intr();
		}
		break;
	case 0x70:	// F1
		fctn = shift ? 6 : 0;
		goto fkey;
	case 0x71:	// F2
		fctn = shift ? 7 : 1;
		goto fkey;
	case 0x72:	// F3
		fctn = shift ? 8 : 2;
		goto fkey;
	case 0x73:	// F4
		fctn = shift ? 9 : 3;
		goto fkey;
	case 0x74:	// F5
		fctn = shift ? 10 : 4;
		goto fkey;
	case 0x75:	// F6
		strig = 0;
		fctn = shift ? 11 : 5;
		goto fkey;
	case 0x76:	// F7
		fctn = 6;
		goto fkey;
	case 0x77:	// F8
		fctn = 7;
		goto fkey;
	case 0x78:	// F9
		fctn = 8;
		goto fkey;
	case 0x79:	// F10
		fctn = 9;
		goto fkey;
	case 0x7a:	// F11
		fctn = 10;
		goto fkey;
	case 0x7b:	// F12
		fctn = 11;
fkey:
		ptr = udk_ofs[fctn] + 3;
		for(;;) {
			uint8_t val = wram[ptr++];
			if(!val) {
				break;
			}
			if(!key_buf->full()) {
				key_buf->write(val);
			}
		}
		if(!key_buf->empty()) {
			update_intr();
		}
		break;
	default:
		if(!key_buf->full()) {
			uint8_t val = 0;
			if(ctrl) {
				val = key_tbl_c[code];
			} else if(kana) {
				if(shift) {
					val = key_tbl_ks[code];
				} else {
					val = key_tbl_k[code];
				}
			} else if(graph) {
				val = key_tbl_g[code];
			} else {
				if(shift) {
					val = key_tbl_s[code];
				} else {
					val = key_tbl[code];
				}
			}
			if(val) {
				key_buf->write(val);
				update_intr();
			}
		}
	}
}

void IO::key_up(int code)
{
	switch(code) {
	case 0x08:	// bs->left
		stick = 0x30;
		break;
	case 0x10:
		shift = false;
		if(udk_on) {
			draw_udk();
		}
		break;
	case 0x11:
		ctrl = false;
		break;
	case 0x20:
		strig1 = 0xff;
		break;
	case 0x25:	// left
	case 0x26:	// up
	case 0x27:	// right
	case 0x28:	// down
		stick = 0x30;
		break;
	case 0x75:	// F6
		strig = 0xff;
		break;
	}
}

// ----------------------------------------------------------------------------
// cmt
// ----------------------------------------------------------------------------

void IO::play_tape(const _TCHAR* file_path)
{
	close_tape();
	if(cmt_fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		cmt_fio->Fseek(0, FILEIO_SEEK_END);
		cmt_len = min((int)cmt_fio->Ftell(), (int)CMT_BUF_SIZE);
		cmt_fio->Fseek(0, FILEIO_SEEK_SET);
		memset(cmt_buf, 0, sizeof(cmt_buf));
		cmt_fio->Fread(cmt_buf, cmt_len, 1);
		cmt_fio->Fclose();
		cmt_ptr = 0;
		cmt_play = true;
		// receive first byte
		if(cmt_mode && (wregs[5] & 4)) {
			recv_from_cmt();
		}
	}
}

void IO::rec_tape(const _TCHAR* file_path)
{
	close_tape();
	if(cmt_fio->Fopen(file_path, FILEIO_READ_WRITE_NEW_BINARY)) {
		my_tcscpy_s(rec_file_path, _MAX_PATH, file_path);
		cmt_ptr = 0;
		cmt_rec = true;
	}
}

void IO::close_tape()
{
	if(cmt_fio->IsOpened()) {
		if(cmt_rec && cmt_ptr) {
			cmt_fio->Fwrite(cmt_buf, cmt_ptr, 1);
		}
		cmt_fio->Fclose();
	}
	cmt_play = cmt_rec = false;
}

void IO::send_to_cmt()
{
	if(cmt_rec && cmt_mode) {
		cmt_buf[cmt_ptr++] = wregs[7];
		if(!(cmt_ptr &= (CMT_BUF_SIZE - 1))) {
			cmt_fio->Fwrite(cmt_buf, sizeof(cmt_buf), 1);
		}
	}
}

void IO::recv_from_cmt()
{
	if(cmt_play && cmt_mode) {
		rregs[6] |= 2;
		rregs[7] = (cmt_ptr < cmt_len) ? cmt_buf[cmt_ptr++] : 0;
		// register event for rstb
		register_event(this, EVENT_CMT, 2000, false, NULL);
	}
}

// ----------------------------------------------------------------------------
// sub cpu
// ----------------------------------------------------------------------------

void IO::send_to_sub()
{
	// send command
	if(cmd_buf->empty()) {
		if(locate_on && (wregs[1] & 0x7f) != 0x24 && 0x20 <= wregs[1] && wregs[1] < 0x80) {
			cursor_x++;
			draw_font(cursor_x, cursor_x, wregs[1]);
		} else {
			if((wregs[1] & 0x7f) < 0x47) {
				cmd_buf->write(wregs[1] & 0x7f);
			}
			locate_on = 0;
		}
	} else {
		cmd_buf->write(wregs[1]);
		if(cmd_buf->count() == 2) {
			uint8_t cmd_type = cmd_buf->read_not_remove(0);
			if(cmd_type == 7 && wregs[1] > 4) {
				cmd_buf->clear();
				cmd_buf->write(wregs[1] & 0x7f);
			} else if(cmd_type == 0x0c && wregs[1] == 0xb0) {
				memset(lcd, 0, sizeof(lcd));
				cmd_buf->clear();
				cmd_buf->write(wregs[1] & 0x7f);
			}
		}
	}
	// check cmd length
	if(!cmd_buf->empty()) {
		uint8_t cmd_type = cmd_buf->read_not_remove(0);
		uint8_t cmd_len = sub_cmd_len[cmd_type];
		if(cmd_len & 0x80) {
			if((cmd_len & 0x7f) < cmd_buf->count() && !wregs[1]) {
				cmd_len = cmd_buf->count();
			}
		}
		if(cmd_buf->count() == cmd_len) {
			// process command
			rsp_buf->clear();
			process_sub();
			cmd_buf->clear();
			if(rsp_buf->count()) {
				sub_int |= 1;
				update_intr();
			}
//			this->out_debug_log(_T("CMD TYPE = %2x, LEN = %d RSP=%d\n"), cmd_type, cmd_len, rsp_buf->count());
		}
	}
}

void IO::recv_from_sub()
{
	rregs[0] = 0x40;
	rregs[1] = rsp_buf->read_not_remove(0);
	rregs[2] |= 1;
}

void IO::ack_from_sub()
{
	rsp_buf->read();
	rregs[2] &= 0xfe;
	if(!rsp_buf->empty()) {
		sub_int |= 1;
		update_intr();
	}
}

void IO::process_sub()
{
	static const uint8_t dow[8] = {128, 192, 224, 240, 248, 252, 254, 255};
	uint8_t val;
	uint16_t addr;
	int sx, sy, ex, ey, cr, i;
	
	uint8_t cmd_type = cmd_buf->read();
	switch(cmd_type & 0x7f) {
	case 0x00:	// unknown
		break;
	case 0x01:	// TimeCall
		rsp_buf->write((cur_time.year >> 8) & 0xff);
		rsp_buf->write(cur_time.year & 0xff);
		rsp_buf->write(cur_time.month);
		rsp_buf->write(cur_time.day);
		rsp_buf->write(dow[cur_time.day_of_week]);
		rsp_buf->write(cur_time.hour);
		rsp_buf->write(cur_time.minute);
		rsp_buf->write(cur_time.second);
		break;
	case 0x02:	// Stick
		rsp_buf->write(stick);
		break;
	case 0x03:	// Strig
		rsp_buf->write(strig);
		break;
	case 0x04:	// Strig1
		rsp_buf->write(strig1);
		break;
	case 0x05:	// RamRead
		addr = cmd_buf->read();
		addr |= cmd_buf->read() << 8;
		if(addr == 0xc00e) {
			val = 0x0a;
		} else if(addr == 0xd000) {
			val = 0x30;
		} else if(WRAM_OFS_UDC0 <= addr && addr < WRAM_OFS_UDC1) {
			val = udc[FONT_OFS_UDC0 + (addr - WRAM_OFS_UDC0)];
		} else if(WRAM_OFS_UDC1 <= addr && addr < WRAM_OFS_KBUF) {
			val = udc[FONT_OFS_UDC1 + (addr - WRAM_OFS_UDC1)];
		} else {
			val = wram[addr & 0x7ff];
		}
		rsp_buf->write(val);
		break;
	case 0x06:	// RamWrite
		val = cmd_buf->read();
		addr = cmd_buf->read();
		addr |= cmd_buf->read() << 8;
		if(WRAM_OFS_UDC0 <= addr && addr < WRAM_OFS_UDC1) {
			udc[FONT_OFS_UDC0 + (addr - WRAM_OFS_UDC0)] = val;
		} else if(WRAM_OFS_UDC1 <= addr && addr < WRAM_OFS_KBUF) {
			udc[FONT_OFS_UDC1 + (addr - WRAM_OFS_UDC1)] = val;
		} else {
			wram[addr & 0x7ff] = val;
		}
		break;
	case 0x07:	// ScrollSet
		scroll_min = cmd_buf->read();
		scroll_max = cmd_buf->read() + 1;
		break;
	case 0x08:	// ScrollExec
		scroll();
		break;
	case 0x09:	// LineClear
		val = cmd_buf->read();
		line_clear(val);
		break;
	case 0x0a:	// TimeSet
		cur_time.second = cmd_buf->read();
		cur_time.minute = cmd_buf->read();
		cur_time.hour = cmd_buf->read();
		cmd_buf->read(); // day of week
		cur_time.day = cmd_buf->read();
		cur_time.month = cmd_buf->read();
		cur_time.year = cmd_buf->read();
		cur_time.year |= cmd_buf->read() << 8;
		cur_time.update_year();
		cur_time.update_day_of_week();
		// restart event
		cancel_event(this, register_id_1sec);
		register_event(this, EVENT_1SEC, 1000000, true, &register_id_1sec);
		break;
	case 0x0b:	// CalcDay
		break;
	case 0x0c:	// AlarmSet
		for(i = 0; i < 8; i++) {
			alarm[i] = cmd_buf->read();
		}
		break;
	case 0x0d:	// BuzzerOff
//		d_beep->write_signal(SIG_BEEP_ON, 0, 0);
		break;
	case 0x0e:	// BuzzerOn
//		d_beep->write_signal(SIG_BEEP_ON, 1, 1);
		break;
	case 0x0f:	// TrfLine
		sy = cmd_buf->read();
		for(i = 0; i < 120; i++) {
			if(sy < 32) {
				rsp_buf->write(lcd[sy][i]);
			} else {
				rsp_buf->write(0);
			}
		}
		break;
	case 0x10:	// TestPoint
		sx = cmd_buf->read();
		sy = cmd_buf->read();
		if(sx < 120 && sy < 32) {
			rsp_buf->write(lcd[sy][sx]);
		} else {
			rsp_buf->write(0);
		}
		break;
	case 0x11:	// Pset
		sx = cmd_buf->read();
		sy = cmd_buf->read();
		draw_point(sx, sy, 0xff);
		break;
	case 0x12:	// Preset
		sx = cmd_buf->read();
		sy = cmd_buf->read();
		draw_point(sx, sy, 0);
		break;
	case 0x13:	// Peor
		sx = cmd_buf->read();
		sy = cmd_buf->read();
		if(sx < 120 && sy < 32) {
			lcd[sy][sx] = ~lcd[sy][sx];
		}
		break;
	case 0x14:	// Line
		sx = cmd_buf->read();
		sy = cmd_buf->read();
		ex = cmd_buf->read();
		ey = cmd_buf->read();
		draw_line(sx, sy, ex, ey);
		break;
	case 0x15:	// Circle
		sx = cmd_buf->read();
		sy = cmd_buf->read();
		cr = cmd_buf->read();
		draw_circle(sx, sy, cr);
		break;
	case 0x16:	// UDKWrite
		val = cmd_buf->read();
		for(i = 0; i < udk_size[val]; i++) {
			wram[udk_ofs[val] + i] = cmd_buf->read();
		}
		break;
	case 0x17:	// UDKRead
		val = cmd_buf->read();
		for(i = 0; i < udk_size[val]; i++) {
			uint8_t code = wram[udk_ofs[val] + i];
			rsp_buf->write(code);
			if(!code) {
				break;
			}
		}
		break;
	case 0x18:	// UDKOn
	case 0x19:	// UDKOff
		break;
	case 0x1a:	// UDCWrite
		addr = cmd_buf->read() << 3;
		for(i = 0; i < 8; i++) {
			udc[addr + i] = cmd_buf->read();
		}
		break;
	case 0x1b:	// UDCRead
		addr = cmd_buf->read() << 3;
		for(i = 0; i < 8; i++) {
			rsp_buf->write(udc[addr + i]);
		}
		break;
	case 0x1c:	// UDCInit
		memcpy(udc, font, sizeof(udc));
		break;
	case 0x1d:	// StartPgmWrite
		break;
	case 0x1e:	// SPWriteCont
		break;
	case 0x1f:	// SPOn
		break;
	case 0x20:	// SPOff
		break;
	case 0x21:	// StartPgmRead
		for(i = 0; i < 128; i++) {
			rsp_buf->write(0);
		}
		break;
	case 0x22:	// OnStat
		rsp_buf->write(4);	// 0x41 ?
		break;
	case 0x23:	// OFFReq
		break;
	case 0x24:	// Locate
		sx = cmd_buf->read();
		sy = cmd_buf->read();
		val = cmd_buf->read();
		locate_on = (locate_x != sx || locate_y != sy);
		locate_x = cursor_x = sx;
		locate_y = cursor_y = sy;
		if(val) {
			draw_font(sx, sy, val);
		}
		break;
	case 0x25:	// CursOn
		cursor_on = true;
		break;
	case 0x26:	// CursOff
		cursor_on = false;
		break;
	case 0x27:	// TestKey
	case 0x28:	// TestChr
		rsp_buf->write(0);
		break;
	case 0x29:	// InitSec
	case 0x2a:	// InitDate
	case 0x2b:	// ScrOff
	case 0x2c:	// ScrOn
		break;
	case 0x2d:	// KeyBufferClear
		key_buf->clear();
		break;
	case 0x2e:	// ClsScr
		memset(lcd, 0, sizeof(lcd));
		break;
	case 0x2f:	// Home
		cursor_x = cursor_y = 0;
		break;
	case 0x30:	// UDKOn
		udk_on = true;
		draw_udk();
		break;
	case 0x31:	// UDKOff
		udk_on = false;
		line_clear(3);
		break;
	case 0x36:	// AlarmRead
		for(i = 0; i < 8; i++) {
			rsp_buf->write(alarm[i]);
		}
		break;
	case 0x37:	// BuzzZero
		rsp_buf->write(0);
		break;
	case 0x40:
		memset(wram, 0, 0x200);
		for(i = 0; i < 12; i++) {
			strcpy((char*)wram + udk_ofs[i], udk_ini[i]);
		}
		for(i = 0; i < 0x200; i++) {
			// CR
			if(wram[i] == '^') {
				wram[i] = 13;
			}
		}
		break;
	case 0x42:	// ReadCar
		for(i = 0; i < 8; i++) {
			rsp_buf->write(0);
		}
		break;
	case 0x43:	// ScanR
	case 0x44:	// ScanL
		rsp_buf->write(0);
		rsp_buf->write(0);
		break;
	case 0x45:	// TimeChk
	case 0x46:	// AlmChk
		rsp_buf->write(0);
		break;
	}
}

#define STATE_VERSION	1

bool IO::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(rregs, sizeof(rregs), 1);
	state_fio->StateArray(wregs, sizeof(wregs), 1);
	if(!cur_time.process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(register_id_1sec);
	if(!cmd_buf->process_state((void *)state_fio, loading)) {
		return false;
	}
	if(!rsp_buf->process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(sub_int);
	state_fio->StateArray(wram, sizeof(wram), 1);
	state_fio->StateArray(alarm, sizeof(alarm), 1);
	if(!key_buf->process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(ctrl);
	state_fio->StateValue(shift);
	state_fio->StateValue(kana);
	state_fio->StateValue(graph);
	state_fio->StateValue(brk);
	state_fio->StateValue(stick);
	state_fio->StateValue(strig);
	state_fio->StateValue(strig1);
	state_fio->StateValue(cmt_play);
	state_fio->StateValue(cmt_rec);
	state_fio->StateValue(cmt_mode);
	state_fio->StateArray(rec_file_path, sizeof(rec_file_path), 1);
	if(loading) {
		int length_tmp = state_fio->FgetInt32_LE();
		if(cmt_rec) {
			cmt_fio->Fopen(rec_file_path, FILEIO_READ_WRITE_NEW_BINARY);
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
	state_fio->StateValue(cmt_len);
	state_fio->StateValue(cmt_ptr);
	state_fio->StateArray(cmt_buf, sizeof(cmt_buf), 1);
	state_fio->StateValue(vblank);
	state_fio->StateValue(font_code);
	state_fio->StateArray(udc, sizeof(udc), 1);
	state_fio->StateArray(&lcd[0][0], sizeof(lcd), 1);
	state_fio->StateValue(locate_on);
	state_fio->StateValue(cursor_on);
	state_fio->StateValue(udk_on);
	state_fio->StateValue(locate_x);
	state_fio->StateValue(locate_y);
	state_fio->StateValue(cursor_x);
	state_fio->StateValue(cursor_y);
	state_fio->StateValue(cursor_blink);
	state_fio->StateValue(scroll_min);
	state_fio->StateValue(scroll_max);
	state_fio->StateValue(register_id_beep);
	return true;
}

