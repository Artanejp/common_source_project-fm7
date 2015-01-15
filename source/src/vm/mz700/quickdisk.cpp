/*
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2011.02.17-

	[ quick disk ]
*/

#include "quickdisk.h"
#include "../z80sio.h"
#include "../../fileio.h"

#define MZT_HEADER_SIZE	128
#define HEADER_SIZE	64

#define EVENT_RESTORE	0
#define EVENT_END	1

// 100usec
#define PERIOD_RESTORE	100
// 1sec
#define PERIOD_END	1000000

#define DATA_SYNC	0x16
#define DATA_MARK	0xa5
#define DATA_CRC	0xff
#define DATA_BREAK	0x100
#define DATA_EMPTY	0x101

#define HEADER_BLOCK_ID	0
#define DATA_BLOCK_ID	1

void QUICKDISK::initialize()
{
	insert = protect = false;
	home = true;
	first_data = send_break = true;
}

void QUICKDISK::release()
{
	release_disk();
}

void QUICKDISK::reset()
{
	wrga = mton = true;
	sync = false;
	motor_on = false;
	accessed = false;
	restore_id = end_id = -1;
	
	set_insert(insert);
	set_protect(protect);
	set_home(true);
}

/*
	PROTECT -> CTSA
		H: write protected
	INSERT -> DCDA
		L: inserted
	HOME -> DCDB
		L: reach to head position
		H: reset, reach to end of disk, DTRB is L->H

	RTSA -> WRGA
		L: write disk / stop motor at the end of disk
		H: read disk
	DTRB -> MTON
		H->L: start motor
		H: stop motor at the end of disk
*/

#define REGISTER_RESTORE_EVENT() { \
	if(restore_id == -1) { \
		register_event(this, EVENT_RESTORE, PERIOD_RESTORE, false, &restore_id); \
	} \
}

#define CANCEL_RESTORE_EVENT() { \
	if(restore_id != -1) { \
		cancel_event(this, restore_id); \
		restore_id = -1; \
	} \
}

#define REGISTER_END_EVENT() { \
	if(end_id != -1) { \
		cancel_event(this, end_id); \
	} \
	register_event(this, EVENT_END, PERIOD_END, false, &end_id); \
}

#define CANCEL_END_EVENT() { \
	if(end_id != -1) { \
		cancel_event(this, end_id); \
		end_id = -1; \
	} \
}

#define WRITE_BUFFER(v) { \
	if(buffer_ptr < QUICKDISK_BUFFER_SIZE) { \
		if(buffer[buffer_ptr] != v) { \
			buffer[buffer_ptr] = v; \
			modified = true; \
		} \
		buffer_ptr++; \
	} \
}

void QUICKDISK::write_signal(int id, uint32 data, uint32 mask)
{
	bool next = ((data & mask) != 0);
	
	if(id == QUICKDISK_SIO_RTSA) {
		if(wrga && !next) {
			// start to write
			first_data = true;
			write_ptr = 0;
		} else if(!wrga && next) {
			// end to write
			write_crc();
		}
		wrga = next;
	} else if(id == QUICKDISK_SIO_DTRB) {
		if(mton && !next) {
			// H->L: start motor
			if(motor_on && wrga) {
				// restart to send
				send_data();
				REGISTER_END_EVENT();
			} else {
				// start motor and restore to home position
				motor_on = true;
				REGISTER_RESTORE_EVENT();
				CANCEL_END_EVENT();
			}
		} else if(!mton && next) {
			// L->H: home signal is high
			set_home(true);
		}
		mton = next;
	} else if(id == QUICKDISK_SIO_SYNC) {
		// enter hunt/sync phase
		sync = next;
		if(sync) {
			// hack: start to send for verify
			if(!wrga) {
				write_crc();
				wrga = true;
			}
			send_data();
		}
	} else if(id == QUICKDISK_SIO_RXDONE) {
		// send next data
		send_data();
	} else if(id == QUICKDISK_SIO_DATA || id == QUICKDISK_SIO_BREAK) {
		// write data
		if(!(motor_on && !wrga)) {
			return;
		}
		if(id == QUICKDISK_SIO_DATA) {
			if(first_data) {
				// write sync chars at the top of message
				WRITE_BUFFER(DATA_SYNC);
				WRITE_BUFFER(DATA_SYNC);
				first_data = false;
			}
			WRITE_BUFFER(data);
			write_ptr = buffer_ptr;
		} else if(id == QUICKDISK_SIO_BREAK) {
			write_crc();
			WRITE_BUFFER(DATA_BREAK);
			first_data = true;
			write_ptr = 0;
		}
		accessed = true;
		
		if(buffer_ptr < QUICKDISK_BUFFER_SIZE) {
			REGISTER_END_EVENT();
		} else {
			CANCEL_END_EVENT();
			end_of_disk();
		}
	}
}

uint32 QUICKDISK::read_signal(int ch)
{
	// access lamp signal
	if(accessed) {
		accessed = false;
		return 1;
	}
	return 0;
}

void QUICKDISK::event_callback(int event_id, int err)
{
	if(event_id == EVENT_RESTORE) {
		// reached to home position
		restore_id = -1;
		restore();
	} else if(event_id == EVENT_END) {
		// reached to end of disk
		end_id = -1;
		end_of_disk();
	}
}

void QUICKDISK::restore()
{
	// reached to home position
	set_home(false);
	buffer_ptr = 0;
	first_data = send_break = true;
	
	// start to send
	send_data();
}

void QUICKDISK::send_data()
{
	if(!(motor_on && wrga) || restore_id != -1) {
		return;
	}
retry:
	if(buffer_ptr < QUICKDISK_BUFFER_SIZE && buffer[buffer_ptr] != DATA_EMPTY) {
		if(buffer[buffer_ptr] == DATA_BREAK) {
			// send break signal
			if(send_break) {
				d_sio->write_signal(SIG_Z80SIO_BREAK_CH0, 1, 1);
				send_break = false;
			}
			// wait until sio enters hunt/sync phase
			if(!sync) {
				return;
			}
			buffer_ptr++;
			goto retry;
		}
		// send data
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, buffer[buffer_ptr++], 0xff);
		send_break = true;
		accessed = true;
		REGISTER_END_EVENT();
	} else {
		// reached to end of disk
		CANCEL_END_EVENT();
		end_of_disk();
	}
}

void QUICKDISK::write_crc()
{
	if(!wrga && write_ptr != 0) {
		buffer_ptr = write_ptr;
		
		WRITE_BUFFER(DATA_CRC);
		WRITE_BUFFER(DATA_CRC);
		WRITE_BUFFER(DATA_SYNC);
		WRITE_BUFFER(DATA_SYNC);
		// don't increment pointer !!!
		WRITE_BUFFER(DATA_BREAK);
		buffer_ptr--;
	}
	write_ptr = 0;
}

void QUICKDISK::end_of_disk()
{
	// write crc
	write_crc();
	
	// reached to end of disk
	if(mton || !wrga) {
		motor_on = false;
	} else {
		REGISTER_RESTORE_EVENT();
	}
	set_home(true);
}

void QUICKDISK::set_insert(bool val)
{
	// L=inserted
	d_sio->write_signal(SIG_Z80SIO_DCD_CH0, val ? 0 : 1, 1);
	insert = val;
}

void QUICKDISK::set_protect(bool val)
{
	// H=protected
	d_sio->write_signal(SIG_Z80SIO_CTS_CH0, val ? 1 : 0, 1);
	protect = val;
}

void QUICKDISK::set_home(bool val)
{
	if(home != val) {
		d_sio->write_signal(SIG_Z80SIO_DCD_CH1, val ? 1 : 0, 1);
		home = val;
	}
}

void QUICKDISK::open_disk(_TCHAR path[])
{
	// check current disk image
	if(insert) {
		if(_tcsicmp(file_path, path) == 0) {
			return;
		}
		// close current disk
		close_disk();
	}
	memset(buffer, 0, sizeof(buffer));
	
	// load disk image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(path, FILEIO_READ_BINARY)) {
		_tcscpy_s(file_path, _MAX_PATH, path);
		
		// clear buffer
		for(int i = 0; i < QUICKDISK_BUFFER_SIZE; i++) {
			buffer[i] = DATA_EMPTY;
		}
		buffer_ptr = 0;
		modified = false;
		
		// check extension
		if(check_file_extension(file_path, _T(".mzt")) || check_file_extension(file_path, _T(".q20"))) {
			// load mzt file
			fio->Fseek(0, FILEIO_SEEK_END);
			int remain = fio->Ftell();
			fio->Fseek(0, FILEIO_SEEK_SET);
			
			int num_block = 0;
			int block_num_ptr = 0;
			
			// create block file
			buffer[buffer_ptr++] = DATA_BREAK;
			buffer[buffer_ptr++] = DATA_SYNC;
			buffer[buffer_ptr++] = DATA_SYNC;
			buffer[buffer_ptr++] = DATA_MARK;
			block_num_ptr = buffer_ptr;
			buffer[buffer_ptr++] = 0; // block number
			buffer[buffer_ptr++] = DATA_CRC;
			buffer[buffer_ptr++] = DATA_CRC;
			buffer[buffer_ptr++] = DATA_SYNC;
			buffer[buffer_ptr++] = DATA_SYNC;
			buffer[buffer_ptr++] = DATA_BREAK;
			
			while(remain >= MZT_HEADER_SIZE) {
				// load header
				uint8 header[MZT_HEADER_SIZE], ram[0x20000];
				fio->Fread(header, MZT_HEADER_SIZE, 1);
				remain -= MZT_HEADER_SIZE;
				
				// load data
				int size = header[0x12] | (header[0x13] << 8);
				int offs = header[0x14] | (header[0x15] << 8);
				memset(ram, 0, sizeof(ram));
				fio->Fread(ram + offs, size, 1);
				remain -= size;
#if 0
				// apply mz700win patch
				if(header[0x40] == 'P' && header[0x41] == 'A' && header[0x42] == 'T' && header[0x43] == ':') {
					int patch_ofs = 0x44;
					for(; patch_ofs < 0x80; ) {
						uint16 patch_addr = header[patch_ofs] | (header[patch_ofs + 1] << 8);
						patch_ofs += 2;
						if(patch_addr == 0xffff) {
							break;
						}
						int patch_len = header[patch_ofs++];
						for(int i = 0; i < patch_len; i++) {
							ram[patch_addr + i] = header[patch_ofs++];
						}
					}
					// clear patch data
					for(int i = 0x40; i < patch_ofs; i++) {
						header[i] = 0;
					}
				}
#endif
				// copy header
				buffer[block_num_ptr] = ++num_block;
				
				buffer[buffer_ptr++] = DATA_SYNC;
				buffer[buffer_ptr++] = DATA_SYNC;
				buffer[buffer_ptr++] = DATA_MARK;
				buffer[buffer_ptr++] = HEADER_BLOCK_ID;
				buffer[buffer_ptr++] = HEADER_SIZE;
				buffer[buffer_ptr++] = 0;
				buffer[buffer_ptr++] = header[0];	// attribute
				for(int i = 0; i < 17; i++) {
					buffer[buffer_ptr++] = header[i + 1]; // file name
				}
				buffer[buffer_ptr++] = header[0x3e];	// lock
				buffer[buffer_ptr++] = header[0x3f];	// secret
				buffer[buffer_ptr++] = header[0x12];	// file size
				buffer[buffer_ptr++] = header[0x13];
				buffer[buffer_ptr++] = header[0x14];	// load addr
				buffer[buffer_ptr++] = header[0x15];
				buffer[buffer_ptr++] = header[0x16];	// exec addr
				buffer[buffer_ptr++] = header[0x17];
				for(int i = 26; i < HEADER_SIZE; i++) {
					buffer[buffer_ptr++] = 0;	// comment
				}
				buffer[buffer_ptr++] = DATA_CRC;
				buffer[buffer_ptr++] = DATA_CRC;
				buffer[buffer_ptr++] = DATA_SYNC;
				buffer[buffer_ptr++] = DATA_SYNC;
				buffer[buffer_ptr++] = DATA_BREAK;
				
				// copy data
				buffer[block_num_ptr] = ++num_block;
				
				buffer[buffer_ptr++] = DATA_SYNC;
				buffer[buffer_ptr++] = DATA_SYNC;
				buffer[buffer_ptr++] = DATA_MARK;
				buffer[buffer_ptr++] = DATA_BLOCK_ID;
				buffer[buffer_ptr++] = (uint8)(size & 0xff);
				buffer[buffer_ptr++] = (uint8)(size >> 8);
				for(int i = 0; i < size; i++) {
					buffer[buffer_ptr++] = ram[offs + i];
				}
				buffer[buffer_ptr++] = DATA_CRC;
				buffer[buffer_ptr++] = DATA_CRC;
				buffer[buffer_ptr++] = DATA_SYNC;
				buffer[buffer_ptr++] = DATA_SYNC;
				buffer[buffer_ptr++] = DATA_BREAK;
			}
		} else {
			// check header
			uint8 header[16];
			fio->Fread(header, sizeof(header), 1);
			if(memcmp(header, "-QD format-", 11) != 0) {
				fio->Fseek(0, FILEIO_SEEK_SET);
			}
			
			// load raw file
			bool in_gap = true;
			int sync_top_ptr = 0, sync_num = 0, sync_num_prev = 0, data;
			
			buffer[buffer_ptr++] = DATA_BREAK;
			
			while((data = fio->Fgetc()) != EOF) {
				if(data == DATA_SYNC) {
					if(sync_num == 0) {
						sync_top_ptr = buffer_ptr;
					}
					sync_num++;
				} else {
					sync_num_prev = sync_num;
					sync_num = 0;
				}
				if(in_gap) {
					if(sync_num_prev >= 4 && sync_num == 0) {
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = data;
						in_gap = false;
					}
				} else {
					if(sync_num_prev >= 4 && sync_num == 0 && data == 0x00) {
						buffer_ptr = sync_top_ptr;
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_BREAK;
						in_gap = true;
					} else {
						buffer[buffer_ptr++] = data;
					}
				}
			}
		}
		set_insert(true);
		set_protect(fio->IsProtected(path));
		set_home(true);
		
		fio->Fclose();
	}
	delete fio;
}

void QUICKDISK::close_disk()
{
	release_disk();
	set_insert(false);
	set_protect(false);
	set_home(true);
	
	// cancel all events
	CANCEL_RESTORE_EVENT();
	CANCEL_END_EVENT();
}

void QUICKDISK::release_disk()
{
	if(insert && !protect && modified) {
		// check extension
		_TCHAR file_path_tmp[_MAX_PATH];
		if(check_file_extension(file_path, _T(".mzt")) || check_file_extension(file_path, _T(".q20"))) {
			_tcscpy_s(file_path_tmp, _MAX_PATH, file_path);
		} else {
			_stprintf_s(file_path_tmp, _MAX_PATH, _T("%s.mzt"), get_file_path_without_extensiton(file_path));
		}
		// save blocks as mzt file
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(file_path_tmp, FILEIO_WRITE_BINARY)) {
			int block_num = buffer[4];
			buffer_ptr = 10;
			
			for(int i = 0; i < block_num; i++) {
				if(buffer[buffer_ptr] == DATA_EMPTY) {
					break;
				}
				int id = buffer[buffer_ptr + 3] & 3;
				int size = buffer[buffer_ptr + 4] | (buffer[buffer_ptr + 5] << 8);
				buffer_ptr += 6;
				
				if(id == HEADER_BLOCK_ID) {
					// create mzt header
					uint8 header[MZT_HEADER_SIZE];
					memset(header, 0, sizeof(header));
					
					header[0x00] = (uint8)buffer[buffer_ptr + 0];	// attribute
					for(int i = 1; i <= 17; i++) {
						header[i] = (uint8)buffer[buffer_ptr + i];	// file name
					}
					header[0x3e] = (uint8)buffer[buffer_ptr + 18];	// lock
					header[0x3f] = (uint8)buffer[buffer_ptr + 19];	// lock
					header[0x12] = (uint8)buffer[buffer_ptr + 20];	// file size
					header[0x13] = (uint8)buffer[buffer_ptr + 21];
					header[0x14] = (uint8)buffer[buffer_ptr + 22];	// load addr
					header[0x15] = (uint8)buffer[buffer_ptr + 23];
					header[0x16] = (uint8)buffer[buffer_ptr + 24];	// exec addr
					header[0x17] = (uint8)buffer[buffer_ptr + 25];
					fio->Fwrite(header, MZT_HEADER_SIZE, 1);
				} else {
					// data
					for(int i = 0; i < size; i++) {
						fio->Fputc(buffer[buffer_ptr + i]);
					}
				}
				buffer_ptr += size + 5;
			}
			fio->Fclose();
		}
		delete fio;
	}
}

#define STATE_VERSION	1

void QUICKDISK::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(file_path, sizeof(file_path), 1);
	state_fio->FputBool(insert);
	state_fio->FputBool(protect);
	state_fio->FputBool(home);
	state_fio->FputBool(modified);
	state_fio->FputBool(accessed);
	state_fio->Fwrite(buffer, sizeof(buffer), 1);
	state_fio->FputInt32(buffer_ptr);
	state_fio->FputInt32(write_ptr);
	state_fio->FputBool(first_data);
	state_fio->FputBool(send_break);
	state_fio->FputBool(wrga);
	state_fio->FputBool(mton);
	state_fio->FputBool(sync);
	state_fio->FputBool(motor_on);
	state_fio->FputInt32(restore_id);
	state_fio->FputInt32(end_id);
}

bool QUICKDISK::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(file_path, sizeof(file_path), 1);
	insert = state_fio->FgetBool();
	protect = state_fio->FgetBool();
	home = state_fio->FgetBool();
	modified = state_fio->FgetBool();
	accessed = state_fio->FgetBool();
	state_fio->Fread(buffer, sizeof(buffer), 1);
	buffer_ptr = state_fio->FgetInt32();
	write_ptr = state_fio->FgetInt32();
	first_data = state_fio->FgetBool();
	send_break = state_fio->FgetBool();
	wrga = state_fio->FgetBool();
	mton = state_fio->FgetBool();
	sync = state_fio->FgetBool();
	motor_on = state_fio->FgetBool();
	restore_id = state_fio->FgetInt32();
	end_id = state_fio->FgetInt32();
	return true;
}

