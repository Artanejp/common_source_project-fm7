/*
 * Qt / Tape Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License : GPLv2
 *   History :
 *     Jan 13 2015 : Start
 */

#include <QApplication>
#include <QStyle>
#include <QActionGroup>

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_cmt.h"

#include "emu_thread_tmpl.h"
#include "qt_dialogs.h"
//#include "emu.h"


Menu_CMTClass::Menu_CMTClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent, int drv, int base_drv) : Menu_MetaClass(root_entry, desc, p, parent, drv, base_drv)
{
	use_write_protect = true;
	use_d88_menus = false;

	icon_cmt = QIcon(":/icon_cmt.png");
	icon_play_start = QApplication::style()->standardIcon(QStyle::SP_MediaPlay);
	icon_play_stop = QApplication::style()->standardIcon(QStyle::SP_MediaStop);
	icon_ff = QApplication::style()->standardIcon(QStyle::SP_MediaSkipForward);
	icon_rew = QApplication::style()->standardIcon(QStyle::SP_MediaSkipBackward);
	icon_apss_forward = QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward);
	icon_apss_backward = QApplication::style()->standardIcon(QStyle::SP_MediaSeekBackward);
	icon_record_to_wav = QIcon(":/icon_record_to_tape.png");
}

Menu_CMTClass::~Menu_CMTClass()
{
}

void Menu_CMTClass::connect_via_emu_thread(EmuThreadClassBase *p)
{
	if(p == nullptr) return;

	connect(action_eject, SIGNAL(triggered()), p, SLOT(do_close_tape()), Qt::QueuedConnection);

	connect(action_play_start, SIGNAL(triggered()), p, SLOT(do_cmt_push_play()), Qt::QueuedConnection);
	connect(action_play_stop, SIGNAL(triggered()), p, SLOT(do_cmt_push_stop()), Qt::QueuedConnection);
	connect(action_fast_forward, SIGNAL(triggered()), p, SLOT(do_cmt_push_fast_forward()), Qt::QueuedConnection);
	connect(action_fast_rewind, SIGNAL(triggered()), p, SLOT(do_cmt_push_fast_rewind()), Qt::QueuedConnection);
	connect(action_apss_forward, SIGNAL(triggered()), p, SLOT(do_cmt_push_apss_forward()), Qt::QueuedConnection);
	connect(action_apss_rewind, SIGNAL(triggered()), p, SLOT(do_cmt_push_apss_rewind()), Qt::QueuedConnection);
	connect(action_wave_shaper, SIGNAL(toggled(bool)), p, SLOT(do_cmt_wave_shaper(bool)), Qt::QueuedConnection);

	if(using_flags->is_machine_cmt_mz_series()) {
		if(action_direct_load_mzt != nullptr) {
			connect(action_direct_load_mzt, SIGNAL(toggled(bool)),
					p, SLOT(do_cmt_direct_load_from_mzt(bool)),
					Qt::QueuedConnection);
		}
	}
}



void Menu_CMTClass::create_pulldown_menu_device_sub(void)
{
	struct CSP_Ui_Menu::DriveIndexPair tmp;
	QVariant _tmp_ins;

	tmp.drive = media_drive;
	tmp.index = 0;
	_tmp_ins.setValue(tmp);

	action_wave_shaper = new QAction(p_wid);
	action_wave_shaper->setVisible(true);
	action_wave_shaper->setCheckable(true);
	action_wave_shaper->setData(_tmp_ins);

	if(using_flags->is_machine_cmt_mz_series()) {
		action_direct_load_mzt = new QAction(p_wid);
		action_direct_load_mzt->setVisible(true);
		action_direct_load_mzt->setCheckable(true);
		action_direct_load_mzt->setData(_tmp_ins);
	}
	action_recording = new QAction(p_wid);
	action_recording->setVisible(true);
	action_recording->setCheckable(false);

	if(p_config->wave_shaper[media_drive] == 0) {
		action_wave_shaper->setChecked(false);
	} else {
		action_wave_shaper->setChecked(true);
	}
	if(using_flags->is_machine_cmt_mz_series()) {
		if(p_config->direct_load_mzt[media_drive] == 0) {
			action_direct_load_mzt->setChecked(false);
		} else {
			action_direct_load_mzt->setChecked(true);
		}
	}
	/*if(using_flags->is_use_tape_button())*/ {
		action_play_start = new QAction(p_wid);
		action_play_start->setVisible(true);
		action_play_start->setCheckable(true);
		action_play_start->setData(_tmp_ins);

		action_play_stop = new QAction(p_wid);
		action_play_stop->setVisible(true);
		action_play_stop->setCheckable(true);
		action_play_stop->setData(_tmp_ins);

		action_fast_forward = new QAction(p_wid);
		action_fast_forward->setVisible(true);
		action_fast_forward->setCheckable(true);
		action_fast_forward->setData(_tmp_ins);

		action_fast_rewind = new QAction(p_wid);
		action_fast_rewind->setVisible(true);
		action_fast_rewind->setCheckable(true);
		action_fast_rewind->setData(_tmp_ins);

		action_apss_forward = new QAction(p_wid);
		action_apss_forward->setVisible(true);
		action_apss_forward->setCheckable(true);
		action_apss_forward->setData(_tmp_ins);

		action_apss_rewind = new QAction(p_wid);
		action_apss_rewind->setVisible(true);
		action_apss_rewind->setCheckable(true);
		action_apss_rewind->setData(_tmp_ins);

		action_group_tape_button = new QActionGroup(p_wid);

		action_group_tape_button->setExclusive(true);
		action_group_tape_button->addAction(action_play_start);
		action_group_tape_button->addAction(action_play_stop);
		action_group_tape_button->addAction(action_fast_forward);
		action_group_tape_button->addAction(action_fast_rewind);
		action_group_tape_button->addAction(action_apss_forward);
		action_group_tape_button->addAction(action_apss_rewind);
	}
}


void Menu_CMTClass::connect_menu_device_sub(void)
{
	if(using_flags->is_use_tape()) {
		this->addSeparator();
		this->addAction(action_recording);
		this->addSeparator();
		/*if(using_flags->is_use_tape_button())*/ {
			this->addAction(action_play_start);
			this->addAction(action_play_stop);
			this->addSeparator();

			this->addAction(action_fast_forward);
			this->addAction(action_fast_rewind);
			this->addSeparator();

			this->addAction(action_apss_forward);
			this->addAction(action_apss_rewind);
			this->addSeparator();
		}
		this->addAction(action_wave_shaper);

		if(using_flags->is_machine_cmt_mz_series()) {
			this->addAction(action_direct_load_mzt);
		}
		connect(action_recording, SIGNAL(triggered()), this, SLOT(do_open_save_dialog()));
		connect(this, SIGNAL(sig_open_media_load(int, QString)),	p_wid, SLOT(do_open_read_cmt(int, QString)));

		connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_cmt(int, int)));
		
		connect(this, SIGNAL(sig_open_media_save(int, QString)), p_wid, SLOT(do_open_write_cmt(int, QString)));
		
		/*if(using_flags->is_use_tape_button())*/ {
		}
	}
}


void Menu_CMTClass::do_open_save_dialog()
{
	QFileDialog* dlgptr = new QFileDialog(nullptr, Qt::Dialog);
	do_open_dialog_common(dlgptr , true);
   
	//dlgptr->setWindowTitle(QApplication::translate("MenuMedia", "Save Tape", 0));
	emit sig_show();
}



void Menu_CMTClass::retranslate_pulldown_menu_device_sub(void)
{
	action_insert->setText(QApplication::translate("MenuMedia", "Insert CMT", 0));
	action_insert->setToolTip(QApplication::translate("MenuMedia", "Insert a TAPE image file.", 0));
	action_eject->setText(QApplication::translate("MenuMedia", "Eject CMT", 0));
	action_eject->setToolTip(QApplication::translate("MenuMedia", "Eject a TAPE image file.", 0));

	action_wave_shaper->setText(QApplication::translate("MenuMedia", "Enable Wave Shaper", 0));
	action_wave_shaper->setToolTip(QApplication::translate("MenuMedia", "Enable wave shaping.\nUseful for some images.", 0));

	if(using_flags->is_machine_cmt_mz_series()) {
		action_direct_load_mzt->setText(QApplication::translate("MenuMedia", "Direct load from MZT", 0));
		action_direct_load_mzt->setToolTip(QApplication::translate("MenuMedia", "Direct loading to memory.\nOnly for MZT image file.", 0));
	}
	this->setTitle(QApplication::translate("MenuMedia", "Cassette Tape" , 0));
	action_insert->setIcon(icon_cmt);

	/*if(using_flags->is_use_tape_button())*/ {
		action_play_start->setIcon(icon_play_start);
		action_play_stop->setIcon(icon_play_stop);
		action_fast_forward->setIcon(icon_ff);
		action_fast_rewind->setIcon(icon_rew);
		action_apss_forward->setIcon(icon_apss_forward);
		action_apss_rewind->setIcon(icon_apss_backward);

		action_play_stop->setText(QApplication::translate("MenuMedia", "Play Stop", 0));
		action_play_start->setText(QApplication::translate("MenuMedia", "Play Start", 0));
		action_fast_forward->setText(QApplication::translate("MenuMedia", "Fast Forward", 0));
		action_fast_rewind->setText(QApplication::translate("MenuMedia", "Rewind", 0));
		action_apss_forward->setText(QApplication::translate("MenuMedia", "APSS Forward", 0));
		action_apss_rewind->setText(QApplication::translate("MenuMedia", "APSS Rewind", 0));
	}
	action_recording->setIcon(icon_record_to_wav);
	action_recording->setText(QApplication::translate("MenuMedia", "Record to a WAV File", 0));
	action_recording->setToolTip(QApplication::translate("MenuMedia", "Record CMT output to a file.", 0));
}
