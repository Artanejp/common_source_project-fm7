/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.06.24-
	History: Split constants from display.cpp.
	
	[ display ]
*/



#pragma once

#define SCROLL_PL	0
#define SCROLL_BL	1
#define SCROLL_CL	2
#define SCROLL_SSL	3
#define SCROLL_SUR	4
#define SCROLL_SDR	5

#define MODE1_ATRSEL	0
#define MODE1_GRAPHIC	1
#define MODE1_COLUMN	2
#define MODE1_FONTSEL	3
#define MODE1_200LINE	4
#define MODE1_KAC	5
#define MODE1_MEMSW	6
#define MODE1_DISP	7

#define MODE2_16COLOR	0x00
#define MODE2_EGC		0x02
#define MODE2_EGC_WP	0x03
#define MDOE2_TXTSHIFT	0x20

#define GRCG_PLANE_0	0x01
#define GRCG_PLANE_1	0x02
#define GRCG_PLANE_2	0x04
#define GRCG_PLANE_3	0x08
#define GRCG_RW_MODE	0x40
#define GRCG_CG_MODE	0x80

#define ATTR_ST		0x01
#define ATTR_BL		0x02
#define ATTR_RV		0x04
#define ATTR_UL		0x08
#define ATTR_VL		0x10
#define ATTR_COL	0xe0

namespace PC9801 {

#if !defined(SUPPORT_HIRESO)
	#define TVRAM_ADDRESS		0xa0000
	#define VRAM_PLANE_SIZE		0x08000
	#define VRAM_PLANE_ADDR_MASK	0x07fff
	#define VRAM_PLANE_ADDR_0	0x08000
	#define VRAM_PLANE_ADDR_1	0x10000
	#define VRAM_PLANE_ADDR_2	0x18000
	#define VRAM_PLANE_ADDR_3	0x00000
#else
	#define TVRAM_ADDRESS		0xe0000
	#define VRAM_PLANE_SIZE		0x20000
	#define VRAM_PLANE_ADDR_MASK	0x1ffff
	#define VRAM_PLANE_ADDR_0	0x00000
	#define VRAM_PLANE_ADDR_1	0x20000
	#define VRAM_PLANE_ADDR_2	0x40000
	#define VRAM_PLANE_ADDR_3	0x60000
#endif
}
