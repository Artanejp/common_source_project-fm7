/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2025.03.27 -

	[ FM-Towns CRTC / Event number defines. ]
	History: 2025.03.27 Sprit from crtc.cpp .
*/
#pragma once


namespace FMTOWNS {
	// Event numbers.
	enum {
		EVENT_HSYNC_OFF = 1,
		EVENT_HDS0 = 2,
		EVENT_HDS1 = 3,
		EVENT_HDE0 = 4,
		EVENT_HDE1 = 5,
	};
}
