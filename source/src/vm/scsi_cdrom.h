/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.06-

	[ SCSI CD-ROM drive ]
*/

#ifndef _SCSI_CDROM_H_
#define _SCSI_CDROM_H_

#include "scsi_dev.h"

#define SIG_SCSI_CDROM_PLAYING	0
#define SIG_SCSI_CDROM_SAMPLE_L	1
#define SIG_SCSI_CDROM_SAMPLE_R	2

class FILEIO;

class SCSI_CDROM : public SCSI_DEV
{
protected:
	outputs_t outputs_done;
	
	FILEIO* fio_img;
	struct {
		uint32_t index0, index1, pregap;
		uint32_t lba_offset;
		uint32_t lba_size;
		bool is_audio;
	} toc_table[1024];
	bool is_cue;
	_TCHAR track_data_path[99][_MAX_PATH];
	int current_track;
	
	int track_num;
	uint32_t max_logical_block;
	bool access;
	
	uint32_t cdda_start_frame, cdda_end_frame, cdda_playing_frame;
	uint8_t cdda_status;
	bool cdda_repeat, cdda_interrupt;
	uint8_t cdda_buffer[2352 * 75];
	int cdda_buffer_ptr;
	int cdda_sample_l, cdda_sample_r;
	int event_cdda, mix_loop_num;
	int event_cdda_delay_play;
	
	void set_cdda_status(uint8_t status);
	int get_track(uint32_t lba);
	double get_seek_time(uint32_t lba);
	
	int volume_m;
	int volume_l, volume_r;

	bool open_cue_file(const _TCHAR *file_path);
	void get_track_by_track_num(int track);
	int get_track_noop(uint32_t lba);
	uint32_t lba_to_msf(uint32_t lba);
	uint32_t lba_to_msf_alt(uint32_t lba);
public:
	SCSI_CDROM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : SCSI_DEV(parent_vm, parent_emu) 
	{
		initialize_output_signals(&outputs_done);
		
		volume_m = 1024;
		volume_l = volume_r = 1024;
		
		my_sprintf_s(vendor_id, 9, "NECITSU");
		my_sprintf_s(product_id, 17, "SCSI-CDROM");
		device_type = 0x05; // CD-ROM drive
		is_removable = true;
		is_hot_swappable = false;
//		seek_time = 400000; // 400msec (temporary)
		seek_time = 10.0;
		bytes_per_sec = 2048 * 75; // speed x1
		max_logical_block = 0;
		access = false;

		set_device_name(_T("SCSI CDROM"));
	}
	~SCSI_CDROM() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	uint32_t read_signal(int id);
	void event_callback(int event_id, int err);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// virtual scsi functions
	void reset_device();
	bool is_device_ready();
	uint32_t physical_block_size()
	{
		return 2352;
	}
	uint32_t logical_block_size()
	{
		return 2048;
	}
	uint32_t max_logical_block_addr()
	{
		if(max_logical_block > 0) {
			return max_logical_block - 1;
		}
		return 0;
	}
	int get_command_length(int value);
	void start_command();
	bool read_buffer(int length);
	
	// unique functions
	void set_context_done(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_done, device, id, mask);
	}
	void open(const _TCHAR* file_path);
	void close();
	bool mounted();
	bool accessed();
	void set_volume(int volume);
};

#endif

