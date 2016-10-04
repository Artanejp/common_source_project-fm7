//
// PC-6001/6601 disk I/O
// This file is based on a disk I/O program in C++
// by Mr. Yumitaro and translated into C for Cocoa iP6
// by Koichi NISHIDA 2006
//

/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Origin : tanam
	Date   : 2014.05.21-

	[ PC-6031 ]
*/

#include "pc6031.h"
#include "disk.h"
#if defined(USE_SOUND_FILES)
#define EVENT_SEEK_SOUND 2
#endif

void PC6031::event_callback(int event_id, int err)
{
#if defined(USE_SOUND_FILES)
	if((event_id >= EVENT_SEEK_SOUND) && (event_id < (EVENT_SEEK_SOUND + 2))) {
		int drvno = event_id - EVENT_SEEK_SOUND;
		if((mdisk.trk * 2) < seek_track_num[drvno]) {
			seek_track_num[drvno] -= 2;
			register_event(this, EVENT_SEEK_SOUND + drvno, 16000, false, &seek_event_id[drvno]);
		} else if((mdisk.trk * 2)> seek_track_num[drvno]) {
			seek_track_num[drvno] += 2;
			register_event(this, EVENT_SEEK_SOUND + drvno, 16000, false, &seek_event_id[drvno]);
		} else {
			seek_event_id[drvno] = -1;
		}
		add_sound(PC6031_SND_TYPE_SEEK);
	}
#endif
}

int PC6031::Seek88(int drvno, int trackno, int sectno)
{
	if(drvno < 2) {
#if defined(USE_SOUND_FILES)
		if(cur_trk[drvno] != trackno) {
			seek_track_num[drvno] = (cur_trk[drvno] & 0xfe);
			if(seek_event_id[drvno] >= 0) {
				cancel_event(this, seek_event_id[drvno]);
			}
			register_event(this, EVENT_SEEK_SOUND + drvno, 16000, false, &seek_event_id[drvno]);
		}
#endif
		cur_trk[drvno] = trackno;
		cur_sct[drvno] = sectno;
		cur_pos[drvno] = 0;
		if(disk[drvno]->get_track(trackno >> 1, trackno & 1)) {
			for(int i = 0; i < disk[drvno]->sector_num.sd; i++) {
				if(disk[drvno]->get_sector(trackno >> 1, 0/*trackno & 1*/, i)) {
					if(disk[drvno]->id[2] == sectno) {
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

unsigned char PC6031::Getc88(int drvno)
{
	if(drvno < 2 && disk[drvno]->sector != NULL) {
		if(cur_pos[drvno] >= disk[drvno]->sector_size.sd) {
			cur_sct[drvno]++;
			if(!Seek88(drvno, cur_trk[drvno], cur_sct[drvno])) {
//				cur_trk[drvno]++;
				cur_trk[drvno] += 2;
				cur_sct[drvno] = 1;
				if(!Seek88(drvno, cur_trk[drvno], cur_sct[drvno])) {
					return 0xff;
				}
			}
		}
		access[drvno] = true;
		return disk[drvno]->sector[cur_pos[drvno]++];
	}
	return 0xff;
}

int PC6031::Putc88(int drvno, unsigned char dat)
{
	if(drvno < 2 && disk[drvno]->sector != NULL) {
		if(cur_pos[drvno] >= disk[drvno]->sector_size.sd) {
			cur_sct[drvno]++;
			if(!Seek88(drvno, cur_trk[drvno], cur_sct[drvno])) {
//				cur_trk[drvno]++;
				cur_trk[drvno] += 2;
				cur_sct[drvno] = 1;
				if(!Seek88(drvno, cur_trk[drvno], cur_sct[drvno])) {
					return 0xff;
				}
			}
		}
		access[drvno] = true;
		disk[drvno]->sector[cur_pos[drvno]++] = dat;
		return 1;
	}
	return 0;
}

// command
enum FddCommand
{
	INIT				= 0x00,
	WRITE_DATA			= 0x01,
	READ_DATA			= 0x02,
	SEND_DATA			= 0x03,
	COPY				= 0x04,
	FORMAT				= 0x05,
	SEND_RESULT_STATUS	= 0x06,
	SEND_DRIVE_STATUS	= 0x07,
	TRANSMIT			= 0x11,
	RECEIVE				= 0x12,
	LOAD				= 0x14,
	SAVE				= 0x15,
	WAIT				= 0xff,	// waiting state
	EndofFdcCmd
};

// data input (port D0H)
unsigned char PC6031::FddIn60()
{
	unsigned char ret;

	if (mdisk.DAV) {		// if data is valid
		if (mdisk.step == 6) {
			mdisk.retdat = Getc88(mdisk.drv);
			if(--mdisk.size == 0) mdisk.step = 0;
		}
		mdisk.DAC = 1;
		ret = mdisk.retdat;
	} else {			// if data is not valid
		ret = 0xff;
	}
	return ret;
}

// data/command output (port D1H)
void PC6031::FddOut60(unsigned char dat)
{
	if (mdisk.command == WAIT) {	// when command
		mdisk.command = dat;
		switch (mdisk.command) {
		case INIT:					// 00h init
			break;
		case WRITE_DATA:			// 01h write data
			mdisk.step = 1;
			break;
		case READ_DATA:				// 02h read data
			mdisk.step = 1;
			break;
		case SEND_DATA:				// 03h send data
			mdisk.step = 6;
			break;
		case COPY:					// 04h copy
			break;
		case FORMAT:				// 05h format
			break;
		case SEND_RESULT_STATUS:	// 06h send result status
			mdisk.retdat = 0x40;
			break;
		case SEND_DRIVE_STATUS:		// 07h send drive status
			mdisk.retdat |= 0x0a;
			break;
		case TRANSMIT:				// 11h transnmit
			break;
		case RECEIVE:				// 12h receive
			break;
		case LOAD:					// 14h load
			break;
		case SAVE:					// 15h save
			break;
		}
	} else {					// when data
		switch (mdisk.command) {
		case WRITE_DATA:			// 01h write data
			switch (mdisk.step) {
			case 1:	// 01h:block number
				mdisk.blk = dat;
				mdisk.size = mdisk.blk*256;
				mdisk.step++;
				break;
			case 2:	// 02h:drive number - 1
				mdisk.drv = dat;
				mdisk.step++;
				break;
			case 3:	// 03h:track number
				mdisk.trk = dat;
				mdisk.step++;
				break;
			case 4:	// 04h:sector number
				mdisk.sct = dat;
				// double track number(1D->2D)
				Seek88(mdisk.drv, mdisk.trk*2, mdisk.sct);
				mdisk.step++;
				break;
			case 5:	// 05h:write data
				Putc88(mdisk.drv, dat);
				if( --mdisk.size == 0 ){
					mdisk.step = 0;
				}
				break;
			}
			break;
		case READ_DATA:				// 02h read data
			switch (mdisk.step) {
			case 1:	// 01h:block number
				mdisk.blk = dat;
				mdisk.size = mdisk.blk*256;
				mdisk.step++;
				break;
			case 2:	// 02h:drive number-1
				mdisk.drv = dat;
				mdisk.step++;
				break;
			case 3:	// 03h:track number
				mdisk.trk = dat;
				mdisk.step++;
				break;
			case 4:	// 04h:sector number
				mdisk.sct = dat;
				// double track number(1D->2D)
				Seek88(mdisk.drv, mdisk.trk*2, mdisk.sct);
				mdisk.step = 0;
				break;
			}
		}
	}
}

// control input from disk unit (port D2H)
unsigned char PC6031::FddCntIn60(void)
{
	if (((old_D2H & 0x01) ^ mdisk.DAV) || mdisk.RFD && mdisk.DAV) {
		mdisk.DAC = mdisk.DAV;
	} else if (mdisk.ATN) {
		mdisk.RFD = 1;
		mdisk.command = WAIT;
	} else if (mdisk.DAC) {
		mdisk.DAV = 0;
	} else if (mdisk.RFD) {
		mdisk.DAV = 1;
	}	
	old_D2H = io_D2H;
	io_D2H = 0xf0 | 0x08 /* (mdisk.ATN<<3) */ | (mdisk.DAC<<2) | (mdisk.RFD<<1) | mdisk.DAV;
	return (io_D2H);
}

// control output to disk unit (port D3H)
void PC6031::FddCntOut60(unsigned char dat)
{
	// 8255 basic behavior
	if (!(dat&0x80)) {		// check msb
							// ignore when 1
		switch ((dat>>1)&0x07) {
		case 7:	// bit7 ATN
			mdisk.ATN = dat&1;
			break;
		case 6:	// bit6 DAC
			mdisk.DAC = dat&1;
			break;
		case 5:	// bit5 RFD
			mdisk.RFD = dat&1;
			break;
		case 4:	// bit4 DAV
			mdisk.DAV = dat&1;
			break;
		}
		io_D2H = 0xf0 | 0x08 /* (mdisk.ATN<<3) */ | (mdisk.DAC<<2) | (mdisk.RFD<<1) | mdisk.DAV;
	}
}

// I/O access functions
void PC6031::OutD1H_60(unsigned char data) { io_D1H = data; FddOut60(io_D1H); }
void PC6031::OutD2H_60(unsigned char data) {
	mdisk.ATN = (data & 0x80) >> 7;
	mdisk.DAC = (data & 0x40) >> 6;
	mdisk.RFD = (data & 0x20) >> 5;
	mdisk.DAV = (data & 0x10) >> 4;
	io_D2H = 0xf0 | 0x08 /* (mdisk.ATN<<3) */ | (mdisk.DAC<<2) | (mdisk.RFD<<1) | mdisk.DAV;
}
void PC6031::OutD3H_60(unsigned char data) { io_D3H = data; FddCntOut60(io_D3H); }

unsigned char PC6031::InD0H_60() { return FddIn60(); }
unsigned char PC6031::InD1H_60() { return io_D1H; }
unsigned char PC6031::InD2H_60() { io_D2H = FddCntIn60(); return io_D2H; }
unsigned char PC6031::InD3H_60() { return io_D3H; }

void PC6031::initialize()
{
	for(int i = 0; i < 2; i++) {
		disk[i] = new DISK(emu);
	}
	DrvNum = 1;
	memset(&mdisk, 0, sizeof(DISK60));
	mdisk.command = WAIT;		// received command
	mdisk.retdat  = 0xff;		// data from port D0H
	io_D1H = 0;
	io_D2H = 0xf0 | 0x08 /* (mdisk.ATN<<3) */ | (mdisk.DAC<<2) | (mdisk.RFD<<1) | mdisk.DAV;
	io_D3H = 0;
	old_D2H = 0;
}

void PC6031::release()
{
	for(int i = 0; i < 2; i++) {
		if(disk[i]) {
			disk[i]->close();
			delete disk[i];
		}
	}
}

void PC6031::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 3) {
	case 1:
		OutD1H_60(data);
		break;
	case 2:
		OutD2H_60(data);
		break;
	case 3:
		OutD3H_60(data);
		break;
	}
}

uint32_t PC6031::read_io8(uint32_t addr)
{
	switch(addr & 3) {
	case 0:
		return InD0H_60(); break;
	case 1:
		return InD1H_60(); break;
	case 2:
		return InD2H_60(); break;
	case 3:
		return InD3H_60(); break;
	}
	return 0xff;
}

uint32_t PC6031::read_signal(int ch)
{
	// get access status
	uint32_t stat = 0;
	for(int drv = 0; drv < 2; drv++) {
		if(access[drv]) {
			stat |= 1 << drv;
		}
		access[drv] = false;
	}
	return stat;
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void PC6031::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < 2) {
		disk[drv]->open(file_path, bank);
		Seek88(drv, 0, 1);
	}
}

void PC6031::close_disk(int drv)
{
	if(drv < 2 && disk[drv]->inserted) {
		disk[drv]->close();
	}
}

bool PC6031::is_disk_inserted(int drv)
{
	if(drv < 2) {
		return disk[drv]->inserted;
	}
	return false;
}

bool PC6031::disk_ejected(int drv)
{
	if(drv < 2) {
		return disk[drv]->ejected;
	}
	return false;
}

void PC6031::is_disk_protected(int drv, bool value)
{
	if(drv < 2) {
		disk[drv]->write_protected = value;
	}
}

bool PC6031::is_disk_protected(int drv)
{
	if(drv < 2) {
		return disk[drv]->write_protected;
	}
	return false;
}

#if defined(USE_SOUND_FILES)
void PC6031::add_sound(int type)
{
	int *p;
	if(type == PC6031_SND_TYPE_SEEK) {
		p = snd_seek_mix_tbl;
	} else if(type == PC6031_SND_TYPE_HEAD) {
		p = snd_head_mix_tbl;
	} else {
		return;
	}
	for(int i = 0; i < PC6031_SND_TBL_MAX; i++) {
		if(p[i] < 0) {
			p[i] = 0;
			break;
		}
	}
}

bool PC6031::load_sound_data(int type, const _TCHAR *pathname)
{
	if((type < 0) || (type > 1)) return false;
	int16_t *data = NULL;
	int dst_size = 0;
	int id = (this_device_id << 8) + type;
	const _TCHAR *sp;
	sp = create_local_path(pathname);
	emu->load_sound_file(id, sp, &data, &dst_size);
	if((dst_size <= 0) || (data == NULL)) { // Failed
		this->out_debug_log("ID=%d : Failed to load SOUND FILE for %s:%s", id, (type == 0) ? _T("SEEK") : _T("HEAD") ,pathname);
		return false;
	} else {
		int utl_size = dst_size * 2 * sizeof(int16_t);
		int alloc_size = utl_size + 64;
		switch(type) {
		case PC6031_SND_TYPE_SEEK: // SEEK
			snd_seek_data = (int16_t *)malloc(alloc_size);
			memcpy(snd_seek_data, data, utl_size);
			strncpy(snd_seek_name, pathname, 511);
			snd_seek_samples_size = dst_size;
			break;
		case PC6031_SND_TYPE_HEAD: // HEAD
			snd_seek_data = (int16_t *)malloc(alloc_size);
			memcpy(snd_head_data, data, utl_size);
			strncpy(snd_head_name, pathname, 511);
			snd_head_samples_size = dst_size;
			break;
		default:
			this->out_debug_log("ID=%d : Illegal type (%d): 0 (SEEK SOUND) or 1 (HEAD SOUND) is available.",
								id, type);
			return false;
		}
		this->out_debug_log("ID=%d : Success to load SOUND FILE for %s:%s",
							id, (type == 0) ? _T("SEEK") : _T("HEAD") ,
							pathname);
	}
	return true;
}

void PC6031::release_sound_data(int type)
{
	switch(type) {
	case PC6031_SND_TYPE_SEEK: // SEEK
		if(snd_seek_data != NULL) free(snd_seek_data);
		memset(snd_seek_name, 0x00, sizeof(snd_seek_name));
		snd_seek_data = NULL;
		break;
	case PC6031_SND_TYPE_HEAD: // HEAD
		if(snd_head_data != NULL) free(snd_head_data);
		memset(snd_head_name, 0x00, sizeof(snd_head_name));
		snd_head_data = NULL;
			break;
	default:
		break;
	}
}

bool PC6031::reload_sound_data(int type)
{
	switch(type) {
	case PC6031_SND_TYPE_SEEK: // SEEK
		if(snd_seek_data != NULL) free(snd_seek_data);
		break;
	case PC6031_SND_TYPE_HEAD:
		if(snd_seek_data != NULL) free(snd_seek_data);
		break;
	default:
		return false;
		break;
	}
	_TCHAR *p = (type == PC6031_SND_TYPE_SEEK) ? snd_seek_name : snd_head_name;
    _TCHAR tmps[512];
	strncpy(tmps, p, 511);
	return load_sound_data(type, tmps);
}

void PC6031::mix_main(int32_t *dst, int count, int16_t *src, int *table, int samples)
{
	int ptr, pp;
	int i, j, k;
	int32_t data[2];
	int32_t *dst_tmp;
	for(i=0; i < PC6031_SND_TBL_MAX; i++) {
		ptr = table[i];
		if(ptr >= 0) {
			if(ptr < samples) {
				if(!snd_mute) {
					pp = ptr << 1;
					dst_tmp = dst;
					k = 0;
					for(j = 0; j < count; j++) {
						if(ptr >= samples) {
							break;
						}
						data[0] = (int32_t)src[pp + 0];
						data[1] = (int32_t)src[pp + 1];
						dst_tmp[k + 0] += apply_volume((int32_t)data[0], snd_level_l);
						dst_tmp[k + 1] += apply_volume((int32_t)data[1], snd_level_r);
						k += 2;
						pp += 2;
						ptr++;
					}
				} else {
					ptr += count;
				}
			}
			if(ptr >= samples) {
				table[i] = -1;
			} else {
				table[i] = ptr;
			}
		}
	}
}

void PC6031::mix(int32_t *buffer, int cnt)
{
	if(snd_seek_data != NULL) mix_main(buffer, cnt, snd_seek_data, snd_seek_mix_tbl, snd_seek_samples_size);
	if(snd_head_data != NULL) mix_main(buffer, cnt, snd_head_data, snd_head_mix_tbl, snd_head_samples_size);
}

void PC6031::set_volume(int ch, int decibel_l, int decibel_r)
{
	snd_level_l = decibel_to_volume(decibel_l);
	snd_level_r = decibel_to_volume(decibel_r);
}
#endif

#define STATE_VERSION	2

void PC6031::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	for(int i = 0; i < 2; i++) {
		disk[i]->save_state(state_fio);
	}
	state_fio->Fwrite(cur_trk, sizeof(cur_trk), 1);
	state_fio->Fwrite(cur_sct, sizeof(cur_sct), 1);
	state_fio->Fwrite(cur_pos, sizeof(cur_pos), 1);
	state_fio->Fwrite(access, sizeof(access), 1);
	state_fio->Fwrite(&mdisk, sizeof(DISK60), 1);
	state_fio->FputUint8(io_D1H);
	state_fio->FputUint8(io_D2H);
	state_fio->FputUint8(old_D2H);
	state_fio->FputUint8(io_D3H);
	state_fio->FputInt32(DrvNum);
#if defined(USE_SOUND_FILES)
	for(int i = 0; i < 2; i++) {
		state_fio->FputInt32(seek_event_id[i]);
		state_fio->FputInt32(seek_track_num[i]);
	}
	state_fio->Fwrite(snd_seek_name, sizeof(snd_seek_name), 1);
	state_fio->Fwrite(snd_head_name, sizeof(snd_head_name), 1);
	for(int i = 0; i < PC6031_SND_TBL_MAX; i++) {
		state_fio->FputInt32(snd_seek_mix_tbl[i]);
	}
	for(int i = 0; i < PC6031_SND_TBL_MAX; i++) {
		state_fio->FputInt32(snd_head_mix_tbl[i]);
	}
	state_fio->FputBool(snd_mute);
	state_fio->FputInt32(snd_level_l);
	state_fio->FputInt32(snd_level_r);
#endif
}

bool PC6031::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	for(int i = 0; i < 2; i++) {
		if(!disk[i]->load_state(state_fio)) {
			return false;
		}
	}
	state_fio->Fread(cur_trk, sizeof(cur_trk), 1);
	state_fio->Fread(cur_sct, sizeof(cur_sct), 1);
	state_fio->Fread(cur_pos, sizeof(cur_pos), 1);
	state_fio->Fread(access, sizeof(access), 1);
	state_fio->Fread(&mdisk, sizeof(DISK60), 1);
	io_D1H = state_fio->FgetUint8();
	io_D2H = state_fio->FgetUint8();
	old_D2H = state_fio->FgetUint8();
	io_D3H = state_fio->FgetUint8();
	DrvNum = state_fio->FgetInt32();
#if defined(USE_SOUND_FILES)
	for(int i = 0; i < 2; i++) {
		seek_event_id[i] = state_fio->FgetInt32();
		seek_track_num[i] = state_fio->FgetInt32();
	}
	state_fio->Fread(snd_seek_name, sizeof(snd_seek_name), 1);
	state_fio->Fread(snd_head_name, sizeof(snd_head_name), 1);
	for(int i = 0; i < PC6031_SND_TBL_MAX; i++) {
		snd_seek_mix_tbl[i] = state_fio->FgetInt32();
	}
	for(int i = 0; i < PC6031_SND_TBL_MAX; i++) {
		snd_head_mix_tbl[i] = state_fio->FgetInt32();
	}
	snd_mute = state_fio->FgetBool();
	snd_level_l = state_fio->FgetInt32();
	snd_level_r = state_fio->FgetInt32();
	if(snd_seek_data != NULL) free(snd_seek_data);
	if(snd_head_data != NULL) free(snd_head_data);
	if(strlen(snd_seek_name) > 0) {
		_TCHAR tmps[512];
		strncpy(tmps, snd_seek_name, 511);
		load_sound_data(PC6031_SND_TYPE_SEEK, (const _TCHAR *)tmps);
	}
	if(strlen(snd_head_name) > 0) {
		_TCHAR tmps[512];
		strncpy(tmps, snd_head_name, 511);
		load_sound_data(PC6031_SND_TYPE_HEAD, (const _TCHAR *)tmps);
	}
#endif
	return true;
}

