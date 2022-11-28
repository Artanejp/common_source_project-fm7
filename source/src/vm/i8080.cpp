/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2008.11.04 -

	[ i8080 / i8085 ]
*/

#include "i8080.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#define AF	regs[0].w.l
#define BC	regs[1].w.l
#define DE	regs[2].w.l
#define HL	regs[3].w.l

#define _F	regs[0].b.l
#define _A	regs[0].b.h
#define _C	regs[1].b.l
#define _B	regs[1].b.h
#define _E	regs[2].b.l
#define _D	regs[2].b.h
#define _L	regs[3].b.l
#define _H	regs[3].b.h

#define CF	0x01
#define NF	0x02
#define VF	0x04
#define XF	0x08
#define HF	0x10
#define YF	0x20
#define ZF	0x40
#define SF	0x80

#define IM_M5	0x01
#define IM_M6	0x02
#define IM_M7	0x04
#define IM_IEN	0x08
#define IM_I5	0x10
#define IM_I6	0x20
#define IM_I7	0x40
#define IM_SID	0x80
// special
#define IM_INT	0x100
#define IM_NMI	0x200
//#define IM_REQ	(IM_I5 | IM_I6 | IM_I7 | IM_INT | IM_NMI)
#define IM_REQ	0x370

#ifndef CPU_START_ADDR
#define CPU_START_ADDR	0
#endif

static const int cc_op[0x100] = {
#ifdef HAS_I8085
	 4,10, 7, 6, 4, 4, 7, 4,10,10, 7, 6, 4, 4, 7, 4, 7,10, 7, 6, 4, 4, 7, 4,10,10, 7, 6, 4, 4, 7, 4,
	 7,10,16, 6, 4, 4, 7, 4,10,10,16, 6, 4, 4, 7, 4, 7,10,13, 6,10,10,10, 4,10,10,13, 6, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 7, 7, 7, 7, 7, 7, 5, 7, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 6,10,10,10,11,12, 7,12, 6,10,10, 0,11,17, 7,12, 6,10,10,10,11,12, 7,12, 6,10,10,10,11, 7, 7,12,
	 6,10,10,16,11,12, 7,12, 6, 6,10, 6,11,10, 7,12, 6,10,10, 4,11,12, 7,12, 6, 6,10, 4,11, 7, 7,12
#else
	 4,10, 7, 5, 5, 5, 7, 4, 4,10, 7, 5, 5, 5, 7, 4, 4,10, 7, 5, 5, 5, 7, 4, 4,10, 7, 5, 5, 5, 7, 4,
	 4,10,16, 5, 5, 5, 7, 4, 4,10,16, 5, 5, 5, 7, 4, 4,10,13, 5,10,10,10, 4, 4,10,13, 5, 5, 5, 7, 4,
	 5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
	 5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5, 7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 7, 5,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 5,10,10,10,11,11, 7,11, 5,10,10,10,11,17, 7,11, 5,10,10,10,11,11, 7,11, 5,10,10,10,11,17, 7,11,
	 5,10,10,18,11,11, 7,11, 5, 5,10, 5,11,17, 7,11, 5,10,10, 4,11,11, 7,11, 5, 5,10, 4,11,17, 7,11
#endif
};

static const uint8_t ZS[256] = {
	0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
};

static const uint8_t ZSP[256] = {
	0x44,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,
	0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,
	0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,
	0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,
	0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,
	0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,
	0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,
	0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,
	0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,
	0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,
	0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,
	0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,
	0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,
	0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,
	0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,
	0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84
};

static const uint16_t DAA[2048] = {
	0x0044,0x0100,0x0200,0x0304,0x0400,0x0504,0x0604,0x0700,0x0808,0x090c,0x1010,0x1114,0x1214,0x1310,0x1414,0x1510,
	0x1000,0x1104,0x1204,0x1300,0x1404,0x1500,0x1600,0x1704,0x180c,0x1908,0x2030,0x2134,0x2234,0x2330,0x2434,0x2530,
	0x2020,0x2124,0x2224,0x2320,0x2424,0x2520,0x2620,0x2724,0x282c,0x2928,0x3034,0x3130,0x3230,0x3334,0x3430,0x3534,
	0x3024,0x3120,0x3220,0x3324,0x3420,0x3524,0x3624,0x3720,0x3828,0x392c,0x4010,0x4114,0x4214,0x4310,0x4414,0x4510,
	0x4000,0x4104,0x4204,0x4300,0x4404,0x4500,0x4600,0x4704,0x480c,0x4908,0x5014,0x5110,0x5210,0x5314,0x5410,0x5514,
	0x5004,0x5100,0x5200,0x5304,0x5400,0x5504,0x5604,0x5700,0x5808,0x590c,0x6034,0x6130,0x6230,0x6334,0x6430,0x6534,
	0x6024,0x6120,0x6220,0x6324,0x6420,0x6524,0x6624,0x6720,0x6828,0x692c,0x7030,0x7134,0x7234,0x7330,0x7434,0x7530,
	0x7020,0x7124,0x7224,0x7320,0x7424,0x7520,0x7620,0x7724,0x782c,0x7928,0x8090,0x8194,0x8294,0x8390,0x8494,0x8590,
	0x8080,0x8184,0x8284,0x8380,0x8484,0x8580,0x8680,0x8784,0x888c,0x8988,0x9094,0x9190,0x9290,0x9394,0x9490,0x9594,
	0x9084,0x9180,0x9280,0x9384,0x9480,0x9584,0x9684,0x9780,0x9888,0x998c,0x0055,0x0111,0x0211,0x0315,0x0411,0x0515,
	0x0045,0x0101,0x0201,0x0305,0x0401,0x0505,0x0605,0x0701,0x0809,0x090d,0x1011,0x1115,0x1215,0x1311,0x1415,0x1511,
	0x1001,0x1105,0x1205,0x1301,0x1405,0x1501,0x1601,0x1705,0x180d,0x1909,0x2031,0x2135,0x2235,0x2331,0x2435,0x2531,
	0x2021,0x2125,0x2225,0x2321,0x2425,0x2521,0x2621,0x2725,0x282d,0x2929,0x3035,0x3131,0x3231,0x3335,0x3431,0x3535,
	0x3025,0x3121,0x3221,0x3325,0x3421,0x3525,0x3625,0x3721,0x3829,0x392d,0x4011,0x4115,0x4215,0x4311,0x4415,0x4511,
	0x4001,0x4105,0x4205,0x4301,0x4405,0x4501,0x4601,0x4705,0x480d,0x4909,0x5015,0x5111,0x5211,0x5315,0x5411,0x5515,
	0x5005,0x5101,0x5201,0x5305,0x5401,0x5505,0x5605,0x5701,0x5809,0x590d,0x6035,0x6131,0x6231,0x6335,0x6431,0x6535,
	0x6025,0x6121,0x6221,0x6325,0x6421,0x6525,0x6625,0x6721,0x6829,0x692d,0x7031,0x7135,0x7235,0x7331,0x7435,0x7531,
	0x7021,0x7125,0x7225,0x7321,0x7425,0x7521,0x7621,0x7725,0x782d,0x7929,0x8091,0x8195,0x8295,0x8391,0x8495,0x8591,
	0x8081,0x8185,0x8285,0x8381,0x8485,0x8581,0x8681,0x8785,0x888d,0x8989,0x9095,0x9191,0x9291,0x9395,0x9491,0x9595,
	0x9085,0x9181,0x9281,0x9385,0x9481,0x9585,0x9685,0x9781,0x9889,0x998d,0xa0b5,0xa1b1,0xa2b1,0xa3b5,0xa4b1,0xa5b5,
	0xa0a5,0xa1a1,0xa2a1,0xa3a5,0xa4a1,0xa5a5,0xa6a5,0xa7a1,0xa8a9,0xa9ad,0xb0b1,0xb1b5,0xb2b5,0xb3b1,0xb4b5,0xb5b1,
	0xb0a1,0xb1a5,0xb2a5,0xb3a1,0xb4a5,0xb5a1,0xb6a1,0xb7a5,0xb8ad,0xb9a9,0xc095,0xc191,0xc291,0xc395,0xc491,0xc595,
	0xc085,0xc181,0xc281,0xc385,0xc481,0xc585,0xc685,0xc781,0xc889,0xc98d,0xd091,0xd195,0xd295,0xd391,0xd495,0xd591,
	0xd081,0xd185,0xd285,0xd381,0xd485,0xd581,0xd681,0xd785,0xd88d,0xd989,0xe0b1,0xe1b5,0xe2b5,0xe3b1,0xe4b5,0xe5b1,
	0xe0a1,0xe1a5,0xe2a5,0xe3a1,0xe4a5,0xe5a1,0xe6a1,0xe7a5,0xe8ad,0xe9a9,0xf0b5,0xf1b1,0xf2b1,0xf3b5,0xf4b1,0xf5b5,
	0xf0a5,0xf1a1,0xf2a1,0xf3a5,0xf4a1,0xf5a5,0xf6a5,0xf7a1,0xf8a9,0xf9ad,0x0055,0x0111,0x0211,0x0315,0x0411,0x0515,
	0x0045,0x0101,0x0201,0x0305,0x0401,0x0505,0x0605,0x0701,0x0809,0x090d,0x1011,0x1115,0x1215,0x1311,0x1415,0x1511,
	0x1001,0x1105,0x1205,0x1301,0x1405,0x1501,0x1601,0x1705,0x180d,0x1909,0x2031,0x2135,0x2235,0x2331,0x2435,0x2531,
	0x2021,0x2125,0x2225,0x2321,0x2425,0x2521,0x2621,0x2725,0x282d,0x2929,0x3035,0x3131,0x3231,0x3335,0x3431,0x3535,
	0x3025,0x3121,0x3221,0x3325,0x3421,0x3525,0x3625,0x3721,0x3829,0x392d,0x4011,0x4115,0x4215,0x4311,0x4415,0x4511,
	0x4001,0x4105,0x4205,0x4301,0x4405,0x4501,0x4601,0x4705,0x480d,0x4909,0x5015,0x5111,0x5211,0x5315,0x5411,0x5515,
	0x5005,0x5101,0x5201,0x5305,0x5401,0x5505,0x5605,0x5701,0x5809,0x590d,0x6035,0x6131,0x6231,0x6335,0x6431,0x6535,
	0x0604,0x0700,0x0808,0x090c,0x0a0c,0x0b08,0x0c0c,0x0d08,0x0e08,0x0f0c,0x1010,0x1114,0x1214,0x1310,0x1414,0x1510,
	0x1600,0x1704,0x180c,0x1908,0x1a08,0x1b0c,0x1c08,0x1d0c,0x1e0c,0x1f08,0x2030,0x2134,0x2234,0x2330,0x2434,0x2530,
	0x2620,0x2724,0x282c,0x2928,0x2a28,0x2b2c,0x2c28,0x2d2c,0x2e2c,0x2f28,0x3034,0x3130,0x3230,0x3334,0x3430,0x3534,
	0x3624,0x3720,0x3828,0x392c,0x3a2c,0x3b28,0x3c2c,0x3d28,0x3e28,0x3f2c,0x4010,0x4114,0x4214,0x4310,0x4414,0x4510,
	0x4600,0x4704,0x480c,0x4908,0x4a08,0x4b0c,0x4c08,0x4d0c,0x4e0c,0x4f08,0x5014,0x5110,0x5210,0x5314,0x5410,0x5514,
	0x5604,0x5700,0x5808,0x590c,0x5a0c,0x5b08,0x5c0c,0x5d08,0x5e08,0x5f0c,0x6034,0x6130,0x6230,0x6334,0x6430,0x6534,
	0x6624,0x6720,0x6828,0x692c,0x6a2c,0x6b28,0x6c2c,0x6d28,0x6e28,0x6f2c,0x7030,0x7134,0x7234,0x7330,0x7434,0x7530,
	0x7620,0x7724,0x782c,0x7928,0x7a28,0x7b2c,0x7c28,0x7d2c,0x7e2c,0x7f28,0x8090,0x8194,0x8294,0x8390,0x8494,0x8590,
	0x8680,0x8784,0x888c,0x8988,0x8a88,0x8b8c,0x8c88,0x8d8c,0x8e8c,0x8f88,0x9094,0x9190,0x9290,0x9394,0x9490,0x9594,
	0x9684,0x9780,0x9888,0x998c,0x9a8c,0x9b88,0x9c8c,0x9d88,0x9e88,0x9f8c,0x0055,0x0111,0x0211,0x0315,0x0411,0x0515,
	0x0605,0x0701,0x0809,0x090d,0x0a0d,0x0b09,0x0c0d,0x0d09,0x0e09,0x0f0d,0x1011,0x1115,0x1215,0x1311,0x1415,0x1511,
	0x1601,0x1705,0x180d,0x1909,0x1a09,0x1b0d,0x1c09,0x1d0d,0x1e0d,0x1f09,0x2031,0x2135,0x2235,0x2331,0x2435,0x2531,
	0x2621,0x2725,0x282d,0x2929,0x2a29,0x2b2d,0x2c29,0x2d2d,0x2e2d,0x2f29,0x3035,0x3131,0x3231,0x3335,0x3431,0x3535,
	0x3625,0x3721,0x3829,0x392d,0x3a2d,0x3b29,0x3c2d,0x3d29,0x3e29,0x3f2d,0x4011,0x4115,0x4215,0x4311,0x4415,0x4511,
	0x4601,0x4705,0x480d,0x4909,0x4a09,0x4b0d,0x4c09,0x4d0d,0x4e0d,0x4f09,0x5015,0x5111,0x5211,0x5315,0x5411,0x5515,
	0x5605,0x5701,0x5809,0x590d,0x5a0d,0x5b09,0x5c0d,0x5d09,0x5e09,0x5f0d,0x6035,0x6131,0x6231,0x6335,0x6431,0x6535,
	0x6625,0x6721,0x6829,0x692d,0x6a2d,0x6b29,0x6c2d,0x6d29,0x6e29,0x6f2d,0x7031,0x7135,0x7235,0x7331,0x7435,0x7531,
	0x7621,0x7725,0x782d,0x7929,0x7a29,0x7b2d,0x7c29,0x7d2d,0x7e2d,0x7f29,0x8091,0x8195,0x8295,0x8391,0x8495,0x8591,
	0x8681,0x8785,0x888d,0x8989,0x8a89,0x8b8d,0x8c89,0x8d8d,0x8e8d,0x8f89,0x9095,0x9191,0x9291,0x9395,0x9491,0x9595,
	0x9685,0x9781,0x9889,0x998d,0x9a8d,0x9b89,0x9c8d,0x9d89,0x9e89,0x9f8d,0xa0b5,0xa1b1,0xa2b1,0xa3b5,0xa4b1,0xa5b5,
	0xa6a5,0xa7a1,0xa8a9,0xa9ad,0xaaad,0xaba9,0xacad,0xada9,0xaea9,0xafad,0xb0b1,0xb1b5,0xb2b5,0xb3b1,0xb4b5,0xb5b1,
	0xb6a1,0xb7a5,0xb8ad,0xb9a9,0xbaa9,0xbbad,0xbca9,0xbdad,0xbead,0xbfa9,0xc095,0xc191,0xc291,0xc395,0xc491,0xc595,
	0xc685,0xc781,0xc889,0xc98d,0xca8d,0xcb89,0xcc8d,0xcd89,0xce89,0xcf8d,0xd091,0xd195,0xd295,0xd391,0xd495,0xd591,
	0xd681,0xd785,0xd88d,0xd989,0xda89,0xdb8d,0xdc89,0xdd8d,0xde8d,0xdf89,0xe0b1,0xe1b5,0xe2b5,0xe3b1,0xe4b5,0xe5b1,
	0xe6a1,0xe7a5,0xe8ad,0xe9a9,0xeaa9,0xebad,0xeca9,0xedad,0xeead,0xefa9,0xf0b5,0xf1b1,0xf2b1,0xf3b5,0xf4b1,0xf5b5,
	0xf6a5,0xf7a1,0xf8a9,0xf9ad,0xfaad,0xfba9,0xfcad,0xfda9,0xfea9,0xffad,0x0055,0x0111,0x0211,0x0315,0x0411,0x0515,
	0x0605,0x0701,0x0809,0x090d,0x0a0d,0x0b09,0x0c0d,0x0d09,0x0e09,0x0f0d,0x1011,0x1115,0x1215,0x1311,0x1415,0x1511,
	0x1601,0x1705,0x180d,0x1909,0x1a09,0x1b0d,0x1c09,0x1d0d,0x1e0d,0x1f09,0x2031,0x2135,0x2235,0x2331,0x2435,0x2531,
	0x2621,0x2725,0x282d,0x2929,0x2a29,0x2b2d,0x2c29,0x2d2d,0x2e2d,0x2f29,0x3035,0x3131,0x3231,0x3335,0x3431,0x3535,
	0x3625,0x3721,0x3829,0x392d,0x3a2d,0x3b29,0x3c2d,0x3d29,0x3e29,0x3f2d,0x4011,0x4115,0x4215,0x4311,0x4415,0x4511,
	0x4601,0x4705,0x480d,0x4909,0x4a09,0x4b0d,0x4c09,0x4d0d,0x4e0d,0x4f09,0x5015,0x5111,0x5211,0x5315,0x5411,0x5515,
	0x5605,0x5701,0x5809,0x590d,0x5a0d,0x5b09,0x5c0d,0x5d09,0x5e09,0x5f0d,0x6035,0x6131,0x6231,0x6335,0x6431,0x6535,
	0x0046,0x0102,0x0202,0x0306,0x0402,0x0506,0x0606,0x0702,0x080a,0x090e,0x0402,0x0506,0x0606,0x0702,0x080a,0x090e,
	0x1002,0x1106,0x1206,0x1302,0x1406,0x1502,0x1602,0x1706,0x180e,0x190a,0x1406,0x1502,0x1602,0x1706,0x180e,0x190a,
	0x2022,0x2126,0x2226,0x2322,0x2426,0x2522,0x2622,0x2726,0x282e,0x292a,0x2426,0x2522,0x2622,0x2726,0x282e,0x292a,
	0x3026,0x3122,0x3222,0x3326,0x3422,0x3526,0x3626,0x3722,0x382a,0x392e,0x3422,0x3526,0x3626,0x3722,0x382a,0x392e,
	0x4002,0x4106,0x4206,0x4302,0x4406,0x4502,0x4602,0x4706,0x480e,0x490a,0x4406,0x4502,0x4602,0x4706,0x480e,0x490a,
	0x5006,0x5102,0x5202,0x5306,0x5402,0x5506,0x5606,0x5702,0x580a,0x590e,0x5402,0x5506,0x5606,0x5702,0x580a,0x590e,
	0x6026,0x6122,0x6222,0x6326,0x6422,0x6526,0x6626,0x6722,0x682a,0x692e,0x6422,0x6526,0x6626,0x6722,0x682a,0x692e,
	0x7022,0x7126,0x7226,0x7322,0x7426,0x7522,0x7622,0x7726,0x782e,0x792a,0x7426,0x7522,0x7622,0x7726,0x782e,0x792a,
	0x8082,0x8186,0x8286,0x8382,0x8486,0x8582,0x8682,0x8786,0x888e,0x898a,0x8486,0x8582,0x8682,0x8786,0x888e,0x898a,
	0x9086,0x9182,0x9282,0x9386,0x9482,0x9586,0x9686,0x9782,0x988a,0x998e,0x3423,0x3527,0x3627,0x3723,0x382b,0x392f,
	0x4003,0x4107,0x4207,0x4303,0x4407,0x4503,0x4603,0x4707,0x480f,0x490b,0x4407,0x4503,0x4603,0x4707,0x480f,0x490b,
	0x5007,0x5103,0x5203,0x5307,0x5403,0x5507,0x5607,0x5703,0x580b,0x590f,0x5403,0x5507,0x5607,0x5703,0x580b,0x590f,
	0x6027,0x6123,0x6223,0x6327,0x6423,0x6527,0x6627,0x6723,0x682b,0x692f,0x6423,0x6527,0x6627,0x6723,0x682b,0x692f,
	0x7023,0x7127,0x7227,0x7323,0x7427,0x7523,0x7623,0x7727,0x782f,0x792b,0x7427,0x7523,0x7623,0x7727,0x782f,0x792b,
	0x8083,0x8187,0x8287,0x8383,0x8487,0x8583,0x8683,0x8787,0x888f,0x898b,0x8487,0x8583,0x8683,0x8787,0x888f,0x898b,
	0x9087,0x9183,0x9283,0x9387,0x9483,0x9587,0x9687,0x9783,0x988b,0x998f,0x9483,0x9587,0x9687,0x9783,0x988b,0x998f,
	0xa0a7,0xa1a3,0xa2a3,0xa3a7,0xa4a3,0xa5a7,0xa6a7,0xa7a3,0xa8ab,0xa9af,0xa4a3,0xa5a7,0xa6a7,0xa7a3,0xa8ab,0xa9af,
	0xb0a3,0xb1a7,0xb2a7,0xb3a3,0xb4a7,0xb5a3,0xb6a3,0xb7a7,0xb8af,0xb9ab,0xb4a7,0xb5a3,0xb6a3,0xb7a7,0xb8af,0xb9ab,
	0xc087,0xc183,0xc283,0xc387,0xc483,0xc587,0xc687,0xc783,0xc88b,0xc98f,0xc483,0xc587,0xc687,0xc783,0xc88b,0xc98f,
	0xd083,0xd187,0xd287,0xd383,0xd487,0xd583,0xd683,0xd787,0xd88f,0xd98b,0xd487,0xd583,0xd683,0xd787,0xd88f,0xd98b,
	0xe0a3,0xe1a7,0xe2a7,0xe3a3,0xe4a7,0xe5a3,0xe6a3,0xe7a7,0xe8af,0xe9ab,0xe4a7,0xe5a3,0xe6a3,0xe7a7,0xe8af,0xe9ab,
	0xf0a7,0xf1a3,0xf2a3,0xf3a7,0xf4a3,0xf5a7,0xf6a7,0xf7a3,0xf8ab,0xf9af,0xf4a3,0xf5a7,0xf6a7,0xf7a3,0xf8ab,0xf9af,
	0x0047,0x0103,0x0203,0x0307,0x0403,0x0507,0x0607,0x0703,0x080b,0x090f,0x0403,0x0507,0x0607,0x0703,0x080b,0x090f,
	0x1003,0x1107,0x1207,0x1303,0x1407,0x1503,0x1603,0x1707,0x180f,0x190b,0x1407,0x1503,0x1603,0x1707,0x180f,0x190b,
	0x2023,0x2127,0x2227,0x2323,0x2427,0x2523,0x2623,0x2727,0x282f,0x292b,0x2427,0x2523,0x2623,0x2727,0x282f,0x292b,
	0x3027,0x3123,0x3223,0x3327,0x3423,0x3527,0x3627,0x3723,0x382b,0x392f,0x3423,0x3527,0x3627,0x3723,0x382b,0x392f,
	0x4003,0x4107,0x4207,0x4303,0x4407,0x4503,0x4603,0x4707,0x480f,0x490b,0x4407,0x4503,0x4603,0x4707,0x480f,0x490b,
	0x5007,0x5103,0x5203,0x5307,0x5403,0x5507,0x5607,0x5703,0x580b,0x590f,0x5403,0x5507,0x5607,0x5703,0x580b,0x590f,
	0x6027,0x6123,0x6223,0x6327,0x6423,0x6527,0x6627,0x6723,0x682b,0x692f,0x6423,0x6527,0x6627,0x6723,0x682b,0x692f,
	0x7023,0x7127,0x7227,0x7323,0x7427,0x7523,0x7623,0x7727,0x782f,0x792b,0x7427,0x7523,0x7623,0x7727,0x782f,0x792b,
	0x8083,0x8187,0x8287,0x8383,0x8487,0x8583,0x8683,0x8787,0x888f,0x898b,0x8487,0x8583,0x8683,0x8787,0x888f,0x898b,
	0x9087,0x9183,0x9283,0x9387,0x9483,0x9587,0x9687,0x9783,0x988b,0x998f,0x9483,0x9587,0x9687,0x9783,0x988b,0x998f,
	0xfabe,0xfbba,0xfcbe,0xfdba,0xfeba,0xffbe,0x0046,0x0102,0x0202,0x0306,0x0402,0x0506,0x0606,0x0702,0x080a,0x090e,
	0x0a1e,0x0b1a,0x0c1e,0x0d1a,0x0e1a,0x0f1e,0x1002,0x1106,0x1206,0x1302,0x1406,0x1502,0x1602,0x1706,0x180e,0x190a,
	0x1a1a,0x1b1e,0x1c1a,0x1d1e,0x1e1e,0x1f1a,0x2022,0x2126,0x2226,0x2322,0x2426,0x2522,0x2622,0x2726,0x282e,0x292a,
	0x2a3a,0x2b3e,0x2c3a,0x2d3e,0x2e3e,0x2f3a,0x3026,0x3122,0x3222,0x3326,0x3422,0x3526,0x3626,0x3722,0x382a,0x392e,
	0x3a3e,0x3b3a,0x3c3e,0x3d3a,0x3e3a,0x3f3e,0x4002,0x4106,0x4206,0x4302,0x4406,0x4502,0x4602,0x4706,0x480e,0x490a,
	0x4a1a,0x4b1e,0x4c1a,0x4d1e,0x4e1e,0x4f1a,0x5006,0x5102,0x5202,0x5306,0x5402,0x5506,0x5606,0x5702,0x580a,0x590e,
	0x5a1e,0x5b1a,0x5c1e,0x5d1a,0x5e1a,0x5f1e,0x6026,0x6122,0x6222,0x6326,0x6422,0x6526,0x6626,0x6722,0x682a,0x692e,
	0x6a3e,0x6b3a,0x6c3e,0x6d3a,0x6e3a,0x6f3e,0x7022,0x7126,0x7226,0x7322,0x7426,0x7522,0x7622,0x7726,0x782e,0x792a,
	0x7a3a,0x7b3e,0x7c3a,0x7d3e,0x7e3e,0x7f3a,0x8082,0x8186,0x8286,0x8382,0x8486,0x8582,0x8682,0x8786,0x888e,0x898a,
	0x8a9a,0x8b9e,0x8c9a,0x8d9e,0x8e9e,0x8f9a,0x9086,0x9182,0x9282,0x9386,0x3423,0x3527,0x3627,0x3723,0x382b,0x392f,
	0x3a3f,0x3b3b,0x3c3f,0x3d3b,0x3e3b,0x3f3f,0x4003,0x4107,0x4207,0x4303,0x4407,0x4503,0x4603,0x4707,0x480f,0x490b,
	0x4a1b,0x4b1f,0x4c1b,0x4d1f,0x4e1f,0x4f1b,0x5007,0x5103,0x5203,0x5307,0x5403,0x5507,0x5607,0x5703,0x580b,0x590f,
	0x5a1f,0x5b1b,0x5c1f,0x5d1b,0x5e1b,0x5f1f,0x6027,0x6123,0x6223,0x6327,0x6423,0x6527,0x6627,0x6723,0x682b,0x692f,
	0x6a3f,0x6b3b,0x6c3f,0x6d3b,0x6e3b,0x6f3f,0x7023,0x7127,0x7227,0x7323,0x7427,0x7523,0x7623,0x7727,0x782f,0x792b,
	0x7a3b,0x7b3f,0x7c3b,0x7d3f,0x7e3f,0x7f3b,0x8083,0x8187,0x8287,0x8383,0x8487,0x8583,0x8683,0x8787,0x888f,0x898b,
	0x8a9b,0x8b9f,0x8c9b,0x8d9f,0x8e9f,0x8f9b,0x9087,0x9183,0x9283,0x9387,0x9483,0x9587,0x9687,0x9783,0x988b,0x998f,
	0x9a9f,0x9b9b,0x9c9f,0x9d9b,0x9e9b,0x9f9f,0xa0a7,0xa1a3,0xa2a3,0xa3a7,0xa4a3,0xa5a7,0xa6a7,0xa7a3,0xa8ab,0xa9af,
	0xaabf,0xabbb,0xacbf,0xadbb,0xaebb,0xafbf,0xb0a3,0xb1a7,0xb2a7,0xb3a3,0xb4a7,0xb5a3,0xb6a3,0xb7a7,0xb8af,0xb9ab,
	0xbabb,0xbbbf,0xbcbb,0xbdbf,0xbebf,0xbfbb,0xc087,0xc183,0xc283,0xc387,0xc483,0xc587,0xc687,0xc783,0xc88b,0xc98f,
	0xca9f,0xcb9b,0xcc9f,0xcd9b,0xce9b,0xcf9f,0xd083,0xd187,0xd287,0xd383,0xd487,0xd583,0xd683,0xd787,0xd88f,0xd98b,
	0xda9b,0xdb9f,0xdc9b,0xdd9f,0xde9f,0xdf9b,0xe0a3,0xe1a7,0xe2a7,0xe3a3,0xe4a7,0xe5a3,0xe6a3,0xe7a7,0xe8af,0xe9ab,
	0xeabb,0xebbf,0xecbb,0xedbf,0xeebf,0xefbb,0xf0a7,0xf1a3,0xf2a3,0xf3a7,0xf4a3,0xf5a7,0xf6a7,0xf7a3,0xf8ab,0xf9af,
	0xfabf,0xfbbb,0xfcbf,0xfdbb,0xfebb,0xffbf,0x0047,0x0103,0x0203,0x0307,0x0403,0x0507,0x0607,0x0703,0x080b,0x090f,
	0x0a1f,0x0b1b,0x0c1f,0x0d1b,0x0e1b,0x0f1f,0x1003,0x1107,0x1207,0x1303,0x1407,0x1503,0x1603,0x1707,0x180f,0x190b,
	0x1a1b,0x1b1f,0x1c1b,0x1d1f,0x1e1f,0x1f1b,0x2023,0x2127,0x2227,0x2323,0x2427,0x2523,0x2623,0x2727,0x282f,0x292b,
	0x2a3b,0x2b3f,0x2c3b,0x2d3f,0x2e3f,0x2f3b,0x3027,0x3123,0x3223,0x3327,0x3423,0x3527,0x3627,0x3723,0x382b,0x392f,
	0x3a3f,0x3b3b,0x3c3f,0x3d3b,0x3e3b,0x3f3f,0x4003,0x4107,0x4207,0x4303,0x4407,0x4503,0x4603,0x4707,0x480f,0x490b,
	0x4a1b,0x4b1f,0x4c1b,0x4d1f,0x4e1f,0x4f1b,0x5007,0x5103,0x5203,0x5307,0x5403,0x5507,0x5607,0x5703,0x580b,0x590f,
	0x5a1f,0x5b1b,0x5c1f,0x5d1b,0x5e1b,0x5f1f,0x6027,0x6123,0x6223,0x6327,0x6423,0x6527,0x6627,0x6723,0x682b,0x692f,
	0x6a3f,0x6b3b,0x6c3f,0x6d3b,0x6e3b,0x6f3f,0x7023,0x7127,0x7227,0x7323,0x7427,0x7523,0x7623,0x7727,0x782f,0x792b,
	0x7a3b,0x7b3f,0x7c3b,0x7d3f,0x7e3f,0x7f3b,0x8083,0x8187,0x8287,0x8383,0x8487,0x8583,0x8683,0x8787,0x888f,0x898b,
	0x8a9b,0x8b9f,0x8c9b,0x8d9f,0x8e9f,0x8f9b,0x9087,0x9183,0x9283,0x9387,0x9483,0x9587,0x9687,0x9783,0x988b,0x998f
};

// opecode definitions

#define INR(v) { \
	uint8_t hc = ((v & 0x0f) == 0x0f) ? HF : 0; \
	++v; \
	_F = (_F & CF) | ZSP[v] | hc; \
}
#define DCR(v) { \
	uint8_t hc = ((v & 0x0f) == 0x00) ? HF : 0; \
	--v; \
	_F = (_F & CF) | ZSP[v] | hc | NF; \
}
#define MVI(v) { \
	v = FETCH8(); \
}
#ifdef HAS_I8085
#define ANA(v) { \
	_A &= v; \
	_F = ZSP[_A]; \
	_F |= HF; \
}
#else
#define ANA(v) { \
	int i = (((_A | v) >> 3) & 1) * HF; \
	_A &= v; \
	_F = ZSP[_A]; \
	_F |= i; \
}
#endif
#define ORA(v) { \
	_A |= v; \
	_F = ZSP[_A]; \
}
#define XRA(v) { \
	_A ^= v; \
	_F = ZSP[_A]; \
}
#define RLC() { \
	_A = (_A << 1) | (_A >> 7); \
	_F = (_F & 0xfe) | (_A & CF); \
}
#define RRC() { \
	_F = (_F & 0xfe) | (_A & CF); \
	_A = (_A >> 1) | (_A << 7); \
}
#define RAL() { \
	int c = _F & CF; \
	_F = (_F & 0xfe) | (_A >> 7); \
	_A = (_A << 1) | c; \
}
#define RAR() { \
	int c = (_F & CF) << 7; \
	_F = (_F & 0xfe) | (_A & CF); \
	_A = (_A >> 1) | c; \
}
#define ADD(v) { \
	int q = _A + v; \
	_F = ZSP[q & 255] | ((q >> 8) & CF) | ((_A ^ q ^ v) & HF) | (((v ^ _A ^ SF) & (v ^ q) & SF) >> 5); \
	_A = q; \
}
#define ADC(v) {\
	int q = _A + v + (_F & CF); \
	_F = ZSP[q & 255] | ((q >> 8) & CF) | ((_A ^ q ^ v) & HF) | (((v ^ _A ^ SF) & (v ^ q) & SF) >> 5); \
	_A = q; \
}
#define SUB(v) { \
	int q = _A - v; \
	_F = ZSP[q & 255] | ((q >> 8) & CF) | NF | ((_A ^ q ^ v) & HF) | (((v ^ _A) & (_A ^ q) & SF) >> 5); \
	_A = q; \
}
#define SBB(v) { \
	int q = _A - v - (_F & CF); \
	_F = ZSP[q & 255] | ((q >> 8) & CF) | NF | ((_A ^ q ^ v) & HF) | (((v ^ _A) & (_A ^ q) & SF) >> 5); \
	_A = q; \
}
#define CMP(v) { \
	int q = _A - v; \
	_F = ZSP[q & 255] | ((q >> 8) & CF) | NF | ((_A ^ q ^ v) & HF) | (((v ^ _A) & (_A ^ q) & SF) >> 5); \
}
#define DAD(v) { \
	int q = HL + v; \
	_F = (_F & ~(HF + CF)) | (((HL ^ q ^ v) >> 8) & HF) | ((q >> 16) & CF); \
	HL = q; \
}
#define RET(c) { \
	if(c) { \
		count -= 6; \
		PC = POP16(); \
	}	\
}
#ifdef HAS_I8085
#define JMP(c) { \
	if(c) { \
		PC = FETCH16(); \
	} else { \
		PC += 2; \
		count += 3; \
	} \
}
#define CALL(c) { \
	if(c) { \
		uint16_t a = FETCH16(); \
		count -= 7; \
		PUSH16(PC); \
		PC = a; \
	} else { \
		PC += 2; \
		count += 2; \
	} \
}
#else
#define JMP(c) { \
	if(c) { \
		PC = FETCH16(); \
	} else { \
		PC += 2; \
	} \
}
#define CALL(c) { \
	if(c) { \
		uint16_t a = FETCH16(); \
		count -= 6; \
		PUSH16(PC); \
		PC = a; \
	} else { \
		PC += 2; \
	} \
}
#endif
#define RST(n) { \
	PUSH16(PC); \
	PC = 8 * n; \
}
#define DSUB() {\
	int q = _L - _C; \
	_F = ZS[q & 255] | ((q >> 8) & CF) | NF | ((_L ^ q ^ _C) & HF) | (((_C ^ _L) & (_L ^ q) & SF) >> 5); \
	_L = q; \
	q = _H - _B - (_F & CF); \
	_F = ZS[q & 255] | ((q >> 8) & CF) | NF | ((_H ^ q ^ _B) & HF) | (((_B ^ _H) & (_H ^ q) & SF) >> 5); \
	if(_L != 0) \
		_F &= ~ZF; \
}
#define INT(v) { \
	if(afterHALT) { \
		PC++; afterHALT = 0; \
	} \
	PUSH16(PC); PC = (v); \
}

// main

void I8080::initialize()
{
#ifdef USE_DEBUGGER
	d_mem_stored = d_mem;
	d_io_stored = d_io;
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void I8080::reset()
{
	// reset
	AF = BC = DE = HL = 0;
	PC = CPU_START_ADDR;
	SP = 0;
	IM = IM_M5 | IM_M6 | IM_M7;
	afterHALT = BUSREQ = false;
	
	count = 0;
}

void I8080::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CPU_NMI) {
		if(data & mask) {
			IM |= IM_NMI;
		} else {
			IM &= ~IM_NMI;
		}
	} else if(id == SIG_CPU_BUSREQ) {
		BUSREQ = ((data & mask) != 0);
		write_signals(&outputs_busack, BUSREQ ? 0xffffffff : 0);
	} else if(id == SIG_I8080_INTR) {
		if(data & mask) {
			IM |= IM_INT;
		} else {
			IM &= ~IM_INT;
		}
#ifdef HAS_I8085
	} else if(id == SIG_I8085_RST5) {
		if(data & mask) {
			IM |= IM_I5;
		} else {
			IM &= ~IM_I5;
		}
	} else if(id == SIG_I8085_RST6) {
		if(data & mask) {
			IM |= IM_I6;
		} else {
			IM &= ~IM_I6;
		}
	} else if(id == SIG_I8085_RST7) {
		if(data & mask) {
			IM |= IM_I7;
		} else {
			IM &= ~IM_I7;
		}
	} else if(id == SIG_I8085_SID) {
		SID = ((data & mask) != 0);
#endif
	}
}

uint32_t I8080::read_signal(int ch)
{
	if(ch == SIG_I8080_INTE) {
		if(!afterEI && (IM & IM_IEN)) {
			return 0xffffffff;
		}
	}
	return 0;
}

void I8080::set_intr_line(bool line, bool pending, uint32_t bit)
{
	if(line) {
		IM |= IM_INT;
	} else {
		IM &= ~IM_INT;
	}
}

int I8080::run(int clock)
{
	if(clock == -1) {
		if(BUSREQ) {
			// don't run cpu!
#ifdef USE_DEBUGGER
			total_count += 1;
#endif
			return 1;
		} else {
			// run only one opcode
			count = 0;
			run_one_opecode();
			return -count;
		}
	} else {
		count += clock;
		int first_count = count;
		
		// run cpu while given clocks
		while(count > 0 && !BUSREQ) {
			run_one_opecode();
		}
		// if busreq is raised, spin cpu while remained clock
		if(count > 0 && BUSREQ) {
#ifdef USE_DEBUGGER
			total_count += count;
#endif
			count = 0;
		}
		return first_count - count;
	}
}

void I8080::run_one_opecode()
{
	// rune one opecode
#ifdef USE_DEBUGGER
	bool now_debugging = d_debugger->now_debugging;
	if(now_debugging) {
		d_debugger->check_break_points(PC);
		if(d_debugger->now_suspended) {
			d_debugger->now_waiting = true;
			emu->start_waiting_in_debugger();
			while(d_debugger->now_debugging && d_debugger->now_suspended) {
				emu->process_waiting_in_debugger();
			}
			emu->finish_waiting_in_debugger();
			d_debugger->now_waiting = false;
		}
		if(d_debugger->now_debugging) {
			d_mem = d_io = d_debugger;
		} else {
			now_debugging = false;
		}
		
		afterHALT = afterEI = false;
		d_debugger->add_cpu_trace(PC);
		int first_count = count;
		OP(FETCHOP());
		total_count += first_count - count;
		if(!afterEI) {
			check_interrupt();
		}
		
		if(now_debugging) {
			if(!d_debugger->now_going) {
				d_debugger->now_suspended = true;
			}
			d_mem = d_mem_stored;
			d_io = d_io_stored;
		}
	} else {
#endif
		afterHALT = afterEI = false;
#ifdef USE_DEBUGGER
		d_debugger->add_cpu_trace(PC);
		int first_count = count;
#endif
		OP(FETCHOP());
#ifdef USE_DEBUGGER
		total_count += first_count - count;
#endif
		if(!afterEI) {
			check_interrupt();
		}
#ifdef USE_DEBUGGER
	}
#endif
	
	// ei: run next opecode
	if(afterEI) {
#ifdef USE_DEBUGGER
		bool now_debugging = d_debugger->now_debugging;
		if(now_debugging) {
			d_debugger->check_break_points(PC);
			if(d_debugger->now_suspended) {
				d_debugger->now_waiting = true;
				emu->start_waiting_in_debugger();
				while(d_debugger->now_debugging && d_debugger->now_suspended) {
					emu->process_waiting_in_debugger();
				}
				emu->finish_waiting_in_debugger();
				d_debugger->now_waiting = false;
			}
			if(d_debugger->now_debugging) {
				d_mem = d_io = d_debugger;
			} else {
				now_debugging = false;
			}
			
			afterHALT = false;
			d_debugger->add_cpu_trace(PC);
			int first_count = count;
			OP(FETCHOP());
			total_count += first_count - count;
			d_pic->notify_intr_ei();
			check_interrupt();
			
			if(now_debugging) {
				if(!d_debugger->now_going) {
					d_debugger->now_suspended = true;
				}
				d_mem = d_mem_stored;
				d_io = d_io_stored;
			}
		} else {
#endif
			afterHALT = false;
#ifdef USE_DEBUGGER
			d_debugger->add_cpu_trace(PC);
			int first_count = count;
#endif
			OP(FETCHOP());
#ifdef USE_DEBUGGER
			total_count += first_count - count;
#endif
			d_pic->notify_intr_ei();
			check_interrupt();
#ifdef USE_DEBUGGER
		}
#endif
	}
}

void I8080::check_interrupt()
{
#ifdef USE_DEBUGGER
	int first_count = count;
#endif
	// check interrupt
	if(IM & IM_REQ) {
		if(IM & IM_NMI) {
			INT(0x24);
			count -= 5;	// unknown
			RIM_IEN = IM & IM_IEN;
			IM &= ~(IM_IEN | IM_NMI);
		} else if(IM & IM_IEN) {
#ifdef HAS_I8085
#ifdef _FP200
			if(/*!(IM & IM_M7) &&*/ (IM & IM_I7)) {
#else
			if(!(IM & IM_M7) && (IM & IM_I7)) {
#endif
				INT(0x3c);
				count -= 7;	// unknown
				RIM_IEN = 0;
				IM &= ~(IM_IEN | IM_I7);
			} else if(!(IM & IM_M6) && (IM & IM_I6)) {
				INT(0x34);
				count -= 7;	// unknown
				RIM_IEN = 0;
				IM &= ~(IM_IEN | IM_I6);
			} else if(!(IM & IM_M5) && (IM & IM_I5)) {
				INT(0x2c);
				count -= 7;	// unknown
				RIM_IEN = 0;
				IM &= ~(IM_IEN | IM_I5);
			} else
#endif
			if(IM & IM_INT) {
				uint32_t vector = ACK_INTR();
				uint8_t v0 = vector;
				uint16_t v12 = vector >> 8;
				// support JMP/CALL/RST only
				count -= cc_op[v0];
				switch(v0) {
				case 0x00:	// NOP
					break;
				case 0xc3:	// JMP
					PC = v12;
					break;
				case 0xcd:	// CALL
					PUSH16(PC);
					PC = v12;
#ifdef HAS_I8085
					count -= 7;
#else
					count -= 6;
#endif
					break;
				case 0xc7:	// RST 0
					RST(0);
					break;
				case 0xcf:	// RST 1
					RST(1);
					break;
				case 0xd7:	// RST 2
					RST(2);
					break;
				case 0xdf:	// RST 3
					RST(3);
					break;
				case 0xe7:	// RST 4
					RST(4);
					break;
				case 0xef:	// RST 5
					RST(5);
					break;
				case 0xf7:	// RST 6
					RST(6);
					break;
				case 0xff:	// RST 7
					RST(7);
					break;
				}
				RIM_IEN = 0;
				IM &= ~(IM_IEN | IM_INT);
			}
		}
	}
#ifdef USE_DEBUGGER
	total_count += first_count - count;
#endif
}

void I8080::OP(uint8_t code)
{
	uint8_t tmp8;
	uint16_t tmp16;
	
	prevPC = PC - 1;
	count -= cc_op[code];
	
	switch(code) {
	case 0x00: // NOP
		break;
	case 0x01: // LXI B,nnnn
		BC = FETCH16();
		break;
	case 0x02: // STAX B
		WM8(BC, _A);
		break;
	case 0x03: // INX B
		BC++;
#ifdef HAS_I8085
		if(_C == 0x00) _F |= XF; else _F &= ~XF;
#endif
		break;
	case 0x04: // INR B
		INR(_B);
		break;
	case 0x05: // DCR B
		DCR(_B);
		break;
	case 0x06: // MVI B,nn
		MVI(_B);
		break;
	case 0x07: // RLC
		RLC();
		break;
	case 0x08: // DSUB (NOP)
#ifdef HAS_I8085
		DSUB();
#endif
		break;
	case 0x09: // DAD B
		DAD(BC);
		break;
	case 0x0a: // LDAX B
		_A = RM8(BC);
		break;
	case 0x0b: // DCX B
		BC--;
#ifdef HAS_I8085
		if(_C == 0xff) _F |= XF; else _F &= ~XF;
#endif
		break;
	case 0x0c: // INR C
		INR(_C);
		break;
	case 0x0d: // DCR C
		DCR(_C);
		break;
	case 0x0e: // MVI C,nn
		MVI(_C);
		break;
	case 0x0f: // RRC
		RRC();
		break;
	case 0x10: // ASRH (NOP)
#ifdef HAS_I8085
		_F = (_F & ~CF) | (_L & CF);
		HL = (HL >> 1);
#endif
		break;
	case 0x11: // LXI D,nnnn
		DE = FETCH16();
		break;
	case 0x12: // STAX D
		WM8(DE, _A);
		break;
	case 0x13: // INX D
		DE++;
#ifdef HAS_I8085
		if(_E == 0x00) _F |= XF; else _F &= ~XF;
#endif
		break;
	case 0x14: // INR D
		INR(_D);
		break;
	case 0x15: // DCR D
		DCR(_D);
		break;
	case 0x16: // MVI D,nn
		MVI(_D);
		break;
	case 0x17: // RAL
		RAL();
		break;
	case 0x18: // RLDE (NOP)
#ifdef HAS_I8085
		_F = (_F & ~(CF | VF)) | (_D >> 7);
		DE = (DE << 1) | (DE >> 15);
		if(0 != (((DE >> 15) ^ _F) & CF)) {
			_F |= VF;
		}
#endif
		break;
	case 0x19: // DAD D
		DAD(DE);
		break;
	case 0x1a: // LDAX D
		_A = RM8(DE);
		break;
	case 0x1b: // DCX D
		DE--;
#ifdef HAS_I8085
		if(_E == 0xff) _F |= XF; else _F &= ~XF;
#endif
		break;
	case 0x1c: // INR E
		INR(_E);
		break;
	case 0x1d: // DCR E
		DCR(_E);
		break;
	case 0x1e: // MVI E,nn
		MVI(_E);
		break;
	case 0x1f: // RAR
		RAR();
		break;
	case 0x20: // RIM (NOP)
#ifdef HAS_I8085
		_A = (IM & 0x7f) | (SID ? 0x80 : 0) | RIM_IEN;
		RIM_IEN = 0;
#endif
		break;
	case 0x21: // LXI H,nnnn
		HL = FETCH16();
		break;
	case 0x22: // SHLD nnnn
		WM16(FETCH16(), HL);
		break;
	case 0x23: // INX H
		HL++;
#ifdef HAS_I8085
		if(_L == 0x00) _F |= XF; else _F &= ~XF;
#endif
		break;
	case 0x24: // INR H
		INR(_H);
		break;
	case 0x25: // DCR H
		DCR(_H);
		break;
	case 0x26: // MVI H,nn
		MVI(_H);
		break;
	case 0x27: // DAA
		tmp16 = _A;
		if(_F & CF) tmp16 |= 0x100;
		if(_F & HF) tmp16 |= 0x200;
		if(_F & NF) tmp16 |= 0x400;
		AF = DAA[tmp16];
#ifdef HAS_I8080
		_F &= 0xd5;
#endif
		break;
	case 0x28: // LDEH nn (NOP)
#ifdef HAS_I8085
		DE = (HL + FETCH8()) & 0xffff;
#endif
		break;
	case 0x29: // DAD H
		DAD(HL);
		break;
	case 0x2a: // LHLD nnnn
		HL = RM16(FETCH16());
		break;
	case 0x2b: // DCX H
		HL--;
#ifdef HAS_I8085
		if(_L == 0xff) _F |= XF; else _F &= ~XF;
#endif
		break;
	case 0x2c: // INR L
		INR(_L);
		break;
	case 0x2d: // DCR L
		DCR(_L);
		break;
	case 0x2e: // MVI L,nn
		MVI(_L);
		break;
	case 0x2f: // CMA
#ifdef HAS_I8085
		_A ^= 0xff;
		_F |= HF + NF;
#else
		_A ^= 0xff;
#endif
		break;
	case 0x30: // SIM (NOP)
#ifdef HAS_I8085
		if(_A & 0x40) {
			write_signals(&outputs_sod, (_A & 0x80) ? 0xffffffff : 0);
		}
		if(_A & 0x10) {
			IM &= ~IM_I7;
		}
		if(_A & 8) {
			IM = (IM & ~(IM_M5 | IM_M6 | IM_M7)) | (_A & (IM_M5 | IM_M6 | IM_M7));
		}
#endif
		break;
	case 0x31: // LXI SP,nnnn
		SP = FETCH16();
		break;
	case 0x32: // STA nnnn
		WM8(FETCH16(), _A);
		break;
	case 0x33: // INX SP
		SP++;
#ifdef HAS_I8085
		if((SP & 0xff) == 0) _F |= XF; else _F &= ~XF;
#endif
		break;
	case 0x34: // INR M
		tmp8 = RM8(HL);
		INR(tmp8);
		WM8(HL, tmp8);
		break;
	case 0x35: // DCR M
		tmp8 = RM8(HL);
		DCR(tmp8);
		WM8(HL, tmp8);
		break;
	case 0x36: // MVI M,nn
		WM8(HL, FETCH8());
		break;
	case 0x37: // STC
		_F = (_F & 0xfe) | CF;
		break;
	case 0x38: // LDES nn (NOP)
#ifdef HAS_I8085
		DE = (SP + FETCH8()) & 0xffff;
#endif
		break;
	case 0x39: // DAD SP
		DAD(SP);
		break;
	case 0x3a: // LDA nnnn
		_A = RM8(FETCH16());
		break;
	case 0x3b: // DCX SP
		SP--;
#ifdef HAS_I8085
		if((SP & 0xff) == 0xff) _F |= XF; else _F &= ~XF;
#endif
		break;
	case 0x3c: // INR A
		INR(_A);
		break;
	case 0x3d: // DCR A
		DCR(_A);
		break;
	case 0x3e: // MVI A,nn
		MVI(_A);
		break;
	case 0x3f: // CMC
		_F = (_F & 0xfe) | ((_F & CF)==1 ? 0 : 1);
		break;
	case 0x40: // MOV B,B
		break;
	case 0x41: // MOV B,C
		_B = _C;
		break;
	case 0x42: // MOV B,D
		_B = _D;
		break;
	case 0x43: // MOV B,E
		_B = _E;
		break;
	case 0x44: // MOV B,H
		_B = _H;
		break;
	case 0x45: // MOV B,L
		_B = _L;
		break;
	case 0x46: // MOV B,M
		_B = RM8(HL);
		break;
	case 0x47: // MOV B,A
		_B = _A;
		break;
	case 0x48: // MOV C,B
		_C = _B;
		break;
	case 0x49: // MOV C,C
		break;
	case 0x4a: // MOV C,D
		_C = _D;
		break;
	case 0x4b: // MOV C,E
		_C = _E;
		break;
	case 0x4c: // MOV C,H
		_C = _H;
		break;
	case 0x4d: // MOV C,L
		_C = _L;
		break;
	case 0x4e: // MOV C,M
		_C = RM8(HL);
		break;
	case 0x4f: // MOV C,A
		_C = _A;
		break;
	case 0x50: // MOV D,B
		_D = _B;
		break;
	case 0x51: // MOV D,C
		_D = _C;
		break;
	case 0x52: // MOV D,D
		break;
	case 0x53: // MOV D,E
		_D = _E;
		break;
	case 0x54: // MOV D,H
		_D = _H;
		break;
	case 0x55: // MOV D,L
		_D = _L;
		break;
	case 0x56: // MOV D,M
		_D = RM8(HL);
		break;
	case 0x57: // MOV D,A
		_D = _A;
		break;
	case 0x58: // MOV E,B
		_E = _B;
		break;
	case 0x59: // MOV E,C
		_E = _C;
		break;
	case 0x5a: // MOV E,D
		_E = _D;
		break;
	case 0x5b: // MOV E,E
		break;
	case 0x5c: // MOV E,H
		_E = _H;
		break;
	case 0x5d: // MOV E,L
		_E = _L;
		break;
	case 0x5e: // MOV E,M
		_E = RM8(HL);
		break;
	case 0x5f: // MOV E,A
		_E = _A;
		break;
	case 0x60: // MOV H,B
		_H = _B;
		break;
	case 0x61: // MOV H,C
		_H = _C;
		break;
	case 0x62: // MOV H,D
		_H = _D;
		break;
	case 0x63: // MOV H,E
		_H = _E;
		break;
	case 0x64: // MOV H,H
		break;
	case 0x65: // MOV H,L
		_H = _L;
		break;
	case 0x66: // MOV H,M
		_H = RM8(HL);
		break;
	case 0x67: // MOV H,A
		_H = _A;
		break;
	case 0x68: // MOV L,B
		_L = _B;
		break;
	case 0x69: // MOV L,C
		_L = _C;
		break;
	case 0x6a: // MOV L,D
		_L = _D;
		break;
	case 0x6b: // MOV L,E
		_L = _E;
		break;
	case 0x6c: // MOV L,H
		_L = _H;
		break;
	case 0x6d: // MOV L,L
		break;
	case 0x6e: // MOV L,M
		_L = RM8(HL);
		break;
	case 0x6f: // MOV L,A
		_L = _A;
		break;
	case 0x70: // MOV M,B
		WM8(HL, _B);
		break;
	case 0x71: // MOV M,C
		WM8(HL, _C);
		break;
	case 0x72: // MOV M,D
		WM8(HL, _D);
		break;
	case 0x73: // MOV M,E
		WM8(HL, _E);
		break;
	case 0x74: // MOV M,H
		WM8(HL, _H);
		break;
	case 0x75: // MOV M,L
		WM8(HL, _L);
		break;
	case 0x76: // HLT
		PC--;
		afterHALT = 1;
		break;
	case 0x77: // MOV M,A
		WM8(HL, _A);
		break;
	case 0x78: // MOV A,B
		_A = _B;
		break;
	case 0x79: // MOV A,C
		_A = _C;
		break;
	case 0x7a: // MOV A,D
		_A = _D;
		break;
	case 0x7b: // MOV A,E
		_A = _E;
		break;
	case 0x7c: // MOV A,H
		_A = _H;
		break;
	case 0x7d: // MOV A,L
		_A = _L;
		break;
	case 0x7e: // MOV A,M
		_A = RM8(HL);
		break;
	case 0x7f: // MOV A,A
		break;
	case 0x80: // ADD B
		ADD(_B);
		break;
	case 0x81: // ADD C
		ADD(_C);
		break;
	case 0x82: // ADD D
		ADD(_D);
		break;
	case 0x83: // ADD E
		ADD(_E);
		break;
	case 0x84: // ADD H
		ADD(_H);
		break;
	case 0x85: // ADD L
		ADD(_L);
		break;
	case 0x86: // ADD M
		ADD(RM8(HL));
		break;
	case 0x87: // ADD A
		ADD(_A);
		break;
	case 0x88: // ADC B
		ADC(_B);
		break;
	case 0x89: // ADC C
		ADC(_C);
		break;
	case 0x8a: // ADC D
		ADC(_D);
		break;
	case 0x8b: // ADC E
		ADC(_E);
		break;
	case 0x8c: // ADC H
		ADC(_H);
		break;
	case 0x8d: // ADC L
		ADC(_L);
		break;
	case 0x8e: // ADC M
		ADC(RM8(HL));
		break;
	case 0x8f: // ADC A
		ADC(_A);
		break;
	case 0x90: // SUB B
		SUB(_B);
		break;
	case 0x91: // SUB C
		SUB(_C);
		break;
	case 0x92: // SUB D
		SUB(_D);
		break;
	case 0x93: // SUB E
		SUB(_E);
		break;
	case 0x94: // SUB H
		SUB(_H);
		break;
	case 0x95: // SUB L
		SUB(_L);
		break;
	case 0x96: // SUB M
		SUB(RM8(HL));
		break;
	case 0x97: // SUB A
		SUB(_A);
		break;
	case 0x98: // SBB B
		SBB(_B);
		break;
	case 0x99: // SBB C
		SBB(_C);
		break;
	case 0x9a: // SBB D
		SBB(_D);
		break;
	case 0x9b: // SBB E
		SBB(_E);
		break;
	case 0x9c: // SBB H
		SBB(_H);
		break;
	case 0x9d: // SBB L
		SBB(_L);
		break;
	case 0x9e: // SBB M
		SBB(RM8(HL));
		break;
	case 0x9f: // SBB A
		SBB(_A);
		break;
	case 0xa0: // ANA B
		ANA(_B);
		break;
	case 0xa1: // ANA C
		ANA(_C);
		break;
	case 0xa2: // ANA D
		ANA(_D);
		break;
	case 0xa3: // ANA E
		ANA(_E);
		break;
	case 0xa4: // ANA H
		ANA(_H);
		break;
	case 0xa5: // ANA L
		ANA(_L);
		break;
	case 0xa6: // ANA M
		ANA(RM8(HL));
		break;
	case 0xa7: // ANA A
		ANA(_A);
		break;
	case 0xa8: // XRA B
		XRA(_B);
		break;
	case 0xa9: // XRA C
		XRA(_C);
		break;
	case 0xaa: // XRA D
		XRA(_D);
		break;
	case 0xab: // XRA E
		XRA(_E);
		break;
	case 0xac: // XRA H
		XRA(_H);
		break;
	case 0xad: // XRA L
		XRA(_L);
		break;
	case 0xae: // XRA M
		XRA(RM8(HL));
		break;
	case 0xaf: // XRA A
		XRA(_A);
		break;
	case 0xb0: // ORA B
		ORA(_B);
		break;
	case 0xb1: // ORA C
		ORA(_C);
		break;
	case 0xb2: // ORA D
		ORA(_D);
		break;
	case 0xb3: // ORA E
		ORA(_E);
		break;
	case 0xb4: // ORA H
		ORA(_H);
		break;
	case 0xb5: // ORA L
		ORA(_L);
		break;
	case 0xb6: // ORA M
		ORA(RM8(HL));
		break;
	case 0xb7: // ORA A
		ORA(_A);
		break;
	case 0xb8: // CMP B
		CMP(_B);
		break;
	case 0xb9: // CMP C
		CMP(_C);
		break;
	case 0xba: // CMP D
		CMP(_D);
		break;
	case 0xbb: // CMP E
		CMP(_E);
		break;
	case 0xbc: // CMP H
		CMP(_H);
		break;
	case 0xbd: // CMP L
		CMP(_L);
		break;
	case 0xbe: // CMP M
		CMP(RM8(HL));
		break;
	case 0xbf: // CMP A
		CMP(_A);
		break;
	case 0xc0: // RNZ
		RET(!(_F & ZF));
		break;
	case 0xc1: // POP B
		BC = POP16();
		break;
	case 0xc2: // JNZ nnnn
		JMP(!(_F & ZF));
		break;
	case 0xc3: // JMP nnnn
		JMP(1);
		break;
	case 0xc4: // CNZ nnnn
		CALL(!(_F & ZF));
		break;
	case 0xc5: // PUSH B
		PUSH16(BC);
		break;
	case 0xc6: // ADI nn
		tmp8 = FETCH8();
		ADD(tmp8);
		break;
	case 0xc7: // RST 0
		RST(0);
		break;
	case 0xc8: // RZ
		RET(_F & ZF);
		break;
	case 0xc9: // RET
		RET(1);
		break;
	case 0xca: // JZ nnnn
		JMP(_F & ZF);
		break;
	case 0xcb: // RST 8 (JMP nnnn)
#ifdef HAS_I8085
		if(_F & VF) {
			count -= 12;
			RST(8);
		} else {
			count -= 6;
		}
#else
		JMP(1);
#endif
		break;
	case 0xcc: // CZ nnnn
		CALL(_F & ZF);
		break;
	case 0xcd: // CALL nnnn
		CALL(1);
		break;
	case 0xce: // ACI nn
		tmp8 = FETCH8();
		ADC(tmp8);
		break;
	case 0xcf: // RST 1
		RST(1);
		break;
	case 0xd0: // RNC
		RET(!(_F & CF));
		break;
	case 0xd1: // POP D
		DE = POP16();
		break;
	case 0xd2: // JNC nnnn
		JMP(!(_F & CF));
		break;
	case 0xd3: // OUT nn
		OUT8(FETCH8(), _A);
		break;
	case 0xd4: // CNC nnnn
		CALL(!(_F & CF));
		break;
	case 0xd5: // PUSH D
		PUSH16(DE);
		break;
	case 0xd6: // SUI nn
		tmp8 = FETCH8();
		SUB(tmp8);
		break;
	case 0xd7: // RST 2
		RST(2);
		break;
	case 0xd8: // RC
		RET(_F & CF);
		break;
	case 0xd9: // SHLX (RET)
#ifdef HAS_I8085
		WM16(DE, HL);
#else
		RET(1);
#endif
		break;
	case 0xda: // JC nnnn
		JMP(_F & CF);
		break;
	case 0xdb: // IN nn
		_A = IN8(FETCH8());
		break;
	case 0xdc: // CC nnnn
		CALL(_F & CF);
		break;
	case 0xdd: // JNX nnnn (CALL nnnn)
#ifdef HAS_I8085
		JMP(!(_F & XF));
#else
		CALL(1);
#endif
		break;
	case 0xde: // SBI nn
		tmp8 = FETCH8();
		SBB(tmp8);
		break;
	case 0xdf: // RST 3
		RST(3);
		break;
	case 0xe0: // RPO
		RET(!(_F & VF));
		break;
	case 0xe1: // POP H
		HL = POP16();
		break;
	case 0xe2: // JPO nnnn
		JMP(!(_F & VF));
		break;
	case 0xe3: // XTHL
		tmp16 = POP16();
		PUSH16(HL);
		HL = tmp16;
		break;
	case 0xe4: // CPO nnnn
		CALL(!(_F & VF));
		break;
	case 0xe5: // PUSH H
		PUSH16(HL);
		break;
	case 0xe6: // ANI nn
		tmp8 = FETCH8();
		ANA(tmp8);
		break;
	case 0xe7: // RST 4
		RST(4);
		break;
	case 0xe8: // RPE
		RET(_F & VF);
		break;
	case 0xe9: // PCHL
		PC = HL;
		break;
	case 0xea: // JPE nnnn
		JMP(_F & VF);
		break;
	case 0xeb: // XCHG
		tmp16 = DE;
		DE = HL;
		HL = tmp16;
		break;
	case 0xec: // CPE nnnn
		CALL(_F & VF);
		break;
	case 0xed: // LHLX (CALL nnnn)
#ifdef HAS_I8085
		HL = RM16(DE);
#else
		CALL(1);
#endif
		break;
	case 0xee: // XRI nn
		tmp8 = FETCH8();
		XRA(tmp8);
		break;
	case 0xef: // RST 5
		RST(5);
		break;
	case 0xf0: // RP
		RET(!(_F & SF));
		break;
	case 0xf1: // POP A
		AF = POP16();
		break;
	case 0xf2: // JP nnnn
		JMP(!(_F & SF));
		break;
	case 0xf3: // DI
		IM &= ~IM_IEN;
		break;
	case 0xf4: // CP nnnn
		CALL(!(_F & SF));
		break;
	case 0xf5: // PUSH A
		PUSH16(AF);
		break;
	case 0xf6: // ORI nn
		tmp8 = FETCH8();
		ORA(tmp8);
		break;
	case 0xf7: // RST 6
		RST(6);
		break;
	case 0xf8: // RM
		RET(_F & SF);
		break;
	case 0xf9: // SPHL
		SP = HL;
		break;
	case 0xfa: // JM nnnn
		JMP(_F & SF);
		break;
	case 0xfb: // EI
		IM |= IM_IEN;
		afterEI = true;
		break;
	case 0xfc: // CM nnnn
		CALL(_F & SF);
		break;
	case 0xfd: // JX nnnn (CALL nnnn)
#ifdef HAS_I8085
		JMP(_F & XF);
#else
		CALL(1);
#endif
		break;
	case 0xfe: // CPI nn
		tmp8 = FETCH8();
		CMP(tmp8);
		break;
	case 0xff: // RST 7
		RST(7);
		break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default:
		__assume(0);
#endif
	}
}

#ifdef USE_DEBUGGER
void I8080::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem_stored->write_data8w(addr, data, &wait);
}

uint32_t I8080::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem_stored->read_data8w(addr, &wait);
}

void I8080::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	d_io_stored->write_io8w(addr, data, &wait);
}

uint32_t I8080::read_debug_io8(uint32_t addr)
{
	int wait;
	return d_io_stored->read_io8w(addr, &wait);
}

bool I8080::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if(_tcsicmp(reg, _T("PC")) == 0) {
		PC = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		SP = data;
	} else if(_tcsicmp(reg, _T("AF")) == 0) {
		AF = data;
	} else if(_tcsicmp(reg, _T("BC")) == 0) {
		BC = data;
	} else if(_tcsicmp(reg, _T("DE")) == 0) {
		DE = data;
	} else if(_tcsicmp(reg, _T("HL")) == 0) {
		HL = data;
	} else if(_tcsicmp(reg, _T("A")) == 0) {
		_A = data;
	} else if(_tcsicmp(reg, _T("F")) == 0) {
		_F = data;
	} else if(_tcsicmp(reg, _T("B")) == 0) {
		_B = data;
	} else if(_tcsicmp(reg, _T("C")) == 0) {
		_C = data;
	} else if(_tcsicmp(reg, _T("D")) == 0) {
		_D = data;
	} else if(_tcsicmp(reg, _T("E")) == 0) {
		_E = data;
	} else if(_tcsicmp(reg, _T("H")) == 0) {
		_H = data;
	} else if(_tcsicmp(reg, _T("L")) == 0) {
		_L = data;
	} else {
		return false;
	}
	return true;
}

bool I8080::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
/*
F = [--------]  A = 00  BC = 0000  DE = 0000  HL = 0000  SP = 0000  PC = 0000
IM= [--------]         (BC)= 0000 (DE)= 0000 (HL)= 0000 (SP)= 0000
Clocks = 0 (0)  Since Scanline = 0/0 (0/0)
*/
	int wait;
	my_stprintf_s(buffer, buffer_len,
	_T("F = [%c%c%c%c%c%c%c%c]  A = %02X  BC = %04X  DE = %04X  HL = %04X  SP = %04X  PC = %04X\n")
	_T("IM= [%c%c%c%c%c%c%c%c]         (BC)= %04X (DE)= %04X (HL)= %04X (SP)= %04X\n")
	_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
	(_F & CF) ? _T('C') : _T('-'), (_F & NF) ? _T('N') : _T('-'), (_F & VF) ? _T('V') : _T('-'), (_F & XF) ? _T('X') : _T('-'),
	(_F & HF) ? _T('H') : _T('-'), (_F & YF) ? _T('Y') : _T('-'), (_F & ZF) ? _T('Z') : _T('-'), (_F & SF) ? _T('S') : _T('-'),
	_A, BC, DE, HL, SP, PC,
	(IM & 0x80) ? _T('S') : _T('-'), (IM & 0x40) ? _T('7') : _T('-'), (IM & 0x20) ? _T('6') : _T('-'), (IM & 0x10) ? _T('5') : _T('-'),
	(IM & 0x08) ? _T('E') : _T('-'), (IM & 0x04) ? _T('7') : _T('-'), (IM & 0x02) ? _T('6') : _T('-'), (IM & 0x01) ? _T('5') : _T('-'),
	d_mem_stored->read_data16w(BC, &wait), d_mem_stored->read_data16w(DE, &wait), d_mem_stored->read_data16w(HL, &wait), d_mem_stored->read_data16w(SP, &wait),
	total_count, total_count - prev_total_count,
	get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
	prev_total_count = total_count;
	return true;
}

// disassembler

int I8080::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	uint8_t ops[4];
	int ptr = 0;
	
	for(int i = 0; i < 4; i++) {
		int wait;
		ops[i] = d_mem_stored->read_data8w(pc + i, &wait);
	}
	switch(ops[ptr++])
	{
		case 0x00: my_stprintf_s(buffer, buffer_len, _T("nop")); break;
		case 0x01: my_stprintf_s(buffer, buffer_len, _T("lxi  b,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x02: my_stprintf_s(buffer, buffer_len, _T("stax b")); break;
		case 0x03: my_stprintf_s(buffer, buffer_len, _T("inx  b")); break;
		case 0x04: my_stprintf_s(buffer, buffer_len, _T("inr  b")); break;
		case 0x05: my_stprintf_s(buffer, buffer_len, _T("dcr  b")); break;
		case 0x06: my_stprintf_s(buffer, buffer_len, _T("mvi  b,$%02x"), ops[ptr++]); break;
		case 0x07: my_stprintf_s(buffer, buffer_len, _T("rlc")); break;
#ifdef HAS_I8085
		case 0x08: my_stprintf_s(buffer, buffer_len, _T("dsub")); break;
#else
		case 0x08: my_stprintf_s(buffer, buffer_len, _T("nop")); break;
#endif
		case 0x09: my_stprintf_s(buffer, buffer_len, _T("dad  b")); break;
		case 0x0a: my_stprintf_s(buffer, buffer_len, _T("ldax b")); break;
		case 0x0b: my_stprintf_s(buffer, buffer_len, _T("dcx  b")); break;
		case 0x0c: my_stprintf_s(buffer, buffer_len, _T("inr  c")); break;
		case 0x0d: my_stprintf_s(buffer, buffer_len, _T("dcr  c")); break;
		case 0x0e: my_stprintf_s(buffer, buffer_len, _T("mvi  c,$%02x"), ops[ptr++]); break;
		case 0x0f: my_stprintf_s(buffer, buffer_len, _T("rrc")); break;
#ifdef HAS_I8085
		case 0x10: my_stprintf_s(buffer, buffer_len, _T("asrh")); break;
#else
		case 0x10: my_stprintf_s(buffer, buffer_len, _T("nop")); break;
#endif
		case 0x11: my_stprintf_s(buffer, buffer_len, _T("lxi  d,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x12: my_stprintf_s(buffer, buffer_len, _T("stax d")); break;
		case 0x13: my_stprintf_s(buffer, buffer_len, _T("inx  d")); break;
		case 0x14: my_stprintf_s(buffer, buffer_len, _T("inr  d")); break;
		case 0x15: my_stprintf_s(buffer, buffer_len, _T("dcr  d")); break;
		case 0x16: my_stprintf_s(buffer, buffer_len, _T("mvi  d,$%02x"), ops[ptr++]); break;
		case 0x17: my_stprintf_s(buffer, buffer_len, _T("ral")); break;
#ifdef HAS_I8085
		case 0x18: my_stprintf_s(buffer, buffer_len, _T("rlde")); break;
#else
		case 0x18: my_stprintf_s(buffer, buffer_len, _T("nop")); break;
#endif
		case 0x19: my_stprintf_s(buffer, buffer_len, _T("dad  d")); break;
		case 0x1a: my_stprintf_s(buffer, buffer_len, _T("ldax d")); break;
		case 0x1b: my_stprintf_s(buffer, buffer_len, _T("dcx  d")); break;
		case 0x1c: my_stprintf_s(buffer, buffer_len, _T("inr  e")); break;
		case 0x1d: my_stprintf_s(buffer, buffer_len, _T("dcr  e")); break;
		case 0x1e: my_stprintf_s(buffer, buffer_len, _T("mvi  e,$%02x"), ops[ptr++]); break;
		case 0x1f: my_stprintf_s(buffer, buffer_len, _T("rar")); break;
		case 0x20: my_stprintf_s(buffer, buffer_len, _T("rim")); break;
		case 0x21: my_stprintf_s(buffer, buffer_len, _T("lxi  h,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x22: my_stprintf_s(buffer, buffer_len, _T("shld %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x23: my_stprintf_s(buffer, buffer_len, _T("inx  h")); break;
		case 0x24: my_stprintf_s(buffer, buffer_len, _T("inr  h")); break;
		case 0x25: my_stprintf_s(buffer, buffer_len, _T("dcr  h")); break;
		case 0x26: my_stprintf_s(buffer, buffer_len, _T("mvi  h,$%02x"), ops[ptr++]); break;
		case 0x27: my_stprintf_s(buffer, buffer_len, _T("daa")); break;
#ifdef HAS_I8085
		case 0x28: my_stprintf_s(buffer, buffer_len, _T("ldeh $%02x"), ops[ptr++]); break;
#else
		case 0x28: my_stprintf_s(buffer, buffer_len, _T("nop")); break;
#endif
		case 0x29: my_stprintf_s(buffer, buffer_len, _T("dad  h")); break;
		case 0x2a: my_stprintf_s(buffer, buffer_len, _T("lhld %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x2b: my_stprintf_s(buffer, buffer_len, _T("dcx  h")); break;
		case 0x2c: my_stprintf_s(buffer, buffer_len, _T("inr  l")); break;
		case 0x2d: my_stprintf_s(buffer, buffer_len, _T("dcr  l")); break;
		case 0x2e: my_stprintf_s(buffer, buffer_len, _T("mvi  l,$%02x"), ops[ptr++]); break;
		case 0x2f: my_stprintf_s(buffer, buffer_len, _T("cma")); break;
		case 0x30: my_stprintf_s(buffer, buffer_len, _T("sim")); break;
		case 0x31: my_stprintf_s(buffer, buffer_len, _T("lxi  sp,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x32: my_stprintf_s(buffer, buffer_len, _T("stax %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x33: my_stprintf_s(buffer, buffer_len, _T("inx  sp")); break;
		case 0x34: my_stprintf_s(buffer, buffer_len, _T("inr  m")); break;
		case 0x35: my_stprintf_s(buffer, buffer_len, _T("dcr  m")); break;
		case 0x36: my_stprintf_s(buffer, buffer_len, _T("mvi  m,$%02x"), ops[ptr++]); break;
		case 0x37: my_stprintf_s(buffer, buffer_len, _T("stc")); break;
		case 0x38: my_stprintf_s(buffer, buffer_len, _T("ldes $%02x"), ops[ptr++]); break;
		case 0x39: my_stprintf_s(buffer, buffer_len, _T("dad sp")); break;
		case 0x3a: my_stprintf_s(buffer, buffer_len, _T("ldax %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x3b: my_stprintf_s(buffer, buffer_len, _T("dcx  sp")); break;
		case 0x3c: my_stprintf_s(buffer, buffer_len, _T("inr  a")); break;
		case 0x3d: my_stprintf_s(buffer, buffer_len, _T("dcr  a")); break;
		case 0x3e: my_stprintf_s(buffer, buffer_len, _T("mvi  a,$%02x"), ops[ptr++]); break;
		case 0x3f: my_stprintf_s(buffer, buffer_len, _T("cmf")); break;
		case 0x40: my_stprintf_s(buffer, buffer_len, _T("mov  b,b")); break;
		case 0x41: my_stprintf_s(buffer, buffer_len, _T("mov  b,c")); break;
		case 0x42: my_stprintf_s(buffer, buffer_len, _T("mov  b,d")); break;
		case 0x43: my_stprintf_s(buffer, buffer_len, _T("mov  b,e")); break;
		case 0x44: my_stprintf_s(buffer, buffer_len, _T("mov  b,h")); break;
		case 0x45: my_stprintf_s(buffer, buffer_len, _T("mov  b,l")); break;
		case 0x46: my_stprintf_s(buffer, buffer_len, _T("mov  b,m")); break;
		case 0x47: my_stprintf_s(buffer, buffer_len, _T("mov  b,a")); break;
		case 0x48: my_stprintf_s(buffer, buffer_len, _T("mov  c,b")); break;
		case 0x49: my_stprintf_s(buffer, buffer_len, _T("mov  c,c")); break;
		case 0x4a: my_stprintf_s(buffer, buffer_len, _T("mov  c,d")); break;
		case 0x4b: my_stprintf_s(buffer, buffer_len, _T("mov  c,e")); break;
		case 0x4c: my_stprintf_s(buffer, buffer_len, _T("mov  c,h")); break;
		case 0x4d: my_stprintf_s(buffer, buffer_len, _T("mov  c,l")); break;
		case 0x4e: my_stprintf_s(buffer, buffer_len, _T("mov  c,m")); break;
		case 0x4f: my_stprintf_s(buffer, buffer_len, _T("mov  c,a")); break;
		case 0x50: my_stprintf_s(buffer, buffer_len, _T("mov  d,b")); break;
		case 0x51: my_stprintf_s(buffer, buffer_len, _T("mov  d,c")); break;
		case 0x52: my_stprintf_s(buffer, buffer_len, _T("mov  d,d")); break;
		case 0x53: my_stprintf_s(buffer, buffer_len, _T("mov  d,e")); break;
		case 0x54: my_stprintf_s(buffer, buffer_len, _T("mov  d,h")); break;
		case 0x55: my_stprintf_s(buffer, buffer_len, _T("mov  d,l")); break;
		case 0x56: my_stprintf_s(buffer, buffer_len, _T("mov  d,m")); break;
		case 0x57: my_stprintf_s(buffer, buffer_len, _T("mov  d,a")); break;
		case 0x58: my_stprintf_s(buffer, buffer_len, _T("mov  e,b")); break;
		case 0x59: my_stprintf_s(buffer, buffer_len, _T("mov  e,c")); break;
		case 0x5a: my_stprintf_s(buffer, buffer_len, _T("mov  e,d")); break;
		case 0x5b: my_stprintf_s(buffer, buffer_len, _T("mov  e,e")); break;
		case 0x5c: my_stprintf_s(buffer, buffer_len, _T("mov  e,h")); break;
		case 0x5d: my_stprintf_s(buffer, buffer_len, _T("mov  e,l")); break;
		case 0x5e: my_stprintf_s(buffer, buffer_len, _T("mov  e,m")); break;
		case 0x5f: my_stprintf_s(buffer, buffer_len, _T("mov  e,a")); break;
		case 0x60: my_stprintf_s(buffer, buffer_len, _T("mov  h,b")); break;
		case 0x61: my_stprintf_s(buffer, buffer_len, _T("mov  h,c")); break;
		case 0x62: my_stprintf_s(buffer, buffer_len, _T("mov  h,d")); break;
		case 0x63: my_stprintf_s(buffer, buffer_len, _T("mov  h,e")); break;
		case 0x64: my_stprintf_s(buffer, buffer_len, _T("mov  h,h")); break;
		case 0x65: my_stprintf_s(buffer, buffer_len, _T("mov  h,l")); break;
		case 0x66: my_stprintf_s(buffer, buffer_len, _T("mov  h,m")); break;
		case 0x67: my_stprintf_s(buffer, buffer_len, _T("mov  h,a")); break;
		case 0x68: my_stprintf_s(buffer, buffer_len, _T("mov  l,b")); break;
		case 0x69: my_stprintf_s(buffer, buffer_len, _T("mov  l,c")); break;
		case 0x6a: my_stprintf_s(buffer, buffer_len, _T("mov  l,d")); break;
		case 0x6b: my_stprintf_s(buffer, buffer_len, _T("mov  l,e")); break;
		case 0x6c: my_stprintf_s(buffer, buffer_len, _T("mov  l,h")); break;
		case 0x6d: my_stprintf_s(buffer, buffer_len, _T("mov  l,l")); break;
		case 0x6e: my_stprintf_s(buffer, buffer_len, _T("mov  l,m")); break;
		case 0x6f: my_stprintf_s(buffer, buffer_len, _T("mov  l,a")); break;
		case 0x70: my_stprintf_s(buffer, buffer_len, _T("mov  m,b")); break;
		case 0x71: my_stprintf_s(buffer, buffer_len, _T("mov  m,c")); break;
		case 0x72: my_stprintf_s(buffer, buffer_len, _T("mov  m,d")); break;
		case 0x73: my_stprintf_s(buffer, buffer_len, _T("mov  m,e")); break;
		case 0x74: my_stprintf_s(buffer, buffer_len, _T("mov  m,h")); break;
		case 0x75: my_stprintf_s(buffer, buffer_len, _T("mov  m,l")); break;
		case 0x76: my_stprintf_s(buffer, buffer_len, _T("hlt")); break;
		case 0x77: my_stprintf_s(buffer, buffer_len, _T("mov  m,a")); break;
		case 0x78: my_stprintf_s(buffer, buffer_len, _T("mov  a,b")); break;
		case 0x79: my_stprintf_s(buffer, buffer_len, _T("mov  a,c")); break;
		case 0x7a: my_stprintf_s(buffer, buffer_len, _T("mov  a,d")); break;
		case 0x7b: my_stprintf_s(buffer, buffer_len, _T("mov  a,e")); break;
		case 0x7c: my_stprintf_s(buffer, buffer_len, _T("mov  a,h")); break;
		case 0x7d: my_stprintf_s(buffer, buffer_len, _T("mov  a,l")); break;
		case 0x7e: my_stprintf_s(buffer, buffer_len, _T("mov  a,m")); break;
		case 0x7f: my_stprintf_s(buffer, buffer_len, _T("mov  a,a")); break;
		case 0x80: my_stprintf_s(buffer, buffer_len, _T("add  b")); break;
		case 0x81: my_stprintf_s(buffer, buffer_len, _T("add  c")); break;
		case 0x82: my_stprintf_s(buffer, buffer_len, _T("add  d")); break;
		case 0x83: my_stprintf_s(buffer, buffer_len, _T("add  e")); break;
		case 0x84: my_stprintf_s(buffer, buffer_len, _T("add  h")); break;
		case 0x85: my_stprintf_s(buffer, buffer_len, _T("add  l")); break;
		case 0x86: my_stprintf_s(buffer, buffer_len, _T("add  m")); break;
		case 0x87: my_stprintf_s(buffer, buffer_len, _T("add  a")); break;
		case 0x88: my_stprintf_s(buffer, buffer_len, _T("adc  b")); break;
		case 0x89: my_stprintf_s(buffer, buffer_len, _T("adc  c")); break;
		case 0x8a: my_stprintf_s(buffer, buffer_len, _T("adc  d")); break;
		case 0x8b: my_stprintf_s(buffer, buffer_len, _T("adc  e")); break;
		case 0x8c: my_stprintf_s(buffer, buffer_len, _T("adc  h")); break;
		case 0x8d: my_stprintf_s(buffer, buffer_len, _T("adc  l")); break;
		case 0x8e: my_stprintf_s(buffer, buffer_len, _T("adc  m")); break;
		case 0x8f: my_stprintf_s(buffer, buffer_len, _T("adc  a")); break;
		case 0x90: my_stprintf_s(buffer, buffer_len, _T("sub  b")); break;
		case 0x91: my_stprintf_s(buffer, buffer_len, _T("sub  c")); break;
		case 0x92: my_stprintf_s(buffer, buffer_len, _T("sub  d")); break;
		case 0x93: my_stprintf_s(buffer, buffer_len, _T("sub  e")); break;
		case 0x94: my_stprintf_s(buffer, buffer_len, _T("sub  h")); break;
		case 0x95: my_stprintf_s(buffer, buffer_len, _T("sub  l")); break;
		case 0x96: my_stprintf_s(buffer, buffer_len, _T("sub  m")); break;
		case 0x97: my_stprintf_s(buffer, buffer_len, _T("sub  a")); break;
		case 0x98: my_stprintf_s(buffer, buffer_len, _T("sbb  b")); break;
		case 0x99: my_stprintf_s(buffer, buffer_len, _T("sbb  c")); break;
		case 0x9a: my_stprintf_s(buffer, buffer_len, _T("sbb  d")); break;
		case 0x9b: my_stprintf_s(buffer, buffer_len, _T("sbb  e")); break;
		case 0x9c: my_stprintf_s(buffer, buffer_len, _T("sbb  h")); break;
		case 0x9d: my_stprintf_s(buffer, buffer_len, _T("sbb  l")); break;
		case 0x9e: my_stprintf_s(buffer, buffer_len, _T("sbb  m")); break;
		case 0x9f: my_stprintf_s(buffer, buffer_len, _T("sbb  a")); break;
		case 0xa0: my_stprintf_s(buffer, buffer_len, _T("ana  b")); break;
		case 0xa1: my_stprintf_s(buffer, buffer_len, _T("ana  c")); break;
		case 0xa2: my_stprintf_s(buffer, buffer_len, _T("ana  d")); break;
		case 0xa3: my_stprintf_s(buffer, buffer_len, _T("ana  e")); break;
		case 0xa4: my_stprintf_s(buffer, buffer_len, _T("ana  h")); break;
		case 0xa5: my_stprintf_s(buffer, buffer_len, _T("ana  l")); break;
		case 0xa6: my_stprintf_s(buffer, buffer_len, _T("ana  m")); break;
		case 0xa7: my_stprintf_s(buffer, buffer_len, _T("ana  a")); break;
		case 0xa8: my_stprintf_s(buffer, buffer_len, _T("xra  b")); break;
		case 0xa9: my_stprintf_s(buffer, buffer_len, _T("xra  c")); break;
		case 0xaa: my_stprintf_s(buffer, buffer_len, _T("xra  d")); break;
		case 0xab: my_stprintf_s(buffer, buffer_len, _T("xra  e")); break;
		case 0xac: my_stprintf_s(buffer, buffer_len, _T("xra  h")); break;
		case 0xad: my_stprintf_s(buffer, buffer_len, _T("xra  l")); break;
		case 0xae: my_stprintf_s(buffer, buffer_len, _T("xra  m")); break;
		case 0xaf: my_stprintf_s(buffer, buffer_len, _T("xra  a")); break;
		case 0xb0: my_stprintf_s(buffer, buffer_len, _T("ora  b")); break;
		case 0xb1: my_stprintf_s(buffer, buffer_len, _T("ora  c")); break;
		case 0xb2: my_stprintf_s(buffer, buffer_len, _T("ora  d")); break;
		case 0xb3: my_stprintf_s(buffer, buffer_len, _T("ora  e")); break;
		case 0xb4: my_stprintf_s(buffer, buffer_len, _T("ora  h")); break;
		case 0xb5: my_stprintf_s(buffer, buffer_len, _T("ora  l")); break;
		case 0xb6: my_stprintf_s(buffer, buffer_len, _T("ora  m")); break;
		case 0xb7: my_stprintf_s(buffer, buffer_len, _T("ora  a")); break;
		case 0xb8: my_stprintf_s(buffer, buffer_len, _T("cmp  b")); break;
		case 0xb9: my_stprintf_s(buffer, buffer_len, _T("cmp  c")); break;
		case 0xba: my_stprintf_s(buffer, buffer_len, _T("cmp  d")); break;
		case 0xbb: my_stprintf_s(buffer, buffer_len, _T("cmp  e")); break;
		case 0xbc: my_stprintf_s(buffer, buffer_len, _T("cmp  h")); break;
		case 0xbd: my_stprintf_s(buffer, buffer_len, _T("cmp  l")); break;
		case 0xbe: my_stprintf_s(buffer, buffer_len, _T("cmp  m")); break;
		case 0xbf: my_stprintf_s(buffer, buffer_len, _T("cmp  a")); break;
		case 0xc0: my_stprintf_s(buffer, buffer_len, _T("rnz")); break;
		case 0xc1: my_stprintf_s(buffer, buffer_len, _T("pop  b")); break;
		case 0xc2: my_stprintf_s(buffer, buffer_len, _T("jnz  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xc3: my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xc4: my_stprintf_s(buffer, buffer_len, _T("cnz  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xc5: my_stprintf_s(buffer, buffer_len, _T("push b")); break;
		case 0xc6: my_stprintf_s(buffer, buffer_len, _T("adi  $%02x"), ops[ptr++]); break;
		case 0xc7: my_stprintf_s(buffer, buffer_len, _T("rst  0")); break;
		case 0xc8: my_stprintf_s(buffer, buffer_len, _T("rz")); break;
		case 0xc9: my_stprintf_s(buffer, buffer_len, _T("ret")); break;
		case 0xca: my_stprintf_s(buffer, buffer_len, _T("jz   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
#ifdef HAS_I8085
		case 0xcb: my_stprintf_s(buffer, buffer_len, _T("rstv 8")); break;
#else
		case 0xcb: my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
#endif
		case 0xcc: my_stprintf_s(buffer, buffer_len, _T("cz   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xcd: my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xce: my_stprintf_s(buffer, buffer_len, _T("aci  $%02x"), ops[ptr++]); break;
		case 0xcf: my_stprintf_s(buffer, buffer_len, _T("rst  1")); break;
		case 0xd0: my_stprintf_s(buffer, buffer_len, _T("rnc")); break;
		case 0xd1: my_stprintf_s(buffer, buffer_len, _T("pop  d")); break;
		case 0xd2: my_stprintf_s(buffer, buffer_len, _T("jnc  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xd3: my_stprintf_s(buffer, buffer_len, _T("out  $%02x"), ops[ptr++]); break;
		case 0xd4: my_stprintf_s(buffer, buffer_len, _T("cnc  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xd5: my_stprintf_s(buffer, buffer_len, _T("push d")); break;
		case 0xd6: my_stprintf_s(buffer, buffer_len, _T("sui  $%02x"), ops[ptr++]); break;
		case 0xd7: my_stprintf_s(buffer, buffer_len, _T("rst  2")); break;
		case 0xd8: my_stprintf_s(buffer, buffer_len, _T("rc")); break;
#ifdef HAS_I8085
		case 0xd9: my_stprintf_s(buffer, buffer_len, _T("shlx d")); break;
#else
		case 0xd9: my_stprintf_s(buffer, buffer_len, _T("ret")); break;
#endif
		case 0xda: my_stprintf_s(buffer, buffer_len, _T("jc   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xdb: my_stprintf_s(buffer, buffer_len, _T("in   $%02x"), ops[ptr++]); break;
		case 0xdc: my_stprintf_s(buffer, buffer_len, _T("cc   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
#ifdef HAS_I8085
		case 0xdd: my_stprintf_s(buffer, buffer_len, _T("jnx  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
#else
		case 0xdd: my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
#endif
		case 0xde: my_stprintf_s(buffer, buffer_len, _T("sbi  $%02x"), ops[ptr++]); break;
		case 0xdf: my_stprintf_s(buffer, buffer_len, _T("rst  3")); break;
		case 0xe0: my_stprintf_s(buffer, buffer_len, _T("rpo")); break;
		case 0xe1: my_stprintf_s(buffer, buffer_len, _T("pop  h")); break;
		case 0xe2: my_stprintf_s(buffer, buffer_len, _T("jpo  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xe3: my_stprintf_s(buffer, buffer_len, _T("xthl")); break;
		case 0xe4: my_stprintf_s(buffer, buffer_len, _T("cpo  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xe5: my_stprintf_s(buffer, buffer_len, _T("push h")); break;
		case 0xe6: my_stprintf_s(buffer, buffer_len, _T("ani  $%02x"), ops[ptr++]); break;
		case 0xe7: my_stprintf_s(buffer, buffer_len, _T("rst  4")); break;
		case 0xe8: my_stprintf_s(buffer, buffer_len, _T("rpe")); break;
		case 0xe9: my_stprintf_s(buffer, buffer_len, _T("PChl")); break;
		case 0xea: my_stprintf_s(buffer, buffer_len, _T("jpe  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xeb: my_stprintf_s(buffer, buffer_len, _T("xchg")); break;
		case 0xec: my_stprintf_s(buffer, buffer_len, _T("cpe  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
#ifdef HAS_I8085
		case 0xed: my_stprintf_s(buffer, buffer_len, _T("lhlx d")); break;
#else
		case 0xed: my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
#endif
		case 0xee: my_stprintf_s(buffer, buffer_len, _T("xri  $%02x"), ops[ptr++]); break;
		case 0xef: my_stprintf_s(buffer, buffer_len, _T("rst  5")); break;
		case 0xf0: my_stprintf_s(buffer, buffer_len, _T("rp")); break;
		case 0xf1: my_stprintf_s(buffer, buffer_len, _T("pop  a")); break;
		case 0xf2: my_stprintf_s(buffer, buffer_len, _T("jp   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xf3: my_stprintf_s(buffer, buffer_len, _T("di")); break;
		case 0xf4: my_stprintf_s(buffer, buffer_len, _T("cp   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xf5: my_stprintf_s(buffer, buffer_len, _T("push a")); break;
		case 0xf6: my_stprintf_s(buffer, buffer_len, _T("ori  $%02x"), ops[ptr++]); break;
		case 0xf7: my_stprintf_s(buffer, buffer_len, _T("rst  6")); break;
		case 0xf8: my_stprintf_s(buffer, buffer_len, _T("rm")); break;
		case 0xf9: my_stprintf_s(buffer, buffer_len, _T("sphl")); break;
		case 0xfa: my_stprintf_s(buffer, buffer_len, _T("jm   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xfb: my_stprintf_s(buffer, buffer_len, _T("ei")); break;
		case 0xfc: my_stprintf_s(buffer, buffer_len, _T("cm   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
#ifdef HAS_I8085
		case 0xfd: my_stprintf_s(buffer, buffer_len, _T("jx   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
#else
		case 0xfd: my_stprintf_s(buffer, buffer_len, _T("cm   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
#endif
		case 0xfe: my_stprintf_s(buffer, buffer_len, _T("cpi  $%02x"), ops[ptr++]); break;
		case 0xff: my_stprintf_s(buffer, buffer_len, _T("rst  7")); break;
	}
	return ptr;
}
#endif

#define STATE_VERSION	2

bool I8080::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#ifdef USE_DEBUGGER
	state_fio->StateValue(total_count);
#endif
	state_fio->StateValue(count);
	state_fio->StateArray(regs, sizeof(regs), 1);
	state_fio->StateValue(SP);
	state_fio->StateValue(PC);
	state_fio->StateValue(prevPC);
	state_fio->StateValue(IM);
	state_fio->StateValue(RIM_IEN);
	state_fio->StateValue(afterHALT);
	state_fio->StateValue(BUSREQ);
	state_fio->StateValue(SID);
	state_fio->StateValue(afterEI);
	
#ifdef USE_DEBUGGER
	// post process
	if(loading) {
		prev_total_count = total_count;
	}
#endif
	return true;
}

