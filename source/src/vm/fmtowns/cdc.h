/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[FM-Towns CD Controller with PSEUDO MB88505H 4bit MCU]
*/
#pragma once

#include "../../common.h"
#include "../device.h"

#define SIG_TOWNS_CDC_DRQ                 1
#define SIG_TOWNS_CDC_IRQ                 2
#define SIG_TOWNS_CDC_BSY                 3
#define SIG_TOWNS_CDC_CD                  4
#define SIG_TOWNS_CDC_IO                  5
#define SIG_TOWNS_CDC_MSG                 6
#define SIG_TOWNS_CDC_REQ                 7
#define SIG_TOWNS_CDC_ACK                 8
#define SIG_TOWNS_CDC_CDROM_DONE          9
#define SIG_TOWNS_CDC_NEXT_SECTOR        10
#define SIG_TOWNS_CDC_TRANSFER_COMPLETE  11

class SCSI_HOST;
class FIFO;

namespace FMTOWNS {
	class TOWNS_CDROM;
}
typedef struct {
	uint8_t cmd_table_size;
	uint8_t cmd_write_ptr;
	uint8_t command[16];
} towns_cdc_cmdqueue_t;

#define CDC_COMMAND_QUEUE_LENGTH 8
namespace FMTOWNS {
class CDC : public DEVICE {
protected:
	outputs_t output_dma_line;
	outputs_t output_dma_intr;
	outputs_t output_submpu_intr;

	DEVICE*    d_dmac;
	SCSI_HOST* d_scsi_host;
	TOWNS_CDROM* d_cdrom;
	uint8_t param_queue[8];

	towns_cdc_cmdqueue_t cmdqueue[CDC_COMMAND_QUEUE_LENGTH];
	uint8_t current_cmdqueue;
	uint8_t next_cmdqueue;
	int left_cmdqueue;
	
	int param_ptr;
	FIFO* stat_fifo;

	int readptr;

	int sectors_count;
	int read_lba;
	
	bool has_status;
	int extra_status;
	bool submpu_ready;
	bool software_transfer_phase;
	bool dma_transfer_phase;
	bool dma_transfer;
	bool pio_transfer;

	bool dma_intr;
	bool submpu_intr;
	bool dma_intr_mask;
	bool submpu_intr_mask;
	
	bool busy_status;
	bool cd_status;
	bool io_status;
	bool msg_status;
	bool scsi_req_status;
	bool ack_status;
	uint8_t raw_status;
	
	bool data_in_status;
	uint8_t data_reg;
	
	uint8_t w_regs[16];
	bool command_type_play;
	bool stat_reply_intr;

	bool accept_command;
	bool req_status;
	int buffer_count;
	
	int event_cdrom_sel;
	int event_poll_cmd;
	int event_enqueue_cmd;
	int event_wait_req;
	int event_wait_cmd_req_off;
	int event_cdc_status;
	int event_cdrom_status;
	
	virtual void read_cdrom(bool req_reply);
	virtual void stop_cdda(bool req_reply);
	virtual void stop_cdda2(bool req_reply);
	virtual void unpause_cdda(bool rea_reply);
	virtual void play_cdda(bool req_reply);
	virtual void __FASTCALL write_status(uint8_t a, uint8_t b, uint8_t c, uint8_t d, bool immediately = false);
	virtual void enqueue_command_play(uint8_t cmd);
	virtual void enqueue_command_status(uint8_t cmd);

	bool check_bus_free();
	bool check_command_phase();
	bool check_data_in();
	bool check_data_out();
	bool check_status();
	bool check_message_in();
	bool check_message_out();
	
	void select_unit_on();
	void select_unit_off();
	void select_unit_off2();
	
	void prologue_command_phase();
	
	void enqueue_cmdqueue(int size, uint8_t data[]);
	void start_poll_bus_free(int unit);
	void start_poll_cmd_phase();
	void start_enqueue_command();

	void read_a_sector(int lba, bool req_reply);
public:
	CDC(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&output_dma_line);
		initialize_output_signals(&output_dma_intr);
		initialize_output_signals(&output_submpu_intr);
		
		set_device_name(_T("FM-Towns CD-ROM controller"));
	}
	~CDC() { }

	virtual void initialize();
	virtual void release();
	virtual void reset();

	virtual void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);

	virtual void __FASTCALL write_io8(uint32_t address, uint32_t data);
	virtual uint32_t __FASTCALL read_io8(uint32_t address);

	virtual uint32_t __FASTCALL read_dma_io8(uint32_t addr);
	virtual uint32_t __FASTCALL read_dma_io16(uint32_t addr);
	virtual void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data);
	virtual void __FASTCALL write_dma_io16(uint32_t addr, uint32_t data);

	virtual void event_callback(int id, int error);
	virtual bool process_state(FILEIO* state_fio, bool loading);
	
	virtual void set_context_scsi_host(SCSI_HOST* dev);
	virtual void set_context_cdrom(TOWNS_CDROM* dev);

	void set_context_dmac(DEVICE *dev)
	{
		d_dmac = dev;
	}
	void set_context_dmareq_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&output_dma_line, dev, id, mask);
	}
	
	void set_context_dmaint_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&output_dma_intr, dev, id, mask);
	}
	
	void set_context_mpuint_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&output_submpu_intr, dev, id, mask);
	}
};

}
