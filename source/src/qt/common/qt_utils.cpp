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
#include <memory>

#include <QApplication>
#include <QString>
#include <QTextCodec>
#include <QImage>
#include <QImageReader>
#include <QDateTime>
#include <QDir>
#include <QTranslator>
#include <QProcessEnvironment>
#include <QCommandLineParser>

#include "common.h"
#include "fileio.h"
#include "config.h"
#include "emu.h"
#include "../osd.h"

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

#include "menu_binary.h"
#include "menu_bubble.h"
#include "menu_cart.h"
#include "menu_cmt.h"
#include "menu_compactdisc.h"
#include "menu_disk.h"
#include "menu_harddisk.h"
#include "menu_laserdisc.h"
#include "menu_quickdisk.h"


#include "menu_flags_ext.h"
#include "dialog_movie.h"
#include "../avio/movie_saver.h"
// emulation core
#include "../../vm/vm_limits.h"
#include "../../vm/fmgen/fmgen.h"

//QApplication *GuiMain = NULL;
extern config_t config;

// Start to define MainWindow.
class META_MainWindow *rMainWindow;


// buttons

// menu
extern DLL_PREFIX_I std::string cpp_homedir;
extern DLL_PREFIX_I std::string cpp_confdir;
extern DLL_PREFIX_I std::string my_procname;
extern DLL_PREFIX_I std::string sRssDir;

void Ui_MainWindow::do_set_mouse_enable(bool flag)
{
#ifdef USE_MOUSE
	if(hRunEmu == nullptr) return;
	EMU_TEMPLATE *p_emu = hRunEmu->get_emu();
	if(p_emu == nullptr) return;

	p_emu->lock_vm();
	if(flag) {
		graphicsView->grabMouse();
		p_emu->enable_mouse();
	} else {
		graphicsView->releaseMouse();
		p_emu->disable_mouse();
	}
	p_emu->unlock_vm();
#endif
}

void Ui_MainWindow::do_toggle_mouse(void)
{
#ifdef USE_MOUSE
	if(hRunEmu == nullptr) return;
	EMU_TEMPLATE *p_emu = hRunEmu->get_emu();
	if(p_emu == nullptr) return;
	if(graphicsView == nullptr) return;

	p_emu->lock_vm();
	bool flag = p_emu->is_mouse_enabled();
	if(!flag) {
		graphicsView->grabMouse();
		p_emu->enable_mouse();
	} else {
		graphicsView->releaseMouse();
		p_emu->disable_mouse();
	}
	p_emu->unlock_vm();
#endif
}

void Ui_MainWindow::rise_movie_dialog(void)
{
	CSP_DialogMovie *dlg = new CSP_DialogMovie(hSaveMovieThread, using_flags);
	dlg->setWindowTitle(QApplication::translate("CSP_DialogMovie", "Configure movie encodings", 0));
	dlg->show();
}

void Ui_MainWindow::LaunchEmuThread(EmuThreadClassBase *m)
{
	QString objNameStr;
	GLDrawClass *glv = this->getGraphicsView();

	int drvs;

	hRunEmu = m;
	if(hRunEmu == nullptr) return;
	EMU_TEMPLATE* p_emu = hRunEmu->get_emu();
	if(p_emu == nullptr) return;

	OSD_BASE* p_osd = p_emu->get_osd();
	if(p_osd == nullptr) return;

	connect(hRunEmu, SIGNAL(message_changed(QString)), this, SLOT(message_status_bar(QString)), Qt::QueuedConnection);
	connect(hRunEmu, SIGNAL(sig_is_enable_mouse(bool)), this, SLOT(do_set_mouse_enable(bool)));
	connect(glv, SIGNAL(sig_key_down(uint32_t, uint32_t, bool)), hRunEmu, SLOT(do_key_down(uint32_t, uint32_t, bool)));
	connect(glv, SIGNAL(sig_key_up(uint32_t, uint32_t)),hRunEmu, SLOT(do_key_up(uint32_t, uint32_t)));
	connect(this, SIGNAL(sig_quit_widgets()), glv, SLOT(do_stop_run_vm()));

	if(action_ResetFixedCpu != nullptr) {
		connect(action_ResetFixedCpu, SIGNAL(triggered()),
				hRunEmu, SLOT(do_set_emu_thread_to_fixed_cpu_from_action()));

	}
	for(int i = 0 ; i < 128 ; i++) {
		if(action_SetFixedCpu[i] == nullptr) break;
		connect(action_SetFixedCpu[i], SIGNAL(triggered()),
				hRunEmu, SLOT(do_set_emu_thread_to_fixed_cpu_from_action()));
	}
	connect(this, SIGNAL(sig_vm_reset()), hRunEmu, SLOT(do_reset()));

	for(int i = 0 ; i < using_flags->get_use_special_reset_num() ; i++) {
		if(actionSpecial_Reset[i] != nullptr) {
			connect(actionSpecial_Reset[i], SIGNAL(triggered()), hRunEmu, SLOT(do_special_reset()));
		}
		if(i >= 15) break;
	}
	connect(this, SIGNAL(sig_emu_update_config()), hRunEmu, SLOT(do_update_config()));
	connect(this, SIGNAL(sig_emu_update_volume_level(int, int)), hRunEmu, SLOT(do_update_volume_level(int, int)));
	connect(this, SIGNAL(sig_emu_update_volume_balance(int, int)), hRunEmu, SLOT(do_update_volume_balance(int, int)));
	connect(this, SIGNAL(sig_emu_start_rec_sound()), hRunEmu, SLOT(do_start_record_sound()));
	connect(this, SIGNAL(sig_emu_stop_rec_sound()), hRunEmu, SLOT(do_stop_record_sound()));
	connect(this, SIGNAL(sig_emu_set_display_size(int, int, int, int)), hRunEmu, SLOT(do_set_display_size(int, int, int, int)));
	connect(this, SIGNAL(sig_emu_thread_to_fixed_cpu(int)), hRunEmu, SLOT(set_emu_thread_to_fixed_cpu(int)));

	if(using_flags->is_use_state()) {
		for(int i = 0; i < 10; i++) {
			connect(actionLoad_State[i], SIGNAL(triggered()), hRunEmu, SLOT(do_load_state())); // OK?
			connect(actionSave_State[i], SIGNAL(triggered()), hRunEmu, SLOT(do_save_state())); // OK?
		}
	}
#if defined(USE_FLOPPY_DISK)
//	connect(this, SIGNAL(sig_write_protect_floppy_disk(int, bool)),
//			hRunEmu, SLOT(do_write_protect_floppy_disk(int, bool)),
//			Qt::QueuedConnection);
	connect(this, SIGNAL(sig_open_floppy_disk(int, QString, int)),
			hRunEmu, SLOT(do_open_floppy_disk(int, QString, int)),
			Qt::QueuedConnection);
	connect(this, SIGNAL(sig_close_floppy_disk_ui(int)),
			hRunEmu, SLOT(do_close_floppy_disk_ui(int)),
		Qt::QueuedConnection);
	//connect(hRunEmu, SIGNAL(sig_change_osd_fd(int, QString)), this, SLOT(do_change_osd_fd(int, QString)));

	// ToDo: eject from EMU_THREAD:: .
	connect(p_osd, SIGNAL(sig_ui_floppy_insert_history(int, QString, quint64)),
					 this, SLOT(do_ui_floppy_insert_history(int, QString, quint64)),
					 Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_floppy_write_protect(int, quint64)),
			this, SLOT(do_ui_write_protect_floppy_disk(int, quint64)),
			Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_floppy_close(int)),
			this, SLOT(do_ui_eject_floppy_disk(int)),
			Qt::QueuedConnection);

	drvs = USE_FLOPPY_DISK;
	for(int ii = 0; ii < drvs; ii++) {
		if(menu_fds[ii] != nullptr) {
			menu_fds[ii]->connect_via_emu_thread(hRunEmu);
			connect(menu_fds[ii], SIGNAL(sig_set_inner_slot(int, int)),
					hRunEmu, SLOT(do_select_floppy_disk_d88(int, int)),
					Qt::QueuedConnection);
		}
	}
#endif
#if defined(USE_HARD_DISK)
	for(int ii = 0; ii < USE_HARD_DISK; ii++) {
		if(ii >= USE_HARD_DISK_TMP) break;
		Menu_HDDClass *mp = menu_hdds[ii];
		if(mp != nullptr) {
			mp->connect_via_emu_thread(hRunEmu);
		}
	}
	connect(this, SIGNAL(sig_open_hard_disk(int, QString)),
			hRunEmu, SLOT(do_open_hard_disk(int, QString)),
			Qt::QueuedConnection);
//	connect(this, SIGNAL(sig_close_hard_disk_ui(int)),
//			hRunEmu, SLOT(do_close_hard_disk_ui(int)),
//			Qt::QueuedConnection);

	connect(p_osd, SIGNAL(sig_ui_hard_disk_insert_history(int, QString)),
					 this, SLOT(do_ui_hard_disk_insert_history(int, QString)),
					 Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_hard_disk_close(int)),
			this, SLOT(do_ui_eject_hard_disk(int)),
			Qt::QueuedConnection);

#endif
#if defined(USE_TAPE)
	for(int ii = 0; ii < USE_TAPE; ii++) {
		if(ii >= USE_TAPE_TMP) break;
		Menu_CMTClass *mp = menu_CMT[ii];
		if(mp != nullptr) {
			mp->connect_via_emu_thread(hRunEmu);
		}
	}
	connect(this, SIGNAL(sig_play_tape(int, QString)), hRunEmu, SLOT(do_play_tape(int, QString)));
	connect(this, SIGNAL(sig_rec_tape(int, QString)),  hRunEmu, SLOT(do_rec_tape(int, QString)));

	connect(p_osd, SIGNAL(sig_ui_tape_play_insert_history(int, QString)),
					 this, SLOT(do_ui_tape_play_insert_history(int, QString)),
					 Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_tape_record_insert_history(int, QString)),
					 this, SLOT(do_ui_tape_record_insert_history(int, QString)),
					 Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_tape_write_protect(int, quint64)),
			this, SLOT(do_ui_write_protect_tape(int, quint64)),
			Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_tape_eject(int)),
			this, SLOT(do_ui_eject_tape(int)),
			Qt::QueuedConnection);
#endif
#if defined(USE_QUICK_DISK)
	for(int ii = 0; ii < USE_QUICK_DISK; ii++) {
		if(ii >= USE_QUICK_DISK) break;
		Menu_QDClass *mp = menu_QDs[ii];
		if(mp != nullptr) {
			mp->connect_via_emu_thread(hRunEmu);
		}
	}

	connect(this, SIGNAL(sig_write_protect_quick_disk(int, bool)),
			hRunEmu, SLOT(do_write_protect_quick_disk(int, bool)),
			Qt::QueuedConnection);
	connect(this, SIGNAL(sig_open_quick_disk(int, QString)),
			hRunEmu, SLOT(do_open_quick_disk(int, QString)),
			Qt::QueuedConnection);
	connect(this, SIGNAL(sig_close_quick_disk_ui(int)),
			hRunEmu, SLOT(do_close_quick_disk_ui(int)),
		Qt::QueuedConnection);

	connect(p_osd, SIGNAL(sig_ui_quick_disk_write_protect(int, quint64)),
			this, SLOT(do_ui_quick_disk_write_protect(int, quint64)),
			Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_quick_disk_insert_history(int, QString)),
					 this, SLOT(do_ui_quick_disk_insert_history(int, QString)),
					 Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_quick_disk_close(int)),
			this, SLOT(do_ui_eject_quick_disk(int)),
			Qt::QueuedConnection);

#endif
#if defined(USE_CART)
	for(int ii = 0; ii < USE_CART; ii++) {
		if(ii >= USE_CART_TMP) break;
		Menu_CartClass *mp = menu_Cart[ii];
		if(mp != nullptr) {
			mp->connect_via_emu_thread(hRunEmu);
		}
	}
	connect(this, SIGNAL(sig_open_cartridge(int, QString)), hRunEmu, SLOT(do_open_cartridge(int, QString)));
	connect(this, SIGNAL(sig_eject_cartridge_ui(int)), hRunEmu, SLOT(do_close_cartridge_ui(int)));

	connect(p_osd, SIGNAL(sig_ui_cartridge_insert_history(int, QString)),
					 this, SLOT(do_ui_cartridge_insert_history(int, QString)),
					 Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_cartridge_eject(int)),
			this, SLOT(do_ui_eject_cartridge(int)),
			Qt::QueuedConnection);
#endif
#if defined(USE_COMPACT_DISC)
	for(int ii = 0; ii < USE_COMPACT_DISC; ii++) {
		if(ii >= USE_COMPACT_DISC_TMP) break;
		Menu_CompactDiscClass *mp = menu_CDROM[ii];
		if(mp != nullptr) {
			mp->connect_via_emu_thread(hRunEmu);
		}
	}
	connect(this, SIGNAL(sig_open_compact_disc(int, QString)),
			hRunEmu, SLOT(do_open_compact_disc(int, QString)),
			Qt::QueuedConnection);
	connect(this, SIGNAL(sig_eject_compact_disc_ui(int)),
			hRunEmu, SLOT(do_eject_compact_disc_ui(int)),
			Qt::QueuedConnection);

	connect(p_osd, SIGNAL(sig_ui_compact_disc_insert_history(int, QString)),
					 this, SLOT(do_ui_compact_disc_insert_history(int, QString)),
					 Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_compact_disc_eject(int)),
			this, SLOT(do_ui_eject_compact_disc(int)),
			Qt::QueuedConnection);
#endif
#if defined(USE_LASER_DISC)
	for(int ii = 0; ii < USE_LASER_DISC; ii++) {
		if(ii >= USE_LASER_DISC_TMP) break;
		Menu_LaserdiscClass *mp = menu_Laserdisc[ii];
		if(mp != nullptr) {
			mp->connect_via_emu_thread(hRunEmu);
		}
	}
	connect(this, SIGNAL(sig_open_laser_disc(int, QString)),
			hRunEmu, SLOT(do_open_laser_disc(int, QString)),
			Qt::QueuedConnection);
	connect(this, SIGNAL(sig_close_laser_disc_ui(int)),
			hRunEmu, SLOT(do_close_laser_disc_ui(int)),
			Qt::QueuedConnection);

	connect(p_osd, SIGNAL(sig_ui_laser_disc_insert_history(int, QString)),
					 this, SLOT(do_ui_laser_disc_insert_history(int, QString)),
					 Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_laser_disc_eject(int)),
			this, SLOT(do_ui_eject_laser_disc(int)),
			Qt::QueuedConnection);

	// ToDo: multiple LDs
#endif
#if defined(USE_BINARY_FILE)
	connect(this, SIGNAL(sig_load_binary(int, QString)), hRunEmu, SLOT(do_load_binary(int, QString)));
	connect(this, SIGNAL(sig_save_binary(int, QString)), hRunEmu, SLOT(do_save_binary(int, QString)));
#endif
#if defined(USE_BUBBLE)
	for(int ii = 0; ii < USE_BUBBLE; ii++) {
		if(ii >= USE_BUBBLE_TMP) break;
		Menu_BubbleClass *mp = menu_bubbles[ii];
		if(mp != nullptr) {
			mp->connect_via_emu_thread(hRunEmu);
			connect(mp, SIGNAL(sig_set_inner_slot(int, int)),
					hRunEmu, SLOT(do_select_bubble_casette_b77(int, int)),
					Qt::QueuedConnection);
		}
	}
	connect(this, SIGNAL(sig_open_bubble(int, QString, int)),
			hRunEmu, SLOT(do_open_bubble_casette(int, QString, int)),
			Qt::QueuedConnection);
	connect(this, SIGNAL(sig_close_bubble_ui(int)),
			hRunEmu, SLOT(do_close_bubble_casette_ui(int)),
			Qt::QueuedConnection);

	connect(p_osd, SIGNAL(sig_ui_bubble_insert_history(int, QString, quint64)),
			this, SLOT(do_ui_bubble_insert_history(int, QString, quint64)),
					 Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_bubble_write_protect(int, quint64)),
			this, SLOT(do_ui_bubble_write_protect(int, quint64)),
					 Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_ui_bubble_closed(int)),
			this, SLOT(do_ui_eject_bubble_casette(int)),
			Qt::QueuedConnection);

#endif

	hRunEmu->set_tape_play(false);
#if defined(USE_KEY_LOCKED) || defined(USE_LED_DEVICE)
	connect(hRunEmu, SIGNAL(sig_send_data_led(quint32)), this, SLOT(do_recv_data_led(quint32)), Qt::QueuedConnection);
#endif
#ifdef USE_AUTO_KEY
	connect(this, SIGNAL(sig_start_auto_key(QString)), hRunEmu, SLOT(do_start_auto_key(QString)), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_stop_auto_key()), hRunEmu, SLOT(do_stop_auto_key()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_set_roma_kana(bool)), hRunEmu, SLOT(set_romakana(bool)), Qt::QueuedConnection);
#endif

	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "EmuThread : Start.");
	objNameStr = QString("EmuThreadClass");
	hRunEmu->setObjectName(objNameStr);

	hDrawEmu = new DrawThreadClass((OSD*)p_osd, csp_logger, this);
	p_emu->set_parent_handler((EmuThreadClass*)hRunEmu, hDrawEmu);

#ifdef ONE_BOARD_MICRO_COMPUTER
	QImageReader *reader = new QImageReader(":/background.png");
	QImage *result = new QImage(reader->read()); // this acts as a default if the size is not matched
	QImage result2 = result->convertToFormat(QImage::Format_ARGB32);
	glv->updateBitmap(&result2);
	p_osd->upload_bitmap(&result2);
	delete result;
	delete reader;
	p_osd->set_buttons();
#endif

	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "DrawThread : Start.");

	connect(hDrawEmu, SIGNAL(sig_draw_frames(int)), hRunEmu, SLOT(print_framerate(int)), Qt::DirectConnection);
	//connect((OSD*)p_osd, SIGNAL(sig_draw_frames(int)), hRunEmu, SLOT(print_framerate(int)));
	connect(hDrawEmu, SIGNAL(message_changed(QString)), this, SLOT(message_status_bar(QString)));
	connect(this, SIGNAL(quit_draw_thread()), hDrawEmu, SLOT(doExit()));
	connect(hDrawEmu, SIGNAL(finished()), hDrawEmu, SLOT(deleteLater()));

	connect(hRunEmu, SIGNAL(window_title_changed(QString)), this, SLOT(do_set_window_title(QString)), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_quit_emu_thread()), hRunEmu, SLOT(doExit()), Qt::QueuedConnection);
	connect(hRunEmu, SIGNAL(sig_mouse_enable(bool)),
			this, SLOT(do_set_mouse_enable(bool)), Qt::QueuedConnection);
	/*if(config.use_separate_thread_draw) {
		connect(hRunEmu, SIGNAL(sig_draw_thread(bool)), hDrawEmu, SLOT(doDraw(bool)));
		connect(hRunEmu, SIGNAL(sig_set_draw_fps(double)), hDrawEmu, SLOT(do_set_frames_per_second(double)));
		connect(hRunEmu, SIGNAL(sig_draw_one_turn(bool)), hDrawEmu, SLOT(do_draw_one_turn(bool)));
		} else*/ {
		connect(hRunEmu, SIGNAL(sig_draw_thread(bool)), hDrawEmu, SLOT(doDraw(bool)));
		connect(hRunEmu, SIGNAL(sig_set_draw_fps(double)), hDrawEmu, SLOT(do_set_frames_per_second(double)));
		connect(hRunEmu, SIGNAL(sig_draw_one_turn(bool)), hDrawEmu, SLOT(do_draw_one_turn(bool)));
	}
	//connect(hRunEmu, SIGNAL(sig_draw_thread(bool)), (OSD*)p_osd, SLOT(do_draw(bool)));
	connect(hRunEmu, SIGNAL(quit_draw_thread()), hDrawEmu, SLOT(doExit()));

	connect(glv, SIGNAL(sig_notify_move_mouse(double, double, double, double)),
			hRunEmu, SLOT(moved_mouse(double, double, double, double)), Qt::QueuedConnection);
	connect(glv, SIGNAL(do_notify_button_pressed(Qt::MouseButton)),
	        hRunEmu, SLOT(button_pressed_mouse(Qt::MouseButton)));
	connect(glv, SIGNAL(do_notify_button_released(Qt::MouseButton)),
			hRunEmu, SLOT(button_released_mouse(Qt::MouseButton)));

	connect(actionCapture_Screen, SIGNAL(triggered()), glv, SLOT(do_save_frame_screen()));
	connect(this, SIGNAL(sig_emu_launched()), glv, SLOT(set_emu_launched()));

#ifdef USE_MOUSE
	connect(glv, SIGNAL(sig_toggle_mouse(void)),
			this, SLOT(do_toggle_mouse(void)),  Qt::QueuedConnection);
	connect(glv, SIGNAL(sig_toggle_grab_mouse()), this, SLOT(do_toggle_mouse()),  Qt::QueuedConnection);
#endif
	connect(hRunEmu, SIGNAL(sig_resize_screen(int, int)),
			glv, SLOT(resizeGL(int, int)), Qt::QueuedConnection);
	connect(hRunEmu, SIGNAL(sig_resize_osd(int)), driveData, SLOT(setScreenWidth(int)), Qt::QueuedConnection);
	connect(hRunEmu, SIGNAL(sig_change_osd(int, int, QString)), driveData, SLOT(updateMessage(int, int, QString)), Qt::QueuedConnection);

	connect(glv, SIGNAL(sig_resize_uibar(int, int)),
			this, SLOT(resize_statusbar(int, int)), Qt::QueuedConnection);
	connect(hRunEmu, SIGNAL(sig_resize_uibar(int, int)),
			this, SLOT(resize_statusbar(int, int)), Qt::QueuedConnection);

	connect((OSD*)p_osd, SIGNAL(sig_req_encueue_video(int, int, int)),
			hDrawEmu, SLOT(do_req_encueue_video(int, int, int)));

	objNameStr = QString("EmuDrawThread");
	hDrawEmu->setObjectName(objNameStr);

	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "DrawThread : Launch done.");

	hSaveMovieThread = new MOVIE_SAVER(640, 400,  30, (OSD*)p_osd, &config);

	// SAVING MOVIES
//	connect(this, SIGNAL(sig_start_saving_movie()),	hRunEmu, SLOT(do_start_record_video()));
	connect(actionStart_Record_Movie, SIGNAL(triggered()), hRunEmu, SLOT(do_start_record_video()));
	connect(actionStop_Record_Movie, SIGNAL(triggered()), hRunEmu, SLOT(do_stop_record_video()));

	connect(hSaveMovieThread, SIGNAL(sig_set_state_saving_movie(bool)), this, SLOT(do_set_state_saving_movie(bool)));

	connect((OSD*)p_osd, SIGNAL(sig_save_as_movie(QString, int, int)),
			hSaveMovieThread, SLOT(do_open(QString, int, int)));
	connect((OSD*)p_osd, SIGNAL(sig_stop_saving_movie()), hSaveMovieThread, SLOT(do_close()));

	actionStop_Record_Movie->setIcon(QIcon(":/icon_process_stop.png"));
	actionStop_Record_Movie->setVisible(false);

	connect(this, SIGNAL(sig_movie_set_width(int)), hSaveMovieThread, SLOT(do_set_width(int)));
	connect(this, SIGNAL(sig_movie_set_height(int)), hSaveMovieThread, SLOT(do_set_height(int)));

	connect((OSD*)p_osd, SIGNAL(sig_movie_set_width(int)), hSaveMovieThread, SLOT(do_set_width(int)));
	connect((OSD*)p_osd, SIGNAL(sig_movie_set_height(int)), hSaveMovieThread, SLOT(do_set_height(int)));

	connect((OSD*)p_osd, SIGNAL(sig_enqueue_audio(int16_t*, int)), hSaveMovieThread, SLOT(enqueue_audio(int16_t *, int)));
	connect((OSD*)p_osd, SIGNAL(sig_enqueue_video(int, int, int, QImage *)),
			hSaveMovieThread, SLOT(enqueue_video(int, int, int, QImage *)), Qt::DirectConnection);
	connect(glv->extfunc, SIGNAL(sig_push_image_to_movie(int, int, int, QImage *)),
			hSaveMovieThread, SLOT(enqueue_video(int, int, int, QImage *)));
	connect(this, SIGNAL(sig_quit_movie_thread()), hSaveMovieThread, SLOT(do_exit()));

	objNameStr = QString("EmuMovieThread");
	hSaveMovieThread->setObjectName(objNameStr);
	hSaveMovieThread->start();
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "MovieThread : Launch done.");

	connect(action_SetupMovie, SIGNAL(triggered()), this, SLOT(rise_movie_dialog()));
	connect(hRunEmu, SIGNAL(sig_change_access_lamp(int, int, QString)), driveData, SLOT(updateLabel(int, int, QString)), Qt::QueuedConnection);
	connect(hRunEmu, SIGNAL(sig_set_access_lamp(int, bool)), graphicsView, SLOT(do_display_osd_leds(int, bool)), Qt::QueuedConnection);
	connect(hRunEmu, SIGNAL(sig_change_virtual_media(int, int, QString)), driveData, SLOT(updateMediaFileName(int, int, QString)), Qt::QueuedConnection);
	connect((OSD*)p_osd, SIGNAL(sig_change_virtual_media(int, int, QString)), driveData, SLOT(updateMediaFileName(int, int, QString)));
	connect((OSD*)p_osd, SIGNAL(sig_enable_mouse()), glv, SLOT(do_enable_mouse()));
	connect((OSD*)p_osd, SIGNAL(sig_disable_mouse()), glv, SLOT(do_disable_mouse()));

	connect(this, SIGNAL(sig_unblock_task()), hRunEmu, SLOT(do_unblock()));
	connect(this, SIGNAL(sig_block_task()), hRunEmu, SLOT(do_block()));
	connect(this, SIGNAL(sig_start_emu_thread()), hRunEmu, SLOT(do_start_emu_thread()));
	connect(this, SIGNAL(sig_start_draw_thread()), hDrawEmu, SLOT(do_start_draw_thread()));


//	hRunEmu->start(QThread::HighestPriority);
	this->set_screen_aspect(config.window_stretch_type);
	emit sig_movie_set_width(SCREEN_WIDTH);
	emit sig_movie_set_height(SCREEN_HEIGHT);
//	hRunEmu->start(QThread::HighestPriority);
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "EmuThread : Launch done.");
}

void Ui_MainWindow::do_create_d88_media(int drv, quint8 media_type, QString name)
{
	if(hRunEmu == nullptr) return;
	EMU_TEMPLATE *p_emu = hRunEmu->get_emu();
	if(p_emu == nullptr) return;

	if(!(name.isEmpty()) && (drv >= 0)) {
#if defined(USE_FLOPPY_DISK)
		if(drv < USE_FLOPPY_DISK) {
			const _TCHAR* path = (const _TCHAR *)(name.toLocal8Bit().data());
			if(p_emu->create_blank_floppy_disk(path, media_type)) {
				emit sig_open_floppy_disk(drv, name, 0);
			}
		}
#endif
	}
}

void Ui_MainWindow::do_create_hard_disk(int drv, int sector_size, int sectors, int surfaces, int cylinders, QString name)
{
	if(hRunEmu == nullptr) return;
	EMU_TEMPLATE *p_emu = hRunEmu->get_emu();
	if(p_emu == nullptr) return;

	if(!(name.isEmpty()) && (drv >= 0)) {
#if defined(USE_HARD_DISK)
		if(drv < USE_HARD_DISK) {
			const _TCHAR* path = (const _TCHAR *)(name.toLocal8Bit().data());
			if(p_emu->create_blank_hard_disk(path, sector_size, sectors, surfaces, cylinders)) {
				emit sig_open_hard_disk(drv, name);
			}
		}
#endif
	}
}

void Ui_MainWindow::LaunchJoyThread(void)
{
#if defined(USE_JOYSTICK)
	if(hRunEmu == nullptr) return;
	EMU_TEMPLATE *p_emu = hRunEmu->get_emu();
	if(p_emu == nullptr) return;

	hRunJoy = new JoyThreadClass(p_emu, using_flags, &config);
	connect(this, SIGNAL(quit_joy_thread()), hRunJoy, SLOT(doExit()));
	connect(hRunJoy, SIGNAL(finished()), hRunJoy, SLOT(deleteLater()));

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
	if(hRunEmu == nullptr) return;
	EMU_TEMPLATE *p_emu = hRunEmu->get_emu();
	if(p_emu == nullptr) return;
	if(p_emu) {
		//emu->update_screen();
	}
}

void Ui_MainWindow::OnWindowMove(void)
{
	if(hRunEmu == nullptr) return;
	EMU_TEMPLATE *p_emu = hRunEmu->get_emu();
	if(p_emu == nullptr) return;

	if(p_emu) {
		p_emu->suspend();
	}
}



#include <string>

void Ui_MainWindow::OnMainWindowClosed(void)
{
	// notify power off
	emit sig_notify_power_off();
	if(statusUpdateTimer != NULL) statusUpdateTimer->stop();
#if defined(USE_KEY_LOCKED) || defined(USE_LED_DEVICE)
	if(ledUpdateTimer != NULL) ledUpdateTimer->stop();
#endif
	emit quit_draw_thread();
	emit quit_joy_thread();
	emit sig_quit_emu_thread();
	emit sig_quit_movie_thread();
	emit sig_quit_widgets();

	if(hSaveMovieThread != nullptr) {
		// When recording movie, stopping will spend a lot of seconds.
		if(!(hSaveMovieThread->wait(60 * 1000))) { // 60 Sec
			hSaveMovieThread->terminate();
			QThread::msleep(1000);
		}
		delete hSaveMovieThread;
		hSaveMovieThread = NULL;
	}

//	if(hDrawEmu != nullptr) {
//		if(!(hDrawEmu->wait(1000))) {
//			hDrawEmu->terminate();
//		}
//		delete hDrawEmu;
//		hDrawEmu = nullptr;
//	}
	if(hRunEmu != nullptr) {
		OnCloseDebugger();
		if(hRunEmu->get_emu() != nullptr) {
			OSD* op = (OSD*)(hRunEmu->get_emu()->get_osd());
			if(op != nullptr) {
				op->setParent(this);
				op->moveToThread(this->thread());
			}
		}
		hRunEmu->quit();
		if(!(hRunEmu->wait(2000))) {
			hRunEmu->terminate();
			QThread::msleep(100);
		}
		delete hRunEmu;
		hRunEmu = nullptr;
#if 0
		save_config(create_local_path(_T("%s.ini"), _T(CONFIG_NAME)));
#else
		{
			char tmps[128] = {0};
			std::string localstr;
			//snprintf(tmps, sizeof(tmps), _T("%s.ini"), _T(CONFIG_NAME));
			my_stprintf_s(tmps, sizeof(tmps) - 1, _T("%s.ini"), _T(CONFIG_NAME));
			localstr = tmps;
			localstr = cpp_confdir + localstr;
			save_config(localstr.c_str());
		}
#endif
	}
#if defined(USE_JOYSTICK)
//	if(hRunJoy != nullptr) {
//		if(!(hRunJoy->wait(1000))) {
//			hRunJoy->terminate();
//		}
//		delete hRunJoy;
//		hRunJoy = nullptr;
//	}
#endif
//	do_release_emu_resources();

	return;
}



// Will remove.
void Ui_MainWindow::do_release_emu_resources(void)
{
	emit sig_quit_emu_thread();
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
extern QCommandLineOption *_opt_homedir;
extern QCommandLineOption *_opt_cfgfile;
extern QCommandLineOption *_opt_cfgdir;
extern QCommandLineOption *_opt_resdir;
extern QCommandLineOption *_opt_opengl;
extern QCommandLineOption *_opt_envver;
extern QCommandLineOption *_opt_dump_envver;
extern QCommandLineOption *_opt_dipsw_on;
extern QCommandLineOption *_opt_dipsw_off;
extern QProcessEnvironment _envvers;
extern bool _b_dump_envver;
extern std::string config_fullpath;

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

extern void SetOptions_Sub(QCommandLineParser *cmdparser);
void SetOptions(QCommandLineParser *cmdparser)
{
	QString emudesc = QString::fromUtf8("Emulator for ");
	emudesc = emudesc + QString::fromUtf8(DEVICE_NAME);
    cmdparser->setApplicationDescription(emudesc);
    cmdparser->addHelpOption();
    //cmdparser->addVersionOption();

	SetOptions_Sub(cmdparser);
	QStringList _cl;

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

extern void ProcessCmdLine_Sub(QCommandLineParser *cmdparser, QStringList *_l);

void ProcessCmdLine(QCommandLineParser *cmdparser, QStringList *_l)
{
	char homedir[PATH_MAX];
	std::string delim;
#if defined(Q_OS_WIN)
	delim = "\\";
#else
	delim = "/";
#endif
	ProcessCmdLine_Sub(cmdparser, _l);
	{
		char tmps[128];
		std::string localstr;
		memset(tmps, 0x00, 128);
		if(cmdparser->isSet(*_opt_cfgfile)) {
			strncpy(tmps, cmdparser->value(*_opt_cfgfile).toLocal8Bit().constData(), 127);
		}
		if(strlen(tmps) <= 0){
			my_stprintf_s(tmps, sizeof(tmps) - 1, _T("%s.ini"), _T(CONFIG_NAME));
		}
		localstr = tmps;
		localstr = cpp_confdir + localstr;
		load_config(localstr.c_str());
		config_fullpath = localstr;
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
			} else if((render == QString::fromUtf8("GL3")) ||
					  (render == QString::fromUtf8("GLV3")) ||
					  (render == QString::fromUtf8("OPENGLV3")) ||
					  (render == QString::fromUtf8("OPENGL")) ||
					  (render == QString::fromUtf8("GL"))) {
				config.render_platform = CONFIG_RENDER_PLATFORM_OPENGL_MAIN;
				config.render_major_version = 3;
				config.render_minor_version = 0;
			} else if((render == QString::fromUtf8("GLES2")) ||
					  (render == QString::fromUtf8("GLESV2")) ||
					  (render == QString::fromUtf8("ESV2")) ||
					  (render == QString::fromUtf8("ES2")) ||
					  (render == QString::fromUtf8("ES")) ||
					  (render == QString::fromUtf8("GLES"))) {
				config.render_platform = CONFIG_RENDER_PLATFORM_OPENGL_ES;
				config.render_major_version = 2;
				config.render_minor_version = 1;
			} else if((render == QString::fromUtf8("GLES3")) ||
					  (render == QString::fromUtf8("GLESV3")) ||
					  (render == QString::fromUtf8("ESV3")) ||
					  (render == QString::fromUtf8("ES3"))) {
				config.render_platform = CONFIG_RENDER_PLATFORM_OPENGL_ES;
				config.render_major_version = 3;
				config.render_minor_version = 1;
			} else if((render == QString::fromUtf8("GL4")) ||
					 (render == QString::fromUtf8("GL43")) ||
					 (render == QString::fromUtf8("GL4.3")) ||
					 (render == QString::fromUtf8("GL4_3")) ||
					 (render == QString::fromUtf8("GL4_CORE"))) {
				config.render_platform = CONFIG_RENDER_PLATFORM_OPENGL_CORE;
				config.render_major_version = 4;
				config.render_minor_version = 3;
			}
		}
	}
	switch(config.render_platform) {
	case CONFIG_RENDER_PLATFORM_OPENGL_MAIN:
	case CONFIG_RENDER_PLATFORM_OPENGL_CORE:
		QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
		QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true); // Enable shared contexts.
		QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, false);
		break;
	case CONFIG_RENDER_PLATFORM_OPENGL_ES:
		QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, false);
		QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true); // Enable shared contexts.
		QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);
		break;
	default: // to GLES 2.1 as Default
		QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, false);
		QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);
		config.render_platform = CONFIG_RENDER_PLATFORM_OPENGL_ES;
		config.render_major_version = 2;
		config.render_minor_version = 1;
		break;
	}

	SetProcCmdFD(cmdparser, _l);
	SetProcCmdHDD(cmdparser, _l);
	SetProcCmdQuickDisk(cmdparser, _l);
	SetProcCmdCmt(cmdparser, _l);
	SetProcCmdCart(cmdparser, _l);
	SetProcCmdBinary(cmdparser, _l);
	SetProcCmdBubble(cmdparser, _l);
	SetProcCmdLD(cmdparser, _l);
	SetProcCmdCD(cmdparser, _l);

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

void OpeningMessage(std::shared_ptr<CSP_Logger>p_logger, std::string archstr)
{
	p_logger->set_emu_vm_name(DEVICE_NAME); // Write to syslog, console
	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Start Common Source Project '%s'", my_procname.c_str());
	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "(C) Toshiya Takeda / Qt Version K.Ohta");
	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Architecture: %s", archstr.c_str());
	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Use -h or --help for help.");

	//p_logger->debug_log(AGAR_LOG_INFO, " -? is print help(s).");
	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Home = %s, Resource directory = %s",
						  cpp_homedir.c_str(),
						  sRssDir.c_str()); // Debug

	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Config dir = %s, config_file = %s",
						  cpp_confdir.c_str(),
						  config_fullpath.c_str()); // Debug

	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "DIPSW VALUE IS 0x%08x", config.dipswitch);
	if(virtualMediaList.size() >= 2) {
		for(int i = 0; i < virtualMediaList.size(); i += 2) {
			p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Virtual media %d, type %s, name %s",
								  i / 2,
								  virtualMediaList.at(i).toLocal8Bit().constData(),
								  virtualMediaList.at(i + 1).toLocal8Bit().constData());
		}
	}
}

void SetupSDL(std::shared_ptr<CSP_Logger>p_logger)
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
					svar = _s.right(_s.length() - (_nl + 1));
				} else {
					skey = _s;
					svar = QString::fromUtf8("");
				}
				SDL_setenv(skey.toLocal8Bit().constData(), svar.toLocal8Bit().constData(), 1);
				p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Note: SDL ENVIROMENT : %s to %s.",
									  skey.toLocal8Bit().constData(),
									  svar.toLocal8Bit().constData());
			}
		}
	}
#if defined(USE_SDL2)
	//SDL_Init(SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
	SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
	//SDL_Init(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS);
#else
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
#endif
	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Audio subsystem was initialised.");
}

extern void DLL_PREFIX_I set_debug_logger(std::shared_ptr<CSP_Logger> p);

void SetupLogger(std::shared_ptr<CSP_Logger> csp_logger, QObject *parent, std::string emustr, int _size)
{

	csp_logger->set_emu_vm_name((const char *)(emustr.c_str()));
	csp_logger->set_log_stdout(CSP_LOG_DEBUG, true);
	csp_logger->set_log_stdout(CSP_LOG_INFO, true);
	csp_logger->set_log_stdout(CSP_LOG_WARN, true);

	csp_logger->set_state_log(0, config.state_log_to_recording);
	csp_logger->set_state_log(1, config.state_log_to_syslog);
	csp_logger->set_state_log(2, config.state_log_to_console);

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

	QApplication *GuiMain = NULL;
	GuiMain = new QApplication(argc, argv);
	GuiMain->setObjectName(QString::fromUtf8("Gui_Main"));
    QCommandLineParser cmdparser;

	virtualMediaList.clear();
	SetOptions(&cmdparser);

	QStringList arglist;
	if(argv != NULL) {
		for(int i = 0; i < argc; i++) {
			if(argv[i] != NULL) {
				arglist.append(QString::fromLocal8Bit(argv[i]));
			}
		}
	}
	archstr = "Generic";
#if defined(__x86_64__)
	archstr = "amd64";
#endif
#if defined(__i386__)
	archstr = "ia32";
#endif
	emustr = emustr + cfgstr;
	std::shared_ptr<USING_FLAGS> using_flags = std::shared_ptr<USING_FLAGS>(new USING_FLAGS_EXT(&config));
	cmdparser.process(arglist);
	ProcessCmdLine(&cmdparser, &virtualMediaList);

	_envvers = QProcessEnvironment::systemEnvironment();

	std::shared_ptr<CSP_Logger>p_logger = std::shared_ptr<CSP_Logger>(new CSP_Logger(GuiMain, config.log_to_syslog, config.log_to_console, emustr.c_str()));
	set_debug_logger(p_logger);
	SetupLogger(p_logger, GuiMain, emustr, CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1);
	OpeningMessage(p_logger, archstr);
	SetupSDL(p_logger);


	/*
	 * Into Qt's Loop.
	 */

	//SetupTranslators();
	QTranslator local_translator;
	QLocale s_locale = QLocale::system();

	if(local_translator.load(s_locale, "machine", ".", ":/", ".qm")) {
		GuiMain->installTranslator(&local_translator);
	}
	QTranslator s_translator;
	if(s_translator.load(s_locale, "gui", ".", ":/", ".qm")) {
		GuiMain->installTranslator(&s_translator);
	}

	QTranslator common_translator;
	if(common_translator.load(s_locale, "common", ".", ":/", ".qm")) {
		GuiMain->installTranslator(&common_translator);
	}
	QTranslator debugger_translator;
	if(debugger_translator.load(s_locale, "debugger", ".", ":/", ".qm")) {
		GuiMain->installTranslator(&debugger_translator);
	}
	//QProcessEnvironment::systemEnvironment() = _envvers;
	if(_b_dump_envver) {
		//QProcessEnvironment ev = QProcessEnvironment::systemEnvironment();
		QProcessEnvironment ev = _envvers;
		QStringList el = _envvers.toStringList();
		if(el.size() > 0) {
			p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "Environment Variables:");
			for(int i = 0; i < el.size(); i++) {
				p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "#%d : %s", i, el.at(i).toLocal8Bit().constData());
			}
		}
	}

//	USING_FLAGS_EXT *using_flags = new USING_FLAGS_EXT(&config);
	// initialize emulation core
	rMainWindow = new META_MainWindow(using_flags, p_logger);
	rMainWindow->connect(rMainWindow, SIGNAL(sig_quit_all(void)), rMainWindow, SLOT(deleteLater(void)));
	rMainWindow->setCoreApplication(GuiMain);
	rMainWindow->getWindow()->show();
	rMainWindow->retranselateUi_Depended_OSD();
//	QMetaObject::connectSlotsByName(rMainWindow);
   	EmuThreadClass *hRunEmu_Real = new EmuThreadClass(rMainWindow, using_flags);
	OSD_BASE* p_osd = hRunEmu_Real->get_emu()->get_osd();

	QObject::connect((OSD*)p_osd, SIGNAL(sig_update_device_node_name(int, const _TCHAR *)),
					 rMainWindow, SLOT(do_update_device_node_name(int, const _TCHAR *)));
	for(int i = 0; i < (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1); i++) {
		rMainWindow->do_update_device_node_name(i, using_flags->get_vm_node_name(i));
	}
	p_logger->set_osd((OSD*)p_osd);
	p_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL, "InitInstance() OK.");

	// ToDo: Update raltime.
	rMainWindow->connect(rMainWindow, SIGNAL(sig_osd_sound_output_device(QString)), (OSD*)p_osd, SLOT(do_set_host_sound_output_device(QString)));
	rMainWindow->do_update_sound_output_list();

	QObject::connect((OSD*)p_osd, SIGNAL(sig_update_sound_output_list()), rMainWindow, SLOT(do_update_sound_output_list()));
	QObject::connect((OSD*)p_osd, SIGNAL(sig_clear_sound_output_list()), rMainWindow, SLOT(do_clear_sound_output_list()));
	QObject::connect((OSD*)p_osd, SIGNAL(sig_append_sound_output_list(QString)), rMainWindow, SLOT(do_append_sound_output_list(QString)));

	QObject::connect(rMainWindow, SIGNAL(sig_update_master_volume(int)), (OSD*)p_osd, SLOT(do_update_master_volume(int)));

	QObject::connect(GuiMain, SIGNAL(lastWindowClosed()),
					 rMainWindow, SLOT(on_actionExit_triggered()));

	QObject::connect((OSD*)p_osd, SIGNAL(sig_clear_keyname_table()),	 rMainWindow, SLOT(do_clear_keyname_table()));
	QObject::connect((OSD*)p_osd, SIGNAL(sig_add_keyname_table(uint32_t, QString)),	 rMainWindow, SLOT(do_add_keyname_table(uint32_t, QString)));
	p_osd->update_keyname_table();

	QObject::connect(rMainWindow, SIGNAL(sig_notify_power_off()), hRunEmu_Real, SLOT(do_notify_power_off()), Qt::QueuedConnection);

	GLDrawClass *pgl = rMainWindow->getGraphicsView();
	pgl->do_set_texture_size(NULL, -1, -1);  // It's very ugly workaround (;_;) 20191028 K.Ohta
//	pgl->setFixedSize(pgl->width(), pgl->height());
	// main loop
	rMainWindow->LaunchEmuThread(hRunEmu_Real);

#if defined(USE_JOYSTICK)
	rMainWindow->LaunchJoyThread();
#endif
	rMainWindow->do_start_emu_thread();
	rMainWindow->do_unblock_task();
	rMainWindow->do_start_draw_thread();
	GuiMain->exec();
	return 0;
}


#ifdef USE_DEBUGGER
#include <../debugger/qt_debugger.h>

void Ui_MainWindow::OnOpenDebugger()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int no = cp->data().value<int>();

	if((no < 0) || (no >= MAX_CPU)) return;
	//emu->open_debugger(no);
	if(hRunEmu == nullptr) return;
	EMU_TEMPLATE *p_emu = hRunEmu->get_emu();

	VM *vm = static_cast<VM*>(p_emu->get_vm());

	// ToDo: Multiple debugger 20221105 K.O
 	if((p_emu->now_debugging ) || (p_emu->hDebugger.get() != nullptr)) /* OnCloseDebugger(); */ return;

	if(!(p_emu->now_debugging && p_emu->debugger_thread_param.cpu_index == no)) {
		//p_emu->close_debugger();
		if(vm->get_cpu(no) != NULL && vm->get_cpu(no)->get_debugger() != NULL) {
			QString windowName = QString::fromUtf8(vm->get_cpu(no)->get_device_name());
			windowName = QString::fromUtf8("Debugger ") + windowName;
			p_emu->hDebugger.reset(new CSP_Debugger(p_emu, this));
			if(p_emu->hDebugger.get() == nullptr) {
				return;
			}
			QString objNameStr = QString("EmuDebugThread");
			p_emu->hDebugger->setObjectName(objNameStr);

			p_emu->hDebugger->debugger_thread_param.osd = (OSD_BASE *)(p_emu->get_osd());
			p_emu->hDebugger->debugger_thread_param.emu = p_emu;
			p_emu->hDebugger->debugger_thread_param.vm = vm;
			p_emu->hDebugger->debugger_thread_param.cpu_index = no;
			p_emu->hDebugger->debugger_thread_param.running = false;
			p_emu->hDebugger->debugger_thread_param.request_terminate = false;

			p_emu->stop_record_sound();
			p_emu->stop_record_video();
			//p_emu->now_debugging = true;
			connect(this, SIGNAL(quit_debugger_thread()), p_emu->hDebugger.get(), SLOT(doExit()));
			connect(this, SIGNAL(destroyed()), p_emu->hDebugger.get(), SLOT(do_destroy_thread()));
			//connect(this, SIGNAL(quit_debugger_thread()), p_emu->hDebugger, SLOT(close()));
			connect(p_emu->hDebugger.get(), SIGNAL(sig_finished()), this, SLOT(OnCloseDebugger()));
			connect(p_emu->hDebugger.get(), SIGNAL(sig_put_string(QString)), p_emu->hDebugger.get(), SLOT(put_string(QString)));
			p_emu->hDebugger->show();
			p_emu->hDebugger->run();
			p_emu->hDebugger->setWindowTitle(windowName);
		}
	}
}

void Ui_MainWindow::OnCloseDebugger(void )
{
	if(hRunEmu == nullptr) return;
	EMU_TEMPLATE *p_emu = hRunEmu->get_emu();
	if(p_emu == nullptr) {
		return;
	}
//	p_emu->close_debugger();
	// ToDo: Multiple debugger 20221105 K.O
 	if((p_emu->now_debugging) && (p_emu->hDebugger.get() != nullptr)) {
		if(p_emu->hDebugger->debugger_thread_param.running) {
			emit quit_debugger_thread();
			//if(!(p_emu->hDebugger->wait(2000))) {
			//	p_emu->hDebugger->terminate();
			//	QThread::msleep(100);
			//}
		}
 	}
	p_emu->hDebugger.reset();
	p_emu->now_debugging = false;
}
#endif

QString Ui_MainWindow::get_system_version()
{
	if(hRunEmu == nullptr) return QString::fromUtf8("");
	EMU_TEMPLATE *p_emu = hRunEmu->get_emu();

	QString guiver = get_gui_version();
	QString aviover;
	QString vm_gitver;
	QString common_vmver;
	QString osdver;
	QString libcommon_ver;
	QString libfmgen_ver;
	QString build_date;

	QString outstr;

	aviover.clear();
	common_vmver.clear();
	vm_gitver.clear();
	osdver.clear();
	libcommon_ver.clear();

	if(hSaveMovieThread != NULL) {
		aviover = hSaveMovieThread->get_avio_version();
	}

	if(p_emu != nullptr) {
		if(p_emu->get_osd() != NULL) {
			_TCHAR *cvp = (_TCHAR *)p_emu->get_osd()->get_lib_common_vm_version();
			_TCHAR *gvp = (_TCHAR *)p_emu->get_osd()->get_lib_common_vm_git_version();
			_TCHAR *ovp = (_TCHAR *)p_emu->get_osd()->get_lib_osd_version();
			if(cvp != NULL) {
				common_vmver = QString::fromUtf8(cvp);
			}
			if(gvp != NULL) {
				vm_gitver = QString::fromUtf8(gvp);
			}
			if(ovp != NULL) {
				osdver = QString::fromUtf8(ovp);
			}
		}
	}

	const _TCHAR *pp = get_lib_common_version();
	if(pp != NULL) {
		libcommon_ver = QString::fromUtf8(pp);
	}
	libfmgen_ver = QString::fromUtf8(FM::get_libfmgen_version());

	outstr.clear();
	outstr.append(QString::fromUtf8("<FONT SIZE=+1>"));
	if(!(common_vmver.isEmpty())) {
		outstr.append(common_vmver);
		outstr.append("<BR>\n");
	}
	if(!(libcommon_ver.isEmpty())) {
		outstr.append(libcommon_ver);
		outstr.append("<BR>\n");
	}
	if(!(osdver.isEmpty())) {
		outstr.append(osdver);
		outstr.append("<BR>\n");
	}
	if(!(libfmgen_ver.isEmpty())) {
		outstr.append(libfmgen_ver);
		outstr.append("<BR>\n");
	}
	if(!(osdver.isEmpty())) {
		outstr.append(osdver);
		outstr.append("<BR>\n");
	}
	if(!(guiver.isEmpty())) {
		outstr.append(guiver);
		outstr.append("<BR>\n");
	}
	if(!(aviover.isEmpty())) {
		outstr.append(aviover);
		outstr.append("<BR>\n");
	}
	outstr.append(QString::fromUtf8("</FONT>"));
	if(!(vm_gitver.isEmpty())) {
		outstr.append("Build Version: ");
		outstr.append(vm_gitver);
		outstr.append("<BR>\n");
	}
	return outstr;
}

QString Ui_MainWindow::get_build_date()
{
#if defined(__BUILD_DATE)
	return QString::fromUtf8(__BUILD_DATE);
#else
	return QString::fromUtf8("");
#endif
}
