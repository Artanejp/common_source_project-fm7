/*
 * Widget: Meta Menu Class.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History: 2015-11-11 Initial.
 */

#include <QAction>
#include <QActionGroup>
#include <QWidget>
#include <QFileInfo>
#include <QDir>
#include <QStyle>
#include <QApplication>
#include <QMenuBar>
#include <QSettings>

#include "qt_dialogs.h"
#include "menu_metaclass.h"

#include "mainwidget_base.h"
#include "emu_thread_tmpl.h"

#include "qt_main.h"

Menu_MetaClass::Menu_MetaClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent, int drv, int base_drv) : QMenu(root_entry)
{
	QString tmps;
	int ii;

	p_wid = parent;
	menu_root = root_entry;
	//p_emu = ep;
	using_flags = p;
	p_config = p->get_config_ptr();

	media_drive = drv;
	base_drive = base_drv;

	
	tmps.setNum(drv);
	menu_desc = desc + tmps;
	object_desc = QString::fromUtf8("Obj_") + menu_desc;
	setObjectName(object_desc);

	for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) {
		action_select_media_list[ii] = nullptr;
	}
	use_write_protect = true;
	use_d88_menus = false;
	initial_dir = QString::fromUtf8("");

	ext_filter.clear();
	ext_save_filter.clear();

	load_dir_history = load_state(false);
	save_dir_history = load_state(true);

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

QStringList Menu_MetaClass::load_state(bool is_save)
{
	QStringList tmplist;
	QSettings *setting = using_flags->get_settings();
	if(setting == nullptr) {
		return tmplist;
	}
	
	QString head;
	if(is_save) {
		head = QString::fromUtf8("FileDialog_Save_");
	} else {
		head = QString::fromUtf8("FileDialog_Load_");
	}
	head = head + menu_desc;
	setting->beginGroup(head);
	QString tmps;
	for(int i = 0; i < MAX_HISTORY; i++) {
		tmps = QString::fromUtf8("Directory_%1").arg(i);
		if(setting->contains(tmps)) {
			tmplist.push_front(setting->value(tmps).toString());
		}
	}
	setting->endGroup();
	return tmplist;
}

void Menu_MetaClass::save_state(QStringList listval, bool is_save, bool is_sync)
{
	QSettings *setting = using_flags->get_settings();
	if(setting == nullptr) {
		return;
	}
	QString head;
	if(is_save) {
		head = QString::fromUtf8("FileDialog_Save_");
	} else {
		head = QString::fromUtf8("FileDialog_Load_");
	}
	head = head + menu_desc;
	setting->beginGroup(head);
	QString tmps;
	for(int i = 0; i < listval.size(); i++) {
		if(i >= MAX_HISTORY) break;
		tmps = QString::fromUtf8("Directory_%1").arg(i);
		setting->setValue(tmps, listval.at(i));
	}
	setting->endGroup();
	if(is_sync) {
		setting->sync();
	}
}


// This is dummy.Please implement.
void Menu_MetaClass::connect_via_emu_thread(EmuThreadClassBase *p)
{
	if(p == nullptr) return;
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

QStringList Menu_MetaClass::insert_dir_history_by_filename(QStringList& l, QString name)
{
	QFileInfo info(name);
	QDir dir;
	QStringList tmplist;
	tmplist = l;
	
//	if(!info.exists()) {
//		return tmplist;
//	}
	dir = info.dir();
	dir.makeAbsolute();
	printf("%s %s\n", name.toLocal8Bit().constData(), dir.path().toLocal8Bit().constData());
	if(!(dir.path().isEmpty())) {
		tmplist.removeAll(dir.path());
		tmplist.push_front(dir.path());
		if(tmplist.size() > MAX_HISTORY) {
			tmplist.resize(MAX_HISTORY);
			tmplist.squeeze();
		}
		l = tmplist;
	}
	return l;
}

void Menu_MetaClass::do_open_media_load(QString name)
{
	//write_protect = false; // Right? On D88, May be writing entry  exists.
	if(!(name.isEmpty())) {
		insert_dir_history_by_filename(load_dir_history, name);
		save_state(load_dir_history, false, true); // ToDo.
		emit sig_open_media_load(media_drive, name);
	}
}

void Menu_MetaClass::do_open_media_save(QString name)
{
	//write_protect = false; // Right? On D88, May be writing entry  exists.
	if(!(name.isEmpty())) {
		insert_dir_history_by_filename(save_dir_history, name);
		save_state(save_dir_history, true, true); // ToDo.
		emit sig_open_media_save(media_drive, name);
	}
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

void Menu_MetaClass::do_open_recent_media_save(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	struct CSP_Ui_Menu::DriveIndexPair tmp = cp->data().value<CSP_Ui_Menu::DriveIndexPair>();

  //   write_protect = false; // Right? On D88, May be writing entry  exists.
	emit sig_set_recent_media_save(tmp.drive, tmp.index);
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

void Menu_MetaClass::do_add_save_media_extension(QString ext, QString description)
{
	QString tmps;
	QString all = QString::fromUtf8("All Files (*.*)");

	tmps = description + QString::fromUtf8(" (%1 %2)").arg(ext.toLower()).arg(ext.toUpper());

	ext_save_filter << tmps;
	ext_save_filter << all;

	ext_save_filter.removeDuplicates();
}

void Menu_MetaClass::do_select_inner_media(int num)
{
	if(use_d88_menus && (num < using_flags->get_max_d88_banks())) {
		if(action_select_media_list[num] != nullptr) {
			action_select_media_list[num]->setChecked(true);
		}
	}
}
void Menu_MetaClass::do_close_window()
{
	
}
void Menu_MetaClass::do_finish(int i)
{
	//emit sig_stop_timer();
}

void Menu_MetaClass::do_open_dialog()
{
	//QFileDialog* dlgptr = new QFileDialog(nullptr, Qt::Dialog);
	//QFileDialog* dlgptr = new QFileDialog(nullptr, Qt::BypassWindowManagerHint | Qt::Dialog);
	QFileDialog* dlgptr = new QFileDialog(nullptr);
	do_open_dialog_common(dlgptr , false);
}

void Menu_MetaClass::do_open_save_dialog()
{
	QFileDialog* dlgptr = new QFileDialog(nullptr, Qt::Dialog);
	do_open_dialog_common(dlgptr , true);
//	emit sig_show();
}


bool Menu_MetaClass::do_open_dialog_common(QFileDialog* dlg, bool is_save)
{
	// ToDo : Load State of Qt.


	if(initial_dir.isEmpty()) {
		QDir dir;
		char app[_MAX_PATH];
		initial_dir = dir.currentPath();
		strncpy(app, initial_dir.toLocal8Bit().constData(), _MAX_PATH - 1);
		initial_dir = QString::fromLocal8Bit(get_parent_dir(app));
	}
	if(dlg == nullptr) {
		return false;
	}
	dlg->setAttribute(Qt::WA_DeleteOnClose, true);
	dlg->setAttribute(Qt::WA_ForceUpdatesDisabled, true);	
	dlg->setOption(QFileDialog::DontUseNativeDialog, true);
	//dlg->setOption(QFileDialog::DontUseNativeDialog, false);
	dlg->setOption(QFileDialog::ReadOnly, (is_save) ? false : true);
	dlg->setOption(QFileDialog::DontUseCustomDirectoryIcons, true);
	dlg->setAcceptMode((is_save) ? QFileDialog::AcceptSave : QFileDialog::AcceptOpen);
	dlg->setFileMode((is_save) ? QFileDialog::AnyFile : QFileDialog::ExistingFile);
	//dlg->setViewMode(QFileDialog::Detail);
	dlg->setViewMode(QFileDialog::List);

	QString tmps;
	if(is_save) {
		tmps = QApplication::translate("MenuMedia", "Save", 0);
	} else {
		tmps = QApplication::translate("MenuMedia", "Open", 0);
	}
	if(!window_title.isEmpty()) {
		tmps = tmps + QString::fromUtf8(" ") + window_title;
	} else {
		tmps = tmps + QString::fromUtf8(" ") + this->title();
	}
	dlg->setWindowTitle(tmps);

	dlg->setModal(false);
//	dlg->setWindowModality(Qt::ApplicationModal);
//	dlg->setVisible(true);
	
	// ToDo: Be more sotisficated .
	if(is_save) {
		connect(dlg, SIGNAL(fileSelected(QString)), this, SLOT(do_open_media_save(QString)));
	} else {
		connect(dlg, SIGNAL(fileSelected(QString)), this, SLOT(do_open_media_load(QString)));
	}
	connect(dlg, SIGNAL(finished(int)), this, SLOT(do_finish(int)));
	connect(dlg, SIGNAL(destroyed()), this, SLOT(do_close_window()));
//	connect(this, SIGNAL(sig_show()), dlg, SLOT(open()));

	dlg->setDirectory(initial_dir);
	QStringList hist = (is_save) ? save_dir_history : load_dir_history;
	if(!(hist.isEmpty())) {
		dlg->setHistory(hist);
	}
	if(is_save) {
		dlg->setNameFilters(ext_save_filter);
	} else {
		dlg->setNameFilters(ext_filter);
	}
	// They are workaround from window_bring_to_front(QWidget * window)
	// of libaudqt/audqt.cc of audacious. - 240525 K.O
//	dlg->show();
    dlg->open();
//	dlg->windowHandle()->setWindowState(Qt::WindowActive, true);
	dlg->windowHandle()->setVisibility(QWindow::AutomaticVisibility);
	dlg->windowHandle()->requestActivate();
//    dlg->raise();
//	dlg->activateWindow();

	return true;
}

QStringList Menu_MetaClass::get_history(bool is_save)
{
	QStringList lst;
	QAction **p = nullptr;
	if(!(is_save)) { // ToDo: History for saving.
		p = action_recent_list;
	}
	if(p != nullptr) {
		for(int i = 0; i < MAX_HISTORY ; i++) {
			if(p[i] != nullptr) {
				QString tmpstr = p[i]->text();
				lst.append(tmpstr);
			}
		}
	}
	return lst;
}

void Menu_MetaClass::do_insert_history(QString path)
{
	QStringList lst;
	for(int i = 0; i < MAX_HISTORY ; i++) {
		if(action_recent_list[i] != nullptr) {
			QString tmpstr = action_recent_list[i]->text();
			lst.append(tmpstr);
		}
	}
	if(lst.at(0) == path) {
		// None changed.
		return;
	}
	QStringList lst2;
	lst2.clear();
	for(size_t i = 0 ; i < lst.size(); i++) {
		if(path != lst.at(i)) lst2.append(lst.at(i));
	}
	lst2.push_front(path);
	do_update_histories(lst2);
}
void Menu_MetaClass::do_update_histories(QStringList lst)
{
	QString tmps;
	int ptr = 0;
	for(int i = 0; i < lst.size(); i++) {
		if(i >= MAX_HISTORY) break;
		action_recent_list[i]->setText(lst.at(i));
		action_recent_list[i]->setVisible(true);
		ptr++;
	}
	for(int i = ptr; i < MAX_HISTORY; i++) {
		action_recent_list[i]->setText(QString::fromUtf8(""));
		action_recent_list[i]->setVisible(false);
	}
}


void Menu_MetaClass::do_clear_inner_media(void)
{
	int ii;
	//inner_media_list.clear();
	if(use_d88_menus) {
		for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) {
			if(action_select_media_list[ii] != nullptr) {
				action_select_media_list[ii]->setText(QString::fromUtf8(""));
				action_select_media_list[ii]->setVisible(false);
			}
		}
		if(action_select_media_list[0] != nullptr) {
			action_select_media_list[0]->setChecked(true);
		}
	}
}


void Menu_MetaClass::do_update_inner_media(QStringList lst, int num)
{
	QString tmps;
	if(use_d88_menus) {
		//inner_media_list.clear();
		do_clear_inner_media();
		int _n = using_flags->get_max_d88_banks();
		if(_n <= 0) return;
		int count = 0;
		for(auto _l = lst.begin(); _l != lst.end(); ++_l) {
			if(count >= _n) break;
			//inner_media_list.push_back((*_l));
			//printf("do_update_inner_media: num=%d count=%d val=%s\n", num, count, (*_l).toLocal8Bit().constData());
			if(action_select_media_list[count] != nullptr) {
				action_select_media_list[count]->setText((*_l));
				if(count == num) {
					action_select_media_list[count]->setChecked(true);
				}
				action_select_media_list[count]->setVisible(true);
			}
			count++;
		}
	}
}

//void Menu_MetaClass::do_update_inner_media_bubble(QStringList lst, int num)
//{
//	QString tmps;
//	inner_media_list.clear();
//	emit sig_update_inner_bubble(media_drive, inner_media_list, action_select_media_list,
//								 lst, num, use_d88_menus);
//}

void Menu_MetaClass::create_pulldown_menu_sub(void)
{
	action_insert = new QAction(p_wid);
	action_insert->setObjectName(QString::fromUtf8("action_insert_") + object_desc);

	struct CSP_Ui_Menu::DriveIndexPair tmp;
	QVariant _tmp_ins;

	tmp.drive = media_drive;
	tmp.index = 0;
	_tmp_ins.setValue(tmp);
	action_insert->setData(_tmp_ins);

	
	//connect(action_insert, SIGNAL(triggered()), this, SLOT(do_delayed_open_dialog()));
	//connect(action_insert, SIGNAL(triggered()), this, SLOT(do_open_dialog()), Qt::QueuedConnection);
	connect(action_insert, SIGNAL(triggered()), this, SLOT(do_open_dialog()), Qt::DirectConnection);
	//connect(this, SIGNAL(sig_open_dialog()), this, SLOT(do_open_dialog()));	
	action_insert->setIcon(icon_insert);

	action_eject = new QAction(p_wid);
	action_eject->setObjectName(QString::fromUtf8("action_eject_") + object_desc);

	QVariant _tmp_eject;
	tmp.drive = media_drive;
	tmp.index = 0;
	_tmp_eject.setValue(tmp);
	action_eject->setData(_tmp_eject);
	action_eject->setIcon(icon_eject);

	{
		QString tmps;
		int ii;
		action_group_recent = new QActionGroup(p_wid);
		action_group_recent->setExclusive(true);

		for(ii = 0; ii < MAX_HISTORY; ii++) {
			tmps = QString::fromUtf8("");
			action_recent_list[ii] = new QAction(p_wid);
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
			tmps = QString::fromUtf8("");
			action_select_media_list[ii] = new QAction(p_wid);
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

		action_write_protect_on = new QAction(p_wid);
		action_write_protect_on->setObjectName(QString::fromUtf8("action_write_protect_on_") + object_desc);
		action_write_protect_on->setCheckable(true);
		action_write_protect_on->setChecked(true);
		action_write_protect_on->setData(QVariant(media_drive));

		action_write_protect_off = new QAction(p_wid);
		action_write_protect_off->setObjectName(QString::fromUtf8("action_write_protect_off_") + object_desc);
		action_write_protect_off->setCheckable(true);
		action_write_protect_off->setData(QVariant(media_drive));

		action_group_protect->addAction(action_write_protect_on);
		action_group_protect->addAction(action_write_protect_off);
		connect(action_write_protect_on, SIGNAL(triggered()), this, SLOT(do_write_protect_media()));
		connect(action_write_protect_off, SIGNAL(triggered()), this, SLOT(do_write_unprotect_media()));
	}
}

void Menu_MetaClass::do_delayed_open_dialog()
{
	QTimer::singleShot(10, this, SLOT(do_open_dialog()));
}

void Menu_MetaClass::do_delayed_open_save_dialog()
{
	QTimer::singleShot(10, this, SLOT(do_open_save_dialog()));
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

}
