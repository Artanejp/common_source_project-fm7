/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date  : 2011.04.23-

	[ MC6801 ]
*/

#ifndef _MC6801_H_ 
#define _MC6801_H_

//#include "vm.h"
//#include "../emu.h"
#include "mc6800.h"
#include "device.h"

//#if defined(HAS_MC6801) || defined(HAS_HD6301)
#define SIG_MC6801_PORT_1	0
#define SIG_MC6801_PORT_2	1
#define SIG_MC6801_PORT_3	2
#define SIG_MC6801_PORT_4	3
#define SIG_MC6801_PORT_3_SC1	4
#define SIG_MC6801_PORT_3_SC2	5
#define SIG_MC6801_SIO_RECV	6

class DEBUGGER;
class FIFO;
//#endif
class MC6801 : public MC6800
{
private:
protected:
	const int RMCR_SS[4] = { 16, 128, 1024, 4096 };
#define XX 5 // invalid opcode unknown cc
	const uint8_t cycles[256] = {
//#elif defined(HAS_MC6801)
		XX, 2,XX,XX, 3, 3, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2,
		2, 2,XX,XX,XX,XX, 2, 2,XX, 2,XX, 2,XX,XX,XX,XX,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 4, 4, 3, 3, 3, 3, 5, 5, 3,10, 4,10, 9,12,
		2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
		2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
		6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
		6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
		2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 4, 6, 3, 3,
		3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4,
		4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 5, 5,
		4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 5, 5,
		2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 3,XX, 3, 3,
		3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
		4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
		4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5
	};
#undef XX
	//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	// data
	struct {
		uint8_t wreg;
		uint8_t rreg;
		uint8_t ddr;
		uint8_t latched_data;
		bool latched;
		// output signals
		outputs_t outputs;
		bool first_write;
	} port[4];
	
	uint8_t p3csr;
	bool p3csr_is3_flag_read;
	bool sc1_state;
	bool sc2_state;
	
	// timer
	pair_t counter;
	pair_t output_compare;
	pair_t timer_over;
	uint8_t tcsr;
	uint8_t pending_tcsr;
	uint16_t input_capture;
//#ifdef HAS_HD6301
//	uint16_t latch09;
//#endif
	uint32_t timer_next;
	
	// serial i/o
	outputs_t outputs_sio;
	FIFO *recv_buffer;
	uint8_t trcsr, rdr, tdr;
	bool trcsr_read_tdre, trcsr_read_orfe, trcsr_read_rdrf;
	uint8_t rmcr;
	int sio_counter;
	
	// memory controller
	uint8_t ram_ctrl;
	uint8_t ram[128];
	

	uint32_t mc6801_io_r(uint32_t offset);
	virtual void mc6801_io_w(uint32_t offset, uint32_t data);
	void increment_counter(int amount);

	uint32_t RM(uint32_t Addr) override;
	void WM(uint32_t Addr, uint32_t Value) override;

	void run_one_opecode() override;

	void insn(uint8_t code) override;
	void abx();
	void addd_di();
	void addd_ex();
	void addd_im();
	void addd_ix();
	void asld();
	void ldd_di();
	void ldd_ex();
	void ldd_im();
	void ldd_ix();
	void lsrd();
	void mul();
	void pshx();
	void pulx();
	void std_di();
	void std_ex();
	void std_im();
	void std_ix();
	void subd_di();
	void subd_ex();
	void subd_im();
	void subd_ix();
	void cpx_di();
	void cpx_ex();
	void cpx_im();
	void cpx_ix();

//#endif
public:
	MC6801(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MC6800(parent_vm, parent_emu)
	{
		for(int i = 0; i < 4; i++) {
			initialize_output_signals(&port[i].outputs);
			port[i].wreg = port[i].rreg = 0;//0xff;
		}
		initialize_output_signals(&outputs_sio);
		set_device_name(_T("MC6801 MPU"));
	}
	~MC6801() {}
	void initialize() override;
	void release();
	void reset() override;
	int run(int clock) override;
	void write_signal(int id, uint32_t data, uint32_t mask) override;
	bool process_state(FILEIO* state_fio, bool loading);


	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len) override;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	void set_context_port1(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[0].outputs, device, id, mask, shift);
	}
	void set_context_port2(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[1].outputs, device, id, mask, shift);
	}
	void set_context_port3(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[2].outputs, device, id, mask, shift);
	}
	void set_context_port4(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[2].outputs, device, id, mask, shift);
	}
	void set_context_sio(DEVICE* device, int id)
	{
		register_output_signal(&outputs_sio, device, id, 0xff);
	}
//#endif
};

#endif
