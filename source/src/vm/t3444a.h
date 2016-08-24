/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.09.03-

	[ T3444A / T3444M ]
*/

#ifndef _T3444A_H_ 
#define _T3444A_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_T3444A_DRIVE	0
#define SIG_T3444A_TND		1
#define SIG_T3444A_MOTOR	2

// for reading signal
#define SIG_T3444A_DRDY		4
#define SIG_T3444A_CRDY		5
#define SIG_T3444A_RQM		6

#ifdef HAS_T3444M
#define SECTORS_IN_TRACK	16
#else
#define SECTORS_IN_TRACK	26
#endif

class DISK;

class T3444A : public DEVICE
{
private:
	// output signals
	outputs_t outputs_rqm;
	
	// drive info
	struct {
		int track;
		int index;
		bool access;
		// timing
		int cur_position;
		int next_trans_position;
		int bytes_before_2nd_rqm;
		int next_sync_position;
		uint32_t prev_clock;
	} fdc[4];
	DISK* disk[4];
	
	// register
	uint8_t status;
	uint8_t cmdreg;
	uint8_t trkreg;
	uint8_t secreg;
	uint8_t datareg;
	uint8_t drvreg;
	uint8_t sidereg;
	bool timerflag;
	uint8_t sector_id[SECTORS_IN_TRACK * 4];
	
	// event
	int register_id[5];
	
	void cancel_my_event(int event);
	void register_my_event(int event, double usec);
	void register_seek_event();
	void register_rqm_event(int bytes);
	void register_lost_event(int bytes);
	
	// status
	bool now_search;
	int seektrk;
	bool rqm;
	bool tnd;
	bool motor_on;
	
	// timing
	uint32_t prev_rqm_clock;
	
	int get_cur_position();
	double get_usec_to_start_trans();
	double get_usec_to_next_trans_pos();
	double get_usec_to_detect_index_hole(int count);
	
	// image handler
	uint8_t search_sector();
	
	// command
	void process_cmd();
	void cmd_seek_zero();
	void cmd_seek();
	void cmd_read_write();
	void cmd_write_id();
	void cmd_sence();
	
	// rqm
	void set_rqm(bool val);
	
public:
	T3444A(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_rqm);
		tnd = true;
		motor_on = false;
		set_device_name(_T("T3444A"));
	}
	~T3444A() {}
	
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
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("T3444A");
	}
	
	// unique functions
	void set_context_rqm(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_rqm, device, id, mask);
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
	void set_drive_type(int drv, uint8_t type);
	uint8_t get_drive_type(int drv);
	void set_drive_rpm(int drv, int rpm);
	void set_drive_mfm(int drv, bool mfm);
};

#endif
