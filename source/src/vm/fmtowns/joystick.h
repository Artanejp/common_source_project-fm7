/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.01.28 -
    History : 2020.01.28 Initial from FM7.
	[ Towns Joystick ports]

*/

#pragma once

#include "../device.h"
#include <mutex>

// OUTPUT TO PARENT PORT (SOME JOYSTICK DEVICES -> JOYSTICK PORT)
#define SIG_JSPORT_COM		0x01001
#define SIG_JSPORT_DATA		0x01002

#define SIG_JSPORT_PORT1	0x00000000
#define SIG_JSPORT_PORT2	0x00010000
#define SIG_JSPORT_PORT3	0x00020000
#define SIG_JSPORT_PORT4	0x00030000
#define SIG_JSPORT_PORT5	0x00040000
#define SIG_JSPORT_PORT6	0x00050000
#define SIG_JSPORT_PORT7	0x00060000
#define SIG_JSPORT_PORT8	0x00070000

// MSX RELATED PORT CONFIGURATION
// https://www.msx.org/wiki/General_Purpose_port
#define LINE_JOYPORT_UP				(1 << 0) /* IN 1 */
#define LINE_JOYPORT_DOWN			(1 << 1) /* IN 2 */
#define LINE_JOYPORT_LEFT			(1 << 2) /* IN 3 */
#define LINE_JOYPORT_RIGHT			(1 << 3) /* IN 4 */
#define LINE_JOYPORT_A				(1 << 4) /* IN 6 */
#define LINE_JOYPORT_B				(1 << 5) /* IN 7 */
#define LINE_JOYPORT_TRIGGER		(1 << 6) /* OUT 8 */
#define LINE_JOYPORT_DUMMY			(1 << 7) /* DUMMY */
// Belows are dummy define
#define LINE_JOYPORT_POW_PLUS		(1 << 8) /* +5V PIN 5*/
#define LINE_JOYPORT_POW_GND		(1 << 9) /* GND PIN 8*/

namespace FMTOWNS {
class JSDEV_TEMPLATE;
	
class JOYSTICK : public DEVICE
{
private:
	JSDEV_TEMPLATE* d_port[2][8];

	std::mutex _locker;
	
	int port_count[2];
	int port_using[2];

	uint8_t reg_val;
	uint8_t data_reg[2];
	uint8_t data_mask[2];
	
	bool stat_com[2];
	bool stat_triga[2];
	bool stat_trigb[2];
	
	
	virtual void __FASTCALL write_data_to_port(uint8_t data);
	std::unique_lock<std::mutex> lock_device()
	{
		return  std::unique_lock<std::mutex>(_locker, std::adopt_lock);
	}
public:
	JOYSTICK(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int j = 0; j < 2; j++) {
			for(int i = 0; i < 8; i++) {
				d_port[j][i] = NULL;
			}
			port_count[j] = 0;
			port_using[j] = -1;
		}
		set_device_name(_T("FM-Towns JOYSTICK Port"));
	}
	~JOYSTICK() {}
	
	// common functions
	void initialize(void);
	void release();
	void reset();
	
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t __FASTCALL read_signal(int id);

	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);

	// unique functions
	void set_context_joystick(int portnum, JSDEV_TEMPLATE* dev)
	{
		std::unique_lock<std::mutex> _l = lock_device();
		
		if((portnum < 0) || (portnum > 1)) return;
		if(port_count[portnum] < 8) {
			d_port[portnum][port_count[portnum]] = dev;
			port_count[portnum]++;
		}
	}
	void set_using_pad(int portnum, int num)
	{
		port_using[portnum] = num;
	}

			
};
}


