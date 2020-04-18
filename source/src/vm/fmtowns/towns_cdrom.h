/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.31 -

	[FM-Towns CD-ROM based on SCSI CDROM]
*/
#pragma once

#include "../../common.h"
#include "../scsi_dev.h"
#include "../scsi_cdrom.h"

// 0 - 9 : SCSI_CDROM::
// 100 - : SCSI_DEV::
#define SIG_TOWNS_CDROM_SET_TRACK         0x10
#define SIG_TOWNS_CDROM_MAX_TRACK         0x11
#define SIG_TOWNS_CDROM_IS_MEDIA_INSERTED 0x12
#define SIG_TOWNS_CDROM_REACHED_MAX_TRACK 0x13
#define SIG_TOWNS_CDROM_CURRENT_TRACK     0x14
#define SIG_TOWNS_CDROM_START_MSF         0x15
#define SIG_TOWNS_CDROM_START_MSF_AA      0x16
#define SIG_TOWNS_CDROM_GET_ADR           0x17
#define SIG_TOWNS_CDROM_SET_STAT_TRACK    0x18
#define SIG_TOWNS_CDROM_RELATIVE_MSF      0x20
#define SIG_TOWNS_CDROM_ABSOLUTE_MSF      0x21

// Virtual (pseudo) SCSI command.
#define TOWNS_CDROM_CDDA_PLAY             0xf0
#define TOWNS_CDROM_CDDA_PAUSE            0xf1
#define TOWNS_CDROM_CDDA_UNPAUSE          0xf2
#define TOWNS_CDROM_CDDA_STOP             0xf3

class SCSI_HOST;
class FIFO;
class FILEIO;

namespace FMTOWNS {
	class CDC;
}

namespace FMTOWNS {
	#pragma pack(1)
	typedef union {
		struct {
			uint8_t P:1;
			uint8_t Q:1;
			uint8_t R:1;
			uint8_t S:1;
			uint8_t T:1;
			uint8_t U:1;
			uint8_t V:1;
			uint8_t W:1;
		} bit;
		uint8_t byte;
	} SUBC_t;
#pragma pack()

class TOWNS_CDROM : public SCSI_CDROM {
protected:
	outputs_t outputs_drq;
	outputs_t outputs_next_sector;
	outputs_t outputs_done;
	outputs_t outputs_mcuint;
	
	FIFO* subq_buffer;
	FIFO* buffer;
	FIFO* status_queue;
	FIFO* status_pre_queue;
	
//	SUBC_t subq_buffer[98]; // OK?
	int subq_bitptr;
	int subq_bitwidth;
	bool subq_overrun;
	
	int stat_track;
	int cdda_status;
	
	int read_length;
	
	uint8_t extra_command;
	bool mcu_intr;
	bool dma_intr;
	bool mcu_intr_mask;
	bool dma_intr_mask;
	
	int event_drq;
	int event_next_sector;
	int event_cdda;
	int event_cdda_delay_play;
	int event_delay_interrupt;
	
	static const uint16_t crc_table[256];
	uint8_t param_queue[8];
	
	virtual void play_cdda_from_cmd();
	virtual void pause_cdda_from_cmd();
	virtual void unpause_cdda_from_cmd();
	virtual void stop_cdda_from_cmd();

//	bool is_device_ready();
//	void reset_device();
//	void set_cdda_status(uint8_t status);
//	void read_a_cdda_sample();
//	bool read_buffer(int length);
//	void set_status(uint8_t cmd, bool type0, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
//	void set_status_extra(uint8_t cmd, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
//	void copy_status_queue();
//	void read_cdrom(bool req_reply);
//	uint32_t execute_command(uint8_t command);
//	void set_dma_intr(bool val);
//	void set_mcu_intr(bool val);
	

//	void make_bitslice_subc_q(uint8_t *data, int bitwidth);
	uint16_t calc_subc_crc16(uint8_t *databuf, int bytes, uint16_t initval);
//	int get_track(uint32_t lba);
//	int get_track_noop(uint32_t lba);
//	void get_track_by_track_num(int track);

//	double get_seek_time(uint32_t lba);
//	uint32_t lba_to_msf(uint32_t lba);
//	uint32_t lba_to_msf_alt(uint32_t lba);

public:
	TOWNS_CDROM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : SCSI_CDROM(parent_vm, parent_emu)
	{
		my_sprintf_s(vendor_id, 9, "FUJITSU");
		my_sprintf_s(product_id, 17, "CDROM");
		device_type = 0x05; // CD-ROM drive
		is_removable = true;
		is_hot_swappable = false;
//		seek_time = 400000; // 400msec (temporary)
		seek_time = 10.0;
		bytes_per_sec = 2048 * 75; // speed x1
		max_logical_block = 0;
		access = false;
		subq_buffer = NULL;
		buffer = NULL;
		status_queue = NULL;
		status_pre_queue = NULL;
		
		initialize_output_signals(&outputs_drq);
		initialize_output_signals(&outputs_next_sector);
		initialize_output_signals(&outputs_done);
		initialize_output_signals(&outputs_mcuint);
		
		set_device_name(_T("FM-Towns CD-ROM drive"));
	}
	~TOWNS_CDROM() { }
	virtual void initialize();
	virtual void release();

	virtual void reset();
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t __FASTCALL read_signal(int id);
	
	virtual void event_callback(int event_id, int err);

	virtual bool process_state(FILEIO* state_fio, bool loading);

	// SCSI SPECIFIC COMMANDS
	virtual int get_command_length(int value);
	virtual void start_command();

	// Towns specified command
	virtual void set_subq(void);
	virtual uint8_t get_subq_status();
	virtual uint8_t read_subq();
	virtual void set_cdda_status(uint8_t status)
	{
		SCSI_CDROM::set_cdda_status(status);
	}
	virtual int get_track(uint32_t lba)
	{
		return SCSI_CDROM::get_track(lba);
	}
	virtual double get_seek_time(uint32_t lba)
	{
		return SCSI_CDROM::get_seek_time(lba);
	}
	virtual uint8_t get_cdda_status()
	{
		return cdda_status;
	}
	void set_read_mode(bool is_mode2)
	{
		read_mode = is_mode2;
	}

	void set_context_done_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_done, dev, id, mask);
	}
	void set_context_next_sector_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_next_sector, dev, id, mask);
	}
	void set_context_mpuint_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_mcuint, dev, id, mask);
	}
	void set_context_drq_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_drq, dev, id, mask);
	}
};

}
