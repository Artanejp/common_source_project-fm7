

#include "qt_emuevents.h"
#include "qt_main.h"
#include "qt_dialogs.h"
#include "emu_utils.h"
#include "agar_logger.h"

extern EMU *emu;

void Ui_MainWindow::OnReset(void)
{
    AGAR_DebugLog(AGAR_LOG_INFO, "Reset");
    if(emu) emu->reset();
}
  void Ui_MainWindow::OnSpecialReset(void)
  {
#ifdef USE_SPECIAL_RESET
    AGAR_DebugLog(AGAR_LOG_INFO, "Special Reset");
    if(emu) emu->special_reset();
#endif
  }
#ifdef USE_STATE
  void Ui_MainWindow::OnLoadState(void) // Final entry of load state.
  {
    if(emu) emu->load_state();
  }
  
  void Ui_MainWindow::OnSaveState(void)
  {
    if(emu) emu->save_state();
  }
#endif
#ifdef USE_BOOT_MODE
#endif

#ifdef USE_CPU_TYPE
#endif

void Ui_MainWindow::OnCpuPower(int mode)
{
  config.cpu_power = mode;
  if(emu) {
    emu->update_config();
  }
}

#ifdef USE_AUTO_KEY
void Ui_MainWindow::OnStartAutoKey(void)
{
  if(emu) {
    emu->start_auto_key();
  }
}
void Ui_MainWindow::OnStopAutoKey(void)
{
  if(emu) {
    emu->stop_auto_key();
  }
}
#endif
#ifdef USE_DEBUGGER
 void Ui_MainWindow::OnOpenDebugger(int no)
 {
   if((no < 0) || (no > 3)) return;
   emu->open_debugger(no);
 }
void Ui_MainWindow::OnCloseDebugger(void )
 {
   emu->close_debugger();
 }
#endif


// Will move to other file.
#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
//void OpenRecentFloppy(QWidget *parent, int drv, int num)

void OnCloseFD(int drive)
{
  if(emu) close_disk(drive);
}
// Use Dialog
#endif

#ifdef USE_DIPSWITCH
void OnToggleDipSw(int dipsw)
{
  if((dipsw < 0) || (dipsw > 31)) return;
  config.dipswitch ^= (1 << dipsw);
}
void OnChangeDipSw(int dipsw, int flag)
{
 
  if((dipsw < 0) || (dipsw > 31)) return;
  if(flag == 0) {
    config.dipswitch &= ~(1 << dipsw);
  } else {
    config.dipswitch |= (1 << dipsw);
  }
}
#endif
#ifdef USE_DEVICE_TYPE
void OnSetDeviceType(int devtype)
{
  if((devtype < 0) || (devtype > 7)) return;
  config.device_type = devtype;
}
#endif

#if defined(USE_CART1) || defined(USE_CART2)
#endif


#if defined(USE_QD1) || defined(USE_QD2)
void OnOpenQD(QWidget *parent, int drive)
{
//  if(emu) open_quickdisk_dialog(AGWIDGET(hWindow), drive);
}

void OnCloseQD(int drive)
{
//  if(emu) emu->close_quickdisk(drive);
}

void OnRecentQD(int drive, int menunum)
{
  std::string path;
  int i;

  if((menunum < 0) || (menunum > 7)) return;
  path = config.recent_quickdisk_path[drive][menunum];
  
  for(int i = menunum; i > 0; i--) {
    strcpy(config.recent_quickdisk_path[drive][i], config.recent_quickdisk_path[drive][i - 1]);
  }
  strcpy(config.recent_quickdisk_path[drive][0], path.c_str());
  if(emu) {
    emu->open_quickdisk(drive, (_TCHAR *)(path.c_str()));
  }
}
#endif
#ifdef USE_TAPE

#endif
#ifdef USE_TAPE_BUTTON
#endif
// Implement LASER-DISC, BINARY
//

void OnStartRecordScreen(int num)
{

  const int fps[3] = {60, 30, 15};
  if((num < 0) || (num > 2)) return;
  if(emu) {
    emu->start_rec_sound();
    if(!emu->start_rec_video(fps[num])) {
      emu->stop_rec_sound();
    }
  }
}
void OnStopRecordScreen(void)
{
  if(emu) {
    emu->stop_rec_video();
    emu->stop_rec_sound();
  }
}

void OnScreenCapture(QWidget *parent)
{
  if(emu) emu->capture_screen();
}




void OnFullScreen(QMainWindow *MainWindow, QWidget *drawspace, int mode)
{
}

