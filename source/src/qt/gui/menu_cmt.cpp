/*
 * Qt / Tape Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License : GPLv2
 *   History : 
 *     Jan 13 2015 : Start
 */

#include "commonclasses.h"
#include "mainwidget.h"
#include "menu_cmt.h"

#include "qt_dialogs.h"
#include "emu.h"


Menu_CMTClass::Menu_CMTClass(EMU *ep, QMenuBar *root_entry, QString desc, QWidget *parent, int drv) : Menu_MetaClass(ep, root_entry, desc, parent, drv)
{
	use_write_protect = true;
	use_d88_menus = false;

	ext_rec_filter.clear();
}

Menu_CMTClass::~Menu_CMTClass()
{
}

void Menu_CMTClass::create_pulldown_menu_device_sub(void)
{
#ifdef USE_TAPE
	action_wave_shaper = new Action_Control(p_wid);
	action_wave_shaper->setVisible(true);
	action_wave_shaper->setCheckable(true);

	action_direct_load_mzt = new Action_Control(p_wid);
	action_direct_load_mzt->setVisible(true);
	action_direct_load_mzt->setCheckable(true);

	action_recording = new Action_Control(p_wid);
	action_recording->setVisible(true);
	action_recording->setCheckable(false);

	if(config.wave_shaper == 0) {
		action_wave_shaper->setChecked(false);
	} else {
		action_wave_shaper->setChecked(true);
	}
	if(config.direct_load_mzt == 0) {
		action_direct_load_mzt->setChecked(false);
	} else {
		action_direct_load_mzt->setChecked(true);
	}
# if defined(USE_TAPE_BUTTON)
	action_play_start = new Action_Control(p_wid);
	action_play_start->setVisible(true);
	action_play_start->setCheckable(true);

	action_play_stop = new Action_Control(p_wid);
	action_play_stop->setVisible(true);
	action_play_stop->setCheckable(true);

	action_fast_forward = new Action_Control(p_wid);
	action_fast_forward->setVisible(true);
	action_fast_forward->setCheckable(true);

	action_fast_rewind = new Action_Control(p_wid);
	action_fast_rewind->setVisible(true);
	action_fast_rewind->setCheckable(true);

	action_apss_forward = new Action_Control(p_wid);
	action_apss_forward->setVisible(true);
	action_apss_forward->setCheckable(true);

	action_apss_rewind = new Action_Control(p_wid);
	action_apss_rewind->setVisible(true);
	action_apss_rewind->setCheckable(true);

	action_group_tape_button = new QActionGroup(p_wid);

	action_group_tape_button->setExclusive(true);
	action_group_tape_button->addAction(action_play_start);
	action_group_tape_button->addAction(action_play_stop);
	action_group_tape_button->addAction(action_fast_forward);
	action_group_tape_button->addAction(action_fast_rewind);
	action_group_tape_button->addAction(action_apss_forward);
	action_group_tape_button->addAction(action_apss_rewind);
# endif
#endif
}


void Menu_CMTClass::connect_menu_device_sub(void)
{
#ifdef USE_TAPE
	this->addSeparator();
	this->addAction(action_recording);
	this->addSeparator();
#if defined(USE_TAPE_BUTTON)
	this->addAction(action_play_start);
	this->addAction(action_play_stop);
	this->addSeparator();
	
	this->addAction(action_fast_forward);
	this->addAction(action_fast_rewind);
	this->addSeparator();
	
	this->addAction(action_apss_forward);
	this->addAction(action_apss_rewind);
	this->addSeparator();
#endif
	this->addAction(action_wave_shaper);
	this->addAction(action_direct_load_mzt);
	
	connect(action_direct_load_mzt, SIGNAL(toggled(bool)),
			p_wid, SLOT(set_direct_load_from_mzt(bool)));
	connect(action_wave_shaper, SIGNAL(toggled(bool)),
			p_wid, SLOT(set_wave_shaper(bool)));
	
	connect(action_recording, SIGNAL(triggered()),
			this, SLOT(do_open_rec_dialog()));
	connect(this, SIGNAL(sig_open_media(int, QString)),
			p_wid, SLOT(do_open_read_cmt(int, QString)));

	connect(this, SIGNAL(sig_eject_media(int)),
			this, SLOT(do_eject_cmt(int)));
	connect(this, SIGNAL(sig_close_tape()),
			p_wid, SLOT(eject_cmt()));
   
	connect(this, SIGNAL(sig_write_protect_media(int, bool)), p_wid, SLOT(do_write_protect_cmt(int, bool)));	
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_cmt(int, int)));

#if defined(USE_TAPE_BUTTON)
	connect(action_play_start, SIGNAL(triggered()), p_wid, SLOT(do_push_play_tape(void)));
	connect(action_play_stop,  SIGNAL(triggered()), p_wid, SLOT(do_push_stop_tape(void)));
	connect(action_fast_forward,  SIGNAL(triggered()), p_wid, SLOT(do_push_fast_forward_tape(void)));
	connect(action_fast_rewind,   SIGNAL(triggered()), p_wid, SLOT(do_push_rewind_tape(void)));
	connect(action_apss_forward,  SIGNAL(triggered()), p_wid, SLOT(do_push_apss_forward_tape(void)));
	connect(action_apss_rewind,   SIGNAL(triggered()), p_wid, SLOT(do_push_apss_rewind_tape(void)));
#endif	
#endif	
}

void Menu_CMTClass::do_add_rec_media_extension(QString ext, QString description)
{
	QString tmps = description;
	QString all = QString::fromUtf8("All Files (*.*)");

	tmps.append(QString::fromUtf8(" ("));
	tmps.append(ext.toLower());
	tmps.append(QString::fromUtf8(" "));
	tmps.append(ext.toUpper());
	tmps.append(QString::fromUtf8(")"));

	ext_rec_filter << tmps;
	ext_rec_filter << all;
	
	ext_rec_filter.removeDuplicates();
}

void Menu_CMTClass::do_open_rec_dialog()
{
#ifdef USE_TAPE
	CSP_DiskDialog dlg;
	
	if(initial_dir.isEmpty()) { 
		QDir dir;
		char app[PATH_MAX];
		initial_dir = dir.currentPath();
		strncpy(app, initial_dir.toLocal8Bit().constData(), PATH_MAX);
		initial_dir = QString::fromLocal8Bit(get_parent_dir(app));
	}
	dlg.setOption(QFileDialog::ReadOnly, false);
	dlg.setOption(QFileDialog::DontUseNativeDialog, true);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.param->setDrive(media_drive);
	dlg.setDirectory(initial_dir);
	dlg.setNameFilters(ext_rec_filter);
	dlg.setWindowTitle(desc_rec);
	dlg.setWindowTitle(QApplication::translate("MainWindow", "Save Tape", 0));
	QObject::connect(&dlg, SIGNAL(fileSelected(QString)),
					 p_wid, SLOT(do_open_write_cmt(QString))); 
	dlg.show();
	dlg.exec();
	return;
#endif
}

void Menu_CMTClass::do_eject_cmt(int dummy) 
{
#ifdef USE_TAPE
	emit sig_close_tape();
#endif
}

void Menu_CMTClass::retranslate_pulldown_menu_device_sub(void)
{
#ifdef USE_TAPE	
	action_insert->setText(QApplication::translate("MainWindow", "Insert CMT", 0));
	action_eject->setText(QApplication::translate("MainWindow", "Eject CMT", 0));

	action_wave_shaper->setText(QApplication::translate("MainWindow", "Enable Wave Shaper", 0));
	action_direct_load_mzt->setText(QApplication::translate("MainWindow", "Direct load from MZT", 0));
  
	this->setTitle(QApplication::translate("MainWindow", "Cassette Tape" , 0));

#ifdef USE_TAPE_BUTTON
	action_play_stop->setText(QApplication::translate("MainWindow", "Play Stop", 0));
	action_play_start->setText(QApplication::translate("MainWindow", "Play Start", 0));
	action_fast_forward->setText(QApplication::translate("MainWindow", "Fast Forward", 0));
	action_fast_rewind->setText(QApplication::translate("MainWindow", "Rewind", 0));
	action_apss_forward->setText(QApplication::translate("MainWindow", "APSS Forward", 0));
	action_apss_rewind->setText(QApplication::translate("MainWindow", "APSS Rewind", 0));
#endif
	action_recording->setText(QApplication::translate("MainWindow", "Record to a WAV File", 0));
#endif
}
