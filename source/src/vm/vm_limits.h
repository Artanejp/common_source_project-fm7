/*
	Skelton for retropc emulator

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2022.11.05-

	[ limit values for some VM ]
	History: 2022-11-05 Split from event.h .
*/

#pragma once

#define MAX_DEVICE	128		/*!< Maximum devices to manage. */
#define MAX_CPU		8		/*!> Maximum number of CPUs to RUN. */
#define MAX_SOUND	32		/*!< Maximum number of sound sources. */
#define MAX_LINES	1024	/*!< Maximum lines per a frame. */
#define MAX_EVENT	128		/*!< Maximum events to manage. */
#define NO_EVENT	-1		/*!< This has no event */

