/*
 * Qt / DIsk Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   LIcense: GPLv2
 *   History: Jan 10, 2015 (MAYBE) : Initial.
 */
#include <QApplication>

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_harddisk.h"

#include "emu_thread_tmpl.h"
#include "qt_dialogs.h"

Menu_HDDClass::Menu_HDDClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent, int drv, int base_drv) : Menu_MetaClass(root_entry, desc, p, parent, drv, base_drv)
{
	use_write_protect = false;
	use_d88_menus = false;
}

Menu_HDDClass::~Menu_HDDClass()
{
}

void Menu_HDDClass::create_pulldown_menu_device_sub(void)
{
	action_create_hdd = new Action_Control(p_wid, using_flags);
	action_create_hdd->setVisible(true);
	action_create_hdd->setCheckable(false);
}


void Menu_HDDClass::connect_menu_device_sub(void)
{
	this->addSeparator();
	this->addAction(action_create_hdd);
	this->addSeparator();

   	connect(this, SIGNAL(sig_open_media(int, QString)), p_wid, SLOT(_open_hard_disk(int, QString)));
	connect(this, SIGNAL(sig_eject_media(int)), p_wid, SLOT(eject_hard_disk(int)));
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_hard_disk(int, int)));
	connect(action_create_hdd, SIGNAL(triggered()), this, SLOT(do_open_dialog_create_hd()));
}

void Menu_HDDClass::connect_via_emu_thread(EmuThreadClassBase *p)
{
	if(p == nullptr) return;
	connect(action_eject, SIGNAL(triggered()), p, SLOT(do_close_hard_disk()), Qt::QueuedConnection);
}

void Menu_HDDClass::retranslate_pulldown_menu_device_sub(void)
{
	//action_insert->setIcon(icon_floppy);
	action_insert->setText(QApplication::translate("MenuHDD", "Mount", 0));
	action_insert->setToolTip(QApplication::translate("MenuHDD", "Mount virtual hard disk file.", 0));

	action_eject->setText(QApplication::translate("MenuHDD", "Unmount", 0));
	action_eject->setToolTip(QApplication::translate("MenuHDD", "Unmount virtual hard disk.", 0));

	action_create_hdd->setText(QApplication::translate("MenuHDD", "Create Virtual HDD", 0));
	action_create_hdd->setToolTip(QApplication::translate("MenuHDD", "Create and mount virtual blank-hard disk.\nThis makes only NHD/HDI format.", 0));

}

void Menu_HDDClass::do_open_dialog_create_hd()
{
	CSP_CreateHardDiskDialog dlg(media_drive, 512, 15, 4, 1024);

	if(initial_dir.isEmpty()) {
		QDir dir;
		char app[PATH_MAX];
		initial_dir = dir.currentPath();
		strncpy(app, initial_dir.toLocal8Bit().constData(), PATH_MAX - 1);
		initial_dir = QString::fromLocal8Bit(get_parent_dir(app));
	}
	dlg.dlg->setDirectory(initial_dir);
	QString create_ext = QString::fromUtf8("*.nhd *.hdi");
	QString create_desc = QString::fromUtf8("Virtual HARDDISK Image.");
	QString all = QString::fromUtf8("All Files (*.*)");
	QString tmps = create_desc;
	tmps.append(QString::fromUtf8(" ("));
	tmps.append(create_ext.toLower());
	tmps.append(QString::fromUtf8(" "));
	tmps.append(create_ext.toUpper());
	tmps.append(QString::fromUtf8(")"));
	QStringList __filter;
	__filter.clear();
	__filter << tmps;
	__filter << all;
	__filter.removeDuplicates();
	dlg.dlg->setNameFilters(__filter);

	tmps.clear();
	tmps = QApplication::translate("MenuMedia", "Create NHD/HDI Virtual HARDDISK", 0);
	if(!window_title.isEmpty()) {
		tmps = tmps + QString::fromUtf8(" ") + window_title;
	} else {
		tmps = tmps + QString::fromUtf8(" ") + this->title();
	}
	dlg.dlg->setWindowTitle(tmps);
	connect(&dlg, SIGNAL(sig_create_disk(int, int, int, int, int, QString)),
			p_wid, SLOT(do_create_hard_disk(int, int, int, int, int, QString)));
//	QObject::connect(&dlg, SIGNAL(sig_create_disk(QString)), this, SLOT(do_create_media(QString)));
//	QObject::connect(this, SIGNAL(sig_create_d88_media(int, quint8, QString)), p_wid, SLOT(do_create_d88_media(int, quint8, QString)));

	dlg.show();
	dlg.dlg->exec();
	return;
}
