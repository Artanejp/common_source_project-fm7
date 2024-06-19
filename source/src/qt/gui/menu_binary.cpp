/*
 * Qt / Tape Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License : GPLv2
 *   History :
 *     Jan 13 2015 : Start
 */
#include <QApplication>
#include <QActionGroup>

#include "mainwidget_base.h"
#include "menu_binary.h"

#include "qt_dialogs.h"


Menu_BinaryClass::Menu_BinaryClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent, int drv, int base_drv) : Menu_MetaClass(root_entry, desc, p, parent, drv, base_drv)
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
	action_saving = new QAction(p_wid);
	action_saving->setVisible(true);
	action_saving->setCheckable(false);

	struct CSP_Ui_Menu::DriveIndexPair tmp;
	QVariant _tmp_ins;

	tmp.drive = media_drive;
	tmp.index = 0;
	_tmp_ins.setValue(tmp);
	action_saving->setData(_tmp_ins);
	connect(action_insert, SIGNAL(triggered()), this, SLOT(do_open_save_dialog()), Qt::QueuedConnection);
	//menu_history_save = new QMenu(this);
	//menu_history_save->setObjectName(QString::fromUtf8("menu_history_save_") + object_desc);

	{
		QString tmps;
		action_group_save_recent = new QActionGroup(p_wid);
		action_group_save_recent->setExclusive(true);

		for(ii = 0; ii < MAX_HISTORY; ii++) {
			tmps = QString::fromUtf8("");
			action_recent_save_list[ii] = new QAction(p_wid);
			struct CSP_Ui_Menu::DriveIndexPair tmp;
			tmp.drive = media_drive;
			tmp.index = ii;
			QVariant _tmpv;
			_tmpv.setValue(tmp);
			action_recent_save_list[ii]->setData(_tmpv);

			action_recent_save_list[ii]->setText(tmps);
			action_group_save_recent->addAction(action_recent_save_list[ii]);
			if(!tmps.isEmpty()) {
				action_recent_save_list[ii]->setVisible(true);
			} else {
				action_recent_save_list[ii]->setVisible(false);
			}
		}
	}
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
	int ii;
	if(!using_flags->is_machine_pasopia_variants()) {
		this->addAction(action_saving);
		this->addSeparator();
	} else {
		action_saving->setVisible(false);
	}
	action_eject->setVisible(false);
	for(ii = 0; ii < MAX_HISTORY; ii++) {
		connect(action_recent_save_list[ii], SIGNAL(triggered()),
				p_wid, SLOT(set_recent_binary_save()));
	}
	connect(action_saving, SIGNAL(triggered()),	this, SLOT(do_open_save_dialog()));

	connect(this, SIGNAL(sig_open_media_load(int, QString)), p_wid, SLOT(_open_binary_load(int, QString)));
	connect(this, SIGNAL(sig_open_media_save(int, QString)), p_wid, SLOT(_open_binary_save(int, QString)));
}


void Menu_BinaryClass::retranslate_pulldown_menu_device_sub(void)
{
	action_insert->setText(QApplication::translate("MenuMedia", "Load", 0));
	action_insert->setToolTip(QApplication::translate("MenuMedia", "Load memory image from a file.", 0));
	action_saving->setText(QApplication::translate("MenuMedia", "Save", 0));
	action_saving->setToolTip(QApplication::translate("MenuMedia", "Save memory image to a file.", 0));
	action_saving->setIcon(QIcon(":/icon_saveas.png"));
	//menu_history_save->setTitle(QApplication::translate("MenuMedia", "Recently Saved", 0));
	menu_history->setTitle(QApplication::translate("MenuMedia", "Recently Loaded", 0));
}
