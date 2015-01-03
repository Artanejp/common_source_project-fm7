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
  /* FD05 : R */
  bool sub_halt = false; // bit7 : '1' = HALT, maybe dummy.
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
  uint8 opn_cmdreg = 0b11110000; // OPN register, bit 3-0, maybe dummy.
  /* FD16 : R/W */
  uint8 opn_data; // OPN data, maybe dummy.

  /* FD17 : R */
  bool intstat_opn = false;   // bit3 : OPN interrupt. '0' = happened.
  bool intstat_mouse = false; // bit2 : Mouse interrupt (not OPN-Mouse?), '0' = happened.
  /* FD17 : W */
  bool mouse_enable = false; // bit2 : '1' = enable.

  /* FD18 : R */
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

  void write_fd0f(void)
 public:
 FM7_MAINIO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
    {
    }
  ~FM7_MAINIO(){}
  virtual void set_clockmode(uint8 flags){
    if(flags == FM7_MAINCLOCK_SLOW) {
      clock_fast = false;
    } else {
      clock_fast = true;
    }
  }
  virtual uint8 get_clockmode(void){
    if(clock_fast) return FM7_MAINCLOCK_SLOW;
    return FM7_MAINCLOCK_HIGH;
  }
  void set_cmt_motor(uint8 flag) {
    
    if((flag & 0x02) == 0) {
      crt_motor = true;
      // Motor ON
    } else {
      crt_motor = false;
      // Motor OFF
    }
  }
  bool get_cmt_motor(void) { return cmt_motor; }

  void set_cmt_writedata(uint8 data) {
    if((data & 0x01) == 1) {
      // write '1'
    } else {
      // write '0'
    }
  }
  bool get_cmt_writedata(void) {
    return cmt_wrtdata;
  }
  
  bool get_cmt_readdata(void)
  {
    return cmt_rdata;
  }
  void set_cmt_readdata(bool flag)
  {
    cmt_rdata = flag;
  }
  
  uint32 get_keyboard(void) {
    uint32 kbd_data = (uint32) kbd_bit7_0;
    kbd_data &= 0x0ff;
    if(kbd_bit8) kbd_data |= 0x0100;
    return kbd_data;
  }

  void set_irq_timer(bool flag)
  {
    if((flag) && ((irqmask_reg0 & 0b00000100) != 0)) {
      irqstat_reg0 &= 0b11111011;
      // Call IRQ
    }
    if(flag == false) {
      irqstat_reg0 |= 0b00000100;
      // Unset IRQ??
    }
  }
  void set_irq_printer(bool flag)
  {
    if((flag) && ((irqmask_reg0 & 0b00000010) != 0)) {
      irqstat_reg0 &= 0b11111101;
      // Call IRQ
    }
    if(flag == false) {
      irqstat_reg0 |= 0b00000010;
      // Unset IRQ?
    }
  }

  void set_irq_keyboard(bool flag)
  {
    if((flag) && ((irqmask_reg0 & 0b00000001) != 0)) {
      irqstat_reg0 &= 0b11111110;
      // Call IRQ
    }
    if(flag == false) {
      irqstat_reg0 |= 0b00000001;
      // Unset IRQ?
    }
  }
    
  virtual void set_keyboard(uint32 data){
    if((data & 0x100) != 0){
      kbd_bit8 = true;
    } else {
      kbd_bit8 = false;
    }
    kbd_bit7_0 = (data & 0xff);
  }

  virtual void set_psg(uint8 cmdreg, uint8 datareg)
  {
    if((cmdreg & 0x03) == 0){
      psg_bus_high = true;
      return;
    }
    //
    psg_bus_high = false;
    switch(cmdreg & 0x03) {
    case 0: // High inpedanse.
      psg_bus_high = true;
      break;
    case 1: // Read data.
      psg_data = psg->read_io8(1);
      break;
    case 2: // Write Data.
      psg_data = ((uint32)datareg) & 0xff;
      psg->write_io8(1, psg_data);
      break;
    case 3: // Latch address.
      psg_address = datareg & 0x0f;  // Really?
      psg_data = psg_address;
      psg->write_io8(0, psg_address);
      break;
    }
  }
  virtual uint32 get_psg(void) {
    if(psg_bus_high) return 0xff;
    return psg_data & 0x00ff;
  }
}
