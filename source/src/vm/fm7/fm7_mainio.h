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

enum {
  FM7_MAINCLOCK_SLOW,
  FM7_MAINCLOCK_HIGH,
  FM7_MAINCLOCK_MMRSLOW,
  FM7_MAINCLOCK_MMRHIGH
};

class FM7_MAINIO : public DEVICE {
 pretected:
  /* FD00: R */
  bool clock_fast = true; // bit0 : maybe dummy
  uint8 kbd_bit8;  // bit7
  /* FD00: W */
  bool cmt_wrtdata; // bit0 : maybe dummy entry
  bool cmt_motor;   // bit1 : maybe dummy entry
  bool lpt_strobe;  // bit6 : maybe dummy entry
  bool lpt_slctin;  // bit7 : maybe dummy entry

  /* FD01 : R */
  uint8 kbd_bit7_0;
  /* FD01: W */
  uint8 lpt_outdata; // maybe dummy.

  /* FD02 : R */
  bool cmt_rdata; // bit7 : maybe dummy.
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
  bool irqmask_mfd      = false; // bit4: "0" = mask.
  bool irqmask_timer    = false; // bit2: "0" = mask.
  bool irqmask_printer  = false; // bit1: "0" = mask.
  bool irqmask_keyboard = false; // bit0: "0" = mask.

  
  /* FD03: R */
  uint8 irqstat_reg0 = 0xff; // bit 3-0, '0' is happened, '1' is not happened.
  // bit3 : extended interrupt
  // bit2-0: Same as FD02 : W .
  /* FD03 : W , '1' = ON*/
  bool beep_cont = false; // bit7 , maybe dummy.
  bool beep_single = false; // bit6, maybe dummy.
  bool beep_out = false;  // bit0, maybe dummy???

  /* FD04 : R */
  //bool stat_fdmode_2hd = false; //  R/W : bit6, '0' = 2HD, '1' = 2DD. FM-77 Only.
  //bool stat_kanjirom = true;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
  //bool stat_400linecard = false;//  R/W : bit4, '0' = connected. FM-77 Only.
  //bool stat_400linemode = false; // R/W : bit3, '0' = 400line, '1' = 200line.
  bool firq_break_key = false; // bit1, ON = '0'.
  bool firq_sub_attention = false; // bit0, ON = '0'.
  /* FD04 : W */
  //bool intmode_fdc = false; // bit2, '0' = normal, '1' = SFD.

  /* FD05 : R */
  bool sub_busy = false; // bit7 : '0' = READY '1' = BUSY.
  bool extdet_neg = false; // bit0 : '1' = none , '0' = exists.
  /* FD05 : W */
 
  bool sub_haltreq = false; // bit7 : '1' = HALT, maybe dummy.
  bool sub_cansel = false; // bit6 : '1' Cansel req.
  bool z80_sel = false;    // bit0 : '1' = Z80. Maybe only FM-7/77.

  /* FD06 : R/W : RS-232C */
  /* FD07 : R/W : RS-232C */
  /* FD08 : Grafic pen, not implemented */
  /* FD09 : Grafic pen, not implemented */
  /* FD0A : Grafic pen, not implemented */
  /* FD0B : R */
  //bool stat_bootsw_basic = true; // bit0 : '0' = BASIC '1' = DOS. Only 77AV/20/40.

  /* FD0D : W */
  uint8 psg_cmdreg = 0b11111100; // PSG Register, Only bit 0-1 at FM-7/77 , 3-0 at FM-77AV series. Maybe dummy.
  /* FD0E : R */
  uint8 psg_statreg; // PSG data. maybe dummy.
  uint32 psg_address = 0x00;
  uint32 psg_data = 0x00;
  bool  psg_bus_high = false; // true when bus = high inpedance.
  
  /* FD0F : R/W */
  bool stat_romrammode = true; // R(true) = ROM, W(false) = RAM.

  /* FD15 : W */
  bool connect_opn = false;
  uint32 opn_address = 0x00;
  uint32 opn_data = 0x00;
  uint8 opn_cmdreg = 0b11110000; // OPN register, bit 3-0, maybe dummy.
  /* FD16 : R/W */
  uint8 opn_data; // OPN data, maybe dummy.

  /* FD17 : R */
  bool intstat_opn = false;   // bit3 : OPN interrupt. '0' = happened.
  bool intstat_mouse = false; // bit2 : Mouse interrupt (not OPN-Mouse?), '0' = happened.
  /* FD17 : W */
  bool mouse_enable = false; // bit2 : '1' = enable.

  /* FD18 : R */
  bool fdc_connected = false;
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
  bool fdc_motor = false; // bit7 : '1' = ON, '0' = OFF
  uint8 fdc_drvsel; // bit 1-0

  /* FD1F : R */
  bool fdc_drq  = false; // bit7 : '1' = ON
  bool fdc_irq  = false; // bit6 : '1' = ON
  uint8 irqstat_fdc = 0;
  
  /* FD20,FD21 : W */
  bool connect_kanjiroml1 = false;
  uint8 kaddress_hi; // FD20 : ADDRESS OF HIGH.
  uint8 kaddress_lo; // FD21 : ADDRESS OF LOW.
  /* FD20, FD21 : R */
  uint8 kdata_left;  // FD20 DATA OF LEFT. Maybe dummy.
  uint8 kdata_right; // FD21 DATA OF RIGHT. Maybe dummy.

  /* FD37 : W */
  uint8 multipage_disp;   // bit6-4 : to display : GRB. '1' = disable, '0' = enable.
  uint8 multipage_access; // bit2-0 : to access  : GRB. '1' = disable, '0' = enable.
  /* OPN Joystick */
  uint32 opnport_a = 0;
  uint32 opnport_b = 0;
  void stop_beep(void) // event
  {
     beep->write_signal(SIG_BEEP_ON, 0b00000000, 0c00100000);
  }

 
 public:
 FM7_MAINIO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
    {
    }
  ~FM7_MAINIO(){}
  virtual void set_clockmode(uint8 flags);
  virtual uint8 get_clockmode(void);
  void set_cmt_motor(uint8 flag);
  bool get_cmt_motor(void);
  void set_cmt_writedata(uint8 data) {
  }
  bool get_cmt_writedata(void) {
    return cmt_wrtdata;
  }
  
  bool get_cmt_readdata(void)
  {
    return cmt_rdata;
  }
  
  virtual uint8 get_port_fd00(void);
  virtual void  set_port_fd00(uint8 data);
  virtual uint32 get_keyboard(void); // FD01
  virtual uint8 get_port_fd02(void);

  void set_beep(uint32 data); // fd03

  void do_irq(bool flag)
  void set_irq_timer(bool flag);
  void set_irq_printer(bool flag);
  void set_irq_keyboard(bool flag);
  void set_irq_opn(bool flag);
  virtual void set_keyboard(uint32 data);  

  // FD04
  void do_firq(bool flag);
  
  void set_break_key(bool pressed);
  void set_sub_attention(bool flag);
  
  virtual uint8 get_fd04(void);
  virtual void  set_fd04(uint8 val);
  virtual uint8 get_fd05(void);
  virtual void  set_fd05(uint8 val);

  virtual void set_extdet(bool flag);
  // FD0D
  virtual void set_psg(uint8 val);
  virtual uint8 get_psg(void);
  // FD0E
  virtual void set_psg_cmd(uint32 cmd);
  virtual uint8 get_psg_cmd(void);
  
  void write_fd0f(void)
  {
	stat_romrammode = false;
  }
  uint8 read_fd0f(void)
  {
	stat_romrammode = true;
	return 0xff;
  }
  bool get_rommode_fd0f(void)
  {
	return stat_romrammode;
  }

  // OPN
  virtual void set_opn(uint8 val);
  virtual uint8 get_opn(void);
  virtual void set_opn_cmd(uint32 cmd);
  virtual uint8 get_opn_cmd(void);
  
  void write_kanjiaddr_lo(uint8 addr);
  void write_kanjiaddr_hi(uint8 addr);
  uint8 read_kanjidata_left(void);
  uint8 read_kanjidata_right(void);

  // FDC
  virtual void set_fdc_stat(uint8 val)
  {
    write_io8(0, val & 0x00ff);
  }
  virtual uint8 get_fdc_stat(void)
  {
    return read_io8(0);
  }
  virtual void set_fdc_track(uint8 val)
  {
    // if mode is 2DD and type-of-image = 2D then val >>= 1;
    write_io8(1, val & 0x00ff);
  }
  virtual uint8 get_fdc_track(void)
  {
    return read_io8(1);
  }
  virtual void set_fdc_sector(uint8 val)
  {
    write_io8(2, val & 0x00ff);
  }
  virtual uint8 get_fdc_sector(void)
  {
    return read_io8(2);
  }
  
  virtual void set_fdc_data(uint8 val)
  {
    write_io8(3, val & 0x00ff);
  }
  virtual uint8 get_fdc_data(void)
  {
    return read_io8(3);
  }

  void write_signals(int id, uint32 data, uint32 mask);
  
}
