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
#define SIG_SCSI_CDROM_CDDA_PLAY	3
#define SIG_SCSI_CDROM_CDDA_STOP	4
#define SIG_SCSI_CDROM_CDDA_PAUSE	5

class FILEIO;

class  DLL_PREFIX SCSI_CDROM : public SCSI_DEV
{
protected:
	outputs_t outputs_done;

	FILEIO* fio_img;
	struct {
		int32_t index0, index1, pregap;
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

	uint32_t cdda_start_frame, cdda_start_pregap;
	uint32_t cdda_end_frame;
	uint32_t cdda_playing_frame;
	uint8_t cdda_status;
//	bool cdda_repeat, cdda_interrupt;
	uint8_t cdda_play_mode;

	uint8_t cdda_buffer[2352 * 75];
	int read_sectors;
	int cdda_buffer_ptr;
	int cdda_sample_l, cdda_sample_r;
	int event_cdda, mix_loop_num;
	int event_cdda_delay_play;
	int event_delay_interrupt;

	bool read_mode;

	void __FASTCALL set_cdda_status(uint8_t status);
	int get_track(uint32_t lba);

	int volume_m;
	int volume_l, volume_r;

	bool open_cue_file(const _TCHAR *file_path);
	void __FASTCALL get_track_by_track_num(int track);
	int __FASTCALL get_track_noop(uint32_t lba);
	uint32_t __FASTCALL lba_to_msf(uint32_t lba);
	uint32_t __FASTCALL lba_to_msf_alt(uint32_t lba);
	
	// Backport from FM TOWNS.
	// 20240910 K.O
	uint32_t get_image_cur_position();
	
	bool __CDROM_DEBUG_LOG;
public:
	SCSI_CDROM(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : SCSI_DEV(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_done);
		__CDROM_DEBUG_LOG = false;
		volume_m = 1024;
		volume_l = volume_r = 1024;

		my_sprintf_s(vendor_id, 9, "NECITSU");
		my_sprintf_s(product_id, 17, "SCSI-CDROM");
		my_sprintf_s(product_rev, 5, "1.0");
		device_type = 0x05; // CD-ROM drive
		is_removable = true;
		is_hot_swappable = false;
//		seek_time = 400000; // 400msec (temporary)
		seek_time = 10.0;
		bytes_per_sec = 2048 * 75; // speed x1
		data_req_delay = 0.1;
		max_logical_block = 0;
		access = false;

		set_device_name(_T("SCSI CDROM"));
	}
	~SCSI_CDROM() {}

	// common functions
	virtual void initialize() override;
	virtual void release() override;
	
	virtual void reset() override;
	
	virtual uint32_t __FASTCALL read_signal(int id) override;
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;

	virtual void __FASTCALL event_callback(int event_id, int err) override;
	virtual void __FASTCALL mix(int32_t* buffer, int cnt) override;
	virtual void set_volume(int ch, int decibel_l, int decibel_r) override;
	virtual bool process_state(FILEIO* state_fio, bool loading) override;
	virtual void out_debug_log(const _TCHAR *format, ...) override;

	// virtual scsi functions
	virtual void reset_device() override;
	virtual bool is_device_ready() override;
	uint32_t physical_block_size() override
	{
		return 2352;
	}
	uint32_t logical_block_size() override
	{
		return read_mode ? 2340 : 2048;
	}
	uint32_t max_logical_block_addr() override
	{
		if(max_logical_block > 0) {
			return max_logical_block - 1;
		}
		return 0;
	}

	// unique functions
	void set_context_done(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_done, device, id, mask);
	}
	double __FASTCALL get_seek_time(uint64_t new_position, uint64_t length) override;
	double __FASTCALL get_seek_time(uint32_t lba)
	{
		return get_seek_time((uint64_t)lba, 1);
	}
	
	virtual int __FASTCALL get_command_length(int value) override;
	virtual void start_command() override;
	virtual bool read_buffer(int length) override;
	virtual bool write_buffer(int length) override;
	
	void open(const _TCHAR* file_path);
	void close();
	bool mounted();
	bool accessed();
	void set_volume(int volume);
};

#endif
