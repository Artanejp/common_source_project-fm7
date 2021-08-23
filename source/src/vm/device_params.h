/*!
 * @file device_params.h
 * @brief For device.h :  commonly definition values.
 * @author Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
 * @version 1.0
 * @data 2021-08-23
 */
#pragma once

//!< max devices connected to the output port
#define MAX_OUTPUT	16

//!< common signal id
#define SIG_CPU_IRQ				101
#define SIG_CPU_FIRQ			102
#define SIG_CPU_NMI				103
#define SIG_CPU_BUSREQ			104
#define SIG_CPU_HALTREQ			105
#define SIG_CPU_DEBUG			106
#define SIG_CPU_ADDRESS_DIRTY	107
#define SIG_CPU_TOTAL_CYCLE_LO	108
#define SIG_CPU_TOTAL_CYCLE_HI	109
#define SIG_CPU_WAIT_FACTOR		110

#define SIG_PRINTER_DATA	201
#define SIG_PRINTER_STROBE	202
#define SIG_PRINTER_RESET	203
#define SIG_PRINTER_BUSY	204
#define SIG_PRINTER_ACK		205
#define SIG_PRINTER_SELECT	206

#define SIG_SCSI_DAT			301
#define SIG_SCSI_BSY			302
#define SIG_SCSI_CD				303
#define SIG_SCSI_IO				304
#define SIG_SCSI_MSG			305
#define SIG_SCSI_REQ			306
#define SIG_SCSI_SEL			307
#define SIG_SCSI_ATN			308
#define SIG_SCSI_ACK			309
#define SIG_SCSI_RST			310
#define SIG_SCSI_16BIT_BUS		311
#define SIG_SCSI_CLEAR_QUEUE	312
