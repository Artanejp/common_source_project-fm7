/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.12.01 -

	[ common definitions ]
*/
#pragma once

// These are common definitions for FM-Towns.
#define SIG_FMTOWNS_RAM_WAIT      0x10000000 /**< SET MAIN RAM WAIT VALUE ARG=RAW WAIT VALUE */
#define SIG_FMTOWNS_ROM_WAIT      0x10000001 /**< SET ROM WAIT VALUE ARG=RAW WAIT VALUE */
#define SIG_FMTOWNS_VRAM_WAIT     0x10000002 /**< SET VIDEO RAM WAIT VALUE ARG=RAW WAIT VALUE */
#define SIG_FMTOWNS_NOTIFY_RESET  0x10000003 /**< NOTIFY RESET TO VM FROM OUTSIDE. */
#define TOWNS_CRTC_MAX_LINES  1024			 /**< MAXIMUM DISPLAY LINES at VM. */  
#define TOWNS_CRTC_MAX_PIXELS 1024			 /**< MAXIMUM DISPLAY PIXELS PER A LINE at VM. */  

//! These are configuration values
/*!
	@var config.machine_features[0]
	@brief JOYPORT #1
	@see JOYSTICK::update_config()
*/
/*!
	@var config.machine_features[1]
	@brief JOYPORT #2
	@see JOYSTICK::update_config()
*/
/*!
	@var config.machine_features[3]
	@brief FORCE TO USE I386SX.
	@note This is reserved value.Will implement.
*/
#define TOWNS_MACHINE_JOYPORT1		0	/**< CONFIG for JOYPORT #1 */
#define TOWNS_MACHINE_JOYPORT2		1	/**< CONFIG for JOYPORT #2 */
#define TOWNS_MACHINE_WITH_386SX	2	/**< CONFIG for force to use I386SX for any VMs. */
#define TOWNS_MACHINE_MIDI			3	/**< CONFIG for use type of MIDI. */
#define TOWNS_MACHINE_SIO0			4	/**< CONFIG for use type of SIO #0. */
#define TOWNS_MACHINE_SIO1			5	/**< CONFIG for use type of SIO #1. */
#define TOWNS_MACHINE_SIO2			6	/**< CONFIG for use type of SIO #2. */
#define TOWNS_MACHINE_SIO3			7	/**< CONFIG for use type of SIO #3. */
#define TOWNS_MACHINE_LPT0_OUT		8	/**< CONFIG for use type of phisical output of Printer #0. */
#define TOWNS_MACHINE_LPT1_OUT		9	/**< CONFIG for use type of phisical output of Printer #1. */

