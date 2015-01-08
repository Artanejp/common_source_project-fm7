/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.01.08

	[agar screen manu]
*/
#include "menu_common.h"
#include "agar_logger.h"


static void DoMultiplyWindowed(AG_Event *event)
{
   int mode = AG_INT(1);
   
   if((mode < 0) || (mode >= 8)) return;
   if(emu) {
      emu->suspend();
      set_window(hScreenWidget, mode);
   }
   
}

static void DoChangeStretch(AG_Event *event)
{
   int mode = AG_INT(1);
   
   if((mode < 0) || (mode > 2)) return;
   config.stretch_type = mode;
   if(emu){
      emu->set_display_size(-1, -1, true);
   }
}

static volatile void Display_Multiply(AG_Event *event)
{
  AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
  AG_Menu *menu = (AG_Menu *)AG_SELF();
  int mode = AG_INT(1);
  char str[128];
  snprintf(str, 127, "x%d", mode);
  AG_MenuSetLabel(me, str);
  me->state = 1;
}
   
static volatile void Display_Stretch(AG_Event *event)
{
  AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
  AG_Menu *menu = (AG_Menu *)AG_SELF();
  int mode = AG_INT(1);
  char str[128];
  switch(mode) {
   case 0:
	strncpy(str, _N("Dot by Dot"), 127);
        break;
   case 1:
	strncpy(str, _N("Keep Aspect"), 127);
        break;
   case 2:
	strncpy(str, _N("Fill screen"), 127);
        break;
   default:
        return;
  }
  AG_MenuSetLabel(me, str);
  me->state = 1; 
}
   


void ScreenMenu(AG_MenuItem *parent)
{
   AG_MenuItem *item;
   AG_MenuItem *subitem;
   char str[256];
   int i;
   
   item = AG_MenuNode(parent, _N("Window Multiply"), NULL);
   for(i = 0; i < 8; i++) {
      snprintf(str, 255, "%s x%d", _N("Multiply"), i + 1);
      subitem = AG_MenuAction(item, str, NULL, DoMultiplyWindowed, "%i", i);
//        subitem = MakeDynamicElement(item, str, NULL, 
//				     Display_Multiply, DoMultiplyWindowed, "%i", i);
   }
   AG_MenuSeparator(parent);
   item = AG_MenuNode(parent, _N("Stretch Mode"), NULL);
   subitem = AG_MenuAction(item, _N("Dot by Dot"), NULL, DoChangeStretch, "%i", 0);
   subitem = AG_MenuAction(item, _N("Keep Aspect"), NULL, DoChangeStretch, "%i", 1);
   subitem = AG_MenuAction(item, _N("Fill screen"), NULL, DoChangeStretch, "%i", 2);
   AG_MenuSeparator(parent);
}

   
   
				     
