/*
	Skelton for retropc emulator

	Origin : XM7
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ MB8877 / MB8876 / MB8866 / MB89311 ]
*/

#ifndef _MB8877_H_ 
#define _MB8877_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_MB8877_ACCESS	0
#define SIG_MB8877_DRIVEREG	1
#define SIG_MB8877_SIDEREG	2
#define SIG_MB8877_MOTOR	3

class DISK;
class NOISE;

class  DLL_PREFIX MB8877 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_irq;
	outputs_t outputs_drq;
	outputs_t outputs_rdy;

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
		bool count_immediate; // Hack for FLEX.
	} fdc[16];
	DISK* disk[16];
	
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
	int register_id[10];
	
	void cancel_my_event(int event);
	void register_my_event(int event, double usec);
	bool register_my_event_with_check(int event, double usec);
	void register_seek_event(bool first);
	void register_drq_event(int bytes);
	void register_lost_event(int bytes);
	
	bool check_drive(void);
	bool check_drive2(void);
	
	// status
	bool now_search;
	bool now_seek;
	bool sector_changed;
	int no_command;
	int seektrk;
	bool seekvct;
	bool motor_on;
	bool drive_sel;
	
//#ifdef HAS_MB89311
	// MB89311
	bool extended_mode;
//#endif
	
	// timing
	uint32_t prev_drq_clock;
	uint32_t seekend_clock;

	// flags
	bool fdc_debug_log;
	bool invert_registers;
	bool type_fm77av_2dd;
	bool type_mb8866;
	bool type_mb89311;
	bool type_x1;
	bool type_fm7;
	bool type_fmr50;
	bool type_fmr60;
	bool mb8877_no_busy_after_seek;
	int  mb8877_delay_after_seek;
	int _max_drive;
	int _drive_mask;
	
	int __FASTCALL get_cur_position();
	double __FASTCALL  get_usec_to_start_trans(bool first_sector);
	double __FASTCALL get_usec_to_next_trans_pos(bool delay);
	double __FASTCALL get_usec_to_detect_index_hole(int count, bool delay);
	
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
//#ifdef HAS_MB89311
	void cmd_format();
//#endif
	void cmd_forceint();
	void update_head_flag(int drv, bool head_load);
	virtual void update_ready();
	
	// irq/dma
	void __FASTCALL set_irq(bool val);
	void __FASTCALL set_drq(bool val);

public:
	MB8877(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_irq);
		initialize_output_signals(&outputs_drq);
		initialize_output_signals(&outputs_rdy);
		d_noise_seek = NULL;
		d_noise_head_down = NULL;
		d_noise_head_up = NULL;
		// these parameters may be modified before calling initialize()
		drvreg = sidereg = 0;
		motor_on = drive_sel = false;
		//
		mb8877_delay_after_seek = 0;
		fdc_debug_log = invert_registers = type_fm77av_2dd = false;
		type_mb8866 = type_mb89311 = false;
		type_x1 = type_fm7 = type_fmr50 = type_fmr60 = false;
		mb8877_no_busy_after_seek = false;
		_max_drive = 4;
		_drive_mask = _max_drive - 1;
		for(int i = 0; i < 16; i++) {
			disk[i] = NULL;
		}
//#if defined(HAS_MB89311)
//		set_device_name(_T("MB89311 FDC"));
//#elif defined(HAS_MB8866)
//		set_device_name(_T("MB8866 FDC"));
//#elif defined(HAS_MB8876)
//		set_device_name(_T("MB8876 FDC"));
//#else
//		set_device_name(_T("MB8877 FDC"));
//#endif
	}
	~MB8877() {}
	
	// common functions
	void initialize() override;
	void release() override;
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_dma_io8(uint32_t addr) override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	uint32_t __FASTCALL read_signal(int ch) override;
	void __FASTCALL event_callback(int event_id, int err) override;
	void update_config() override;
	bool is_debugger_available() override
	{
		return true;
	}
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_drq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_drq, device, id, mask);
	}
	void set_context_rdy(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_rdy, device, id, mask);
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
	bool is_disk_changed(int drv);
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);
	bool is_drive_ready();
	bool is_drive_ready(int drv);
	uint8_t get_media_type(int drv);
	void set_drive_type(int drv, uint8_t type);
	uint8_t get_drive_type(int drv);
	void set_drive_rpm(int drv, int rpm);
	void set_drive_mfm(int drv, bool mfm);
	void set_track_size(int drv, int size);
	uint8_t fdc_status();
};

#endif
