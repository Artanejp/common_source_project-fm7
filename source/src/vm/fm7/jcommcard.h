/*
 * Emulation of Fujitsu Japanese Communication Card.
 * (C) 2018 K.Ohta.
 * Note:
 * Based on XM7 L70 , with permittion from Ryu Takegami. 
 */

#include "../device.h"
#include "../../common.h"

class MC6809;

class FM7_JCOMMCARD : public DEVICE {
private:
	MC6809 *cpu;
	
	uint8_t n_bank;
	uint8_t rcb_address;
	pair_t kanji_address;
	
	bool jis78_emulation;

	bool halted;

	bool modified;
	bool firmware_ok;
	
	uint8_t prog_rom[0x4000];
	uint8_t dict_rom[0x40000];
	uint8_t kanji_rom[0x20000];
	uint8_t backup_ram[0x2000];
	
	bool patch_jis78(void);
public:
	FM7_JCOMMCARD(VM *parent_vm, EMU *parent_emu);
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
	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);

	
};

