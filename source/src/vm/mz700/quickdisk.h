/*
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2011.02.17-

	[ quick disk ]
*/

#ifndef _QUICKDISK_H_
#define _QUICKDISK_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define QUICKDISK_SIO_RTSA	0
#define QUICKDISK_SIO_DTRB	1
#define QUICKDISK_SIO_SYNC	2
#define QUICKDISK_SIO_RXDONE	3
#define QUICKDISK_SIO_DATA	4
#define QUICKDISK_SIO_BREAK	5

#define QUICKDISK_BUFFER_SIZE	65536

class NOISE;

namespace MZ700 {

class QUICKDISK : public DEVICE
{
private:
	DEVICE *d_sio;
	NOISE* d_noise_seek;
	
	_TCHAR file_path[_MAX_PATH];
	bool insert, protect, home;
	bool modified;
	bool accessed;
	
	uint16_t buffer[QUICKDISK_BUFFER_SIZE];
	int buffer_ptr, write_ptr;
	bool first_data;
	bool send_break;
	
	bool wrga, mton, sync;
	bool motor_on;
	int restore_id, end_id;
	
	void restore();
	void send_data();
	void write_crc();
	void end_of_disk();
	void set_insert(bool val);
	void set_protect(bool val);
	void set_home(bool val);
	void release_disk();
	unsigned short calc_crc(int* buff, int size);
	
public:
	QUICKDISK(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Quick Disk"));
	}
	~QUICKDISK() {}
	
	// common functions
	void initialize() override;
	void release() override;
	void reset() override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	uint32_t __FASTCALL read_signal(int ch) override;
	void __FASTCALL event_callback(int event_id, int err) override;
	void update_config() override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique functions
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
	void set_context_noise_seek(NOISE* device)
	{
		d_noise_seek = device;
	}
	NOISE* get_context_noise_seek()
	{
		return d_noise_seek;
	}
	void open_disk(const _TCHAR* path);
	void close_disk();
	bool is_disk_inserted()
	{
		return insert;
	}
};

}
#endif

