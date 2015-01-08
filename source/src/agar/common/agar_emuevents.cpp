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

#ifdef USE_FD1
void OpenRecentFloppy(AG_Event *event)
{
  char path[4096];
  int i;
  int drv = AG_INT(1);
  int num = AG_INT(2);
  
  if((num < 0) || (num > 7)) return;
  strcpy(path, config.recent_disk_path[drv][num]);
  for(int i = num; i > 0; i--) {
    strcpy(config.recent_disk_path[drv][i], config.recent_disk_path[drv][i - 1]);
  }
  strcpy(config.recent_disk_path[drv][0], path);
  if(emu) {
    open_disk(drv, path, 0);
  }
}
#endif
// Need to implement Socket routines
void OnReset(AG_Event *event)
{

  if(emu) {
    emu->reset();
  }
}

void OnSpecialReset(AG_Event *event)
{
  printf("Special Reset!!\n");
  if(emu) {
    emu->special_reset();
  }
}

#ifdef USE_STATE
void OnSaveState(AG_Event *event)
{
  if(emu) {
    emu->save_state();
  }
}

void OnLoadState(AG_Event *event)
{
  if(emu) {
    emu->load_state();
  }
}
#endif
#ifdef USE_BOOT_MODE
void OnBootMode(AG_Event *event)
{
  int mode = AG_INT(1);
  config.boot_mode = mode;
  if(emu) {
    emu->update_config();
  }
}
#endif
#ifdef USE_CPU_TYPE
void OnCpuType(AG_Event *event)
{
  int cputype = AG_INT(1);
  config.cpu_type = cputype;
  if(emu) {
    emu->update_config();
  }
}
#endif

void OnCpuPower(AG_Event *event)
{
  int cpupower = AG_INT(1);
  config.cpu_power = cpupower;
  if(emu) {
    emu->update_config();
  }
}

#ifdef USE_DIPSWITCH
void OnToggleDipSw(AG_Event *event)
{
  int dipsw = AG_INT(1);
  if((dipsw < 0) || (dipsw > 31)) return;
  config.dipswitch ^= (1 << dipsw);
}
void OnChangeDipSw(AG_Event *event)
{
  int dipsw = AG_INT(1);
  int flag  = AG_INT(2);
  
  if((dipsw < 0) || (dipsw > 31)) return;
  if(flag == 0) {
    config.dipswitch &= ~(1 << dipsw);
  } else {
    config.dipswitch |= (1 << dipsw);
  }
}
#endif
#ifdef USE_DEVICE_TYPE
void OnSetDeviceType(AG_Event *event)
{
  int devtype = AG_INT(1);
  if((devtype < 0) || (devtype > 7)) return;
  config.device_type = devtype;
}
#endif
#ifdef USE_AUTO_KEY
void OnStartAutoKey(AG_Event *event)
{
  if(emu) {
    emu->start_auto_key();
  }
}
void OnStopAutoKey(AG_Event *event)
{
  if(emu) {
    emu->stop_auto_key();
  }
}
#endif
#ifdef USE_DEBUGGER
void OnOpenDebugger(AG_Event *event)
{
  int no = AG_INT(1);
  if((no < 0) || (no > 3)) return;
  if(emu) emu->open_debugger(no);
}
void OnCloseDebugger(AG_Event *event)
{
  if(emu) emu->close_debugger();
}
#endif

void OnGuiExit(AG_Event *event)
{
  if(hWindow != NULL) AG_PostEvent(AG_SELF(), AGOBJECT(hWindow), "window-close", "%p", NULL);
}
#if defined(USE_CART1) || defined(USE_CART2)
void OnOpenCart(AG_Event *event)
{
  AG_Widget *my = AG_SELF();
  int drive = AG_INT(1);
  if(emu) open_cart_dialog(hWindow, drive);
}

void OnCloseCart(AG_Event *event)
{
  AG_Widget *my = AG_SELF();
  int drive = AG_INT(1);
  if(emu) emu->close_cart(drive);
}

void OnRecentCart(AG_Event *event)
{
  AG_Widget *my = AG_SELF();
  int drive = AG_INT(1);
  int menunum = AG_INT(2);
  char path[4096];
  int i;
  if(drive < 0) return;
#if !defined(USE_CART2)
  if(drive > 0) return;
#else
  if(drive > 1) return;
#endif
  if((menunum < 0) || (menunum > 7)) return;
  strcpy(path, config.recent_cart_path[drv][menunum]);
  for(int i = menunum; i > 0; i--) {
    strcpy(config.recent_cart_path[drive][i], config.recent_cart_path[drive][i - 1]);
  }
  strcpy(config.recent_cart_path[drive][0], path);
  if(emu) {
    emu->open_cart(drive, path);
  }
}
#endif

#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
void OnOpenFD(AG_Event *event)
{
  int drive = AG_INT(1);
  if(emu) open_disk_dialog(AGWIDGET(hWindow), drive);
}

void OnCloseFD(AG_Event *event)
{
  int drive = AG_INT(1);
  if(emu) close_disk(drive);
}
void OnRecentFD(AG_Event *event)
{
  char path[4096];
  int drive = AG_INT(1);
  int menunum = AG_INT(2);
  int i;

  if((menunum < 0) || (menunum > 7)) return;
  strcpy(path, config.recent_disk_path[drive][menunum]);
  for(int i = menunum; i > 0; i--) {
    strcpy(config.recent_disk_path[drive][i], config.recent_disk_path[drive][i - 1]);
  }
  strcpy(config.recent_disk_path[drive][0], path);
  if(emu) {
    open_disk(drive, path, 0);
  }
}

void OnSelectD88Bank(AG_Event *event)
{
  int drive = AG_INT(1);
  int no = AG_INT(2);

  if((no < 0) || (no > 63)) return;
  if(emu && emu->d88_file[drive].cur_bank != no) {
    emu->open_disk(drive, emu->d88_file[drive].path, emu->d88_file[drive].bank[no].offset);
    emu->d88_file[drive].cur_bank = no;
  }
}
#endif

#if defined(USE_QD1) || defined(USE_QD2)
void OnOpenQD(AG_Event *event)
{
  int drive = AG_INT(1);
  if(emu) open_quickdisk_dialog(AGWIDGET(hWindow), drive);
}

void OnCloseQD(AG_Event *event)
{
  int drive = AG_INT(1);
  
  if(emu) emu->close_quickdisk(drive);
}
void OnRecentQD(AG_Event *event)
{
  char path[4096];
  int drive = AG_INT(1);
  int menunum = AG_INT(2);
  int i;

  if((menunum < 0) || (menunum > 7)) return;
  strcpy(path, config.recent_quickdisk_path[drive][menunum]);
  for(int i = menunum; i > 0; i--) {
    strcpy(config.recent_quickdisk_path[drive][i], config.recent_quickdisk_path[drive][i - 1]);
  }
  strcpy(config.recent_quickdisk_path[drive][0], path);
  if(emu) {
    emu->open_quickdisk(drive, path);
  }
}
#endif
#ifdef USE_TAPE
void OnPlayTAPE(AG_Event *event)
{
  if(emu) open_tape_dialog(AGWIDGET(hWindow), true);
}

void OnRecTAPE(AG_Event *event)
{
  if(emu) open_tape_dialog(AGWIDGET(hWindow), false);
}

void OnCloseTAPE(AG_Event *event)
{
  if(emu) emu->close_tape();
}

void OnUseWaveShaperTAPE(AG_Event *event)
{
  config.wave_shaper = !config.wave_shaper;
}

void OnDirectLoadMZT(AG_Event *event)
{
  config.direct_load_mzt = !config.direct_load_mzt;
}
void OnRecentTAPE(AG_Event *event)
{
  char path[4096];
  int menunum = AG_INT(1);
  int i;

  if((menunum < 0) || (menunum > 7)) return;
  strcpy(path, config.recent_tape_path[menunum]);
  for(int i = menunum; i > 0; i--) {
    strcpy(config.recent_tape_path[i], config.recent_tape_path[i - 1]);
  }
  strcpy(config.recent_tape_path[0], path);
  if(emu) {
    emu->play_tape(path);
  }
}
#endif
#ifdef USE_TAPE_BUTTON
void OnPushPlayButton(AG_Event *event)
{
  if(emu) emu->push_play();
}
void OnPushStopButton(AG_Event *event)
{
  if(emu) emu->push_stop();
}
#endif
// Implement LASER-DISC, BINARY
//

void OnStartRecordScreen(AG_Event *event)
{
  int num = AG_INT(1);
  const int fps[3] = {60, 30, 15};
  if((num < 0) || (num > 2)) return;
  if(emu) {
    emu->start_rec_sound();
    if(!emu->start_rec_video(fps[num])) {
      emu->stop_rec_sound();
    }
  }
}
void OnStopRecordScreen(AG_Event *event)
{
  if(emu) {
    emu->stop_rec_video();
    emu->stop_rec_sound();
  }
}

void OnScreenCapture(AG_Event *event)
{
  if(emu) emu->capture_screen();
}

void OnSetScreenMode(AG_Event *event)
{
  int mode = AG_INT(1);
  if((mode < 0) || (mode > 7)) return;
  if(emu){
    set_window(AGWIDGET(hWindow), mode);
  }
}

void OnFullScreen(AG_Event *event)
{
}

void OnSetStretchMode(AG_Event *event)
{
  int mode = AG_INT(1);
  if((mode < 0) || (mode > 2)) return;
  // 0 = DOT
  // 1 = ASPECT
  // 2 = FILL
  config.stretch_type = mode;
  // On Common Sourcecode Project / Agar,
  // Scaling is done by Agar Widget.
  // So, does need below action?
  // Maybe, needs Agar's changing action. 
  if(emu) {
    emu->set_display_size(-1, -1, false);
  }
}

