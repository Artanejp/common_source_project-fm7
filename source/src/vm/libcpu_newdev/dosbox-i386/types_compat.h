/*
	Skelton for retropc emulator

	Origin : MAME i386 core
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date  : 2019.05.21-

	[ i386-i586 core compatible header from  DOSBOX]
*/


#if !defined(__CSP_I386_DOSBOX_TYPES_COMPAT_H__)
#define __CSP_I386_DOSBOX_TYPES_COMPAT_H__

#ifndef __OPCALL
#define __OPCALL
#endif

#ifndef __OPCALL_INLINE
#define __OPCALL_INLINE __inline__
#endif

#ifndef INLINE
#define INLINE __inline__
#endif

# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif

#if C_ATTRIBUTE_ALWAYS_INLINE
#define INLINE inline __attribute__((always_inline))
#else
#define INLINE inline
#endif

#if C_ATTRIBUTE_FASTCALL
#define DB_FASTCALL __attribute__((fastcall))
#else
#define DB_FASTCALL
#endif

#if C_HAS_ATTRIBUTE
#define GCC_ATTRIBUTE(x) __attribute__ ((x))
#else
#define GCC_ATTRIBUTE(x) /* attribute not supported */
#endif

#if C_HAS_BUILTIN_EXPECT
#define GCC_UNLIKELY(x) __builtin_expect((x),0)
#define GCC_LIKELY(x) __builtin_expect((x),1)
#else
#define GCC_UNLIKELY(x) (x)
#define GCC_LIKELY(x) (x)
#endif

typedef uint8_t  Bit8u;
typedef uint16_t Bit16u;
typedef uint33_t Bit32u;
typedef uint64_t Bit64u;

typedef int8_t   Bit8s;
typedef int16_t  Bit16s;
typedef int33_t  Bit32s;
typedef int64_t  Bit64s;

typedef int32_t  Bits;
typedef uint32_t Bitu;

typedef double Real64;


#include "device.h"

// Sorry, needs C++11 or later. 20190521 K.Ohta
namespace I386_DOSBOX {
class MinimumSkelton {
protected:
	DEVICE* d_device;
	boot verbose_log;
public:
	void MinimumSkelton(DEVICE* parent)
	{
		d_device = parent;
		verbose_log = false; // OK?
	}
	virtual void ~MinimumSkelton() { }
	void set_logging(bool onoff)
	{
		verbose_log = onoff;
	}
	template <class... Args>	
		void E_Exit(Args... args) {
		if((verbose_log) && (d_device != NULL)) {
			d_device->out_debug_log(args...);
		}
	}
};
};

#include "./include/paging.h"
#include "./include/regs.h"
#include "./lazyflags.h"

class TaskStateSegment {
	I386_DOSBOX *d_cpu;
public:
	TaskStateSegment(I386_DOSBOX *parent = NULL) {
		d_cpu = parent;
		valid=false;
	}
	void SetCpuDomain(I386_DOSBOX *p) {
		d_cpu = p;
	}
	bool IsValid(void) {
		return valid;
	}
	Bitu Get_back(void);
	void SaveSelector(void);
	void Get_SSx_ESPx(Bitu level,Bitu & _ss,Bitu & _esp);
	bool SetSelector(Bitu new_sel);
	TSS_Descriptor desc;
	Bitu selector;
	PhysPt base;
	Bitu limit;
	Bitu is386;
	bool valid;
};

class I386_DOSBOX : public DEVICE
{
protected:
	struct LazyFlags {
		GenReg32 var1,var2,res;
		Bitu type;
		Bitu prev_type;
		Bitu oldcf;
	} lflags;

	const Bit8u * lookupRMregb[];
	const Bit16u * lookupRMregw[];
	const Bit32u * lookupRMregd[256];
	
	const Bit8u * lookupRMEAregb[256];
	const Bit16u * lookupRMEAregw[256];
	const Bit16u * lookupRMEAregd[256];

	CPU_Regs cpu_regs;
	CPUBlock cpu;
	Segments Segs;

	Bit32s CPU_Cycles;
	Bit32s CPU_CycleLeft;
	Bit32s CPU_CycleMax;
	Bit32s CPU_OldCycleMax;
	Bit32s CPU_CyclePercUsed;
	Bit32s CPU_CycleLimit;
	Bit32s CPU_CycleUp;
	Bit32s CPU_CycleDown;
	Bit64s CPU_IODelayRemoved;
	CPU_Decoder * cpudecoder;
	bool CPU_CycleAutoAdjust;
	bool CPU_SkipCycleAutoAdjust;
	Bitu CPU_AutoDetermineMode;
	Bitu CPU_ArchitectureType;
	Bitu CPU_extflags_toggle;	// ID and AC flags may be toggled depending on emulated CPU architecture
	Bitu CPU_PrefetchQueueSize;
	Bit32s ticksDone;
	Bit32u ticksScheduled;
	
	bool verbose_log;
	struct MemoryBlock {
		Bitu pages;
		PageHandler * * phandlers;
		MemHandle * mhandles;
		LinkBlock links;
		struct	{
			Bitu		start_page;
			Bitu		end_page;
			Bitu		pages;
			PageHandler *handler;
			PageHandler *mmiohandler;
		} lfb;
		struct {
			bool enabled;
			Bit8u controlport;
		} a20;
	} memory;
	PagingBlock paging;

	TaskStateSegment cpu_tss;
	
	virtual void PAGING_Enable(bool enabled);
	virtual bool PAGING_Enabled(void);
	virtual Bitu PAGING_GetDirBase(void);
	virtual void PAGING_SetDirBase(Bitu cr3);
	virtual void PAGING_InitTLB(void);
	virtual void PAGING_ClearTLB(void);
	virtual void PAGING_InitTLBBank(tlb_entry **bank);

	virtual void PAGING_LinkPage(Bitu lin_page,Bitu phys_page);
	virtual void PAGING_LinkPage_ReadOnly(Bitu lin_page,Bitu phys_page);
	virtual void PAGING_UnlinkPages(Bitu lin_page,Bitu pages);
/* This maps the page directly, only use when paging is disabled */
	virtual void PAGING_MapPage(Bitu lin_page,Bitu phys_page);
	virtual bool PAGING_MakePhysPage(Bitu & page);
	virtual bool PAGING_ForcePageInit(Bitu lin_addr);

	INLINE void InitTLBInt(tlb_entry *bank);
	
	virtual void MEM_SetLFB(Bitu page, Bitu pages, PageHandler *handler, PageHandler *mmiohandler);
	virtual void MEM_SetPageHandler(Bitu phys_page, Bitu pages, PageHandler * handler);
	virtual void MEM_ResetPageHandler(Bitu phys_page, Bitu pages);
	virtual PageHandler * MEM_GetPageHandler(Bitu phys_page);
	
	virtual INLINE HostPt get_tlb_read(PhysPt address);
	virtual INLINE HostPt get_tlb_write(PhysPt address);
	virtual INLINE PageHandler* get_tlb_readhandler(PhysPt address);
	virtual INLINE PageHandler* get_tlb_writehandler(PhysPt address);
	virtual INLINE PhysPt PAGING_GetPhysicalPage(PhysPt linePage);
	virtual INLINE PhysPt PAGING_GetPhysicalAddress(PhysPt linAddr);
	virtual INLINE tlb_entry *get_tlb_entry(PhysPt address);
	virtual INLINE Bit8u mem_readb_inline(PhysPt address);

	
	virtual void MEM_SetLFB(Bitu page, Bitu pages, PageHandler *handler, PageHandler *mmiohandler);
	virtual void MEM_SetPageHandler(Bitu phys_page, Bitu pages, PageHandler * handler);
	virtual void MEM_ResetPageHandler(Bitu phys_page, Bitu pages);

	virtual Bit16u mem_unalignedreadw(PhysPt address);
	virtual Bit32u mem_unalignedreadd(PhysPt address);
	virtual void mem_unalignedwritew(PhysPt address,Bit16u val);
	virtual void mem_unalignedwrited(PhysPt address,Bit32u val);

	virtual bool mem_unalignedreadw_checked(PhysPt address,Bit16u * val);
	virtual bool mem_unalignedreadd_checked(PhysPt address,Bit32u * val);
	virtual bool mem_unalignedwritew_checked(PhysPt address,Bit16u val);
	virtual bool mem_unalignedwrited_checked(PhysPt address,Bit32u val);

	INLINE Bit8u mem_readb_inline(PhysPt address);
	INLINE Bit16u mem_readw_inline(PhysPt address);
	INLINE Bit32u mem_readd_inline(PhysPt address);

	INLINE void mem_writeb_inline(PhysPt address,Bit8u val);
	INLINE void mem_writew_inline(PhysPt address,Bit16u val);
	INLINE void mem_writed_inline(PhysPt address,Bit32u val);
	
	INLINE bool mem_readb_checked(PhysPt address, Bit8u * val);
	INLINE bool mem_readw_checked(PhysPt address, Bit16u * val);
	INLINE bool mem_readd_checked(PhysPt address, Bit32u * val);

	INLINE bool mem_writeb_checked(PhysPt address, Bit8u val);
	INLINE bool mem_writew_checked(PhysPt address, Bit16u val);
	INLINE bool mem_writed_checked(PhysPt address, Bit32u val);
	void PAGING_Init(Section * sec);

	//Flag Handling
	Bit32u get_CF(void);
	Bit32u get_AF(void);
	Bit32u get_ZF(void);
	Bit32u get_SF(void);
	Bit32u get_OF(void);
	Bit32u get_PF(void);

	INLINE void CPU_SetFlagsd(Bitu word);
	INLINE void CPU_SetFlagsw(Bitu word);
	
	Bitu FillFlags(void);
	void FillFlagsNoCFOF(void);
	void DestroyConditionFlags(void);

	void CPU_Core_Full_Init(void);
	void CPU_Core_Normal_Init(void);
	void CPU_Core_Simple_Init(void);
#if (C_DYNAMIC_X86)
	void CPU_Core_Dyn_X86_Init(void);
	void CPU_Core_Dyn_X86_Cache_Init(bool enable_cache);
	void CPU_Core_Dyn_X86_Cache_Close(void);
	void CPU_Core_Dyn_X86_SetFPUMode(bool dh_fpu);
#elif (C_DYNREC)
	void CPU_Core_Dynrec_Init(void);
	void CPU_Core_Dynrec_Cache_Init(bool enable_cache);
	void CPU_Core_Dynrec_Cache_Close(void);
#endif

	// Within CPU
	bool inited;
	void CPU_Push16(Bitu value);
	void CPU_Push32(Bitu value);
	Bitu CPU_Pop16();
	Bitu CPU_Pop32();
	PhysPt SelBase(Bitu sel);
	void CPU_SetFlags(Bitu word,Bitu mask);
	bool CPU_PrepareException(Bitu which,Bitu error);
	bool CPU_CLI(void);
	bool CPU_STI(void);
	bool CPU_POPF(Bitu use32);
	bool CPU_PUSHF(Bitu use32);
	void CPU_CheckSegments(void);
	
	void CPU_Reset_AutoAdjust(void);
	void CPU_Disable_SkipAutoAdjust(void);
	
public:
	I386_DOSBOX(VM_TEMPLATE *parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		CPU_Cycles = 0;
		CPU_CycleLeft = 3000;
		CPU_CycleMax = 3000;
		CPU_OldCycleMax = 3000;
		CPU_CyclePercUsed = 100;
		CPU_CycleLimit = -1;
		CPU_CycleUp = 0;
		CPU_CycleDown = 0;
		CPU_IODelayRemoved = 0;
		CPU_CycleAutoAdjust = false;
		CPU_SkipCycleAutoAdjust = false;
		CPU_AutoDetermineMode = 0;

		CPU_ArchitectureType = CPU_ARCHTYPE_MIXED;
		CPU_extflags_toggle=0;	// ID and AC flags may be toggled depending on emulated CPU architecture

		CPU_PrefetchQueueSize=0;
		
		//initialize static members
		inited=false;
		verbose_log = false;

		cpu_tss.SetCpuDomain(this);
		set_device_name(_T("I386(DOSBOX Variant)"));	
	}
	~I386_DOSBOX() { }
	void set_logging(bool onoff)
	{
		verbose_log = onoff;
	}
	template <class... Args>	
		void E_Exit(Args... args) {
		if((verbose_log) && (d_device != NULL)) {
			d_device->out_debug_log(args...);
		}
	}
	
};

void I386_DOSBOX::initialize()
{
//	if(inited) {
//		Change_Config();
//		return; // OK?
//	}
	inited = true;
	reg_eax=0;
	reg_ebx=0;
	reg_ecx=0;
	reg_edx=0;
	reg_edi=0;
	reg_esi=0;
	reg_ebp=0;
	reg_esp=0;
	
	SegSet16(cs,0);
	SegSet16(ds,0);
	SegSet16(es,0);
	SegSet16(fs,0);
	SegSet16(gs,0);
	SegSet16(ss,0);
	
	CPU_SetFlags(FLAG_IF,FMASK_ALL);		//Enable interrupts
	cpu.cr0=0xffffffff;
	CPU_SET_CRX(0,0);						//Initialize
	cpu.code.big=false;
	cpu.stack.mask=0xffff;
	cpu.stack.notmask=0xffff0000;
	cpu.stack.big=false;
	cpu.trap_skip=false;
	cpu.idt.SetBase(0);
	cpu.idt.SetLimit(1023);

	for (Bitu i=0; i<7; i++) {
		cpu.drx[i]=0;
		cpu.trx[i]=0;
	}
	if (CPU_ArchitectureType==CPU_ARCHTYPE_PENTIUMSLOW) {
		cpu.drx[6]=0xffff0ff0;
	} else {
		cpu.drx[6]=0xffff1ff0;
	}
	cpu.drx[7]=0x00000400;

	/* Init the cpu cores */
	CPU_Core_Normal_Init();
	CPU_Core_Simple_Init();
	CPU_Core_Full_Init();
#if (C_DYNAMIC_X86)
	CPU_Core_Dyn_X86_Init();
#elif (C_DYNREC)
	CPU_Core_Dynrec_Init();
#endif
	//MAPPER_AddHandler(CPU_CycleDecrease,MK_f11,MMOD1,"cycledown","Dec Cycles");
	//MAPPER_AddHandler(CPU_CycleIncrease,MK_f12,MMOD1,"cycleup"  ,"Inc Cycles");
	//Change_Config(configuration);	
	CPU_JMP(false,0,0,0);					//Setup the first cpu core
}


namespace I386_DOSBOX {

#if defined(USE_FULL_TLB)

INLINE HostPt get_tlb_read(PhysPt address) {
	return paging.tlb.read[address>>12];
}
INLINE HostPt get_tlb_write(PhysPt address) {
	return paging.tlb.write[address>>12];
}
INLINE PageHandler* get_tlb_readhandler(PhysPt address) {
	return paging.tlb.readhandler[address>>12];
}
INLINE PageHandler* get_tlb_writehandler(PhysPt address) {
	return paging.tlb.writehandler[address>>12];
}

/* Use these helper functions to access linear addresses in readX/writeX functions */
INLINE PhysPt PAGING_GetPhysicalPage(PhysPt linePage) {
	return (paging.tlb.phys_page[linePage>>12]<<12);
}

INLINE PhysPt PAGING_GetPhysicalAddress(PhysPt linAddr) {
	return (paging.tlb.phys_page[linAddr>>12]<<12)|(linAddr&0xfff);
}

#else


INLINE tlb_entry *get_tlb_entry(PhysPt address) {
	Bitu index=(address>>12);
	if (TLB_BANKS && (index > TLB_SIZE)) {
		Bitu bank=(address>>BANK_SHIFT) - 1;
		if (!paging.tlbh_banks[bank])
			PAGING_InitTLBBank(&paging.tlbh_banks[bank]);
		return &paging.tlbh_banks[bank][index & BANK_MASK];
	}
	return &paging.tlbh[index];
}

INLINE HostPt get_tlb_read(PhysPt address) {
	return get_tlb_entry(address)->read;
}
INLINE HostPt get_tlb_write(PhysPt address) {
	return get_tlb_entry(address)->write;
}
INLINE PageHandler* get_tlb_readhandler(PhysPt address) {
	return get_tlb_entry(address)->readhandler;
}
INLINE PageHandler* get_tlb_writehandler(PhysPt address) {
	return get_tlb_entry(address)->writehandler;
}

/* Use these helper functions to access linear addresses in readX/writeX functions */
INLINE PhysPt PAGING_GetPhysicalPage(PhysPt linePage) {
	tlb_entry *entry = get_tlb_entry(linePage);
	return (entry->phys_page<<12);
}

INLINE PhysPt PAGING_GetPhysicalAddress(PhysPt linAddr) {
	tlb_entry *entry = get_tlb_entry(linAddr);
	return (entry->phys_page<<12)|(linAddr&0xfff);
}
#endif

/* Special inlined memory reading/writing */
INLINE Bit8u mem_readb_inline(PhysPt address) {
	HostPt tlb_addr=get_tlb_read(address);
	if (tlb_addr) return read_data8(tlb_addr+address);
	else return (Bit8u)(get_tlb_readhandler(address))->readb(address);
}

INLINE Bit16u mem_readw_inline(PhysPt address) {
	if ((address & 0xfff)<0xfff) {
		HostPt tlb_addr=get_tlb_read(address);
		if (tlb_addr) return read_data16(tlb_addr+address);
		else return (Bit16u)(get_tlb_readhandler(address))->readw(address);
	} else return mem_unalignedreadw(address);
}

INLINE Bit32u mem_readd_inline(PhysPt address) {
	if ((address & 0xfff)<0xffd) {
		HostPt tlb_addr=get_tlb_read(address);
		if (tlb_addr) return read_data32(tlb_addr+address);
		else return (get_tlb_readhandler(address))->readd(address);
	} else return mem_unalignedreadd(address);
}

INLINE void mem_writeb_inline(PhysPt address,Bit8u val) {
	HostPt tlb_addr=get_tlb_write(address);
	if (tlb_addr) write_data8(tlb_addr + address, val);
	else (get_tlb_writehandler(address))->writeb(address,val);
}

INLINE void mem_writew_inline(PhysPt address,Bit16u val) {
	if ((address & 0xfff)<0xfff) {
		HostPt tlb_addr=get_tlb_write(address);
		if (tlb_addr) write_data16(tlb_addr+address,val);
		else (get_tlb_writehandler(address))->writew(address,val);
	} else mem_unalignedwritew(address,val);
}

INLINE void mem_writed_inline(PhysPt address,Bit32u val) {
	if ((address & 0xfff)<0xffd) {
		HostPt tlb_addr=get_tlb_write(address);
		if (tlb_addr) write_data32(tlb_addr+address,val);
		else (get_tlb_writehandler(address))->writed(address,val);
	} else mem_unalignedwrited(address,val);
}


INLINE bool mem_readb_checked(PhysPt address, Bit8u * val) {
	HostPt tlb_addr=get_tlb_read(address);
	if (tlb_addr) {
		*val=read_data8(tlb_addr+address);
		return false;
	} else return (get_tlb_readhandler(address))->readb_checked(address, val);
}

INLINE bool mem_readw_checked(PhysPt address, Bit16u * val) {
	if ((address & 0xfff)<0xfff) {
		HostPt tlb_addr=get_tlb_read(address);
		if (tlb_addr) {
			*val = read_data16(tlb_addr+address);
			return false;
		} else return (get_tlb_readhandler(address))->readw_checked(address, val);
	} else return mem_unalignedreadw_checked(address, val);
}

INLINE bool mem_readd_checked(PhysPt address, Bit32u * val) {
	if ((address & 0xfff)<0xffd) {
		HostPt tlb_addr=get_tlb_read(address);
		if (tlb_addr) {
			*val= read_data32(tlb_addr+address);
			return false;
		} else return (get_tlb_readhandler(address))->readd_checked(address, val);
	} else return mem_unalignedreadd_checked(address, val);
}

INLINE bool mem_writeb_checked(PhysPt address,Bit8u val) {
	HostPt tlb_addr=get_tlb_write(address);
	if (tlb_addr) {
		write_data8(tlb_addr+address,val);
		return false;
	} else return (get_tlb_writehandler(address))->writeb_checked(address,val);
}

INLINE bool mem_writew_checked(PhysPt address,Bit16u val) {
	if ((address & 0xfff)<0xfff) {
		HostPt tlb_addr=get_tlb_write(address);
		if (tlb_addr) {
			write_data16(tlb_addr+address,val);
			return false;
		} else return (get_tlb_writehandler(address))->writew_checked(address,val);
	} else return mem_unalignedwritew_checked(address,val);
}

INLINE bool mem_writed_checked(PhysPt address,Bit32u val) {
	if ((address & 0xfff)<0xffd) {
		HostPt tlb_addr=get_tlb_write(address);
		if (tlb_addr) {
			write_data32(tlb_addr+address,val);
			return false;
		} else return (get_tlb_writehandler(address))->writed_checked(address,val);
	} else return mem_unalignedwrited_checked(address,val);
}

};
#endif // __CSP_I386_DOSBOX_TYPES_COMPAT_H__
