#pragma once


// From np2,, ia32/system_inst.c
// Descriptions of CRx:
/*
 * CR0: From https://en.wikipedia.org/wiki/Control_register
 * 0 	PE 	Protected Mode Enable 	If 1, system is in protected mode, else system is in real mode
 * 1 	MP 	Monitor co-processor 	Controls interaction of WAIT/FWAIT instructions with TS flag in CR0
 * 2 	EM 	Emulation 	If set, no x87 floating-point unit present, if clear, x87 FPU present
 * 3 	TS 	Task switched 	Allows saving x87 task context upon a task switch only after x87 instruction used
 * 4 	ET 	Extension type 	On the 386, it allowed to specify whether the external math coprocessor was an 80287 or 80387
 * 5 	NE 	Numeric error 	Enable internal x87 floating point error reporting when set, else enables PC style x87 error detection
 * 16 	WP 	Write protect 	When set, the CPU can't write to read-only pages when privilege level is 0
 * 18 	AM 	Alignment mask 	Alignment check enabled if AM set, AC flag (in EFLAGS register) set, and privilege level is 3
 * 29 	NW 	Not-write through 	Globally enables/disable write-through caching
 * 30 	CD 	Cache disable 	Globally enables/disable the memory cache
 * 31 	PG 	Paging 	If 1, enable paging and use the § CR3 register, else disable paging. 
 */
#define I386_CR0_PE (1 << 0)
#define I386_CR0_MP (1 << 1)
#define I386_CR0_EM (1 << 2)
#define I386_CR0_TS (1 << 3)
#define I386_CR0_ET (1 << 4)
#define I386_CR0_NE (1 << 5)
#define I386_CR0_WP (1 << 16)
#define I386_CR0_AM (1 << 18)
#define I386_CR0_NW (1 << 29)
#define I386_CR0_CD (1 << 30)
#define I386_CR0_PG (1 << 31)
#define	I386_CR0_ALL		(I386_CR0_PE|I386_CR0_MP|I386_CR0_EM|I386_CR0_TS|I386_CR0_ET|I386_CR0_NE|I386_CR0_WP|I386_CR0_AM|I386_CR0_NW|I386_CR0_CD|I386_CR0_PG)

#define	I386_CR3_PD_MASK 0xfffff000
#define I386_CR3_PCID    0x00000fff
#define	I386_CR3_PWT	 (1 << 3)
#define	I386_CR3_PCD	 (1 << 4)
#define	I386_CR3_MASK	 (I386_CR3_PD_MASK|I386_CR3_PWT|I386_CR3_PCD)

/*
 * CR4: From https://en.wikipedia.org/wiki/Control_register#CR4
 * 0 VME 	Virtual 8086 Mode Extensions 	If set, enables support for the virtual interrupt flag (VIF) in virtual-8086 mode.
 * 1 PVI 	Protected-mode Virtual Interrupts 	If set, enables support for the virtual interrupt flag (VIF) in protected mode.
 * 2 TSD 	Time Stamp Disable 	If set, RDTSC instruction can only be executed when in ring 0, otherwise RDTSC can be used at any privilege level.
 * 3 DE 	Debugging Extensions 	If set, enables debug register based breaks on I/O space access.
 * 4 PSE 	Page Size Extension 	If unset, page size is 4 KiB, else page size is increased to 4 MiB If PAE is enabled or the processor is in x86-64 long mode this bit is ignored.[2]
 * 5 PAE 	Physical Address Extension 	If set, changes page table layout to translate 32-bit virtual addresses into extended 36-bit physical addresses.
 * 6 MCE 	Machine Check Exception 	If set, enables machine check interrupts to occur.
 * 7 PGE 	Page Global Enabled 	If set, address translations (PDE or PTE records) may be shared between address spaces.
 * 8 PCE 	Performance-Monitoring Counter enable 	If set, RDPMC can be executed at any privilege level, else RDPMC can only be used in ring 0.
 * 9 OSFXSR 	Operating system support for FXSAVE and FXRSTOR instructions 	If set, enables Streaming SIMD Extensions (SSE) instructions and fast FPU save & restore.
 * 10 OSXMMEXCPT 	Operating System Support for Unmasked SIMD Floating-Point Exceptions 	If set, enables unmasked SSE exceptions.
 * 11 UMIP 	User-Mode Instruction Prevention 	If set, the SGDT, SIDT, SLDT, SMSW and STR instructions cannot be executed if CPL > 0.[1]
 * 12 LA57 	(none specified) 	If set, enables 5-Level Paging.[3]
 * 13 VMXE 	Virtual Machine Extensions Enable 	see Intel VT-x x86 virtualization.
 * 14 SMXE 	Safer Mode Extensions Enable 	see Trusted Execution Technology (TXT)
 * 16 FSGSBASE 	Enables the instructions RDFSBASE, RDGSBASE, WRFSBASE, and WRGSBASE.
 * 17 PCIDE 	PCID Enable 	If set, enables process-context identifiers (PCIDs).
 * 18 OSXSAVE 	XSAVE and Processor Extended States Enable 	
 * 20 SMEP 	Supervisor Mode Execution Protection Enable 	If set, execution of code in a higher ring generates a fault.
 * 21 SMAP 	Supervisor Mode Access Prevention Enable 	If set, access of data in a higher ring generates a fault.[5]
 * 22 PKE Protection Key Enable 	See Intel 64 and IA-32 Architectures Software Developer’s Manual. 
*/
#define I386_CR4_VME        (1 << 0)
#define I386_CR4_PVI        (1 << 1)
#define I386_CR4_TSD        (1 << 2)
#define I386_CR4_DE         (1 << 3)
#define I386_CR4_PSE        (1 << 4)
#define I386_CR4_PAE        (1 << 5)
#define I386_CR4_MCE        (1 << 6)
#define I386_CR4_PGE        (1 << 7)
#define I386_CR4_PCE        (1 << 8)
#define I386_CR4_OSFXSR     (1 << 9)
#define I386_CR4_OSXMMEXCPT (1 << 10)
#define I386_CR4_UMIP       (1 << 11)
#define I386_CR4_LA57       (1 << 12)
#define I386_CR4_VMXE       (1 << 13)
#define I386_CR4_SMXE       (1 << 14)
// bit15: ??
#define I386_CR4_FSGBASE    (1 << 16)
#define I386_CR4_PCIDE      (1 << 17)
#define I386_CR4_OSXSAVE    (1 << 18)
// bit19: ??
#define I386_CR4_SMEP       (1 << 20)
#define I386_CR4_SMAP       (1 << 21)
#define I386_CR4_PKE        (1 << 22)

#define I386_CR4_SET_MASK1  ~(I386_CR4_VME | I386_CR4_PVI |  I386_CR4_TSD |  I386_CR4_DE |  I386_CR4_PSE | I386_CR4_PAE | I386_CR4_MCE | I386_CR4_PGE | I386_CR4_PCE |  I386_CR4_OSFXSR)

#define	I386_DR6_B(r)		(1 << (r))
#define	I386_DR6_BD		(1 << 13)
#define	I386_DR6_BS		(1 << 14)
#define	I386_DR6_BT		(1 << 15)

#define	I386_DR7_L(r)		(1 << ((r) * 2))
#define	I386_DR7_G(r)		(1 << ((r) * 2 + 1))
#define	I386_DR7_LE		(1 << 8)
#define	I386_DR7_GE		(1 << 9)
#define	I386_DR7_GD		(1 << 13)
#define	I386_DR7_RW(r)		(3 << ((r) * 4 + 16))
#define	I386_DR7_LEN(r)		(3 << ((r) * 4 + 16 + 2))
