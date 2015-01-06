/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
        Port to Agar : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -

	[ win32 main ] -> [ agar main ]
*/

#include <stdio.h>
#include <string>
#include <vector>
#include "common.h"
#include "emu.h"
#include "agar_main.h"

// emulation core
EMU* emu;

// buttons
#ifdef USE_BUTTON
#define MAX_FONT_SIZE 32
AG_Font   *hFont[MAX_FONT_SIZE];
AG_Widget *hButton[MAX_BUTTONS];
AG_Event  *buttonWndProc[MAX_BUTTONS];
#endif

// menu
AG_GLView *hGLView = NULL;
AGAR_SDLView *hSDLView = NULL;
AG_Widget *hScreenWidget = NULL;
AG_Menu   *hMenu = NULL; // Global Variable.
AG_Window *hWindow = NULL;
AG_Box    *hStatusBar = NULL;

std::string cpp_homedir;
std::string cpp_confdir;
std::string my_procname;
std::string cpp_simdtype;
std::string sAG_Driver;
std::string sRssDir;
bool now_menuloop = false;
static int close_notified = 0;

const int screen_mode_width[]  = {320, 320, 640, 640, 800, 1024, 1280, 1280, 1440, 1440, 1600, 1600, 1920, 1920, 0};
const int screen_mode_height[] = {200, 240, 400, 480, 600, 768,  800,  960,  900,  1080, 1000, 1200, 1080, 1400, 0};
bool bRunEmuThread = false;
static AG_Thread hEmuThread;

// Important Flags
AGAR_CPUID *pCpuID;

void update_menu(AG_Widget *hWnd, AG_Menu* hMenu, int pos);

#ifndef MAX_HISTORY
#define MAX_HISTORY 8
#endif

#define UPDATE_HISTORY(path, recent) { \
	int no = MAX_HISTORY - 1; \
	for(int i = 0; i < MAX_HISTORY; i++) { \
		if(strcmp(recent[i], path) == 0) { \
			no = i; \
			break; \
		} \
	} \
	for(int i = no; i > 0; i--) { \
		strcpy(recent[i], recent[i - 1]); \
	} \
	strcpy(recent[0], path); \
}

#ifdef USE_ICONV
#include <iconv.h>
#endif

void Convert_CP932_to_UTF8(char *dst, char *src, int n_limit)
{
#ifdef USE_ICONV
  char          *pIn, *pOut;
  iconv_t       hd;
  size_t        in, out;
  if((n_limit <= 0) || (src == NULL) || (dst == NULL)) return;
  pIn = src;
  pOut = dst;
  in = strlen(pIn);
  
  out = n_limit;
  hd = iconv_open("utf-8", "cp932");
  if((hd >= 0) && (in > 0)){
    while(in>0) {
      iconv(hd, &pIn, &in, &pOut, &out);
    }
    iconv_close(hd);
  }
#else
  strncpy(dst, src, n_limit);
#endif
}   


bool check_file_extension(_TCHAR *path, const _TCHAR *ext)
{
   std::string::size_type ptr;
   std::string tmp;
   if((path == NULL) || (ext == NULL)) return false;
   tmp = path;
   
   ptr = tmp.rfind(ext, tmp.length());
   if(ptr == std::string::npos) return false;
   if(ptr < tmp.length() - strlen(ext)) return false;
   return true;
}

void get_long_full_path_name(_TCHAR* src, _TCHAR* dst)
{
  AG_Dir * dir;
  char tmp[AG_PATHNAME_MAX];
  std::string r_path;
  std::string delim;
  std::string ss;
  const char *s;

  if(src == NULL) {
    if(dst != NULL) dst[0] = '\0';
    return;
  }
#ifdef _WINDOWS
  delim = "\\";
#else
  delim = "/";
#endif
  
  if(cpp_homedir == "") {
    AG_GetCWD(tmp, AG_PATHNAME_MAX - 1);
    r_path = tmp;
  } else {
    r_path = cpp_homedir;
  }
  s = AG_ShortFilename(src);
  r_path = r_path + my_procname + delim;
  AG_MkPath(r_path.c_str());
  ss = "";
  if(s != NULL) ss = s;
  r_path = r_path + ss;
  if(dst != NULL) r_path.copy(dst, r_path.length() >= AG_PATHNAME_MAX ? AG_PATHNAME_MAX : r_path.length(), 0);
  return;
}

_TCHAR* get_parent_dir(_TCHAR* file)
{
        std::string::size_type ptr;
#ifdef _WINDOWS
	std::string delim = "\\";
#else
	std::string delim = "/";
#endif
	std::string path;
	path = file;

	ptr = path.find_last_of(delim);
	if(ptr != std::string::npos) {
	  if(ptr > 0) path.copy(file, ptr - 1, 0);
	}
	return file;
}




// screen
unsigned int desktop_width;
unsigned int desktop_height;
//int desktop_bpp;
int prev_window_mode = 0;
bool now_fullscreen = false;

#define MAX_WINDOW	8
#define MAX_FULLSCREEN	32

int window_mode_count;
int screen_mode_count;

void set_window(AG_Widget * hWnd, int mode);

// timing control
#define MAX_SKIP_FRAMES 10

int get_interval()
{
	static int accum = 0;
	accum += emu->frame_interval();
	int interval = accum >> 10;
	accum -= interval << 10;
	return interval;
}



extern "C" {

  enum {
    UI_MOUSE_NONE      = 0,
    UI_MOUSE_LEFT      = 1,
    UI_MOUSE_MIDDLE    = 2,
    UI_MOUSE_RIGHT     = 4,
    UI_MOUSE_X1        = 8,
    UI_MOUSE_X2        = 16,
    UI_MOUSE_WHEELUP   = 4096,
    UI_MOUSE_WHEELDOWN = 8192,
  };
  static int mouse_x = 0;
  static int mouse_y = 0;
  static int mouse_relx = 0;
  static int mouse_rely = 0;
  static uint32 mouse_buttons = 0;
void ProcessKeyUp(AG_Event *event)
{
  int key = AG_INT(1);
  int mod = AG_INT(2);
  uint32_t unicode = AG_INT(3);
  uint32 code = (key & 0xffff) | ((mod & 0xffff) << 16);
#ifdef USE_BUTTON
  emu->key_up(code, unicode);
#endif
}

void ProcessKeyDown(AG_Event *event)
{
  int key = AG_INT(1);
  int mod = AG_INT(2);
  uint32_t unicode = AG_INT(3);
  uint32 code = (key & 0xffff) | ((mod & 0xffff) << 16);
#ifdef USE_BUTTON
  emu->key_down(code, unicode);
#endif
}

void OnMouseMotion(AG_Event *event)
{
  // Need lock?
  int x = AG_INT(1);
  int y = AG_INT(2);
  mouse_relx = AG_INT(3);
  mouse_rely = AG_INT(4);
  int buttons = AG_INT(5);

  if((hScreenWidget != NULL) && (emu != NULL)){
    //mouse_x = (x * emu->screen_width)  /  hScreenWidget->w;
    //mouse_y = (y * emu->screen_height) /  hScreenWidget->h;
    mouse_x = x;
    mouse_y = y;
  }
  // Need Unlock?
}

void OnMouseButtonDown(AG_Event *event)
{
  // Need Lock?
  int buttons = AG_INT(1);
  switch (buttons){
  case AG_MOUSE_NONE:
    break;
  case AG_MOUSE_LEFT:
    mouse_buttons |= UI_MOUSE_LEFT;
    break;
  case AG_MOUSE_MIDDLE:
    mouse_buttons |= UI_MOUSE_MIDDLE;
    break;
  case AG_MOUSE_RIGHT:
    mouse_buttons |= UI_MOUSE_RIGHT;
    break;
  case AG_MOUSE_X1:
    mouse_buttons |= UI_MOUSE_X1;
    break;
  case AG_MOUSE_X2:
    mouse_buttons |= UI_MOUSE_X2;
    break;
  case AG_MOUSE_WHEELUP:
    mouse_buttons |= UI_MOUSE_WHEELUP;
    break;
  case AG_MOUSE_WHEELDOWN:
    mouse_buttons |= UI_MOUSE_WHEELDOWN;
    break;
  default:
    break;
  }
  // Need Unlock?
}

void OnMouseButtonUp(AG_Event *event)
{
  // Need Lock?
  int buttons = AG_INT(1);
  switch (buttons){
  case AG_MOUSE_NONE:
    break;
  case AG_MOUSE_LEFT:
    mouse_buttons &= ~UI_MOUSE_LEFT;
    break;
  case AG_MOUSE_MIDDLE:
    mouse_buttons &= ~UI_MOUSE_MIDDLE;
    break;
  case AG_MOUSE_RIGHT:
    mouse_buttons &= ~UI_MOUSE_RIGHT;
    break;
  case AG_MOUSE_X1:
    mouse_buttons &= ~UI_MOUSE_X1;
    break;
  case AG_MOUSE_X2:
    mouse_buttons &= ~UI_MOUSE_X2;
    break;
  case AG_MOUSE_WHEELUP:
    mouse_buttons &= ~UI_MOUSE_WHEELUP;
    break;
  case AG_MOUSE_WHEELDOWN:
    mouse_buttons &= ~UI_MOUSE_WHEELDOWN;
    break;
  default:
    break;
  }
}

  void LostFocus(AG_Event *event)
  {
    if(emu) {
      emu->key_lost_focus();
    }
  }

  void OnMainWindowClosed(AG_Event *event)
  {
    
#ifdef USE_POWER_OFF
    // notify power off
    if(emu) {
      if(!close_notified) {
	emu->notify_power_off();
	close_notified = 1;
	return; 
      }
    }
#endif
    // release window
    if(now_fullscreen) {
      //ChangeDisplaySettings(NULL, 0);
    }
    now_fullscreen = false;
#ifdef USE_BUTTON
    for(int i = 0; i < MAX_FONT_SIZE; i++) {
      if(hFont[i]) {
	DeleteObject(hFont[i]);
      }
    }
#endif
    bRunEmuThread = false;
    AG_ThreadJoin(hEmuThread, NULL); // Okay?
    // release emulation core
    if(emu) {
      delete emu;
      emu = NULL;
    }
    save_config();
    AG_Quit();
    return;
  }

  void OnWindowRedraw(AG_Event *event)
  {
    if(emu) {
      emu->update_screen(hScreenWidget);
    }
  }

  void OnWindowMove(AG_Event *event)
  {
    if(emu) {
      emu->suspend();
    }
  }

  void OnWindowResize(AG_Event *event)
  {
    AG_Widget *my = (AG_Widget *)AG_SELF();
    if(emu) {
      if(now_fullscreen) {
	emu->set_display_size(-1, -1, false);
      } else {
	set_window(my, config.window_mode);
      }
    }
  }

  
}  // extern "C"


static bool InitInstance(void)
{
  AG_Box *vBox;
  AG_Box *hBox;

  AG_RegisterClass(&AGAR_SDLViewClass);

  hWindow = AG_WindowNew(AG_WINDOW_MAIN);
  if(hWindow == NULL) return false;

  vBox = AG_BoxNew(AGWIDGET(hWindow), AG_BOX_VERT, AG_BOX_HFILL);
  {
    hBox = AG_BoxNew(AGWIDGET(vBox), AG_BOX_HORIZ, AG_BOX_VFILL);
    hMenu = AG_MenuNew(AGWIDGET(hBox), AG_MENU_HFILL);
  }
  {
    hBox = AG_BoxNew(AGWIDGET(vBox), AG_BOX_HORIZ, AG_BOX_VFILL);
    //if(AG_UsingGL()) {
      //hGLView = AG_GLViewNew();
      //hScreenWidget = AGWIDGET(hGLView);
    //} else 
    {
      hSDLView = AGAR_SDLViewNew(AGWIDGET(hBox), NULL, NULL);
      if(hSDLView == NULL) return false;
      hScreenWidget = AGWIDGET(hSDLView);
      //AGAR_SDLViewDrawFn(hSDLView, AGAR_SDLViewUpdateSrc, "%p", NULL);
      AGAR_SDLViewSurfaceNew(hSDLView, 640, 400);
      AG_SetEvent(hSDLView, "key-up", ProcessKeyUp, NULL);
      AG_SetEvent(hSDLView, "key-down", ProcessKeyDown, NULL);
      AG_SetEvent(hSDLView, "mouse-motion", OnMouseMotion, NULL);
      AG_SetEvent(hSDLView, "mouse-button-down", OnMouseButtonDown, NULL);
      AG_SetEvent(hSDLView, "mouse-button-up", OnMouseButtonUp, NULL);
      AG_WidgetSetSize(hSDLView, 640, 400);
      AG_WidgetShow(hSDLView);
    }
  }
  {
    hBox = AG_BoxNew(AGWIDGET(vBox), AG_BOX_HORIZ, AG_BOX_VFILL | AG_WIDGET_NOSPACING);
    hStatusBar = hBox;
    AG_WidgetSetSize(hStatusBar, 640, 40);
    AG_WidgetShow(hStatusBar);
  }
  //InitMouse();
  // enumerate screen mode
  screen_mode_count = 0;
}  

#ifdef TRUE
#undef TRUE
#define TRUE true
#endif

#ifdef FALSE
#undef FALSE
#define FALSE false
#endif

void *EmuThread(void *arg)
{
   int total_frames = 0, draw_frames = 0, skip_frames = 0;
   DWORD next_time = 0;
   DWORD update_fps_time = 0;
   bool prev_skip = false;
   bRunEmuThread = true;
  do {
    
    if(emu) {
      // drive machine
      int run_frames = emu->run();
      total_frames += run_frames;
      
			// timing controls
      int interval = 0, sleep_period = 0;
      //			for(int i = 0; i < run_frames; i++) {
      interval += get_interval();
      //			}
      bool now_skip = emu->now_skip() && !emu->now_rec_video;
			
      if((prev_skip && !now_skip) || next_time == 0) {
	next_time = timeGetTime();
      }
      if(!now_skip) {
	next_time += interval;
      }
      prev_skip = now_skip;
      
      if(next_time > timeGetTime()) {
	// update window if enough time
	draw_frames += emu->draw_screen();
	skip_frames = 0;
	
	// sleep 1 frame priod if need
	DWORD current_time = timeGetTime();
	if((int)(next_time - current_time) >= 10) {
	  sleep_period = next_time - current_time;
	}
      } else if(++skip_frames > MAX_SKIP_FRAMES) {
	// update window at least once per 10 frames
	draw_frames += emu->draw_screen();
	skip_frames = 0;
	next_time = timeGetTime();
      }
      Sleep(sleep_period);
      if(bRunEmuThread != true) {
	AG_ThreadExit(NULL);
	//return;
      }
      // calc frame rate
      DWORD current_time = timeGetTime();
      if(update_fps_time <= current_time && update_fps_time != 0) {
	_TCHAR buf[256];
	int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);
	if(emu->message_count > 0) {
	  sprintf(buf, _T("%s - %s"), _T(DEVICE_NAME), emu->message);
	  emu->message_count--;
	} else {
	  sprintf(buf, _T("%s - %d fps (%d %%)"), _T(DEVICE_NAME), draw_frames, ratio);
	}
	AG_WindowSetCaptionS(hWindow, buf);
	update_fps_time += 1000;
	total_frames = draw_frames = 0;
      }
      if(update_fps_time <= current_time) {
	update_fps_time = current_time + 1000;
      }
    } else {
      Sleep(10);
      if(bRunEmuThread != true) {
	AG_ThreadExit(NULL);
	return arg;
      }
    }
  } while(1);
}
#ifndef FONTPATH
#define FONTPATH "."
#endif

int MainLoop(int argc, char *argv[])
{
	  char c;
	  char strbuf[2048];
	  bool flag;
          char homedir[AG_PATHNAME_MAX];
          int thread_ret;
          
          cpp_homedir.copy(homedir, AG_PATHNAME_MAX - 1, 0);
#ifdef _WINDOWS
	  AG_PrtString(agConfig, "font-path", "%s:%s/xm7:%s:.",
		       homedir, homedir, FONTPATH);
#else
	  flag = FALSE;
	  if(AG_GetString(agConfig, "font-path", strbuf, 2047) > 0) {
	    if((strlen(strbuf) <= 0) || (strncmp(strbuf, "_agFontVera", 11) == 0)){
	      flag = TRUE;
	    }
	  } else {
	    flag = TRUE;
	  }
	  if(flag) AG_PrtString(agConfig, "font-path", "%s/.fonts:%s:%s/.xm7:%s:.", 
				homedir, homedir, homedir, FONTPATH);
	  flag = FALSE;
	  
	  AGAR_DebugLog(AGAR_LOG_DEBUG, "font-path = %s", strbuf);
#endif /* _WINDOWS */
	  flag = FALSE;
	  if(AG_GetString(agConfig, "font.face", strbuf, 511) > 0) {
	    if((strlen(strbuf) <= 0) || (strncmp(strbuf, "_agFontVera", 11) == 0)){
	      flag = TRUE;
	    }
	  } else {
	    flag = TRUE;
	  }
	  //if(flag) AG_SetString(agConfig, "font.face", UI_FONT);
	  flag = FALSE;
	  AGAR_DebugLog(AGAR_LOG_DEBUG, "font.face = %s", strbuf);
	  
    // Debug
#ifdef _AGAR_FB_DEBUG // Force FB.
	  sAG_Driver = "sdlfb:width=1280:height=880:depth=32";
#endif
	/*
	 * Into Agar's Loop.
	 */

	  if(sAG_Driver == "")  {
#ifdef USE_OPENGL
	    if(AG_InitGraphics(NULL) == -1){
	      fprintf(stderr, "%s\n", AG_GetError());
	      return -1;
	    }
#else
	    if(AG_InitGraphics("cocoa,sdlfb") == -1){
	      fprintf(stderr, "%s\n", AG_GetError());
	      return -1;
	    }
#endif
	  } else {
	    if (AG_InitGraphics(sAG_Driver.c_str()) == -1) {
	      fprintf(stderr, "%s\n", AG_GetError());
	      return -1;
	    }
	  }
	  AGAR_DebugLog(AGAR_LOG_DEBUG, "Widget creation OK.");
	  AG_GetDisplaySize(NULL, &desktop_width, &desktop_height);
   
	  SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
	  AGAR_DebugLog(AGAR_LOG_DEBUG, "Audio and JOYSTICK subsystem was initialised.");

	  InitInstance();
	  AGAR_DebugLog(AGAR_LOG_DEBUG, "InitInstance() OK.");
	  if(agDriverSw && AG_UsingSDL(NULL)) {
	    SDL_Init(SDL_INIT_VIDEO);
	    AGAR_DebugLog(AGAR_LOG_INFO, "Start Single WM with SDL.");
	  
	  } else { // WM function is managed by SDL, load and set icon for WM. 
	    SDL_Init(SDL_INIT_VIDEO);
	    AGAR_DebugLog(AGAR_LOG_INFO, "Start multi window mode.");
	  }
  
	  // load config
	  load_config();
	// create window


	screen_mode_count = 0;
	do {
	  if(screen_mode_width[screen_mode_count] <= 0) break;
	  screen_mode_count++;
	} while(1);
	
#if 0	
	// restore screen mode
	if(config.window_mode >= 0 && config.window_mode < MAX_WINDOW) {
		PostMessage(hWnd, WM_COMMAND, ID_SCREEN_WINDOW1 + config.window_mode, 0L);
	} else if(config.window_mode >= MAX_WINDOW && config.window_mode < screen_mode_count + MAX_WINDOW) {
		PostMessage(hWnd, WM_COMMAND, ID_SCREEN_FULLSCREEN1 + config.window_mode - MAX_WINDOW, 0L);
	} else {
		config.window_mode = 0;
		PostMessage(hWnd, WM_COMMAND, ID_SCREEN_WINDOW1, 0L);
	}
#endif	
	// accelerator
	//	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	
	// disenable ime
	//ImmAssociateContext(hWnd, 0);
	
	// initialize emulation core
	emu = new EMU(hWindow, hScreenWidget);
	emu->set_display_size(WINDOW_WIDTH, WINDOW_HEIGHT, true);
	
#ifdef SUPPORT_DRAG_DROP
	// open command line path
	//	if(szCmdLine[0]) {
	//	if(szCmdLine[0] == _T('"')) {
	//		int len = strlen(szCmdLine);
	//		szCmdLine[len - 1] = _T('\0');
	//		szCmdLine++;
	//	}
	//	_TCHAR path[_MAX_PATH];
	//	get_long_full_path_name(szCmdLine, path);
	//	open_any_file(path);
	//}
#endif
	
	// set priority
	//SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
	
	// main loop
	

	bRunEmuThread = false;
	AG_ThreadCreate(&hEmuThread, EmuThread, &thread_ret);
	AG_EventLoop(); // Right? maybe unusable Joystick.

	return 0;
}


void update_menu(AG_Widget * hWnd, AG_Menu *hMenu, int pos)
{
#if 0
#ifdef MENU_POS_CONTROL
	if(pos == MENU_POS_CONTROL) {
		// control menu
#ifdef USE_BOOT_MODE
		if(config.boot_mode >= 0 && config.boot_mode < USE_BOOT_MODE) {
			CheckMenuRadioItem(hMenu, ID_BOOT_MODE0, ID_BOOT_MODE0 + USE_BOOT_MODE - 1, ID_BOOT_MODE0 + config.boot_mode, MF_BYCOMMAND);
		}
#endif
#ifdef USE_CPU_TYPE
		if(config.cpu_type >= 0 && config.cpu_type < USE_CPU_TYPE) {
			CheckMenuRadioItem(hMenu, ID_CPU_TYPE0, ID_CPU_TYPE0 + USE_CPU_TYPE - 1, ID_CPU_TYPE0 + config.cpu_type, MF_BYCOMMAND);
		}
#endif
		if(config.cpu_power >= 0 && config.cpu_power < 5) {
			CheckMenuRadioItem(hMenu, ID_CPU_POWER0, ID_CPU_POWER4, ID_CPU_POWER0 + config.cpu_power, MF_BYCOMMAND);
		}
#ifdef USE_DIPSWITCH
		for(int i = 0; i < 32; i++) {
			CheckMenuItem(hMenu, ID_DIPSWITCH0 + i, (config.dipswitch & (1 << i)) ? MF_CHECKED : MF_UNCHECKED);
		}
#endif
#ifdef USE_DEVICE_TYPE
		if(config.device_type >= 0 && config.device_type < USE_DEVICE_TYPE) {
			CheckMenuRadioItem(hMenu, ID_DEVICE_TYPE0, ID_DEVICE_TYPE0 + USE_DEVICE_TYPE - 1, ID_DEVICE_TYPE0 + config.device_type, MF_BYCOMMAND);
		}
#endif
#ifdef USE_AUTO_KEY
		// auto key
		bool now_paste = true, now_stop = true;
		if(emu) {
			now_paste = emu->now_auto_key();
			now_stop = !now_paste;
		}
		EnableMenuItem(hMenu, ID_AUTOKEY_START, now_paste ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_AUTOKEY_STOP, now_stop ? MF_GRAYED : MF_ENABLED);
#endif
#ifdef USE_DEBUGGER
		// debugger
		EnableMenuItem(hMenu, ID_OPEN_DEBUGGER0, emu && !emu->now_debugging && emu->debugger_enabled(0) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_OPEN_DEBUGGER1, emu && !emu->now_debugging && emu->debugger_enabled(1) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_OPEN_DEBUGGER2, emu && !emu->now_debugging && emu->debugger_enabled(2) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_OPEN_DEBUGGER3, emu && !emu->now_debugging && emu->debugger_enabled(3) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_CLOSE_DEBUGGER, emu &&  emu->now_debugging                             ? MF_ENABLED : MF_GRAYED);
#endif
	}
#endif
#ifdef MENU_POS_CART1
	else if(pos == MENU_POS_CART1) {
		#define UPDATE_MENU_CART(drv, ID_RECENT_CART, ID_CLOSE_CART) \
		bool flag = false; \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			DeleteMenu(hMenu, ID_RECENT_CART + i, MF_BYCOMMAND); \
		} \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			if(_tcsicmp(config.recent_cart_path[drv][i], _T(""))) { \
				AppendMenu(hMenu, MF_STRING, ID_RECENT_CART + i, config.recent_cart_path[drv][i]); \
				flag = true; \
			} \
		} \
		if(!flag) { \
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_CART, _T("None")); \
		} \
		EnableMenuItem(hMenu, ID_CLOSE_CART, emu->cart_inserted(drv) ? MF_ENABLED : MF_GRAYED);
		// cart slot #1
		UPDATE_MENU_CART(0, ID_RECENT_CART1, ID_CLOSE_CART1)
	}
#endif
#ifdef MENU_POS_CART2
	else if(pos == MENU_POS_CART2) {
		// cart slot #2
		UPDATE_MENU_CART(1, ID_RECENT_CART2, ID_CLOSE_CART2)
	}
#endif
#ifdef MENU_POS_FD1
	else if(pos == MENU_POS_FD1) {
		#define UPDATE_MENU_FD(drv, ID_RECENT_FD, ID_D88_FILE_PATH, ID_SELECT_D88_BANK, ID_CLOSE_FD) \
		bool flag = false; \
		while(DeleteMenu(hMenu, 3, MF_BYPOSITION) != 0) {} \
		if(emu->d88_file[drv].bank_num > 1) { \
			AppendMenu(hMenu, MF_STRING | MF_DISABLED, ID_D88_FILE_PATH, emu->d88_file[drv].path); \
			for(int i = 0; i < emu->d88_file[drv].bank_num; i++) { \
				_TCHAR tmp[32]; \
				_stprintf(tmp, _T("%d: %s"), i + 1, emu->d88_file[drv].bank[i].name); \
				AppendMenu(hMenu, MF_STRING | (i == emu->d88_file[drv].cur_bank ? MF_CHECKED : 0), ID_SELECT_D88_BANK + i, tmp); \
			} \
			AppendMenu(hMenu, MF_SEPARATOR, 0, NULL); \
		} \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			if(_tcsicmp(config.recent_disk_path[drv][i], _T(""))) { \
				AppendMenu(hMenu, MF_STRING, ID_RECENT_FD + i, config.recent_disk_path[drv][i]); \
				flag = true; \
			} \
		} \
		if(!flag) { \
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_FD, _T("None")); \
		} \
		EnableMenuItem(hMenu, ID_CLOSE_FD, emu->disk_inserted(drv) ? MF_ENABLED : MF_GRAYED);
		// floppy drive #1
		UPDATE_MENU_FD(0, ID_RECENT_FD1, ID_D88_FILE_PATH1, ID_SELECT_D88_BANK1, ID_CLOSE_FD1)
	}
#endif
#ifdef MENU_POS_FD2
	else if(pos == MENU_POS_FD2) {
		// floppy drive #2
		UPDATE_MENU_FD(1, ID_RECENT_FD2, ID_D88_FILE_PATH2, ID_SELECT_D88_BANK2, ID_CLOSE_FD2)
	}
#endif
#ifdef MENU_POS_FD3
	else if(pos == MENU_POS_FD3) {
		// floppy drive #3
		UPDATE_MENU_FD(2, ID_RECENT_FD3, ID_D88_FILE_PATH3, ID_SELECT_D88_BANK3, ID_CLOSE_FD3)
	}
#endif
#ifdef MENU_POS_FD4
	else if(pos == MENU_POS_FD4) {
		// floppy drive #4
		UPDATE_MENU_FD(3, ID_RECENT_FD4, ID_D88_FILE_PATH4, ID_SELECT_D88_BANK4, ID_CLOSE_FD4)
	}
#endif
#ifdef MENU_POS_FD5
	else if(pos == MENU_POS_FD5) {
		// floppy drive #5
		UPDATE_MENU_FD(4, ID_RECENT_FD5, ID_D88_FILE_PATH5, ID_SELECT_D88_BANK5, ID_CLOSE_FD5)
	}
#endif
#ifdef MENU_POS_FD6
	else if(pos == MENU_POS_FD6) {
		// floppy drive #6
		UPDATE_MENU_FD(5, ID_RECENT_FD6, ID_D88_FILE_PATH6, ID_SELECT_D88_BANK6, ID_CLOSE_FD6)
	}
#endif
#ifdef MENU_POS_FD7
	else if(pos == MENU_POS_FD7) {
		// floppy drive #7
		UPDATE_MENU_FD(6, ID_RECENT_FD7, ID_D88_FILE_PATH7, ID_SELECT_D88_BANK7, ID_CLOSE_FD7)
	}
#endif
#ifdef MENU_POS_FD8
	else if(pos == MENU_POS_FD8) {
		// floppy drive #8
		UPDATE_MENU_FD(7, ID_RECENT_FD8, ID_D88_FILE_PATH8, ID_SELECT_D88_BANK8, ID_CLOSE_FD8)
	}
#endif
#ifdef MENU_POS_QD1
	else if(pos == MENU_POS_QD1) {
		#define UPDATE_MENU_QD(drv, ID_RECENT_QD, ID_CLOSE_QD) \
		bool flag = false; \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			DeleteMenu(hMenu, ID_RECENT_QD + i, MF_BYCOMMAND); \
		} \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			if(_tcsicmp(config.recent_quickdisk_path[drv][i], _T(""))) { \
				AppendMenu(hMenu, MF_STRING, ID_RECENT_QD + i, config.recent_quickdisk_path[drv][i]); \
				flag = true; \
			} \
		} \
		if(!flag) { \
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_QD, _T("None")); \
		} \
		EnableMenuItem(hMenu, ID_CLOSE_QD, emu->quickdisk_inserted(drv) ? MF_ENABLED : MF_GRAYED);
		// quick disk drive #1
		UPDATE_MENU_QD(0, ID_RECENT_QD1, ID_CLOSE_QD1)
	}
#endif
#ifdef MENU_POS_QD2
	else if(pos == MENU_POS_QD2) {
		// quick disk drive #2
		UPDATE_MENU_QD(1, ID_RECENT_QD2, ID_CLOSE_QD2)
	}
#endif
#ifdef MENU_POS_TAPE
	else if(pos == MENU_POS_TAPE) {
		// data recorder
		bool flag = false;
		for(int i = 0; i < MAX_HISTORY; i++) {
			DeleteMenu(hMenu, ID_RECENT_TAPE + i, MF_BYCOMMAND);
		}
		for(int i = 0; i < MAX_HISTORY; i++) {
			if(_tcsicmp(config.recent_tape_path[i], _T(""))) {
				AppendMenu(hMenu, MF_STRING, ID_RECENT_TAPE + i, config.recent_tape_path[i]);
				flag = true;
			}
		}
		if(!flag) {
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_TAPE, _T("None"));
		}
		EnableMenuItem(hMenu, ID_CLOSE_TAPE, emu->tape_inserted() ? MF_ENABLED : MF_GRAYED);
#ifdef USE_TAPE_BUTTON
		EnableMenuItem(hMenu, ID_PLAY_BUTTON, emu->tape_inserted() ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_STOP_BUTTON, emu->tape_inserted() ? MF_ENABLED : MF_GRAYED);
#endif
		CheckMenuItem(hMenu, ID_USE_WAVE_SHAPER, config.wave_shaper ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_DIRECT_LOAD_MZT, config.direct_load_mzt ? MF_CHECKED : MF_UNCHECKED);
	}
#endif
#ifdef MENU_POS_LASER_DISC
	else if(pos == MENU_POS_LASER_DISC) {
		// data recorder
		bool flag = false;
		for(int i = 0; i < MAX_HISTORY; i++) {
			DeleteMenu(hMenu, ID_RECENT_LASER_DISC + i, MF_BYCOMMAND);
		}
		for(int i = 0; i < MAX_HISTORY; i++) {
			if(_tcsicmp(config.recent_laser_disc_path[i], _T(""))) {
				AppendMenu(hMenu, MF_STRING, ID_RECENT_LASER_DISC + i, config.recent_laser_disc_path[i]);
				flag = true;
			}
		}
		if(!flag) {
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_LASER_DISC, _T("None"));
		}
		EnableMenuItem(hMenu, ID_CLOSE_LASER_DISC, emu->laser_disc_inserted() ? MF_ENABLED : MF_GRAYED);
	}
#endif
#ifdef MENU_POS_BINARY1
	else if(pos == MENU_POS_BINARY1) {
		// binary #1
		#define UPDATE_MENU_BINARY(drv, ID_RECENT_BINARY) \
		bool flag = false; \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			DeleteMenu(hMenu, ID_RECENT_BINARY + i, MF_BYCOMMAND); \
		} \
		for(int i = 0; i < MAX_HISTORY; i++) { \
			if(_tcsicmp(config.recent_binary_path[drv][i], _T(""))) { \
				AppendMenu(hMenu, MF_STRING, ID_RECENT_BINARY + i, config.recent_binary_path[drv][i]); \
				flag = true; \
			} \
		} \
		if(!flag) { \
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_BINARY, _T("None")); \
		}
		UPDATE_MENU_BINARY(0, ID_RECENT_BINARY1)
	}
#endif
#ifdef MENU_POS_BINARY2
	else if(pos == MENU_POS_BINARY2) {
		// binary #2
		UPDATE_MENU_BINARY(1, ID_RECENT_BINARY2)
	}
#endif
#ifdef MENU_POS_SCREEN
	else if(pos == MENU_POS_SCREEN) {
		// recording
		bool now_rec = true, now_stop = true;
		if(emu) {
			now_rec = emu->now_rec_video;
			now_stop = !now_rec;
		}
		EnableMenuItem(hMenu, ID_SCREEN_REC60, now_rec ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SCREEN_REC30, now_rec ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SCREEN_REC15, now_rec ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SCREEN_STOP, now_stop ? MF_GRAYED : MF_ENABLED);
		
		// screen mode
		UINT last = ID_SCREEN_WINDOW1;
		for(int i = 1; i < MAX_WINDOW; i++) {
			DeleteMenu(hMenu, ID_SCREEN_WINDOW1 + i, MF_BYCOMMAND);
		}
		for(int i = 1; i < MAX_WINDOW; i++) {
			if(emu && emu->get_window_width(i) <= desktop_width && emu->get_window_height(i) <= desktop_height) {
				_TCHAR buf[16];
				_stprintf(buf, _T("Window x%d"), i + 1);
				InsertMenu(hMenu, ID_SCREEN_FULLSCREEN1, MF_BYCOMMAND | MF_STRING, ID_SCREEN_WINDOW1 + i, buf);
				last = ID_SCREEN_WINDOW1 + i;
			}
		}
		for(int i = 0; i < MAX_FULLSCREEN; i++) {
			if(i < screen_mode_count) {
				MENUITEMINFO info;
				ZeroMemory(&info, sizeof(info));
				info.cbSize = sizeof(info);
				_TCHAR buf[64];
				_stprintf(buf, _T("Fullscreen %dx%d"), screen_mode_width[i], screen_mode_height[i]);
				info.fMask = MIIM_TYPE;
				info.fType = MFT_STRING;
				info.dwTypeData = buf;
				SetMenuItemInfo(hMenu, ID_SCREEN_FULLSCREEN1 + i, FALSE, &info);
				EnableMenuItem(hMenu, ID_SCREEN_FULLSCREEN1 + i, now_fullscreen ? MF_GRAYED : MF_ENABLED);
				last = ID_SCREEN_FULLSCREEN1 + i;
			} else {
				DeleteMenu(hMenu, ID_SCREEN_FULLSCREEN1 + i, MF_BYCOMMAND);
			}
		}
		if(config.window_mode >= 0 && config.window_mode < MAX_WINDOW) {
			CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW1, last, ID_SCREEN_WINDOW1 + config.window_mode, MF_BYCOMMAND);
		} else if(config.window_mode >= MAX_WINDOW && config.window_mode < screen_mode_count + MAX_WINDOW) {
			CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW1, last, ID_SCREEN_FULLSCREEN1 + config.window_mode - MAX_WINDOW, MF_BYCOMMAND);
		}
		CheckMenuItem(hMenu, ID_SCREEN_USE_D3D9, config.use_d3d9 ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_SCREEN_WAIT_VSYNC, config.wait_vsync ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuRadioItem(hMenu, ID_SCREEN_STRETCH_DOT, ID_SCREEN_STRETCH_FILL, ID_SCREEN_STRETCH_DOT + config.stretch_type, MF_BYCOMMAND);
		
#ifdef USE_MONITOR_TYPE
		if(config.monitor_type >= 0 && config.monitor_type < USE_MONITOR_TYPE) {
			CheckMenuRadioItem(hMenu, ID_SCREEN_MONITOR_TYPE0, ID_SCREEN_MONITOR_TYPE0 + USE_MONITOR_TYPE - 1, ID_SCREEN_MONITOR_TYPE0 + config.monitor_type, MF_BYCOMMAND);
		}
#endif
#ifdef USE_CRT_FILTER
		CheckMenuItem(hMenu, ID_SCREEN_CRT_FILTER, config.crt_filter ? MF_CHECKED : MF_UNCHECKED);
#endif
#ifdef USE_SCANLINE
		CheckMenuItem(hMenu, ID_SCREEN_SCANLINE, config.scan_line ? MF_CHECKED : MF_UNCHECKED);
#endif
	}
#endif
#ifdef MENU_POS_SOUND
	else if(pos == MENU_POS_SOUND) {
		// sound menu
		bool now_rec = false, now_stop = false;
		if(emu) {
			now_rec = emu->now_rec_sound;
			now_stop = !now_rec;
		}
		EnableMenuItem(hMenu, ID_SOUND_REC, now_rec ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_SOUND_STOP, now_stop ? MF_GRAYED : MF_ENABLED);
		
		if(config.sound_frequency >= 0 && config.sound_frequency < 8) {
			CheckMenuRadioItem(hMenu, ID_SOUND_FREQ0, ID_SOUND_FREQ7, ID_SOUND_FREQ0 + config.sound_frequency, MF_BYCOMMAND);
		}
		if(config.sound_latency >= 0 && config.sound_latency < 5) {
			CheckMenuRadioItem(hMenu, ID_SOUND_LATE0, ID_SOUND_LATE4, ID_SOUND_LATE0 + config.sound_latency, MF_BYCOMMAND);
		}
#ifdef USE_SOUND_DEVICE_TYPE
		if(config.sound_device_type >= 0 && config.sound_device_type < USE_SOUND_DEVICE_TYPE) {
			CheckMenuRadioItem(hMenu, ID_SOUND_DEVICE_TYPE0, ID_SOUND_DEVICE_TYPE0 + USE_SOUND_DEVICE_TYPE - 1, ID_SOUND_DEVICE_TYPE0 + config.sound_device_type, MF_BYCOMMAND);
		}
#endif
	}
#endif
#ifdef MENU_POS_CAPTURE
	else if(pos == MENU_POS_CAPTURE) {
		// video capture menu
		int num_devs = emu->get_num_capture_devs();
		int cur_index = emu->get_cur_capture_dev_index();
		
		for(int i = 0; i < 8; i++) {
			DeleteMenu(hMenu, ID_CAPTURE_DEVICE1 + i, MF_BYCOMMAND);
		}
		for(int i = 0; i < 8; i++) {
			if(num_devs >= i + 1) {
				AppendMenu(hMenu, MF_STRING, ID_CAPTURE_DEVICE1 + i, emu->get_capture_dev_name(i));
			}
		}
		if(num_devs == 0) {
			AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_CAPTURE_DEVICE1, _T("None"));
		}
		if(cur_index != -1) {
			CheckMenuRadioItem(hMenu, ID_CAPTURE_DEVICE1, ID_CAPTURE_DEVICE1, ID_CAPTURE_DEVICE1 + cur_index, MF_BYCOMMAND);
		}
		EnableMenuItem(hMenu, ID_CAPTURE_FILTER, (cur_index != -1) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_CAPTURE_PIN, (cur_index != -1) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_CAPTURE_SOURCE, (cur_index != -1) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_CAPTURE_CLOSE, (cur_index != -1) ? MF_ENABLED : MF_GRAYED);
	}
#endif
	DrawMenuBar(hWnd);
#endif
}

#ifdef USE_CART1
static void OnOpenCartSub(AG_Event *event)
{
  char *path = AG_STRING(1);
  int drv = AG_INT(2);
  AG_FileDlg *my = (AG_FileDlg *)AG_SELF();
  if(path) {
    if(strlen(path) <= 0) return;
    UPDATE_HISTORY(path, config.recent_cart_path[drv]);
    strcpy(config.initial_cart_dir, get_parent_dir(path));
    if(emu) emu->open_cart(drv, path);
  }
}



void open_cart_dialog(AG_Widget * hWnd, int drv)
{
                AG_FileDlg *dlg;
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
		dlg = AG_FileDlgNew(hWnd, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
		if(dlg == NULL) return;
	
		if(config.initial_cart_dir != NULL) {
		  AG_FileDlgSetDirectory(dlg, "%s", config.initial_cart_dir);	        
		} else {
		  _TCHAR app[AG_PATHNAME_MAX];
		  AG_GetCWD(app, AG_PATHNAME_MAX);
		  AG_FileDlgSetDirectory(dlg, "%s", get_parent_dir(app));
		}
		AG_FileDlgAddType(dlg, desc, ext, OnOpenCartSub, "%i", drv);
		return;
}
#endif

#ifdef USE_FD1
void open_disk(int drv, _TCHAR* path, int bank);

void OnOpenFDSub(AG_Event *event)
{
  char *path = AG_STRING(1);
  int drv = AG_INT(2);
  AG_FileDlg *my = (AG_FileDlg *)AG_SELF();
  if(path) {
    if(strlen(path) <= 0) return;
    UPDATE_HISTORY(path, config.recent_disk_path[drv]);
    strcpy(config.initial_disk_dir, get_parent_dir(path));
    open_disk(drv, path, 0);
  }
}


void open_disk_dialog(AG_Widget * hWnd, int drv)
{
  const char *ext = "*.d88,*.d77,*.td0,*.imd,*.dsk,*.fdi,*.hdm,*.tfd,*.xdf,*.2d,*.sf7";
  char *desc = _N("Floppy Disk");
  AG_FileDlg *dlg;
   
  dlg = AG_FileDlgNew(hWnd, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
  if(dlg == NULL) return;
  
  if(config.initial_disk_dir != NULL) {
    AG_FileDlgSetDirectory(dlg, "%s", config.initial_disk_dir);	        
  } else {
    _TCHAR app[AG_PATHNAME_MAX];
    AG_GetCWD(app, AG_PATHNAME_MAX);
    AG_FileDlgSetDirectory(dlg, "%s", get_parent_dir(app));
  }
  AG_FileDlgAddType(dlg, desc, ext, OnOpenFDSub, "%i", drv);
  return;
}

void Convert_CP932_to_UTF8(char *dst, char *src, int n_limit);

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
  char *path = AG_STRING(1);
  int drv = AG_INT(2);
  AG_FileDlg *my = (AG_FileDlg *)AG_SELF();
  if(path) {
    if(strlen(path) <= 0) return;
    UPDATE_HISTORY(path, config.recent_quickdisk_path[drv]);
    strcpy(config.initial_disk_dir, get_parent_dir(path));
    if(emu) emu->open_quickdisk(drv, path, 0);
  }
}

void open_quickdisk_dialog(AG_Widget * hWnd, int drv)
{
  const char *ext = "*.mzt,*.q20,*qdf";
  char *desc = _N("Quick Disk");
  dlg = AG_FileDlgNew(hWnd, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
  if(dlg == NULL) return;
  
  if(config.initial_quickdisk_dir != NULL) {
    AG_FileDlgSetDirectory(dlg, "%s", config.initial_quickdisk_dir);	        
  } else {
    _TCHAR app[AG_PATHNAME_MAX];
    AG_GetCWD(app, AG_PATHNAME_MAX);
    AG_FileDlgSetDirectory(dlg, "%s", get_parent_dir(app));
  }
  AG_FileDlgAddType(dlg, desc, ext, OnOpenQDSub, "%i", drv);
  return;
}
#endif

#ifdef USE_TAPE
void OnOpenTapeSub(AG_Event *event)
{
  char *path = AG_STRING(1);
  int play = AG_INT(2);
  AG_FileDlg *my = (AG_FileDlg *)AG_SELF();
  if(path) {
    if(strlen(path) <= 0) return;
    UPDATE_HISTORY(path, config.recent_tape_path);
    strcpy(config.initial_tape_dir, get_parent_dir(path));
    if(play != 0) {
      emu->play_tape(path);
    } else {
      emu->rec_tape(path);
    }
  }
}

void open_tape_dialog(AG_Widget * hWnd, bool play)
{
  AG_FileDlg *dlg;
  const char *ext;
  char *desc;
  int playFlag = 0;
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

  if(play) playFlag = 1;
  dlg = AG_FileDlgNew(hWnd, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
  if(dlg == NULL) return;
  
  if(config.initial_tape_dir != NULL) {
    AG_FileDlgSetDirectory(dlg, "%s", config.initial_tape_dir);	        
  } else {
    _TCHAR app[AG_PATHNAME_MAX];
    AG_GetCWD(app, AG_PATHNAME_MAX);
    AG_FileDlgSetDirectory(dlg, "%s", get_parent_dir(app));
  }
  AG_FileDlgAddType(dlg, desc, ext, OnOpenTapeSub, "%i", playFlag);
  return;
}
#endif

#ifdef USE_LASER_DISC
void OnOpenLaserDiscSub(AG_Event *event)
{
  char *path = AG_STRING(1);
  AG_FileDlg *my = (AG_FileDlg *)AG_SELF();
  if(path) {
    if(strlen(path) <= 0) return;
    UPDATE_HISTORY(path, config.recent_laser_disc_path);
    strcpy(config.initial_laser_disc_dir, get_parent_dir(path));
    if(emu) emu->open_laser_disc(path);
  }
}

void open_laser_disc_dialog(AG_Widget * hWnd)
{
  const char *ext = "*.avi,*.mpg,*.mpeg,*.wmv,*.ogv";
  char *desc = _N("Laser Disc");
  AG_FileDlg *dlg = AG_FileDlgNew(hWnd, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
  if(dlg == NULL) return;
  
  if(config.initial_laser_disc_dir != NULL) {
    AG_FileDlgSetDirectory(dlg, "%s", config.initial_laser_disc_dir);	        
  } else {
    _TCHAR app[AG_PATHNAME_MAX];
    AG_GetCWD(app, AG_PATHNAME_MAX);
    AG_FileDlgSetDirectory(dlg, "%s", get_parent_dir(app));
  }
  AG_FileDlgAddType(dlg, desc, ext, OnOpenLaserDiscSub, "%p", NULL);
  return;
}
#endif

#ifdef USE_BINARY_FILE1

void OnOpenBinarySub(AG_Event *event)
{
  char *path = AG_STRING(1);
  int drv = AG_INT(2);
  int load = AG_INT(3);
  AG_FileDlg *my = (AG_FileDlg *)AG_SELF();
  if(path) {
    UPDATE_HISTORY(path, config.recent_binary_path[drv]);
    strcpy(config.initial_binary_dir, get_parent_dir(path));
    if(load != 0) {
      emu->load_binary(drv, path);
    } else {
      emu->save_binary(drv, path);
    }
  }
}

void open_binary_dialog(AG_Widget * hWnd, int drv, bool load)
{
  const char ext = "*.ram,*.bin";
#if defined(_PASOPIA) || defined(_PASOPIA7)
  char *desc = _N("RAM Pack Cartridge");
#else
  char *desc = _N("Memory Dump");
#endif
  int loadFlg = 0;
  AG_FileDlg *dlg = AG_FileDlgNew(hWnd, AG_FILEDLG_MASK_EXT | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
  if(dlg == NULL) return;
  
  if(config.initial_binary_dir != NULL) {
    AG_FileDlgSetDirectory(dlg, "%s", config.initial_binary_dir);	        
  } else {
    _TCHAR app[AG_PATHNAME_MAX];
    AG_GetCWD(app, AG_PATHNAME_MAX);
    AG_FileDlgSetDirectory(dlg, "%s", get_parent_dir(app));
  }
  if(load) loadFlg = 1;
  AG_FileDlgAddType(dlg, desc, ext, OnOpenBinarySub, "%i,%i", drv, loadFlg);
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

void set_window(AG_Widget *hWnd, int mode)
{
 
//	static LONG style = WS_VISIBLE;
	AG_Window *window;

	if(hWnd == NULL) return;
	window = hWnd->window;
	if(window == NULL) return;
	
	if(mode >= 0 && mode < MAX_WINDOW) {
		// window
		int width = emu->get_window_width(mode);
		int height = emu->get_window_height(mode);
		AG_Rect rect = {0, 0, width, height};

		AG_WindowSetGeometryRect(window, rect, 0); // OK?
		int dest_x = 0;
		int dest_y = 0;
		//dest_x = (dest_x < 0) ? 0 : dest_x;
		dest_y = (dest_y < 0) ? 0 : dest_y;
		
		if(now_fullscreen) {
		  //ChangeDisplaySettings(NULL, 0);
			//SetWindowLong(hWnd, GWL_STYLE, style);
			//SetWindowPos(hWnd, AG_Widget *_TOP, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
			now_fullscreen = false;
			
			// show menu
			if(hMenu != NULL) AG_WidgetShow(AGWIDGET(hMenu));
		} else {
		  //SetWindowPos(hWnd, NULL, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		}
		
		//AG_Rect2 rect_tmp;
		//GetClientRect(hWnd, &rect_tmp);
		//if(rect_tmp.bottom != height) {
		//	rect.bottom += height - rect_tmp.bottom;
		//	dest_y = (int)((desktop_height - (rect.bottom - rect.top)) / 2);
		//	dest_y = (dest_y < 0) ? 0 : dest_y;
		//	SetWindowPos(hWnd, NULL, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		//}
		config.window_mode = prev_window_mode = mode;
		
		// set screen size to emu class
		emu->set_display_size(width, height, true);
	} else if(!now_fullscreen) {
		// fullscreen
		int width = (mode == -1) ? desktop_width : screen_mode_width[mode - MAX_WINDOW];
		int height = (mode == -1) ? desktop_height : screen_mode_height[mode - MAX_WINDOW];
		
		config.window_mode = mode;
			
			// remove menu
		if(hMenu != NULL) AG_WidgetHide(AGWIDGET(hMenu));
			
			// set screen size to emu class
		emu->set_display_size(width, height, false);
	}
}




/*
 * This is main for agar.
 */
int main(int argc, char *argv[])
{
     int rgb_size[3];
     int flags;
     char simdstr[1024];
     char *optArg;
     std::string archstr;
     std::string delim;
     int  bLogSYSLOG;
     int  bLogSTDOUT;
     char c;
     int nErrorCode;
     /*
      * Get current DIR
      */
     char    *p;
     int simdid = 0;
       /*
	* Check CPUID
	*/
     pCpuID = initCpuID();
     if(argc > 0) {
       if(argv[0] != NULL) {
	 my_procname = argv[0];
       } else {
	 my_procname = "CommonSourceProject";
       }
     } else {
	 my_procname = "CommonSourceProject";
     }
#ifdef _WINDOWS
     delim = "\\";
#else
     delim = "/";
#endif
     
     p = SDL_getenv("HOME");
     if(p == NULL) {
       p = SDL_getenv("PWD");
       if(p == NULL) {
	 cpp_homedir = ".";
       } else {
	 cpp_homedir = p;
       }
       std::string tmpstr;
       tmpstr = "Warning : Can't get HOME directory...Making conf on " +  cpp_homedir + delim;
       perror(tmpstr.c_str());
     } else {
       cpp_homedir = p;
     }
     cpp_homedir = cpp_homedir + delim;
#ifdef _WINDOWS
     cpp_confdir = cpp_homedir + my_procname + delim;
#else
     cpp_confdir = cpp_homedir + ".config" + delim + my_procname + delim;
#endif
     AG_InitCore(my_procname.c_str(), AG_VERBOSE | AG_CREATE_DATADIR);
     AG_ConfigLoad();

     if(AG_GetVariable(agConfig, "logger.syslog", NULL) == NULL) { 
       AG_SetInt(agConfig, "logger.syslog", FALSE);
     }
     if(AG_GetVariable(agConfig, "logger.stdout", NULL) == NULL) { 
       AG_SetInt(agConfig, "logger.stdout", TRUE);
     }
     bLogSYSLOG = (int)AG_GetInt(agConfig, "logger.syslog");
     bLogSTDOUT = (int)AG_GetInt(agConfig, "logger.stdout");
     AGAR_OpenLog(bLogSYSLOG, bLogSTDOUT); // Write to syslog, console

     archstr = "Generic";
#if defined(__x86_64__)
     archstr = "amd64";
#endif
#if defined(__i386__)
     archstr = "ia32";
#endif
     printf("Common Source Project : %s %s\n", my_procname.c_str(), "1.0");
     printf("(C) Toshiya Takeda / SDL Version K.Ohta <whatisthis.sowhat@gmail.com>\n");
     printf("Architecture: %s\n", archstr.c_str());
     printf(" -? is print help(s).\n");
   
        /* Print SIMD features */ 
     simdstr[0] = '\0';
#if defined(__x86_64__) || defined(__i386__)
        if(pCpuID != NULL) {
	   
	   if(pCpuID->use_mmx) {
	      strcat(simdstr, " MMX");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_mmxext) {
	      strcat(simdstr, " MMXEXT");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_sse) {
	      strcat(simdstr, " SSE");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_sse2) {
	      strcat(simdstr, " SSE2");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_sse3) {
	      strcat(simdstr, " SSE3");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_ssse3) {
	      strcat(simdstr, " SSSE3");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_sse41) {
	      strcat(simdstr, " SSE4.1");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_sse42) {
	      strcat(simdstr, " SSE4.2");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_sse4a) {
	      strcat(simdstr, " SSE4a");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_abm) {
	      strcat(simdstr, " ABM");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_avx) {
	      strcat(simdstr, " AVX");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_3dnow) {
	      strcat(simdstr, " 3DNOW");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_3dnowp) {
	      strcat(simdstr, " 3DNOWP");
	      simdid |= 0xffffffff;
	   }
	   if(simdid == 0) strcpy(simdstr, "NONE");
	} else {
	   strcpy(simdstr, "NONE");
	}
#else // Not ia32 or amd64
	strcpy(simdstr, "NONE");
#endif
   printf("Supported SIMD: %s\n", simdstr);
   cpp_simdtype = simdstr;


   if(AG_GetVariable(agConfig, "font.size", NULL) == NULL) { 
     AG_SetInt(agConfig, "font.size", 16);
   }
   sAG_Driver = "";
   while ((c = AG_Getopt(argc, argv, "?fWd:w:h:T:t:c:T:F:S:o:O:l:s:i:", &optArg, NULL))
	  != -1) {
	    switch (c) {
	    case 'd':
	      sAG_Driver = optArg;
	      break;
	    case 'f':
	      /* Force full screen */
	      AG_SetBool(agConfig, "view.full-screen", 1);
	      break;
	    case 'W':
	      /* Force Window */
	      AG_SetBool(agConfig, "view.full-screen", 0);
	      break;
	    case 'T':
	      /* Set an alternate font directory */
	      AG_SetString(agConfig, "font-path", optArg);
	      break;
	    case 'F':
	      /* Set an alternate font face */
	      AG_SetString(agConfig, "font.face", optArg);
	      break;
	    case 'S':
                  /* Set an alternate font face */
	      AG_SetInt(agConfig, "font.size", atoi(optArg));
	      break;
	    case 'o':
	      /* Set an alternate font face */
	      AG_SetString(agConfig, "osdfont.face", optArg);
	      break;
	    case 'O':
	      /* Set an alternate font face */
	      AG_SetInt(agConfig, "osdfont.size", atoi(optArg));
	      break;
	    case 'l':
	      /* Set an alternate font face */
	      AG_SetString(agConfig, "load-path", optArg);
	      break;
	    case 's':
	      /* Set an alternate font face */
	      AG_SetString(agConfig, "save-path", optArg);
	      break;
	    case 'i':
	      /* Set an alternate font face */
	      AG_SetString(agConfig, "save-path", optArg);
	      AG_SetString(agConfig, "load-path", optArg);
	      break;
	    case 't':
	      /* Change the default font */
	      AG_TextParseFontSpec(optArg);
	      break;
	    case '?':
	    default:
	      printf("%s [-v] [-f|-W] [-d driver] [-r fps] [-t fontspec] "
		     "[-w width] [-h height] "
		     "[-F font.face] [-S font.size]"
		     "[-o osd-font.face] [-O osd-font.size]"
		     "[-s SavePath] [-l LoadPath] "
		     "[-T font-path]\n\n"
		     "Usage:\n"
		     "-f : FullScreen\n-W:Window Mode\n",
		     agProgName);
	      exit(0);
	    }
    }

   
   AGAR_DebugLog(AGAR_LOG_INFO, "Start Common Source Project '%s'", my_procname.c_str());
   AGAR_DebugLog(AGAR_LOG_INFO, "(C) Toshiya Takeda / Agar Version K.Ohta");
   AGAR_DebugLog(AGAR_LOG_INFO, "Architecture: %s", archstr.c_str());

   AGAR_DebugLog(AGAR_LOG_DEBUG, "Start Common Source Project '%s'", my_procname.c_str());
   AGAR_DebugLog(AGAR_LOG_DEBUG, "(C) Toshiya Takeda / Agar Version K.Ohta");
   AGAR_DebugLog(AGAR_LOG_DEBUG, "Architecture: %s", archstr.c_str());
   AGAR_DebugLog(AGAR_LOG_DEBUG, "Supported SIMD: %s", cpp_simdtype.c_str());
   AGAR_DebugLog(AGAR_LOG_DEBUG, " -? is print help(s).");
   AGAR_DebugLog(AGAR_LOG_DEBUG, "Moduledir = %s home = %s", cpp_confdir.c_str(), cpp_homedir.c_str()); // Debug

   AG_MkPath(cpp_confdir.c_str());
   /* Gettext */
#ifndef RSSDIR
#if defined(_USE_AGAR) || defined(_USE_SDL)
   sRssDir = "/usr/local/share/";
#else
   sRssDir = "." + delim;
#endif
   sRssDir = sRssDir + "CommonSourceCodeProject" + delim + my_procname;
#else
   sRssDir = RSSDIR;
#endif
   
   setlocale(LC_ALL, "");
   bindtextdomain("messages", sRssDir.c_str());
   textdomain("messages");
   AGAR_DebugLog(AGAR_LOG_DEBUG, "I18N via gettext initialized.");
   AGAR_DebugLog(AGAR_LOG_DEBUG, "I18N resource dir: %s", sRssDir.c_str());
   

   //SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_TIMER);

#if ((AGAR_VER <= 2) && defined(FMTV151))
   bFMTV151 = TRUE;
#endif				/*  */
/*
 * アプリケーション初期化
 */
   nErrorCode = MainLoop(argc, argv);
   return nErrorCode;
}



