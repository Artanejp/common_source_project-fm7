/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2025.05.10 -

	[FM-Towns CD-ROM ; Event processing function parts.]
	
	History:
	  2025-05-10 K.Ohta : Split from vm/fmtowns/cdrom.h .
*/

#pragma once

#include "../../types/optimizer_utils.h"
/* #include "../../common.h" */
/* #include "../cdrom.h" */

//namespace FMTOWNS {
	
_CONSTEXPR_FUNC void TOWNS_CDROM::event_callback_delay_ready(const bool forceint)
{
	event_delay_ready = -1;
	stop_time_out();
	media_changed = false;
	media_ejected = false;
	bool is_intr = ((forceint) || ((req_status) && (stat_reply_intr)));
	end_of_command(true, is_intr, false);
}

inline void TOWNS_CDROM::event_callback_not_ready(const bool forceint)
{
	event_delay_ready = -1;
	stop_time_out();
	media_changed = false;
	media_ejected = false;

	bool is_intr = ((forceint) || ((req_status) && (stat_reply_intr)));
	end_of_command(true, is_intr, true);
}
	
_CONSTEXPR_FUNC void TOWNS_CDROM::event_callback_delay_interrupt(const bool _set_mcu_ready, const bool interrupt_on)
{
	event_delay_interrupt = -1;
//	has_status = !(status_queue.isEmpty());
	if(_set_mcu_ready) {
		mcu_ready = true;
	}
	if(interrupt_on) {
		set_mcu_intr(true);
	} else {
		mcu_intr = false;
		write_mcuint_signals(false);
	}
}

_CONSTEXPR_FUNC void TOWNS_CDROM::event_callback_eot(const bool is_dma, const bool forceint)
{
	event_delay_ready = -1;
	dma_transfer = false;
	pio_transfer = false;
	status_seek = false;
	media_changed = false; // OK?
	media_ejected = false; // OK?
	status_read_done(forceint);
}
//}
