/*
 * Common Source code Project
 * [UI -> Agar -> Menu -> Sound]
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License : GPLv2
 *     History :
 *      Jan 09, 2015 : Initial
 */


#if 0 // Win32 Ui
#ifdef USE_SOUND_DEVICE_TYPE
		case ID_SOUND_DEVICE_TYPE0:
		case ID_SOUND_DEVICE_TYPE1:
		case ID_SOUND_DEVICE_TYPE2:
		case ID_SOUND_DEVICE_TYPE3:
		case ID_SOUND_DEVICE_TYPE4:
		case ID_SOUND_DEVICE_TYPE5:
		case ID_SOUND_DEVICE_TYPE6:
		case ID_SOUND_DEVICE_TYPE7:
			config.sound_device_type = LOWORD(wParam) - ID_SOUND_DEVICE_TYPE0;
			//if(emu) {
			//	emu->update_config();
			//}
			break;
#endif

#endif // Win32 End

#include <agar/core.h>
#include <agar/gui.h>
#include "menu_common.h"
#include "agar_logger.h"

static const int snd_freq_table[8] = {
   2000, 4000, 8000, 11025, 22050, 44100,
   #ifdef OVERRIDE_SOUND_FREQ_48000HZ
		OVERRIDE_SOUND_FREQ_48000HZ,
#else
		48000,
#endif
		96000,
};

static const double snd_late_table[5] = {0.05, 0.1, 0.2, 0.3, 0.4};


void DoSetVolume(AG_Event *event)
{ 
   AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
   AG_Menu *menu = (AG_Menu *)AG_SELF();

}

void DoRecordSound(AG_Event *event)
{ 
   AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
   AG_Menu *menu = (AG_Menu *)AG_SELF();
   int rec = AG_INT(1);
   
   if(emu) {
	if(rec == 0) {
	   emu->stop_rec_sound();
	} else {
	   emu->start_rec_sound();
	}
   }
}

void DoChangeFrequency(AG_Event *event)
{
   AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
   AG_Menu *menu = (AG_Menu *)AG_SELF();
   int mode = AG_INT(1);
   
   if((mode < 0) || (mode >= 8)) return;
   config.sound_frequency = mode;
   if(emu) {
	emu->update_config();
   }
}

void DoChangeLatency(AG_Event *event)
{
   AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
   AG_Menu *menu = (AG_Menu *)AG_SELF();
   int mode = AG_INT(1);
   
   if((mode < 0) || (mode >= 8)) return;
   config.sound_latency = mode;
   if(emu) {
	emu->update_config();
   }
}

#ifdef USE_SOUND_DEVICE_TYPE
void OnChangeSndDevType(AG_Event *event)
{
   AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
   AG_Menu *menu = (AG_Menu *)AG_SELF();
   int mode = AG_INT(1);
   
   if((mode < 0) || (mode >= 8)) return;
   config.sound_device_type = mode;
   if(emu) {
      	emu->update_config();
   }
}
#endif


void SoundMenu(AG_MenuItem *parent)
{
   AG_MenuItem *item;
   AG_MenuItem *subitem;
   char str[256];
   int i;
   
   item = AG_MenuAction(parent, _N("Sound Volume"), NULL, DoSetVolume, NULL);
   
   item = AG_MenuNode(parent, _N("Recording Sound"), NULL);
   subitem = AG_MenuAction(item, _N("Start Recording"), NULL, DoRecordSound, "%i", 1);
   subitem = AG_MenuAction(item, _N("Stop Recording"), NULL, DoRecordSound, "%i", 0);
   AG_MenuSeparator(parent);
   
   item = AG_MenuNode(parent, _N("Sound frequency"), NULL);
   for(i = 0; i < 8; i++) {
      snprintf(str, 255, "%dHz", snd_freq_table[i]);
      subitem = AG_MenuAction(item, str, NULL, DoChangeFrequency, "%i", i);
   }
   item = AG_MenuNode(parent, _N("Sound latency"), NULL);
   for(i = 0; i < 5; i++) {
      snprintf(str, 255, "%d msec", (int)(snd_late_table[i] * 1000));
      subitem = AG_MenuAction(item, str, NULL, DoChangeLatency, "%i", i);
   }
#ifdef USE_SOUND_DEVICE_TYPE
   AG_MenuSeparator(parent);
   item = AG_MenuNode(parent, _N("Sound Device Type"), NULL);
   AGAR_SelectSoundDevice(item);
#endif
}

   
   
				     
