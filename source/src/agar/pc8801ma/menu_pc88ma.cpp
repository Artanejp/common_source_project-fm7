/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.01.09

	[Menu / PC8801MA]
*/

#include <stdio.h>
#include <string>
#include <vector>
#include "common.h"
#include "emu.h"
#include "agar_main.h"
#include "menu_common.h"

AG_MenuItem *MenuNode_Machine = NULL;
#if 0
void AGAR_SelectSoundDevice(AG_MenuItem *parent)
{
   AG_MenuItem *subitem;
   int i;
   
  subitem = AG_MenuAction(item, _N("PSG"), NULL, OnChangeSndDevType, "%i", 0);
  subitem = AG_MenuAction(item, _N("CZ-8BS1 x1"), NULL, OnChangeSndDevType, "%i", 1);
  subitem = AG_MenuAction(item, _N("CZ-8BS1 x2"), NULL, OnChangeSndDevType, "%i", 2);
}
#endif

std::string Str_BootMode[4];
std::string Str_CpuType[2];
std::string Str_DipSw[1];

static volatile void Disp_BootMode(AG_Event *event)
{
  AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
  AG_Menu *menu = (AG_Menu *)AG_SELF();
  int num = AG_INT(1);
   std::string *s = (std::string *)AG_PTR(2);
  
  char str[258];
   
  if((me == NULL) || (menu == NULL)) return;
      AG_ObjectLock(AGOBJECT(menu->root));
      me->state = 1;
      strcpy(str, " ");
      if(config.boot_mode == num) strcpy(str, "*");

      if(s != NULL) strncat(str, s->c_str(), 257);
      AG_MenuSetLabel(me, str);
      AG_ObjectUnlock(AGOBJECT(menu->root));

}

static volatile void Disp_CpuType(AG_Event *event)
{
  AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
  AG_Menu *menu = (AG_Menu *)AG_SELF();
  int num = AG_INT(1);
  std::string *s = (std::string *)AG_PTR(2);
  
  char str[258];
   
  if((me == NULL) || (menu == NULL)) return;
      AG_ObjectLock(AGOBJECT(menu->root));
      me->state = 1;
      strcpy(str, " ");
      if(config.cpu_type == num) strcpy(str, "*");

      strncat(str, s->c_str(), 257);
      AG_MenuSetLabel(me, str);
      AG_ObjectUnlock(AGOBJECT(menu->root));

}

static volatile void Disp_DipSw(AG_Event *event)
{
  AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
  AG_Menu *menu = (AG_Menu *)AG_SELF();
  int num = AG_INT(1);
  std::string *s = (std::string *)AG_PTR(2);
  uint32 mask = (1 << num);
  
  char str[258];
   
  if((me == NULL) || (menu == NULL)) return;
      AG_ObjectLock(AGOBJECT(menu->root));
      me->state = 1;
      strcpy(str, " ");
      if((config.dipswitch & mask) != 0) strcpy(str, "*");

      strncat(str, s->c_str(), 257);
      AG_MenuSetLabel(me, str);
      AG_ObjectUnlock(AGOBJECT(menu->root));

}


void MachineConfigMenu(AG_MenuItem *parent)
{
  AG_MenuItem *item;
  AG_MenuItem *subitem;
  int i;
   
  item = AG_MenuNode(parent, _N("Boot mode"), NULL);
  for(i = 0; i <4; i++) Str_BootMode[i][0] = '\0';
  Str_BootMode[0] = _N("N88-V1(S) mode");
  Str_BootMode[1] = _N("N88-V1(H) mode");
  Str_BootMode[2] = _N("N88-V2 mode");
  Str_BootMode[3] = _N("N (N80) mode");

  for(i = 0; i < 4; i++) {
    subitem = AG_MenuAction(item, Str_BootMode[i].c_str(), NULL, OnBootMode, "%i", i);
     // subitem = MakeDynamicElement(item, Str_BootMode[i].c_str(), NULL,
//					      Disp_BootMode,
//					      OnBootMode,
//					      "%i,%p", i, (void *)&Str_BootMode[i]);
  }    

  item = AG_MenuNode(parent, _N("Clock mode"), NULL);
  Str_CpuType[0] = _N("CPU 8MHz");
  Str_CpuType[1] = _N("CPU 4MHz");
  for(i = 0; i < 2; i++) {
     subitem = AG_MenuAction(item, Str_CpuType[i].c_str(), NULL, OnCpuType, "%i", i);
//      subitem = MakeDynamicElement(item, Str_BootMode[i].c_str(), NULL,
//				   Disp_CpuType,
//				   OnCpuType,
//				   "%i,%p", i, (void *)&Str_CpuType[i]);
  }    
   

  item = AG_MenuNode(parent, _N("Dip Switch"), NULL);
  Str_DipSw[0] = _N("Memory Wait");
  subitem = AG_MenuAction(item, Str_DipSw[0].c_str(), NULL, OnToggleDipSw, "%i", 0);
//  subitem = MakeDynamicElement(item, Str_DipSw[0].c_str(), NULL,
//				   Disp_DipSw,
//				   OnToggleDipSw,
//				   "%i,%p", 0, (void *)&Str_DipSw[0]);
}

void ControlMenu(AG_MenuItem *parent)
{
  AG_MenuItem *item;
  AG_MenuItem *subitem;
  item = AG_MenuAction(parent, _N("Reset"), NULL, OnReset, NULL);
  AG_MenuSeparator(parent);
  
  item = AG_MenuAction(parent, _N("CPU x1"), NULL,  OnCpuPower, "%i", 0);
  item = AG_MenuAction(parent, _N("CPU x2"), NULL,  OnCpuPower, "%i", 1);
  item = AG_MenuAction(parent, _N("CPU x4"), NULL,  OnCpuPower, "%i", 2);
  item = AG_MenuAction(parent, _N("CPU x8"), NULL,  OnCpuPower, "%i", 3);
  item = AG_MenuAction(parent, _N("CPU x16"), NULL, OnCpuPower, "%i", 4);
  AG_MenuSeparator(parent);

#ifdef USE_AUTO_KEY
  item = AG_MenuAction(parent, _N("Paste"), NULL,  OnStartAutoKey, NULL);
  item = AG_MenuAction(parent, _N("Stop"),  NULL,   OnStopAutoKey, NULL);
  AG_MenuSeparator(parent);
#endif
#ifdef USE_STATE
  item = AG_MenuAction(parent, _N("Save State"), NULL,  OnSaveState, NULL);
  item = AG_MenuAction(parent, _N("Load State"), NULL,  OnLoadState, NULL);
  AG_MenuSeparator(parent);
#endif
#ifdef USE_DEBUGGER
  item = AG_MenuAction(parent, _N("Debug Main     CPU"), NULL,  OnOpenDebugger, "%i", 0);
  item = AG_MenuAction(parent, _N("Debug Sub      CPU"), NULL,  OnOpenDebugger, "%i", 1);
  item = AG_MenuAction(parent, _N("Debug Keyboard CPU"), NULL,  OnOpenDebugger, "%i", 2);
  item = AG_MenuAction(parent, _N("Close Debugger"), NULL,  OnCloseDebugger, NULL);
  AG_MenuSeparator(parent);
#endif
  item = AG_MenuAction(parent, _N("Exit Emulator"), NULL,  OnGuiExit, NULL);
  
}

AG_Menu *AGAR_MainMenu(AG_Widget *parent)
{
   AG_Menu *menu = AG_MenuNew(parent, AG_MENU_HFILL);

   MenuNode_Control = AG_MenuNode(menu->root, _N("File"), NULL);
   ControlMenu(MenuNode_Control);

#ifdef USE_FD1
   MenuNode_FD_1.Node = AG_MenuNode(menu->root, _N("Drive 0"), NULL);
   FloppyMenu(&MenuNode_FD_1, 0);
#endif
#ifdef USE_FD2
   MenuNode_FD_2.Node = AG_MenuNode(menu->root, _N("Drive 1"), NULL);
   FloppyMenu(&MenuNode_FD_2, 1);
#endif
#ifdef USE_FD3
   MenuNode_FD_3.Node = AG_MenuNode(menu->root, _N("Drive 2"), NULL);
   FloppyMenu(&MenuNode_FD_3, 2);
#endif
#ifdef USE_FD4
   MenuNode_FD_4.Node = AG_MenuNode(menu->root, _N("Drive 3"), NULL);
   FloppyMenu(&MenuNode_FD_4, 3);
#endif
#ifdef USE_TAPE
   MenuNode_Tape.Node = AG_MenuNode(menu->root, _N("CMT"), NULL);
   TapeMenu(&MenuNode_Tape);
#endif
   MenuNode_Machine = AG_MenuNode(menu->root, _N("Machine"), NULL);
   MachineConfigMenu(MenuNode_Machine);

   MenuNode_Screen = AG_MenuNode(menu->root, _N("Screen"), NULL);
   ScreenMenu(MenuNode_Screen);

   MenuNode_Sound = AG_MenuNode(menu->root, _N("Sound"), NULL);
   SoundMenu(MenuNode_Sound);
}   
