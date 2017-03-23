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
#include "csp_logger.h"

#include "menu_cmt.h"
#include "menu_flags.h"

//extern USING_FLAGS *using_flags;
void Ui_MainWindowBase::CreateCMTMenu(int drive)
{
	QString ext_play;
	QString ext_rec;
	QString desc_play;
	QString desc_rec;
	
	listCMT[drive].clear();
	menu_CMT[drive] = new Menu_CMTClass(menubar, "Object_CMT_Menu", using_flags, this, drive);
	menu_CMT[drive]->setObjectName(QString::fromUtf8("menuCMT", -1));
	
	menu_CMT[drive]->create_pulldown_menu();	
	// Translate Menu
	SETUP_HISTORY(using_flags->get_config_ptr()->recent_tape_path[drive], listCMT[drive]);
	menu_CMT[drive]->do_set_write_protect(false);
	menu_CMT[drive]->do_update_histories(listCMT[drive]);
	menu_CMT[drive]->do_set_initialize_directory(using_flags->get_config_ptr()->initial_tape_dir);

	if(using_flags->is_machine_pc6001()) {
		ext_play = "*.wav *.p6 *.cas *.gz";
		ext_rec = "*.wav *.p6 *.cas";
	} else if(using_flags->is_machine_pc8001_variants()) {
		ext_play = "*.cas *.cmt *.n80 *.t88 *.gz";
		ext_rec  = "*.cas *.cmt";
	} else if(using_flags->is_machine_mz80a_variants()) {
		ext_play = "*.wav *.cas *.mzt *.mzf *.m12 *.t77 *.gz";
		ext_rec = "*.wav *.cas *.t77";
	} else if(using_flags->is_machine_mz80b_variants()) {
		ext_play = "*.wav *.cas *.mzt *.mzf *.mti *.mtw *.dat *.gz";
		ext_rec =  "*.wav *.cas";
	} else if(using_flags->is_machine_x1_series()) {
		ext_play = "*.wav *.cas *.tap *.t77 *.gz";
		ext_rec =  "*.wav *.cas *.t77";
	} else if(using_flags->is_machine_fm7_series()) {
		ext_play = "*.wav *.t77 *.gz";
		ext_rec = "*.wav *.t77";
	} else if(using_flags->is_machine_basicmaster_variants()) {
		ext_play = "*.wav *.bin *.gz";
		ext_rec = "*.wav";
	} else if(using_flags->is_tape_binary_only()) {
		ext_play = "*.cas *.cmt *.gz";
		ext_rec = "*.cas *.cmt";
	} else {
		ext_play = "*.wav *.cas *.gz";
		ext_rec = "*.wav *.cas";
	}
	desc_play = "Data Recorder Tape [Play]";
	desc_rec  = "Data Recorder Tape [Rec]";

	menu_CMT[drive]->do_add_media_extension(ext_play, desc_play);
	menu_CMT[drive]->do_add_rec_media_extension(ext_rec, desc_rec);
}

int Ui_MainWindowBase::set_recent_cmt(int drv, int num) 
{
	QString s_path;
	char path_shadow[PATH_MAX];

	if((num < 0) || (num >= MAX_HISTORY)) return -1;
    
	s_path = QString::fromLocal8Bit(using_flags->get_config_ptr()->recent_tape_path[drv][num]);
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, using_flags->get_config_ptr()->recent_tape_path[drv], listCMT[drv]);
   
	strcpy(using_flags->get_config_ptr()->initial_tape_dir, get_parent_dir(path_shadow));
	strncpy(path_shadow, s_path.toLocal8Bit().constData(), PATH_MAX);
	csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_VFILE_CMT + 0, "Open READ (from history) : filename = %s", s_path.toLocal8Bit().constData());
	
	emit sig_close_tape(drv);
	emit sig_play_tape(drv, s_path);
	menu_CMT[drv]->do_update_histories(listCMT[drv]);
	menu_CMT[drv]->do_set_initialize_directory(using_flags->get_config_ptr()->initial_tape_dir);
	return 0;
}

void Ui_MainWindowBase::do_write_protect_cmt(int drv, bool flag)
{
	//cmt_write_protect[drv] = flag;
}


void Ui_MainWindowBase::do_push_play_tape(int drive)
{
	// Do notify?
	emit sig_cmt_push_play(drive);
}

void Ui_MainWindowBase::do_push_stop_tape(int drive)
{
	// Do notify?
	emit sig_cmt_push_stop(drive);
}

void Ui_MainWindowBase::do_push_fast_forward_tape(int drive)
{
	// Do notify?
	emit sig_cmt_push_fast_forward(drive);
}
void Ui_MainWindowBase::do_push_rewind_tape(int drive)
{
	// Do notify?
	emit sig_cmt_push_fast_rewind(drive);
}
void Ui_MainWindowBase::do_push_apss_forward_tape(int drive)
{
	// Do notify?
	emit sig_cmt_push_apss_forward(drive);
}
void Ui_MainWindowBase::do_push_apss_rewind_tape(int drive)
{
	// Do notify?
	emit sig_cmt_push_apss_rewind(drive);
}

void Ui_MainWindowBase::set_wave_shaper(int drive, bool f)
{
	if(f) {
		using_flags->get_config_ptr()->wave_shaper[drive] = 1;
	} else {
		using_flags->get_config_ptr()->wave_shaper[drive] = 0;
	}
}

bool Ui_MainWindowBase::get_wave_shaper(int drive)
{
	if(using_flags->get_config_ptr()->wave_shaper[drive] == 0) return false;
	return true;
}

void Ui_MainWindowBase::set_direct_load_from_mzt(int drive, bool f)
{
	if(f) {
		using_flags->get_config_ptr()->direct_load_mzt[drive] = 1;
	} else {
		using_flags->get_config_ptr()->direct_load_mzt[drive] = 0;
	}
}

bool Ui_MainWindowBase::get_direct_load_mzt(int drive)
{
	if(using_flags->get_config_ptr()->direct_load_mzt[drive] == 0) return false;
	return true;
}

void Ui_MainWindowBase::eject_cmt(int drv) 
{
	emit sig_close_tape(drv);
}

void Ui_MainWindowBase::do_open_read_cmt(int drive, QString path) 
{
	char path_shadow[PATH_MAX];

	if(path.length() <= 0) return;
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, using_flags->get_config_ptr()->recent_tape_path[drive], listCMT[drive]);
	strcpy(using_flags->get_config_ptr()->initial_tape_dir, get_parent_dir(path_shadow));
	// Copy filename again.
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX);

	emit sig_close_tape(drive);
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_VFILE_CMT + 0, "Open READ : filename = %s", path_shadow);
	emit sig_play_tape(drive, path);
	menu_CMT[drive]->do_update_histories(listCMT[drive]);
	menu_CMT[drive]->do_set_initialize_directory(using_flags->get_config_ptr()->initial_tape_dir);
}

void Ui_MainWindowBase::do_open_write_cmt(int drive, QString path) 
{
	char path_shadow[PATH_MAX];

	if(path.length() <= 0) return;
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX);
	UPDATE_HISTORY(path_shadow, using_flags->get_config_ptr()->recent_tape_path[drive], listCMT[drive]);
	get_parent_dir(path_shadow);
	strcpy(using_flags->get_config_ptr()->initial_tape_dir,	get_parent_dir(path_shadow));
	// Copy filename again.
	strncpy(path_shadow, path.toLocal8Bit().constData(), PATH_MAX);

	emit sig_close_tape(drive);
	if(menu_CMT[drive]->getWriteProtect() != false) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_VFILE_CMT + 0, "Open READ : filename = %s", path_shadow);
		emit sig_play_tape(drive, path);
	} else {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_VFILE_CMT + 0, "Open Write : filename = %s", path_shadow);
		emit sig_rec_tape(drive, path);
	}
	menu_CMT[drive]->do_update_histories(listCMT[drive]);
	menu_CMT[drive]->do_set_initialize_directory(using_flags->get_config_ptr()->initial_tape_dir);
}


void Ui_MainWindowBase::retranslateCMTMenu(int drive)
{
	if(using_flags->is_use_tape() && (using_flags->get_max_tape() > drive)) {
		menu_CMT[drive]->retranslateUi();
	}
}

void Ui_MainWindowBase::ConfigCMTMenu(void)
{
}
