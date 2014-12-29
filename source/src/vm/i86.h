/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date  : 2011.04.23-

	[ 80x86 ]
*/

#ifndef _I86_H_ 
#define _I86_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I86_TEST	0

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class I86 : public DEVICE
{
private:
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */
	
	DEVICE *d_mem, *d_io, *d_pic, *d_bios;
#ifdef SINGLE_MODE_DMA
	DEVICE *d_dma;
#endif
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored, *d_io_stored;
#endif
	
	/* ---------------------------------------------------------------------------
	registers
	--------------------------------------------------------------------------- */
	
	union REGTYPE {
		uint16 w[8]; /* viewed as 16 bits registers */
		uint8 b[16]; /* or as 8 bit registers       */
	} regs;
	uint32 pc;
	uint32 prevpc;
	uint32 base[4];
	uint16 sregs[4];
	uint16 flags;
	int32 AuxVal, OverVal, SignVal, ZeroVal, CarryVal, DirVal;
	uint8 ParityVal;
	uint8 TF, IF, MF;
	
	int int_state;
	bool test_state;
	bool busreq, halted;
	
	int icount;
	int extra_icount;
	
	bool seg_prefix;	/* prefix segment indicator */
	uint8 prefix_seg;	/* The prefixed segment */
	unsigned ea;
	uint16 eo;		/* effective offset of the address (before segment is added) */
	uint8 ea_seg;		/* effective segment of the address */
	
	/* ---------------------------------------------------------------------------
	opecode
	--------------------------------------------------------------------------- */
	
	// sub
	void interrupt(int num);
	void trap();
	unsigned GetEA(unsigned ModRM);
	void rotate_shift_byte(unsigned ModRM, unsigned cnt);
	void rotate_shift_word(unsigned ModRM, unsigned cnt);
	
	// opecode
	void run_one_opecode();
#ifdef USE_DEBUGGER
	void run_one_opecode_debugger();
#endif
	void instruction(uint8 code);
	inline void _add_br8();
	inline void _add_wr16();
	inline void _add_r8b();
	inline void _add_r16w();
	inline void _add_ald8();
	inline void _add_axd16();
	inline void _push_es();
	inline void _pop_es();
	inline void _or_br8();
	inline void _or_wr16();
	inline void _or_r8b();
	inline void _or_r16w();
	inline void _or_ald8();
	inline void _or_axd16();
	inline void _push_cs();
#if defined(HAS_V30)
	inline void _0fpre();
#endif
	inline void _adc_br8();
	inline void _adc_wr16();
	inline void _adc_r8b();
	inline void _adc_r16w();
	inline void _adc_ald8();
	inline void _adc_axd16();
	inline void _push_ss();
	inline void _pop_ss();
	inline void _sbb_br8();
	inline void _sbb_wr16();
	inline void _sbb_r8b();
	inline void _sbb_r16w();
	inline void _sbb_ald8();
	inline void _sbb_axd16();
	inline void _push_ds();
	inline void _pop_ds();
	inline void _and_br8();
	inline void _and_wr16();
	inline void _and_r8b();
	inline void _and_r16w();
	inline void _and_ald8();
	inline void _and_axd16();
	inline void _es();
	inline void _daa();
	inline void _sub_br8();
	inline void _sub_wr16();
	inline void _sub_r8b();
	inline void _sub_r16w();
	inline void _sub_ald8();
	inline void _sub_axd16();
	inline void _cs();
	inline void _das();
	inline void _xor_br8();
	inline void _xor_wr16();
	inline void _xor_r8b();
	inline void _xor_r16w();
	inline void _xor_ald8();
	inline void _xor_axd16();
	inline void _ss();
	inline void _aaa();
	inline void _cmp_br8();
	inline void _cmp_wr16();
	inline void _cmp_r8b();
	inline void _cmp_r16w();
	inline void _cmp_ald8();
	inline void _cmp_axd16();
	inline void _ds();
	inline void _aas();
	inline void _inc_ax();
	inline void _inc_cx();
	inline void _inc_dx();
	inline void _inc_bx();
	inline void _inc_sp();
	inline void _inc_bp();
	inline void _inc_si();
	inline void _inc_di();
	inline void _dec_ax();
	inline void _dec_cx();
	inline void _dec_dx();
	inline void _dec_bx();
	inline void _dec_sp();
	inline void _dec_bp();
	inline void _dec_si();
	inline void _dec_di();
	inline void _push_ax();
	inline void _push_cx();
	inline void _push_dx();
	inline void _push_bx();
	inline void _push_sp();
	inline void _push_bp();
	inline void _push_si();
	inline void _push_di();
	inline void _pop_ax();
	inline void _pop_cx();
	inline void _pop_dx();
	inline void _pop_bx();
	inline void _pop_sp();
	inline void _pop_bp();
	inline void _pop_si();
	inline void _pop_di();
	inline void _pusha();
	inline void _popa();
	inline void _bound();
	inline void _repc(int flagval);
	inline void _push_d16();
	inline void _imul_d16();
	inline void _push_d8();
	inline void _imul_d8();
	inline void _insb();
	inline void _insw();
	inline void _outsb();
	inline void _outsw();
	inline void _jo();
	inline void _jno();
	inline void _jb();
	inline void _jnb();
	inline void _jz();
	inline void _jnz();
	inline void _jbe();
	inline void _jnbe();
	inline void _js();
	inline void _jns();
	inline void _jp();
	inline void _jnp();
	inline void _jl();
	inline void _jnl();
	inline void _jle();
	inline void _jnle();
	inline void _80pre();
	inline void _81pre();
	inline void _82pre();
	inline void _83pre();
	inline void _test_br8();
	inline void _test_wr16();
	inline void _xchg_br8();
	inline void _xchg_wr16();
	inline void _mov_br8();
	inline void _mov_wr16();
	inline void _mov_r8b();
	inline void _mov_r16w();
	inline void _mov_wsreg();
	inline void _lea();
	inline void _mov_sregw();
	inline void _popw();
	inline void _nop();
	inline void _xchg_axcx();
	inline void _xchg_axdx();
	inline void _xchg_axbx();
	inline void _xchg_axsp();
	inline void _xchg_axbp();
	inline void _xchg_axsi();
	inline void _xchg_axdi();
	inline void _cbw();
	inline void _cwd();
	inline void _call_far();
	inline void _wait();
	inline void _pushf();
	inline void _popf();
	inline void _sahf();
	inline void _lahf();
	inline void _mov_aldisp();
	inline void _mov_axdisp();
	inline void _mov_dispal();
	inline void _mov_dispax();
	inline void _movsb();
	inline void _movsw();
	inline void _cmpsb();
	inline void _cmpsw();
	inline void _test_ald8();
	inline void _test_axd16();
	inline void _stosb();
	inline void _stosw();
	inline void _lodsb();
	inline void _lodsw();
	inline void _scasb();
	inline void _scasw();
	inline void _mov_ald8();
	inline void _mov_cld8();
	inline void _mov_dld8();
	inline void _mov_bld8();
	inline void _mov_ahd8();
	inline void _mov_chd8();
	inline void _mov_dhd8();
	inline void _mov_bhd8();
	inline void _mov_axd16();
	inline void _mov_cxd16();
	inline void _mov_dxd16();
	inline void _mov_bxd16();
	inline void _mov_spd16();
	inline void _mov_bpd16();
	inline void _mov_sid16();
	inline void _mov_did16();
	inline void _rotshft_bd8();
	inline void _rotshft_wd8();
	inline void _ret_d16();
	inline void _ret();
	inline void _les_dw();
	inline void _lds_dw();
	inline void _mov_bd8();
	inline void _mov_wd16();
	inline void _enter();
	inline void _leav();	// _leave()
	inline void _retf_d16();
	inline void _retf();
	inline void _int3();
	inline void _int();
	inline void _into();
	inline void _iret();
	inline void _rotshft_b();
	inline void _rotshft_w();
	inline void _rotshft_bcl();
	inline void _rotshft_wcl();
	inline void _aam();
	inline void _aad();
	inline void _setalc();
	inline void _xlat();
	inline void _escape();
	inline void _loopne();
	inline void _loope();
	inline void _loop();
	inline void _jcxz();
	inline void _inal();
	inline void _inax();
	inline void _outal();
	inline void _outax();
	inline void _call_d16();
	inline void _jmp_d16();
	inline void _jmp_far();
	inline void _jmp_d8();
	inline void _inaldx();
	inline void _inaxdx();
	inline void _outdxal();
	inline void _outdxax();
	inline void _lock();
	inline void _rep(int flagval);
	inline void _repne();
	inline void _repe();
	inline void _hlt();
	inline void _cmc();
	inline void _f6pre();
	inline void _f7pre();
	inline void _clc();
	inline void _stc();
	inline void _cli();
	inline void _sti();
	inline void _cld();
	inline void _std();
	inline void _fepre();
	inline void _ffpre();
	inline void _invalid();
	
public:
	I86(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_bios = NULL;
#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
#endif
		busreq = false;
	}
	~I86() {}
	
	// common functions
	void initialize();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32 data, uint32 mask);
	void set_intr_line(bool line, bool pending, uint32 bit);
	void set_extra_clock(int clock)
	{
		extra_icount += clock;
	}
	int get_extra_clock()
	{
		return extra_icount;
	}
	uint32 get_pc()
	{
		return prevpc;
	}
	uint32 get_next_pc()
	{
		return pc;
	}
#ifdef USE_DEBUGGER
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32 debug_prog_addr_mask()
	{
		return 0xfffff;
	}
	uint32 debug_data_addr_mask()
	{
		return 0xfffff;
	}
	void debug_write_data8(uint32 addr, uint32 data);
	uint32 debug_read_data8(uint32 addr);
	void debug_write_data16(uint32 addr, uint32 data);
	uint32 debug_read_data16(uint32 addr);
	void debug_write_io8(uint32 addr, uint32 data);
	uint32 debug_read_io8(uint32 addr);
	void debug_write_io16(uint32 addr, uint32 data);
	uint32 debug_read_io16(uint32 addr);
	bool debug_write_reg(_TCHAR *reg, uint32 data);
	void debug_regs_info(_TCHAR *buffer);
	int debug_dasm(uint32 pc, _TCHAR *buffer);
#endif
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_context_intr(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_bios(DEVICE* device)
	{
		d_bios = device;
	}
#ifdef SINGLE_MODE_DMA
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
#endif
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif
