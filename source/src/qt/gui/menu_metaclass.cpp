/*
 * Widget: Meta Menu Class.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History: 2015-11-11 Initial.
 */

#include <QAction>
#include <QActionGroup>
#include <QWidget>
#include <QDir>
#include <QStyle>
#include <QApplication>
#include <QMenuBar>
#include <QThreadPool>

//#include "emu.h"
//#include "vm.h"
#include "qt_dialogs.h"
#include "menu_metaclass.h"
#include "commonclasses.h"
#include "mainwidget_base.h"
#include "commonclasses.h"
#include "qt_main.h"

Menu_MetaClass::Menu_MetaClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent, int drv, int base_drv) : QMenu(root_entry)
{
	QString tmps;
	int ii;
	
	p_wid = parent;
	menu_root = root_entry;

	//p_emu = ep;
	p_emu = NULL;
	using_flags = p;
	p_config = p->get_config_ptr();
	
	media_drive = drv;
	base_drive = base_drv;
	
	tmps.setNum(drv);
	object_desc = QString::fromUtf8("Obj_") + desc;
	object_desc.append(tmps);
	setObjectName(object_desc);
	

	for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) {
		action_select_media_list[ii] = NULL;
	}
	use_write_protect = true;
	use_d88_menus = false;
	initial_dir = QString::fromUtf8("");
	
	ext_filter.clear();
	history.clear();
	inner_media_list.clear();
	window_title = QString::fromUtf8("");

	icon_insert = QIcon(":/icon_open.png");
	icon_eject = QIcon(":/icon_eject.png");
	icon_write_protected = QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
	icon_write_enabled = QIcon();
	setToolTipsVisible(true);

	
	connect(this, SIGNAL(sig_emu_update_config()), p_wid, SLOT(do_emu_update_config()));
	
	tmps = QString::fromUtf8("%1").arg(drv + base_drv);
	tmps = desc + tmps;
	setTitle(tmps);
}

Menu_MetaClass::~Menu_MetaClass()
{
}


// This is virtual function, pls. override.
void Menu_MetaClass::do_set_write_protect(bool f)
{
	write_protect = f;
	if(f) {
		if(use_write_protect) {
			menu_write_protect->setIcon(icon_write_protected);
		}
		action_write_protect_on->setChecked(true);
	} else {
		if(use_write_protect) {
			menu_write_protect->setIcon(icon_write_enabled);
		}
		action_write_protect_off->setChecked(true);
	}		
}

void Menu_MetaClass::do_set_initialize_directory(const char *s)
{
	initial_dir = QString::fromLocal8Bit(s);
}

void Menu_MetaClass::do_open_media(int drv, QString name)
{
	//write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit sig_open_media(drv, name);
}

void Menu_MetaClass::do_insert_media(void)
{
	//write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit sig_insert_media(media_drive);
}

void Menu_MetaClass::do_eject_media(void)
{
	write_protect = false;
	emit sig_eject_media(media_drive);
}

void Menu_MetaClass::do_open_inner_media(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	
	emit sig_set_inner_slot(tmp.drive, tmp.index);
}

void Menu_MetaClass::do_open_recent_media(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();
	
  //   write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit sig_set_recent_media(tmp.drive, tmp.index);
}

void Menu_MetaClass::do_write_protect_media(void)
{
	write_protect = true;
	{
		if(use_write_protect) {
			menu_write_protect->setIcon(icon_write_protected);
		}
		action_write_protect_on->setChecked(true);
	}		
	emit sig_write_protect_media(media_drive, write_protect);
}

void Menu_MetaClass::do_write_unprotect_media(void) {
	write_protect = false;
	{
		if(use_write_protect) {
			menu_write_protect->setIcon(icon_write_enabled);
		}
		action_write_protect_off->setChecked(true);
	}		
	emit sig_write_protect_media(media_drive, write_protect);
}

void Menu_MetaClass::do_set_window_title(QString s) {
	window_title = s;
}

void Menu_MetaClass::do_add_media_extension(QString ext, QString description)
{
	QString tmps = description;
	QString all = QString::fromUtf8("All Files (*.*)");

	tmps.append(QString::fromUtf8(" ("));
	tmps.append(ext.toLower());
	tmps.append(QString::fromUtf8(" "));
	tmps.append(ext.toUpper());
	tmps.append(QString::fromUtf8(")"));

	ext_filter << tmps;
	ext_filter << all;

	ext_filter.removeDuplicates();
}

void Menu_MetaClass::do_select_inner_media(int num)
{
	if(use_d88_menus && (num < using_flags->get_max_d88_banks())) {
		if(action_select_media_list[num] != NULL) {
			action_select_media_list[num]->setChecked(true);
		}
	}
}
void Menu_MetaClass::do_close_window()
{
	CSP_DiskDialog *p = qobject_cast<CSP_DiskDialog*>(QObject::sender());
	for(auto i = dialogs.begin(); i != dialogs.end(); ++i) {
		if((*i) == p) {
			dialogs.erase(i);
			p->deleteLater();
			//dialogs.squeeze();
			return;
		}
	}
}
void Menu_MetaClass::do_finish(int i)
{
	do_close_window();
}

void Menu_MetaClass::do_open_dialog()
{
	if(initial_dir.isEmpty()) { 
		QDir dir;
		char app[PATH_MAX];
		initial_dir = dir.currentPath();
		strncpy(app, initial_dir.toLocal8Bit().constData(), PATH_MAX - 1);
		initial_dir = QString::fromLocal8Bit(get_parent_dir(app));
	}
	CSP_DiskDialog *dlg = new CSP_DiskDialog(nullptr);
	
	dlg->setOption(QFileDialog::ReadOnly, false);
	dlg->setOption(QFileDialog::DontUseNativeDialog, true);
	//dlg->setAcceptMode(QFileDialog::AcceptSave);
	dlg->setFileMode(QFileDialog::AnyFile);
	//dlg->setLabelText(QFileDialog::Accept, QApplication::translate("MenuMedia", "Open File", 0));
	dlg->param->setDrive(media_drive);
	dlg->param->setPlay(true);
	dlg->param->setDirectory(initial_dir);
	dlg->param->setNameFilters(ext_filter);

	QString tmps;
	tmps = QApplication::translate("MenuMedia", "Open", 0);
	if(!window_title.isEmpty()) {
		tmps = tmps + QString::fromUtf8(" ") + window_title;
	} else {
		tmps = tmps + QString::fromUtf8(" ") + this->title();
	}
	dlg->setWindowTitle(tmps);
	
	connect(dlg, SIGNAL(fileSelected(QString)), dlg->param, SLOT(_open_disk(QString))); 
	connect(dlg->param, SIGNAL(sig_open_disk(int, QString)), this, SLOT(do_open_media(int, QString)));
	connect(dlg, SIGNAL(accepted()), this, SLOT(do_close_window()), Qt::QueuedConnection); 
	connect(dlg, SIGNAL(rejected()), this, SLOT(do_close_window()), Qt::QueuedConnection); 
	connect(dlg, SIGNAL(finished(int)), this, SLOT(do_finish(int)), Qt::QueuedConnection); 

	connect(this, SIGNAL(sig_show()), dlg, SLOT(open()), Qt::QueuedConnection); 

	dlg->setModal(false);
	dialogs.append(dlg);
	
	//dlg->open();
	dlg->show();
	//dlg->exec();
	//emit sig_show();
	return;
}

void Menu_MetaClass::do_update_histories(QStringList lst)
{
	int ii;
	QString tmps;
	
	history.clear();
	for(ii = 0; ii < MAX_HISTORY; ii++) {
		tmps = QString::fromUtf8("");
		if(ii < lst.size()) tmps = lst.value(ii);
		history << tmps;
		action_recent_list[ii]->setText(tmps);
		if(!tmps.isEmpty()) {
			action_recent_list[ii]->setVisible(true);
		} else {
			action_recent_list[ii]->setVisible(false);
		}			
	}
}

void Menu_MetaClass::do_clear_inner_media(void)
{
	int ii;
	inner_media_list.clear();
	if(use_d88_menus) {
		for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) {
			if(action_select_media_list[ii] != NULL) {
				action_select_media_list[ii]->setText(QString::fromUtf8(""));
				action_select_media_list[ii]->setVisible(false);
			}
		}
	}
}


void Menu_MetaClass::do_update_inner_media(QStringList lst, int num)
{
	QString tmps;
	inner_media_list.clear();
	emit sig_update_inner_fd(media_drive, inner_media_list, action_select_media_list, lst , num, use_d88_menus);
}

void Menu_MetaClass::do_update_inner_media_bubble(QStringList lst, int num)
{
	QString tmps;
	inner_media_list.clear();
	emit sig_update_inner_bubble(media_drive, inner_media_list, action_select_media_list,
								 lst, num, use_d88_menus);
}

void Menu_MetaClass::create_pulldown_menu_sub(void)
{
	action_insert = new Action_Control(p_wid, using_flags);
	action_insert->setObjectName(QString::fromUtf8("action_insert_") + object_desc);
	
	struct CSP_Ui_Menu::DriveIndexPair tmp;
	QVariant _tmp_ins;
	
	tmp.drive = media_drive;
	tmp.index = 0;
	_tmp_ins.setValue(tmp);
	action_insert->setData(_tmp_ins);
	
	connect(action_insert, SIGNAL(triggered()), this, SLOT(do_open_dialog()), Qt::QueuedConnection);
	action_insert->setIcon(icon_insert);
	
	action_eject = new Action_Control(p_wid, using_flags);
	action_eject->setObjectName(QString::fromUtf8("action_eject_") + object_desc);

	QVariant _tmp_eject;
	tmp.drive = media_drive;
	tmp.index = 0;
	_tmp_eject.setValue(tmp);
	action_eject->setData(_tmp_eject);

	connect(action_eject, SIGNAL(triggered()), this, SLOT(do_eject_media()));
	action_eject->setIcon(icon_eject);

	
	{
		QString tmps;
		int ii;
		action_group_recent = new QActionGroup(p_wid);
		action_group_recent->setExclusive(true);
		
		for(ii = 0; ii < MAX_HISTORY; ii++) {
			tmps = history.value(ii, "");
			action_recent_list[ii] = new Action_Control(p_wid, using_flags);
			struct CSP_Ui_Menu::DriveIndexPair tmp2;
			tmp2.drive = media_drive;
			tmp2.index = ii;
			QVariant _tmp2v;
			_tmp2v.setValue(tmp2);
			
 			action_recent_list[ii]->setData(_tmp2v);
			action_recent_list[ii]->setText(tmps);
			
			action_group_recent->addAction(action_recent_list[ii]);
			if(!tmps.isEmpty()) {
				action_recent_list[ii]->setVisible(true);
			} else {
				action_recent_list[ii]->setVisible(false);
			}			
		}
	}
	if(use_d88_menus) {
		int ii;
		QString tmps;
		action_group_inner_media = new QActionGroup(p_wid);
		action_group_inner_media->setExclusive(true);
		
		for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) {
			tmps = history.value(ii, "");
			action_select_media_list[ii] = new Action_Control(p_wid, using_flags);
			struct CSP_Ui_Menu::DriveIndexPair tmp2;
			tmp2.drive = media_drive;
			tmp2.index = ii;
			QVariant _tmp2v;
			_tmp2v.setValue(tmp2);
			action_select_media_list[ii]->setData(_tmp2v);
			
			action_select_media_list[ii]->setText(tmps);
			action_select_media_list[ii]->setCheckable(true);
			if(ii == 0) action_select_media_list[ii]->setChecked(true);
			action_group_inner_media->addAction(action_select_media_list[ii]);
			if(!tmps.isEmpty()) {
				action_select_media_list[ii]->setVisible(true);
			} else {
				action_select_media_list[ii]->setVisible(false);
			}			
		}
	}
	if(use_write_protect) {
		action_group_protect = new QActionGroup(p_wid);
		action_group_protect->setExclusive(true);

		action_write_protect_on = new Action_Control(p_wid, using_flags);
		action_write_protect_on->setObjectName(QString::fromUtf8("action_write_protect_on_") + object_desc);
		action_write_protect_on->setCheckable(true);
		action_write_protect_on->setChecked(true);
		action_write_protect_on->setData(QVariant(media_drive));
		
		action_write_protect_off = new Action_Control(p_wid, using_flags);
		action_write_protect_off->setObjectName(QString::fromUtf8("action_write_protect_off_") + object_desc);
		action_write_protect_off->setCheckable(true);
		action_write_protect_off->setData(QVariant(media_drive));
		
		action_group_protect->addAction(action_write_protect_on);
		action_group_protect->addAction(action_write_protect_off);
		connect(action_write_protect_on, SIGNAL(triggered()), this, SLOT(do_write_protect_media()));
		connect(action_write_protect_off, SIGNAL(triggered()), this, SLOT(do_write_unprotect_media()));
	}
}

// This is virtual function, pls.apply
void Menu_MetaClass::create_pulldown_menu_device_sub(void)
{
	// Create device specific entries.
}

// This is virtual function, pls.apply
void Menu_MetaClass::connect_menu_device_sub(void)
{

}

//void Menu_MetaClass::setTitle(QString s)
//{
//	QMenu::setTitle(s);
//}

//QAction *Menu_MetaClass::menuAction()
//{
//	return QMenu::menuAction();
//}

void Menu_MetaClass::create_pulldown_menu(void)
{
	int ii;
	// Example:: Disk.
	create_pulldown_menu_sub();
	create_pulldown_menu_device_sub();
	// Create 

	menu_history = new QMenu(this);
	menu_history->setObjectName(QString::fromUtf8("menu_history_") + object_desc);

	if(use_d88_menus) {
		menu_inner_media = new QMenu(this);
		menu_inner_media->setObjectName(QString::fromUtf8("menu_inner_media_") + object_desc);
		for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) menu_inner_media->addAction(action_select_media_list[ii]);
		this->addAction(menu_inner_media->menuAction());
	}
	{
		menu_history = new QMenu(this);
		menu_history->setObjectName(QString::fromUtf8("menu_history_") + object_desc);
		for(ii = 0; ii < MAX_HISTORY; ii++) menu_history->addAction(action_recent_list[ii]);
		this->addAction(menu_history->menuAction());
	}
	if(use_write_protect) {
		this->addSeparator();
		menu_write_protect = new QMenu(this);
	}
	// Belows are setup of menu.
	this->addAction(action_insert);
	this->addAction(action_eject);

	// Connect extra menus to this.
	connect_menu_device_sub();
	this->addSeparator();
	
	// More actions
	this->addAction(menu_history->menuAction());
	if(use_d88_menus) {
		this->addAction(menu_inner_media->menuAction());
	}
	if(use_write_protect) {
		this->addSeparator();
		this->addAction(menu_write_protect->menuAction());
		menu_write_protect->addAction(action_write_protect_on);
		menu_write_protect->addAction(action_write_protect_off);
	}
	// Do connect!
	
	for(ii = 0; ii < MAX_HISTORY; ii++) {
		connect(action_recent_list[ii], SIGNAL(triggered()),
				this, SLOT(do_open_recent_media()));
	}
	if(use_d88_menus) {
		for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) {
			connect(action_select_media_list[ii], SIGNAL(triggered()),
					this, SLOT(do_open_inner_media()));
		}
	}
}

void Menu_MetaClass::retranslate_pulldown_menu_sub(void)
{
	action_insert->setText(QApplication::translate("MenuMedia", "Insert", 0));
	action_insert->setToolTip(QApplication::translate("MenuMedia", "Insert a virtual image file.", 0));
	action_eject->setText(QApplication::translate("MenuMedia", "Eject", 0));
	action_eject->setToolTip(QApplication::translate("MenuMedia", "Eject a inserted virtual image file.", 0));
	if(use_write_protect) {
		menu_write_protect->setTitle(QApplication::translate("MenuMedia", "Write Protection", 0));
		menu_write_protect->setToolTipsVisible(true);
		action_write_protect_on->setText(QApplication::translate("MenuMedia", "On", 0));
		action_write_protect_on->setToolTip(QApplication::translate("MenuMedia", "Enable write protection.\nYou can't write any data to this media.", 0));
		action_write_protect_off->setText(QApplication::translate("MenuMedia", "Off", 0));
		action_write_protect_off->setToolTip(QApplication::translate("MenuMedia", "Disable write protection.\nYou *can* write datas to this media.", 0));
	}
	
	if(use_d88_menus) {
		menu_inner_media->setTitle(QApplication::translate("MenuMedia", "Select D88 Image", 0));
	} else {
		//menu_inner_media->setVisible(false);
	}		
	menu_history->setTitle(QApplication::translate("MenuMedia", "Recent opened", 0));

}

void Menu_MetaClass::retranslate_pulldown_menu_device_sub(void)
{
}

void Menu_MetaClass::retranslateUi(void)
{
	retranslate_pulldown_menu_sub();
	retranslate_pulldown_menu_device_sub();
}

void Menu_MetaClass::setEmu(EMU_TEMPLATE *p)
{
	p_emu = p;
}

