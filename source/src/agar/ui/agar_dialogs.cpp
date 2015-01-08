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
#include "agar_main.h"


#ifdef USE_CART1
static void OnOpenCartSub(AG_Event *event)
{
  char *path = AG_STRING(2);
  char path_shadow[AG_PATHNAME_MAX];
   
  AG_FileType *filetype = (AG_FileType *)AG_PTR(3);
  int drv = AG_INT(1);
  AG_FileDlg *my = (AG_FileDlg *)AG_SELF();
  if(path) {
    if(strlen(path) <= 0) return;
    strncpy(path_shadow, path, AG_PATHNAME_MAX);
    UPDATE_HISTORY(path, config.recent_cart_path[drv]);
    get_parent_dir(path_shadow);
    strcpy(config.initial_cart_dir, path_shadow);
    if(emu) emu->open_cart(drv, path);
  }
}


void open_cart_dialog(AG_Widget *hWnd, int drv)
{
  AG_FileDlg *dlg;
  AG_Window *win;
#if defined(_GAMEGEAR)
		const char *ext = "*.rom,*.bin,*.gg,*.col";
		char *desc = _N("Game Cartridge");
#elif defined(_MASTERSYSTEM)
		const char *ext = "*.rom,*.bin,*.sms";
		char *desc = _N("Game Cartridge");
#elif defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
		const char *ext = "*.rom,*.bin,*.60";
		char *desc = _N("Game Cartridge");
#elif defined(_PCENGINE) || defined(_X1TWIN)
		const char *ext = "*.rom,*.bin,*.pce";
		char *desc = _N("HuCARD");
#else
		const char *ext = "*.rom,*.bin"; 
		char *desc = _N("Game Cartridge");
#endif
                win = AG_WindowNew(0);
		dlg = AG_FileDlgNew(win, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
		if(dlg == NULL) return;
	
		if(config.initial_cart_dir != NULL) {
		  AG_FileDlgSetDirectory(dlg, "%s", config.initial_cart_dir);	        
		} else {
		  _TCHAR app[AG_PATHNAME_MAX];
		  AG_GetCWD(app, AG_PATHNAME_MAX);
		  AG_FileDlgSetDirectory(dlg, "%s", get_parent_dir(app));
		}
		AG_FileDlgAddType(dlg, desc, ext, OnOpenCartSub, "%i", drv);
                AG_WindowShow(win);
		return;
}
#endif

#ifdef USE_FD1
void open_disk(int drv, _TCHAR* path, int bank);

void OnOpenFDSub(AG_Event *event)
{
  char *path = AG_STRING(2);
  char path_shadow[AG_PATHNAME_MAX];
  AG_FileType *filetype = (AG_FileType *)AG_PTR(3);
  int drv = AG_INT(1);
  AG_FileDlg *my = (AG_FileDlg *)AG_SELF();
  if(path) {
    if(strlen(path) <= 0) return;

    strncpy(path_shadow, path, AG_PATHNAME_MAX);
    UPDATE_HISTORY(path, config.recent_disk_path[drv]);
    get_parent_dir(path_shadow);
    strcpy(config.initial_disk_dir, path_shadow);
    open_disk(drv, path, 0);
  }
}


void open_disk_dialog(AG_Widget *hWnd, int drv)
{
  const char *ext = "*.d88,*.d77,*.td0,*.imd,*.dsk,*.fdi,*.hdm,*.tfd,*.xdf,*.2d,*.sf7";
  char *desc = _N("Floppy Disk");
  AG_FileDlg *dlg;
  AG_Window *win;
   
  win = AG_WindowNew(0);
  dlg = AG_FileDlgNew(win, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
  if(dlg == NULL) return;
  
  if(config.initial_disk_dir != NULL) {
    AG_FileDlgSetDirectory(dlg, "%s", config.initial_disk_dir);	        
  } else {
    _TCHAR app[AG_PATHNAME_MAX];
    AG_GetCWD(app, AG_PATHNAME_MAX);
    AG_FileDlgSetDirectory(dlg, "%s", get_parent_dir(app));
  }
  AG_FileDlgAddType(dlg, desc, ext, OnOpenFDSub, "%i", drv);
  AG_WindowShow(win);
  return;
}

extern void Convert_CP932_to_UTF8(char *dst, char *src, int n_limit);

void open_disk(int drv, _TCHAR* path, int bank)
{
	emu->d88_file[drv].bank_num = 0;
	emu->d88_file[drv].cur_bank = -1;
	emu->d88_file[drv].bank[0].offset = 0;
	
	if(check_file_extension(path, ".d88") || check_file_extension(path, ".d77")) {
		FILE *fp = fopen(path, "rb");
		if(fp != NULL) {
			try {
				fseek(fp, 0, SEEK_END);
				int file_size = ftell(fp), file_offset = 0;
				while(file_offset + 0x2b0 <= file_size && emu->d88_file[drv].bank_num < MAX_D88_BANKS) {
					emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].offset = file_offset;
					fseek(fp, file_offset, SEEK_SET);
#ifdef _UNICODE
					char tmp[18];
					fread(tmp, 17, 1, fp);
					tmp[17] = 0;
					Convert_CP932_to_UTF8(emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].name, tmp, 18);

#else
					fread(emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].name, 17, 1, fp);
					emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].name[17] = 0;
#endif
					fseek(fp, file_offset + 0x1c, SEEK_SET);
					file_offset += fgetc(fp);
					file_offset += fgetc(fp) << 8;
					file_offset += fgetc(fp) << 16;
					file_offset += fgetc(fp) << 24;
					emu->d88_file[drv].bank_num++;
				}
				strcpy(emu->d88_file[drv].path, path);
				emu->d88_file[drv].cur_bank = bank;
			}
			catch(...) {
				emu->d88_file[drv].bank_num = 0;
			}
		}
	}
	emu->open_disk(drv, path, emu->d88_file[drv].bank[bank].offset);
#ifdef USE_FD2
	if((drv & 1) == 0 && drv + 1 < MAX_FD && bank + 1 < emu->d88_file[drv].bank_num) {
		open_disk(drv + 1, path, bank + 1);
	}
#endif
}

void close_disk(int drv)
{
  emu->close_disk(drv);
  emu->d88_file[drv].bank_num = 0;
  emu->d88_file[drv].cur_bank = -1;

}
#endif

#ifdef USE_QD1
void OnOpenQDSub(AG_Event *event)
{
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
}

void open_quickdisk_dialog(AG_Widget *hWnd, int drv)
{
   char path_shadow[AG_PATHNAME_MAX];
  const char *ext = "*.mzt,*.q20,*qdf";
  char *desc = _N("Quick Disk");
  AG_Window *win;
   
  win = AG_WindowNew(0);
 
  dlg = AG_FileDlgNew(win, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
  if(dlg == NULL) return;
  
  if(config.initial_quickdisk_dir != NULL) {
    AG_FileDlgSetDirectory(dlg, "%s", config.initial_quickdisk_dir);	        
  } else {
    _TCHAR app[AG_PATHNAME_MAX];
    AG_GetCWD(app, AG_PATHNAME_MAX);
    AG_FileDlgSetDirectory(dlg, "%s", get_parent_dir(app));
  }
  AG_FileDlgAddType(dlg, desc, ext, OnOpenQDSub, "%i", drv);
  AG_WindowShow(win);
  return;
}
#endif

#ifdef USE_TAPE
void OnOpenTapeSub(AG_Event *event)
{
   AG_FileType *filetype = (AG_FileType *)AG_PTR(3);
  char path_shadow[AG_PATHNAME_MAX];
  char *path = AG_STRING(2);
  int play = AG_INT(1);
  AG_FileDlg *my = (AG_FileDlg *)AG_SELF();
  if(path) {
    if(strlen(path) <= 0) return;
    strncpy(path_shadow, path, AG_PATHNAME_MAX);
    UPDATE_HISTORY(path, config.recent_tape_path);
    get_parent_dir(path_shadow);
    strcpy(config.initial_tape_dir, path_shadow);
    if(play != 0) {
      emu->play_tape(path);
    } else {
      emu->rec_tape(path);
    }
  }
}

void open_tape_dialog(AG_Widget *hWnd, bool play)
{
  int playf = play ? 1 : 0;
  AG_FileDlg *dlg;
  const char *ext;
  char *desc;
  AG_Window *win;
   
  win = AG_WindowNew(0);
#if defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
  ext = "*.wav,*.p6,*.cas";
#elif defined(_PC8001SR) || defined(_PC8801MA) || defined(_PC98DO)
  ext = play ? "*.cas,*.cmt,*.n80,*.t88" : "*.cas,*.cmt";
#elif defined(_MZ80A) || defined(_MZ80K) || defined(_MZ1200) || defined(_MZ700) || defined(_MZ800) || defined(_MZ1500)
  ext = play ? "*.wav,*.cas,*.mzt,*.m12" :"*.wav,*.cas";
#elif defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
  ext = play ? "*.wav,*.cas,*.mzt,*.mti,*.mtw,*.dat" : "*.wav,*.cas";
#elif defined(_X1) || defined(_X1TWIN) || defined(_X1TURBO) || defined(_X1TURBOZ)
  ext = play ? "*.wav,*.cas,*.tap" : "*.wav,*.cas";
#elif defined(_FM7) || defined(_FM77) || defined(_FM77AV) || defined(_FM77AV40)
  ext = "*.wav,*.t77";
#elif defined(TAPE_BINARY_ONLY)
  ext = "*.cas,*.cmt";
#else
  ext = "*.wav;*.cas";
#endif
  desc = play ? _N("Data Recorder Tape [Play]") : _N("Data Recorder Tape [Rec]");

  dlg = AG_FileDlgNew(win, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
  if(dlg == NULL) return;
  
  if(config.initial_tape_dir != NULL) {
    AG_FileDlgSetDirectory(dlg, "%s", config.initial_tape_dir);	        
  } else {
    _TCHAR app[AG_PATHNAME_MAX];
    AG_GetCWD(app, AG_PATHNAME_MAX);
    AG_FileDlgSetDirectory(dlg, "%s", get_parent_dir(app));
  }
  AG_FileDlgAddType(dlg, desc, ext, OnOpenTapeSub, "%i", playf);
  AG_WindowShow(win);
  return;
}
#endif

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
  char *desc = _N("Laser Disc");
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
  char *desc = _N("RAM Pack Cartridge");
#else
  char *desc = _N("Memory Dump");
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

