/*
 * BUBBLE CASETTE for FM-8/7? [bubblecasette.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Mar 22, 2016 : Initial
 *
 */
#include "../vm.h"
#include "../device.h"
#include "./bubblecasette.h"

BUBBLECASETTE::BUBBLECASETTE(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	fio = NULL;
	memset(image_path, 0x00, _MAX_PATH * sizeof(_TCHAR));
	for(i = 0; i < 16; i++) {
		bubble_type[i] = -1;
		memset(bbl_header[i], 0x00, sizeof(bbl_header_t));
		memset(bubble_data[i], 0x00, 0x20000);
	}
	bubble_inserted = false;
	read_access = write_access = false;
}

BUBBLECASETTE::~BUBBLECASETTE()
{
}

void BUBBLECASETTE::initialize()
{
	is_wrote = false;
	cmd_reg = 0;
}

void BUBBLECASETTE::reset()
{
	data_reg = 0;
	if(is_wrote) {
		write_one_page();
		is_wrote = false;
	}
	cmd_reg = 0;
	page_addr.d = 0;
	page_count.d = 0;
	write_access = read_access = false;
	
	stat_busy = false;
	stat_tdra = false;
	stat_rda = false;
	cmd_error = false;
	stat_error = false;
	
	eject_error = false;
	povr_error = false;
	crc_error = false;
	transfer_error = false;
	bad_loop_over_error = false;
	no_merker_error = false;
	undefined_cmd_error = false;
}

uint32_t BUBBLECASETTE::read_data8(uint32_t address)
{
	uint32_t val = 0xff;
	uint32_t offset;
	uint16_t mask;
	uint16_t page_size_tmp;
	uint32_t media_size_tmp;
	if(bubble_type[media_num] == BUBBLE_TYPE_32KB) {
		page_size_tmp = 0x20;
		mask = 0x3ff;
		media_size_tmp = 0x8000;
	} else if(bubble_type[media_num] == BUBBLE_TYPE_128KB) {
		page_size_tmp = 0x40;
		mask = 0x7ff;
		media_size_tmp = 0x20000;
	} else {
		return val; // Not inserted.
	}
	switch(address & 7) {
	case 0: // Data Resistor
		val = (uint32_t)data_reg;
		offset = page_address.w.l * page_size_tmp + offset_reg;
		if(stat_rda) {
			if(offset >= media_size_tmp) {
				error_povr = true;
				stat_error = true;
				cmd_error = true;
				return;
			}
			write_access = false;
			read_access = true;
			offset_reg++;
			if(offset_reg == page_size_tmp) {
				stat_busy = false;
				stat_rda = false;
				cmd_error = true;
				offset_reg = 0;
				if(page_count.w.l > 0) page_count.w.l--;
				if((page_count.w.l > 0) && (offset < media_size_tmp)) {
					// Multi read
					stat_busy = true;
					cmd_error = false;
					page_address.w.l = (page_address.w.l + 1) & mask;
					data_reg = bubble_data[media_num][page_address.w.l * page_size_tmp];
					stat_rda = true;
				} else { // End multi read
					offset_reg = 0;
				}
			} else {
				data_reg = bubble_data[media_num][(page_address.w.l & mask) * page_size_tmp + offset_reg];
				stat_rda = true;
				cmd_error = false;
				//busy = true;
			}
		}
		break;
	case 2:
		val = 0x00;
		val |= (cmd_error)  ? 0x80 : 0;
		val |= (stat_tdra)  ? 0x40 : 0;
		val |= (stat_rda)   ? 0x20 : 0;
		val |= (stat_error) ? 0x02 : 0;
		val |= (stat_busy)  ? 0x01 : 0;
		break;
	case 3:
		val = 0x00;
		val |= (eject_error)         ? 0x80 : 0;
		val |= (povr_error)          ? 0x20 : 0;
		val |= (crc_error)           ? 0x10 : 0;
		val |= (transfer_error)      ? 0x08 : 0;
		val |= (bad_loop_over_error) ? 0x04 : 0;
		val |= (no_merker_error)     ? 0x02 : 0;
		val |= (undefined_cmd_error) ? 0x01 : 0;
		break;
	}
	return val;
}

void BUBBLECASETTE::bubble_command(uint8_t cmd)
{
	uint16_t mask;
	uint16_t page_size_tmp;
	uint32_t media_size_tmp;
	if(bubble_type[media_num] == BUBBLE_TYPE_32KB) {
		page_size_tmp = 0x20;
		mask = 0x3ff;
		media_size_tmp = 0x8000;
	} else if(bubble_type[media_num] == BUBBLE_TYPE_128KB) {
		page_size_tmp = 0x40;
		mask = 0x7ff;
		media_size_tmp = 0x20000;
	}
	switch(cmd) {
	case 1: // Read
		offset_reg = 0;
		read_access = false;
		if(contains_medias <= media_num) {
			stat_error = true;
			cmd_error = true;
			no_merker_error = true;
		} else {
			data_reg = bubble_data[media_num][(page_address.w.l & mask) * page_size_tmp];
			stat_rda = true;
			read_access = true;
		}
		break;
	case 2: // Write :: Will not check insert?
		stat_busy = true;
		write_access = false;
		if(contains_medias <= media_num) {
			stat_error = true;
			cmd_error = true;
			no_merker_error = true;
		} else {
			if(write_protect[media_num]) {
				stat_busy = false;
				cmd_error = true;
			} else {
				offset_reg = 0;
				write_access = true;
				stat_tdra = true;
			}
		}
		break;
	case 4:
	case 15: // Reset
		data_reg = 0;
		if(is_wrote) {
			write_one_page();
			is_wrote = false;
		}
		page_addr.d = 0;
		page_count.d = 0;
		write_access = read_access = false;
		break;
	default:
		stat_error = true;
		cmd_error = true;
		undefined_cmd_error = true;
		break;
	}
}
void BUBBLECASETTE::write_data8(uint32_t address, uint32_t data)
{
	uint8_t val;
	uint32_t offset;
	uint16_t mask;
	uint16_t page_size_tmp;
	uint32_t media_size_tmp;
	if(bubble_type[media_num] == BUBBLE_TYPE_32KB) {
		page_size_tmp = 0x20;
		mask = 0x3ff;
		media_size_tmp = 0x8000;
	} else if(bubble_type[media_num] == BUBBLE_TYPE_128KB) {
		page_size_tmp = 0x40;
		mask = 0x7ff;
		media_size_tmp = 0x20000;
	} else {
		return; // Not inserted.
	}
	val = (uint8_t)data;

	switch(address & 7) {
	case 0: // WIP
		data_reg = val;
		if(stat_tdra) {
			if(offset >= media_size_tmp) {
				error_povr = true;
				stat_error = true;
				cmd_error = true;
				return;
			}
			is_wrote = true;
			offset = page_address.w.l * page_size_tmp + offset_reg;
			bubble_data[media_num][(page_address.w.l & mask) * page_size_tmp + offset_reg] = data_reg;
			write_access = true;
			read_access = false;
			offset_reg++;
			if(offset_reg == page_size_tmp) {
				stat_busy = false;
				stat_tdra = false;
				cmd_error = true;
				if(!write_one_page()) {
					stat_error = true;
					cmd_error = true;
				}
				offset_reg = 0;
				if(page_count.w.l > 0) page_count.w.l--;
				if((page_count.w.l > 0) && (offset < media_size_tmp)) {
					stat_busy = true;
					stat_tdra = true;
					cmd_wrror = false;
				} else {
					write_access = false;
				}
			}
		}
		break;
	case 1: // WIP
		stat_busy = false;
		stat_tdra = false;
		stat_rda = false;
		cmd_error = false;
		stat_error = false;

		eject_error = false;
		povr_error = false;
		crc_error = false;
		transfer_error = false;
		bad_loop_over_error = false;
		no_merker_error = false;
		undefined_cmd_error = false;
		cmd_reg = val;
		bubble_command(val);
		
		break;
	case 2: // Read only
	case 3: // Read only
		break;
	case 4:
		page_address.b.h = val;
		break;
	case 5:
		page_address.b.l = val;
		break;
	case 6:
		page_count.b.h = val;
		break;
	case 7:
		page_count.b.l = val;
		break;
	}
}

uint32_t BUBBLECASETTE::read_signal(int address)
{
}

void BUBBLECASETTE::write_signal(int id, uint32_t data, uint32_t mask)
{
}

bool BUBBLECASETTE::insert_bubble(_TCHAR* file_path)
{
	int i;
	is_wrote = false;
	fio = NULL;
	media_num = 0;
	contain_medias = 0;
	file_length = 0;

	for(i = 0; i < 16; i++) memset(bubble_data[i], 0, 0x20000);
	for(i = 0; i < 16; i++) memset(bbl_header[i], 0, sizeof(bbl_header_t));

	if(fio != NULL) {
		close_bubble();
	}
	fio = new FILEIO;
	if(fio == NULL) return false;
	memset(image_path, 0x00, _MAX_PATH * sizeof(_TCHAR));
	strncpy(image_path, file_path, _MAX_PATH);
	
	if(fio->IsFileExisting(file_path)) {
		fio->Fopen(file_path);
		file_length = fio->FileLength();
		if(file_length == 0) return false;
		for(i = 0; i < 16; i++) bubble_type[i] = -1;
		if(file_length[media_num] == 0x8000) { // 32KB
			bubble_type[media_num] = BUBBLE_TYPE_32KB;
		} else if(file_length == 0x10000) {
			bubble_type[media_num] = BUBBLE_TYPE_128KB;
		} else {
			bubble_type[media_num] = BUBBLE_TYPE_B77;
		}
		
		if(bubble_type[media_num] != BUBBLE_TYPE_B77) {
			if(bubble_type < 0) return false;
			write_protect[media_num] = false;
			switch(bubble_type) {
			case BUBBLE_TYPE_32KB:
				fio->Fread(bubble_data[0], 0x8000, 1);
				break;
			case BUBBLE_TYPE_128KB:
				fio->Fread(bubble_data[0], 0x20000, 1);
				break;
			}
			contain_medias = 1;
			bubble_inserted = true;
		} else {
			int remain;
			do {
				write_protect[media_num] = false;
				if(!this->read_header()) break;
				if(bubble_type[media_num] == BUBBLE_TYPE_32KB) {
					if(bbl_header[media_num].offset.d > 0x20) fio->Fseek(bbl_header[media_num].offset.d - 0x20, FILEIO_SEEK_SET);
					remain = media_size[media_num] - bbl_header[media_num].offset.d;
					if(remain >= 0x8000) {
						remain = 0x8000;
					}
					if(fio->Fread(bubble_data[media_num], remain, 1) != remain) break; // EOF
				} else if(bubble_type[media_num] == BUBBLE_TYPE_128KB) {
					if(bbl_header[media_num].offset.d > 0x20) fio->Fseek(bbl_header[media_num].offset.d - 0x20, FILEIO_SEEK_SET);
					remain = media_size[media_num] - bbl_header[media_num].offset.d;
					if(remain >= 0x20000) {
						remain = 0x20000;
					}
					if(fio->Fread(bubble_data[media_num], remain, 1) != remain) break; // EOF
				} else {
					break;
				}
				media_num++;
				contain_medias++;
			} while(contain_medias < 16);
			if(contain_medias <= 0) return false;
			bubble_inserted = true;
		}
		return true;
	}
	return false; // Unexpected error;
}

bool BUBBLECASETTE::read_header()
{
	uint32 f_pos;
	uint8_t tmpval[12];
	if(fio == NULL) return false;
	if(fio->Ftell() >= file_length) return false;
	uint8_t* ptr = (uint8_t *)(&bbl_header[media_num]);
	fio->Fread(ptr->filename, 0x10, 1);

	if(fio->Fread(&tmpval, 16, 1) != 16) return false;
	// Offset(little endian)
	ptr->offset.b.l  = tmpval[4];
	ptr->offset.b.h  = tmpval[5];
	ptr->offset.b.h2 = tmpval[6];
	ptr->offset.b.h3 = tmpval[7];
	// Casette size(little endian)
	ptr->size.b.l  = tmpval[12];
	ptr->size.b.h  = tmpval[13];
	ptr->size.b.h2 = tmpval[14];
	ptr->size.b.h3 = tmpval[15];
	// Misc
	ptr->misc[0] = tmpval[0];
	ptr->misc[1] = tmpval[1];
	ptr->misc[2] = tmpval[2];
	ptr->misc[3] = tmpval[3];
	
	ptr->misc[4] = tmpval[8];
	ptr->misc[5] = tmpval[9];
	ptr->misc[6] = tmpval[10];
	ptr->misc[7] = tmpval[11];
	
	switch(tmpval[11]) {
	case 0x80:
		bubble_type[media_num] = BUBBLE_TYPE_32KB;
		break;
	case 0x90:
		bubble_type[media_num] = BUBBLE_TYPE_128KB;
		break;
	default:
		return false;
		break;
	}
	if(media_num > 0) {
		media_offset[media_num] = media_size[media_num - 1] + media_size[media_num - 1]; 
		media_size[media_num] = ptr->size.d;
	}
	return true;
}	

bool BUBBLECASETTE::read_one_page()
{
	uint32_t f_pos;
	if(fio = NULL) return;
	if(!fio->IsOpened()) return;
	
	if(media_num >= contain_medias) return;
	f_pos = media_offset[media_num];
	{
		uint32_t offset;
		switch(bubble_type[media_num]) {
		case BUBBLE_TYPE_32KB:
			offset = bbl_header[media_num].offset.d + (page_address.w.l & 0x03ff) * 0x20;
			fio->Fseek(f_pos + offset, FILEIO_SEEK_SET);
			fio->Fread(&bubble_data[media_num][(page_address.w.l & 0x03ff) * 0x20], 0x20, 1);
			break;
		case BUBBLE_TYPE_128KB:
			offset = bbl_header[media_num].offset.d + (page_address.w.l & 0x07ff) * 0x40;
			fio->Fseek(f_pos + offset, FILEIO_SEEK_SET);
			fio->Fread(&bubble_data[media_num][(page_address.w.l & 0x07ff) * 0x40], 0x40, 1);
			break;
		}
	}
}

bool BUBBLECASETTE::write_one_page()
{
	uint32_t f_pos;
	if(fio = NULL) return false;
	if(!fio->IsOpened()) return false;
	
	if(media_num >= contain_medias) return false;
	f_pos = media_offset[media_num];
	if(is_wrote) {
		uint32_t offset;
		switch(bubble_type[media_num]) {
		case BUBBLE_TYPE_32KB:
			offset = bbl_header[media_num].offset.d + (page_address.w.l & 0x03ff) * 0x20;
			fio->Fseek(f_pos + offset, FILEIO_SEEK_SET);
			fio->Fwrite(&bubble_data[media_num][(page_address.w.l & 0x03ff) * 0x20], 0x20, 1);
			break;
		case BUBBLE_TYPE_128KB:
			offset = bbl_header[media_num].offset.d + (page_address.w.l & 0x07ff) * 0x40;
			fio->Fseek(f_pos + offset, FILEIO_SEEK_SET);
			fio->Fwrite(&bubble_data[media_num][(page_address.w.l & 0x07ff) * 0x40], 0x40, 1);
			break;
		}
		is_wrote = false;
	}
	return true;
}
void BUBBLECASETTE::close_bubble()
{
	int i;
	if(fio != NULL) {
		if(fio->IsOpened()) {
			if(is_wrote) write_one_page();
			fio->Fclose();
		}
		delete fio;
	}
	fio = NULL;
	memset(image_path, 0x00, _MAX_PATH * sizeof(_TCHAR));
	for(i = 0; i < 16; i++) {
		bubble_type[i] = -1;
		memset(bbl_header[i], 0x00, sizeof(bbl_header_t));
		memset(bubble_data[i], 0x00, 0x20000);
	}
	bubble_inserted = false;
	read_access = write_access = false;
}

void BUBBLECASETTE::event_callback(int event_id, int err)
{
}

#define STATE_VERSION 1
void BUBBLECASETTE::save_state(FILEIO *state_fio)
{
	int i, j;
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);

	// Data reg
	state_fio->FputUint8(data_reg);
	// Command reg
	state_fio->FputUint8(cmd_reg);
	// Status reg
	state_fio->FputBool(stat_busy);
	state_fio->FputBool(stat_tdra);
	state_fio->FputBool(stat_rda);
	state_fio->FputBool(cmd_error);
	state_fio->FputBool(stat_error);
	// Error reg
	state_fio->FputBool(eject_error);
	state_fio->FputBool(povr_error);
	state_fio->FputBool(crc_error);
	state_fio->FputBool(transfer_error);
	state_fio->FputBool(bad_loop_over_error);
	state_fio->FputBool(no_merker_error);
	state_fio->FputBool(undefined_cmd_error);
	// Page address
	state_fio->FputUint32_BE(page_address.d);
	//Page Count
	state_fio->FputUint32_BE(page_count.d);
	// Misc flags
	state_fio->FputBool(is_wrote);
	state_fio->FputBool(read_access);
	state_fio->FputBool(write_access);
	state_fio->FputInt32_BE(contains_medias);
	state_fio->FputInt32_BE(media_num);
	state_fio->FputUint8(offset_reg);
	
	state_fio->Fwrite(image_path, _MAX_PATH * sizeof(_TCHAR), 1);
	if(fio != NULL) {
		if(fio->IsFileOpened()) {
			if(is_wrote) write_one_page();
			fio->Fclose();
			if(strlen(image_path > 0)) {
				insert_bubble(image_path);
			}
		}
	}
			
}

bool BUBBLECASETTE::load_state(FILEIO *state_fio)
{
	int i, j;
	if(state_fio->FgetUint32_BE() != STATE_VERSION) return false;
	if(state_fio->FgetInt32_BE() != this_device_id) return false;

	// Data reg
	data_reg = state_fio->FgetUint8();
	// Command reg
	cmd_reg = state_fio->FgetUint8();
	// Status reg
	stat_busy = state_fio->FgetBool();
	stat_tdra = state_fio->FgetBool();
	stat_rda = state_fio->FgetBool();
	cmd_error = state_fio->FgetBool();
	stat_error = state_fio->FgetBool();
	// Error reg
	eject_error = state_fio->FgetBool();
	povr_error = state_fio->FgetBool();
	crc_error = state_fio->FgetBool();
	transfer_error = state_fio->FgetBool();
	bad_loop_over_error = state_fio->FgetBool();
	no_merker_error = state_fio->FgetBool();
	undefined_cmd_error = state_fio->FgetBool();
	// Page address
	page_address.d = state_fio->FgetUint32_BE();
	//Page Count
	page_count.d = state_fio->FgetUint32_BE();
	// Misc flags
	is_wrote = state_fio->FgetBool();
	read_access = state_fio->FgetBool();
	write_access = state_fio->FgetBool();
	contains_medias = state_fio->FgetInt32_BE();
	media_num = state_fio->FgetInt32_BE();
	offset_reg = state_fio->FgetUint8();
	
	if(state_fio->Fread(image_path, _MAX_PATH * sizeof(_TCHAR), 1) != (_MAX_PATH * sizeof(_TCHAR))) return false;
	if(strlen(image_path > 0)) {
		if(!insert_bubble(image_path)) return false;
	}
	return true;
}

