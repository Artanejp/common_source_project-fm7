/*
 * Qt / Tape Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License : GPLv2
 *   History : 
 *     Jan 13 2015 : Start
 */

#include "commonclasses.h"
#include "mainwidget.h"
#include "menu_binary.h"

#include "qt_dialogs.h"
#include "emu.h"


Menu_BinaryClass::Menu_BinaryClass(EMU *ep, QMenuBar *root_entry, QString desc, QWidget *parent, int drv) : Menu_MetaClass(ep, root_entry, desc, parent, drv)
{
	use_write_protect = false;
	use_d88_menus = false;
}

Menu_BinaryClass::~Menu_BinaryClass()
{
}

void Menu_BinaryClass::create_pulldown_menu_device_sub(void)
{
	int ii;
	action_saving = new Action_Control(p_wid);
	action_saving->setVisible(true);
	action_saving->setCheckable(false);

	//menu_history_save = new QMenu(this);
	//menu_history_save->setObjectName(QString::fromUtf8("menu_history_save_") + object_desc);

	{
		QString tmps;
		action_group_save_recent = new QActionGroup(p_wid);
		action_group_save_recent->setExclusive(true);
		
		for(ii = 0; ii < MAX_HISTORY; ii++) {
			tmps = history.value(ii, "");
			action_recent_save_list[ii] = new Action_Control(p_wid);
			action_recent_save_list[ii]->binds->setDrive(media_drive);
			action_recent_save_list[ii]->binds->setNumber(ii);
			
			action_recent_save_list[ii]->setText(tmps);
			action_group_save_recent->addAction(action_recent_save_list[ii]);
			if(!tmps.isEmpty()) {
				action_recent_save_list[ii]->setVisible(true);
			} else {
				action_recent_save_list[ii]->setVisible(false);
			}			
		}
	}

	//for(ii = 0; ii < MAX_HISTORY; ii++) menu_history_save->addAction(action_recent_save_list[ii]);
}

void Menu_BinaryClass::do_open_media_save(int drv, QString name) {
	emit sig_open_media_save(drv, name);
}

void Menu_BinaryClass::do_open_recent_media_save(int drv, int slot) {
	emit sig_set_recent_media_save(drv, slot);
}

void Menu_BinaryClass::do_update_histories(QStringList lst)
{
	int ii;
	QString tmps;
	
	Menu_MetaClass::do_update_histories(lst);
	
	for(ii = 0; ii < MAX_HISTORY; ii++) {
		tmps = QString::fromUtf8("");
		if(ii < lst.size()) tmps = lst.value(ii);
		action_recent_save_list[ii]->setText(tmps);
		if(!tmps.isEmpty()) {
			action_recent_save_list[ii]->setVisible(true);
		} else {
			action_recent_save_list[ii]->setVisible(false);
		}			
	}
}



void Menu_BinaryClass::connect_menu_device_sub(void)
{
#ifdef USE_BINARY_FILE1
	int ii;
# if !defined(_PASOPIA7) && !defined(_PASOPIA)
	this->addAction(action_saving);
	this->addSeparator();
	//this->addAction(menu_history_save->menuAction());
# else
	action_saving->setVisible(false);
	//menu_history_save->setVisible(false);
# endif   
	action_eject->setVisible(false);
	for(ii = 0; ii < MAX_HISTORY; ii++) {
		connect(action_recent_save_list[ii], SIGNAL(triggered()),
				action_recent_save_list[ii]->binds, SLOT(on_recent_binary_save()));
		connect(action_recent_save_list[ii]->binds, SIGNAL(set_recent_binary_save(int, int)),
				this, SLOT(do_open_recent_media_save(int, int)));
	}
	
	connect(action_saving, SIGNAL(triggered()),	this, SLOT(do_open_save_dialog()));
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_binary_load(int, int)));
	connect(this, SIGNAL(sig_set_recent_media_save(int, int)), p_wid, SLOT(set_recent_binary_save(int, int)));

	connect(this, SIGNAL(sig_open_media(int, QString)), p_wid, SLOT(_open_binary_load(int, QString)));
	connect(this, SIGNAL(sig_open_media_save(int, QString)), p_wid, SLOT(_open_binary_save(int, QString)));
#endif	
}



void Menu_BinaryClass::do_open_save_dialog()
{
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
	dlg.param->setPlay(false);
	dlg.setWindowTitle(QApplication::translate("MainWindow", "Save Binary", 0));
	dlg.setDirectory(initial_dir);
	dlg.setNameFilters(ext_filter);

	QObject::connect(&dlg, SIGNAL(fileSelected(QString)),
					 dlg.param, SLOT(_open_disk(QString))); 
	QObject::connect(dlg.param, SIGNAL(do_open_disk(int, QString)),
					 this, SLOT(do_open_media_save(int, QString)));
	dlg.show();
	dlg.exec();
	return;
}

void Menu_BinaryClass::retranslate_pulldown_menu_device_sub(void)
{
	action_insert->setText(QApplication::translate("MainWindow", "Load", 0));
	action_saving->setText(QApplication::translate("MainWindow", "Save", 0));
	action_saving->setIcon(QIcon(":/icon_saveas.png"));
	//menu_history_save->setTitle(QApplication::translate("MainWindow", "Recently Saved", 0));
	menu_history->setTitle(QApplication::translate("MainWindow", "Recently Loaded", 0));
}

