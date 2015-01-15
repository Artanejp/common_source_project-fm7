/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.01.07

	[agar dialogs]
*/

#include <stdio.h>
#include <string>
#include <vector>
#include "common.h"
#include "emu.h"
#include "qt_main.h"


void CSP_DiskParams::_open_disk(QString s) {
   int d = getDrive();
   printf("Tray to open disk: %s", s.toUtf8().constData());
   printf("Drive = %d\n", d);
   emit do_open_disk(d, s);
}
void CSP_DiskParams::_open_cart(QString s) {
   int d = getDrive();
   emit do_open_cart(d, s);
}
void CSP_DiskParams::_open_cmt(QString s) {
  emit do_open_cmt(play, s);
}

extern "C" 
{
   
#ifdef USE_CART1

   
void open_cart_dialog(QWidget *hWnd, int drv)
{
               CSP_DiskDialog dlg(hWnd);
   
#if defined(_GAMEGEAR)
		QString ext = "*.rom,*.bin,*.gg,*.col";
		QString desc = "Game Cartridge";
#elif defined(_MASTERSYSTEM)
		QString ext = "*.rom,*.bin,*.sms";
		QString desc = "Game Cartridge";
#elif defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
		QString ext = "*.rom,*.bin,*.60";
		QString desc = "Game Cartridge";
#elif defined(_PCENGINE) || defined(_X1TWIN)
		QString ext = "*.rom,*.bin,*.pce";
		QString desc = "HuCARD";
#else
		QString ext = "*.rom,*.bin"; 
		QString desc = "Game Cartridge";
#endif
                QString dirname;
                desc = desc + " (" + ext + ")";
                QStringList filter(desc);
                if(config.initial_cart_dir != NULL) {
		   dirname = config.initial_cart_dir;	        
		} else {
		   _TCHAR app[PATH_MAX];
		   getcwd(app, PATH_MAX);
		   dirname = get_parent_dir(app);
		}
                dlg.param.setDrive(drv);
                dlg.setDirectory(dirname);
                dlg.setNameFilters(filter); 
                QObject::connect(&dlg, SIGNAL(fileSelected(const QString)), dlg.param, SLOT(dlg.param->_open_cart(const QString))); 
                dlg.exec();
}
#endif


#ifdef USE_QD1
void OnOpenQDSub(void)
{
#if 0
   AG_FileType *filetype = (AG_FileType *)AG_PTR(3);
  char *path = AG_STRING(2);
  int drv = AG_INT(1);
  AG_FileDlg *my = (AG_FileDlg *)AG_SELF();
  if(path) {
    if(strlen(path) <= 0) return;
    strncpy(path_shadow, path, AG_PATHNAME_MAX);
    UPDATE_HISTORY(path, config.recent_quickdisk_path[drv]);
    get_parent_dir(path_shadow);
    strcpy(config.initial_quickdisk_dir, path_shadow);
    if(emu) emu->open_quickdisk(drv, path, 0);
  }
#endif
}

void open_quickdisk_dialog(int drv)
{
#if 0
   char path_shadow[AG_PATHNAME_MAX];
  const char *ext = "*.mzt,*.q20,*qdf";
  char *desc = _N("Quick Disk");
  QString dirname;
  AG_Window *win;
   
  win = AG_WindowNew(0);
 
  dlg = AG_FileDlgNew(win, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
  if(dlg == NULL) return;
  
  if(config.initial_quickdisk_dir != NULL) {
    AG_FileDlgSetDirectory(dlg, "%s", config.initial_quickdisk_dir);	        
  } else {
    char app[PATH_MAX];
    QDir df;
    dirname = df.currentPath();
    strncpy(app, dirname.toUtf8().constData(), PATH_MAX);
    dirname = get_parent_dir(app);
  }
  AG_FileDlgAddType(dlg, desc, ext, OnOpenQDSub, "%i", drv);
  AG_WindowShow(win);
  return;
#endif
}
#endif

#ifdef USE_TAPE
void open_tape_dialog(QWidget *hWnd, bool play)
{
  int playf = play ? 1 : 0;
  QString ext;
  QString dirname;
  QString desc;
  QStringList filter(desc);
  if(config.initial_tape_dir != NULL) {
     dirname = config.initial_tape_dir;	        
  } else {
    char app[PATH_MAX];
    QDir df;
    dirname = df.currentPath();
    strncpy(app, dirname.toUtf8().constData(), PATH_MAX);
     dirname = get_parent_dir(app);
  }
   CSP_DiskDialog dlg(hWnd);
   dlg.param->setRecMode(play);
   dlg.setDirectory(dirname);
   dlg.setNameFilters(filter); 
   QObject::connect(&dlg, SIGNAL(fileSelected(QString)), dlg.param, SLOT(dlg.param->_open_cmt(QString))); 
   dlg.exec();
}
#endif
}
#if 0 // !
#ifdef USE_LASER_DISC
void OnOpenLaserDiscSub(AG_Event *event)
{
  AG_FileType *filetype = (AG_FileType *)AG_PTR(2);
  char *path = AG_STRING(1);
  char path_shadow[AG_PATHNAME_MAX];
  AG_FileDlg *my = (AG_FileDlg *)AG_SELF();
  if(path) {
    if(strlen(path) <= 0) return;
    strncpy(path_shadow, path, AG_PATHNAME_MAX);
    UPDATE_HISTORY(path, config.recent_laser_disc_path);
    get_parent_dir(path_shadow);
    strcpy(config.initial_laser_disc_dir, path_shadow);
    if(emu) emu->open_laser_disc(path);
  }
}

void open_laser_disc_dialog(AG_Widget *hWnd)
{
  const char *ext = "*.avi,*.mpg,*.mpeg,*.wmv,*.ogv";
  char *desc = "Laser Disc";
  AG_Window *win;
   
  win = AG_WindowNew(0);
 
  AG_FileDlg *dlg = AG_FileDlgNew(win, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
  if(dlg == NULL) return;
  
  if(config.initial_laser_disc_dir != NULL) {
    AG_FileDlgSetDirectory(dlg, "%s", config.initial_laser_disc_dir);	        
  } else {
    _TCHAR app[AG_PATHNAME_MAX];
    AG_GetCWD(app, AG_PATHNAME_MAX);
    AG_FileDlgSetDirectory(dlg, "%s", get_parent_dir(app));
  }
  AG_FileDlgAddType(dlg, desc, ext, OnOpenLaserDiscSub, "%p", NULL);
  AG_WindowShow(win);
  return;
}
#endif

#ifdef USE_BINARY_FILE1

void OnOpenBinarySub(AG_Event *event)
{
  AG_FileType *filetype = (AG_FileType *)AG_PTR(4);
  char *path = AG_STRING(3);
  char path_shadow[AG_PATHNAME_MAX];
  int drv = AG_INT(1);
  int load = AG_INT(2);
  AG_FileDlg *my = (AG_FileDlg *)AG_SELF();
  if(path) {
    strncpy(path_shadow, path, AG_PATHNAME_MAX);
    UPDATE_HISTORY(path, config.recent_binary_path[drv]);
    get_parent_dir(path_shadow);
    strcpy(config.initial_binary_dir, path_shadow);
    if(load != 0) {
      emu->load_binary(drv, path);
    } else {
      emu->save_binary(drv, path);
    }
  }
}

void open_binary_dialog(AG_Widget *hWnd, int drv, bool load)
{
  const char ext = "*.ram,*.bin";
  AG_Window *win;
   
  int loadf = load ? 1 : 0;
#if defined(_PASOPIA) || defined(_PASOPIA7)
  char *desc = "RAM Pack Cartridge";
#else
  char *desc = "Memory Dump";
#endif
  win = AG_WIndowNew(0);
  AG_FileDlg *dlg = AG_FileDlgNew(win, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
  if(dlg == NULL) return;
  
  if(config.initial_binary_dir != NULL) {
    AG_FileDlgSetDirectory(dlg, "%s", config.initial_binary_dir);	        
  } else {
    _TCHAR app[AG_PATHNAME_MAX];
    AG_GetCWD(app, AG_PATHNAME_MAX);
    AG_FileDlgSetDirectory(dlg, "%s", get_parent_dir(app));
  }
  AG_FileDlgAddType(dlg, desc, ext, OnOpenBinarySub, "%i,%i", drv, loadf);
  AG_WindowShow(win);
  return;

}
#endif
#endif // !

#ifdef SUPPORT_DRAG_DROP
void open_any_file(_TCHAR* path)
{
#if defined(USE_CART1)
	if(check_file_extension(path, _T(".rom")) || 
	   check_file_extension(path, _T(".bin")) || 
	   check_file_extension(path, _T(".gg" )) || 
	   check_file_extension(path, _T(".col")) || 
	   check_file_extension(path, _T(".sms")) || 
	   check_file_extension(path, _T(".60" )) || 
	   check_file_extension(path, _T(".pce"))) {
		UPDATE_HISTORY(path, config.recent_cart_path[0]);
		strcpy(config.initial_cart_dir, get_parent_dir(path));
		emu->open_cart(0, path);
		return;
	}
#endif
#if defined(USE_FD1)
	if(check_file_extension(path, _T(".d88")) || 
	   check_file_extension(path, _T(".d77")) || 
	   check_file_extension(path, _T(".td0")) || 
	   check_file_extension(path, _T(".imd")) || 
	   check_file_extension(path, _T(".dsk")) || 
	   check_file_extension(path, _T(".fdi")) || 
	   check_file_extension(path, _T(".hdm")) || 
	   check_file_extension(path, _T(".tfd")) || 
	   check_file_extension(path, _T(".xdf")) || 
	   check_file_extension(path, _T(".2d" )) || 
	   check_file_extension(path, _T(".sf7"))) {
		UPDATE_HISTORY(path, config.recent_disk_path[0]);
		strcpy(config.initial_disk_dir, get_parent_dir(path));
		open_disk(0, path, 0);
		return;
	}
#endif
#if defined(USE_TAPE)
	if(check_file_extension(path, _T(".wav")) || 
	   check_file_extension(path, _T(".cas")) || 
	   check_file_extension(path, _T(".p6" )) || 
	   check_file_extension(path, _T(".cmt")) || 
	   check_file_extension(path, _T(".n80")) || 
	   check_file_extension(path, _T(".t88")) || 
	   check_file_extension(path, _T(".mzt")) || 
	   check_file_extension(path, _T(".m12")) || 
	   check_file_extension(path, _T(".mti")) || 
	   check_file_extension(path, _T(".mtw")) || 
	   check_file_extension(path, _T(".tap"))) {
		UPDATE_HISTORY(path, config.recent_tape_path);
		strcpy(config.initial_tape_dir, get_parent_dir(path));
		emu->play_tape(path);
		return;
	}
#endif
#if defined(USE_BINARY_FILE1)
	if(check_file_extension(path, _T(".ram")) || 
	   check_file_extension(path, _T(".bin"))) {
		UPDATE_HISTORY(path, config.recent_binary_path[0]);
		strcpy(config.initial_binary_dir, get_parent_dir(path));
		emu->load_binary(0, path);
		return;
	}
#endif
}
#endif

