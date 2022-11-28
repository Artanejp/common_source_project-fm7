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

#include "./fm7.h"
#include "fm7_common.h"

#include "../device.h"

class DATAREC;
class MC6809;
class Z80;
class YM2203;
#if defined(USE_AY_3_8910_AS_PSG) && !defined(_FM77AV_VARIANTS)
class AY_3_891X;
#endif
class MB8877;
class I8251;
class AND;
#if defined(HAS_DMA)
class HD6844;
#endif

class JOYSTICK;
class FM7_MAINMEM;
class DISPLAY;
class KEYBOARD;
class KANJIROM;
#if defined(CAPABLE_JCOMMCARD)
class FM7_JCOMMCARD;
#endif

class FM7_MAINIO : public DEVICE {
 protected:
	bool opn_psg_77av;
	bool beep_flag;
	bool beep_snd;
	int event_beep;  
	int event_beep_oneshot;  
	int event_timerirq;  
	int event_fdc_motor;
	int event_fdc_motor_2HD;
#if defined(HAS_2HD)
	int event_2hd_nmi;
#endif	
	outputs_t clock_status;
	outputs_t printer_reset_bus;
	outputs_t printer_strobe_bus;
	outputs_t printer_select_bus;
	outputs_t irq_bus;
	outputs_t firq_bus;
	outputs_t nmi_bus;
 protected:

	uint8_t io_w_latch[0x100];
   
	/* FD00: R */
	bool clock_fast; // bit0
	/* FD00: W */
	bool lpt_strobe;  // bit6
	bool lpt_slctin;  // bit7
	/* FD01: W */
	uint8_t lpt_outdata; //

	/* FD02 : R */
	bool cmt_indat;     // bit7 : Data of casette.
	bool cmt_invert;    // Invert signal
	bool lpt_det2;      // bit5 : DET2(Normally high).
	bool lpt_det1;      // bit4 : DET1(Normally high).
	bool lpt_pe;        // bit3 : PAPER EMPTY.
	bool lpt_ackng_inv; // bit2 : ACK
	bool lpt_error_inv; // bit1 : ERROR
	bool lpt_busy;      // bit0 : BUSY.

	int lpt_type;
	/* FD02 : W */
	uint8_t irqmask_reg0; // bit7-4, bit2-0 , '1' is enable.  '0' is disable.
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
	uint8_t irqstat_reg0; // bit 3-0, '0' is happened, '1' is not happened.
	// bit3 : extended interrupt
	// bit2-0: Same as FD02 : W .
	/* FD03 : W , '1' = ON*/
	bool irqstat_timer;
	bool irqstat_printer;
	bool irqstat_keyboard;
   
	bool irqreq_printer;
	bool irqreq_keyboard;

	/* FD04 : R */
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX) 
	bool stat_kanjirom;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
	bool stat_400linemode; // R/W : bit3, '0' = 400line, '1' = 200line.
#elif defined(_FM77_VARIANTS)	
	bool stat_kanjirom;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
	bool stat_400linecard;//  R/W : bit4, '0' = connected. FM-77 Only.
	//bool stat_400linemode; // R/W : bit3, '0' = 400line, '1' = 200line.
#endif
#if defined(HAS_2HD)
	uint32_t nmi_delay;
	bool drqstat_fdc_2hd;
	bool irqstat_fdc_2hd;
	bool stat_fdmode_2hd; //  R/W : bit6, '0' = 2HD, '1' = 2DD. FM-77 Only.
#endif	
	bool firq_break_key; // bit1, ON = '0'.
	bool firq_sub_attention; // bit0, ON = '0'.
	/* FD04 : W */
	bool intmode_fdc; // bit2, '0' = normal, '1' = SFD.
#if defined(_FM77AV_VARIANTS)
	bool hotreset;
#endif	
	/* FD05 : R */
	bool extdet_neg; // bit0 : '1' = none , '0' = exists.
	/* FD05 : W */
	bool sub_halt; // bit7 : '1' Halt req.
	bool sub_cancel; // bit6 : '1' Cancel req.
	bool sub_halt_bak; // bit7 : shadow.
	bool sub_cancel_bak; // bit6 : shadow.
	bool req_z80run;    // bit0 : '1' = Z80. Maybe only FM-7/77.
	bool z80_run;
	
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
	uint32_t bootmode;
	/* FD0D : W */
	/* FD0E : R */
  
	/* FD0F : R/W */
#if defined(_FM77AV_VARIANTS)
	/* FD12 : R/W*/
	uint8_t reg_fd12;
	/* FD13 : WO */
	uint8_t sub_monitor_type; // bit 2 - 0: default = 0.
#endif
	
	/* FD15 / FD46 / FD51 : W */
//#if defined(_FM8)
	bool connect_psg; // [0]
//#else	
	bool connect_opn; // [0]
	bool connect_whg; // [1]
	bool connect_thg; // [2]
//#endif
	uint8_t opn_address[4];
	uint8_t opn_data[4];
	uint8_t opn_stat[4];
	uint8_t  opn_cmdreg[4]; // OPN register, bit 3-0, maybe dummy.
	uint8_t  opn_ch3mode[4];
	uint8_t  opn_prescaler_type[4];
	
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
	uint8_t fdc_statreg;
	/* FD18 : W */
	uint8_t fdc_cmdreg;
   
	/* FD19 : R/W */
	uint8_t fdc_trackreg;
	
	/* FD1A : R/W */
	uint8_t fdc_sectreg;
	
	/* FD1B : R/W */
	uint8_t fdc_datareg;
	
	/* FD1C : R/W */
	uint8_t fdc_headreg; // bit0, '0' = side0, '1' = side1
	
	/* FD1D : R/W */
	bool fdc_motor; // bit7 : '1' = ON, '0' = OFF
	uint8_t fdc_drvsel; // bit 1-0
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)|| \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
	//bool fdc_2dd;
	// FD1E
	uint8_t fdc_drive_table[4];
	uint8_t fdc_reg_fd1e;
#endif	
	/* FD1F : R */
	uint8_t irqreg_fdc;
	bool irqstat_fdc;
#if defined(HAS_2HD)   
	/* FD1F : R */
	uint8_t irqreg_fdc_2HD;
#endif   
	/* FD20,FD21 : W */
	bool connect_kanjiroml1;
#ifdef _FM77AV_VARIANTS
	bool connect_kanjiroml2;
#endif	
	/* FD20, FD21 : R */

	/* FD30 - FD36 : RW */
	/* FD37 : R */
	bool connect_fdc_2HD;
	/* FD37 : W */
	/* FD30 : R */
	uint8_t fdc_2HD_statreg;
	/* FD30 : W */
	uint8_t fdc_2HD_cmdreg;
   
	/* FD31 : R/W */
	uint8_t fdc_2HD_trackreg;
	
	/* FD32 : R/W */
	uint8_t fdc_2HD_sectreg;
	
	/* FD33 : R/W */
	uint8_t fdc_2HD_datareg;
	
	/* FD34 : R/W */
	uint8_t fdc_2HD_headreg; // bit0, '0' = side0, '1' = side1
	
	/* FD35 : R/W */
	bool fdc_2HD_motor; // bit7 : '1' = ON, '0' = OFF
	uint8_t fdc_2HD_drvsel; // bit 1-0
	/* FD1F : R */
	//uint8_t irqreg_2HD_fdc;
	//bool irqstat_2HD_fdc;
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	/* FD93: bit0 */
	bool boot_ram;
	/* FD10: bit1 */
	bool enable_initiator;
#endif	
#if defined(HAS_DMA)
	bool intstat_dma;
	uint8_t dma_addr;
#endif	
	void set_clockmode(uint8_t flags);
	uint8_t get_clockmode(void);
	void set_cmt_motor(uint8_t flag);
	bool get_cmt_motor(void);
	
	virtual uint8_t get_port_fd00(void);
	virtual void  set_port_fd00(uint8_t data);
	virtual uint8_t get_port_fd02(void);
	virtual void set_port_fd02(uint8_t val);
	virtual uint8_t get_irqstat_fd03(void);
	virtual uint8_t get_extirq_fd17(void);
	virtual void set_ext_fd17(uint8_t data);

	void set_beep(uint32_t data); // fd03
	virtual void reset_sound(void);
	void reset_printer(void);
	
	void reset_fdc(void);
	void set_fdc_motor(bool flag);
	
	void reset_fdc_2HD(void);
	void set_fdc_motor_2HD(bool flag);
	
	virtual void do_irq(void);
	virtual void set_irq_syndet(bool flag);
	virtual void set_irq_rxrdy(bool flag);
	virtual void set_irq_txrdy(bool flag);
	
	virtual void set_irq_timer(bool flag);
	virtual void set_irq_printer(bool flag);
	virtual void set_irq_keyboard(bool flag);
	//virtual void set_irq_opn(bool flag);
	virtual void set_irq_mfd(bool flag);
	virtual void set_drq_mfd(bool flag);
#if defined(HAS_2HD)
	virtual void set_irq_mfd_2HD(bool flag);
	virtual void set_drq_mfd_2HD(bool flag);
#endif
	// FD04
	virtual void do_firq(void);
	virtual void do_nmi(bool flag);
	  
	void set_break_key(bool pressed);
	void set_sub_attention(bool flag);
	  
	uint8_t get_fd04(void);
	void  set_fd04(uint8_t val);
	uint8_t get_fd05(void);
	void  set_fd05(uint8_t val);
	
	void set_extdet(bool flag);
	// FD0D
	virtual void set_psg(uint8_t val);
	virtual uint8_t get_psg(void);
	// FD0E
	virtual void set_psg_cmd(uint8_t cmd);
	
	virtual void write_fd0f(void);
	virtual uint8_t read_fd0f(void);
	bool get_rommode_fd0f(void);
#if defined(_FM77AV_VARIANTS)
	// FD12
	uint8_t subsystem_read_status(void);
#endif   
	// OPN
	virtual void set_opn(int index, uint8_t val);
	virtual uint8_t get_opn(int index);
	virtual void set_opn_cmd(int index, uint8_t cmd);
	virtual void write_opn_reg(int index, uint32_t addr, uint32_t data);
  
	virtual uint8_t get_extirq_whg(void);
	virtual uint8_t get_extirq_thg(void);
	
	void write_kanjiaddr_lo(uint8_t addr);
	void write_kanjiaddr_hi(uint8_t addr);
	uint8_t read_kanjidata_left(void);
	uint8_t read_kanjidata_right(void);
#if defined(CAPABLE_KANJI_CLASS2)
	void write_kanjiaddr_lo_l2(uint8_t addr);
	void write_kanjiaddr_hi_l2(uint8_t addr);
	uint8_t read_kanjidata_left_l2(void);
	uint8_t read_kanjidata_right_l2(void);
#endif	
	  // FDC
	uint8_t get_fdc_fd1c(void);
	void set_fdc_fd1c(uint8_t val);
	void set_fdc_fd1d(uint8_t val);
	
	uint8_t get_fdc_fd1e(void);
	void set_fdc_fd1e(uint8_t val);
	
	uint8_t get_fdc_stat(void);
	void set_fdc_cmd(uint8_t val);
	uint8_t fdc_getdrqirq(void);

	void set_fdc_track(uint8_t val);
	uint8_t get_fdc_track(void);

	uint8_t get_fdc_motor(void);
	void set_fdc_sector(uint8_t val);
	uint8_t get_fdc_sector(void);
	  
	void set_fdc_data(uint8_t val);
	uint8_t get_fdc_data(void);
	
	void set_fdc_misc(uint8_t val);
	uint8_t get_fdc_misc(void);

	uint8_t get_fdc_fd1c_2HD(void);
	void set_fdc_fd1c_2HD(uint8_t val);
	void set_fdc_fd1d_2HD(uint8_t val);
	
	uint8_t get_fdc_fd1e_2HD(void);
	void set_fdc_fd1e_2HD(uint8_t val);
	
	uint8_t get_fdc_stat_2HD(void);
	void set_fdc_cmd_2HD(uint8_t val);
	uint8_t fdc_getdrqirq_2HD(void);

	void set_fdc_track_2HD(uint8_t val);
	uint8_t get_fdc_track_2HD(void);

	uint8_t get_fdc_motor_2HD(void);
	void set_fdc_sector_2HD(uint8_t val);
	uint8_t get_fdc_sector_2HD(void);
	  
	void set_fdc_data_2HD(uint8_t val);
	uint8_t get_fdc_data_2HD(void);
	
	void set_fdc_misc_2HD(uint8_t val);
	uint8_t get_fdc_misc_2HD(void);
	
	/* Signal Handlers */
	void set_beep_oneshot(void);
	
	/* Event Handlers */
	void event_beep_off(void);
	void event_beep_cycle(void);
	/* Devices */

	YM2203* opn[3]; // 0=OPN 1=WHG 2=THG
# if !defined(_FM77AV_VARIANTS)
#  if defined(USE_AY_3_8910_AS_PSG)
	AY_3_891X *psg;
#  else
	YM2203* psg; // Optional PSG.
#  endif
#endif

	DATAREC* drec;
	PCM1BIT* pcm1bit;
	JOYSTICK* joystick;
	
	I8251 *uart[3];
# if defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV20EX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	AND   *rs232c_dtr;
#endif
	bool uart_enabled[3];
	bool rs232c_enabled;
	bool rs232c_dcd;
	
	bool modem_irqmask_rxrdy;
	bool modem_irqmask_txrdy;
	bool modem_syndet;
	bool modem_rxrdy;
	bool modem_txrdy;
	
	bool midi_uart_irqmask;
	bool midi_syndet;
	bool midi_rxrdy;
	bool midi_txrdy;
	
	MB8877* fdc;
#if defined(HAS_2HD)
	MB8877* fdc_2HD;
#endif
#if defined(HAS_DMA)
	HD6844* dmac;
#endif
	DEVICE *printer;
	//FM7_RS232C *rs232c;
	/* */
	KANJIROM *kanjiclass1;
	KANJIROM *kanjiclass2;
	DISPLAY *display;
	KEYBOARD *keyboard;
	MC6809 *maincpu;
	FM7_MAINMEM *mainmem;
	MC6809 *subcpu;
#ifdef WITH_Z80
	Z80 *z80;
#endif
#if defined(CAPABLE_JCOMMCARD)
	FM7_JCOMMCARD *jcommcard;
#endif
	template <class T>
	void call_write_signal(T *np, int id, uint32_t data, uint32_t mask)
	{
		//T *nnp = static_cast<T *>(np);
		static_cast<T *>(np)->write_signal(id, data, mask);
	}
	template <class T>
		void call_write_data8(T *np, uint32_t addr, uint32_t data)
	{
		//T *nnp = static_cast<T *>(np);
		static_cast<T *>(np)->write_data8(addr, data);
	}
	template <class T>
		uint32_t call_read_data8(T *np, uint32_t addr)
	{
		//T *nnp = static_cast<T *>(np);
		return static_cast<T *>(np)->read_data8(addr);
	}
	template <class T>
		void call_write_dma_data8(T *np, uint32_t addr, uint32_t data)
	{
		//T *nnp = static_cast<T *>(np);
		static_cast<T *>(np)->write_dma_data8(addr, data);
	}
	template <class T>
		uint32_t call_read_dma_data8(T *np, uint32_t addr)
	{
		//T *nnp = static_cast<T *>(np);
		return static_cast<T *>(np)->read_dma_data8(addr);
	}
	bool decl_state_opn(FILEIO *state_fio, bool loading);
public:
	FM7_MAINIO(VM_TEMPLATE* parent_vm, EMU* parent_emu);
	~FM7_MAINIO();
	void event_vline(int v, int clock);

	uint8_t  opn_regs[4][0x100];
	uint32_t read_io8(uint32_t addr); // This is only for debug.
  
	virtual void initialize();

	virtual void write_data8(uint32_t addr, uint32_t data);
	void write_dma_data8(uint32_t addr, uint32_t data);
	void write_dma_io8(uint32_t addr, uint32_t data);
   
	virtual uint32_t read_data8(uint32_t addr);
	uint32_t read_dma_data8(uint32_t addr);
	uint32_t read_dma_io8(uint32_t addr);

	virtual void write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t read_signal(int id);
	
	virtual void event_callback(int event_id, int err);
	virtual void reset();
	virtual void update_config();
	virtual void save_state(FILEIO *state_fio);
	virtual bool load_state(FILEIO *state_fio);
	virtual bool decl_state(FILEIO *state_fio, bool loading);
	
	void set_context_printer(DEVICE *p)
	{
		printer = p;
	}
	void set_context_kanjirom_class1(DEVICE *p)
	{
		kanjiclass1 = (KANJIROM *)p;
		if(p != NULL) connect_kanjiroml1 = true;
	}
	virtual void set_context_kanjirom_class2(DEVICE *p)
	{
#if defined(_FM77AV_VARIANTS)
		kanjiclass2 = (KANJIROM *)p;
		if(p != NULL) connect_kanjiroml2 = true;
#endif
	}
	void set_context_beep(DEVICE *p)
	{
		pcm1bit = (PCM1BIT *)p;
		//beep = p;
	}
	void set_context_datarec(DATAREC *p)
	{
		drec = (DATAREC *)p;
	}
//#if !defined(_FM8)
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
		extdet_neg = true;
	}
//#endif
#if !defined(_FM77AV_VARIANTS)
# if defined(USE_AY_3_8910_AS_PSG)
	virtual void set_context_psg(AY_3_891X *p)
	{
		psg = p;
	}
# else
	virtual void set_context_psg(YM2203 *p)
	{
		psg = p;
	}
# endif
#endif
	void set_context_fdc(MB8877 *p){
		if(p == NULL) {
	  		connect_fdc = false;
			irqreg_fdc = 0xff; //0b11111111;
		} else {
			connect_fdc = true;
			extdet_neg = true;
			irqreg_fdc = 0x3f; //0b00111111;
		}
		this->out_debug_log(_T("FDC: connect=%d"), connect_fdc);
		fdc = p;
	}	
#if defined(HAS_2HD)
	void set_context_fdc_2HD(MB8877 *p){
		if(p == NULL) {
	  		connect_fdc_2HD = false;
			irqreg_fdc_2HD = 0xff; //0b11111111;
		} else {
			connect_fdc_2HD = true;
			extdet_neg = true;
			irqreg_fdc_2HD = 0x3f; //0b00111111;
		}
		this->out_debug_log(_T("FDC(2HD): connect=%d"), connect_fdc);
		fdc_2HD = p;
	} 
#endif
	void set_context_maincpu(MC6809 *p){
		maincpu = p;
	}
	void set_context_mainmem(DEVICE *p){
		mainmem = (FM7_MAINMEM *)p;
	}
	void set_context_subcpu(MC6809 *p){
		subcpu = p;
	}
	void set_context_display(DEVICE *p){
		display = (DISPLAY *)p;
	}
	void set_context_keyboard(DEVICE *p){
		keyboard = (KEYBOARD *)p;
	}
	void set_context_joystick(DEVICE *p){
		joystick = (JOYSTICK *)p;
	}
	void set_context_clock_status(DEVICE *p, int id, uint32_t mask) {
		register_output_signal(&clock_status, p, id, mask);
	}
	void set_context_printer_reset(DEVICE *p, int id, uint32_t mask) {
		register_output_signal(&printer_reset_bus, p, id, mask);
	}
	void set_context_printer_strobe(DEVICE *p, int id, uint32_t mask) {
		register_output_signal(&printer_strobe_bus, p, id, mask);
	}
	void set_context_printer_select(DEVICE *p, int id, uint32_t mask) {
		register_output_signal(&printer_select_bus, p, id, mask);
	}
	void set_context_irq(DEVICE *p, int id, uint32_t mask) {
		register_output_signal(&irq_bus, p, id, mask);
	}
	void set_context_firq(DEVICE *p, int id, uint32_t mask) {
		register_output_signal(&firq_bus, p, id, mask);
	}
	void set_context_nmi(DEVICE *p, int id, uint32_t mask) {
		register_output_signal(&nmi_bus, p, id, mask);
	}
	
	void set_context_z80cpu(Z80 *p)	{
#ifdef WITH_Z80
		z80 = p;
#endif
	}
	void set_context_jcommcard(DEVICE *p) {
#if defined(CAPABLE_JCOMMCARD)
		jcommcard = (FM7_JCOMMCARD *)p;
#endif
	}
	void set_context_uart(int num, I8251 *p) {
		if(num < 0) return;
		if(num > 2) return;
		uart[num] = p;
		if(p != NULL) {
			uart_enabled[num] = true;
		} else {
			uart_enabled[num] = false;
		}
	}
	void set_context_rs232c_dtr(AND *p) {
# if defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV20EX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		rs232c_dtr = p;
# endif
	}
#if defined(HAS_DMA)
	void set_context_dmac(HD6844 *p) {
		dmac = p;
	}
#endif
};
#endif
