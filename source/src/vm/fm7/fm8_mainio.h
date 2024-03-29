/*
 * FM-8 Main I/O [fm7_mainio.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jan 03, 2015 : Initial
 *   Jan 25, 2018 : Split FM-8 from fm7_mainio.h .
 */

#ifndef _VM_FM8_MAINIO_H_
#define _VM_FM8_MAINIO_H_

#include "./fm7_mainio.h"

namespace FM7 {
	class BUBBLECASETTE;
}

namespace FM7 {
class FM8_MAINIO : public FM7_MAINIO {
 protected:
	FM7::BUBBLECASETTE *bubble_casette[2];
	uint8_t get_port_fd00(void) override;
	void set_port_fd02(uint8_t val) override;
	uint8_t get_irqstat_fd03(void) override;
	uint8_t get_extirq_fd17(void) override;
	void set_ext_fd17(uint8_t data) override;
	
	void set_irq_syndet(bool flag) override;
	void set_irq_rxrdy(bool flag) override;
	void set_irq_txrdy(bool flag) override;
	void set_irq_timer(bool flag) override;
	void set_irq_printer(bool flag) override;
	void set_irq_mfd(bool flag) override;
	void do_irq(void) override;
	void __FASTCALL write_fd0f(void) override;
	uint8_t __FASTCALL read_fd0f(void) override;

	void reset_sound(void) override;
	void set_psg(uint8_t val) override;
	uint8_t get_psg(void) override;
	void set_psg_cmd(uint8_t cmd) override;
	void write_opn_reg(int index, uint32_t addr, uint32_t data) override;
	void set_opn(int index, uint8_t val) override;
	uint8_t get_opn(int index) override;
	void set_opn_cmd(int index, uint8_t cmd) override;
	uint8_t get_extirq_whg(void) override;
	uint8_t get_extirq_thg(void) override;
public:
	FM8_MAINIO(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu);
	~FM8_MAINIO();
	
	void __FASTCALL write_data8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_data8(uint32_t addr) override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;

	void __FASTCALL event_callback(int event_id, int err) override;
	void reset() override;
	void initialize() override;
	void update_config() override;
	bool process_state(FILEIO *state_fio, bool loading) override;

	void set_context_kanjirom_class2(DEVICE *p) override
	{
	}
# if defined(USE_AY_3_8910_AS_PSG)
	void set_context_psg(AY_3_891X *p) override
	{
		psg = p;
		connect_psg = true;
	}
# else
	void set_context_psg(YM2203 *p) override
	{
		psg = p;
		connect_psg = true;
	}
# endif
	void set_context_bubble(BUBBLECASETTE *p, int drive) {
		if(drive > 2) return;
		bubble_casette[drive] = p;
	}
};	

}
#endif
