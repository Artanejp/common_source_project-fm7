
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
class QSettings;
class USING_FLAGS_EXT : public USING_FLAGS
{
private:

public:
	USING_FLAGS_EXT(config_t *cfg, QSettings* settings);
	~USING_FLAGS_EXT();
	const _TCHAR *get_joy_button_captions(int num) override;
	const _TCHAR *get_sound_device_caption(int num) override;

};


#endif //#ifndef __CSP_QT_COMMON_MENU_FLAGS_H
