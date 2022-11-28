/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI base device ]
*/

#ifndef _SCSI_DEV_H_
#define _SCSI_DEV_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SCSI_BUFFER_SIZE	0x10000

static const _TCHAR* scsi_phase_name[9] = {
	_T("Data Out"),
	_T("Data In"),
	_T("Reserved Out"),
	_T("Reverved In"),
	_T("Command"),
	_T("Status"),
	_T("Message Out"),
	_T("Message In"),
	_T("Bus Free"),
};

#define SCSI_PHASE_DATA_OUT	0	// C/D = 0, MSG = 0, I/O = 0
#define SCSI_PHASE_DATA_IN	1	// C/D = 0, MSG = 0, I/O = 1
#define SCSI_PHASE_COMMAND	4	// C/D = 1, MSG = 0, I/O = 0
#define SCSI_PHASE_STATUS	5	// C/D = 1, MSG = 0, I/O = 1
#define SCSI_PHASE_MESSAGE_OUT	6	// C/D = 1, MSG = 1, I/O = 0
#define SCSI_PHASE_MESSAGE_IN	7	// C/D = 1, MSG = 1, I/O = 1
#define SCSI_PHASE_BUS_FREE	8	// C/D = 0, MSG = 0, I/O = 0

// http://www.geocities.jp/pd4pc/HardSoft/SCSI/SCSI_Command.htm

#define SCSI_CMD_CHANGE_DEF	0x40	// Change Definition (Optional)
#define SCSI_CMD_COMPARE	0x39	// Compare (O)
#define SCSI_CMD_COPY		0x18	// Copy (O)
#define SCSI_CMD_COP_VERIFY	0x3A	// Copy and Verify (O)
#define SCSI_CMD_INQUIRY	0x12	// Inquiry (MANDATORY)
#define SCSI_CMD_LOG_SELECT	0x4C	// Log Select (O)
#define SCSI_CMD_LOG_SENSE	0x4D	// Log Sense (O)
#define SCSI_CMD_MODE_SEL6	0x15	// Mode Select 6-byte (Device Specific)
#define SCSI_CMD_MODE_SEL10	0x55	// Mode Select 10-byte (Device Specific)
#define SCSI_CMD_MODE_SEN6	0x1A	// Mode Sense 6-byte (Device Specific)
#define SCSI_CMD_MODE_SEN10	0x5A	// Mode Sense 10-byte (Device Specific)
#define SCSI_CMD_READ_BUFF	0x3C	// Read Buffer (O)
#define SCSI_CMD_REQ_SENSE	0x03	// Request Sense (MANDATORY)
#define SCSI_CMD_SEND_DIAG	0x1D	// Send Diagnostic (O)
#define SCSI_CMD_RCV_DIAG	0x1C	// Receive Diagnostic Results (O)
#define SCSI_CMD_TST_U_RDY	0x00	// Test Unit Ready (MANDATORY)
#define SCSI_CMD_WRITE_BUFF	0x3B	// Write Buffer (O)
#define SCSI_CMD_FORMAT		0x04	// Format Unit (MANDATORY)
#define SCSI_CMD_LCK_UN_CAC	0x36	// Lock Unlock Cache (O)
#define SCSI_CMD_PREFETCH	0x34	// Prefetch (O)
#define SCSI_CMD_MED_REMOVL	0x1E	// Prevent/Allow medium Removal (O)
#define SCSI_CMD_READ6		0x08	// Read 6-byte (MANDATORY)
#define SCSI_CMD_READ10		0x28	// Read 10-byte (MANDATORY)
#define SCSI_CMD_READ12		0xa8	// Read 10-byte (MANDATORY)
#define SCSI_CMD_RD_CAPAC	0x25	// Read Capacity (MANDATORY)
#define SCSI_CMD_RD_DEFECT	0x37	// Read Defect Data (O)
#define SCSI_CMD_READ_LONG	0x3E	// Read Long (O)
#define SCSI_CMD_REASS_BLK	0x07	// Reassign Blocks (O)
#define SCSI_CMD_RELEASE	0x17	// Release Unit (MANDATORY)
#define SCSI_CMD_RESERVE	0x16	// Reserve Unit (MANDATORY)
#define SCSI_CMD_REZERO		0x01	// Rezero Unit (O)
#define SCSI_CMD_SRCH_DAT_E	0x31	// Search Data Equal (O)
#define SCSI_CMD_SRCH_DAT_H	0x30	// Search Data High (O)
#define SCSI_CMD_SRCH_DAT_L	0x32	// Search Data Low (O)
#define SCSI_CMD_SEEK6		0x0B	// Seek 6-Byte (O)
#define SCSI_CMD_SEEK10		0x2B	// Seek 10-Byte (O)
#define SCSI_CMD_SET_LIMIT	0x33	// Set Limits (O)
#define SCSI_CMD_START_STP	0x1B	// Start/Stop Unit (O)
#define SCSI_CMD_SYNC_CACHE	0x35	// Synchronize Cache (O)
#define SCSI_CMD_VERIFY		0x2F	// Verify (O)
#define SCSI_CMD_WRITE6		0x0A	// Write 6-Byte (MANDATORY)
#define SCSI_CMD_WRITE10	0x2A	// Write 10-Byte (MANDATORY)
#define SCSI_CMD_WRITE12	0xAA	// Write 10-Byte (MANDATORY)
#define SCSI_CMD_WRT_VERIFY	0x2E	// Write and Verify (O)
#define SCSI_CMD_WRITE_LONG	0x3F	// Write Long (O)
#define SCSI_CMD_WRITE_SAME	0x41	// Write Same (O)
#define SCSI_CMD_PLAYAUD_10	0x45 	// Play Audio 10-Byte (O)
#define SCSI_CMD_PLAYAUD_12	0xA5 	// Play Audio 12-Byte 12-Byte (O)
#define SCSI_CMD_PLAYAUDMSF	0x47 	// Play Audio MSF (O)
#define SCSI_CMD_PLAYA_TKIN	0x48 	// Play Audio Track/Index (O)
#define SCSI_CMD_PLYTKREL10	0x49 	// Play Track Relative 10-Byte (O)
#define SCSI_CMD_PLYTKREL12	0xA9 	// Play Track Relative 12-Byte (O)
#define SCSI_CMD_READCDCAP	0x25 	// Read CD-ROM Capacity (MANDATORY)
#define SCSI_CMD_READHEADER	0x44 	// Read Header (O)
#define SCSI_CMD_SUBCHANNEL	0x42 	// Read Subchannel (O)
#define SCSI_CMD_READ_TOC	0x43 	// Read TOC (O)

#define SASI_CMD_SPECIFY	0xC2	// Winchester Drive Parameters

#define SCSI_STATUS_GOOD	0x00	// Status Good
#define SCSI_STATUS_CHKCOND	0x02	// Check Condition
#define SCSI_STATUS_CONDMET	0x04	// Condition Met
#define SCSI_STATUS_BUSY	0x08	// Busy 
#define SCSI_STATUS_INTERM	0x10	// Intermediate
#define SCSI_STATUS_INTCDMET	0x14	// Intermediate-Condition Met
#define SCSI_STATUS_RESCONF	0x18	// Reservation Conflict
#define SCSI_STATUS_COMTERM	0x22	// Command Terminated
#define SCSI_STATUS_QFULL	0x28	// Queue Full

#define SCSI_SERROR_CURRENT	0x70	// Current Errors
#define SCSI_SERROR_DEFERED	0x71	// Deferred Errors

#define SCSI_KEY_NOSENSE	0x00	// No Sense
#define SCSI_KEY_RECERROR	0x01	// Recovered Error
#define SCSI_KEY_NOTREADY	0x02	// Not Ready
#define SCSI_KEY_MEDIUMERR	0x03	// Medium Error
#define SCSI_KEY_HARDERROR	0x04	// Hardware Error
#define SCSI_KEY_ILLGLREQ	0x05	// Illegal Request
#define SCSI_KEY_UNITATT	0x06	// Unit Attention
#define SCSI_KEY_DATAPROT	0x07	// Data Protect
#define SCSI_KEY_BLANKCHK	0x08	// Blank Check
#define SCSI_KEY_VENDSPEC	0x09	// Vendor Specific
#define SCSI_KEY_COPYABORT	0x0A	// Copy Abort
#define SCSI_KEY_ABORT		0x0B	// Abort
#define SCSI_KEY_EQUAL		0x0C	// Equal (Search)
#define SCSI_KEY_VOLOVRFLW	0x0D	// Volume Overflow
#define SCSI_KEY_MISCOMP	0x0E	// Miscompare (Search)
#define SCSI_KEY_RESERVED	0x0F	// Reserved

#define SCSI_SENSE_NOSENSE	0x00	// No Sense
#define SCSI_SENSE_NOTREADY	0x04	// Not Ready
#define SCSI_SENSE_NORECORDFND	0x14	// No Record Found
#define SCSI_SENSE_SEEKERR	0x15	// Seek Error
#define SCSI_SENSE_ILLGLBLKADDR	0x21	// Illegal Block Address
#define SCSI_SENSE_WRITEPROTCT	0x27	// Write Protected

class FIFO;

class SCSI_DEV : public DEVICE
{
private:
	outputs_t outputs_dat;
	outputs_t outputs_bsy;
	outputs_t outputs_cd;
	outputs_t outputs_io;
	outputs_t outputs_msg;
	outputs_t outputs_req;
	
	uint32_t data_bus;
	bool sel_status, atn_status, ack_status, rst_status;
	bool selected, atn_pending;
	
	int phase, next_phase, next_req;
	int event_sel, event_phase, event_req;
	uint32_t first_req_clock;
	double next_req_usec;
	
	uint8_t sense_code;
	
public:
	SCSI_DEV(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_dat);
		initialize_output_signals(&outputs_bsy);
		initialize_output_signals(&outputs_cd);
		initialize_output_signals(&outputs_io);
		initialize_output_signals(&outputs_msg);
		initialize_output_signals(&outputs_req);
		
		set_device_name(_T("SCSI Device"));
	}
	~SCSI_DEV() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_interface(DEVICE* device)
	{
#ifdef SCSI_HOST_WIDE
		register_output_signal(&outputs_dat, device, SIG_SCSI_DAT, 0xffff);
#else
		register_output_signal(&outputs_dat, device, SIG_SCSI_DAT, 0xff);
#endif
		register_output_signal(&outputs_dat, device, SIG_SCSI_DAT, 1 << scsi_id);
		register_output_signal(&outputs_bsy, device, SIG_SCSI_BSY, 1 << scsi_id);
		register_output_signal(&outputs_cd,  device, SIG_SCSI_CD,  1 << scsi_id);
		register_output_signal(&outputs_io,  device, SIG_SCSI_IO,  1 << scsi_id);
		register_output_signal(&outputs_msg, device, SIG_SCSI_MSG, 1 << scsi_id);
		register_output_signal(&outputs_req, device, SIG_SCSI_REQ, 1 << scsi_id);
	}
	uint8_t get_sense_code()
	{
		return sense_code;
	}
	void set_sense_code(uint8_t value)
	{
		sense_code = value;
	}
	void set_phase(int value);
	void set_phase_delay(int value, double usec);
	void set_dat(int value);
	void set_bsy(int value);
	void set_cd(int value);
	void set_io(int value);
	void set_msg(int value);
	void set_req(int value);
	void set_req_delay(int value, double usec);
	
	virtual void reset_device() {}
	virtual bool is_device_existing()
	{
		return true;
	}
	virtual bool is_device_ready()
	{
		return true;
	}
	virtual uint32_t physical_block_size()
	{
		return 0;
	}
	virtual uint32_t logical_block_size()
	{
		return 0;
	}
	virtual uint32_t max_logical_block_addr()
	{
		return 0;
	}
	virtual double get_seek_time(uint64_t new_position, uint64_t length)
	{
		return seek_time;
	}
	virtual int get_command_length(int value);
	virtual void start_command();
	virtual bool read_buffer(int length);
	virtual bool write_buffer(int length);
	
	uint8_t get_cur_command()
	{
		return command[0];
	}
	uint8_t get_logical_unit_number()
	{
		return command[1] >> 5;
	}
	uint8_t command[12];
	int command_index;
	
	FIFO *buffer;
	uint64_t position, remain;
	
	char vendor_id[8 + 1];
	char product_id[16 + 1];
	uint8_t device_type;
	bool is_removable;
	bool is_hot_swappable;
	double seek_time;
	int bytes_per_sec;
	double data_req_delay;
	
	int scsi_id;
};

#endif

