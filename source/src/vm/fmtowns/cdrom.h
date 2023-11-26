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
#define SIG_TOWNS_CDROM_DMAMASK				0x26

#define SIG_TOWNS_CDROM_MUTE_L				0x29
#define SIG_TOWNS_CDROM_MUTE_R				0x2a
#define SIG_TOWNS_CDROM_MUTE_ALL			0x2b
#define SIG_TOWNS_CDROM_VOLUME_OFFSET_L		0x2c
#define SIG_TOWNS_CDROM_VOLUME_OFFSET_R		0x2d
#define SIG_TOWNS_CDROM_VOLUME_OFFSET_ALL	0x2e


class SCSI_HOST;
class FIFO;
class RINGBUFFER;
class FILEIO;
class DEBUGGER;

namespace FMTOWNS {
	#pragma pack(1)
	typedef union SUBC_u {
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
	CDROM_COMMAND_86 =				0x86,
	CDROM_COMMAND_RESUME_CDDA =		0x87,
	CDROM_COMMAND_9F =				0x9f,
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
	TOWNS_CD_STATUS_SUBQ_READ1		= 0x18,
	TOWNS_CD_STATUS_SUBQ_READ2		= 0x19,
	TOWNS_CD_STATUS_SUBQ_READ3		= 0x19,
	TOWNS_CD_STATUS_SUBQ_READ4		= 0x20,
	TOWNS_CD_STATUS_CMD_ABEND		= 0x21,
	TOWNS_CD_STATUS_DATA_PIO		= 0x21,
	TOWNS_CD_STATUS_DATA_DMA		= 0x22,
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
	enum {
		ERR_IO_NONE			=  0,
		ERR_IO_NOT_ATTACHED = -1,
		ERR_IO_CLOSED		= -2,
		ERR_IO_SEEK_ERROR	= -3,
		ERR_IO_READ_ERROR	= -4,
		ERR_BUFFER_FULL		= -16,
		ERR_ILLEGAL_PARAM	= -17,
	};
	DEVICE* d_cpu;
	DEVICE* d_dmac;

	outputs_t outputs_drq;
	outputs_t outputs_eot;
	outputs_t outputs_pic;

	FILEIO* fio_img;
	FIFO* status_queue;

	uint8_t *databuffer;  // With FIFO
	enum {
		SECTOR_EMPTY = UINT32_MAX,
	};

	uint32_t max_fifo_length;
	uint32_t max_fifo_mask;
	uint32_t max_fifo_multiply;

	uint32_t fifo_length;
	uint32_t fifo_mask;
	uint32_t fifo_multiply;

	size_t datacount;
	size_t readptr;
	size_t writeptr;

	uint16_t cpu_id;
	uint16_t machine_id;

	pair16_t data_reg;
	bool dma_transfer;
	bool pio_transfer;

	bool cdrom_halted;
	bool status_seek;

	uint8_t subq_bytes[12];
	uint8_t subq_snapshot[12];
	SUBC_t subq_buffer[98]; // OK?

	int subq_bitptr;
	int subq_bitwidth;
	bool subq_overrun;

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
	_TCHAR track_data_path[101][_MAX_PATH];
	_TCHAR img_file_path_bak[_MAX_PATH];
	bool with_filename[101];

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
	int sectors_count;

	int next_seek_lba;
	int read_mode;

	bool data_in;

	bool req_status;

	bool stat_reply_intr;
	bool dma_transfer_phase;
	bool pio_transfer_phase;
	bool mcu_ready;
	bool has_status;
	bool dmac_running;


	bool command_execute_phase;

	bool mcu_intr;
	bool dma_intr;
	bool mcu_intr_mask;
	bool dma_intr_mask;

	int event_execute;
	int event_drq;
	int event_seek;
	int event_next_sector;
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
	int offset_volume_l;
	int offset_volume_r;

	uint8_t w_regs[16];
	static const uint16_t crc_table[256];

	uint8_t reserved_command;
	RINGBUFFER* param_queue;
	uint8_t prev_command;
	uint8_t prev_params[8];

	uint8_t latest_command;
	uint8_t exec_params[8];

	bool command_received;

	double seek_time;
	int track_num;
	uint32_t max_logical_block;
	int bytes_per_sec;
	bool access;
	bool media_changed;
	bool media_ejected;
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

	void __FASTCALL clear_status_queue(const bool is_clear_extra);
	void __FASTCALL push_status_queue(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);

	virtual int __FASTCALL check_cdda_track_boundary(uint32_t frame_no);
	virtual bool seek_relative_frame_in_image(uint32_t frame_no);
    virtual int prefetch_audio_sectors(int sectors);
	virtual void read_cdrom();
	int read_sectors_image(int sectors, uint32_t& transferred_bytes);
	virtual int __FASTCALL read_sector_data(FILEIO* src, const size_t __logical_size, size_t _offset, size_t footer_size);

	virtual void execute_command(uint8_t command);

	bool __FASTCALL status_not_ready(const bool force_interrupt);
	bool __FASTCALL status_media_changed(const bool force_interrupt);
	bool __FASTCALL status_media_changed_or_not_ready(const bool force_interrupt);

	void __FASTCALL status_hardware_error(const bool force_interrupt);
	void __FASTCALL status_parameter_error(const bool force_interrupt);
	void __FASTCALL status_time_out(const bool force_interrupt);

	void __FASTCALL status_read_done(const bool force_interrupt);
	void __FASTCALL status_data_ready(const bool force_interrupt);

	void __FASTCALL status_accept(int extra, uint8_t s2, uint8_t s3, bool immediate_interrupt, const bool force_interrupt);
	void set_status(const bool push_status, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, const bool force_interrupt);
	void set_status_read_done(bool push_status, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
	void set_status_cddareply(const bool force_interrupt, int extra, uint8_t s2, uint8_t s3);
	void __FASTCALL set_status_immediate(const bool push_status, const bool force_interrupt, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);

	virtual void set_extra_status_values(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, const bool is_immediate, const bool force_interrupt);
	void  __FASTCALL set_status_extra_toc_addr(uint8_t s1, uint8_t s2, uint8_t s3);
	void  __FASTCALL set_status_extra_toc_data(uint8_t s1, uint8_t s2, uint8_t s3);

	virtual void __FASTCALL status_accept2(const bool force_interrupt, int extra, uint8_t s2, uint8_t s3);
	virtual void __FASTCALL status_accept3(int extra, uint8_t s2, uint8_t s3);

	void __FASTCALL status_not_accept(int extra, uint8_t s1, uint8_t s2, uint8_t s3, bool immediate_interrupt, const bool force_interrupt);

	void __FASTCALL status_illegal_lba(int extra, uint8_t s1, uint8_t s2, uint8_t s3);
	void set_delay_ready(const bool force_interrupt);
	void set_delay_ready_eot(const bool force_interrupt);

	uint32_t cdrom_get_adr(int trk);

	void __FASTCALL set_dma_intr(bool val);
	void __FASTCALL set_mcu_intr(bool val);
	/*!
	 * @brief set interrupts without stat_reply_intr.
	 * @param mcu interrupt value to set.
	 * @param dma interrupt value to set.
	 * @param use_mask true when masking mcu and dma.
	 */
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
	virtual void set_subq(uint32_t lba);
	virtual bool start_to_play_cdda();

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
	virtual void dma_transfer_epilogue();
	virtual void pio_transfer_epilogue();

	void start_time_out();
	void stop_time_out();
	virtual void start_drq();
	virtual void stop_drq();
	virtual bool check_dmac_running();
	void do_drq();

	inline void allocate_data_buffer(uint32_t multiply)
	{
		if(databuffer != NULL) {
			delete [] databuffer;
			databuffer = NULL;
		}
		if(multiply == 0) {
			return;
		}
		if(multiply > 20) { // MAX 1MBytes.
			multiply = 20;
		}
		uint32_t len = 1 << multiply;
		max_fifo_multiply = multiply;
		databuffer = new uint8_t[len];
		__UNLIKELY_IF(databuffer == NULL) {
			max_fifo_length = 0;
			max_fifo_mask = 0;
			max_fifo_multiply = 0;
		} else {
			max_fifo_length = len;
			max_fifo_mask = len - 1;
		}
	}
	inline size_t buffer_left()
	{
		__UNLIKELY_IF(datacount >= fifo_length) {
			return 0;
		}
		return fifo_length - datacount;
	}
	inline void set_fifo_length(uint8_t multiply)
	{
		__UNLIKELY_IF(databuffer == NULL) {
			fifo_length = 0;
			fifo_mask = 0;
			fifo_multiply = 0;
			return;
		}
		if(multiply > max_fifo_multiply) {
			multiply = max_fifo_multiply;
		}
		uint32_t len = 1 << multiply;
		if(len > 1) {
			fifo_length = len;
			fifo_mask = len - 1;
			fifo_multiply = multiply;
		} else {
			fifo_length = 1;
			fifo_mask = 0;
			fifo_multiply = 0;
		}
	}
	inline bool read_buffer(uint8_t& val)
	{
		__UNLIKELY_IF(databuffer == NULL) {
			val = 0x00;
			return false;
		}
		__UNLIKELY_IF(datacount == 0) {
			val = 0x00;
			return false;
		}
		val = databuffer[readptr];
		readptr = (readptr + 1) & fifo_mask;
		datacount--;
		return true;
	}
	inline size_t read_buffer(uint8_t* dst, size_t count)
	{
		__UNLIKELY_IF(dst == NULL) {
			return 0;
		}
		__UNLIKELY_IF(datacount < count) {
			count = datacount;
		}
		__UNLIKELY_IF(databuffer == NULL) {
			for(size_t i = 0; i < count; i++) {
				dst[i] = 0x00;
			}
			return count;
		}
		for(size_t i = 0; i < count; i++) {
			dst[i] = databuffer[readptr];
			readptr = (readptr + 1) & fifo_mask;
		}
		datacount -= count;
		return count;
	}

	inline bool read_buffer(pair16_t& val_l, pair16_t& val_r, bool is_swap)
	{
		val_l.w = 0x00;
		val_r.w = 0x00;
		__UNLIKELY_IF((databuffer == NULL) || (datacount == 0)) {
			datacount = 0;
			return false;
		}
		__UNLIKELY_IF(datacount < 4) { // Skip
			return false;
		}
		const uint32_t ptr0 = readptr & fifo_mask;
		const uint32_t ptr1 = (readptr + 1) & fifo_mask;
		const uint32_t ptr2 = (readptr + 2) & fifo_mask;
		const uint32_t ptr3 = (readptr + 3) & fifo_mask;

		__UNLIKELY_IF(is_swap) {
			val_l.b.h = databuffer[ptr0];
			val_l.b.l = databuffer[ptr1];
			val_r.b.h = databuffer[ptr2];
			val_r.b.l = databuffer[ptr3];
		} else {
			val_l.b.l = databuffer[ptr0];
			val_l.b.h = databuffer[ptr1];
			val_r.b.l = databuffer[ptr2];
			val_r.b.h = databuffer[ptr3];
		}
		readptr = (readptr + 4) & fifo_mask;
		datacount -= 4;
		return true;
	}
	void reset_buffer(const uint8_t val = 0x00)
	{
		datacount = 0;
		readptr = 0;
		writeptr = 0;
		__UNLIKELY_IF(databuffer == NULL) {
			return;
		}
		for(size_t i = 0; i < max_fifo_length; i++) {
			databuffer[i] = val;
		}
	}
	inline void __FASTCALL write_mcuint_signals(const bool val)
	{
		write_signals(&outputs_pic, (val) ? 0xffffffff : 0x00000000);
	}
	void cdrom_debug_log(const char *fmt, ...);
	virtual const _TCHAR* __FASTCALL get_cdda_status_name(int _status);
	virtual const _TCHAR* __FASTCALL get_command_name_from_command(uint8_t cmd);

	inline void fetch_datareg_8()
	{
		data_reg.b.h = data_reg.b.l;
		read_buffer(data_reg.b.l);
	}
	inline void fetch_datareg_16()
	{
		read_buffer(data_reg.b.l);
		read_buffer(data_reg.b.h);
	}
	int __FASTCALL calculate_volume(int volume_db, int minus_offset_db);

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
		param_queue = NULL;
		_decibel_l = 0;
		_decibel_r = 0;

		memset(subq_buffer, 0x00, sizeof(subq_buffer));

		initialize_output_signals(&outputs_drq);
		initialize_output_signals(&outputs_eot);
		initialize_output_signals(&outputs_pic);

		set_device_name(_T("FM-Towns CD-ROM drive"));
		d_dmac = NULL;
		// For Debugging, will remove 20200822 K.O
		d_cpu = NULL;
	}
	~TOWNS_CDROM() { }
	virtual void initialize() override;
	virtual void release() override;

	virtual void reset() override;
	virtual uint32_t __FASTCALL read_io8(uint32_t addr) override;
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_io16(uint32_t addr) override;
	/*
	virtual void __FASTCALL write_io16(uint32_t addr, uint32_t data) override;
	*/
	virtual uint32_t __FASTCALL read_dma_io8w(uint32_t addr, int *wait) override;
	virtual void __FASTCALL write_dma_io8w(uint32_t addr, uint32_t data, int *wait) override;

	virtual uint32_t __FASTCALL read_dma_io16w(uint32_t addr, int *wait) override;
	virtual void __FASTCALL write_dma_io16w(uint32_t addr, uint32_t data, int *wait) override;

	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	virtual uint32_t __FASTCALL read_signal(int id) override;

	virtual void __FASTCALL event_callback(int event_id, int err) override;
	virtual void __FASTCALL mix(int32_t* buffer, int cnt) override;

	virtual bool process_state(FILEIO* state_fio, bool loading) override;

	virtual bool mounted();
	virtual bool accessed();
	virtual void open(const _TCHAR* file_path);
	virtual void close();

	// for debug
	virtual void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_debug_data8(uint32_t addr) override;
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;
	virtual bool write_debug_reg(const _TCHAR *reg, uint32_t data) override;
	bool is_debugger_available() override
	{
		return true;
	}
	uint64_t get_debug_data_addr_space() override
	{
		return 0x2000; // Will change
	}
	virtual void set_volume(int ch, int decibel_l, int decibel_r) override;
	virtual void get_volume(int ch, int& decibel_l, int& decibel_r) override;
	virtual bool get_sectors(int sectors);

	// unique functions
	// Towns specified command
	virtual void set_cdda_status(uint8_t status);
	int get_track(uint32_t lba);
	virtual double get_seek_time(uint32_t lba);
	virtual uint8_t read_status();
	virtual const int logical_block_size();
	virtual const int physical_block_size();
	virtual const int real_physical_block_size();
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
		if(dev != NULL) {
			register_output_signal(&outputs_pic, dev, id, mask);
		}
	}
	void set_context_drq_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_drq, dev, id, mask);
	}
	void set_context_eot_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_eot, dev, id, mask);
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
