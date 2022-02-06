/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.31 -

	[FM-Towns CD-ROM based on SCSI CDROM]
*/
#pragma once

#include "../../common.h"
#include "../device.h"


// 0 - 9 : SCSI_CDROM::
// 100 - : SCSI_DEV::
#define SIG_TOWNS_CDROM_PLAYING				0
#define SIG_TOWNS_CDROM_SAMPLE_L			1
#define SIG_TOWNS_CDROM_SAMPLE_R			2
#define SIG_TOWNS_CDROM_CDDA_PLAY			3
#define SIG_TOWNS_CDROM_CDDA_STOP			4
#define SIG_TOWNS_CDROM_CDDA_PAUSE			5

#define SIG_TOWNS_CDROM_SET_TRACK			0x10
#define SIG_TOWNS_CDROM_MAX_TRACK			0x11
#define SIG_TOWNS_CDROM_IS_MEDIA_INSERTED	0x12
#define SIG_TOWNS_CDROM_REACHED_MAX_TRACK	0x13
#define SIG_TOWNS_CDROM_CURRENT_TRACK		0x14
#define SIG_TOWNS_CDROM_START_MSF			0x15
#define SIG_TOWNS_CDROM_START_MSF_AA		0x16
#define SIG_TOWNS_CDROM_GET_ADR				0x17
#define SIG_TOWNS_CDROM_SET_STAT_TRACK		0x18
#define SIG_TOWNS_CDROM_RELATIVE_MSF		0x20
#define SIG_TOWNS_CDROM_ABSOLUTE_MSF		0x21
#define SIG_TOWNS_CDROM_READ_DATA			0x22
#define SIG_TOWNS_CDROM_RESET				0x23
#define SIG_TOWNS_CDROM_DMAINT				0x24
#define SIG_TOWNS_CDROM_DMAACK				0x25
#define SIG_TOWNS_CDROM_MUTE_L				0x29
#define SIG_TOWNS_CDROM_MUTE_R				0x2a
#define SIG_TOWNS_CDROM_MUTE_ALL			0x2b

class SCSI_HOST;
class FIFO;
class FILEIO;
class DEBUGGER;

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
	/*!
	 * @note Belows are CD-ROM sector structuer.
	 * @note See https://en.wikipedia.org/wiki/CD-ROM#Sector_structure .
	 */
#pragma pack(1)
	typedef struct {
		uint8_t sync[12];
		uint8_t addr_m;
		uint8_t addr_s;
		uint8_t addr_f;
		uint8_t sector_type; //! 1 = MODE1, 2=MODE2
	} cd_data_head_t;
#pragma pack()
#pragma pack(1)
	/*!
	 * @note ToDo: Still not implement crc32 and ecc.
	 * @note 20201116 K.O
	 */
	typedef struct {
		cd_data_head_t header;
		uint8_t data[2048];
		uint8_t crc32[4]; //! CRC32 checksum.
		uint8_t reserved[8];
		uint8_t ecc[276]; //! ERROR CORRECTIOM DATA; by read solomon code.
	} cd_data_mode1_t;
#pragma pack()
#pragma pack(1)
	/*!
	 * 
	 * 
	 */
	typedef struct {
		cd_data_head_t header;
		uint8_t data[2336];
	} cd_data_mode2_t;
#pragma pack()
#pragma pack(1)
	typedef struct {
		uint8_t data[2352];
	} cd_audio_sector_t;
#pragma pack()
#pragma pack(1)
	/*!
	 * @note ToDo: Add fake header and crc and ecc.
	 * @note 20201116 K.O
	 */
	typedef struct {
		uint8_t data[2048];
	} cd_data_iso_t;
#pragma pack()
#pragma pack(1)
	/*!
	 * @note 
	 * @note 20201116 K.O
	 */
	typedef union cdimage_buffer_s {
		uint8_t rawdata[2352]; //!< @note OK?
		cd_data_mode1_t mode1;
		cd_data_mode2_t mode2;
		cd_audio_sector_t audio;
	} cdimage_buffer_t;
#pragma pack()

// From Towns Linux : include/linux/towns_cd.h
enum {
	MODE_AUDIO = 0,
	MODE_MODE1_2352,
	MODE_MODE1_2048,
	MODE_CD_G,
	MODE_MODE2_2336,
	MODE_MODE2_2352,
	MODE_CDI_2336,
	MODE_CDI_2352,
	MODE_NONE
};
enum {
	CCD_PHASE_NULL = 0,
	CCD_PHASE_ENTRY = 1,
	CCD_PHASE_SESSION,
	CCD_PHASE_CLONECD,
	CCD_PHASE_DISC,
	CCD_PHASE_TRACK
};
enum {
	CCD_TYPE_NULL = 0,
	CCD_POINT = 1,
	CCD_CONTROL,
	CCD_PLBA,
	CCD_ALBA,
	CCD_INDEX_0,
	CCD_INDEX_1,
	CCD_MODE,

	CCD_TOC_ENTRIES = 0x101,
	CCD_CDTEXT_LENGTH,

	CCD_PREGAP_MODE = 0x201,
	CCD_PREGAP_SUBC
};
	
enum {
	CDROM_READ_MODE1 = 1,
	CDROM_READ_MODE2 = 2,
	CDROM_READ_RAW   = 3,
	CDROM_READ_AUDIO = 4,
	CDROM_READ_NONE = 0
};
enum {
	CDROM_COMMAND_SEEK =			0x00,
	CDROM_COMMAND_READ_MODE2 =		0x01,
	CDROM_COMMAND_READ_MODE1 =		0x02,
	CDROM_COMMAND_READ_RAW   =		0x03,
	CDROM_COMMAND_PLAY_TRACK =		0x04,
	CDROM_COMMAND_READ_TOC =		0x05,
	CDROM_COMMAND_READ_CDDA_STATE =	0x06,
	CDROM_COMMAND_1F =				0x1f,
	CDROM_COMMAND_SET_STATE =		0x80,
	CDROM_COMMAND_SET_CDDASET =		0x81,
	CDROM_COMMAND_STOP_CDDA =		0x84,
	CDROM_COMMAND_PAUSE_CDDA =		0x85,
	CDROM_COMMAND_RESUME_CDDA =		0x87,
};

// STATUS[0].
// Update from Tsugaru Thanks to Yamakawa-San.
enum {
	TOWNS_CD_STATUS_ACCEPT			= 0x00,
	TOWNS_CD_STATUS_NOT_ACCEPT		= 0x01,
	TOWNS_CD_STATUS_SEEK_COMPLETED	= 0x04,
	TOWNS_CD_STATUS_READ_DONE		= 0x06,
	TOWNS_CD_STATUS_PLAY_DONE		= 0x07,
	TOWNS_CD_STATUS_DOOR_OPEN_DONE	= 0x09,
	TOWNS_CD_STATUS_DISC_NOT_READY	= 0x10,
	TOWNS_CD_STATUS_DOOR_CLOSE_DONE	= 0x10,
	TOWNS_CD_STATUS_STOP_DONE		= 0x11,
	TOWNS_CD_STATUS_PAUSE_DONE		= 0x12,
	TOWNS_CD_STATUS_RESUME_DONE		= 0x13,
	TOWNS_CD_STATUS_TOC_ADDR		= 0x16,
	TOWNS_CD_STATUS_TOC_DATA		= 0x17,
	TOWNS_CD_STATUS_SUBQ_READ		= 0x18,
	TOWNS_CD_STATUS_SUBQ_READ2		= 0x18,
	TOWNS_CD_STATUS_SUBQ_READ3		= 0x18,
	TOWNS_CD_STATUS_CMD_ABEND		= 0x21,
	TOWNS_CD_STATUS_DATA_READY		= 0x22,
	TOWNS_CD_STATUS_UNKNOWN			= 0xff,
};

// status[1] @ status[0] == 00h
// From Tsugaru Thanks to Yamakawa-San.
// Belows are quote from cdrom/cdrom.h for Tsugaru.
//00H 04H xx xx   CDROM BIOS re-shoots command A0H if CDROM returns this code.       (0b00000100)
//00H 08H xx xx   CDROM BIOS re-shoots command A0H if CDROM returns this code.       (0b00001000)
//00H 0DH xx xx   CDROM BIOS Checking (2ndByte)&0x0D and wait for it to be non zero. (0b00001101)
enum {
	TOWNS_CD_ACCEPT_NOERROR			= 0x00,
	TOWNS_CD_ACCEPT_DATA_TRACK		= 0x01,
	TOWNS_CD_ACCEPT_CDDA_PAUSED		= 0x01,
	TOWNS_CD_ACCEPT_CDDA_PLAYING	= 0x03,
	TOWNS_CD_ACCEPT_04H_FOR_CMD_A0H	= 0x04,
	TOWNS_CD_ACCEPT_08H_FOR_CMD_A0H	= 0x08,
	TOWNS_CD_ACCEPT_MEDIA_CHANGED	= 0x09,
	TOWNS_CD_ACCEPT_WAIT			= 0x0d, 
};
		
// status[1] @ status[0] == 21h
// From Tsugaru Thanks to Yamakawa-San.
enum {
	TOWNS_CD_ABEND_PARAMETER_ERROR		= 0x01,
	TOWNS_CD_ABEND_ERR02				= 0x02,
	TOWNS_CD_ABEND_HARDWARE_ERROR_03	= 0x03,
	TOWNS_CD_ABEND_HARDWARE_ERROR_04	= 0x04,
	TOWNS_CD_ABEND_READ_AUDIO_TRACK		= 0x05,
	TOWNS_CD_ABEND_MEDIA_ERROR_06		= 0x06,
	TOWNS_CD_ABEND_DRIVE_NOT_READY		= 0x07,
	TOWNS_CD_ABEND_MEDIA_CHANGED		= 0x08,
	TOWNS_CD_ABEND_HARDWARE_ERROR_09	= 0x09,
	TOWNS_CD_ABEND_ERROR_0C				= 0x0c,
	TOWNS_CD_ABEND_HARDWARE_ERROR_0D	= 0x0d,
	TOWNS_CD_ABEND_RETRY				= 0x0f, // Indicate RETRY ?
};

enum {
	TOWNS_CD_READ_NONE = 0,
	TOWNS_CD_READ_MODE1,
	TOWNS_CD_READ_MODE2,
	TOWNS_CD_READ_RAW,
	TOWNS_CD_READ_CDDA,
	TOWNS_CD_STOP_CDDA,
	TOWNS_CD_PAUSE_CDDA,
	TOWNS_CD_UNPAUSE_CDDA,
};
	
/*class TOWNS_CDROM : public SCSI_CDROM */
class TOWNS_CDROM: public DEVICE {
protected:
	outputs_t outputs_drq;
	outputs_t outputs_mcuint;
	FILEIO* fio_img;
//	FIFO* subq_buffer;
	FIFO* databuffer;
	FIFO* status_queue;

	// For Debugging, will remove 20200822 K.O
	DEVICE* d_cpu;
	DEVICE* d_dmac;
	
	uint32_t max_fifo_length;
	uint32_t fifo_length;
	
	uint16_t cpu_id;
	uint16_t machine_id;
	
	uint8_t data_reg;
	bool dma_transfer;
	bool pio_transfer;
	bool dma_transfer_phase;
	bool pio_transfer_phase;

	bool cdrom_halted;
	bool status_seek;
	
	SUBC_t subq_buffer[98]; // OK?
	int subq_bitptr;
	int subq_bitwidth;
	bool subq_overrun;
	bool is_playing;
	
	int stat_track;

	bool is_cue;
	bool is_iso;
	struct {
		uint8_t type;
		int32_t index0, index1, pregap;
		uint32_t lba_offset;
		uint32_t lba_size;
		bool is_audio;
		int physical_size;
		int logical_size;
	} toc_table[108];
	_TCHAR track_data_path[100][_MAX_PATH];
	_TCHAR img_file_path_bak[_MAX_PATH];
	bool with_filename[100];

	uint32_t cdda_start_frame;
	uint32_t cdda_end_frame;
	uint32_t cdda_playing_frame;

	int cdda_status;
	int cdda_repeat_count;
	bool cdda_interrupt;
	int cdda_buffer_ptr;
	
	int mix_loop_num;
	int current_track;
	int read_sector;
	int transfer_speed;
	int read_length;

	int next_seek_lba;
	int read_mode;
	
	uint8_t prev_command;
	uint8_t latest_command;
	uint8_t reserved_command;
	bool has_status;
	bool req_status;

	bool stat_reply_intr;
	bool mcu_ready;

	bool mcu_intr;
	bool dma_intr;
	bool mcu_intr_mask;
	bool dma_intr_mask;
	
	bool mcuint_val;
	
	int event_drq;
	int event_seek;
	int event_next_sector;
	int event_seek_completed;
	int event_cdda;
	int event_cdda_delay_play;
	int event_cdda_delay_stop;
	int event_delay_interrupt;
	int event_delay_ready;
	int event_halt;
	int event_delay_command;
	int event_time_out;
	int event_eot;
	
	int cdda_sample_l;
	int cdda_sample_r;
		
	int _decibel_l;
	int _decibel_r;
	int volume_l;
	int volume_r;

	bool mute_left;
	bool mute_right;
	
	uint8_t w_regs[16];
	static const uint16_t crc_table[256];

	int param_ptr;
	bool command_entered;
	bool param_filled;
	uint8_t param_pre_queue[8];
	uint8_t param_queue[8];

	bool command_received;
	
	double seek_time;
	int track_num;
	uint32_t max_logical_block;
	int bytes_per_sec;
	bool access;
	bool media_changed;
	bool cdda_stopped;
	uint32_t read_lba;

	bool cdrom_prefetch;
	
	int extra_status;
	void play_cdda_from_cmd();
	void unpause_cdda_from_cmd();
	void stop_cdda_from_cmd();
	void pause_cdda_from_cmd();

	bool is_device_ready();
	void reset_device();
	virtual void read_a_cdda_sample();

	void send_mcu_ready();
	virtual void set_extra_status();

	void set_status(bool _req_status, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
	void set_status_read_done(bool _req_status, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
	void set_status_cddareply(bool _req_status, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
	void set_status_immediate(bool _req_status, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
	virtual void set_status_extra(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
	void set_status_extra_toc_addr(uint8_t s1, uint8_t s2, uint8_t s3);
	void set_status_extra_toc_data(uint8_t s1, uint8_t s2, uint8_t s3);
	virtual int __FASTCALL check_cdda_track_boundary(uint32_t frame_no);
	virtual bool seek_relative_frame_in_image(uint32_t frame_no);
    virtual int prefetch_audio_sectors(int sectors);
	virtual int __FASTCALL dequeue_audio_data(pair16_t& left, pair16_t& right);
	
	virtual void read_cdrom();
	virtual int read_sectors_image(int sectors, uint32_t& transferred_bytes);	

	bool check_notready_and_changed(bool force_int);
	
	virtual void execute_command(uint8_t command);
	
	bool __FASTCALL status_not_ready(bool forceint);
	bool __FASTCALL status_media_changed(bool forceint);
	bool __FASTCALL status_media_changed_or_not_ready(bool forceint);
	
	void __FASTCALL status_hardware_error(bool forceint);
	void __FASTCALL status_parameter_error(bool forceint);
	void __FASTCALL status_read_done(bool forceint);
	void __FASTCALL status_data_ready(bool forceint);
	
	void __FASTCALL status_accept(int extra, uint8_t s2, uint8_t s3);
	void __FASTCALL status_not_accept(int extra, uint8_t s1, uint8_t s2, uint8_t s3);
	
	void __FASTCALL status_illegal_lba(int extra, uint8_t s1, uint8_t s2, uint8_t s3);
	void set_delay_ready();
	void set_delay_ready_nostatus();
	void set_delay_ready_eot();
	void set_delay_ready_cddareply();
	
	uint32_t cdrom_get_adr(int trk);

	void __FASTCALL set_dma_intr(bool val);
	void __FASTCALL set_mcu_intr(bool val);
	/*!
	 * @brief set interrupts without stat_reply_intr.
	 * @param mcu interrupt value to set.
	 * @param dma interrupt value to set.
	 * @param use_mask true when masking mcu and dma.
	 */
	void __FASTCALL set_intrs_force(bool mcu, bool dma, bool use_mask = false);
	
	void __FASTCALL make_bitslice_subc_q(uint8_t *data, int bitwidth);
	uint16_t __FASTCALL calc_subc_crc16(uint8_t *databuf, int bytes, uint16_t initval);

	bool open_cue_file(const _TCHAR* file_path);
	bool parse_cue_file_args(std::string& _arg2, const _TCHAR *parent_dir, std::string& imgpath);
	void parse_cue_track(std::string &_arg2, int& nr_current_track, std::string imgpath);
	int parse_cue_index(std::string &_arg2, int nr_current_track);

	virtual bool open_iso_file(const _TCHAR* file_path);
	virtual bool open_ccd_file(const _TCHAR* file_path, _TCHAR* img_file_path);
	
	virtual uint8_t read_subq();
	virtual uint8_t get_subq_status();
	virtual void set_subq(void);
	
	virtual int get_track_noop(uint32_t lba);
	virtual void get_track_by_track_num(int track);
	virtual uint32_t get_image_cur_position();

	uint32_t __FASTCALL lba_to_msf(uint32_t lba);
	uint32_t __FASTCALL lba_to_msf_alt(uint32_t lba);
	int __FASTCALL get_frames_from_msf(const char *s);
	int64_t __FASTCALL hexatoi(const char *s);
	int64_t __FASTCALL string_to_numeric(std::string s);

	virtual void open_from_cmd(const _TCHAR* file_path);
	virtual void close_from_cmd();
	virtual void do_dma_eot(bool by_signal);

	void __FASTCALL write_mcuint_signals(uint32_t val)
	{
		mcuint_val = (val != 0) ? true : false;
		write_signals(&outputs_mcuint, val);
	}
	void cdrom_debug_log(const char *fmt, ...);
	virtual const _TCHAR* __FASTCALL get_cdda_status_name(int _status);
	virtual const _TCHAR* __FASTCALL get_command_name_from_command(uint8_t cmd);

	bool __CDROM_DEBUG_LOG;
	bool _USE_CDROM_PREFETCH;
	bool force_logging;
public:
	TOWNS_CDROM(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
//		seek_time = 400000; // 400msec (temporary)
		seek_time = 10.0;
		bytes_per_sec = 2048 * 75; // speed x1
		max_logical_block = 0;
		access = false;
		databuffer = NULL;
		status_queue = NULL;
		_decibel_l = 0;
		_decibel_r = 0;
		
		memset(subq_buffer, 0x00, sizeof(subq_buffer));
		
		initialize_output_signals(&outputs_drq);
		initialize_output_signals(&outputs_mcuint);
		set_device_name(_T("FM-Towns CD-ROM drive"));
		d_dmac = NULL;
		// For Debugging, will remove 20200822 K.O
		d_cpu = NULL;
	}
	~TOWNS_CDROM() { }
	virtual void initialize();
	virtual void release();

	virtual void reset();
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_dma_io8(uint32_t addr);
	virtual void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data);
	
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t __FASTCALL read_signal(int id);
	
	virtual void __FASTCALL event_callback(int event_id, int err);
	virtual void __FASTCALL mix(int32_t* buffer, int cnt);
	
	virtual bool process_state(FILEIO* state_fio, bool loading);

	virtual bool mounted();
	virtual bool accessed();
	virtual void open(const _TCHAR* file_path);
	virtual void close();

	// for debug
	virtual void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool is_debugger_available()
	{
		return true;
	}
	uint64_t get_debug_data_addr_space()
	{
		return 0x1fff; // Will change
	}

	
	virtual void set_volume(int ch, int decibel_l, int decibel_r);
	virtual void get_volume(int ch, int& decibel_l, int& decibel_r);
	virtual bool read_buffer(int sectors);

	// unique functions
	// Towns specified command
	virtual void set_cdda_status(uint8_t status);
	int get_track(uint32_t lba);
	virtual double get_seek_time(uint32_t lba);
	virtual uint8_t read_status();
	virtual const int logical_block_size();
	virtual const int physical_block_size();
	virtual bool write_a_byte(uint8_t val)
	{
		uint32_t n = val;
		n = n & 0xff;
//		if(databuffer->count() >= fifo_length) {
//			return false;
//		}
		databuffer->write((int)n);
		return true;
	}
	virtual bool write_bytes(uint8_t* val, int bytes)
	{
		int n_count = databuffer->count();
		if((val == NULL) ||
		   (n_count >= max_fifo_length) || ((n_count + bytes) >= fifo_length)) {
			return false;
		}
		for(int i = 0; i < bytes; i++) {
			int d = ((int)val[i]) & 0xff ;
			databuffer->write(d);
		}
		return true;
	}
	virtual bool change_buffer_size(int size)
	{
		if((size <= 0) || (size >= max_fifo_length) || (databuffer == NULL)) return false;
		uint8_t tbuf[size];
		if(fifo_length > size) { // truncate
			// Dummy read
			for(int i = 0; i < (fifo_length - size); i++) {
				uint8_t dummy = (uint8_t)(databuffer->read() & 0xff);
			}
			for(int i = 0; i < size; i++) {
				tbuf[i] = (uint8_t)(databuffer->read() & 0xff);
			}
			databuffer->clear();
			for(int i = 0; i < size; i++) {
				databuffer->write(tbuf[i]);
			}
		} else if(fifo_length < size) {
			for(int i = 0; i < fifo_length; i++) {
				tbuf[i] = (uint8_t)(databuffer->read() & 0xff);
			}
			databuffer->clear();
			for(int i = 0; i < fifo_length; i++) {
				databuffer->write(tbuf[i]);
			}
//			for(int i = 0; i < (size - fifo_size); i++) {
//				databuffer->write(0);
//			}
		}
		fifo_length = size;
		return true;
	}
	uint8_t get_cdda_status()
	{
		return cdda_status;
	}

	void set_machine_id(uint16_t val)
	{
		machine_id = val & 0xfff8;
	}
	void set_cpu_id(uint16_t val)
	{
		cpu_id = val & 0x07;
	}
	
	void set_context_mpuint_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_mcuint, dev, id, mask);
	}
	void set_context_drq_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_drq, dev, id, mask);
	}
	void set_context_dmac(DEVICE* d)
	{
		d_dmac = d;
	}
	// For Debugging, will remove 20200822 K.O
	void set_context_cpu(DEVICE* d)
	{
		d_cpu = d;
	}
};

	
}
