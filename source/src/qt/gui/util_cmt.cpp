/*
 * UI->Qt->MainWindow : CMT Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */


#include "commonclasses.h"
#include "mainwidget_base.h"
#include "qt_dialogs.h"
#include "agar_logger.h"

#include "menu_cmt.h"
#include "menu_flags.h"

//extern USING_FLAGS *using_flags;
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


void Ui_MainWindowBase::CreateCMTMenu(void)
{
	QString ext_play;
	QString ext_rec;
	QString desc_play;
	QString desc_rec;
	
	listCMT.clear();
	menu_CMT = new Menu_CMTClass(emu, menubar, "Object_CMT_Menu", using_flags, this, 0);
	menu_CMT->setObjectName(QString::fromUtf8("menuCMT", -1));
	
	menu_CMT->create_pulldown_menu();	
	// Translate Menu
	SETUP_HISTORY(using_flags->get_config_ptr()->recent_tape_path, listCMT);
	menu_CMT->do_set_write_protect(false);
	menu_CMT->do_update_histories(listCMT);
	menu_CMT->do_set_initialize_directory(using_flags->get_config_ptr()->initial_tape_dir);

	if(using_flags->is_machine_pc6001()) {
		ext_play = "*.wav *.p6 *.cas";
		ext_rec = "*.wav *.p6 *.cas";
	} else if(using_flags->is_machine_pc8001_variants()) {
		ext_play = "*.cas *.cmt *.n80 *.t88";
		ext_rec  = "*.cas *.cmt";
	} else if(using_flags->is_machine_mz80a_variants()) {
		ext_play = "*.wav *.cas *.mzt *.m12 *.t77";
		ext_rec = "*.wav *.cas";
	} else if(using_flags->is_machine_mz80b_variants()) {
		ext_play = "*.wav *.cas *.mzt *.mti *.mtw *.dat";
		ext_rec =  "*.wav *.cas";
	} else if(using_flags->is_machine_x1_series()) {
		ext_play = "*.wav *.cas *.tap *.t77";
		ext_rec =  "*.wav *.cas";
	} else if(using_flags->is_machine_fm7_series()) {
		ext_play = "*.wav *.t77";
		ext_rec = "*.wav *.t77";
	} else if(using_flags->is_tape_binary_only()) {
		ext_play = "*.cas *.cmt";
		ext_rec = "*.cas *.cmt";
	} else {
		ext_play = "*.wav *.cas";
		ext_rec = "*.wav *.cas";
	}
	desc_play = "Data Recorder Tape [Play]";
	desc_rec  = "Data Recorder Tape [Rec]";

	menu_CMT->do_add_media_extension(ext_play, desc_play);
	menu_CMT->do_add_rec_media_extension(ext_rec, desc_rec);
}

void Ui_MainWindowBase::CreateCMTPulldownMenu(void)
{
}


int Ui_MainWindowBase::set_recent_cmt(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];
	int i;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
    
	s_path = QString::fromLocal8Bit(using_flags->get_config_ptr()->recent_tape_path[num]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, using_flags->get_config_ptr()->recent_tape_path, listCMT);
   
	get_parent_dir(path_shadow);
	strcpy(using_flags->get_config_ptr()->initial_tape_dir, path_shadow);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_VFILE_CMT + 0, "Open READ");
	
	emit sig_close_tape();
	emit sig_play_tape(s_path);
	menu_CMT->do_update_histories(listCMT);
	menu_CMT->do_set_initialize_directory(using_flags->get_config_ptr()->initial_tape_dir);
	return 0;
}

void Ui_MainWindowBase::do_write_protect_cmt(int drv, bool flag)
{
	cmt_write_protect = flag;
	//menu_CMT->do_set_write_protect(flag);
}


void Ui_MainWindowBase::do_push_play_tape(void)
{
	// Do notify?
	emit sig_cmt_push_play();
}

void Ui_MainWindowBase::do_push_stop_tape(void)
{
	// Do notify?
	emit sig_cmt_push_stop();
}

void Ui_MainWindowBase::do_display_tape_play(bool flag)
{
	//if(flag) {
	//	actionPlay_Start->setChecked(true);
	//} else {
	//	actionPlay_Stop->setChecked(true);
	//}
}
 
void Ui_MainWindowBase::do_push_fast_forward_tape(void)
{
	// Do notify?
	emit sig_cmt_push_fast_forward();
}
void Ui_MainWindowBase::do_push_rewind_tape(void)
{
	// Do notify?
	emit sig_cmt_push_fast_rewind();
}
void Ui_MainWindowBase::do_push_apss_forward_tape(void)
{
	// Do notify?
	emit sig_cmt_push_apss_forward();
}
void Ui_MainWindowBase::do_push_apss_rewind_tape(void)
{
	// Do notify?
	emit sig_cmt_push_apss_rewind();
}

void Ui_MainWindowBase::set_wave_shaper(bool f)
{
	if(f) {
		using_flags->get_config_ptr()->wave_shaper = 1;
	} else {
		using_flags->get_config_ptr()->wave_shaper = 0;
	}
}

bool Ui_MainWindowBase::get_wave_shaper(void)
{
	if(using_flags->get_config_ptr()->wave_shaper == 0) return false;
	return true;
}

void Ui_MainWindowBase::set_direct_load_from_mzt(bool f)
{
	if(f) {
		using_flags->get_config_ptr()->direct_load_mzt = 1;
	} else {
		using_flags->get_config_ptr()->direct_load_mzt = 0;
	}
}

bool Ui_MainWindowBase::get_direct_load_mzt(void)
{
	if(using_flags->get_config_ptr()->direct_load_mzt == 0) return false;
	return true;
}

void Ui_MainWindowBase::eject_cmt(void) 
{
	emit sig_close_tape();
}

void Ui_MainWindowBase::ConfigCMTMenuSub(void)
{
}

void Ui_MainWindowBase::do_open_read_cmt(int dummy, QString path) 
{
	char path_shadow[PATH_MAX];
	int i;

	if(path.length() <= 0) return;
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, using_flags->get_config_ptr()->recent_tape_path, listCMT);
	get_parent_dir(path_shadow);
	strcpy(using_flags->get_config_ptr()->initial_tape_dir, path_shadow);
	// Copy filename again.
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX);

	emit sig_close_tape();
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_VFILE_CMT + 0, "Open READ : filename = %s", path_shadow);
	emit sig_play_tape(path);
	menu_CMT->do_update_histories(listCMT);
	menu_CMT->do_set_initialize_directory(using_flags->get_config_ptr()->initial_tape_dir);
}

void Ui_MainWindowBase::do_open_write_cmt(QString path) 
{
	char path_shadow[PATH_MAX];
	int i;

	if(path.length() <= 0) return;
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, using_flags->get_config_ptr()->recent_tape_path, listCMT);
	get_parent_dir(path_shadow);
	strcpy(using_flags->get_config_ptr()->initial_tape_dir, path_shadow);
	// Copy filename again.
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX);

	emit sig_close_tape();
	if(menu_CMT->getWriteProtect() != false) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_VFILE_CMT + 0, "Open READ : filename = %s", path_shadow);
		emit sig_play_tape(path);
	} else {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_VFILE_CMT + 0, "Open Write : filename = %s", path_shadow);
		emit sig_rec_tape(path);
	}
	menu_CMT->do_update_histories(listCMT);
	menu_CMT->do_set_initialize_directory(using_flags->get_config_ptr()->initial_tape_dir);
}


void Ui_MainWindowBase::retranslateCMTMenu(void)
{
	if(using_flags->is_use_tape()) {
		menu_CMT->retranslateUi();
	}
}

void Ui_MainWindowBase::ConfigCMTMenu(void)
{
	ConfigCMTMenuSub(); 
}
