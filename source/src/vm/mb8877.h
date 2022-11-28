/*
	Skelton for retropc emulator

	Origin : XM7
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ MB8877 / MB8876 / MB8866 / MB89311 ]
*/

#ifndef _MB8877_H_ 
#define _MB8877_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_MB8877_ACCESS	0
#define SIG_MB8877_DRIVEREG	1
#define SIG_MB8877_SIDEREG	2
#define SIG_MB8877_MOTOR	3

class DISK;
class NOISE;

class MB8877 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_irq;
	outputs_t outputs_drq;
	
	// drive noise
	NOISE* d_noise_seek;
	NOISE* d_noise_head_down;
	NOISE* d_noise_head_up;
	
	// drive info
	struct {
		int track;
		int index;
		bool access;
		bool head_load;
		// write track
		bool id_written;
		bool sector_found;
		int sector_length;
		int sector_index;
		int side;
		bool side_changed;
		// timing
		int cur_position;
		int next_trans_position;
		int bytes_before_2nd_drq;
		int next_am1_position;
		uint32_t prev_clock;
	} fdc[MAX_DRIVE];
	DISK* disk[MAX_DRIVE];
	
	// registor
	uint8_t status, status_tmp;
	uint8_t cmdreg, cmdreg_tmp;
	uint8_t trkreg;
	uint8_t secreg;
	uint8_t datareg;
	uint8_t drvreg;
	uint8_t sidereg;
	uint8_t cmdtype;
	
	// event
	int register_id[8];
	
	void cancel_my_event(int event);
	void register_my_event(int event, double usec);
	void register_seek_event(bool first);
	void register_drq_event(int bytes);
	void register_lost_event(int bytes);
	
	// status
	bool now_search;
	bool now_seek;
	bool sector_changed;
	int no_command;
	int seektrk;
	bool seekvct;
	bool motor_on;
	bool drive_sel;
	
#ifdef HAS_MB89311
	// MB89311
	bool extended_mode;
#endif
	
	// timing
	uint32_t prev_drq_clock;
	uint32_t seekend_clock;
	
	int get_cur_position();
	double get_usec_to_start_trans(bool first_sector);
	double get_usec_to_next_trans_pos(bool delay);
	double get_usec_to_detect_index_hole(int count, bool delay);
	
	// image handler
	uint8_t search_track();
	uint8_t search_sector();
	uint8_t search_addr();
	
	// command
	void process_cmd();
	void cmd_restore();
	void cmd_seek();
	void cmd_step();
	void cmd_stepin();
	void cmd_stepout();
	void cmd_readdata(bool first_sector);
	void cmd_writedata(bool first_sector);
	void cmd_readaddr();
	void cmd_readtrack();
	void cmd_writetrack();
#ifdef HAS_MB89311
	void cmd_format();
#endif
	void cmd_forceint();
	void update_head_flag(int drv, bool head_load);
	
	// irq/dma
	void set_irq(bool val);
	void set_drq(bool val);
	
public:
	MB8877(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_irq);
		initialize_output_signals(&outputs_drq);
		d_noise_seek = NULL;
		d_noise_head_down = NULL;
		d_noise_head_up = NULL;
		// these parameters may be modified before calling initialize()
		drvreg = sidereg = 0;
		motor_on = drive_sel = false;
#if defined(HAS_MB89311)
		set_device_name(_T("MB89311 FDC"));
#elif defined(HAS_MB8866)
		set_device_name(_T("MB8866 FDC"));
#elif defined(HAS_MB8876)
		set_device_name(_T("MB8876 FDC"));
#else
		set_device_name(_T("MB8877 FDC"));
#endif
	}
	~MB8877() {}
	
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
	
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_drq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_drq, device, id, mask);
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
		return disk[drv];
	}
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool is_disk_inserted(int drv);
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);
	uint8_t get_media_type(int drv);
	void set_drive_type(int drv, uint8_t type);
	uint8_t get_drive_type(int drv);
	void set_drive_rpm(int drv, int rpm);
	void set_drive_mfm(int drv, bool mfm);
	void set_track_size(int drv, int size);
	uint8_t fdc_status();
};

#endif
