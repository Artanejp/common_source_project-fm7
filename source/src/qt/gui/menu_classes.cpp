

#include "menu_metaclass.h"
#include "mainwidget.h"
#include "commonclasses.h"


Menu_MetaClass::Menu_MetaClass(EMU *ep, QMenu *root_entry, QString desc,
							   QWidget *parent, int drv) : QMenu(root_entry)
{
	QString tmps;
	int ii;
	
	p_wid = parent;
	menu_root = root_entry;
	p_emu = ep;
	media_drive = drv;
	
	tmps.arg(drv);
	object_desc = desc;
	object_desc.append(tmps);
	for(ii = 0; ii < MAX_D88_BANK; ii++) {
		action_select_inner_image[ii] = NULL;
	}
	use_write_protect = true;
	use_d88_menus = false;
	initial_dir = QString::fromUtf8("");
	
	ext_filter.clear();
	history.clear();
	inner_media_list.clear();
}

Menu_MetaClass::~Menu_MetaClass()
{
}


// This is virtual function, pls. override.
Menu_MetaClass::do_set_write_protect(bool f)
{
	write_protect = f;
}

void Menu_MetaClass::do_open_media(int drv, QString name) {
	//write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit sig_open_media(drv, name);
}

void Menu_MetaClass::do_insert_media(void) {
	//write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit sig_insert_media(media_drive);
}
void Menu_MetaClass::do_eject_media(void) {
	write_protect = false;
	emit sig_eject_media(media_drive);
}

void Menu_MetaClass::do_open_inner_media(int s_num) {
	emit sig_set_inner_slot(media_drive, s_num);
}

void Menu_MetaClass::do_open_recent_media(int s_num){
  //   write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit sig_set_recent_media(media_drive, s_num);
}
void Menu_MetaClass::do_write_protect_media(void) {
	write_protect = true;
	emit sig_write_protect_media(media_drive, write_protect);
}
void Menu_MetaClass::do_write_unprotect_media(void) {
	write_protect = false;
	emit sig_write_protect_media(drive, write_protect);
}

void Menu_MetaClass::do_add_media_extention(QString ext, QString description)
{
	QString tmps = description;
	QString all = QString::fromUtf8("All Files (*.* \)");

	tmps.append(QString::fromUtf8(" ("));
	tmps.append(ext.toLower());
	tmps.append(QString::fromUtf8(" "));
	tmps.append(ext.ToUpper());
	tmps.append(QString::fromUtf8(")"));

	ext_filter << tmps;
	ext_filter << all;

	ext_filter.removeDuplicates();
}

void Menu_MetaClass::do_open_dialog()
{
	CSP_DiskDialog dlg;
	if(initial_dir.isEmpty()) { 
		QDir dir;
		char app[PATH_MAX];
		initial_dir = df.currentPath();
		strncpy(app, initial_dir.toLocal8bit().constData(), PATH_MAX);
		initial_dir = QString::fromLocal8Bit(get_parent_dir(app));
	}
	dlg.param->setDrive(media_drive);
	dlg.setDirectory(initial_dir);
	dlg.setNameFilters(ext_filter);
	
	QObject::connect(&dlg, SIGNAL(fileSelected(QString)),
					 dlg.param, SLOT(_open_disk(QString))); 
	QObject::connect(dlg.param, SIGNAL(do_open_disk(int, QString)),
					 this, SLOT(do_open_media(int, QString)));

	dlg.show();
	dlg.exec();
	return;
}

void Menu_MetaClass::do_update_histories(const char *str[])
{
	int ii;
	QString tmps;
	
	history.clear();
	for(ii = 0; ii < MAX_HISTORY; ii++) {
		if(str[ii] == NULL) {
			tmps = QString::fromUtf8("");
		} else {
			tmps = QString::fromUtf8(str[ii]);
		}
		history << tmps;
		action_recent_list[ii]->setText(tmps);
		if(!tmps.isEmpty()) {
			action_recent_list[ii]->setVisible(true);
		} else {
			action_recent_list[ii]->setVisible(false);
		}			
	}
}

void Menu_MetaClass::do_update_inner_media(const char *str[])
{
	QString tmps;
	
	inner_media_list.clear();
	if(str == NULL) {
		for(ii = 0; ii < MAX_D88_BANK; ii++) {
			if(action_select_inner_image[ii] != NULL) action_select_inner_image[ii]->setVisible(false);
		}
		return;
	}
	
	for(ii = 0; ii < MAX_D88_BANK; ii++) {
		if(action_select_inner_image[ii] != NULL) {
			if(str[ii] != NULL) {
				tmps = QString::fromUtf8(str[ii]);
				if(!tmps.isEmpty()) {
					action_select_inner_image[ii]->setText(tmps);
					action_select_inner_image[ii]->setVisible(true);
				} else {
					action_select_inner_image[ii]->setText(QString::fromUtf8(""));
					action_select_inner_image[ii]->setVisible(false);
				}					
			} else {
				action_select_inner_image[ii]->setText(QString::fromUtf8(""));
				action_select_inner_image[ii]->setVisible(false);
			}					
		}
	}
}

void Menu_MetaClass::create_pulldown_menu_sub(void)
{
	action_insert = new Action_Control(p_wid);
	action_insert->setObjectName(QString::fromUtf8("action_insert_") + object_desc);
	action_insert->binds->setDrive(media_drive);
	connect(action_insert, SIGNAL(triggered()), this, SLOT(do_open_dialog()));
	
	action_eject = new Action_Control(p_wid);
	action_eject->setObjectName(QString::fromUtf8("action_eject_") + object_desc);
	action_eject->binds->setDrive(media_drive);
	connect(action_eject, SIGNAL(triggered()), this, SLOT(do_eject_media()));

	
	{
		int ii;
		action_group_recent = new QActionGroup(p_wid);
		action_group_recent->setExclusive(true);
		
		action_recent = new Action_Control(p_wid);
		action_recent->setObjectName(QString::fromUtf8("action_recent_") + object_desc);
		action_recent->binds->setDrive(media_drive);
		action_recent->binds->setNumber(0);
		for(ii = 0; ii < MAX_HISTORY; ii++) {
			tmps = history.value(ii, "");
			action_recent_list[ii] = new Action_Control(p_wid);
			action_recent_list[ii]->binds->setDrive(drv);
			action_recent_list[ii]->binds->setNumber(ii);
			
			action_recent_list[ii]->setText(tmps);
			action_group_recent[drv]->addAction(action_recent_list[ii]);
			if(!tmps.isEmpty()) {
				action_recent_list[ii]->setVisible(true);
			} else {
				action_recent_list[ii]->setVisible(false);
			}			
			connect(action_recent_list[ii], SIGNAL(triggered()),
					action_recent_list[ii]->binds, SLOT(on_recent_media()));
			connect(action_recent_list[ii]->binds, SIGNAL(set_recent_media(int, int)),
					this, SLOT(do_open_recent_media(int, int)));
			action_recent->addAction(action_recent_list[ii]);
		}
	}
	if(use_d88_menus) {
		
		action_group_inner_media = new QActionGroup(p_wid);
		action_group_inner_media->setExclusive(true);
		
		action_inner_media = new Action_Control(p_wid);
		action_inner_media->setObjectName(QString::fromUtf8("action_recent_") + object_desc);
		action_inner_media->binds->setDrive(media_drive);
		action_inner_media->binds->setNumber(0);
		for(ii = 0; ii < D88_BANKS; ii++) {
			tmps = history.value(ii, "");
			action_inner_media_list[ii] = new Action_Control(p_wid);
			action_inner_media_list[ii]->binds->setDrive(drv);
			action_inner_media_list[ii]->binds->setNumber(ii);
			
			action_inner_media_list[ii]->setText(tmps);
			action_group_recent[drv]->addAction(action_inner_media_list[ii]);
			if(!tmps.isEmpty()) {
				action_inner_media_list[ii]->setVisible(true);
			} else {
				action_inner_media_list[ii]->setVisible(false);
			}			
			connect(action_inner_media_list[ii], SIGNAL(triggered()),
					action_inner_media_list[ii]->binds, SLOT(on_d88_media()));
			connect(action_inner_media_list[ii]->binds, SIGNAL(set_d88_slot(int, int)),
					this, SLOT(do_open_recent_media(int, int)));
			action_inner_media->addAction(action_inner_media_list[ii]);
		}
	}
		
	if(use_write_protect) {
		action_group_protect = new QActionGroup(p_wid);
		action_group_protect->setExclusive(true);

		action_write_protect_on = new Action_Control(p_wid);
		action_write_protect_on->setObjectName(QString::fromUtf8("action_write_protect_on_") + object_desc);
		action_write_protect_on->setCheckable(true);
		action_write_protect_on->binds->setDrive(media_drive);
		action_write_protect_on->binds->setNumber(0);
		
		action_write_protect_off = new Action_Control(p_wid);
		action_write_protect_off->setObjectName(QString::fromUtf8("action_write_protect_off_") + object_desc);
		action_write_protect_off->setCheckable(true);
		action_write_protect_off->binds->setDrive(media_drive);
		action_write_protect_off->binds->setNumber(0);
		
		action_group_protect->addAction(action_write_protect_on);
		action_group_protect->addAction(action_write_protect_off);
		connect(action_write_protect_on, SIGNAL(triggered()), this, SLOT(do_write_protect_media()));
		connect(action_write_protect_off, SIGNAL(triggered()), this, SLOT(do_write_unprotect_media()));
	}
}

// This is virtual function, pls.apply
void Menu_MetaClass::create_menu_device_sub(void)
{
	// Create device specific entries.
}

// This is virtual function, pls.apply
void Menu_MetaClass::connect_menu_device_sub(void)
{

	// Do connect!
	connect(this, SIGNAL(sig_open_media(int, QString)), p_wid, SLOT(_open_disk(int, QString)));
	connect(this, SIGNAL(sig_eject_media(int)), p_wid, SLOT(eject_fd(int)));
	connect(this, SIGNAL(sig_write_protect_media(int, bool)), p_wid, SLOT(write_protect_fd(int, bool)));	
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_disk(int, int)));
	connect(this, SIGNAL(sig_set_inner_slot(int, int)), p_wid, SLOT(set_d88_slot(int, int)));
}

void Menu_MetaClass::create_pulldown_menu(void)
{
	int ii;
	// Example:: Disk.
	create_pulldown_menu_sub();
	create_pulldown_menu_device_sub();
	// Create 

	menu_recent = new QMenu(this);
	menu_recent->setObjectName(QString::fromUtf8("menu_recent_") + object_desc);
	this->addAction(menu_recent->menuAction());
	if(use_d88_menus) {
		menu_inner_media = new QMenu(this);
		menu_inner_media->setObjectName(QString::fromUtf8("menu_inner_media_") + object_desc);
		for(ii = 0; ii < MAX_D88_BANKS; ii++) menu_inner_media->addAction(action_inner_media_list[ii]);
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
		menu_write_protect->setObjectName(QString::fromUtf8("menu_write_protect_") + object_desc);
		menu_write_protect->addAction(action_write_protect);
	}
	// Belows are setup of menu.
	this->addAction(action_insert);
	this->addAction(action_eject);
	this->addSeparator();
	
	connect_menu_device_sub();
	
	this->addAction(menu_history->menuAction());
	if(use_d88_menus) {
		this->addAction(menu_inner_media->menuAction());
	}
	if(use_write_protect) {
		this-addSeparator();
		this->addAction(menu_write_protect->menuAction());
		menu_write_protect->addAction(action_write_protect_on);
		menu_write_protect->addAction(action_write_protect_off);
	}
}

	
