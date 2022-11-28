/*
 * Emulation of Fujitsu Japanese Communication Card.
 * (C) 2018 K.Ohta.
 * Note:
 * Based on XM7 L70 , with permittion from Ryu Takegami. 
 */


#if !defined(___CSP_FM7_JCOMM_CARD_H)
#define ___CSP_FM7_JCOMM_CARD_H

#include "../device.h"
#include "../../common.h"

class MC6809;

class FM7_JCOMMCARD : public DEVICE {
private:
	MC6809 *cpu;
   
	uint8_t n_bank;
	uint8_t rcb_address;
	pair32_t kanji_address;
	
	bool diag_dictrom_load;
	
	bool halted;

	//bool modified;
	bool firmware_ok;
	
	uint8_t prog_rom[0x4000];
	uint8_t dict_rom[0x60000]; // Is this right? Is not size 0x80000? 20180216 K.O
	uint8_t p_ram[0x2000];
	
public:
	FM7_JCOMMCARD(VM_TEMPLATE* parent_vm, EMU *parent_emu);
	~FM7_JCOMMCARD();
	void initialize(void);
	void release(void);
	
	void reset(void);
	void write_signal(int id, uint32_t data, uint32_t mask);
	/* 
	 *  I/O port:
     * Read:   $FD28 : SYNC Flag (JSUB HALTED = 0x7F)
     *         $FD29 : RCB Data.
     *         $FD2A : KANJI ROM UPPER
     *         $FD2B : KANJI ROM LOWER
     *
     * Write : $FD28 : KANJI ROM ADDRESS LOWER
     *         $FD29 : KANJI ROM ADDRESS UPPER
     *         $FD2A : Bit7: "0" = HALTREQ. Clear address.
     *         $FD2B : Write Data to RCB.
     */
	uint32_t read_io8(uint32_t address);
	void write_io8(uint32_t address, uint32_t data);
	uint32_t read_data8(uint32_t address);
	void write_data8(uint32_t address, uint32_t data);

	void set_context_cpu(MC6809 *p)	{
		cpu = p;
	}
	bool decl_state(FILEIO *state_fio, bool loading);
	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);
};

#endif  /* ___CSP_FM7_JCOMM_CARD_H  */
