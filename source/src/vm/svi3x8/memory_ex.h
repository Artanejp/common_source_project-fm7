/*
	Common Source Code Project
	SVI-3x8

	Origin : src/vm/msx/memory.h

	modified by tanam
	Date   : 2018.12.09-

	[ memory ]
*/

#ifndef _MEMORY_EX_H_
#define _MEMORY_EX_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_SEL	0

#if defined(FDD_PATCH_SLOT)
class DISK;
#endif

#define MAX_TAPE_LEN 524288

// memory bus
class MEMORY_EX : public DEVICE
{
private:
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t bio[0x8000];  /* BANK01 */
	uint8_t ram[0x8000];  /* BANK02 */
	uint8_t rom[0x8000];  /* BANK11 */
	uint8_t r12[0x8000];  /* BANK12 */
	uint8_t r21[0x8000];  /* BANK21 */
	uint8_t r22[0x8000];  /* BANK22 */
	uint8_t r31[0x8000];  /* BANK31 */
	uint8_t r32[0x8000];  /* BANK32 */
	bool inserted;
	bool play;
	uint8_t strig;
#if defined(FDD_PATCH_SLOT)
	DISK* disk[MAX_DRIVE];
	DEVICE *d_fdpat;
	bool access[MAX_DRIVE];
#endif
	int count;
	int done;
	int tapePos;
	int tapeLen;
	byte tapedata[MAX_TAPE_LEN];

public:
	MEMORY_EX(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY_EX() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	uint32_t fetch_op(uint32_t addr, int* wait);
	bool process_state(FILEIO* state_fio, bool loading);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_io8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	// unique functions
	void open_cart(const _TCHAR *file_path);
	void close_cart();
	bool load_cart(const _TCHAR *file_path/*, uint8_t *rom*/);
	bool is_cart_inserted()
	{
		return inserted;
	}
	bool play_tape(const _TCHAR* file_path);
//	bool rec_tape(const _TCHAR* file_path);
	void close_tape();
#if defined(FDD_PATCH_SLOT)
	void release();
	void set_context_fdd_patch(DEVICE *device)
	{
		d_fdpat = device;
	}
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool is_disk_inserted(int drv);
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);
#endif
	bool is_tape_inserted()
	{
		return play;
	}
	const _TCHAR* get_message()
	{
		if (play) return "Play";
		else return "Stop";
	}
};

#endif
