/*
 * Qt / DIsk Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   LIcense: GPLv2
 *   History: Jan 10, 2015 (MAYBE) : Initial.
 */
#include <QApplication>

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_disk.h"

#include "qt_dialogs.h"
//#include "emu.h"


Menu_FDClass::Menu_FDClass(QMenuBar *root_entry, QString desc, USING_FLAGS *p, QWidget *parent, int drv, int base_drv) : Menu_MetaClass(root_entry, desc, p, parent, drv, base_drv)
{
	use_write_protect = true;
	use_d88_menus = true;
	icon_floppy = QIcon(":/icon_floppy.png");

	for(int i = 0; i < 4; i++) {
		type_mask[i] = true;
	}
}

Menu_FDClass::~Menu_FDClass()
{
}

void Menu_FDClass::do_set_create_mask(quint8 type, bool flag)
{
	switch(type) {
	case 0x00: // 2D
		type_mask[0] = flag;
		break;
	case 0x10: // 2DD
		type_mask[1] = flag;
		break;
	case 0x20: // 2HD
		type_mask[2] = flag;
		break;
	case 0x30: // 2HD/1.44M
		type_mask[3] = flag;
		break;
	}
}

void Menu_FDClass::do_open_dialog_create_fd()
{
	CSP_CreateDiskDialog dlg(type_mask);
	
	if(initial_dir.isEmpty()) { 
		QDir dir;
		char app[PATH_MAX];
		initial_dir = dir.currentPath();
		strncpy(app, initial_dir.toLocal8Bit().constData(), PATH_MAX - 1);
		initial_dir = QString::fromLocal8Bit(get_parent_dir(app));
	}

	dlg.dlg->setDirectory(initial_dir);
	QString create_ext = QString::fromUtf8("*.d88 *.d77");
	QString create_desc = QString::fromUtf8("D88/D77 Virtual Floppy Image.");
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
	tmps = QApplication::translate("MenuMedia", "Create D88/D77 virtual floppy", 0);
	if(!window_title.isEmpty()) {
		tmps = tmps + QString::fromUtf8(" ") + window_title;
	} else {
		tmps = tmps + QString::fromUtf8(" ") + this->title();
	}
	dlg.dlg->setWindowTitle(tmps);
	
	QObject::connect(&dlg, SIGNAL(sig_create_disk(quint8, QString)), this, SLOT(do_create_media(quint8, QString)));
	QObject::connect(this, SIGNAL(sig_create_d88_media(int, quint8, QString)), p_wid, SLOT(do_create_d88_media(int, quint8, QString)));

	dlg.show();
	dlg.dlg->exec();
	return;
}

void Menu_FDClass::do_create_media(quint8 media_type, QString name)
{
	emit sig_create_d88_media((int)media_drive, media_type, name);
}

void Menu_FDClass::do_set_correct_disk_timing(bool flag)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int drive = cp->data().value<int>();
	if(p_config == nullptr) return;
	
	p_config->correct_disk_timing[drive] = flag;
	emit sig_emu_update_config();
}

void Menu_FDClass::do_set_disk_count_immediate(bool flag)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int drive = cp->data().value<int>();
	if(p_config == nullptr) return;
	
	p_config->disk_count_immediate[drive] = flag;
	emit sig_emu_update_config();
}

void Menu_FDClass::do_set_ignore_crc_error(bool flag)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int drive = cp->data().value<int>();
	
	if(p_config == nullptr) return;
	p_config->ignore_disk_crc[drive] = flag;
	emit sig_emu_update_config();
}

void Menu_FDClass::create_pulldown_menu_device_sub(void)
{
	config_t *p;
	struct CSP_Ui_Menu::DriveIndexPair tmp;
	
	action_ignore_crc_error = new Action_Control(p_wid, using_flags);
	action_ignore_crc_error->setVisible(true);
	action_ignore_crc_error->setCheckable(true);
	tmp.drive = media_drive;
	tmp.index = 0;
	QVariant _tmp_ic;
	_tmp_ic.setValue(tmp);
	action_ignore_crc_error->setData(_tmp_ic);
	
	
	action_correct_timing = new Action_Control(p_wid, using_flags);
	action_correct_timing->setVisible(true);
	action_correct_timing->setCheckable(true);
	QVariant _tmp_ct;
	_tmp_ct.setValue(tmp);
	action_correct_timing->setData(_tmp_ct);

	action_count_immediate = new Action_Control(p_wid, using_flags);
	action_count_immediate->setVisible(true);
	action_count_immediate->setCheckable(true);
	QVariant _tmp_ci;
	_tmp_ci.setValue(tmp);
	action_count_immediate->setData(_tmp_ci);
	
	action_create_fd = new Action_Control(p_wid, using_flags);
	action_create_fd->setVisible(true);
	action_create_fd->setCheckable(false);

	p = p_config;
	if(p != NULL) {
		if(p->correct_disk_timing[media_drive]) action_correct_timing->setChecked(true);
		if(p->ignore_disk_crc[media_drive]) action_ignore_crc_error->setChecked(true);
		if(p->disk_count_immediate[media_drive]) action_count_immediate->setChecked(true);
	}		
}


void Menu_FDClass::connect_menu_device_sub(void)
{
	this->addSeparator();
	this->addAction(action_create_fd);
	this->addSeparator();
	this->addAction(action_ignore_crc_error);
	this->addAction(action_correct_timing);
	this->addAction(action_count_immediate);
	
	connect(action_ignore_crc_error, SIGNAL(toggled(bool)),
			this, SLOT(do_set_ignore_crc_error(bool)));
	
	connect(action_correct_timing, SIGNAL(toggled(bool)),
			this, SLOT(do_set_correct_disk_timing(bool)));
	
	connect(action_count_immediate, SIGNAL(toggled(bool)),
			this, SLOT(do_set_disk_count_immediate(bool)));

	connect(action_create_fd, SIGNAL(triggered()), this, SLOT(do_open_dialog_create_fd()));
	
   	connect(this, SIGNAL(sig_open_media(int, QString)), p_wid, SLOT(_open_disk(int, QString)));
	connect(this, SIGNAL(sig_eject_media(int)), p_wid, SLOT(eject_fd(int)));
	
	connect(this, SIGNAL(sig_write_protect_media(int, bool)), p_wid, SLOT(write_protect_fd(int, bool)));	
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_disk(int, int)));
	connect(this, SIGNAL(sig_set_inner_slot(int, int)), p_wid, SLOT(set_d88_slot(int, int)));
}

void Menu_FDClass::retranslate_pulldown_menu_device_sub(void)
{
	action_insert->setIcon(icon_floppy);
	action_insert->setToolTip(QApplication::translate("MenuMedia", "Insert virtual floppy disk file.", 0));
	action_eject->setToolTip(QApplication::translate("MenuMedia", "Eject virtual floppy disk.", 0));
	
	action_ignore_crc_error->setText(QApplication::translate("MenuMedia", "Ignore CRC error", 0));
	action_ignore_crc_error->setToolTip(QApplication::translate("MenuMedia", "Ignore CRC error of virtual floppy.\nUseful for some softwares,\n but causes wrong working with some softwares.", 0));
	action_correct_timing->setText(QApplication::translate("MenuMedia", "Correct transfer timing", 0));
	action_correct_timing->setToolTip(QApplication::translate("MenuMedia", "Correct transferring timing.\nUseful for some softwares\n needs strict transfer timing.", 0));
	
	action_create_fd->setText(QApplication::translate("MenuMedia", "Create Virtual Floppy", 0));
	action_create_fd->setToolTip(QApplication::translate("MenuMedia", "Create and mount virtual blank-floppy disk.\nThis makes only D88/D77 format.", 0));

	action_count_immediate->setText(QApplication::translate("MenuMedia", "Immediate increment", 0));
	action_count_immediate->setToolTip(QApplication::translate("MenuMedia", "Increment data pointer immediately.\nThis is test hack for MB8877.\nUseful for some softwares\n needs strict transfer timing.", 0));
}



