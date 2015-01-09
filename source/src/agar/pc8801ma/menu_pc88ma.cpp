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

void MachineConfigMenu(AG_MenuItem *parent)
{
  AG_MenuItem *item;
  AG_MenuItem *subitem;
   
  item = AG_MenuNode(parent, _N("Boot mode"), NULL);
  subitem = AG_MenuAction(item, _N("N88-V1(S) mode"), NULL, OnBootMode, "%i", 0);
  subitem = AG_MenuAction(item, _N("N88-V1(H) mode"), NULL, OnBootMode, "%i", 1);
  subitem = AG_MenuAction(item, _N("N88-V2 mode"), NULL, OnBootMode, "%i", 2);
  subitem = AG_MenuAction(item, _N("N (N80) mode"), NULL, OnBootMode, "%i", 3);
  
  item = AG_MenuNode(parent, _N("Clock mode"), NULL);
  subitem = AG_MenuAction(item, _N("CPU 8MHz"), NULL, OnCpuType, "%i", 0);
  subitem = AG_MenuAction(item, _N("CPU 4MHz"), NULL, OnCpuType, "%i", 1);

  item = AG_MenuNode(parent, _N("Dip Switch"), NULL);
  subitem = AG_MenuAction(item, _N("Memory Wait"), NULL, OnToggleDipSw, "%i", 0);
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
   ScreenMenu(MenuNode_Sound);
}   
