
#include "./joystick.h"
#include "./joypad_2btn.h"
namespace FMTOWNS {

void JOYPAD_2BTN::initialize()
{
	JSDEV_TEMPLATE::initialize();
	pad_type = PAD_TYPE_2BUTTONS;
	set_device_name(_T("FM-Towns 2Buttons PAD #%d"), pad_num + 1);
}
	
uint8_t JOYPAD_2BTN::query(bool& status)
{
	//std::unique_lock<std::mutex> _l = lock_device();
	
	status = false;
	if((pad_num < 0) || (pad_num >= 32)) return 0x00;
	
	const uint32_t* rawdata = emu->get_joy_buffer();
	if(rawdata == nullptr) return 0x00;
	if(!(is_connected)) return 0x00;

	uint32_t data = rawdata[pad_num];
	emu->release_joy_buffer(rawdata);

	bool _sel = ((data & 0x40) != 0) ? true : false;
	bool _run = ((data & 0x80) != 0) ? true : false;
	
	portval_data = data & 0x0f; // AXIS
	if(_sel) {
		portval_data |= (LINE_JOYPORT_LEFT | LINE_JOYPORT_RIGHT);
	}
	if(_run) {
		portval_data |= (LINE_JOYPORT_UP | LINE_JOYPORT_DOWN);
	}
	
	val_trig_a = ((data & 0x10) != 0) ? true : false;
	val_trig_b = ((data & 0x20) != 0) ? true : false;
	//out_debug_log(_T("QUERY JS:%02x %d %d"), portval_data, (val_trig_a) ? 1 : 0, (val_trig_b) ? 1 : 0);
	//unlock_device(_l);
	
	status = true;
	return output_port_signals(false);  // Push results to port;
}
	
uint8_t JOYPAD_2BTN::hook_changed_com(bool changed)
{
	if(changed) {
		// Please write sequences.
		return output_port_com(sig_com, force_output_on_change);
	}
	return 0;
}
	
}
