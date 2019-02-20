
#ifndef __CSP_QT_COMMON_MENU_FLAGS_EXT_H
#define __CSP_QT_COMMON_MENU_FLAGS_EXT_H

#include <QString>
#include "common.h"
#include "config.h"
#include "../gui/menu_flags.h"

#ifndef _SCREEN_MODE_NUM
#define _SCREEN_MODE_NUM 32
#endif

class EMU;
class OSD;
class USING_FLAGS_EXT : public USING_FLAGS
{
private:
public:
	USING_FLAGS_EXT(config_t *cfg);
	~USING_FLAGS_EXT();
	const _TCHAR *get_joy_button_captions(int num);
	const _TCHAR *get_sound_device_caption(int num);
	int get_s_freq_table(int num);
	int get_vm_node_size();
	void set_vm_node_name(int id, const _TCHAR *name);
	_TCHAR *get_vm_node_name(int id);
	const _TCHAR *get_sound_device_name(int num);
	int get_sound_device_num();
	
};
	

#endif //#ifndef __CSP_QT_COMMON_MENU_FLAGS_H
