#include "./joystick.h"
#include "./joypad_6btn.h"
namespace FMTOWNS {

void JOYPAD_6BTN::initialize()
{
	JSDEV_TEMPLATE::initialize();
	pad_type = PAD_TYPE_6BUTTONS;
}

uint8_t JOYPAD_6BTN::query(bool& status)
{
	//std::unique_lock<std::mutex> _l = lock_device();
	
	status = false;
	if((pad_num < 0) || (pad_num >= 32)) return 0x00;
	
	const uint32_t* rawdata = emu->get_joy_buffer();
	if(rawdata == nullptr) return 0x00;
	if(!(is_connected)) return 0x00;

	uint32_t data = rawdata[pad_num];
	emu->release_joy_buffer(rawdata);

	if(sig_com) { // 6 BUTTONS
		uint8_t buttons = (data >> 8) & 0x0f;
		portval_data = 0x00;
		if((buttons & 0x01) != 0) portval_data |= LINE_JOYPORT_RIGHT;
		if((buttons & 0x02) != 0) portval_data |= LINE_JOYPORT_LEFT;
		if((buttons & 0x04) != 0) portval_data |= LINE_JOYPORT_DOWN;
		if((buttons & 0x08) != 0) portval_data |= LINE_JOYPORT_UP;
	} else {
		portval_data = data & 0x0f; // AXIS

		bool _sel = ((data & 0x40) != 0) ? true : false;
		bool _run = ((data & 0x80) != 0) ? true : false;
		if(_sel) {
			portval_data |= (LINE_JOYPORT_LEFT | LINE_JOYPORT_RIGHT);
		}
		if(_run) {
			portval_data |= (LINE_JOYPORT_UP | LINE_JOYPORT_DOWN);
		}
	}
	val_trig_a = ((data & 0x10) != 0) ? true : false;
	val_trig_b = ((data & 0x20) != 0) ? true : false;

	status = true;
	//unlock_device(_l);
	
	return output_port_signals(false);  // Push results to port;
}
	

}
