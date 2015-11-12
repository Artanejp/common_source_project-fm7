/*
 * UI->Qt->MainWindow : CMT Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */


#include "commonclasses.h"
#include "mainwidget.h"
//#include "menuclasses.h"
#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"

#include "menu_cmt.h"


void Object_Menu_Control::start_insert_play_cmt(void) {
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "%d", play);
	emit sig_insert_play_cmt(play);
}
void Object_Menu_Control::eject_cmt(void) {
	emit sig_eject_cmt();
}
void Object_Menu_Control::on_recent_cmt(){
	emit sig_recent_cmt(s_num);
}
void Object_Menu_Control::do_set_write_protect_cmt(void) {
	 write_protect = true;
	 emit sig_set_write_protect_cmt(write_protect);
}
void Object_Menu_Control::do_unset_write_protect_cmt(void) {
	write_protect = false;
	emit sig_set_write_protect_cmt(write_protect);
}

#if defined(USE_TAPE)
void Ui_MainWindow::open_cmt_dialog(bool play)
{
	QString ext;
	QString desc1;
	QString desc2;
	CSP_DiskDialog dlg;
	QString dirname;
  
#if defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
	ext = "*.wav *.p6 *.cas";
#elif defined(_PC8001SR) || defined(_PC8801MA) || defined(_PC98DO)
	ext = play ? "*.cas *.cmt *.n80 *.t88" : "*.cas *.cmt";
#elif defined(_MZ80A) || defined(_MZ80K) || defined(_MZ1200) || defined(_MZ700) || defined(_MZ800) || defined(_MZ1500)
	ext = play ? "*.wav *.cas *.mzt *.m12 *.t77" :"*.wav *.cas";
#elif defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
	ext = play ? "*.wav *.cas *.mzt *.mti *.mtw *.dat" : "*.wav *.cas";
#elif defined(_X1) || defined(_X1TWIN) || defined(_X1TURBO) || defined(_X1TURBOZ)
	ext = play ? "*.wav *.cas *.tap *.t77" : "*.wav *.cas";
#elif defined(_FM8) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	ext = "*.wav *.t77";
#elif defined(TAPE_BINARY_ONLY)
	ext = "*.cas *.cmt";
#else
	ext = "*.wav *.cas";
#endif
	desc1 = play ? "Data Recorder Tape [Play]" : "Data Recorder Tape [Rec]";
	if(play) {
		dlg.setWindowTitle("Open Tape");
	} else {
		dlg.setWindowTitle("Record Tape");
	}
	desc2 = desc1 + " (" + ext.toLower() + " " + ext.toUpper() + ")";
//	desc1 = desc1 + " (" + ext.toUpper() + ")";
	if(config.initial_tape_dir != NULL) {
		dirname = QString::fromUtf8(config.initial_tape_dir);	        
	} else {
		char app[PATH_MAX];
		QDir df;
		dirname = df.currentPath();
		strncpy(app, dirname.toUtf8().constData(), PATH_MAX);
		dirname = get_parent_dir(app);
	}
	QStringList filter;
	filter << desc2;
	dlg.param->setRecMode(play);
	dlg.setDirectory(dirname);
	dlg.setNameFilters(filter); 
	QObject::connect(&dlg, SIGNAL(fileSelected(QString)), dlg.param, SLOT(_open_cmt(QString))); 
	QObject::connect(dlg.param, SIGNAL(do_open_cmt(bool, QString)), this, SLOT(_open_cmt(bool, QString))); 
	dlg.show();
	dlg.exec();
	return;
}

#endif

void Ui_MainWindow::CreateCMTMenu(void)
{
#if defined(USE_TAPE)
	listCMT.clear();
	//CreateCMTPulldownMenu(p);
#endif // USE_TAPE
}

void Ui_MainWindow::CreateCMTPulldownMenu(void)
{
}


#ifdef USE_TAPE
int Ui_MainWindow::set_recent_cmt(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
	int i;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
    
	s_path = QString::fromUtf8(config.recent_tape_path[num]);
	strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_tape_path, listCMT);
   
	get_parent_dir(path_shadow);
	strcpy(config.initial_tape_dir, path_shadow);
	strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Tape: Open READ");
	
	emit sig_close_tape();
	emit sig_play_tape(s_path);
	menu_CMT->do_update_histories(listCMT);
	menu_CMT->do_set_initialize_directory(config.initial_tape_dir);
	return 0;
}

void Ui_MainWindow::do_write_protect_cmt(int drv, bool flag)
{
	cmt_write_protect = flag;
	//menu_CMT->do_set_write_protect(flag);
}


# ifdef USE_TAPE_BUTTON
void Ui_MainWindow::do_push_play_tape(void)
{
	// Do notify?
	emit sig_cmt_push_play();
}

void Ui_MainWindow::do_push_stop_tape(void)
{
	// Do notify?
	emit sig_cmt_push_stop();
}

void Ui_MainWindow::do_display_tape_play(bool flag)
{
	//if(flag) {
	//	actionPlay_Start->setChecked(true);
	//} else {
	//	actionPlay_Stop->setChecked(true);
	//}
}
 
void Ui_MainWindow::do_push_fast_forward_tape(void)
{
	// Do notify?
	emit sig_cmt_push_fast_forward();
}
void Ui_MainWindow::do_push_rewind_tape(void)
{
	// Do notify?
	emit sig_cmt_push_fast_rewind();
}
void Ui_MainWindow::do_push_apss_forward_tape(void)
{
	// Do notify?
	emit sig_cmt_push_apss_forward();
}
void Ui_MainWindow::do_push_apss_rewind_tape(void)
{
	// Do notify?
	emit sig_cmt_push_apss_rewind();
}
# endif
#endif

#ifdef USE_TAPE
void Ui_MainWindow::set_wave_shaper(bool f)
{
	if(f) {
		config.wave_shaper = 1;
	} else {
		config.wave_shaper = 0;
	}
}

bool Ui_MainWindow::get_wave_shaper(void)
{
	if(config.wave_shaper == 0) return false;
	return true;
}
#endif // USE_TAPE

#ifdef USE_TAPE
void Ui_MainWindow::set_direct_load_from_mzt(bool f)
{
	if(f) {
		config.direct_load_mzt = 1;
	} else {
		config.direct_load_mzt = 0;
	}
}

bool Ui_MainWindow::get_direct_load_mzt(void)
{
	if(config.direct_load_mzt == 0) return false;
	return true;
}
#endif // USE_TAPE

void Ui_MainWindow::_open_cmt(bool mode, const QString path)
{
}

void Ui_MainWindow::eject_cmt(void) 
{
#ifdef USE_TAPE
	emit sig_close_tape();
#endif
}

void Ui_MainWindow::ConfigCMTMenuSub(void)
{
#if defined(USE_TAPE)
	QString ext_play;
	QString ext_rec;
	QString desc_play;
	QString desc_rec;
	
	menu_CMT = new Menu_CMTClass(emu, menubar, "Object_CMT_Menu", this, 0);
	menu_CMT->setObjectName(QString::fromUtf8("menuCMT", -1));
	
	menu_CMT->create_pulldown_menu();	
	// Translate Menu
	SETUP_HISTORY(config.recent_tape_path, listCMT);
	menu_CMT->do_set_write_protect(false);
	menu_CMT->do_update_histories(listCMT);
	menu_CMT->do_set_initialize_directory(config.initial_tape_dir);
	
#if defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
	ext_play = "*.wav *.p6 *.cas";
	ext_rec = "*.wav *.p6 *.cas";
#elif defined(_PC8001SR) || defined(_PC8801MA) || defined(_PC98DO)
	ext_play = "*.cas *.cmt *.n80 *.t88";
	ext_rec  = "*.cas *.cmt";
#elif defined(_MZ80A) || defined(_MZ80K) || defined(_MZ1200) || defined(_MZ700) || defined(_MZ800) || defined(_MZ1500)
	ext_play = "*.wav *.cas *.mzt *.m12 *.t77";
	ext_rec = "*.wav *.cas";
#elif defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
	ext_play = "*.wav *.cas *.mzt *.mti *.mtw *.dat";
	ext_rec =  "*.wav *.cas";
#elif defined(_X1) || defined(_X1TWIN) || defined(_X1TURBO) || defined(_X1TURBOZ)
	ext_play = "*.wav *.cas *.tap *.t77";
	ext_rec =  "*.wav *.cas";
#elif defined(_FM8) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	ext_play = "*.wav *.t77";
	ext_rec = "*.wav *.t77";
#elif defined(TAPE_BINARY_ONLY)
	ext_play = "*.cas *.cmt";
	ext_rec = "*.cas *.cmt";
#else
	ext_play = "*.wav *.cas";
	ext_rec = "*.wav *.cas";
#endif
	desc_play = "Data Recorder Tape [Play]";
	desc_rec  = "Data Recorder Tape [Rec]";

	menu_CMT->do_add_media_extension(ext_play, desc_play);
	menu_CMT->do_add_rec_media_extension(ext_rec, desc_rec);
#endif // USE_TAPE
}

void Ui_MainWindow::do_open_read_cmt(int dummy, QString path) 
{
#ifdef USE_TAPE
	char path_shadow[PATH_MAX];
	int i;

	if(path.length() <= 0) return;
	strncpy(path_shadow, path.toUtf8().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_tape_path, listCMT);
	get_parent_dir(path_shadow);
	strcpy(config.initial_tape_dir, path_shadow);
	// Copy filename again.
	strncpy(path_shadow, path.toUtf8().constData(), PATH_MAX);

	emit sig_close_tape();
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Tape: Open READ : filename = %s", path_shadow);
	emit sig_play_tape(path);
	menu_CMT->do_update_histories(listCMT);
	menu_CMT->do_set_initialize_directory(config.initial_tape_dir);
#endif
}

void Ui_MainWindow::do_open_write_cmt(QString path) 
{
#ifdef USE_TAPE
	char path_shadow[PATH_MAX];
	int i;

	if(path.length() <= 0) return;
	strncpy(path_shadow, path.toUtf8().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, config.recent_tape_path, listCMT);
	get_parent_dir(path_shadow);
	strcpy(config.initial_tape_dir, path_shadow);
	// Copy filename again.
	strncpy(path_shadow, path.toUtf8().constData(), PATH_MAX);

	emit sig_close_tape();
	if(menu_CMT->getWriteProtect() != false) {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "Tape: Open READ : filename = %s", path_shadow);
		emit sig_play_tape(path);
	} else {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "Tape: Open Write : filename = %s", path_shadow);
		emit sig_rec_tape(path);
	}
	menu_CMT->do_update_histories(listCMT);
	menu_CMT->do_set_initialize_directory(config.initial_tape_dir);

#endif
}


void Ui_MainWindow::retranslateCMTMenu(void)
{
#ifdef USE_TAPE
	menu_CMT->retranslateUi();
#endif	
}

void Ui_MainWindow::ConfigCMTMenu(void)
{
#if defined(USE_TAPE)
	ConfigCMTMenuSub(); 
#endif
}
