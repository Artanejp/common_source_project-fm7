/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Agar : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -

	[ win32 main ] -> [ agar main ]
*/

#include <qdir.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "common.h"
#include "fileio.h"
#include "emu.h"
#include "emu_utils.h"
#include "menuclasses.h"
#include "qt_main.h"
#include "agar_logger.h"

// emulation core
EMU* emu;
QApplication *GuiMain = NULL;

// Start to define MainWindow.
class META_MainWindow *rMainWindow;

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


void EmuThreadClass::doExit(void)
{
	int status;
//    setRunEmuThread(false);
//    if(hRunEmuThread != NULL) SDL_WaitThread(hRunEmuThread, &status);
	bRunThread = false;
//    delete_emu_thread();
}

void EmuThreadClass::set_tape_play(bool flag)
{
#ifdef USE_TAPE_BUTTON
	tape_play_flag = flag;
#endif
}

void EmuThreadClass::print_framerate(int frames)
{
	if(frames >= 0) draw_frames += frames;
	if(calc_message) {
			DWORD current_time = timeGetTime();
			if(update_fps_time <= current_time && update_fps_time != 0) {
				_TCHAR buf[256];
				QString message;
				int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);
#ifdef USE_POWER_OFF
				if(rMainWindow->GetPowerState() == false){ 	 
					snprintf(buf, 255, _T("*Power OFF*"));
				} else {
#endif // USE_POWER_OFF		
					if(p_emu->message_count > 0) {
						snprintf(buf, 255, _T("%s - %s"), DEVICE_NAME, p_emu->message);
						p_emu->message_count--;
					} else {
						snprintf(buf, 255, _T("%s - %d fps (%d %%)"), DEVICE_NAME, draw_frames, ratio);
					}
#ifdef USE_POWER_OFF
				} 
#endif // USE_POWER_OFF	 
	      
				message = buf;
				emit message_changed(message);
				update_fps_time += 1000;
				total_frames = draw_frames = 0;
				
			}
			if(update_fps_time <= current_time) {
				update_fps_time = current_time + 1000;
			}
			calc_message = false;  
		} else {
			calc_message = true;
		}
}


void EmuThreadClass::doWork(const QString &params)
{
	int interval = 0, sleep_period = 0;			
	if(rMainWindow == NULL) {
		//emit sig_finished();
		goto _exit;
	}
	if(bRunThread == false) {
		goto _exit;
	}
   do {
	
	if(p_emu) {
	 // drive machine
		int run_frames = p_emu->run();
		total_frames += run_frames;
		p_emu->LockVM();
#ifdef USE_TAPE_BUTTON
		bool tape_flag = p_emu->get_tape_play();
		if(tape_play_flag != tape_flag) emit sig_tape_play_stat(tape_flag);
		tape_play_flag = tape_flag;
#endif
		interval = 0;
		sleep_period = 0;
      
		// timing controls
		//for(int i = 0; i < run_frames; i++) {
			interval += get_interval();
		//}

		bool now_skip = p_emu->now_skip() & !p_emu->now_rec_video;
		p_emu->UnlockVM();
		if((prev_skip && !now_skip) || next_time == 0) {
			next_time = timeGetTime();
		}
		if(!now_skip) {
			next_time += interval;
		}
		prev_skip = now_skip;
		//printf("p_emu::RUN Frames = %d SKIP=%d Interval = %d NextTime = %d\n", run_frames, now_skip, interval, next_time);
      
		if(next_time > timeGetTime()) {
			//  update window if enough time
			emit sig_draw_thread();
			skip_frames = 0;
			
			// sleep 1 frame priod if need
			DWORD current_time = timeGetTime();
			if((int)(next_time - current_time) >= 10) {
				sleep_period = next_time - current_time;
			}
		} else if(++skip_frames > MAX_SKIP_FRAMES) {
			// update window at least once per 10 frames
			emit sig_draw_thread();

			//printf("p_emu::Updated Frame %d\n", AG_GetTicks());
			skip_frames = 0;
			next_time = timeGetTime() + get_interval();
			sleep_period = next_time - timeGetTime();
		}
		
		//	    timer.setInterval(sleep_period);
		if(bRunThread == false){
			goto _exit;
		}
		if(sleep_period <= 0) sleep_period = 1;
		//      SDL_Delay(sleep_period);
		if(bRunThread == false){
			goto _exit;
		}
		// calc frame rate
	if(sleep_period <= 0) sleep_period = 1; 
	}
        msleep(sleep_period);
   } while(1);
 _exit:
	AGAR_DebugLog(AGAR_LOG_DEBUG, "EmuThread : EXIT");
	emit sig_finished();
	return;
}


DrawThreadClass::DrawThreadClass(QObject *parent) : QThread(parent) {
	MainWindow = (Ui_MainWindow *)parent;
}

void DrawThreadClass::doDraw(void)
{
	p_emu->LockVM();
	draw_frames = p_emu->draw_screen();
	p_emu->update_screen(MainWindow->getGraphicsView());// Okay?
	p_emu->UnlockVM();
	emit sig_draw_frames(draw_frames);
}

void DrawThreadClass::doExit(void)
{
	bRunThread = false;
//	this->quit();
	this->wait();
	AGAR_DebugLog(AGAR_LOG_DEBUG, "DrawThread : Exit.");
}

void DrawThreadClass::doWork(const QString &param)
{
}




void Ui_MainWindow::doExit_EmuThread(void)
{
	int status;
	setRunEmuThread(false);
	if(hRunEmuThread != NULL) SDL_WaitThread(hRunEmuThread, &status);
	delete_emu_thread();
}

void Ui_MainWindow::doChangeMessage_EmuThread(QString message)
{
      emit message_changed(message);
}


void Ui_MainWindow::LaunchEmuThread(void)
{
	QString objNameStr;
	hRunEmu = new EmuThreadClass(this);
	hRunEmu->SetEmu(emu);
	connect(hRunEmu, SIGNAL(message_changed(QString)), this, SLOT(message_status_bar(QString)));
	connect(hRunEmu, SIGNAL(sig_finished()), this, SLOT(delete_emu_thread()));
	
	//connect(&(hRunEmu->timer), SIGNAL(timeout()), hRunEmu, SLOT(doWork()));
	connect(this, SIGNAL(quit_emu_thread()), hRunEmu, SLOT(doExit()));
#ifdef USE_TAPE_BUTTON
	hRunEmu->set_tape_play(false);
	connect(hRunEmu, SIGNAL(sig_tape_play_stat(bool)), this, SLOT(do_display_tape_play(bool)));
#endif   
	connect(actionExit_Emulator, SIGNAL(triggered()), hRunEmu, SLOT(doExit()));
	//hRunEmu->timer.setSingleShot(true);
	this->set_screen_aspect(config.stretch_type);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "EmuThread : Start.");
	objNameStr = QString("EmuThreadClass");
	hRunEmu->setObjectName(objNameStr);
   
	hDrawEmu = new DrawThreadClass(this);
	hDrawEmu->SetEmu(emu);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "DrawThread : Start.");
	connect(hDrawEmu, SIGNAL(sig_draw_frames(int)), hRunEmu, SLOT(print_framerate(int)));
	connect(hDrawEmu, SIGNAL(message_changed(QString)), this, SLOT(message_status_bar(QString)));
	connect(hRunEmu, SIGNAL(sig_draw_thread()), hDrawEmu, SLOT(doDraw()));
	connect(hRunEmu, SIGNAL(quit_draw_thread()), hDrawEmu, SLOT(doExit()));
	//connect(hRunEmu, SIGNAL(sig_finished()), this, SLOT(delete_draw_thread()));
	objNameStr = QString("EmuDrawThread");
	hDrawEmu->setObjectName(objNameStr);
	hDrawEmu->start();
	AGAR_DebugLog(AGAR_LOG_DEBUG, "DrawThread : Launch done.");

	hRunEmu->start();
	AGAR_DebugLog(AGAR_LOG_DEBUG, "EmuThread : Launch done.");

}


void Ui_MainWindow::StopEmuThread(void) {
	emit quit_emu_thread();
}

void Ui_MainWindow::delete_emu_thread(void)
{
	do_release_emu_resources();
	emit sig_quit_all();
}  
   
// Important Flags
AGAR_CPUID *pCpuID;

//#ifdef USE_ICONV
#include <iconv.h>
//#endif

void Convert_CP932_to_UTF8(char *dst, char *src, int n_limit, int i_limit)
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
  if(i_limit > 1023) i_limit = 1023;
  memset(utf8, 0x00, sizeof(utf8));
  memset(s_in, 0x00, sizeof(s_in));
  memset(dst,  0x00, n_limit);
  //strncpy(s_in, src, i_limit); 
  //pIn = s_in;
  pIn = src;
  pOut = utf8;
  in = strlen(pIn);
  if(in > i_limit) in = i_limit;
  if(in <= 0) return;
   
  out = n_limit;
  hd = iconv_open("utf-8", "cp932");
  if((hd >= 0) && (in > 0)){
    while(in > 0) {
      iconv(hd, &pIn, &in, &pOut, &out);
    }
    iconv_close(hd);
  }
//  printf("UTF8: %s\n", pOut);
  if(out <= 0) return;
  strncpy(dst, utf8, out);
//#else
//  strncpy(dst, src, n_limit);
//#endif
}   

void get_long_full_path_name(_TCHAR* src, _TCHAR* dst)
{
   QString r_path;
   QString delim;
   QString ss;
  const char *s;
  QDir mdir;
  if(src == NULL) {
    if(dst != NULL) dst[0] = '\0';
    return;
  }
#ifdef _WINDOWS
  delim = "\\";
#else
  delim = "/";
#endif
  ss = "";
   
  if(cpp_homedir == "") {
    r_path = mdir.currentPath();
  } else {
     r_path = QString::fromStdString(cpp_homedir);
  }
  //s = AG_ShortFilename(src);
  r_path = r_path + QString::fromStdString(my_procname);
  r_path = r_path + delim;
  mdir.mkdir(r_path);
  ss = "";
//  if(s != NULL) ss = s;
//  r_path.append(ss);
  if(dst != NULL) strncpy(dst, r_path.toUtf8().constData(),
			  strlen(r_path.toUtf8().constData()) >= PATH_MAX ? PATH_MAX : strlen(r_path.toUtf8().constData()));
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

//void set_window(QMainWindow * hWnd, int mode);

void Ui_MainWindow::OnWindowRedraw(void)
{
  if(emu) {
      emu->update_screen(this->getGraphicsView());
  }
}

void Ui_MainWindow::OnWindowMove(void)
{
    if(emu) {
      emu->suspend();
    }
}

void Ui_MainWindow::OnWindowResize(void)
{
  if(emu) {
    //if(now_fullscreen) {
    //emu->set_display_size(-1, -1, false);
    //} else {
    set_window(config.window_mode);
    //}
  }
}

#ifdef USE_POWER_OFF
bool Ui_MainWindow::GetPowerState(void)
{
   if(close_notified == 0) return true;
   return false;
}
#endif
void Ui_MainWindow::OnMainWindowClosed(void)
{
    
#ifdef USE_POWER_OFF
    // notify power off
    if(emu) {
      if(!close_notified) {
	emu->LockVM();
	emu->notify_power_off();
	emu->UnlockVM();
	close_notified = 1;
	return; 
      }
    }
#endif
    setRunJoyThread(false);
    emit quit_joy_thread();
    
    setRunEmuThread(false);
    emit quit_emu_thread();

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
    return;
}

extern "C" {

  void LostFocus(QWidget *widget)
  {
    if(emu) {
      emu->key_lost_focus();
    }
  }
 
}  // extern "C"

void Ui_MainWindow::do_release_emu_resources(void)
{
    if(emu) {
      delete emu;
      emu = NULL;
    }
}


bool InitInstance(int argc, char *argv[])
{
  rMainWindow = new META_MainWindow();
  rMainWindow->connect(rMainWindow, SIGNAL(sig_quit_all(void)), rMainWindow, SLOT(deleteLater(void)));
  return true;
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




int MainLoop(int argc, char *argv[])
{
  char c;
  char strbuf[2048];
  bool flag;
  char homedir[PATH_MAX];
  int thread_ret;
          
  cpp_homedir.copy(homedir, PATH_MAX - 1, 0);
  flag = FALSE;
  /*
   * Into Qt's Loop.
   */
   
  SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK );
  AGAR_DebugLog(AGAR_LOG_DEBUG, "Audio and JOYSTICK subsystem was initialised.");
  GuiMain = new QApplication(argc, argv);

  load_config();
  InitInstance(argc, argv);
  AGAR_DebugLog(AGAR_LOG_DEBUG, "InitInstance() OK.");
  
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
  // disenable ime
  //ImmAssociateContext(hWnd, 0);
	
  // initialize emulation core
  rMainWindow->getWindow()->show();
  
  emu = new EMU(rMainWindow, rMainWindow->getGraphicsView());
  
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
	
  // main loop
  rMainWindow->LaunchEmuThread();
  rMainWindow->LaunchJoyThread();
  GuiMain->exec();
  return 0;
}



void Ui_MainWindow::set_window(int mode)
{
        QMenuBar *hMenu;
//	static LONG style = WS_VISIBLE;

	if(mode >= 0 && mode < MAX_WINDOW) {
		// window
		int width = emu->get_window_width(mode);
		int height = emu->get_window_height(mode);

		this->resize(width + 10, height + 100); // OK?
		int dest_x = 0;
		int dest_y = 0;
		dest_x = (dest_x < 0) ? 0 : dest_x;
		dest_y = (dest_y < 0) ? 0 : dest_y;
		
		config.window_mode = prev_window_mode = mode;
		
		// set screen size to emu class
		emu->suspend();
		emu->set_display_size(width, height, true);
	        if(rMainWindow) rMainWindow->getGraphicsView()->resize(width, height);

	} else if(!now_fullscreen) {
		// fullscreen
		int width = (mode == -1) ? desktop_width : screen_mode_width[mode - MAX_WINDOW];
		int height = (mode == -1) ? desktop_height : screen_mode_height[mode - MAX_WINDOW];
		
		config.window_mode = mode;
		emu->suspend();
		// set screen size to emu class
		emu->set_display_size(width, height, false);
	        
	       if(rMainWindow) rMainWindow->getGraphicsView()->resize(width, height);
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
   AGAR_DebugLog(AGAR_LOG_INFO, "Supported SIMD: %s", simdstr);
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

     {
	QDir dir;
	dir.mkdir( QString::fromStdString(cpp_confdir));
     }
   
   //AG_MkPath(cpp_confdir.c_str());
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



