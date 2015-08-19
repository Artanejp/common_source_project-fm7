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
#include "../mc6809.h"
#include "../z80.h"
#include "../ym2203.h"

#include "fm7_common.h"
#include "./joystick.h"


class MB8877;
#if defined(HAS_DMA)
class HD6844;
#endif

class FM7_MAINIO : public DEVICE {
 private:
	bool opn_psg_77av;
	bool beep_flag;
	bool beep_snd;
	int event_beep;  
	int event_beep_oneshot;  
	int event_timerirq;  
	int event_fdc_motor;  
 protected:
	VM* p_vm;
	EMU* p_emu;

	//bool irqstat_bak;
	//bool firqstat_bak;
	uint8 io_w_latch[0x100];
   
	/* FD00: R */
	bool clock_fast; // bit0 : maybe dummy
	uint16 keycode_7;
	/* FD00: W */
	bool lpt_strobe;  // bit6 : maybe dummy entry
	bool lpt_slctin;  // bit7 : maybe dummy entry
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
	bool irqmask_syndet;   // bit7: "0" = mask.
	bool irqmask_rxrdy;    // bit6: "0" = mask.
	bool irqmask_txrdy;    // bit5: "0" = mask.
	bool irqmask_mfd;      // bit4: "0" = mask.
	bool irqmask_timer;    // bit2: "0" = mask.
	bool irqmask_printer;  // bit1: "0" = mask.
	bool irqmask_keyboard; // bit0: "0" = mask.
  
	/* FD03: R */
	uint8 irqstat_reg0; // bit 3-0, '0' is happened, '1' is not happened.
	// bit3 : extended interrupt
	// bit2-0: Same as FD02 : W .
	/* FD03 : W , '1' = ON*/
	bool irqstat_timer;
	bool irqstat_printer;
	bool irqstat_keyboard;
   
	bool irqreq_timer;
	bool irqreq_printer;
	bool irqreq_keyboard;

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
#if defined(_FM77AV_VARIANTS)
	bool hotreset;
#endif	
	/* FD05 : R */
	bool extdet_neg; // bit0 : '1' = none , '0' = exists.
	bool sub_busy;
	/* FD05 : W */
	bool sub_halt; // bit7 : '1' Halt req.
	bool sub_cancel; // bit6 : '1' Cancel req.
	bool sub_halt_bak; // bit7 : shadow.
	bool sub_cancel_bak; // bit6 : shadow.
#if defined(WITH_Z80)	
	bool z80_sel;    // bit0 : '1' = Z80. Maybe only FM-7/77.
#endif
	/* FD06 : R/W : RS-232C */
	/* FD07 : R/W : RS-232C */
	bool intstat_syndet;
	bool intstat_rxrdy;
	bool intstat_txrdy;
	bool irqreq_syndet;
	bool irqreq_rxrdy;
	bool irqreq_txrdy;	/* FD08 : Grafic pen, not implemented */
	/* FD09 : Grafic pen, not implemented */
	/* FD0A : Grafic pen, not implemented */
	/* FD0B : R */
	uint32 bootmode;
	/* FD0D : W */
	/* FD0E : R */
  
	/* FD0F : R/W */
	bool stat_romrammode; // R(true) = ROM, W(false) = RAM.
#if defined(_FM77AV_VARIANTS)
	/* FD12 : R/W*/
	//bool mode320; // bit6 : true = 320, false = 640
	/* FD13 : WO */
	uint8 sub_monitor_type; // bit 2 - 0: default = 0.
	uint8 sub_monitor_bak; // bit 2 - 0: default = 0.
#endif
	
	/* FD15 / FD46 / FD51 : W */
	bool connect_opn; // [0]
	bool connect_whg; // [1]
	bool connect_thg; // [2]

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
	bool fdc_cmd_type1;
   
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
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)|| \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
	//bool fdc_2dd;
	// FD1E
	uint8 fdc_drive_table[4];
	uint8 fdc_reg_fd1e;
#endif	
	/* FD1F : R */
	uint8 irqreg_fdc;
	bool irqstat_fdc;
	bool irqreq_fdc;
   
	/* FD20,FD21 : W */
	bool connect_kanjiroml1;
	pair kaddress; // FD20 : ADDRESS OF HIGH.
#ifdef _FM77AV_VARIANTS
	bool connect_kanjiroml2;
	pair kaddress_l2; // FD20 : ADDRESS OF HIGH.
#endif	
	/* FD20, FD21 : R */
	
	/* FD37 : W */
#ifdef HAS_MMR
	bool mmr_enabled;
	bool mmr_fast;
	//uint8 mmr_segment;
	//uint8 mmr_table[8 * 16];
	bool window_enabled;
	uint32 window_offset;
#endif	
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	/* FD93: bit0 */
	bool boot_ram;
	/* FD10: bit1 */
	bool enable_initiator;
#endif	
#if defined(HAS_DMA)
	bool intstat_dma;
	uint32 dma_addr;
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
	void set_irq_syndet(bool flag);
	void set_irq_rxrdy(bool flag);
	void set_irq_txrdy(bool flag);
	
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
	
	void write_fd0f(void);
	uint8 read_fd0f(void);
	bool get_rommode_fd0f(void);
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
	
	void write_kanjiaddr_lo(uint8 addr);
	void write_kanjiaddr_hi(uint8 addr);
	uint8 read_kanjidata_left(void);
	uint8 read_kanjidata_right(void);
	  
	  // FDC
	uint8 get_fdc_fd1c(void);
	void set_fdc_fd1c(uint8 val);
	void set_fdc_fd1d(uint8 val);
	
	uint8 get_fdc_fd1e(void);
	void set_fdc_fd1e(uint8 val);
	
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
	
	void set_fdc_misc(uint8 val);
	uint8 get_fdc_misc(void);
	/* Signal Handlers */
	void set_beep_oneshot(void);
	
	/* Event Handlers */
	void event_beep_off(void);
	void event_beep_cycle(void);
	/* Devices */
	YM2203* opn[4]; // 0=OPN 1=WHG 2=THG 3=PSG
	
	DEVICE* drec;
        DEVICE* pcm1bit;
	DEVICE* joystick;
	
        //DEVICE* beep;
	MB8877* fdc;
#if defined(HAS_DMA)
	HD6844* dmac;
#endif
	//FM7_PRINTER *printer;
	//FM7_RS232C *rs232c;
	/* */
	DEVICE *kanjiclass1;
	DEVICE *kanjiclass2;
	DEVICE *display;
	DEVICE *keyboard;
	MC6809 *maincpu;
	DEVICE *mainmem;
	MC6809 *subcpu;
#ifdef WITH_Z80
	Z80 *z80;
#endif	
 public:
	FM7_MAINIO(VM* parent_vm, EMU* parent_emu);
	~FM7_MAINIO();
	void event_vline(int v, int clock);

	uint8  opn_regs[4][0x100];
	uint32 read_io8(uint32 addr); // This is only for debug.
  
	void initialize();

	void write_data8(uint32 addr, uint32 data);
	void write_dma_data8(uint32 addr, uint32 data);
	void write_dma_io8(uint32 addr, uint32 data);
   
	uint32 read_data8(uint32 addr);
	uint32 read_dma_data8(uint32 addr);
	uint32 read_dma_io8(uint32 addr);

	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_signal(int id);

	void event_callback(int event_id, int err);
	void reset();
	void update_config();
	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);

	void set_context_kanjirom_class1(DEVICE *p)
	{
		kanjiclass1 = p;
		if(p != NULL) connect_kanjiroml1 = true;
	}
	void set_context_kanjirom_class2(DEVICE *p)
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
	void set_context_fdc(MB8877 *p){
		if(p == NULL) {
	  		connect_fdc = false;
		} else {
			connect_fdc = true;
		}
		if(connect_fdc) {
			extdet_neg = true;
		}
		emu->out_debug_log("FDC: connect=%d", connect_fdc);
		fdc = p;
	}
	void set_context_maincpu(MC6809 *p){
		maincpu = p;
	}
	void set_context_mainmem(DEVICE *p){
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
	void set_context_joystick(DEVICE *p){
		joystick = p;
	}
	void set_context_z80cpu(Z80 *p){
#ifdef WITH_Z80
		z80 = p;
#endif
	}
#if defined(HAS_DMA)
	void set_context_dmac(HD6844 *p){
		dmac = p;
	}
#endif
};
#endif
