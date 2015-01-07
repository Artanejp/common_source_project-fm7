/*
 * Emulation event handler for Common Sourcecode Project / Agar.
 * (C) K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History : Jan 06, 2015 : Initial
 */


#include <agar/core.h>
#include <agar/gui.h>
#include "simd_types.h"
#include "agar_main.h"


extern void OnReset(AG_Event *event);
extern void OnSpecialReset(AG_Event *event);
extern void OnCpuPower(AG_Event *event); // int cpupower
extern void OnGuiExit(AG_Event *event);

#ifdef USE_STATE
extern void OnSaveState(AG_Event *event);
extern void OnLoadState(AG_Event *event);
#endif

#ifdef USE_BOOT_MODE
extern void OnBootMode(AG_Event *event); // int mode
#endif

#ifdef USE_CPU_TYPE
extern void OnCpuType(AG_Event *event); // int cpytype
#endif

#ifdef USE_DIPSWITCH
extern void OnToggleDipSw(AG_Event *event); // int bit ; 0 to 31.
extern void OnChangeDipSw(AG_Event *event); // int bit, int flag
#endif

#ifdef USE_DEVICE_TYPE
extern void OnSetDeviceType(AG_Event *event); //int devtype
#endif

#ifdef USE_AUTO_KEY
extern void OnStartAutoKey(AG_Event *event);
extern void OnStopAutoKey(AG_Event *event);
#endif

#ifdef USE_DEBUGGER
extern void OnOpenDebugger(AG_Event *event); // int num
extern void OnCloseDebugger(AG_Event *event);
#endif

#if defined(USE_CART1) || defined(USE_CART2)
extern void OnOpenCart(AG_Event *event); // int slot
extern void OnCloseCart(AG_Event *event); // int slot
extern void OnRecentCart(AG_Event *event); // int slot, int menunum
#endif
  

#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
extern void OnOpenFD(AG_Event *event); // int drive
extern void OnCloseFD(AG_Event *event); // int drive
extern void OnRecentFD(AG_Event *event); // int drv, int num
extern void OnSelectD88Bank(AG_Event *event); // int drv, int d88num
extern void OpenRecentFloppy(AG_Event *event); // int drv, int num
#endif

#if defined(USE_QD1) || defined(USE_QD2)
extern void OnOpenQD(AG_Event *event); // int drive
extern void OnCloseQD(AG_Event *event); // int drive
extern void OnRecentQD(AG_Event *event); // int drive, int num
#endif

#ifdef USE_TAPE
extern void OnPlayTAPE(AG_Event *event); 
extern void OnRecTAPE(AG_Event *event);
extern void OnCloseTAPE(AG_Event *event);
extern void OnUseWaveShaperTAPE(AG_Event *event);
extern void OnDirectLoadMZT(AG_Event *event);
extern void OnRecentTAPE(AG_Event *event); // int menunum
#endif

#ifdef USE_TAPE_BUTTON
extern void OnPushPlayButton(AG_Event *event); //
extern void OnPushStopButton(AG_Event *event);
#endif

// Record screen to movie.
extern void OnStartRecordScreen(AG_Event *event); // int fpsnum; 0=60fps, 1=30fps, 2=15fps
extern void OnStopRecordScreen(AG_Event *event);

// Capture screen to bmp.
extern void OnScreenCapture(AG_Event *event);

//Change Screen mode
extern void OnSetScreenMode(AG_Event *event); // int mode
extern void OnFullScreen(AG_Event *event);
extern void OnSetStretchMode(AG_Event *event); // int mode ; 0 = DOT, 1 = ASPECT, 2 = FILL.







