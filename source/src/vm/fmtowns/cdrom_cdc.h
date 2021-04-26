#pragma once

#include "../device.h"

class FMTOWNS::CDROM_SKELON;
class FIFO;

#define SIG_CDC_DMA_EOT		1


#define TOWNS_CDC_RAM_SIZE 8192 // Will be extend for later VMs.

namespace FMTOWNS {

enum WREG_ADDRESS {
	CDC_GENERAL = 0,
	CDC_CMD = 2,
	CDC_PARAM = 4,
	CDC_TRANSFER = 6,
};

class CDROM_CDC : public DEVICE {
protected:
	CDROM_SKELTON* d_cdrom;
	DEVICE* d_dmac;
	
	// About STATUS
	int32_t fifo_size;
	int param_ptr;
	uint32_t max_address;
	
	uint8_t ram[TOWNS_CDC_RAM_SIZE];

	uint8_t regs[16]; // Registers.
	uint8_t param_queue[8];	// 8 (Ring buffer)
	
	FIFO* status_queue;	// 8*100 + 4
	
	// Around CDC's RAM (R/W)
	uint32_t read_ptr;
	uint32_t write_ptr;
	int32_t data_count;
	int32_t data_count_tmp;
	
	int64_t tmp_bytes_count;

	// Re-Interpreted around STATUS.
	bool mcu_intr;				// 04C0h: bit7
	bool dma_intr;				// 04C0h: bit6
	bool pio_transfer_phase;	// 04C0h: bit5
	bool dma_transfer_phase;	// 04C0h: bit4
//	bool has_status;			// 04C0h: bit1
	bool mcu_ready;				// 04C0h: bit0

	uint8_t last_commnd;
	// Transfer status values.
	bool status_seek;
	
	// Around CD-ROM/DVD-ROM
	int64_t sectors_remain;
	uint64_t target_lba;
	
	bool access_status;
	bool in_track;
	bool enable_prefetch;
	
	// CDDA
	int volume_l;
	int volume_r;

	int fadeout_level;
	bool is_plaiing;

	pair16_t cdda_camples[2];
	
	int event_seek;
	int event_drq;
	int event_timeout;
	int event_read_wait;
	
	virtual bool seek_to_lba_msf(uint8_t m, uint8_t s, uint8_t s, uint8_t cmdtype);
	virtual pair16_t read_cdda_sample();
	virtual ssize_t read_sectors(int sectors);
	virtual void write_reg(uint8_t num, uint8_t data);

	inline bool has_status()
	{
//		if(d_cdrom != nullptr) {
			if(status_queue != nullptr) {
				return (status_queue->empty() == false) ? true : false;
			}
//		}
		return false;
	}
	
	inline bool req_status()
	{
		return (((last_command & 0x20) != 0) ? true : false);
//		return (((regs[CDC_CMD] & 0x20) != 0) ? true : false);
	}
	inline bool stat_reply_intr()
	{
		return (((last_command & 0x40) != 0) ? true : false);
//		return (((regs[CDC_CMD] & 0x40) != 0) ? true : false);
	}
	
	inline bool dma_transfer()
	{
		return (((regs[CDC_TRANSFER] & 0x10) != 0) ? true : false);
	}
	inline bool pio_transfer()
	{
		return (((regs[CDC_TRANSFER] & 0x08) != 0) ? true : false);
	}

	inline uint8_t read_data_from_ram()
	{
		uint8_t dat = 0x00;
		if(data_count > 0) {
			dat = ram[read_ptr];
			read_ptr++;
			if(read_ptr >= fifo_size) {
				read_ptr = 0;
			}
			data_count--;
		}
		return dat;
	}
public:
	CDROM_CDC(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		cdrom = NULL;
		param_queue = NULL;
		status_queue = NULL;
		d_cdrom = NULL;
		d_dmac = NULL;
	}
	~CDROM_CDC()
	{
		close();
	}
	
	virtual void initialize();
	virtual void release();
	
	virtual void reset();
	virtual void event_callback(int event_id, int err);
	
	virtual void mix(int32_t* buffer, int cnt);
	virtual void set_volume(int volume);
	virtual void set_volume(int ch, int decibel_l, int decibel_r);

	virtual uint32_t read_debug_data8(uint32_t addr);
	virtual void write_debug_data8(uint32_t addr, uint32_t data);

	virtual uint32_t read_dma_io8(uint32_t addr);
	virtual uint32_t read_io8(uint32_t addr);
	virtual void write_io8(uint32_t addr, uint32_t data);
	
	
	virtual void write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t read_signal(int id);

	virtual bool mounted();
	virtual bool accessed();

	virtual void open(const _TCHAR* file_path);
	virtual void close();

	virtual bool get_debug_regs_info(_TCHAR* buffer, size_t buffer_len);
	virtual bool write_debug_reg(_TCHAR* reg, uint32_t data);
	

};
}
