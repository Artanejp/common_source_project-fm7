/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date   : 2011.04.23-

	[ MC6800 ]
*/

#ifndef _MC6800_H_ 
#define _MC6800_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

//#if defined(HAS_MC6801) || defined(HAS_HD6301)
//#define SIG_MC6801_PORT_1	0
//#define SIG_MC6801_PORT_2	1
//#define SIG_MC6801_PORT_3	2
//#define SIG_MC6801_PORT_4	3
//#define SIG_MC6801_PORT_3_SC1	4
//#define SIG_MC6801_PORT_3_SC2	5
//#define SIG_MC6801_SIO_RECV	6

//class FIFO;
//#endif

//#ifdef USE_DEBUGGER
class DEBUGGER;
//#endif

class  DLL_PREFIX MC6800 : public DEVICE
{
private:

protected:
	DEVICE *d_mem;
//#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored;
//#endif
	static const uint8_t flags8i[256];
	static const uint8_t flags8d[256];
#define XX 5 // invalid opcode unknown cc
	const uint8_t cycles[256] = {
//#if defined(HAS_MC6800)
		XX, 2,XX,XX,XX,XX, 2, 2, 4, 4, 2, 2, 2, 2, 2, 2,
		2, 2,XX,XX,XX,XX, 2, 2,XX, 2,XX, 2,XX,XX,XX,XX,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4,XX, 5,XX,10,XX,XX, 9,12,
		2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
		2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
		7,XX,XX, 7, 7,XX, 7, 7, 7, 7, 7,XX, 7, 7, 4, 7,
		6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
		2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2, 3, 8, 3, 4,
		3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3, 4, 6, 4, 5,
		5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 6, 8, 6, 7,
		4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 5, 9, 5, 6,
		2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2,XX,XX, 3, 4,
		3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3,XX,XX, 4, 5,
		5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5,XX,XX, 6, 7,
		4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4,XX,XX, 5, 6
	};
#undef XX // invalid opcode unknown cc
	
	pair32_t pc;
	uint16_t prevpc;
	pair32_t sp;
	pair32_t ix;
	pair32_t acc_d;
	pair32_t ea;
	
	uint8_t cc;
	int wai_state;
	int int_state;
	

	uint64_t total_icount;
	uint64_t prev_total_icount;

	int icount;
	bool one_more_insn;
	
	virtual uint32_t __FASTCALL RM(uint32_t Addr);
	virtual void __FASTCALL WM(uint32_t Addr, uint32_t Value);
	uint32_t __FASTCALL RM16(uint32_t Addr);
	void __FASTCALL WM16(uint32_t Addr, pair32_t *p);
	
	
	virtual void __FASTCALL increment_counter(int amount);
	virtual void __FASTCALL run_one_opecode();
	void enter_interrupt(uint16_t irq_vector);
	virtual void __FASTCALL insn(uint8_t code);
	
	void __FASTCALL aba();
	void __FASTCALL adca_di();
	void __FASTCALL adca_ex();
	void __FASTCALL adca_im();
	void __FASTCALL adca_ix();
	void __FASTCALL adcb_di();
	void __FASTCALL adcb_ex();
	void __FASTCALL adcb_im();
	void __FASTCALL adcb_ix();
	void __FASTCALL adda_di();
	void __FASTCALL adda_ex();
	void __FASTCALL adda_im();
	void __FASTCALL adda_ix();
	void __FASTCALL addb_di();
	void __FASTCALL addb_ex();
	void __FASTCALL addb_im();
	void __FASTCALL addb_ix();

	void __FASTCALL anda_di();
	void __FASTCALL anda_ex();
	void __FASTCALL anda_im();
	void __FASTCALL anda_ix();
	void __FASTCALL andb_di();
	void __FASTCALL andb_ex();
	void __FASTCALL andb_im();
	void __FASTCALL andb_ix();
	void __FASTCALL asl_ex();
	void __FASTCALL asl_ix();
	void __FASTCALL asla();
	void __FASTCALL aslb();

	void __FASTCALL asr_ex();
	void __FASTCALL asr_ix();
	void __FASTCALL asra();
	void __FASTCALL asrb();
	void __FASTCALL bcc();
	void __FASTCALL bcs();
	void __FASTCALL beq();
	void __FASTCALL bge();
	void __FASTCALL bgt();
	void __FASTCALL bhi();
	void __FASTCALL bita_di();
	void __FASTCALL bita_ex();
	void __FASTCALL bita_im();
	void __FASTCALL bita_ix();
	void __FASTCALL bitb_di();
	void __FASTCALL bitb_ex();
	void __FASTCALL bitb_im();
	void __FASTCALL bitb_ix();
	void __FASTCALL ble();
	void __FASTCALL bls();
	void __FASTCALL blt();
	void __FASTCALL bmi();
	void __FASTCALL bne();
	void __FASTCALL bpl();
	void __FASTCALL bra();
	void __FASTCALL brn();
	void __FASTCALL bsr();
	void __FASTCALL bvc();
	void __FASTCALL bvs();
	void __FASTCALL cba();
	void __FASTCALL clc();
	void __FASTCALL cli();
	void __FASTCALL clr_ex();
	void __FASTCALL clr_ix();
	void __FASTCALL clra();
	void __FASTCALL clrb();
	void __FASTCALL clv();
	void __FASTCALL cmpa_di();
	void __FASTCALL cmpa_ex();
	void __FASTCALL cmpa_im();
	void __FASTCALL cmpa_ix();
	void __FASTCALL cmpb_di();
	void __FASTCALL cmpb_ex();
	void __FASTCALL cmpb_im();
	void __FASTCALL cmpb_ix();
	void __FASTCALL cmpx_di();
	void __FASTCALL cmpx_ex();
	void __FASTCALL cmpx_im();
	void __FASTCALL cmpx_ix();
	void __FASTCALL com_ex();
	void __FASTCALL com_ix();
	void __FASTCALL coma();
	void __FASTCALL comb();
	void __FASTCALL daa();
	void __FASTCALL dec_ex();
	void __FASTCALL dec_ix();
	void __FASTCALL deca();
	void __FASTCALL decb();
	void __FASTCALL des();
	void __FASTCALL dex();

	void __FASTCALL eora_di();
	void __FASTCALL eora_ex();
	void __FASTCALL eora_im();
	void __FASTCALL eora_ix();
	void __FASTCALL eorb_di();
	void __FASTCALL eorb_ex();
	void __FASTCALL eorb_im();
	void __FASTCALL eorb_ix();
	virtual void __FASTCALL illegal();
	void __FASTCALL inc_ex();
	void __FASTCALL inc_ix();
	void __FASTCALL inca();
	void __FASTCALL incb();
	void __FASTCALL ins();
	void __FASTCALL inx();
	void __FASTCALL jmp_ex();
	void __FASTCALL jmp_ix();
	void __FASTCALL jsr_di();
	void __FASTCALL jsr_ex();
	void __FASTCALL jsr_ix();
	void __FASTCALL lda_di();
	void __FASTCALL lda_ex();
	void __FASTCALL lda_im();
	void __FASTCALL lda_ix();
	void __FASTCALL ldb_di();
	void __FASTCALL ldb_ex();
	void __FASTCALL ldb_im();
	void __FASTCALL ldb_ix();
	void __FASTCALL lds_di();
	void __FASTCALL lds_ex();
	void __FASTCALL lds_im();
	void __FASTCALL lds_ix();
	void __FASTCALL ldx_di();
	void __FASTCALL ldx_ex();
	void __FASTCALL ldx_im();
	void __FASTCALL ldx_ix();
	void __FASTCALL lsr_ex();
	void __FASTCALL lsr_ix();
	void __FASTCALL lsra();
	void __FASTCALL lsrb();
	void __FASTCALL neg_ex();
	void __FASTCALL neg_ix();
	void __FASTCALL nega();
	void __FASTCALL negb();
	void __FASTCALL nop();

	void __FASTCALL ora_di();
	void __FASTCALL ora_ex();
	void __FASTCALL ora_im();
	void __FASTCALL ora_ix();
	void __FASTCALL orb_di();
	void __FASTCALL orb_ex();
	void __FASTCALL orb_im();
	void __FASTCALL orb_ix();
	void __FASTCALL psha();
	void __FASTCALL pshb();
	void __FASTCALL pula();
	void __FASTCALL pulb();
	void __FASTCALL rol_ex();
	void __FASTCALL rol_ix();
	void __FASTCALL rola();
	void __FASTCALL rolb();
	void __FASTCALL ror_ex();
	void __FASTCALL ror_ix();
	void __FASTCALL rora();
	void __FASTCALL rorb();
	void __FASTCALL rti();
	void __FASTCALL rts();
	void __FASTCALL sba();
	void __FASTCALL sbca_di();
	void __FASTCALL sbca_ex();
	void __FASTCALL sbca_im();
	void __FASTCALL sbca_ix();
	void __FASTCALL sbcb_di();
	void __FASTCALL sbcb_ex();
	void __FASTCALL sbcb_im();
	void __FASTCALL sbcb_ix();
	void __FASTCALL sec();
	void __FASTCALL sei();
	void __FASTCALL sev();
	void __FASTCALL sta_di();
	void __FASTCALL sta_ex();
	void __FASTCALL sta_im();
	void __FASTCALL sta_ix();
	void __FASTCALL stb_di();
	void __FASTCALL stb_ex();
	void __FASTCALL stb_im();
	void __FASTCALL stb_ix();
	void __FASTCALL sts_di();
	void __FASTCALL sts_ex();
	void __FASTCALL sts_im();
	void __FASTCALL sts_ix();
	void __FASTCALL stx_di();
	void __FASTCALL stx_ex();
	void __FASTCALL stx_im();
	void __FASTCALL stx_ix();
	void __FASTCALL suba_di();
	void __FASTCALL suba_ex();
	void __FASTCALL suba_im();
	void __FASTCALL suba_ix();
	void __FASTCALL subb_di();
	void __FASTCALL subb_ex();
	void __FASTCALL subb_im();
	void __FASTCALL subb_ix();
	void __FASTCALL swi();
	void __FASTCALL tab();
	void __FASTCALL tap();
	void __FASTCALL tba();

	void __FASTCALL tpa();
	void __FASTCALL tst_ex();
	void __FASTCALL tst_ix();
	void __FASTCALL tsta();
	void __FASTCALL tstb();
	void __FASTCALL tsx();
	void __FASTCALL txs();
	void __FASTCALL wai();
	unsigned Dasm680x(int subtype, _TCHAR *buf, unsigned pc, const UINT8 *oprom, const UINT8 *opram, symbol_t *first_symbol);

	bool __USE_DEBUGGER;
public:
	MC6800(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_mem = NULL;
		d_debugger = NULL;
		d_mem_stored = NULL;
		__USE_DEBUGGER = false;
		for(int i = 0; i < 4; i++) {
			initialize_output_signals(&port[i].outputs);
			port[i].ddr = 0;
			port[i].latched_data = 0;
			port[i].latched = false;
			port[i].first_write = false;
			port[i].wreg = port[i].rreg = 0;//0xff;
		}
		memset(ram, 0x00, sizeof(ram));
//		initialize_output_signals(&outputs_sio);
//#if defined(HAS_MC6801)
//		set_device_name(_T("MC6801 MPU"));
//#elif defined(HAS_HD6301)
//		set_device_name(_T("HD6301 MPU"));
//#else
		set_device_name(_T("MC6800 MPU"));
//#endif
	}
	~MC6800() {}
	
	// common functions
	struct {
		uint8_t wreg;
		uint8_t rreg;
		uint8_t ddr;
		uint8_t latched_data;
		bool latched;
		outputs_t outputs;
		bool first_write;
	} port[4];
	uint8_t ram[128];

	virtual void initialize();
	virtual void reset();
	virtual int __FASTCALL run(int clock);
	
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t get_pc()
	{
		return prevpc;
	}
	uint32_t get_next_pc()
	{
		return pc.w.l;
	}
//#ifdef USE_DEBUGGER
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32_t get_debug_prog_addr_mask()
	{
		return 0xffff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xffff;
	}
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	// implement 16bit/32bit functions because this cpu is big endian
	void __FASTCALL write_debug_data16(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data16(uint32_t addr);
	void __FASTCALL write_debug_data32(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data32(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
//#endif
	virtual bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
//#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
//#endif
};

#endif

