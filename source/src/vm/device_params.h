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
#define SIG_CPU_IRQ				101	/*!< Make normal interrupt (IRQ) to a target CPU. */
#define SIG_CPU_FIRQ			102	/*!< Make fast interrupt (FIRQ) to a target CPU. */
#define SIG_CPU_NMI				103	/*!< Make non maskable  interrupt (NMI) to a target CPU. */
#define SIG_CPU_BUSREQ			104	/*!< Make bus request (BUSREQ) to a target CPU. Normally non-zero request to release bus (mostly to halt) to target.*/
#define SIG_CPU_HALTREQ			105	/*!< Make HALT request to a target CPU.Some devices requires halt request separating with BUSREQ. */
#define SIG_CPU_DEBUG			106	/*!< Make DEBUG pin request to a target */ 
#define SIG_CPU_ADDRESS_DIRTY	107	/*!< Notify  ARG address of taget made dirty.Will use for cache controlling. */ 
#define SIG_CPU_TOTAL_CYCLE_LO	108	/*!< Get LOW DWORD of total_icount of a target. */
#define SIG_CPU_TOTAL_CYCLE_HI	109	/*!< Get HIGH DWORD of total_icount of a target. */
#define SIG_CPU_WAIT_FACTOR		110	/*!< Set / Get wait factor of a target.This is useful for variable CPU clocks. This value encodes multiply of 65536. */

#define SIG_PRINTER_DATA	201 /*! Read/Write DATA of (pseudo) printer port (normally 8bit or 16bit width) . */ 
#define SIG_PRINTER_STROBE	202 /*! Read/Write STROBE SIGNAL to (pseudo) printer port. */ 
#define SIG_PRINTER_RESET	203 /*! Read/Write RESET SIGNAL of (pseudo) printer port. */ 
#define SIG_PRINTER_BUSY	204 /*! Read/Write BUSY SIGNAL of (pseudo) printer port. */ 
#define SIG_PRINTER_ACK		205 /*! Read/Write ACKNOWLEDGE SIGNAL of (pseudo) printer port. */ 
#define SIG_PRINTER_SELECT	206 /*! Read/Write SELECT SIGNAL of (pseudo) printer port. */ 

#define SIG_SCSI_DAT			301 /*! Read from / Write to a DATA BUS of SCSI bus.Normally 8bit width, but sometimes 16bit width. */ 
#define SIG_SCSI_BSY			302 /*! Read/Send a BUSY SIGNAL of SCSI bus.*/
#define SIG_SCSI_CD				303 /*! Read/Send a CD SIGNAL of SCSI bus.*/
#define SIG_SCSI_IO				304 /*! Read/Send a I/O SIGNAL of SCSI bus.*/
#define SIG_SCSI_MSG			305 /*! Read/Send a MESSAGE IN SIGNAL of SCSI bus.*/
#define SIG_SCSI_REQ			306 /*! Read/Send a REQ SIGNAL of SCSI bus.*/
#define SIG_SCSI_SEL			307 /*! Read/Send a SELECT SIGNAL of SCSI bus.*/
#define SIG_SCSI_ATN			308 /*! Read/Send a ATTENTION SIGNAL of SCSI bus.*/
#define SIG_SCSI_ACK			309 /*! Read/Send a ACKNOWLEDGE SIGNAL of SCSI bus.*/
#define SIG_SCSI_RST			310 /*! Read/Send a RESET SIGNAL of SCSI bus.*/
#define SIG_SCSI_16BIT_BUS		311 /*! Read/Send a "Notify whether BUS WIDTH is 16bit" SIGNAL of SCSI bus.*/
#define SIG_SCSI_CLEAR_QUEUE	312 /*! Read/Send a request to clear queue of target SCSI device.*/
