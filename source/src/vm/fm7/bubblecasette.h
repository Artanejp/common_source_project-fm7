/*
 * BUBBLE CASETTE for FM-8/7? [bubblecasette.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Mar 22, 2016 : Initial
 *
 */
#ifndef _VM_FM_BUBBLECASETTE_H_
#define _VM_FM_BUBBLECASETTE_H_


#include "../device.h"
#include "../../common.h"

class EMU;
class VM;
class FILEIO;

enum {
	BUBBLE_DATA_REG = 0,
	BUBBLE_CMD_REG,
	BUBBLE_STATUS_REG,
	BUBBLE_ERROR_REG,
	BUBBLE_PAGE_ADDR_HI,
	BUBBLE_PAGE_ADDR_LO,
	BUBBLE_PAGE_COUNT_HI,
	BUBBLE_PAGE_COUNT_LO,
};

enum {
	BUBBLE_TYPE_32KB = 0,
	BUBBLE_TYPE_128KB = 1,
	BUBBLE_TYPE_B77,
};

typedef struct {
	_TCHAR filename[16];
	pair32_t size;
	pair32_t offset;
	uint8_t misc[8];
} bbl_header_t;

class BUBBLECASETTE: public DEVICE {
protected:
	FILEIO* fio;
	
	bool is_wrote;
	bool is_b77;
	bool header_changed; // if change header: e.g:change write protection flag.
	bool read_access;
	bool write_access;

	uint8_t offset_reg;
	// FD10(RW)
	uint8_t data_reg;
	// FD11(RW)
	uint8_t cmd_reg;

	// FD12(RO) : Positive logic
	bool cmd_error;  // bit7 : Command error
	bool stat_tdra;  // bit6: Ready to write.
	bool stat_rda;   // bit5: Ready to read.
	bool not_ready;  // bit4: Not Ready(Slot empty).
	// bit3 : NOOP
	bool write_protect; // bit2
	bool stat_error; // bit 1
	bool stat_busy;  // bit 0

	// FD13(RO): Maybe positive
	bool eject_error;         // bit7
	bool povr_error;          // bit5 : Page over
	bool crc_error;           // bit4
	bool transfer_error;      // bit3
	bool bad_loop_over_error; // bit2
	bool no_marker_error;     // bit1
	bool undefined_cmd_error; // bit0

	//FD14-FD15: Page address register
	pair32_t page_address; // 16bit, Big ENDIAN
	// FD16-FD17: Page Count Resister
	pair32_t page_count;   // 16bit, Big ENDIAN
private:
	bool bubble_inserted;
	int bubble_type;
	int media_num;
	bbl_header_t bbl_header;
	uint32_t media_offset;
	uint32_t media_offset_new;
	uint32_t media_size;
	uint32_t file_length;

	uint8_t bubble_data[0x20000]; // MAX 128KB, normally 32KB at FM-8.
	_TCHAR image_path[_MAX_PATH];
	
	void bubble_command(uint8_t cmd);
	bool read_header(void);
	void write_header(void);
	bool read_one_page(void);
	bool write_one_page(void);
public:
	BUBBLECASETTE(VM_TEMPLATE* parent_vm, EMU* parent_emu);
	~BUBBLECASETTE();

	void initialize();
	void reset();

	uint32_t read_data8(uint32_t addr);
	void write_data8(uint32_t addr, uint32_t data);
	
	uint32_t read_signal(int id);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool open(_TCHAR* file_path, int bank);
	void close();
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	bool decl_state(FILEIO *state_fio, bool loading);
	
	bool is_bubble_inserted()
	{
		return bubble_inserted;
	}
	bool is_bubble_protected()
	{
		return write_protect;
	}
	void set_bubble_protect(bool flag)
	{
		if(write_protect != flag) {
			write_protect = flag;
			header_changed = true;
		}
	}
	bool get_access_lamp()
	{
		return (read_access | write_access);
	}

};
#endif
