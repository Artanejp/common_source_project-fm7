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
//#include "agar_main.h"
#include "menuclasses.h"
//#include "agar_gldraw.h"
#include "qt_main.h"

#include <SDL/SDL.h>
//#include <agar/dev.h>


// emulation core
EMU* emu;
class Ui_MainWindow *rMainWindow;

// buttons
#ifdef USE_BUTTON
#define MAX_FONT_SIZE 32
#endif

// menu

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
bool bRunJoyThread = false;


void EmuThreadClass::run()
{
   int total_frames = 0, draw_frames = 0, skip_frames = 0;
   DWORD next_time = 0;
   DWORD update_fps_time = 0;
   bool prev_skip = false;
   bRunEmuThread = true;
      
    do {
    
    if(emu) {
      int interval = 0, sleep_period = 0;			
      // drive machine
      int run_frames = emu->run();
      total_frames += run_frames;
       
      interval = 0;
      sleep_period = 0;
			// timing controls
      for(int i = 0; i < run_frames; i++) {
               interval += get_interval();
      }
      bool now_skip = emu->now_skip() && !emu->now_rec_video;
      if((prev_skip && !now_skip) || next_time == 0) {
	next_time = timeGetTime();
      }
      if(!now_skip) {
	next_time += interval;
      }
      prev_skip = now_skip;
      //printf("EMU::RUN Frames = %d Interval = %d NextTime = %d\n", run_frames, interval, next_time);
     
      if(next_time > timeGetTime()) {
	// update window if enough time
	draw_frames += emu->draw_screen();
	//emu->update_screen(hScreenWidget);// Okay?
	skip_frames = 0;
	
	// sleep 1 frame priod if need
	DWORD current_time = timeGetTime();
	if((int)(next_time - current_time) >= 10) {
	  sleep_period = next_time - current_time;
	}
      } else if(++skip_frames > MAX_SKIP_FRAMES) {
	// update window at least once per 10 frames
	draw_frames += emu->draw_screen();
	//emu->update_screen(hScreenWidget);// Okay?
	//printf("EMU::Updated Frame %d\n", AG_GetTicks());
	skip_frames = 0;
	next_time = timeGetTime();
      }
      SDL_Delay(sleep_period);
      if(bRunEmuThread != true) {
	exit();
	//return;
      }
      // calc frame rate
      DWORD current_time = timeGetTime();
      if(update_fps_time <= current_time && update_fps_time != 0) {
	_TCHAR buf[256];
	int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);
	if(emu->message_count > 0) {
	  sprintf(buf, _T("%s - %s"), DEVICE_NAME, emu->message);
	  emu->message_count--;
	} else {
	  sprintf(buf, _T("%s - %d fps (%d %%)"), DEVICE_NAME, draw_frames, ratio);
	}
	AG_WindowSetCaptionS(hWindow, buf);
	update_fps_time += 1000;
	total_frames = draw_frames = 0;
      }
      if(update_fps_time <= current_time) {
	update_fps_time = current_time + 1000;
      }
    } else {
      SDL_Delay(10);
      if(bRunEmuThread != true) {
	exit();
      }
    }
  } while(1);
#endif
}

void JoyThreadClass::run()
{
  bRunJoyThread = TRUE;
  do {
    // Event Handling for joystick
    SDL_Delay(10); // Right?
  }
}

static EmuThreadClass *hEmuThread;
static JoyThreadClass *hJoyThread;

// Important Flags
AGAR_CPUID *pCpuID;

//#ifdef USE_ICONV
#include <iconv.h>
//#endif

void Convert_CP932_to_UTF8(char *dst, char *src, int n_limit)
{
//#ifdef USE_ICONV
  char          *pIn, *pOut;
  iconv_t       hd;
  size_t        in, out;
  char utf8[1024];
  char s_in[1024];
   
//  printf("Convert CP932 to UTF8: %08x, %d\n", src, n_limit);
   
  if((n_limit <= 0) || (src == NULL)) return;
  if(n_limit > 1023) n_limit = 1023;
  strncpy(s_in, src, n_limit); 
  pIn = s_in;
  pOut = utf8;
  in = strlen(pIn);
  
  out = n_limit;
  hd = iconv_open("utf-8", "cp932");
  if((hd >= 0) && (in > 0)){
    while(in>0) {
      iconv(hd, &pIn, &in, &pOut, &out);
    }
    iconv_close(hd);
  }
//  printf("UTF8: %s\n", pOut);
  strncpy(dst, utf8, n_limit);
//#else
//  strncpy(dst, src, n_limit);
//#endif
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
  //s = AG_ShortFilename(src);
  r_path = r_path + my_procname + delim;
  AG_MkPath(r_path.c_str());
  ss = "";
//  if(s != NULL) ss = s;
  r_path = r_path + ss;
  if(dst != NULL) r_path.copy(dst, r_path.length() >= AG_PATHNAME_MAX ? AG_PATHNAME_MAX : r_path.length(), 0);
  return;
}

_TCHAR* get_parent_dir(_TCHAR* file)
{
#ifdef _WINDOWS
	char delim = '\\';
#else
	char delim = '/';
#endif
        int ptr;
        char *p = (char *)file;
        if(file == NULL) return NULL;
        for(ptr = strlen(p) - 1; ptr >= 0; ptr--) { 
	   if(p[ptr] == delim) break;
	}
        if(ptr >= 0) for(ptr = ptr + 1; ptr < strlen(p); ptr++) p[ptr] = '\0'; 
	return p;
}

void get_short_filename(_TCHAR *dst, _TCHAR *file, int maxlen)
{
   int i, l;
   if((dst == NULL) || (file == NULL)) return;
#ifdef _WINDOWS
	_TCHAR delim = '\\';
#else
	_TCHAR delim = '/';
#endif
   for(i = strlen(file) - 1; i <= 0; i--) {
	if(file[i] == delim) break;
   }
   if(i >= (strlen(file) - 1)) {
      dst[0] = '\0';
      return;
   }
   l = strlen(file) - i + 1;
   if(l >= maxlen) l = maxlen;
   strncpy(dst, &file[i + 1], l);
   return;
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

void set_window(QMainWindow * hWnd, int mode);

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


  void LostFocus(QWidget *widget)
  {
    if(emu) {
      emu->key_lost_focus();
    }
  }

  void OnMainWindowClosed(Ui_MainWindow *p)
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
    save_config();
    if(hEmuThread != NULL) {
      hEmuThread->terminate();
      do {
	SDL_Delay(50);
	if(hEmuThread->finished()) break;
      } while(1);
      delete hEmuThread;
      hEmuThread = NULL;
    }
    bRunJoyThread = false;
    if(hJoyThread != NULL) {
      hJoyThread->terminate();
      do {
	SDL_Delay(50);
	if(hJoyThread->finished()) break;
      } while(1);
      delete hJoyThread;
      hJoyThread = NULL;
    }
    // release emulation core
    if(emu) {
      SDL_Delay(50);
      delete emu;
      emu = NULL;
    }
    // Detach Resource?
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

  void OnWindowResize(QMainWindow *p)
  {
    if(emu) {
      //if(now_fullscreen) {
	//emu->set_display_size(-1, -1, false);
      //} else {
	set_window(p, config.window_mode);
      //}
    }
  }

  
}  // extern "C"


bool InitInstance(void)
{
  rMainWindow = new Ui_MainWindow();
  rMainWindow->setupUi();
}  

#ifdef TRUE
#undef TRUE
#define TRUE true
#endif

#ifdef FALSE
#undef FALSE
#define FALSE false
#endif

#ifndef FONTPATH
#define FONTPATH "."
#endif


BOOL EventGuiSingle(AG_Driver *drv, AG_DriverEvent *ev)
{
   if(ev == NULL) return TRUE;
   /* Retrieve the next queued event. */
   if (AG_ProcessEvent(drv, ev) == -1) 	return FALSE;
   /* Forward the event to Agar. */
   return TRUE;
}

void ConvertSDLEvent(AG_Driver *obj, SDL_Event *event, AG_DriverEvent *dev)
{
//	AG_SDL_GetNextEvent(void *obj, AG_DriverEvent *dev)
	AG_Driver *drv = AGDRIVER(obj);
	SDL_Event ev = *event;
//        if(drv == NULL) return;

//	if(agDriverSw) {
//		drv = &agDriverSw->_inherit;
//	} else {
//
//	}

	switch (ev.type) {
	 case SDL_MOUSEMOTION:
		if(drv != NULL) AG_MouseMotionUpdate(drv->mouse, ev.motion.x, ev.motion.y);
		dev->type = AG_DRIVER_MOUSE_MOTION;
		dev->win = NULL;
		dev->data.motion.x = ev.motion.x;
		dev->data.motion.y = ev.motion.y;
		break;
	case SDL_MOUSEBUTTONUP:
		if(drv != NULL) AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_RELEASED,
				ev.button.button);
		dev->type = AG_DRIVER_MOUSE_BUTTON_UP;
		dev->win = NULL;
		dev->data.button.which = (AG_MouseButton)ev.button.button;
		dev->data.button.x = ev.button.x;
		dev->data.button.y = ev.button.y;
		break;
	case SDL_MOUSEBUTTONDOWN:
		if(drv != NULL) AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_PRESSED,
				ev.button.button);

		dev->type = AG_DRIVER_MOUSE_BUTTON_DOWN;
		dev->win = NULL;
		dev->data.button.which = (AG_MouseButton)ev.button.button;
		dev->data.button.x = ev.button.x;
		dev->data.button.y = ev.button.y;
		break;
	case SDL_KEYDOWN:
		if(drv != NULL) AG_KeyboardUpdate(drv->kbd, AG_KEY_PRESSED,
				(AG_KeySym)ev.key.keysym.sym,
				(Uint32)ev.key.keysym.unicode);

		dev->type = AG_DRIVER_KEY_DOWN;
		dev->win = NULL;
		dev->data.key.ks = (AG_KeySym)ev.key.keysym.sym;
		dev->data.key.ucs = (Uint32)ev.key.keysym.unicode;
		break;
	case SDL_KEYUP:
		if(drv != NULL) AG_KeyboardUpdate(drv->kbd, AG_KEY_RELEASED,
				(AG_KeySym)ev.key.keysym.sym,
				(Uint32)ev.key.keysym.unicode);

		dev->type = AG_DRIVER_KEY_UP;
		dev->win = NULL;
		dev->data.key.ks = (AG_KeySym)ev.key.keysym.sym;
		dev->data.key.ucs = (Uint32)ev.key.keysym.unicode;
		break;
	 case SDL_VIDEORESIZE:
		dev->type = AG_DRIVER_VIDEORESIZE;
		dev->win = NULL;
		dev->data.videoresize.x = 0;
		dev->data.videoresize.y = 0;
		dev->data.videoresize.w = (int)ev.resize.w;
		dev->data.videoresize.h = (int)ev.resize.h;
		break;
	case SDL_VIDEOEXPOSE:
		dev->type = AG_DRIVER_EXPOSE;
		dev->win = NULL;
		break;
	case SDL_QUIT:
	case SDL_USEREVENT:
		dev->type = AG_DRIVER_CLOSE;
		dev->win = NULL;
		break;
	 default:
	        dev->type = AG_DRIVER_UNKNOWN;
	        dev->win = NULL;
	        break;
	}
}



BOOL EventSDL(AG_Driver *drv)
{
//	SDL_Surface *p;
	SDL_Event eventQueue;
	AG_DriverEvent event;

	/*
	 * JoyStickなどはSDLが管理する
	 */
//	AG_SDL_GetNextEvent(void *obj, AG_DriverEvent *dev)

//	if(SDL_WasInit(SDL_INIT_JOYSTICK) != 0) {
//		p = SDL_GetVideoSurface();
//		if(p == NULL) return TRUE;
		while (SDL_PollEvent(&eventQueue))
//		if(SDL_PollEvent(&eventQueue))
		{
			switch (eventQueue.type)
			{
			case SDL_JOYAXISMOTION:	/* JS */
				//OnMoveJoy(&eventQueue);
				break;
			case SDL_JOYBUTTONDOWN:
				//OnPressJoy(&eventQueue);
				break;
			case SDL_JOYBUTTONUP:
				//OnReleaseJoy(&eventQueue);
				break;
			default:
				ConvertSDLEvent(drv, &eventQueue, &event);
			        if(!EventGuiSingle(drv, &event)) return FALSE;
				break;
			}
		}
//	}
	return TRUE;
}



void AGDrawTaskEvent(BOOL flag)
{
   Uint32 nDrawTick2D;
   AG_Window *win;
   AG_Driver *drv;
   Uint32 fps;
   Uint32 oldfps;
   BOOL skipf = FALSE;
   AG_EventSource *src;
   AG_EventSink *es;
   
   // TMPVARS
   bool bEventRunFlag = TRUE;
   Uint32 nDrawTick1D = AG_GetTicks();
   Uint32 nDrawFPS = 30;
   oldfps = nDrawFPS;
   nDrawTick2D = AG_GetTicks();

   src = AG_GetEventSource();
   
   AG_TAILQ_FOREACH(es, &src->prologues, sinks){
	                es->fn(es, &es->fnArgs);
   }
   if(nDrawFPS > 2) {
      fps = 1000 / nDrawFPS;
   } else {
      fps = 500;
   }
   if(fps < 10) fps = 10; // 10ms = 100fps.
   
   for(;;) {
      if(bEventRunFlag == FALSE) return;
      if(oldfps != nDrawFPS){ // FPS Change 20120120
	 oldfps = nDrawFPS;
	 if(nDrawFPS > 2) {
	    fps = 1000 / nDrawFPS;
	 } else {
	    fps = 500;
	 }
	 if(fps < 10) fps = 10; // 10ms = 100fps.
      }
      if(EventSDL(NULL) == FALSE) return;
      nDrawTick2D = AG_GetTicks();

      if(nDrawTick2D < nDrawTick1D) nDrawTick1D = 0; // オーバーフロー対策
      if((nDrawTick2D - nDrawTick1D) >= fps) {
	 if(skipf != TRUE){
	    AG_WindowDrawQueued();
	    nDrawTick1D = nDrawTick2D;
	    //if(((XM7_timeGetTime() - nDrawTick2D) >= (fps / 4)) && (agDriverSw != NULL)) skipf = TRUE;
	    AG_Delay(1);
	    continue;
	 } else {
	      if((nDrawTick2D - nDrawTick1D) >= ((fps * 2) - 1)) {
		 skipf = FALSE;
		 continue;
	      }
	    
	 }
//	 AG_Delay(1);
      } 
      if(AG_PendingEvents(NULL) != 0) {
	 AG_DriverEvent dev;
	 if(EventSDL(NULL) == FALSE) return;
	 if(AG_GetNextEvent(NULL, &dev) == 1) AG_ProcessEvent(NULL, &dev);
//	 AG_Delay(1);
      }
      { // Timeout
	 src = AG_GetEventSource();
	 if(src == NULL) return;
	 AG_TAILQ_FOREACH(es, &src->spinners, sinks){
	    if(bEventRunFlag == FALSE) return;
	    es->fn(es, &es->fnArgs);
	 }
	 if (src->sinkFn() == -1) {
	    return;
	 }
	 AG_TAILQ_FOREACH(es, &src->epilogues, sinks) {
	    if(bEventRunFlag == FALSE) return;
	    es->fn(es, &es->fnArgs);
	 }
	 if (src->breakReq) return;
	 AG_Delay(1);
      }	// Process Event per 1Ticks;

      AG_WindowProcessQueued();
   }
   
}


int MainLoop(int argc, char *argv[])
{
	  char c;
	  char strbuf[2048];
	  bool flag;
          char homedir[AG_PATHNAME_MAX];
          int thread_ret;
          
          cpp_homedir.copy(homedir, AG_PATHNAME_MAX - 1, 0);
	  flag = FALSE;
	/*
	 * Into Qt's Loop.
	 */

   
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
        set_window(rMainWindow->getWindow(), config.window_mode);


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
	// Launch Emulator loop
	bRunEmuThread = false;
	hEmuThread = new EmuThreadClass(rMainWindow);
	hEmuThread.run();
	// Launch JoystickClass
	bRunJoyThread = false;
	hJoyThread = new JoyThreadClass(rMainWindow);
	hJoyThread.run();
	
	//AG_EventLoop(); // Right? maybe unusable Joystick.
	Qt_GuiMain(argc, argv);
	return 0;
}



void set_window(QMainWindow *hWnd, int mode)
{
 
//	static LONG style = WS_VISIBLE;

	if(hWnd == NULL) return;
	
	if(mode >= 0 && mode < MAX_WINDOW) {
		// window
		int width = emu->get_window_width(mode);
		int height = emu->get_window_height(mode);

		hWnd->resize(width + 10, height + 100); // OK?
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
		emu->suspend();
		emu->set_display_size(width, height, true);
	} else if(!now_fullscreen) {
		// fullscreen
		int width = (mode == -1) ? desktop_width : screen_mode_width[mode - MAX_WINDOW];
		int height = (mode == -1) ? desktop_height : screen_mode_height[mode - MAX_WINDOW];
		
		config.window_mode = mode;
			
			// remove menu
		if(hMenu != NULL) AG_WidgetHide(AGWIDGET(hMenu));
		emu->suspend();
			
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
    
    // if(argc > 0) {
    //   if(argv[0] != NULL) {
    //	 my_procname = AG_ShortFilename(argv[0]);
    //   } else {
    //	 my_procname = "CommonSourceProject";
    //   }
    // } else {
	// my_procname = "CommonSourceProject";
    // }
    my_procname = "emu";
    my_procname = my_procname + CONFIG_NAME;
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
     bLogSYSLOG = (int)0;
     bLogSTDOUT = (int)1;
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
   AGAR_DebugLog(AGAR_LOG_DEBUG, "I18N via gettext initialized."); // Will move to Qt;
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



