/*
 * FM-7 Main I/O [fm7_mainio.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jan 03, 2015 : Initial
 *
 */

#ifndef _VM_FM7_MAINIO_H_
#define _VM_FM7_MAINIO_H_

#include "../device.h"
#include "../memory.h"
#include "../mc6809.h"
#include "../z80.h"
#include "../ym2203.h"

#include "fm7_common.h"


class FM7_MAINIO : public DEVICE {
 private:
	bool opn_psg_77av;
	bool beep_flag;
	bool beep_snd;
	int event_beep;  
	int event_beep_oneshot;  
	int event_timerirq;  
 protected:
	VM* p_vm;
	EMU* p_emu;

	int nmi_count;
	bool irqstat_bak;
	bool firqstat_bak;
	uint8 io_w_latch[0x100];
   
	/* FD00: R */
	bool clock_fast; // bit0 : maybe dummy
	/* FD00: W */
	bool lpt_strobe;  // bit6 : maybe dummy entry
	bool lpt_slctin;  // bit7 : maybe dummy entry
	bool key_irq_req;
	bool key_irq_bak;
	/* FD01: W */
	uint8 lpt_outdata; // maybe dummy.

	/* FD02 : R */
	bool cmt_indat; // bit7
	bool cmt_invert; // Invert signal
	bool lpt_det2; // bit5 : maybe dummy.
	bool lpt_det1; // bit4 : maybe dummy.
	bool lpt_pe;   // bit3 : maybe dummy.
	bool lpt_ackng_inv; // bit2 : maybe dummy.
	bool lpt_error_inv; // bit1 : maybe dummy.
	bool lpt_busy; // bit0 : maybe dummy.
	/* FD02 : W */
	uint8 irqmask_reg0; // bit7-4, bit2-0 , '1' is enable.  '0' is disable.
	// 7-4 :  RS232C / SYNDET,RXRDY,TXRDY,MFD
	// 2 : TIMER
	// 1 : PRINTER
	// 0 : KEYBOARD
	bool irqmask_mfd; // bit4: "0" = mask.
	bool irqmask_timer; // bit2: "0" = mask.
	bool irqmask_printer; // bit1: "0" = mask.
	bool irqmask_keyboard; // bit0: "0" = mask.
  
	/* FD03: R */
	uint8 irqstat_reg0; // bit 3-0, '0' is happened, '1' is not happened.
	// bit3 : extended interrupt
	// bit2-0: Same as FD02 : W .
	/* FD03 : W , '1' = ON*/
	bool irqstat_timer;
	bool irqstat_printer;
	bool irqstat_keyboard;
   

	/* FD04 : R */
	bool stat_fdmode_2hd; //  R/W : bit6, '0' = 2HD, '1' = 2DD. FM-77 Only.
	bool stat_kanjirom;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
	bool stat_400linecard;//  R/W : bit4, '0' = connected. FM-77 Only.
	bool stat_400linemode; // R/W : bit3, '0' = 400line, '1' = 200line.
	bool firq_break_key; // bit1, ON = '0'.
	bool firq_sub_attention; // bit0, ON = '0'.
	bool firq_sub_attention_bak; // bit0, ON = '0'.
	/* FD04 : W */
	bool intmode_fdc; // bit2, '0' = normal, '1' = SFD.

	/* FD05 : R */
	bool extdet_neg; // bit0 : '1' = none , '0' = exists.
	bool sub_busy;
	/* FD05 : W */
	bool sub_halt; // bit7 : '1' Halt req.
	bool sub_cancel; // bit6 : '1' Cancel req.
	bool sub_halt_bak; // bit7 : shadow.
	bool sub_cancel_bak; // bit6 : shadow.
	bool z80_sel;    // bit0 : '1' = Z80. Maybe only FM-7/77.

	/* FD06 : R/W : RS-232C */
	/* FD07 : R/W : RS-232C */
	bool intstat_syndet;
	bool intstat_rxrdy;
	bool intstat_txrdy;
	/* FD08 : Grafic pen, not implemented */
	/* FD09 : Grafic pen, not implemented */
	/* FD0A : Grafic pen, not implemented */
	/* FD0B : R */
	bool stat_bootsw_basic; // bit0 : '0' = BASIC '1' = DOS. Only 77AV/20/40.
	uint32 bootmode;
	/* FD0D : W */
	uint8 psg_cmdreg; // PSG Register, Only bit 0-1 at FM-7/77 , 3-0 at FM-77AV series. Maybe dummy.

	/* FD0E : R */
	uint8 psg_statreg; // PSG data. maybe dummy.
	uint32 psg_address;
	uint32 psg_data;
	bool  psg_bus_high; // true when bus = high inpedance.
  
	/* FD0F : R/W */
	bool stat_romrammode; // R(true) = ROM, W(false) = RAM.
#if defined(_FM77AV_VARIANTS)
	/* FD12 : R/W*/
	bool mode320; // bit6 : true = 320, false = 640
	/* FD13 : WO */
	uint8 sub_monitor_type; // bit 2 - 0: default = 0.
	uint8 sub_monitor_bak; // bit 2 - 0: default = 0.
#endif
	
	/* FD15 / FD46 / FD51 : W */
	bool connect_opn; // [0]
	bool connect_whg; // [1]
	bool connect_thg; // [2]
	bool psg_shared_opn;
	uint32 opn_address[4];
	uint32 opn_data[4];
	uint32 opn_stat[4];
	uint8  opn_cmdreg[4]; // OPN register, bit 3-0, maybe dummy.
	uint8  opn_ch3mode[4];
	/* OPN Joystick */
	uint32 joyport_a;
	uint32 joyport_b;

	/* FD47 */
	bool intstat_whg;   // bit3 : OPN interrupt. '0' = happened.
	/* FD53 */
	bool intstat_thg;   // bit3 : OPN interrupt. '0' = happened.

	
	/* FD17 : R */
	bool intstat_opn;   // bit3 : OPN interrupt. '0' = happened.
	bool intstat_mouse; // bit2 : Mouse interrupt (not OPN-Mouse?), '0' = happened.
	/* FD17 : W */
	bool mouse_enable; // bit2 : '1' = enable.
	
	/* FD18 : R */
	bool connect_fdc;
	uint8 fdc_statreg;
	/* FD18 : W */
	uint8 fdc_cmdreg;
	
	/* FD19 : R/W */
	uint8 fdc_trackreg;
	
	/* FD1A : R/W */
	uint8 fdc_sectreg;
	
	/* FD1B : R/W */
	uint8 fdc_datareg;
	
	/* FD1C : R/W */
	uint8 fdc_headreg; // bit0, '0' = side0, '1' = side1
	
	/* FD1D : R/W */
	bool fdc_motor; // bit7 : '1' = ON, '0' = OFF
	uint8 fdc_drvsel; // bit 1-0
	
	/* FD1F : R */
	uint8 irqreg_fdc;
	bool irqstat_fdc;
   
	/* FD20,FD21 : W */
	bool connect_kanjiroml1;
	pair kaddress; // FD20 : ADDRESS OF HIGH.
#ifdef _FM77AV_VARIANTS
	bool connect_kanjiroml2;
	pair kaddress_l2; // FD20 : ADDRESS OF HIGH.
#endif	
	/* FD20, FD21 : R */
	
	/* FD37 : W */
	uint8 multipage_disp;   // bit6-4 : to display : GRB. '1' = disable, '0' = enable.
	uint8 multipage_access; // bit2-0 : to access  : GRB. '1' = disable, '0' = enable.
#ifdef HAS_MMR
	bool mmr_enabled;
	bool mmr_fast;
	uint8 mmr_segment;
	uint8 mmr_table[8 * 16];
	bool window_enabled;
	uint32 window_offset;
#endif	
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	/* FD93: bit0 */
	bool boot_ram;
	/* FD10: bit1 */
	bool enable_initiator;
#endif	
	
	void set_clockmode(uint8 flags);
	uint8 get_clockmode(void);
	void set_cmt_motor(uint8 flag);
	bool get_cmt_motor(void);
	
	virtual uint8 get_port_fd00(void);
	virtual void  set_port_fd00(uint8 data);
	virtual uint8 get_port_fd02(void);
	virtual void set_port_fd02(uint8 val);
	virtual uint8 get_irqstat_fd03(void);
	virtual uint8 get_extirq_fd17(void);
	virtual void set_ext_fd17(uint8 data);

	void set_beep(uint32 data); // fd03
	void reset_sound(void);
	
	void reset_fdc(void);
	void set_fdc_motor(bool flag);
	
	void do_irq(void);
	void set_irq_timer(bool flag);
	void set_irq_printer(bool flag);
	void set_irq_keyboard(bool flag);
	void set_irq_opn(bool flag);
	void set_irq_mfd(bool flag);
	void set_drq_mfd(bool flag);

	// FD04
	void do_firq(void);
	void do_nmi(bool flag);
	  
	void set_break_key(bool pressed);
	void set_sub_attention(bool flag);
	  
	uint8 get_fd04(void);
	void  set_fd04(uint8 val);
	uint8 get_fd05(void);
	void  set_fd05(uint8 val);
	
	virtual void set_extdet(bool flag);
	// FD0D
	void set_psg(uint8 val);
	uint8 get_psg(void);
	// FD0E
	void set_psg_cmd(uint8 cmd);
	
	void write_fd0f(void)  {
		stat_romrammode = false;
	}
	uint8 read_fd0f(void)  {
		stat_romrammode = true;
		return 0xff;
	}
	bool get_rommode_fd0f(void) {
		return stat_romrammode;
	}
#if defined(_FM77AV_VARIANTS)
	// FD12
	uint8 subsystem_read_status(void);
#endif   
	// OPN
	void opn_note_on(int index);
	void set_opn(int index, uint8 val);
	uint8 get_opn(int index);
	void set_opn_cmd(int index, uint8 cmd);
	void write_opn_reg(int index, uint32 addr, uint32 data);
  
	uint8 get_extirq_whg(void);
	uint8 get_extirq_thg(void);
	uint32 update_joystatus(int index);
	
	void write_kanjiaddr_lo(uint8 addr);
	void write_kanjiaddr_hi(uint8 addr);
	uint8 read_kanjidata_left(void);
	uint8 read_kanjidata_right(void);
	  
	  // FDC
	uint8 get_fdc_fd1c(void);
	void set_fdc_fd1c(uint8 val);
	void set_fdc_fd1d(uint8 val);
	
	uint8 get_fdc_stat(void);
	void set_fdc_cmd(uint8 val);
	uint8 fdc_getdrqirq(void);

	virtual void set_fdc_track(uint8 val);
	virtual uint8 get_fdc_track(void);

	uint8 get_fdc_motor(void);
	  
	void set_fdc_sector(uint8 val);

	uint8 get_fdc_sector(void);
	  
	void set_fdc_data(uint8 val);
	uint8 get_fdc_data(void);
	/* Signal Handlers */
	void set_beep_oneshot(void);
	
	/* Event Handlers */
	void event_beep_off(void);
	void event_beep_cycle(void);
	void proc_sync_to_sub(void);
	void do_sync_main_sub(void);
	/* Devices */
	YM2203* opn[4]; // 0=OPN 1=WHG 2=THG 3=PSG
	
	DEVICE* drec;
        DEVICE* pcm1bit;
        //DEVICE* beep;
	DEVICE* fdc;
	//FM7_PRINTER *printer;
	//FM7_RS232C *rs232c;
	/* */
	MEMORY *kanjiclass1;
	MEMORY *kanjiclass2;
	DEVICE *display;
	DEVICE *keyboard;
	MC6809 *maincpu;
	MEMORY *mainmem;
	MC6809 *subcpu;
	Z80 *z80;
 public:
	FM7_MAINIO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		int i;
		p_vm = parent_vm;
		p_emu = parent_emu;
		kanjiclass1 = NULL;
		kanjiclass2 = NULL;
		opn_psg_77av = false;
		nmi_count = 0;
		// FD00
		clock_fast = true;
		lpt_strobe = false;
		lpt_slctin = false;
		// FD01
		lpt_outdata = 0x00;
		// FD02
		cmt_indat = false; // bit7
		cmt_invert = false; // Invert signal
		lpt_det2 = false;
		lpt_det1 = false;
		lpt_pe = false;
		lpt_ackng_inv = false;
		lpt_error_inv = false;
		lpt_busy = false;
		// FD04
		stat_fdmode_2hd = false; //  R/W : bit6, '0' = 2HD, '1' = 2DD. FM-77 Only.
		stat_kanjirom = true;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
		stat_400linecard = false;//  R/W : bit4, '0' = connected. FM-77 Only.
		stat_400linemode = false; // R/W : bit3, '0' = 400line, '1' = 200line.
		firq_break_key = false; // bit1, ON = '0'.
		firq_sub_attention = false; // bit0, ON = '0'.
		intmode_fdc = false; // bit2, '0' = normal, '1' = SFD.
		// FD05
		extdet_neg = false;
		z80_sel = false;    // bit0 : '1' = Z80. Maybe only FM-7/77.
		// FD06,07
		intstat_syndet = false;
		intstat_rxrdy = false;
		intstat_txrdy = false;
		// FD0B
		stat_bootsw_basic = true; // bit0 : '0' = BASIC '1' = DOS. Only 77AV/20/40.
		bootmode = 0x00;
		// FD0D
		psg_cmdreg = 0;
		psg_statreg = 0x00;
		psg_address = 0x00;
		psg_data = 0x00;
		psg_bus_high = false;
		// FD0F
		stat_romrammode = true; // ROM ON
		
		// FD15/ FD46 / FD51
		connect_opn = false;
		connect_whg = false;
		connect_thg = false;
		psg_shared_opn = false;
		
		for(i = 0; i < 3; i++) {
			opn_address[i] = 0x00;
			opn_data[i] = 0x00;
			opn_cmdreg[i] = 0;
		}
		joyport_a = 0x00;
		joyport_b = 0x00;
		
		intstat_whg = false;
		intstat_thg = false;
		// FD17
		intstat_opn = false;
		intstat_mouse = false;
		mouse_enable = false;
		// FD18-FD1F
		connect_fdc = false;
		fdc_statreg = 0x00;
		fdc_cmdreg = 0x00;
		fdc_trackreg = 0x00;
		fdc_sectreg = 0x00;
		fdc_datareg = 0x00;
		fdc_headreg = 0x00;
		fdc_drvsel = 0x00;
		fdc_motor = false;
		irqstat_fdc = 0;
		// FD20, FD21, FD22, FD23
		connect_kanjiroml1 = false;
#if defined(_FM77AV_VARIANTS)
		enable_initiator = true;
		// FD2C, FD2D, FD2E, FD2F
		connect_kanjiroml2 = false;
#endif		
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		boot_ram = false;
#endif		
		memset(io_w_latch, 0x00, 0x100);
	}
	~FM7_MAINIO(){}
	void event_vline(int v, int clock);

	uint8  opn_regs[4][0x100];
	uint32 read_io8(uint32 addr) { // This is only for debug.
		addr = addr & 0xfff;
		if(addr < 0x100) {
			return io_w_latch[addr];
		} else if(addr < 0x500) {
			uint32 ofset = addr & 0xff;
			uint opnbank = (addr - 0x100) >> 8;
			return opn_regs[opnbank][ofset];
		} else if(addr < 0x600) {
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
			return mmr_table[addr & 0xff];
#elif defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
			return mmr_table[addr & 0x3f];
#else		   
			return 0xff;
#endif
		}
	   return 0xff;
	}
   
	   
   
	void initialize();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);

	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_signal(uint32 addr);
	void event_callback(int event_id, int err);
	void reset();
	void update_config();

	void set_context_kanjirom_class1(MEMORY *p)
	{
		kanjiclass1 = p;
		if(p != NULL) connect_kanjiroml1 = true;
	}
	void set_context_kanjirom_class2(MEMORY *p)
	{
#if defined(_FM77AV_VARIANTS)
		kanjiclass2 = p;
		if(p != NULL) connect_kanjiroml2 = true;
#endif
	}
	void set_context_beep(DEVICE *p)
	{
		pcm1bit = p;
		//beep = p;
	}
	void set_context_datarec(DEVICE *p)
	{
		drec = p;
	}
	void set_context_opn(YM2203 *p, int ch)
	{
		if((ch < 0) || (ch > 2)) return;
		if(p != NULL) {
			switch(ch) {
				case 0:
					connect_opn = true;
					break;
				case 1:
					connect_whg = true;
					break;
				case 2:
					connect_thg = true;
					break;
			}
		}
		opn[ch] = p;
		if(connect_opn) {
			extdet_neg = true;
		}
		if(connect_whg) {
			extdet_neg = true;
		}
		if(connect_thg) {
			extdet_neg = true;
		}
	}
	void set_context_psg(YM2203 *p)
	{
		opn[3] = p;
	}
	void set_context_fdc(DEVICE *p){
		if(p == NULL) {
	  		connect_fdc = false;
		} else {
			connect_fdc = true;
		}
		if(connect_fdc) {
			extdet_neg = true;
		}
		emu->out_debug_log("FDC: connect=%d\n", connect_fdc);
		fdc = p;
	}
	void set_context_maincpu(MC6809 *p){
		maincpu = p;
	}
	void set_context_mainmem(MEMORY *p){
		mainmem = p;
	}
	void set_context_subcpu(MC6809 *p){
		subcpu = p;
	}
	void set_context_display(DEVICE *p){
		display = p;
	}
	void set_context_keyboard(DEVICE *p){
		keyboard = p;
	}
	void set_context_z80cpu(Z80 *p){
		z80 = p;
	}

};
#endif
