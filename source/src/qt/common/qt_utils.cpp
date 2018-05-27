/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -

	[ win32 main ] -> [ agar main ]
*/

#include <stdio.h>
#include <string>
#include <vector>
#include <QString>
#include <QTextCodec>
#include <QImage>
#include <QImageReader>
#include <QDateTime>
#include <QDir>
#include <QTranslator>
#include <QProcessEnvironment>

#include "common.h"
#include "fileio.h"
#include "emu.h"
#include "menuclasses.h"
#include "mainwidget.h"
#include "commonclasses.h"
#include "qt_main.h"
#include "emu_thread.h"
#include "joy_thread.h"
#include "draw_thread.h"

#include "qt_gldraw.h"
#include "../gui/gl2/qt_glutil_gl2_0.h"
#include "csp_logger.h"
#include "dock_disks.h"
#include "menu_disk.h"
#include "menu_bubble.h"
#include "menu_flags_ext.h"
#include "dialog_movie.h"
#include "../avio/movie_saver.h"
// emulation core

EMU* emu;
QApplication *GuiMain = NULL;

// Start to define MainWindow.
class META_MainWindow *rMainWindow;


// buttons
#ifdef MAX_BUTTONS
#define MAX_FONT_SIZE 32
#endif

// menu
extern std::string cpp_homedir;
extern DLL_PREFIX_I std::string cpp_confdir;
extern std::string my_procname;
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

void Ui_MainWindow::do_set_mouse_enable(bool flag)
{
#ifdef USE_MOUSE
	if(emu == NULL) return;
	emu->lock_vm();
	if(flag) {
		graphicsView->grabMouse();
		emu->enable_mouse();
	} else {
		graphicsView->releaseMouse();
		emu->disable_mouse();
	}
	emu->unlock_vm();
#endif	
}

void Ui_MainWindow::do_toggle_mouse(void)
{
#ifdef USE_MOUSE
	if(emu == NULL) return;
	emu->lock_vm();
	bool flag = emu->is_mouse_enabled();
	if(!flag) {
		graphicsView->grabMouse();
		emu->enable_mouse();
	} else {
		graphicsView->releaseMouse();
		emu->disable_mouse();
	}
	emu->unlock_vm();
#endif	
}

void Ui_MainWindow::rise_movie_dialog(void)
{
	CSP_DialogMovie *dlg = new CSP_DialogMovie(hSaveMovieThread, using_flags);
	dlg->setWindowTitle(QApplication::translate("CSP_DialogMovie", "Configure movie encodings", 0));
	dlg->show();
}
void Ui_MainWindow::LaunchEmuThread(void)
{
	QString objNameStr;
	GLDrawClass *glv = this->getGraphicsView();

	int drvs;
	
	hRunEmu = new EmuThreadClass(rMainWindow, using_flags);
	connect(hRunEmu, SIGNAL(message_changed(QString)), this, SLOT(message_status_bar(QString)));
	connect(hRunEmu, SIGNAL(sig_is_enable_mouse(bool)), glv, SLOT(do_set_mouse_enabled(bool)));
	connect(glv, SIGNAL(sig_key_down(uint32_t, uint32_t, bool)), hRunEmu, SLOT(do_key_down(uint32_t, uint32_t, bool)));
	connect(glv, SIGNAL(sig_key_up(uint32_t, uint32_t)),hRunEmu, SLOT(do_key_up(uint32_t, uint32_t)));
	connect(this, SIGNAL(sig_quit_widgets()), glv, SLOT(do_stop_run_vm()));

	
	//connect(hRunEmu, SIGNAL(sig_finished()), this, SLOT(delete_emu_thread()));
	connect(this, SIGNAL(sig_vm_reset()), hRunEmu, SLOT(do_reset()));
	connect(this, SIGNAL(sig_vm_specialreset()), hRunEmu, SLOT(do_special_reset()));

	connect(this, SIGNAL(sig_emu_update_config()), hRunEmu, SLOT(do_update_config()));
	connect(this, SIGNAL(sig_emu_update_volume_level(int, int)), hRunEmu, SLOT(do_update_volume_level(int, int)));
	connect(this, SIGNAL(sig_emu_update_volume_balance(int, int)), hRunEmu, SLOT(do_update_volume_balance(int, int)));
	connect(this, SIGNAL(sig_emu_start_rec_sound()), hRunEmu, SLOT(do_start_record_sound()));
	connect(this, SIGNAL(sig_emu_stop_rec_sound()), hRunEmu, SLOT(do_stop_record_sound()));
	connect(this, SIGNAL(sig_emu_set_display_size(int, int, int, int)), hRunEmu, SLOT(do_set_display_size(int, int, int, int)));
	
	if(using_flags->is_use_state()) {
		for(int i = 0; i < 10; i++) {
			connect(actionLoad_State[i], SIGNAL(sig_load_state(QString)), hRunEmu, SLOT(do_load_state(QString))); // OK?
			connect(actionSave_State[i], SIGNAL(sig_save_state(QString)), hRunEmu, SLOT(do_save_state(QString))); // OK?
		}
	}
#if defined(USE_FLOPPY_DISK)
	connect(this, SIGNAL(sig_write_protect_disk(int, bool)), hRunEmu, SLOT(do_write_protect_disk(int, bool)));
	connect(this, SIGNAL(sig_open_disk(int, QString, int)), hRunEmu, SLOT(do_open_disk(int, QString, int)));
	connect(this, SIGNAL(sig_close_disk(int)), hRunEmu, SLOT(do_close_disk(int)));
	connect(hRunEmu, SIGNAL(sig_update_recent_disk(int)), this, SLOT(do_update_recent_disk(int)));
	//connect(hRunEmu, SIGNAL(sig_change_osd_fd(int, QString)), this, SLOT(do_change_osd_fd(int, QString)));
	drvs = USE_FLOPPY_DISK;
	for(int ii = 0; ii < drvs; ii++) {
		menu_fds[ii]->setEmu(emu);
		connect(menu_fds[ii], SIGNAL(sig_update_inner_fd(int ,QStringList , class Action_Control **, QStringList , int, bool)),
				this, SLOT(do_update_inner_fd(int ,QStringList , class Action_Control **, QStringList , int, bool)));
	}
#endif
#if defined(USE_HARD_DISK)
	connect(this, SIGNAL(sig_open_disk(int, QString, int)), hRunEmu, SLOT(do_open_disk(int, QString, int)));
	connect(this, SIGNAL(sig_close_disk(int)), hRunEmu, SLOT(do_close_disk(int)));
	connect(hRunEmu, SIGNAL(sig_update_recent_disk(int)), this, SLOT(do_update_recent_disk(int)));
	//connect(hRunEmu, SIGNAL(sig_change_osd_fd(int, QString)), this, SLOT(do_change_osd_fd(int, QString)));
	drvs = USE_FHARD_DISK;
	for(int ii = 0; ii < drvs; ii++) {
		menu_fds[ii]->setEmu(emu);
		connect(menu_hdds[ii], SIGNAL(sig_update_inner_hdd(int ,QStringList , class Action_Control **, QStringList , int, bool)),
				this, SLOT(do_update_inner_hdd(int ,QStringList , class Action_Control **, QStringList , int, bool)));
	}
#endif
#if defined(USE_TAPE)
	connect(this, SIGNAL(sig_play_tape(int, QString)), hRunEmu, SLOT(do_play_tape(int, QString)));
	connect(this, SIGNAL(sig_rec_tape(int, QString)),  hRunEmu, SLOT(do_rec_tape(int, QString)));
	connect(this, SIGNAL(sig_close_tape(int)),   hRunEmu, SLOT(do_close_tape(int)));
	//connect(hRunEmu, SIGNAL(sig_change_osd_cmt(QString)), this, SLOT(do_change_osd_cmt(QString)));
# if defined(USE_TAPE_BUTTON)
	connect(this, SIGNAL(sig_cmt_push_play(int)), hRunEmu, SLOT(do_cmt_push_play(int)));
	connect(this, SIGNAL(sig_cmt_push_stop(int)), hRunEmu, SLOT(do_cmt_push_stop(int)));
	connect(this, SIGNAL(sig_cmt_push_fast_forward(int)), hRunEmu, SLOT(do_cmt_push_fast_forward(int)));
	connect(this, SIGNAL(sig_cmt_push_fast_rewind(int)),  hRunEmu, SLOT(do_cmt_push_fast_rewind(int)));
	connect(this, SIGNAL(sig_cmt_push_apss_forward(int)), hRunEmu, SLOT(do_cmt_push_apss_forward(int)));
	connect(this, SIGNAL(sig_cmt_push_apss_rewind(int)),  hRunEmu, SLOT(do_cmt_push_apss_rewind(int)));
# endif
#endif
#if defined(USE_QUICK_DISK)
	connect(this, SIGNAL(sig_write_protect_quickdisk(int, bool)), hRunEmu, SLOT(do_write_protect_quickdisk(int, bool)));
	connect(this, SIGNAL(sig_open_quickdisk(int, QString)), hRunEmu, SLOT(do_open_quickdisk(int, QString)));
	connect(this, SIGNAL(sig_close_quickdisk(int)), hRunEmu, SLOT(do_close_quickdisk(int)));
	//connect(hRunEmu, SIGNAL(sig_change_osd_qd(int, QString)), this, SLOT(do_change_osd_qd(int, QString)));
#endif
#if defined(USE_CART)
	connect(this, SIGNAL(sig_open_cart(int, QString)), hRunEmu, SLOT(do_open_cart(int, QString)));
	connect(this, SIGNAL(sig_close_cart(int)), hRunEmu, SLOT(do_close_cart(int)));
#endif
#if defined(USE_COMPACT_DISC)
	connect(this, SIGNAL(sig_open_cdrom(QString)), hRunEmu, SLOT(do_open_cdrom(QString)));
	connect(this, SIGNAL(sig_close_cdrom()), hRunEmu, SLOT(do_eject_cdrom()));
	//connect(hRunEmu, SIGNAL(sig_change_osd_cdrom(QString)), this, SLOT(do_change_osd_cdrom(QString)));
	// ToDo: multiple CDs
#endif	
#if defined(USE_LASER_DISC)
	connect(this, SIGNAL(sig_open_laserdisc(QString)), hRunEmu, SLOT(do_open_laser_disc(QString)));
	connect(this, SIGNAL(sig_close_laserdisc(void)), hRunEmu, SLOT(do_close_laser_disc(void)));
	// ToDo: multiple LDs
#endif
#if defined(USE_BINARY_FILE)
	connect(this, SIGNAL(sig_load_binary(int, QString)), hRunEmu, SLOT(do_load_binary(int, QString)));
	connect(this, SIGNAL(sig_save_binary(int, QString)), hRunEmu, SLOT(do_save_binary(int, QString)));
#endif
#if defined(USE_BUBBLE)
	connect(this, SIGNAL(sig_write_protect_bubble(int, bool)), hRunEmu, SLOT(do_write_protect_bubble_casette(int, bool)));
	connect(this, SIGNAL(sig_open_bubble(int, QString, int)), hRunEmu, SLOT(do_open_bubble_casette(int, QString, int)));
	connect(this, SIGNAL(sig_close_bubble(int)), hRunEmu, SLOT(do_close_bubble_casette(int)));
	connect(hRunEmu, SIGNAL(sig_update_recent_bubble(int)), this, SLOT(do_update_recent_bubble(int)));
	//connect(hRunEmu, SIGNAL(sig_change_osd_bubble(int, QString)), this, SLOT(do_change_osd_bubble(int, QString)));
	drvs = USE_BUBBLE;
	for(int ii = 0; ii < drvs; ii++) {
		menu_bubbles[ii]->setEmu(emu);
		connect(menu_bubbles[ii],
				SIGNAL(sig_update_inner_bubble(int ,QStringList , class Action_Control **, QStringList , int, bool)),
				this,
				SLOT(do_update_inner_bubble(int ,QStringList , class Action_Control **, QStringList , int, bool)));
	}
#endif
	
	connect(this, SIGNAL(quit_emu_thread()), hRunEmu, SLOT(doExit()));
	connect(hRunEmu, SIGNAL(sig_mouse_enable(bool)),
			this, SLOT(do_set_mouse_enable(bool)));

	
#ifdef USE_TAPE_BUTTON
	hRunEmu->set_tape_play(false);
#endif
#if defined(USE_KEY_LOCKED) || defined(USE_EXTRA_LEDS)
	connect(hRunEmu, SIGNAL(sig_send_data_led(quint32)), this, SLOT(do_recv_data_led(quint32)));
#endif
#ifdef USE_AUTO_KEY
	connect(this, SIGNAL(sig_start_auto_key(QString)), hRunEmu, SLOT(do_start_auto_key(QString)));
	connect(this, SIGNAL(sig_stop_auto_key()), hRunEmu, SLOT(do_stop_auto_key()));
	connect(this, SIGNAL(sig_set_roma_kana(bool)), hRunEmu, SLOT(set_romakana(bool)));
#endif	
	//connect(actionExit_Emulator, SIGNAL(triggered()), hRunEmu, SLOT(doExit()));
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "EmuThread : Start.");
	objNameStr = QString("EmuThreadClass");
	hRunEmu->setObjectName(objNameStr);
	
	hDrawEmu = new DrawThreadClass(emu->get_osd(), csp_logger, this);
	emu->set_parent_handler(hRunEmu, hDrawEmu);
	
#ifdef ONE_BOARD_MICRO_COMPUTER
	QImageReader *reader = new QImageReader(":/background.png");
	QImage *result = new QImage(reader->read()); // this acts as a default if the size is not matched
	QImage result2 = result->convertToFormat(QImage::Format_ARGB32);
	glv->updateBitmap(&result2);
	emu->get_osd()->upload_bitmap(&result2);
	delete result;
	delete reader;
	emu->get_osd()->set_buttons();
#endif
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "DrawThread : Start.");
	connect(hDrawEmu, SIGNAL(sig_draw_frames(int)), hRunEmu, SLOT(print_framerate(int)));
	//connect(emu->get_osd(), SIGNAL(sig_draw_frames(int)), hRunEmu, SLOT(print_framerate(int)));
	connect(hRunEmu, SIGNAL(window_title_changed(QString)), this, SLOT(do_set_window_title(QString)));
	connect(hDrawEmu, SIGNAL(message_changed(QString)), this, SLOT(message_status_bar(QString)));
	connect(actionCapture_Screen, SIGNAL(triggered()), glv, SLOT(do_save_frame_screen()));

	/*if(using_flags->get_config_ptr()->use_separate_thread_draw) {
		connect(hRunEmu, SIGNAL(sig_draw_thread(bool)), hDrawEmu, SLOT(doDraw(bool)), Qt::QueuedConnection);
		connect(hRunEmu, SIGNAL(sig_set_draw_fps(double)), hDrawEmu, SLOT(do_set_frames_per_second(double)), Qt::QueuedConnection);
		connect(hRunEmu, SIGNAL(sig_draw_one_turn(bool)), hDrawEmu, SLOT(do_draw_one_turn(bool)), Qt::QueuedConnection);
		} else*/ {
		connect(hRunEmu, SIGNAL(sig_draw_thread(bool)), hDrawEmu, SLOT(doDraw(bool)));
		connect(hRunEmu, SIGNAL(sig_set_draw_fps(double)), hDrawEmu, SLOT(do_set_frames_per_second(double)));
		connect(hRunEmu, SIGNAL(sig_draw_one_turn(bool)), hDrawEmu, SLOT(do_draw_one_turn(bool)));
	}
	//connect(hRunEmu, SIGNAL(sig_draw_thread(bool)), emu->get_osd(), SLOT(do_draw(bool)));
	//connect(hRunEmu, SIGNAL(quit_draw_thread()), hDrawEmu, SLOT(doExit()));
	connect(this, SIGNAL(quit_draw_thread()), hDrawEmu, SLOT(doExit()));

	connect(glv, SIGNAL(do_notify_move_mouse(int, int)),
			hRunEmu, SLOT(moved_mouse(int, int)));
	connect(glv, SIGNAL(do_notify_button_pressed(Qt::MouseButton)),
	        hRunEmu, SLOT(button_pressed_mouse(Qt::MouseButton)));
	connect(glv, SIGNAL(do_notify_button_released(Qt::MouseButton)),
			hRunEmu, SLOT(button_released_mouse(Qt::MouseButton)));
#ifdef USE_MOUSE
	connect(glv, SIGNAL(sig_toggle_mouse(void)),
			this, SLOT(do_toggle_mouse(void)));
#endif
	connect(hRunEmu, SIGNAL(sig_resize_screen(int, int)),
			glv, SLOT(resizeGL(int, int)));
	connect(hRunEmu, SIGNAL(sig_resize_osd(int)), driveData, SLOT(setScreenWidth(int)));
	
	connect(glv, SIGNAL(sig_resize_uibar(int, int)),
			this, SLOT(resize_statusbar(int, int)));
	connect(hRunEmu, SIGNAL(sig_resize_uibar(int, int)),
			this, SLOT(resize_statusbar(int, int)));

	connect(emu->get_osd(), SIGNAL(sig_req_encueue_video(int, int, int)),
			hDrawEmu, SLOT(do_req_encueue_video(int, int, int)));
	connect(hRunEmu, SIGNAL(sig_finished()), glv, SLOT(releaseKeyCode(void)));
	connect(hRunEmu, SIGNAL(sig_finished()), this, SLOT(delete_emu_thread()));
	objNameStr = QString("EmuDrawThread");
	hDrawEmu->setObjectName(objNameStr);

	if(using_flags->get_config_ptr()->use_separate_thread_draw) hDrawEmu->start();

	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "DrawThread : Launch done.");

	hSaveMovieThread = new MOVIE_SAVER(640, 400,  30, emu->get_osd(), using_flags->get_config_ptr());
	
	connect(actionStart_Record_Movie->binds, SIGNAL(sig_start_record_movie(int)), hRunEmu, SLOT(do_start_record_video(int)));
	connect(this, SIGNAL(sig_start_saving_movie()),
			actionStart_Record_Movie->binds, SLOT(do_save_as_movie()));
	connect(actionStart_Record_Movie, SIGNAL(triggered()), this, SLOT(do_start_saving_movie()));

	connect(actionStop_Record_Movie->binds, SIGNAL(sig_stop_record_movie()), hRunEmu, SLOT(do_stop_record_video()));
	connect(this, SIGNAL(sig_stop_saving_movie()), actionStop_Record_Movie->binds, SLOT(do_stop_saving_movie()));
	connect(hSaveMovieThread, SIGNAL(sig_set_state_saving_movie(bool)), this, SLOT(do_set_state_saving_movie(bool)));
	connect(actionStop_Record_Movie, SIGNAL(triggered()), this, SLOT(do_stop_saving_movie()));

	connect(emu->get_osd(), SIGNAL(sig_save_as_movie(QString, int, int)),
			hSaveMovieThread, SLOT(do_open(QString, int, int)));
	connect(emu->get_osd(), SIGNAL(sig_stop_saving_movie()), hSaveMovieThread, SLOT(do_close()));
	
	actionStop_Record_Movie->setIcon(QIcon(":/icon_process_stop.png"));
	actionStop_Record_Movie->setVisible(false);
	
	connect(this, SIGNAL(sig_movie_set_width(int)), hSaveMovieThread, SLOT(do_set_width(int)));
	connect(this, SIGNAL(sig_movie_set_height(int)), hSaveMovieThread, SLOT(do_set_height(int)));
 
	connect(emu->get_osd(), SIGNAL(sig_movie_set_width(int)), hSaveMovieThread, SLOT(do_set_width(int)));
	connect(emu->get_osd(), SIGNAL(sig_movie_set_height(int)), hSaveMovieThread, SLOT(do_set_height(int)));
   
	connect(emu->get_osd(), SIGNAL(sig_enqueue_audio(int16_t*, int)), hSaveMovieThread, SLOT(enqueue_audio(int16_t *, int)));
	connect(emu->get_osd(), SIGNAL(sig_enqueue_video(int, int, int, QImage *)),
			hSaveMovieThread, SLOT(enqueue_video(int, int, int, QImage *)), Qt::DirectConnection);
	connect(glv->extfunc, SIGNAL(sig_push_image_to_movie(int, int, int, QImage *)),
			hSaveMovieThread, SLOT(enqueue_video(int, int, int, QImage *)));
	connect(this, SIGNAL(sig_quit_movie_thread()), hSaveMovieThread, SLOT(do_exit()));

	objNameStr = QString("EmuMovieThread");
	hSaveMovieThread->setObjectName(objNameStr);
	hSaveMovieThread->start();
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "MovieThread : Launch done.");

	connect(action_SetupMovie, SIGNAL(triggered()), this, SLOT(rise_movie_dialog()));
	connect(hRunEmu, SIGNAL(sig_change_osd(int, int, QString)), driveData, SLOT(updateMessage(int, int, QString)));
	connect(hRunEmu, SIGNAL(sig_change_access_lamp(int, int, QString)), driveData, SLOT(updateLabel(int, int, QString)));
	connect(hRunEmu, SIGNAL(sig_set_access_lamp(int, bool)), graphicsView, SLOT(do_display_osd_leds(int, bool)));

	hRunEmu->start();
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "EmuThread : Launch done.");
	this->set_screen_aspect(using_flags->get_config_ptr()->window_stretch_type);
	emit sig_movie_set_width(SCREEN_WIDTH);
	emit sig_movie_set_height(SCREEN_HEIGHT);
}

void Ui_MainWindow::LaunchJoyThread(void)
{
#if defined(USE_JOYSTICK)
	hRunJoy = new JoyThreadClass(emu, emu->get_osd(), using_flags, using_flags->get_config_ptr(), csp_logger);
	connect(this, SIGNAL(quit_joy_thread()), hRunJoy, SLOT(doExit()));
	hRunJoy->setObjectName("JoyThread");
	hRunJoy->start();
#endif	
}

void Ui_MainWindow::StopJoyThread(void)
{
#if defined(USE_JOYSTICK)
	emit quit_joy_thread();
#endif	
}

void Ui_MainWindow::delete_joy_thread(void)
{
	//    delete hRunJoyThread;
	//  delete hRunJoy;
}

void Ui_MainWindow::on_actionExit_triggered()
{
	OnMainWindowClosed();
}

void Ui_MainWindow::OnWindowRedraw(void)
{
	if(emu) {
		//emu->update_screen();
	}
}

void Ui_MainWindow::OnWindowMove(void)
{
	if(emu) {
		emu->suspend();
	}
}


#ifdef USE_NOTIFY_POWER_OFF
bool Ui_MainWindow::GetPowerState(void)
{
	if(close_notified == 0) return true;
	return false;
}
#endif

#include <string>

void Ui_MainWindow::OnMainWindowClosed(void)
{
	// notify power off
#ifdef USE_NOTIFY_POWER_OFF
	if(emu) {
		if(!close_notified) {
			emu->lock_vm();
			emu->notify_power_off();
			emu->unlock_vm();
			close_notified = 1;
			return; 
		}
	}
#endif
	if(statusUpdateTimer != NULL) statusUpdateTimer->stop();
#if defined(USE_KEY_LOCKED) || defined(USE_EXTRA_LEDS)
	if(ledUpdateTimer != NULL) ledUpdateTimer->stop();
#endif
	emit quit_draw_thread();
	emit quit_joy_thread();
	emit quit_emu_thread();
	emit sig_quit_movie_thread();
	emit sig_quit_widgets();
	
	if(hSaveMovieThread != NULL) {
		hSaveMovieThread->wait();
		delete hSaveMovieThread;
		hSaveMovieThread = NULL;
	}
   
	if(hDrawEmu != NULL) {
		hDrawEmu->wait();
		delete hDrawEmu;
		hDrawEmu = NULL;
	}
	if(hRunEmu != NULL) {
		OnCloseDebugger();
		hRunEmu->quit();
		hRunEmu->wait();
		delete hRunEmu;
#if 0
		save_config(create_local_path(_T("%s.ini"), _T(CONFIG_NAME)));
#else
		{
			char tmps[128];
			std::string localstr;
			snprintf(tmps, sizeof(tmps), _T("%s.ini"), _T(CONFIG_NAME));
			localstr = tmps;
			localstr = cpp_confdir + localstr;
			save_config(localstr.c_str());
		}
#endif
	}
#if defined(USE_JOYSTICK)
	if(hRunJoy != NULL) {
		hRunJoy->wait();
		delete hRunJoy;
		hRunJoy = NULL;
	}
#endif
	do_release_emu_resources();
	hRunEmu = NULL;

	// release window
	if(now_fullscreen) {
		//ChangeDisplaySettings(NULL, 0);
	}
	now_fullscreen = false;
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

extern void DLL_PREFIX_I get_long_full_path_name(_TCHAR* src, _TCHAR* dst);
extern _TCHAR* DLL_PREFIX_I get_parent_dir(_TCHAR* file);
extern void get_short_filename(_TCHAR *dst, _TCHAR *file, int maxlen);

#if defined(Q_OS_CYGWIN)	
#include <sys/stat.h>
#endif
static void my_util_mkdir(std::string n)
{
#if !defined(Q_OS_CYGWIN)	
		QDir dir = QDir::current();
		dir.mkdir( QString::fromStdString(n));
#else
		struct stat st;
		if(stat(n.c_str(), &st) != 0) {
			_mkdir(n.c_str()); // Not found
		}
#endif   
}	

static void setup_logs(void)
{
	std::string delim;
	char    *p;

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

#if !defined(CSP_OS_WINDOWS)
	cpp_confdir = cpp_homedir + ".config" + delim;
	my_util_mkdir(cpp_confdir);
#else
	cpp_confdir = cpp_homedir;
#endif

	cpp_confdir = cpp_confdir + "CommonSourceCodeProject" + delim;
	my_util_mkdir(cpp_confdir);
	
	cpp_confdir = cpp_confdir + my_procname + delim;
	my_util_mkdir(cpp_confdir);
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
}

#include <QString>
#include <QCommandLineParser>
CSP_Logger *csp_logger;
QStringList virtualMediaList; // {TYPE, POSITION}
QCommandLineOption *_opt_fds[8];
QCommandLineOption *_opt_hdds[8];
QCommandLineOption *_opt_cmts[2];
QCommandLineOption *_opt_lds[2];
QCommandLineOption *_opt_cds[2];
QCommandLineOption *_opt_binaries[8];
QCommandLineOption *_opt_bubbles[8];
QCommandLineOption *_opt_qds[8];
QCommandLineOption *_opt_carts[8];
QCommandLineOption *_opt_homedir;
QCommandLineOption *_opt_cfgfile;
QCommandLineOption *_opt_cfgdir;
QCommandLineOption *_opt_resdir;
QCommandLineOption *_opt_opengl;
QCommandLineOption *_opt_envver;
QCommandLineOption *_opt_dump_envver;
QCommandLineOption *_opt_dipsw_on;
QCommandLineOption *_opt_dipsw_off;
QProcessEnvironment _envvers;
bool _b_dump_envver;
std::string config_fullpath;

void SetFDOptions(QCommandLineParser *cmdparser)
{
#if defined(USE_FLOPPY_DISK)
	for(int i = 0; i < USE_FLOPPY_DISK; i++) {
		QString sfdType1 = QString::fromUtf8("fd%1").arg(i);
		QString sfdType2 = QString::fromUtf8("vFd%1").arg(i);
		QString sfdType3 = QString::fromUtf8("vFloppyDisk%1").arg(i);
		QStringList _cl;
	 	_cl.append(sfdType1);
	 	_cl.append(sfdType2);
	 	_cl.append(sfdType3);
		_opt_fds[i] = new QCommandLineOption(_cl,QCoreApplication::translate("main", "Set virtual floppy disk %1.").arg(i) , "[D88_SLOT@]fullpath");
		cmdparser->addOption(*_opt_fds[i]);
		_cl.clear();
	}
#endif
}

void SetHDDOptions(QCommandLineParser *cmdparser)
{
#if defined(USE_HARD_DISK)
	for(int i = 0; i < USE_HARD_DISK; i++) {
		QString sfdType1 = QString::fromUtf8("hd%1").arg(i);
		QString sfdType2 = QString::fromUtf8("vHd%1").arg(i);
		QString sfdType3 = QString::fromUtf8("vHardDisk%1").arg(i);
		QString sfdType4 = QString::fromUtf8("vHardDrive%1").arg(i);
		QStringList _cl;
	 	_cl.append(sfdType1);
	 	_cl.append(sfdType2);
	 	_cl.append(sfdType3);
	 	_cl.append(sfdType4);
		_opt_hdds[i] = new QCommandLineOption(_cl,QCoreApplication::translate("main", "Set virtual hard drive %1.").arg(i) , "[D88_SLOT@]fullpath");
		cmdparser->addOption(*_opt_hdds[i]);
		_cl.clear();
	}
#endif
}

void SetBinaryOptions(QCommandLineParser *cmdparser)
{
#if defined(USE_BINARY_FILE)
	for(int i = 0; i < USE_BINARY_FILE; i++) {
		QString sfdType1 = QString::fromUtf8("bin%1").arg(i);
		QString sfdType2 = QString::fromUtf8("vBinary%1").arg(i);
		QStringList _cl;
	 	_cl.append(sfdType1);
	 	_cl.append(sfdType2);
		_opt_binaries[i] = new QCommandLineOption(_cl,QCoreApplication::translate("main", "Set virtual binary image %1.").arg(i) , "fullpath");
		cmdparser->addOption(*_opt_binaries[i]);
		_cl.clear();
	}
#endif
}

void SetCartOptions(QCommandLineParser *cmdparser)
{
#if defined(USE_CART)
	for(int i = 0; i < USE_CART; i++) {
		QString sfdType1 = QString::fromUtf8("cart%1").arg(i);
		QString sfdType2 = QString::fromUtf8("vCart%1").arg(i);
		QString sfdType3 = QString::fromUtf8("vCartridge%1").arg(i);
		QStringList _cl;
	 	_cl.append(sfdType1);
	 	_cl.append(sfdType2);
	 	_cl.append(sfdType3);
		_opt_carts[i] = new QCommandLineOption(_cl,QCoreApplication::translate("main", "Set virtual cartridge %1 (mostly ROM).").arg(i) , "fullpath");
		cmdparser->addOption(*_opt_carts[i]);
		_cl.clear();
	}
#endif
}
void SetBubbleOptions(QCommandLineParser *cmdparser)
{
#if defined(USE_BUBBLE)
	for(int i = 0; i < USE_BUBBLE; i++) {
		QString sfdType1 = QString::fromUtf8("bub%1").arg(i);
		QString sfdType2 = QString::fromUtf8("vBubble%1").arg(i);
		QStringList _cl;
	 	_cl.append(sfdType1);
	 	_cl.append(sfdType2);
		_opt_bubbles[i] = new QCommandLineOption(_cl,QCoreApplication::translate("main", "Set virtual bubble cartridge %1.").arg(i) , "[B88_SLOT@]fullpath");
		cmdparser->addOption(*_opt_bubbles[i]);
	}
#endif
}

void SetLDOptions(QCommandLineParser *cmdparser)
{
#if defined(USE_LASER_DISC)
	for(int i = 0; i < USE_LASER_DISC; i++) {
		QString sfdType1 = QString::fromUtf8("ld%1").arg(i);
		QString sfdType2 = QString::fromUtf8("vLaserDisc%1").arg(i);
		QStringList _cl;
	 	_cl.append(sfdType1);
	 	_cl.append(sfdType2);
		_opt_lds[i] = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Set virtual laser disc %1 (mostly movie file).").arg(i) , "fullpath");
		cmdparser->addOption(*_opt_lds[i]);
		_cl.clear();
	}
#endif
}
void SetCDOptions(QCommandLineParser *cmdparser)
{
#if defined(USE_COMPACT_DISC)
	for(int i = 0; i < USE_COMPACT_DISC; i++) {
		QString sfdType1 = QString::fromUtf8("cd%1").arg(i);
		QString sfdType2 = QString::fromUtf8("vCompactDisc%1").arg(i);
   		QStringList _cl;
	 	_cl.append(sfdType1);
	 	_cl.append(sfdType2);
		_opt_cds[i] = new QCommandLineOption(_cl,QCoreApplication::translate("main", "Set virtual compact disc %1.").arg(i) , "fullpath");
		cmdparser->addOption(*_opt_cds[i]);
		_cl.clear();
	}
#endif
}

void SetCmtOptions(QCommandLineParser *cmdparser)
{
#if defined(USE_TAPE)
	for(int i = 0; i < USE_TAPE; i++) {
		QString sfdType1 = QString::fromUtf8("cmt%1").arg(i);
		QString sfdType2 = QString::fromUtf8("tape%1").arg(i);
		QString sfdType3 = QString::fromUtf8("vCmt%1").arg(i);
		QString sfdType4 = QString::fromUtf8("vTape%1").arg(i);
		QStringList _cl;
	 	_cl.append(sfdType1);
	 	_cl.append(sfdType2);
	 	_cl.append(sfdType3);
	 	_cl.append(sfdType4);
		_opt_cmts[i] = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Set virtual casette tape %1.").arg(i) , "fullpath");
		cmdparser->addOption(*_opt_cmts[i]);
		_cl.clear();
	}
#endif
}

void SetQuickDiskOptions(QCommandLineParser *cmdparser)
{
#if defined(USE_QUICK_DISK)
	for(int i = 0; i < USE_QUICK_DISK; i++) {
		QString sfdType1 = QString::fromUtf8("qd%1").arg(i);
		QString sfdType2 = QString::fromUtf8("vQuickDisk%1").arg(i);

		QStringList _cl;
	 	_cl.append(sfdType1);
	 	_cl.append(sfdType2);
		_opt_qds[i] = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Set virtual quick disk %1.").arg(i) , "fullpath");
		cmdparser->addOption(*_opt_qds[i]);
		_cl.clear();
	}
#endif
}


void SetProcCmdFD(QCommandLineParser *cmdparser, QStringList *_l)
{
#if defined(USE_FLOPPY_DISK)
	for(int i = 0; i < USE_FLOPPY_DISK; i++) {
		if(_opt_fds[i] != NULL) {
			if(cmdparser->isSet(*_opt_fds[i])) {
				QString sfdType = QString::fromUtf8("vFloppyDisk%1").arg(i);
				QString medianame = cmdparser->value(*_opt_fds[i]);
				_l->append(sfdType);
				_l->append(medianame);
			}
		}
	}
#endif
}

void SetProcCmdHDD(QCommandLineParser *cmdparser, QStringList *_l)
{
#if defined(USE_HARD_DISK)
	for(int i = 0; i < USE_HARD_DISK; i++) {
		if(_opt_hdds[i] != NULL) {
			if(cmdparser->isSet(*_opt_hdds[i])) {
				QString sfdType = QString::fromUtf8("vHardDisk%1").arg(i);
				QString medianame = cmdparser->value(*_opt_hdds[i]);
				_l->append(sfdType);
				_l->append(medianame);
			}
		}
	}
#endif
}

void SetProcCmdQuickDisk(QCommandLineParser *cmdparser, QStringList *_l)
{
#if defined(USE_QUICK_DISK)
	for(int i = 0; i < USE_QUICK_DISK; i++) {
		if(_opt_qds[i] != NULL) {
			if(cmdparser->isSet(*_opt_qds[i])) {
				QString sfdType = QString::fromUtf8("vQuickDisk%1").arg(i);
				QString medianame = cmdparser->value(*_opt_qds[i]);
				_l->append(sfdType);
				_l->append(medianame);
			}
		}
	}
#endif
}

void SetProcCmdCmt(QCommandLineParser *cmdparser, QStringList *_l)
{
#if defined(USE_TAPE)
	for(int i = 0; i < USE_TAPE; i++) {
		if(_opt_cmts[i] != NULL) {
			if(cmdparser->isSet(*_opt_cmts[i])) {
				QString sfdType = QString::fromUtf8("vCmt%1").arg(i);
				QString medianame = cmdparser->value(*_opt_cmts[i]);
				_l->append(sfdType);
				_l->append(medianame);
			}
		}
	}
#endif
}

void SetProcCmdBinary(QCommandLineParser *cmdparser, QStringList *_l)
{
#if defined(USE_BINARY_FILE)
	for(int i = 0; i < USE_BINARY_FILE; i++) {
		if(_opt_binaries[i] != NULL) {
			if(cmdparser->isSet(*_opt_binaries[i])) {
				QString sfdType = QString::fromUtf8("vBinary%1").arg(i);
				QString medianame = cmdparser->value(*_opt_binaries[i]);
				_l->append(sfdType);
				_l->append(medianame);
			}
		}
	}
#endif
}

void SetProcCmdBubble(QCommandLineParser *cmdparser, QStringList *_l)
{
#if defined(USE_BUBBLE)
	for(int i = 0; i < USE_BUBBLE; i++) {
		if(_opt_bubbles[i] != NULL) {
			if(cmdparser->isSet(*_opt_bubbles[i])) {
				QString sfdType = QString::fromUtf8("vBubble%1").arg(i);
				QString medianame = cmdparser->value(*_opt_bubbles[i]);
				_l->append(sfdType);
				_l->append(medianame);
			}
		}
	}
#endif
}

void SetProcCmdCart(QCommandLineParser *cmdparser, QStringList *_l)
{
#if defined(USE_CART)
	for(int i = 0; i < USE_CART; i++) {
		if(_opt_carts[i] != NULL) {
			if(cmdparser->isSet(*_opt_carts[i])) {
				QString sfdType = QString::fromUtf8("vCart%1").arg(i);
				QString medianame = cmdparser->value(*_opt_carts[i]);
				_l->append(sfdType);
				_l->append(medianame);
			}
		}
	}
#endif
}

void SetProcCmdLD(QCommandLineParser *cmdparser, QStringList *_l)
{
#if defined(USE_LASER_DISC)
	for(int i = 0; i < USE_LASER_DISC; i++) {
		if(_opt_lds[i] != NULL) {
			if(cmdparser->isSet(*_opt_lds[i])) {
				QString sfdType = QString::fromUtf8("vLD%1").arg(i);
				QString medianame = cmdparser->value(*_opt_lds[i]);
				_l->append(sfdType);
				_l->append(medianame);
			}
		}
	}
#endif
}

void SetProcCmdCD(QCommandLineParser *cmdparser, QStringList *_l)
{
#if defined(USE_COMPACT_DISC)
	for(int i = 0; i < USE_COMPACT_DISC; i++) {
		if(_opt_cds[i] != NULL) {
			if(cmdparser->isSet(*_opt_cds[i])) {
				QString sfdType = QString::fromUtf8("vCD%1").arg(i);
				QString medianame = cmdparser->value(*_opt_cds[i]);
				_l->append(sfdType);
				_l->append(medianame);
			}
		}
	}
#endif
}


void SetOptions(QCommandLineParser *cmdparser)
{
	QString emudesc = QString::fromUtf8("Emulator for ");
	emudesc = emudesc + QString::fromUtf8(DEVICE_NAME);
    cmdparser->setApplicationDescription(emudesc);
    cmdparser->addHelpOption();
    //cmdparser->addVersionOption();
	QStringList _cl;
    _cl.append("d");
    _cl.append("homedir");
    _opt_homedir = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Custom home directory."), "homedir");
    _cl.clear();
   
    _cl.append("c");
    _cl.append("cfgfile");
    _opt_cfgfile = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Custom config file (without path)."), "cfgfile");
    _cl.clear();
   
    _opt_cfgdir = new QCommandLineOption("cfgdir", QCoreApplication::translate("main", "Custom config directory."), "cfgdir");

    _cl.append("r");
    _cl.append("res");
    _opt_resdir = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Custom resource directory (ROMs, WAVs, etc)."), "resdir");
    _cl.clear();

    _cl.append("on");
    _cl.append("dipsw-on");
    _opt_dipsw_on = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Turn on <onbit> of dip switch."), "onbit");
    _cl.clear();
   
    _cl.append("off");
    _cl.append("dipsw-off");
    _opt_dipsw_off = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Turn off <offbit> of dip switch."), "offbit");
    _cl.clear();
	
    _cl.append("g");
    _cl.append("gl");
    _cl.append("opengl");
    _cl.append("render");
    _opt_opengl = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Force set using renderer type."), "{ GL | GL2 | GLES}");
    _cl.clear();
	
    _cl.append("v");
    _cl.append("env");
    _cl.append("envver");
    _opt_envver = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Set / Delete environment variable."), "{NAME[=VAL] | -NAME}");
    _cl.clear();

    _cl.append("dump-env");
    _cl.append("dump-envver");
    _opt_dump_envver = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Dump environment variables."), "");
    _cl.clear();
	
	for(int i = 0; i < 8; i++) {
		_opt_fds[i] = NULL;
		_opt_hdds[i] = NULL;
		_opt_qds[i] = NULL;
		_opt_bubbles[i] = NULL;
		_opt_binaries[i] = NULL;
		_opt_carts[i] = NULL;
	}
	for(int i = 0; i < 2; i++) {
		_opt_cmts[i] = NULL;
		_opt_lds[i] = NULL;
		_opt_cds[i] = NULL;
	}		
		
    cmdparser->addOption(*_opt_opengl);
    cmdparser->addOption(*_opt_homedir);
    cmdparser->addOption(*_opt_cfgfile);
    cmdparser->addOption(*_opt_cfgdir);
    cmdparser->addOption(*_opt_resdir);
    cmdparser->addOption(*_opt_dipsw_on);
    cmdparser->addOption(*_opt_dipsw_off);

	SetFDOptions(cmdparser);
	SetHDDOptions(cmdparser);
	//SetBinaryOptions(cmdparser); // Temporally disabled.
	SetCmtOptions(cmdparser);
	SetCartOptions(cmdparser);
	SetBubbleOptions(cmdparser); // Temporally disabled.
	SetQuickDiskOptions(cmdparser);
	SetLDOptions(cmdparser); // Temporally disabled.
	SetCDOptions(cmdparser);
	
    cmdparser->addOption(*_opt_envver);
    cmdparser->addOption(*_opt_dump_envver);
}

void ProcessCmdLine(QCommandLineParser *cmdparser, QStringList *_l)
{
	char homedir[PATH_MAX];
	std::string delim;
#if defined(Q_OS_WIN)
	delim = "\\";
#else
	delim = "/";
#endif
	
	SetProcCmdFD(cmdparser, _l);
	SetProcCmdHDD(cmdparser, _l);
	SetProcCmdQuickDisk(cmdparser, _l);
	SetProcCmdCmt(cmdparser, _l);
	SetProcCmdCart(cmdparser, _l);
	SetProcCmdBinary(cmdparser, _l);
	SetProcCmdBubble(cmdparser, _l);
	SetProcCmdLD(cmdparser, _l);
	SetProcCmdCD(cmdparser, _l);

	memset(homedir, 0x00, PATH_MAX);
	if(cmdparser->isSet(*_opt_homedir)) {
		strncpy(homedir, cmdparser->value(*_opt_homedir).toLocal8Bit().constData(), PATH_MAX - 1);
		cpp_homedir = homedir;
		size_t _len = cpp_homedir.length() - 1;
		size_t _pos = cpp_homedir.rfind(delim);
		if((_pos < _len) ||
		   (_pos == std::string::npos)) {
			cpp_homedir.append(delim);
		}
	} else {
		cpp_homedir.copy(homedir, PATH_MAX - 1, 0);
	}

	if(cmdparser->isSet(*_opt_cfgdir)) {
		char tmps[PATH_MAX];
		std::string tmpstr;
		memset(tmps, 0x00, PATH_MAX);
		strncpy(tmps, cmdparser->value(*_opt_cfgdir).toLocal8Bit().constData(), PATH_MAX - 1);
		cpp_confdir = tmps;
		size_t _len = cpp_confdir.length() - 1;
		size_t _pos = cpp_confdir.rfind(delim);
		if((_pos < _len) ||
		   (_pos == std::string::npos)) {
			cpp_confdir.append(delim);
		}
	}
	
	{
		char tmps[128];
		std::string localstr;
		memset(tmps, 0x00, 128);
		if(cmdparser->isSet(*_opt_cfgfile)) {
			strncpy(tmps, cmdparser->value(*_opt_cfgfile).toLocal8Bit().constData(), 127);
		}
		if(strlen(tmps) <= 0){
			snprintf(tmps, sizeof(tmps), _T("%s.ini"), _T(CONFIG_NAME));
		}
		localstr = tmps;
		localstr = cpp_confdir + localstr;
		load_config(localstr.c_str());
		config_fullpath = localstr;
	}
	if(cmdparser->isSet(*_opt_resdir)) {
		char tmps[PATH_MAX];
		std::string tmpstr;
		memset(tmps, 0x00, PATH_MAX);
		strncpy(tmps, cmdparser->value(*_opt_resdir).toLocal8Bit().constData(), PATH_MAX - 1);
		sRssDir = tmps;
		size_t _len = sRssDir.length() - 1;
		size_t _pos = sRssDir.rfind(delim);
		if((_pos < _len) ||
		   (_pos == std::string::npos)) {
			sRssDir.append(delim);
		}
	}
	if(cmdparser->isSet(*_opt_opengl)) {
		char tmps[128] = {0};
		strncpy(tmps, cmdparser->value(*_opt_opengl).toLocal8Bit().constData(), 128 - 1);
		if(strlen(tmps) > 0) {
			QString render = QString::fromLocal8Bit(tmps).toUpper();
			if((render == QString::fromUtf8("GL2")) ||
			   (render == QString::fromUtf8("GLV2"))) {
				config.render_platform = CONFIG_RENDER_PLATFORM_OPENGL_MAIN;
				config.render_major_version = 2;
				config.render_minor_version = 0;
				GuiMain->setAttribute(Qt::AA_UseDesktopOpenGL, true);
				GuiMain->setAttribute(Qt::AA_UseOpenGLES, false);
			} else if((render == QString::fromUtf8("GL3")) ||
					  (render == QString::fromUtf8("GLV3")) ||
					  (render == QString::fromUtf8("OPENGLV3")) ||
					  (render == QString::fromUtf8("OPENGL")) ||
					  (render == QString::fromUtf8("GL"))) {
				config.render_platform = CONFIG_RENDER_PLATFORM_OPENGL_MAIN;
				config.render_major_version = 3;
				config.render_minor_version = 0;
				GuiMain->setAttribute(Qt::AA_UseDesktopOpenGL, true);
				GuiMain->setAttribute(Qt::AA_UseOpenGLES, false);
			} else if((render == QString::fromUtf8("GLES2")) ||
					 (render == QString::fromUtf8("GLESV2")) ||
					 (render == QString::fromUtf8("GLES3")) ||
					 (render == QString::fromUtf8("GLESV3")) ||
					 (render == QString::fromUtf8("GLES"))) {
				config.render_platform = CONFIG_RENDER_PLATFORM_OPENGL_ES;
				config.render_major_version = 2;
				config.render_minor_version = 1;
				GuiMain->setAttribute(Qt::AA_UseDesktopOpenGL, false);
				GuiMain->setAttribute(Qt::AA_UseOpenGLES, true);
			}
		}
	}
	if(cmdparser->isSet(*_opt_envver)) {
		QStringList nList = cmdparser->values(*_opt_envver);
		QString tv;
		//QProcessEnvironment ev = QProcessEnvironment::systemEnvironment();
		QProcessEnvironment ev = _envvers;
		if(nList.size() > 0) {
			for(int i = 0; i < nList.size(); i++) {
				tv = nList.at(i);
				if(tv.indexOf(QString::fromUtf8("-")) == 0) {
					// Delete var
					int n1 = tv.indexOf(QString::fromUtf8("="));
					if(n1 > 0) {
						tv = tv.left(n1).right(n1 - 1);
					} else {
						tv = tv.right(tv.length() - 1);
					}
					printf("DEBUG: DEL ENV:%s\n", tv.toLocal8Bit().constData());
					ev.remove(tv);
				} else if(tv.indexOf(QString::fromUtf8("=")) > 0) {
					// Delete var
					int n1 = tv.indexOf(QString::fromUtf8("="));
					QString skey;
					QString sval;
					skey = tv.left(n1);
					if((tv.length() - n1) < 1) {
						sval = QString::fromUtf8("");
					} else {
						sval = tv.right(tv.length() - n1 - 1);
					}
					printf("DEBUG: SET ENV:%s to %s\n", skey.toLocal8Bit().constData(), sval.toLocal8Bit().constData());
					if(skey.length() > 0) ev.insert(skey, sval);
				} else if(tv.indexOf(QString::fromUtf8("=")) < 0) {
					printf("DEBUG: SET ENV:%s to (NULL)\n", tv.toLocal8Bit().constData());
					if(tv.length() > 0) ev.insert(tv, QString::fromUtf8(""));
				}
			}
			_envvers.swap(ev);
		}
	}
	_b_dump_envver = false;
	if(cmdparser->isSet(*_opt_dump_envver)) {
		_b_dump_envver = true;
	}
	uint32_t dipsw_onbits = 0x0000000;
	uint32_t dipsw_offmask = 0xffffffff;
	if(cmdparser->isSet(*_opt_dipsw_off)) {
		QStringList bitList = cmdparser->values(*_opt_dipsw_off);
		QString tv;
		bool num_ok;
		for(int i = 0; i < bitList.size(); i++) {
			tv = bitList.at(i);
			int _bit = tv.toInt(&num_ok);
			if(num_ok) {
				if((_bit >= 0) && (_bit < 32)) {
					dipsw_offmask &= (uint32_t)(~(1 << _bit));
				}
			}
		}
	}
	if(cmdparser->isSet(*_opt_dipsw_on)) {
		QStringList bitList = cmdparser->values(*_opt_dipsw_on);
		QString tv;
		bool num_ok;
		for(int i = 0; i < bitList.size(); i++) {
			tv = bitList.at(i);
			int _bit = tv.toInt(&num_ok);
			if(num_ok) {
				if((_bit >= 0) && (_bit < 32)) {
					dipsw_onbits |= (uint32_t)(1 << _bit);
				}
			}
		}
	}
	config.dipswitch &= dipsw_offmask;
	config.dipswitch |= dipsw_onbits;

}

void OpeningMessage(std::string archstr)
{
	csp_logger->set_emu_vm_name(DEVICE_NAME); // Write to syslog, console
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Start Common Source Project '%s'", my_procname.c_str());
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "(C) Toshiya Takeda / Qt Version K.Ohta");
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Architecture: %s", archstr.c_str());
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Use -h or --help for help.");
	
	//csp_logger->debug_log(AGAR_LOG_INFO, " -? is print help(s).");
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Home = %s, Resource directory = %s",
						  cpp_homedir.c_str(),
						  sRssDir.c_str()); // Debug
	
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Config dir = %s, config_file = %s",
						  cpp_confdir.c_str(),
						  config_fullpath.c_str()); // Debug
	
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "DIPSW VALUE IS 0x%08x", config.dipswitch);
	if(virtualMediaList.size() >= 2) {
		for(int i = 0; i < virtualMediaList.size(); i += 2) {
			csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Virtual media %d, type %s, name %s",
								  i / 2,
								  virtualMediaList.at(i).toLocal8Bit().constData(),
								  virtualMediaList.at(i + 1).toLocal8Bit().constData());
		}
	}
}

void SetupSDL(void)
{
	QStringList _el = _envvers.toStringList();
	if(_el.size() > 0) {
		for(int i = 0; i < _el.size(); i++) {
			QString _s = _el.at(i);
			if(_s.startsWith("SDL_")) {
				QString skey, svar;
				int _nl;
				_nl = _s.indexOf('=');
				if(_nl >= 0) {
					skey = _s.left(_nl);
					svar = _s.right(_s.length() - _nl);
				} else {
					skey = _s;
					svar = QString::fromUtf8("");
				}
				SDL_setenv(skey.toLocal8Bit().constData(), svar.toLocal8Bit().constData(), 1);
				csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Note: SDL ENVIROMENT : %s to %s.",
									  skey.toLocal8Bit().constData(),
									  svar.toLocal8Bit().constData());
			}
		}
	}
#if defined(USE_SDL2)
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
#else
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
#endif
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Audio and JOYSTICK subsystem was initialised.");
}

void SetupLogger(std::string emustr, int _size)
{

	csp_logger = new CSP_Logger(config.log_to_syslog, config.log_to_console, emustr.c_str()); // Write to syslog, console
	csp_logger->set_log_stdout(CSP_LOG_DEBUG, true);
	csp_logger->set_log_stdout(CSP_LOG_INFO, true);
	csp_logger->set_log_stdout(CSP_LOG_WARN, true);
	for(int ii = 0; ii < _size; ii++) {
		for(int jj = 0; jj < 8; jj++) {
			csp_logger->set_device_node_log(ii, 1, jj, config.dev_log_to_syslog[ii][jj]);
			csp_logger->set_device_node_log(ii, 2, jj, config.dev_log_to_console[ii][jj]);
			csp_logger->set_device_node_log(ii, 0, jj, config.dev_log_recording[ii][jj]);
		}
	}
}

int MainLoop(int argc, char *argv[])
{

	std::string archstr;
	std::string emustr("emu");
	std::string cfgstr(CONFIG_NAME);
	std::string delim;
	QString emudesc;

	setup_logs();
#if defined(Q_OS_WIN)
	delim = "\\";
#else
	delim = "/";
#endif
	
	GuiMain = new QApplication(argc, argv);
	GuiMain->setObjectName(QString::fromUtf8("Gui_Main"));
	_envvers = QProcessEnvironment::systemEnvironment();
	
    QCommandLineParser cmdparser;

	virtualMediaList.clear();
	SetOptions(&cmdparser);

	cmdparser.process(QCoreApplication::arguments());

	ProcessCmdLine(&cmdparser, &virtualMediaList);

	emustr = emustr + cfgstr;

	SetupLogger(emustr, CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1);

	
	archstr = "Generic";
#if defined(__x86_64__)
	archstr = "amd64";
#endif
#if defined(__i386__)
	archstr = "ia32";
#endif
	OpeningMessage(archstr);
	SetupSDL();
	/*
	 * Into Qt's Loop.
	 */
	USING_FLAGS_EXT *using_flags = new USING_FLAGS_EXT(&config);
	// initialize emulation core

	//SetupTranslators();
	QTranslator local_translator;
	QLocale s_locale;
	if(local_translator.load(s_locale, QLatin1String("csp_qt_machine"), QLatin1String("_"), QLatin1String(":/"))) {
		GuiMain->installTranslator(&local_translator);
	}
	QTranslator s_translator;
	if(s_translator.load(s_locale, QLatin1String("csp_qt_gui"), QLatin1String("_"), QLatin1String(":/"))) {
		GuiMain->installTranslator(&s_translator);
	}
	QTranslator common_translator;
	if(common_translator.load(s_locale, QLatin1String("csp_qt_common"), QLatin1String("_"), QLatin1String(":/"))) {
		GuiMain->installTranslator(&common_translator);
	}
	QTranslator debugger_translator;
	if(debugger_translator.load(s_locale, QLatin1String("csp_qt_debugger"), QLatin1String("_"), QLatin1String(":/"))) {
		GuiMain->installTranslator(&debugger_translator);
	}
	//QProcessEnvironment::systemEnvironment() = _envvers;
	if(_b_dump_envver) {
		//QProcessEnvironment ev = QProcessEnvironment::systemEnvironment();
		QProcessEnvironment ev = _envvers;
		QStringList el = _envvers.toStringList();
		if(el.size() > 0) {
			csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Environment Variables:");
			for(int i = 0; i < el.size(); i++) {
				csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "#%d : %s", i, el.at(i).toLocal8Bit().constData());
			}
		}
	}
	
	rMainWindow = new META_MainWindow(using_flags, csp_logger);
	rMainWindow->connect(rMainWindow, SIGNAL(sig_quit_all(void)), rMainWindow, SLOT(deleteLater(void)));
	rMainWindow->setCoreApplication(GuiMain);
	rMainWindow->getWindow()->show();
			
	// main loop
	rMainWindow->LaunchEmuThread();
#if defined(USE_JOYSTICK)
	rMainWindow->LaunchJoyThread();
#endif	
	GLDrawClass *pgl = rMainWindow->getGraphicsView();
	pgl->set_emu_launched();
	pgl->setFixedSize(pgl->width(), pgl->height());
	rMainWindow->retranselateUi_Depended_OSD();
	QObject::connect(emu->get_osd(), SIGNAL(sig_update_device_node_name(int, const _TCHAR *)),
					 rMainWindow, SLOT(do_update_device_node_name(int, const _TCHAR *)));
	for(int i = 0; i < (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1); i++) {
		rMainWindow->do_update_device_node_name(i, using_flags->get_vm_node_name(i));
	}
	csp_logger->set_osd(emu->get_osd());
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "InitInstance() OK.");
	
	QObject::connect(GuiMain, SIGNAL(lastWindowClosed()),
					 rMainWindow, SLOT(on_actionExit_triggered()));

	GuiMain->exec();
	return 0;
}

void Ui_MainWindow::do_update_inner_fd(int drv, QStringList base, class Action_Control **action_select_media_list,
				       QStringList lst, int num, bool use_d88_menus)
{
#if defined(USE_FLOPPY_DISK)
	if(use_d88_menus) {
		for(int ii = 0; ii < using_flags->get_max_d88_banks(); ii++) {
			if(ii < emu->d88_file[drv].bank_num) {
				base << lst.value(ii);
				action_select_media_list[ii]->setText(lst.value(ii));
				action_select_media_list[ii]->setVisible(true);
				if(ii == num) action_select_media_list[ii]->setChecked(true);
			} else {
				if(action_select_media_list[ii] != NULL) {
					action_select_media_list[ii]->setText(QString::fromUtf8(""));
					action_select_media_list[ii]->setVisible(false);
				}
			}
		}
	}
#endif	
}

void Ui_MainWindow::do_update_inner_bubble(int drv, QStringList base, class Action_Control **action_select_media_list,
				       QStringList lst, int num, bool use_d88_menus)
{
#if defined(USE_BUBBLE)	
	if(use_d88_menus) {
		for(int ii = 0; ii < using_flags->get_max_b77_banks(); ii++) {
			if(ii < emu->b77_file[drv].bank_num) {
				base << lst.value(ii);
				action_select_media_list[ii]->setText(lst.value(ii));
				action_select_media_list[ii]->setVisible(true);
				if(ii == num) action_select_media_list[ii]->setChecked(true);
			} else {
				if(action_select_media_list[ii] != NULL) {
					action_select_media_list[ii]->setText(QString::fromUtf8(""));
					action_select_media_list[ii]->setVisible(false);
				}
			}
		}
	}
#endif	
}

int Ui_MainWindow::GetBubbleBankNum(int drv)
{
#if MAX_BUBBLE
	if((emu != NULL) && (drv >= 0) && (drv < MAX_BUBBLE)) {
		return emu->b77_file[drv].bank_num;
	}
#endif
	return 0;
}

int Ui_MainWindow::GetBubbleCurrentBankNum(int drv)
{
#if MAX_BUBBLE
	if((emu != NULL) && (drv >= 0) && (drv < MAX_BUBBLE)) {
		return emu->b77_file[drv].cur_bank;
	}
#endif
	return 0;
}

bool Ui_MainWindow::GetBubbleCasetteIsProtected(int drv)
{
#if MAX_BUBBLE
	if(emu != NULL) {
		if((drv >= 0) && (drv < MAX_BUBBLE)) {
			return emu->is_bubble_casette_protected(drv);
		}
	}
#endif
	return false;
}

QString Ui_MainWindow::GetBubbleB77FileName(int drv)
{
	QString ans = QString::fromUtf8("");
#if MAX_BUBBLE
	if(emu != NULL) {
		if((drv < MAX_BUBBLE) && (drv >= 0)) {
			ans = QString::fromLocal8Bit(emu->b77_file[drv].path);
		}
	}
#endif
	return ans;
}

QString Ui_MainWindow::GetBubbleB77BubbleName(int drv, int num)
{
	QString ans = QString::fromUtf8("");
#if MAX_BUBBLE
	if(emu != NULL) {
		if((drv < MAX_BUBBLE) && (drv >= 0)) {
			if((num >= 0) && (num < MAX_B77_BANKS)) {
				ans = QString::fromLocal8Bit(emu->b77_file[drv].bubble_name[num]);
			}
		}
	}
#endif
	return ans;
}

#ifdef USE_DEBUGGER
#include <../debugger/qt_debugger.h>

void Ui_MainWindow::OnOpenDebugger(int no)
{
	if((no < 0) || (no > 7)) return;
	//emu->open_debugger(no);
	VM *vm = emu->get_vm();

 	if((emu->now_debugging ) || (emu->hDebugger != NULL)) /* OnCloseDebugger(); */ return;
	
	if(!(emu->now_debugging && emu->debugger_thread_param.cpu_index == no)) {
		//emu->close_debugger();
		if(vm->get_cpu(no) != NULL && vm->get_cpu(no)->get_debugger() != NULL) {
			QString windowName = QString::fromUtf8(vm->get_cpu(no)->get_device_name());
			windowName = QString::fromUtf8("Debugger ") + windowName;
			emu->hDebugger = new CSP_Debugger(this);
			QString objNameStr = QString("EmuDebugThread");
			emu->hDebugger->setObjectName(objNameStr);
			emu->hDebugger->debugger_thread_param.osd = emu->get_osd();
			emu->hDebugger->debugger_thread_param.vm = vm;
			emu->hDebugger->debugger_thread_param.cpu_index = no;
			emu->stop_record_sound();
			emu->stop_record_video();
			//emu->now_debugging = true;
			connect(this, SIGNAL(quit_debugger_thread()), emu->hDebugger, SLOT(doExit()));
			connect(this, SIGNAL(destroyed()), emu->hDebugger, SLOT(do_destroy_thread()));
			//connect(this, SIGNAL(quit_debugger_thread()), emu->hDebugger, SLOT(close()));
			connect(emu->hDebugger, SIGNAL(sig_finished()), this, SLOT(OnCloseDebugger()));
			connect(emu->hDebugger, SIGNAL(sig_put_string(QString)), emu->hDebugger, SLOT(put_string(QString)));
			emu->hDebugger->show();
			emu->hDebugger->run();
			emu->hDebugger->setWindowTitle(windowName);
		}
	}
}

void Ui_MainWindow::OnCloseDebugger(void )
{

//	emu->close_debugger();
 	if(emu->now_debugging) {
		if(emu->hDebugger->debugger_thread_param.running) {
			emit quit_debugger_thread();
			//emu->hDebugger->wait();
		}
 	}
	if(emu != NULL) {
		if(emu->hDebugger != NULL) {
			delete emu->hDebugger;
			emu->hDebugger = NULL;
		}
		emu->now_debugging = false;
	}
}
#endif

