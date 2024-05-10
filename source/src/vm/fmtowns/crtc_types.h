/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2024.05.10 -

	[ FM-Towns CRTC / Constants and data types.]
	History: 2024.05.10 Sprit from crtc.h .
*/
#pragma once

namespace FMTOWNS {
	// Constants.
	enum {
		CRTC_BUFFER_NUM = 4,
	};
	enum {
		NOT_LOOP = 0,
		IS_LOOP = 255
	};
	enum {
		TOWNS_CRTC_PALETTE_INDEX = 0xff,
		TOWNS_CRTC_PALETTE_R = 0,
		TOWNS_CRTC_PALETTE_G,
		TOWNS_CRTC_PALETTE_B,
		TOWNS_CRTC_PALETTE_I,
	};
	enum {
		TOWNS_CRTC_REG_HSW1 = 0,
		TOWNS_CRTC_REG_HSW2 = 1,
		TOWNS_CRTC_REG_HST  = 4,
		TOWNS_CRTC_REG_VST1 = 5,
		TOWNS_CRTC_REG_VST2 = 6,
		TOWNS_CRTC_REG_EET  = 7,

		TOWNS_CRTC_REG_VST  = 8,
		TOWNS_CRTC_REG_HDS0 = 9,
		TOWNS_CRTC_REG_HDE0 = 10,
		TOWNS_CRTC_REG_HDS1 = 11,
		TOWNS_CRTC_REG_HDE1 = 12,
		TOWNS_CRTC_REG_VDS0 = 13,
		TOWNS_CRTC_REG_VDE0 = 14,
		TOWNS_CRTC_REG_VDS1 = 15,

		TOWNS_CRTC_REG_VDE1 = 16,
		TOWNS_CRTC_REG_FA0  = 17,
		TOWNS_CRTC_REG_HAJ0 = 18,
		TOWNS_CRTC_REG_FO0  = 19,
		TOWNS_CRTC_REG_LO0  = 20,
		TOWNS_CRTC_REG_FA1  = 21,
		TOWNS_CRTC_REG_HAJ1 = 22,
		TOWNS_CRTC_REG_FO1  = 23,

		TOWNS_CRTC_REG_LO1  = 24,
		TOWNS_CRTC_REG_EHAJ = 25,
		TOWNS_CRTC_REG_EVAJ = 26,
		TOWNS_CRTC_REG_ZOOM = 27,
		TOWNS_CRTC_REG_DISPMODE = 28,
		TOWNS_CRTC_REG_CLK      = 29,
		TOWNS_CRTC_REG_DUMMY    = 30,
		TOWNS_CRTC_REG_CTRL     = 31,
	};

	enum {
		DISPMODE_NONE = 0,
		DISPMODE_256,
		DISPMODE_32768,
		DISPMODE_16,
		DISPMODE_DUP = 0x80,
	};
	enum {
		VOUTREG_CTRL = 0,
		VOUTREG_PRIO = 1,
		VOUTREG_2,
		VOUTREG_3,
	};

	typedef struct {
		uint8_t raw[256][4];
		scrntype_t pixels[256];
	} palette_backup_t;
// May align to be faster.

	// Data types.
	typedef struct {
#pragma pack(push, 1)
		// 32 * 4
		uint8_t mode[4];
		uint8_t is_hloop[4];
		int8_t mag[4];
		uint8_t r50_planemask[2]; // MMIO 000CF882h : BIT 5(C0) and BIT2 to 0
		uint8_t crtout[2];
#pragma pack(pop)
#pragma pack(push, 4)
		// 32 * 12
		int32_t pixels[4];
		int32_t num[4];
		int32_t bitoffset[2];
		int32_t prev_y[2];
#pragma pack(pop)
		// Align of 16 * 32 bits = 512 bits.
//#pragma pack(push, 16)
		uint8_t pixels_layer[2][TOWNS_CRTC_MAX_PIXELS * sizeof(uint16_t)]; // RAW VALUE
		palette_backup_t palettes[2];
//#pragma pack(pop)
		// pixels_lauyer[][] : 1024 * 2 * 8 = 1024 * 16 [bytes]
	} linebuffer_t;

}

