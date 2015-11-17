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
#include <QString>
#include <QTextCodec>
#include <QImage>
#include <QImageReader>

#include "common.h"
#include "fileio.h"
#include "emu.h"
#include "emu_utils.h"
#include "menuclasses.h"
#include "mainwidget.h"
#include "commonclasses.h"
#include "qt_main.h"
#include "emu_thread.h"
#include "joy_thread.h"
#include "draw_thread.h"

#include "qt_gldraw.h"
#include "agar_logger.h"

#include "menu_disk.h"
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
// timing control

// screen
unsigned int desktop_width;
unsigned int desktop_height;
//int desktop_bpp;
int prev_window_mode = 0;
bool now_fullscreen = false;

int window_mode_count;



void Ui_MainWindow::set_window(int mode)
{
	QMenuBar *hMenu;
	//	static LONG style = WS_VISIBLE;

	if(mode >= 0 && mode < _SCREEN_MODE_NUM) {
		if(mode >= screen_mode_count) return;
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
		emit sig_emu_set_display_size(width, height, true);
		if(rMainWindow) {
			rMainWindow->getGraphicsView()->resize(width, height);
			rMainWindow->resize_statusbar(width, height);
		}
	} else if(!now_fullscreen) {
		// fullscreen
		if(mode >= screen_mode_count) return;
		int width;
		int height;
		if(mode < 0) {
			width = desktop_width;
			height = desktop_height;
		} else {
			double nd = actionScreenSize[mode]->binds->getDoubleValue();
			width = (int)(nd * (double)SCREEN_WIDTH);
			height = (int)(nd * (double)SCREEN_HEIGHT);
#if defined(USE_SCREEN_ROTATE)
			if(config.rotate_type) {
				int tmp_w = width;
				width = height;
				height = tmp_w;
			}
#endif	   

		}
		config.window_mode = mode;
		emit sig_emu_set_display_size(width, height, false);
		graphicsView->resize(width, height);
		this->resize_statusbar(width, height);
	}
}

void Ui_MainWindow::do_emu_update_config(void)
{
	emit sig_emu_update_config();
}

void Ui_MainWindow::doChangeMessage_EmuThread(QString message)
{
      emit message_changed(message);
}


void Ui_MainWindow::do_set_mouse_enable(bool flag)
{
	if(emu == NULL) return;
	emu->LockVM();
	if(flag) {
		graphicsView->grabMouse();
		emu->enable_mouse();
	} else {
		graphicsView->releaseMouse();
		emu->disenable_mouse();
	}
	emu->UnlockVM();
}

void Ui_MainWindow::do_toggle_mouse(void)
{
	if(emu == NULL) return;
	emu->LockVM();
	bool flag = emu->get_mouse_enabled();
	if(!flag) {
		graphicsView->grabMouse();
		emu->enable_mouse();
	} else {
		graphicsView->releaseMouse();
		emu->disenable_mouse();
	}
	emu->UnlockVM();
}

void Ui_MainWindow::LaunchEmuThread(void)
{
	QString objNameStr;
	GLDrawClass *glv = this->getGraphicsView();

	int drvs;
	
	hRunEmu = new EmuThreadClass(rMainWindow, emu, this);
	connect(hRunEmu, SIGNAL(message_changed(QString)), this, SLOT(message_status_bar(QString)));
	connect(hRunEmu, SIGNAL(sig_finished()), this, SLOT(delete_emu_thread()));
	connect(this, SIGNAL(sig_vm_reset()), hRunEmu, SLOT(doReset()));
	connect(this, SIGNAL(sig_vm_specialreset()), hRunEmu, SLOT(doSpecialReset()));
	connect(this, SIGNAL(sig_vm_loadstate()), hRunEmu, SLOT(doLoadState()));
	connect(this, SIGNAL(sig_vm_savestate()), hRunEmu, SLOT(doSaveState()));

	connect(this, SIGNAL(sig_emu_update_config()), hRunEmu, SLOT(doUpdateConfig()));
	connect(this, SIGNAL(sig_emu_start_rec_sound()), hRunEmu, SLOT(doStartRecordSound()));
	connect(this, SIGNAL(sig_emu_stop_rec_sound()), hRunEmu, SLOT(doStopRecordSound()));
	connect(this, SIGNAL(sig_emu_set_display_size(int, int, bool)), hRunEmu, SLOT(doSetDisplaySize(int, int, bool)));
	

#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
	connect(this, SIGNAL(sig_write_protect_disk(int, bool)), hRunEmu, SLOT(do_write_protect_disk(int, bool)));
	connect(this, SIGNAL(sig_open_disk(int, QString, int)), hRunEmu, SLOT(do_open_disk(int, QString, int)));
	connect(this, SIGNAL(sig_close_disk(int)), hRunEmu, SLOT(do_close_disk(int)));
	connect(hRunEmu, SIGNAL(sig_update_recent_disk(int)), this, SLOT(do_update_recent_disk(int)));
	connect(hRunEmu, SIGNAL(sig_change_osd_fd(int, QString)), this, SLOT(do_change_osd_fd(int, QString)));
	drvs = 0;
# if defined(USE_FD1)
	drvs = 1;
# endif
# if defined(USE_FD2)
	drvs = 2;
# endif
# if defined(USE_FD3)
	drvs = 3;
# endif
# if defined(USE_FD4)
	drvs = 4;
# endif
# if defined(USE_FD5)
	drvs = 5;
# endif
# if defined(USE_FD6)
	drvs = 6;
# endif
# if defined(USE_FD7)
	drvs = 7;
# endif
# if defined(USE_FD8)
	drvs = 8;
# endif
	for(int ii = 0; ii < drvs; ii++) {
		menu_fds[ii]->setEmu(emu);
	}
#endif
#if defined(USE_TAPE)
	connect(this, SIGNAL(sig_play_tape(QString)), hRunEmu, SLOT(do_play_tape(QString)));
	connect(this, SIGNAL(sig_rec_tape(QString)),  hRunEmu, SLOT(do_rec_tape(QString)));
	connect(this, SIGNAL(sig_close_tape(void)),   hRunEmu, SLOT(do_close_tape(void)));
	connect(hRunEmu, SIGNAL(sig_change_osd_cmt(QString)), this, SLOT(do_change_osd_cmt(QString)));
# if defined(USE_TAPE_BUTTON)
	connect(this, SIGNAL(sig_cmt_push_play(void)), hRunEmu, SLOT(do_cmt_push_play(void)));
	connect(this, SIGNAL(sig_cmt_push_stop(void)), hRunEmu, SLOT(do_cmt_push_stop(void)));
	connect(this, SIGNAL(sig_cmt_push_fast_forward(void)), hRunEmu, SLOT(do_cmt_push_fast_forward(void)));
	connect(this, SIGNAL(sig_cmt_push_fast_rewind(void)),  hRunEmu, SLOT(do_cmt_push_fast_rewind(void)));
	connect(this, SIGNAL(sig_cmt_push_apss_forward(void)), hRunEmu, SLOT(do_cmt_push_apss_forward(void)));
	connect(this, SIGNAL(sig_cmt_push_apss_rewind(void)),  hRunEmu, SLOT(do_cmt_push_apss_rewind(void)));
# endif
#endif
#if defined(USE_QD1)
	connect(this, SIGNAL(sig_write_protect_quickdisk(int, bool)), hRunEmu, SLOT(do_write_protect_quickdisk(int, bool)));
	connect(this, SIGNAL(sig_open_quickdisk(int, QString)), hRunEmu, SLOT(do_open_quickdisk(int, QString)));
	connect(this, SIGNAL(sig_close_quickdisk(int)), hRunEmu, SLOT(do_close_quickdisk(int)));
	connect(hRunEmu, SIGNAL(sig_change_osd_qd(int, QString)), this, SLOT(do_change_osd_qd(int, QString)));
#endif
#if defined(USE_CART1)
	connect(this, SIGNAL(sig_open_cart(int, QString)), hRunEmu, SLOT(do_open_cart(int, QString)));
	connect(this, SIGNAL(sig_close_cart(int)), hRunEmu, SLOT(do_close_cart(int)));
#endif
#if defined(USE_LASER_DISK)
	connect(this, SIGNAL(sig_open_laser_disk(QString)), hRunEmu, SLOT(do_open_laser_disk(QString)));
	connect(this, SIGNAL(sig_close_laser_disk(void)), hRunEmu, SLOT(do_close_laser_disk(void)));
#endif
#if defined(USE_BINARY_FILE1)
	connect(this, SIGNAL(sig_load_binary(int, QString)), hRunEmu, SLOT(do_load_binary(int, QString)));
	connect(this, SIGNAL(sig_save_binary(int, QString)), hRunEmu, SLOT(do_save_binary(int, QString)));
#endif
	
	connect(this, SIGNAL(quit_emu_thread()), hRunEmu, SLOT(doExit()));
	connect(hRunEmu, SIGNAL(sig_mouse_enable(bool)),
			this, SLOT(do_set_mouse_enable(bool)));
#ifdef USE_TAPE_BUTTON
	hRunEmu->set_tape_play(false);
	connect(hRunEmu, SIGNAL(sig_tape_play_stat(bool)), this, SLOT(do_display_tape_play(bool)));
#endif
#ifdef USE_DIG_RESOLUTION
	connect(hRunEmu, SIGNAL(sig_set_grid_vertical(int, bool)), graphicsView, SLOT(doSetGridsVertical(int, bool)));
	connect(hRunEmu, SIGNAL(sig_set_grid_horizonal(int, bool)), graphicsView, SLOT(doSetGridsHorizonal(int, bool)));
#endif	
#ifdef SUPPORT_DUMMY_DEVICE_LED
	connect(hRunEmu, SIGNAL(sig_send_data_led(quint32)), this, SLOT(do_recv_data_led(quint32)));
#endif
#ifdef USE_AUTO_KEY
	connect(this, SIGNAL(sig_start_auto_key(QString)), hRunEmu, SLOT(do_start_auto_key(QString)));
	connect(this, SIGNAL(sig_stop_auto_key()), hRunEmu, SLOT(do_stop_auto_key()));
#endif	
	//connect(actionExit_Emulator, SIGNAL(triggered()), hRunEmu, SLOT(doExit()));
	AGAR_DebugLog(AGAR_LOG_DEBUG, "EmuThread : Start.");
	objNameStr = QString("EmuThreadClass");
	hRunEmu->setObjectName(objNameStr);
	
	hDrawEmu = new DrawThreadClass(emu, this);
#ifdef USE_BITMAP
	QImageReader *reader = new QImageReader(":/background.png");
	QImage *result = new QImage(reader->read()); // this acts as a default if the size is not matched
	glv->updateBitmap(result);
	delete result;
	delete reader;
#endif   
	
	AGAR_DebugLog(AGAR_LOG_DEBUG, "DrawThread : Start.");
	connect(hDrawEmu, SIGNAL(sig_draw_frames(int)), hRunEmu, SLOT(print_framerate(int)));
	connect(hDrawEmu, SIGNAL(message_changed(QString)), this, SLOT(message_status_bar(QString)));
	connect(hDrawEmu, SIGNAL(sig_update_screen(QImage *)), glv, SLOT(update_screen(QImage *)), Qt::QueuedConnection);
	
	connect(hRunEmu, SIGNAL(sig_draw_thread()), hDrawEmu, SLOT(doDraw()));
	connect(hRunEmu, SIGNAL(quit_draw_thread()), hDrawEmu, SLOT(doExit()));
	
	connect(glv, SIGNAL(do_notify_move_mouse(int, int)),
			hRunEmu, SLOT(moved_mouse(int, int)));
	connect(glv, SIGNAL(do_notify_button_pressed(Qt::MouseButton)),
	        hRunEmu, SLOT(button_pressed_mouse(Qt::MouseButton)));
	connect(glv, SIGNAL(do_notify_button_released(Qt::MouseButton)),
			hRunEmu, SLOT(button_released_mouse(Qt::MouseButton)));
	connect(glv, SIGNAL(sig_toggle_mouse(void)),
			this, SLOT(do_toggle_mouse(void)));
	connect(glv, SIGNAL(sig_resize_uibar(int, int)),
			this, SLOT(resize_statusbar(int, int)));
	connect(hRunEmu, SIGNAL(sig_finished()),
			glv, SLOT(releaseKeyCode(void)));
	objNameStr = QString("EmuDrawThread");
	hDrawEmu->setObjectName(objNameStr);
	hDrawEmu->start();
	AGAR_DebugLog(AGAR_LOG_DEBUG, "DrawThread : Launch done.");
	
	hRunEmu->start();
	AGAR_DebugLog(AGAR_LOG_DEBUG, "EmuThread : Launch done.");
	this->set_screen_aspect(config.stretch_type);
}


void Ui_MainWindow::StopEmuThread(void)
{
	emit quit_emu_thread();
}

void Ui_MainWindow::delete_emu_thread(void)
{
	do_release_emu_resources();
	emit sig_quit_all();
}  

void Ui_MainWindow::LaunchJoyThread(void)
{
	hRunJoy = new JoyThreadClass(emu, this);
	//hRunJoy->SetEmu(emu);
	connect(this, SIGNAL(quit_joy_thread()), hRunJoy, SLOT(doExit()));
	hRunJoy->setObjectName("JoyThread");
	hRunJoy->start();
}
void Ui_MainWindow::StopJoyThread(void)
{
	emit quit_joy_thread();
}

void Ui_MainWindow::delete_joy_thread(void)
{
	//    delete hRunJoyThread;
	//  delete hRunJoy;
}


// Important Flags
AGAR_CPUID *pCpuID;

void Convert_CP932_to_UTF8(char *dst, char *src, int n_limit, int i_limit)
{
	QTextCodec *srcCodec = QTextCodec::codecForName( "SJIS" );
	QTextCodec *dstCodec = QTextCodec::codecForName( "UTF-8" );
	QString dst_b;
	QByteArray dst_s;
	if(src == NULL) {
		if(dst != NULL) dst[0] = '\0';
		return;
	}
	if(dst == NULL) return;
	dst_b = srcCodec->toUnicode(src, strlen(src));
	dst_s = dstCodec->fromUnicode(dst_b);
	if(n_limit > 0) {
		memset(dst, 0x00, n_limit);
		strncpy(dst, dst_s.constData(), n_limit - 1);
	}
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

void Ui_MainWindow::OnWindowRedraw(void)
{
	if(emu) {
		emu->update_screen();
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
	if(statusUpdateTimer != NULL) statusUpdateTimer->stop();
#ifdef SUPPORT_DUMMY_DEVICE_LED
	if(ledUpdateTimer != NULL) ledUpdateTimer->stop();
#endif
	emit quit_joy_thread();
	emit quit_emu_thread();
	if(hRunEmu != NULL) {
		hRunEmu->wait();
		delete hRunEmu;
	}
	if(hDrawEmu != NULL) {
		hDrawEmu->wait();
		AGAR_DebugLog(AGAR_LOG_DEBUG, "DrawThread : Exit.");
		delete hDrawEmu;
	}
	if(hRunJoy != NULL) {
		hRunJoy->wait();
		delete hRunJoy;
	}

	// release window
	if(now_fullscreen) {
		//ChangeDisplaySettings(NULL, 0);
	}
	now_fullscreen = false;
#ifdef USE_BUTTON
	//for(int i = 0; i < MAX_FONT_SIZE; i++) {
	//	if(hFont[i]) {
	//		DeleteObject(hFont[i]);
	//	}
	//}
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
	int w, h;
          
	cpp_homedir.copy(homedir, PATH_MAX - 1, 0);
	flag = FALSE;
	/*
	 * Into Qt's Loop.
	 */
#if defined(USE_SDL2)
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK );
#else
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
	//SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
#endif
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Audio and JOYSTICK subsystem was initialised.");
	GuiMain = new QApplication(argc, argv);

	load_config();
	
	rMainWindow = new META_MainWindow();
	rMainWindow->connect(rMainWindow, SIGNAL(sig_quit_all(void)), rMainWindow, SLOT(deleteLater(void)));
	rMainWindow->setCoreApplication(GuiMain);
	
	AGAR_DebugLog(AGAR_LOG_DEBUG, "InitInstance() OK.");
  
	
	// disenable ime
	
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
	GLDrawClass *pgl = rMainWindow->getGraphicsView();
	pgl->setEmuPtr(emu);
	pgl->setFixedSize(pgl->width(), pgl->height());
	
	rMainWindow->LaunchEmuThread();
	rMainWindow->LaunchJoyThread();
	QObject::connect(GuiMain, SIGNAL(lastWindowClosed()),
					 rMainWindow, SLOT(on_actionExit_triggered()));
	GuiMain->exec();
	return 0;
}
/*
 * This is main for Qt.
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
#if defined(Q_OS_WIN)
	delim = "\\";
#else
	delim = "/";
#endif
#if !defined(Q_OS_WIN)
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
#else
	cpp_homedir = ".";
#endif	
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
#if !defined(Q_OS_CYGWIN)	
	{
		QDir dir;
		dir.mkdir( QString::fromStdString(cpp_confdir));
	}
#endif   
   //AG_MkPath(cpp_confdir.c_str());
   /* Gettext */
#ifndef RSSDIR
#if defined(_USE_AGAR) || defined(_USE_QT)
	sRssDir = "/usr/local/share/";
#else
	sRssDir = "." + delim;
#endif
	sRssDir = sRssDir + "CommonSourceCodeProject" + delim + my_procname;
#else
	sRssDir = RSSDIR;
#endif
   
#if ((AGAR_VER <= 2) && defined(FMTV151))
	bFMTV151 = TRUE;
#endif				/*  */
/*
 * アプリケーション初期化
 */
	nErrorCode = MainLoop(argc, argv);
	return nErrorCode;
}
#if defined(Q_OS_WIN) 
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
   char *arg[1] = {""};
   main(0, arg);
}
#endif
