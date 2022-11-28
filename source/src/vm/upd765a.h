/*
	Skelton for retropc emulator

	Origin : M88
	Author : Takeda.Toshiya
	Date   : 2006.09.17-

	[ uPD765A ]
*/

#ifndef _UPD765A_H_
#define _UPD765A_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_UPD765A_RESET	0
#define SIG_UPD765A_TC		1
#define SIG_UPD765A_MOTOR	2
#define SIG_UPD765A_MOTOR_NEG	3
#define SIG_UPD765A_DRVSEL	4
#define SIG_UPD765A_IRQ_MASK	5
#define SIG_UPD765A_DRQ_MASK	6
#define SIG_UPD765A_FREADY	7

class DISK;
class NOISE;

class UPD765A : public DEVICE
{
private:
	// output signals
	outputs_t outputs_irq;
	outputs_t outputs_drq;
	outputs_t outputs_hdu;
	outputs_t outputs_index;
	
	// drive noise
	NOISE* d_noise_seek;
	NOISE* d_noise_head_down;
	NOISE* d_noise_head_up;
	
	// fdc
	struct {
		uint8_t track;
		uint8_t cur_track;
		uint8_t result;
		bool access;
		bool head_load;
		// timing
		int cur_position;
		int next_trans_position;
		uint32_t prev_clock;
	} fdc[4];
	DISK* disk[4];
	
	uint8_t hdu, hdue, id[4], eot, gpl, dtl;
	
	int phase, prevphase;
	uint8_t status, seekstat, command;
	uint32_t result;
	int step_rate_time;
	int head_unload_time;
	bool no_dma_mode, motor_on;
#ifdef UPD765A_DMA_MODE
	bool dma_data_lost;
#endif
	bool irq_masked, drq_masked;
	
	uint8_t* bufptr;
	uint8_t buffer[0x8000];
	int count;
	int event_phase;
	int phase_id, drq_id, lost_id, result7_id, seek_step_id[4], seek_end_id[4], head_unload_id[4];
	bool force_ready;
	bool reset_signal;
	bool prev_index;
	
	// timing
	uint32_t prev_drq_clock;
	
	int get_cur_position(int drv);
	double get_usec_to_exec_phase();
	
	// update status
	void set_irq(bool val);
	void set_drq(bool val);
	void set_hdu(uint8_t val);
	
	// phase shift
	void shift_to_idle();
	void shift_to_cmd(int length);
	void shift_to_exec();
	void shift_to_read(int length);
	void shift_to_write(int length);
	void shift_to_scan(int length);
	void shift_to_result(int length);
	void shift_to_result7();
	void shift_to_result7_event();
	void start_transfer();
	void finish_transfer();
	
	// command
	void process_cmd(int cmd);
	void cmd_sence_devstat();
	void cmd_sence_intstat();
	uint8_t get_devstat(int drv);
	void cmd_seek();
	void cmd_recalib();
	void seek(int drv, int trk);
	void seek_event(int drv);
	void cmd_read_data();
	void cmd_write_data();
	void cmd_scan();
	void cmd_read_diagnostic();
	void read_data(bool deleted, bool scan);
	void write_data(bool deleted);
	void read_diagnostic();
	uint32_t read_sector();
	uint32_t write_sector(bool deleted);
	uint32_t find_id();
	uint32_t check_cond(bool write);
	void get_sector_params();
	bool id_incr();
	void cmd_read_id();
	void cmd_write_id();
	uint32_t read_id();
	uint32_t write_id();
	void cmd_specify();
	void cmd_invalid();
	void update_head_flag(int drv, bool head_load);
	
public:
	UPD765A(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_irq);
		initialize_output_signals(&outputs_drq);
		initialize_output_signals(&outputs_hdu);
		initialize_output_signals(&outputs_index);
		d_noise_seek = NULL;
		d_noise_head_down = NULL;
		d_noise_head_up = NULL;
		force_ready = false;
		raise_irq_when_media_changed = false;
		set_device_name(_T("uPD765A FDC"));
	}
	~UPD765A() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch);
	void event_callback(int event_id, int err);
	void update_config();
#ifdef USE_DEBUGGER
	bool is_debugger_available()
	{
		return true;
	}
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_drq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_drq, device, id, mask);
	}
	void set_context_hdu(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_hdu, device, id, mask);
	}
	void set_context_index(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_index, device, id, mask);
	}
	void set_context_noise_seek(NOISE* device)
	{
		d_noise_seek = device;
	}
	NOISE* get_context_noise_seek()
	{
		return d_noise_seek;
	}
	void set_context_noise_head_down(NOISE* device)
	{
		d_noise_head_down = device;
	}
	NOISE* get_context_noise_head_down()
	{
		return d_noise_head_down;
	}
	void set_context_noise_head_up(NOISE* device)
	{
		d_noise_head_up = device;
	}
	NOISE* get_context_noise_head_up()
	{
		return d_noise_head_up;
	}
	DISK* get_disk_handler(int drv)
	{
		if(drv < 4) {
			return disk[drv];
		}
		return NULL;
	}
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool is_disk_inserted(int drv);
	bool is_disk_inserted();	// current hdu
	bool disk_ejected(int drv);
	bool disk_ejected();	// current hdu
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);
	uint8_t get_media_type(int drv);
	void set_drive_type(int drv, uint8_t type);
	uint8_t get_drive_type(int drv);
	void set_drive_rpm(int drv, int rpm);
	void set_drive_mfm(int drv, bool mfm);
	bool raise_irq_when_media_changed;
};

#endif

