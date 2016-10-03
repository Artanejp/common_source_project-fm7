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

#define SIG_MB8877_DRIVEREG		0
#define SIG_MB8877_SIDEREG		1
#define SIG_MB8877_MOTOR		2

class DISK;
#if defined(USE_SOUND_FILES)
class WAV_SOUNDER;
#endif
class MB8877 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_irq;
	outputs_t outputs_drq;
	
	// drive info
	struct {
		int track;
		int index;
		bool access;
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
#if defined(USE_SOUND_FILES)
	DEVICE *d_seek_sound;
#endif
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
	void register_seek_event();
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
	
	// irq/dma
	void set_irq(bool val);
	void set_drq(bool val);
	
public:
	MB8877(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_irq);
		initialize_output_signals(&outputs_drq);
		motor_on = false;
#if defined(USE_SOUND_FILES)
		d_seek_sound = NULL;
#endif
		set_device_name(_T("MB8877 FDC"));
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
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_drq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_drq, device, id, mask);
	}
#if defined(USE_SOUND_FILES)
	void set_context_seek(DEVICE* device)
	{
		d_seek_sound = device;
	}
#endif
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
	uint8_t fdc_status();
};

#endif
