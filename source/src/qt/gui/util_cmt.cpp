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
#include "dock_disks.h"

#include "menu_cmt.h"
#include "menu_flags.h"
#include "osdcall_types.h"

void Ui_MainWindowBase::CreateCMTMenu(int drive, int base_drv)
{
	QString ext_play;
	QString ext_rec;
	QString desc_play;
	QString desc_rec;
	std::shared_ptr<USING_FLAGS>up = using_flags;
	if(up.get() == nullptr) return;

	listCMT[drive].clear();
	menu_CMT[drive] = new Menu_CMTClass(menubar, QString::fromUtf8("CMT"), using_flags, this, drive, base_drv);
	menu_CMT[drive]->setObjectName(QString::fromUtf8("menuCMT", -1));

	menu_CMT[drive]->create_pulldown_menu();
	// Translate Menu
	SETUP_HISTORY(p_config->recent_tape_path[drive], listCMT[drive]);
	menu_CMT[drive]->do_set_write_protect(false);
	menu_CMT[drive]->do_update_histories(listCMT[drive]);
	menu_CMT[drive]->do_set_initialize_directory(p_config->initial_tape_dir);

	if(up->is_machine_pc6001_variants()) {
		ext_play = "*.wav  *.cas *.p6 *.p6t *.gz";
		ext_rec = "*.wav  *.cas *.p6 *.p6t";
	} else if(up->is_machine_pc8001_variants()) {
		ext_play = "*.cas *.cmt *.n80 *.t88 *.gz";
		ext_rec  = "*.cas *.cmt";
	} else if(up->is_machine_mz80a_variants()) {
		ext_play = "*.wav *.cas *.mzt *.mzf *.m12 *.gz";
		ext_rec = "*.wav *.cas";
	} else if(up->is_machine_mz80b_variants()) {
		if(up->is_machine_mz2500()) {
			ext_play = "*.wav *.cas *.mzt *.mzf *.mti *.gz";
			ext_rec =  "*.wav *.cas";
		} else {
			ext_play = "*.wav *.cas *.mzt *.mzf *.mti *.mtw *.dat *.gz";
			ext_rec =  "*.wav *.cas";
		}
	} else if(up->is_machine_x1_series()) {
		ext_play = "*.wav *.cas *.tap *.t77 *.gz";
		ext_rec =  "*.wav *.cas *.t77";
	} else if(up->is_machine_fm7_series()) {
		ext_play = "*.wav *.t77 *.gz";
		ext_rec = "*.wav *.t77";
	} else if(up->is_machine_basicmaster_variants()) {
		ext_play = "*.wav *.bin *.gz";
		ext_rec = "*.wav";
	} else if(up->is_tape_binary_only()) {
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

	if(using_flags.get() == nullptr) return -1;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p->get_max_tape() <= drv) return -1;
	if(drv < 0) return -1;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;

	s_path = QString::fromLocal8Bit(p_config->recent_tape_path[drv][num]);
	if(!(s_path.isEmpty())) {
		do_open_read_cmt(drv, s_path);
		return 0;
	}
	return -1;
}


bool Ui_MainWindowBase::get_wave_shaper(int drive)
{
	if(p_config->wave_shaper[drive] == 0) return false;
	return true;
}

bool Ui_MainWindowBase::get_direct_load_mzt(int drive)
{
	if(p_config->direct_load_mzt[drive] == 0) return false;
	return true;
}


void Ui_MainWindowBase::do_open_read_cmt(int drive, QString path)
{
	if(using_flags.get() == nullptr) return;
	if(path.length() <= 0) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p->get_max_tape() <= drive) return;
	if(drive < 0) return;

	emit sig_close_tape(drive);

	if(!(FILEIO::IsFileExisting(path.toLocal8Bit().constData()))) return;
	emit sig_play_tape(drive, path);
}

void Ui_MainWindowBase::do_open_write_cmt(int drive, QString path)
{
	if(using_flags.get() == nullptr) return;
	if(path.length() <= 0) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p->get_max_tape() <= drive) return;
	if(drive < 0) return;

	if(menu_CMT[drive] == nullptr) return;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	my_strncpy_s(path_shadow, _MAX_PATH , path.toLocal8Bit().constData(), _TRUNCATE);

	emit sig_close_tape(drive);
	if(!(FILEIO::IsFileExisting(path_shadow))) return;

	if(menu_CMT[drive]->getWriteProtect() != false) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_VFILE_CMT + 0, "Open READ : filename = %s", path_shadow);
		emit sig_play_tape(drive, path);
	} else {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_VFILE_CMT + 0, "Open Write : filename = %s", path_shadow);
		emit sig_rec_tape(drive, path);
	}
//	menu_CMT[drive]->do_update_histories(listCMT[drive]);
//	menu_CMT[drive]->do_set_initialize_directory(p_config->initial_tape_dir);
}

void Ui_MainWindowBase::do_ui_tape_play_insert_history(int drv, QString fname)
{
	if(fname.length() <= 0) return;
	std::shared_ptr<USING_FLAGS>up = using_flags;
	if(up.get() == nullptr) return;
	if(up->get_max_tape() <= drv) return;

	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, fname.toLocal8Bit().constData(), _MAX_PATH - 1);
	if(!(FILEIO::IsFileExisting(path_shadow))) return;

	UPDATE_HISTORY(fname, p_config->recent_tape_path[drv], listCMT[drv]);
	strcpy(p_config->initial_tape_dir, 	get_parent_dir((const _TCHAR *)path_shadow));
	// Update List
	if(menu_CMT[drv] != nullptr) {
		menu_CMT[drv]->do_set_initialize_directory(p_config->initial_tape_dir);
		menu_CMT[drv]->do_update_histories(listCMT[drv]);
	}
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_CMT, drv, fname);
	}
}
void Ui_MainWindowBase::do_ui_tape_record_insert_history(int drv, QString fname)
{
	do_ui_tape_play_insert_history(drv, fname);
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_CMT, drv, fname);
	}
}

void Ui_MainWindowBase::do_ui_write_protect_tape(int drive, quint64 flag)
{
	if(drive < 0) return;

	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_tape()) || (p->get_max_tape() <= drive)) return;
	if(menu_CMT[drive] == nullptr) return;

	if((flag & EMU_MESSAGE_TYPE::WRITE_PROTECT) != 0) {
		menu_CMT[drive]->do_set_write_protect(true);
	} else {
		menu_CMT[drive]->do_set_write_protect(false);
	}
}

void Ui_MainWindowBase::do_ui_eject_tape(int drive)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_tape()) || (p->get_max_tape() <= drive)) return;
	if(menu_CMT[drive] != nullptr) return;

	menu_CMT[drive]->do_clear_inner_media();
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_CMT, drive, QString::fromUtf8(""));
	}
}
void Ui_MainWindowBase::retranslateCMTMenu(int drive)
{
	std::shared_ptr<USING_FLAGS>up = using_flags;
	if(up.get() == nullptr) return;
	if(up->is_use_tape() && (up->get_max_tape() > drive)) {
		menu_CMT[drive]->retranslateUi();
	}
}

void Ui_MainWindowBase::ConfigCMTMenu(void)
{
}
