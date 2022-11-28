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
//#define DATA_BLOCK_ID	1
#define DATA_BLOCK_ID	5

#define BSD_HEADER_BLOCK_ID	2
#define BSD_DATA_BLOCK_ID	3
#define BSD_DATA_BLOCK_ID_LAST	7
#define BSD_DATA_BLOCK_SIZE	1026
#define BSD_ATTR_ID		3

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

void QUICKDISK::write_signal(int id, uint32_t data, uint32_t mask)
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

uint32_t QUICKDISK::read_signal(int ch)
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

unsigned short QUICKDISK::calc_crc(int* buff, int size)
{
	unsigned short crc = 0;
	const unsigned short crc16poly = 0xa001;
	// calculate crc-16/ARC
	int i, j;
	for (i = 0; i < size; i++) {
		crc = crc ^ (unsigned short)buff[i];
		for (j = 0; j < 8; j++) {
			if (crc & (unsigned short)0x1) {
				crc = (crc >> 1) ^ crc16poly;
			}
			else {
				crc = (crc >> 1);
			}
		}
	}
	return crc;
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

void QUICKDISK::open_disk(const _TCHAR* path)
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
		my_tcscpy_s(file_path, _MAX_PATH, path);
		
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
				uint8_t header[MZT_HEADER_SIZE], ram[0x20000];
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
						uint16_t patch_addr = header[patch_ofs] | (header[patch_ofs + 1] << 8);
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
				
				//buffer[buffer_ptr++] = HEADER_BLOCK_ID;
				if (header[0] != BSD_ATTR_ID) {
					buffer[buffer_ptr++] = HEADER_BLOCK_ID;
				} else {
					buffer[buffer_ptr++] = BSD_HEADER_BLOCK_ID;
				}
				
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
				
				if (header[0] != BSD_ATTR_ID) {
					// copy data
					buffer[block_num_ptr] = ++num_block;
					
					buffer[buffer_ptr++] = DATA_SYNC;
					buffer[buffer_ptr++] = DATA_SYNC;
					buffer[buffer_ptr++] = DATA_MARK;
					buffer[buffer_ptr++] = DATA_BLOCK_ID;
					buffer[buffer_ptr++] = (uint8_t)(size & 0xff);
					buffer[buffer_ptr++] = (uint8_t)(size >> 8);
					for(int i = 0; i < size; i++) {
						buffer[buffer_ptr++] = ram[offs + i];
					}
					buffer[buffer_ptr++] = DATA_CRC;
					buffer[buffer_ptr++] = DATA_CRC;
					buffer[buffer_ptr++] = DATA_SYNC;
					buffer[buffer_ptr++] = DATA_SYNC;
					buffer[buffer_ptr++] = DATA_BREAK;
				} else {
					int outsize = 0;
					int blkflg_pos = 0;
					do {
						buffer[block_num_ptr] = ++num_block;
						
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_MARK;
						blkflg_pos = buffer_ptr;
						buffer[buffer_ptr++] = BSD_DATA_BLOCK_ID;
						buffer[buffer_ptr++] = (uint8_t)(BSD_DATA_BLOCK_SIZE & 0xff);
						buffer[buffer_ptr++] = (uint8_t)(BSD_DATA_BLOCK_SIZE >> 8);
						for(int i = 0; i < BSD_DATA_BLOCK_SIZE; i++) {
							buffer[buffer_ptr++] = ram[offs + i];
						}
						buffer[buffer_ptr++] = DATA_CRC;
						buffer[buffer_ptr++] = DATA_CRC;
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_BREAK;
						outsize += BSD_DATA_BLOCK_SIZE;
						offs += BSD_DATA_BLOCK_SIZE;
					} while(outsize < size);
					buffer[blkflg_pos] = BSD_DATA_BLOCK_ID_LAST;
				}
			}
		} else { // qdf file
			// check header
			uint8_t header[16];
			fio->Fread(header, sizeof(header), 1);
			if(memcmp(header, "-QD format-", 11) != 0) {
				fio->Fseek(0, FILEIO_SEEK_SET);
			}
			
			int data, i, temp_data_size=0;
			bool block_file = true;
			buffer[buffer_ptr++] = DATA_BREAK;
			
			// skip 2700 bytes from file top (GAP or gabage)
			fio->Fseek(2700, FILEIO_SEEK_SET);
			while ((data = fio->Fgetc()) != EOF) {
				//skip block top BiSync
				do {
					data = fio->Fgetc();
				} while (data == DATA_SYNC);
				if (data == 0) continue; // garbage BiSync data
				// Check Data_Mark
				if (data != 0xa5) {	// Abnormal block Data Mark
					int sync_top_ptr = 0, sync_num = 0, sync_num_prev = 0;
					int temp_block_buff[65535];
					int temp_block_ptr = 0;
					unsigned short cal_crc = 0, temp_crc = 0;
					const unsigned short crc16poly = 0xa001;
					
					memset(temp_block_buff, 0, sizeof(temp_block_buff));
					temp_block_buff[temp_block_ptr++] = data;
					
					while ((data = fio->Fgetc()) != EOF) {
						if(data == DATA_SYNC) {
							if(sync_num == 0) {
								sync_top_ptr = temp_block_ptr;
							}
							sync_num++;
						} else {
							sync_num_prev = sync_num;
							sync_num = 0;
						}
						// TODO: This code will misread 0x00 last over 4 bytes BiSync data as Block end.
						if (sync_num_prev >= 4 && sync_num == 0 && data == 0x00) { 
							temp_data_size = sync_top_ptr;
							temp_block_ptr = sync_top_ptr;
							temp_crc = (unsigned short)temp_block_buff[temp_block_ptr - 2] + (unsigned short)(temp_block_buff[temp_block_ptr - 1] << 8);
							cal_crc = calc_crc(temp_block_buff, temp_data_size - 2);
							
							// CRC unmatch (lack 1 byte Bisync error), add one more 0x16 data
							if (cal_crc != temp_crc) {
								temp_block_buff[temp_block_ptr++] = 0x16;
								temp_data_size++;
							}
							// put data to main buffer
							buffer[buffer_ptr++] = DATA_SYNC;
							buffer[buffer_ptr++] = DATA_SYNC;
							for (i = 0; i < temp_data_size; i++) {
								buffer[buffer_ptr++] = temp_block_buff[i];
							}
							buffer[buffer_ptr++] = DATA_SYNC;
							buffer[buffer_ptr++] = DATA_SYNC;
							buffer[buffer_ptr++] = DATA_BREAK;
							break;
						} else {
							temp_block_buff[temp_block_ptr++] = data;
							if (temp_block_ptr > (QUICKDISK_BUFFER_SIZE - 2)) break;
						}
					}
				} else { // normal data_mark 0xa5
					if (block_file == true) {
						// BLOCK-FILE area has 4 bytes data
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_SYNC;
						for (i = 0; i < 4; i++) {
							buffer[buffer_ptr++] = data;
							data = fio->Fgetc();
						}
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_BREAK;
						block_file = false;
					} else {
						// Other areas have data size.
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = data;	// DATA MARK
						data = fio->Fgetc();
						buffer[buffer_ptr++] = data;	// BLOCK FLAG
						data = fio->Fgetc();
						buffer[buffer_ptr++] = data;	// DATA SIZE(L)
						temp_data_size = data;
						data = fio->Fgetc();
						buffer[buffer_ptr++] = data;	// DATA SIZE(H)
						temp_data_size |= (data << 8);
						for (i = 0; i < temp_data_size; i++) {
							data = fio->Fgetc();
							buffer[buffer_ptr++] = data;	// data
						}
						data = fio->Fgetc();
						buffer[buffer_ptr++] = data;	// CRC(L)
						data = fio->Fgetc();
						buffer[buffer_ptr++] = data;	// CRC(H)
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_SYNC;
						buffer[buffer_ptr++] = DATA_BREAK;
					}
					// skip block end BiSync
					do {
						data = fio->Fgetc();
					} while (data == DATA_SYNC);
					buffer[buffer_ptr] = DATA_EMPTY;
				}
			}
		}
		set_insert(true);
		set_protect(FILEIO::IsFileProtected(path));
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
		if(check_file_extension(file_path, _T(".mzt")) || check_file_extension(file_path, _T(".q20")) || check_file_extension(file_path, _T(".qdf"))) {
			my_tcscpy_s(file_path_tmp, _MAX_PATH, file_path);
		} else {
			my_stprintf_s(file_path_tmp, _MAX_PATH, _T("%s.mzt"), get_file_path_without_extensiton(file_path));
		}
		
		// save blocks as mzt file
		FILEIO* fio = new FILEIO();
		if(!fio->Fopen(file_path_tmp, FILEIO_WRITE_BINARY)) {
			my_tcscpy_s(file_path_tmp, _MAX_PATH, "temporary_saved_quick_disk.mzt");
			fio->Fopen(create_local_path(file_path_tmp), FILEIO_WRITE_BINARY);
		}
		if(fio->IsOpened()) {
			if (!check_file_extension(file_path_tmp, _T(".qdf"))) {
				int block_num = buffer[4];
				buffer_ptr = 10;
				uint8_t header[MZT_HEADER_SIZE];
				
				bool bLastBSD = false;
				static uint8_t BSDBuffer[0x10000];
				int BSDTotal = 0;
				
				for(int i = 0; i < block_num; i++) {
					if(buffer[buffer_ptr] == DATA_EMPTY) {
						break;
					}
					//int bsd_id = buffer[buffer_ptr + 3] & 3;
					int bsd_id = buffer[buffer_ptr + 3] & 7;
					int id = buffer[buffer_ptr + 3] & 1;
					int size = buffer[buffer_ptr + 4] | (buffer[buffer_ptr + 5] << 8);
					
					buffer_ptr += 6;
					
					if(id == HEADER_BLOCK_ID) {
						if (bLastBSD) {
							header[0x13] = (uint8_t)(BSDTotal >> 8);	// file size
							header[0x12] = (uint8_t)BSDTotal;
							fio->Fwrite(header, MZT_HEADER_SIZE, 1);
							for(int i = 0; i < BSDTotal; i++) {
								fio->Fputc(BSDBuffer[i]);
							}
							bLastBSD = false;
						}
						
						// create mzt header
						memset(header, 0, sizeof(header));
						
						header[0x00] = (uint8_t)buffer[buffer_ptr + 0];	// attribute
						for(int i = 1; i <= 17; i++) {
							header[i] = (uint8_t)buffer[buffer_ptr + i];	// file name
						}
						header[0x3e] = (uint8_t)buffer[buffer_ptr + 18];	// lock
						header[0x3f] = (uint8_t)buffer[buffer_ptr + 19];	// lock
						header[0x12] = (uint8_t)buffer[buffer_ptr + 20];	// file size
						header[0x13] = (uint8_t)buffer[buffer_ptr + 21];
						header[0x14] = (uint8_t)buffer[buffer_ptr + 22];	// load addr
						header[0x15] = (uint8_t)buffer[buffer_ptr + 23];
						header[0x16] = (uint8_t)buffer[buffer_ptr + 24];	// exec addr
						header[0x17] = (uint8_t)buffer[buffer_ptr + 25];
						
						//fio->Fwrite(header, MZT_HEADER_SIZE, 1);
						if (bsd_id != BSD_HEADER_BLOCK_ID) {
							fio->Fwrite(header, MZT_HEADER_SIZE, 1);
						} else {
							BSDTotal = 0;
							//bLastBSD = true;
						}
					} else {
						// data
						//for(int i = 0; i < size; i++) {
						//	fio->Fputc(buffer[buffer_ptr + i]);
						//}
						//if (bsd_id == BSD_DATA_BLOCK_ID) {
						if ((bsd_id == BSD_DATA_BLOCK_ID) || (bsd_id == BSD_DATA_BLOCK_ID_LAST)) {
							for(int i = 0; i < size; i++) {
								BSDBuffer[BSDTotal + i] = (uint8_t)buffer[buffer_ptr + i];
							}
							BSDTotal += size;
							if (bsd_id == BSD_DATA_BLOCK_ID_LAST) {
								bLastBSD = true;
							}
						} else {
							for(int i = 0; i < size; i++) {
								fio->Fputc(buffer[buffer_ptr + i]);
							}
						}
					}
					buffer_ptr += size + 5;
				}
				
				if (bLastBSD) {
					header[0x13] = (uint8_t)(BSDTotal >> 8);	// file size
					header[0x12] = (uint8_t)BSDTotal;
					fio->Fwrite(header, MZT_HEADER_SIZE, 1);
					for(int i = 0; i < BSDTotal; i++) {
						fio->Fputc(BSDBuffer[i]);
					}
					bLastBSD = false;
				}
			}
			else // save as qdf file
			// TODO: This code will create over size qdf files because QD buffer can store many blocks.
			{
				buffer_ptr = 0;
				int i, file_size = 0;
				bool block_file = true;
				int block_data_size = 0;
				unsigned short cal_crc =0;
				const int max_file_size = 0x14010;
				int temp_block_buff[65535];
				int temp_block_ptr = 0;
				
				// add header 
				fio->Fwrite("-QD format-", 11, 1);
				fio->Fputc(0xff);
				fio->Fputc(0xff);
				fio->Fputc(0xff);
				fio->Fputc(0xff);
				fio->Fputc(0xff);
				file_size += 16;
				
				// add media top gap (2032 bytes)+long send break gap(2793) before block file area 
				for (i = 0; i < (2032 + 2793); i++) {
					fio->Fputc(0x00);
					file_size++;
				}
				
				while (buffer[++buffer_ptr] != DATA_EMPTY || file_size > max_file_size) { 
					memset(temp_block_buff, 0, sizeof(temp_block_buff));
					temp_block_ptr = 0;
					block_data_size = 0;
					while ( (buffer[buffer_ptr] != DATA_BREAK) && (buffer_ptr < QUICKDISK_BUFFER_SIZE) ) {
						temp_block_buff[temp_block_ptr++] = buffer[buffer_ptr++];
						block_data_size++;
					}
					if (buffer_ptr >= QUICKDISK_BUFFER_SIZE) break; // end of buffer, garbage block 
					if ( ((temp_block_buff[0] != DATA_SYNC) && (temp_block_buff[1] != DATA_SYNC)) || (block_data_size < 5) ) continue;
					
					// cal crc
					cal_crc = calc_crc((temp_block_buff+2), (block_data_size - 6));
					temp_block_buff[block_data_size - 4] = ((int)cal_crc & 0xff);
					temp_block_buff[block_data_size - 3] = (int)(cal_crc >> 8);
					
					// write buff to file
					temp_block_ptr = 0;
					// add start bisync total 10bytes
					for (i = 0; i < 8; i++) { // 2 bytes bisync data is in temp buff.
						fio->Fputc(DATA_SYNC);
						file_size++;
					}
					// while (temp_block_ptr < data_size || file_size > max_file_size) {
					while (temp_block_ptr < block_data_size ) {
						fio->Fputc(temp_block_buff[temp_block_ptr++]);
						file_size++;
					}
					// add end bisync total 6bytes
					for (i = 0; i < 4; i++) { // 2 bytes bisync data is in temp buff.
						fio->Fputc(DATA_SYNC);
						file_size++;
					}
					// add send break gap for block end
					int gap_size = 0;
					if (block_file) {
						gap_size = 2793;
						block_file = false;
					}
					else {
						gap_size = 254;
					}
					for (i = 0; i < gap_size; i++) {
						fio->Fputc(0x00);
						file_size++;
					}
				}
				// add gap to file end
				while (file_size < max_file_size) {
					fio->Fputc(0x00);
					file_size++;
				}
			}
			fio->Fclose();
		}
		delete fio;
	}
}

#define STATE_VERSION	1

bool QUICKDISK::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(file_path, sizeof(file_path), 1);
	state_fio->StateValue(insert);
	state_fio->StateValue(protect);
	state_fio->StateValue(home);
	state_fio->StateValue(modified);
	state_fio->StateValue(accessed);
	state_fio->StateArray(buffer, sizeof(buffer), 1);
	state_fio->StateValue(buffer_ptr);
	state_fio->StateValue(write_ptr);
	state_fio->StateValue(first_data);
	state_fio->StateValue(send_break);
	state_fio->StateValue(wrga);
	state_fio->StateValue(mton);
	state_fio->StateValue(sync);
	state_fio->StateValue(motor_on);
	state_fio->StateValue(restore_id);
	state_fio->StateValue(end_id);
	return true;
}

