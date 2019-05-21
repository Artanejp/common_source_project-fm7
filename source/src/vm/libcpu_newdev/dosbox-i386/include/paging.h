/*
 *  Copyright (C) 2002-2015  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#ifndef DOSBOX_PAGING_H
#define DOSBOX_PAGING_H


// disable this to reduce the size of the TLB
// NOTE: does not work with the dynamic core (dynrec is fine)
#define USE_FULL_TLB

#include "../types_compat.h"

class I386_DOSBOX;
namespace I386_DOSBOX {

class PageDirectory;

#define MEM_PAGE_SIZE	(4096)
#define XMS_START		(0x110)

#if defined(USE_FULL_TLB)
#define TLB_SIZE		(1024*1024)
#else
#define TLB_SIZE		65536	// This must a power of 2 and greater then LINK_START
#define BANK_SHIFT		28
#define BANK_MASK		0xffff // always the same as TLB_SIZE-1?
#define TLB_BANKS		((1024*1024/TLB_SIZE)-1)
#endif

#define PFLAG_READABLE		0x1
#define PFLAG_WRITEABLE		0x2
#define PFLAG_HASROM		0x4
#define PFLAG_HASCODE		0x8				//Page contains dynamic code
#define PFLAG_NOCODE		0x10			//No dynamic code can be generated here
#define PFLAG_INIT			0x20			//No dynamic code can be generated here

#define LINK_START	((1024+64)/4)			//Start right after the HMA

//Allow 128 mb of memory to be linked
#define PAGING_LINKS (128*1024/4)

#define LINK_TOTAL		(64*1024)

#define USERWRITE_PROHIBITED			((cpu.cpl&cpu.mpl)==3)

struct PF_Entry {
	Bitu cs;
	Bitu eip;
	Bitu page_addr;
	Bitu mpl;
};

#define PF_QUEUESIZE 16
static struct {
	Bitu used;
	PF_Entry entries[PF_QUEUESIZE];
} pf_queue;

	
class PageHandler : public MinimumSkelton{
	I386_DOSBOX *d_parent;
public:
	PageHandler(DEVICE *parent) : MinimumSkelton(parent)
	{
		d_parent = static_cast<I386_DOSBOX *>(parent);
	}
	virtual ~PageHandler() { }
	virtual Bitu readb(PhysPt addr);
	virtual Bitu readw(PhysPt addr);
	virtual Bitu readd(PhysPt addr);
	virtual void writeb(PhysPt addr,Bitu val);
	virtual void writew(PhysPt addr,Bitu val);
	virtual void writed(PhysPt addr,Bitu val);
	virtual HostPt GetHostReadPt(Bitu phys_page);
	virtual HostPt GetHostWritePt(Bitu phys_page);
	virtual bool readb_checked(PhysPt addr,Bit8u * val);
	virtual bool readw_checked(PhysPt addr,Bit16u * val);
	virtual bool readd_checked(PhysPt addr,Bit32u * val);
	virtual bool writeb_checked(PhysPt addr,Bitu val);
	virtual bool writew_checked(PhysPt addr,Bitu val);
	virtual bool writed_checked(PhysPt addr,Bitu val);

	inline Bit8u phys_readb(Bit32u addr);
	inline Bit16u phys_readw(Bit32u addr);
	inline Bit32u phys_readd(Bit32u addr);
	
	inline void phys_writeb(Bit32u addr, Bit8u val);
	inline void phys_writew(Bit32u addr, Bit16u val);
	inline void phys_writed(Bit32u addr, Bit32u val);
	
	INLINE void InitPageUpdateLink(Bitu relink,PhysPt addr);
	INLINE bool InitPageCheckPresence_CheckOnly(PhysPt lin_addr,bool writing,X86PageEntry& table,X86PageEntry& entry);
	INLINE bool InitPage_CheckUseraccess(Bitu u1,Bitu u2);
	INLINE void InitPageCheckPresence(PhysPt lin_addr,bool writing,X86PageEntry& table,X86PageEntry& entry);

	PAGING_PageFault(PhysPt lin_addr,Bitu page_addr,Bitu faultcode);
	Bits PageFaultCore(void);
	Bitu flags;
};

inline Bit8u PageHandler::phys_readb(Bit32u addr)
{
	return d_parent->read_data8(addr);
}
inline Bit16u PageHandler::phys_readw(Bit32u addr)
{
	return d_parent->read_data16(addr);
}

inline Bit32u PageHandler::phys_readd(Bit32u addr)
{
	return d_parent->read_data32(addr);
}
	
inline void PageHandler::phys_writeb(Bit32u addr, Bit32u val)
{
	d_parent->write_data8(addr, val);
}

inline void PageHandler::phys_writew(Bit32u addr, Bit32u val)
{
	d_parent->write_data16(addr, val);
}

inline void PageHandler::phys_writed(Bit32u addr, Bit32u val)
{
	d_parent->write_data32(addr, val);
}
/* Some other functions */
//void PAGING_Enable(bool enabled);
//bool PAGING_Enabled(void);

//Bitu PAGING_GetDirBase(void);
//void PAGING_SetDirBase(Bitu cr3);
//void PAGING_InitTLB(void);
//void PAGING_ClearTLB(void);

//void PAGING_LinkPage(Bitu lin_page,Bitu phys_page);
//void PAGING_LinkPage_ReadOnly(Bitu lin_page,Bitu phys_page);
//void PAGING_UnlinkPages(Bitu lin_page,Bitu pages);
/* This maps the page directly, only use when paging is disabled */
//void PAGING_MapPage(Bitu lin_page,Bitu phys_page);
//bool PAGING_MakePhysPage(Bitu & page);
//bool PAGING_ForcePageInit(Bitu lin_addr);

//void MEM_SetLFB(Bitu page, Bitu pages, PageHandler *handler, PageHandler *mmiohandler);
//void MEM_SetPageHandler(Bitu phys_page, Bitu pages, PageHandler * handler);
//void MEM_ResetPageHandler(Bitu phys_page, Bitu pages);


#ifdef _MSC_VER
#pragma pack (1)
#endif
struct X86_PageEntryBlock{
#ifdef WORDS_BIGENDIAN
	Bit32u		base:20;
	Bit32u		avl:3;
	Bit32u		g:1;
	Bit32u		pat:1;
	Bit32u		d:1;
	Bit32u		a:1;
	Bit32u		pcd:1;
	Bit32u		pwt:1;
	Bit32u		us:1;
	Bit32u		wr:1;
	Bit32u		p:1;
#else
	Bit32u		p:1;
	Bit32u		wr:1;
	Bit32u		us:1;
	Bit32u		pwt:1;
	Bit32u		pcd:1;
	Bit32u		a:1;
	Bit32u		d:1;
	Bit32u		pat:1;
	Bit32u		g:1;
	Bit32u		avl:3;
	Bit32u		base:20;
#endif
} GCC_ATTRIBUTE(packed);
#ifdef _MSC_VER
#pragma pack ()
#endif


union X86PageEntry {
	Bit32u load;
	X86_PageEntryBlock block;
};

#if !defined(USE_FULL_TLB)
typedef struct {
	HostPt read;
	HostPt write;
	PageHandler * readhandler;
	PageHandler * writehandler;
	Bit32u phys_page;
} tlb_entry;
#endif

struct PagingBlock {
	Bitu			cr3;
	Bitu			cr2;
	struct {
		Bitu page;
		PhysPt addr;
	} base;
#if defined(USE_FULL_TLB)
	struct {
		HostPt read[TLB_SIZE];
		HostPt write[TLB_SIZE];
		PageHandler * readhandler[TLB_SIZE];
		PageHandler * writehandler[TLB_SIZE];
		Bit32u	phys_page[TLB_SIZE];
	} tlb;
#else
	tlb_entry tlbh[TLB_SIZE];
	tlb_entry *tlbh_banks[TLB_BANKS];
#endif
	struct {
		Bitu used;
		Bit32u entries[PAGING_LINKS];
	} links;
	Bit32u		firstmb[LINK_START];
	bool		enabled;
};

//extern PagingBlock paging; 

/* Some support functions */

//PageHandler * MEM_GetPageHandler(Bitu phys_page);


/* Unaligned address handlers */
//Bit16u mem_unalignedreadw(PhysPt address);
//Bit32u mem_unalignedreadd(PhysPt address);
//void mem_unalignedwritew(PhysPt address,Bit16u val);
//void mem_unalignedwrited(PhysPt address,Bit32u val);

//bool mem_unalignedreadw_checked(PhysPt address,Bit16u * val);
//bool mem_unalignedreadd_checked(PhysPt address,Bit32u * val);
//bool mem_unalignedwritew_checked(PhysPt address,Bit16u val);
//bool mem_unalignedwrited_checked(PhysPt address,Bit32u val);


};
#endif
