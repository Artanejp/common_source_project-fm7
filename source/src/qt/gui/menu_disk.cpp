/*
 * Qt / DIsk Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   LIcense: GPLv2
 *   History: Jan 10, 2015 (MAYBE) : Initial.
 */

#include "menuclasses.h"
#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"

QT_BEGIN_NAMESPACE
void Object_Menu_Control::insert_fd(void) {
	//write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit sig_insert_fd(getDrive());
}
void Object_Menu_Control::eject_fd(void) {
	write_protect = false;
	emit sig_eject_fd(getDrive());
}
void Object_Menu_Control::on_d88_slot(void) {
	emit set_d88_slot(drive, s_num);
}
void Object_Menu_Control::on_recent_disk(void){
  //   write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit set_recent_disk(drive, s_num);
}
void Object_Menu_Control::write_protect_fd(void) {
	write_protect = true;
	emit sig_write_protect_fd(drive, write_protect);
}
void Object_Menu_Control::no_write_protect_fd(void) {
	write_protect = false;
	emit sig_write_protect_fd(drive, write_protect);
}

void Object_Menu_Control::do_set_ignore_crc_error(bool flag)
{
#ifdef USE_FD1
	if(emu) {
		config.ignore_crc[drive] = flag;
		emu->LockVM();
		emu->update_config();
		emu->UnlockVM();
	}
#endif   
}

// Common Routine
#ifdef USE_FD1
void Ui_MainWindow::open_disk_dialog(int drv)
{
	QString ext = "*.d88 *.d77 *.td0 *.imd *.dsk *.fdi *.hdm *.tfd *.xdf *.2d *.sf7";
	QString desc1 = "Floppy Disk";
	QString desc2;
	CSP_DiskDialog dlg;
	QString dirname;
	dlg.setWindowTitle("Open Floppy Disk");
  
	desc2 = desc1 + " (" + ext.toLower() + ")";
	desc1 = desc1 + " (" + ext.toUpper() + ")";
	if(config.initial_disk_dir != NULL) {
		dirname = QString::fromUtf8(config.initial_disk_dir);	        
	} else {
		char app[PATH_MAX];
		QDir df;
		dirname = df.currentPath();
		strncpy(app, dirname.toUtf8().constData(), PATH_MAX);
		dirname = get_parent_dir(app);
	}
	QStringList filter;
	filter << desc1 << desc2;

	dlg.param->setDrive(drv);
	dlg.setDirectory(dirname);
	dlg.setNameFilters(filter);
	QObject::connect(&dlg, SIGNAL(fileSelected(QString)),
			 dlg.param, SLOT(_open_disk(QString))); 
	QObject::connect(dlg.param, SIGNAL(do_open_disk(int, QString)),
			 this, SLOT(_open_disk(int, QString))); 
	dlg.show();
	dlg.exec();
	return;
}
#endif

void Ui_MainWindow::CreateFloppyMenu(int drv, int drv_base)
{
#ifdef USE_FD1
	QString drv_base_name = QString::number(drv_base); 
	menuFD[drv] = new QMenu(menubar);
	menuFD[drv]->setObjectName(QString::fromUtf8("menuFD", -1) + drv_base_name);
# if defined(USE_DISK_WRITE_PROTECT)
	menuWrite_Protection_FD[drv] = new QMenu(menuFD[drv]);
	menuWrite_Protection_FD[drv]->setObjectName(QString::fromUtf8("menuWrite_Protection_FD", -1) + drv_base_name);
# endif
#endif
}

void Ui_MainWindow::CreateFloppyPulldownMenu(int drv)
{
#ifdef USE_FD1
	menuFD[drv]->addAction(actionInsert_FD[drv]);
	menuFD[drv]->addAction(actionEject_FD[drv]);
	menuFD[drv]->addSeparator();
	menuFD[drv]->addAction(actionIgnoreCRC[drv]);
	menuFD[drv]->addSeparator();
	menuFD_Recent[drv] = new QMenu(menuFD[drv]);
	menuFD_Recent[drv]->setObjectName(QString::fromUtf8("Recent_FD", -1) + QString::number(drv));
	menuFD[drv]->addAction(menuFD_Recent[drv]->menuAction());
	//        menuFD[drv]->addAction(actionRecent_Opened_FD[0]);
	{
		int ii;
		for(ii = 0; ii < MAX_HISTORY; ii++) {
			menuFD_Recent[drv]->addAction(action_Recent_List_FD[drv][ii]);
			action_Recent_List_FD[drv][ii]->setVisible(true);
		}
	}
	menuFD[drv]->addSeparator();
	menuFD_D88[drv] = new QMenu(menuFD[drv]);
	menuFD_D88[drv]->setObjectName(QString::fromUtf8("D88_FD", -1) + QString::number(drv));
	menuFD[drv]->addAction(menuFD_D88[drv]->menuAction());
	//      menuFD[drv]->addAction(actionSelect_D88_Image_FD[0]);
	{
		int ii;
		for(ii = 0; ii < MAX_D88_BANKS; ii++) {
			menuFD_D88[drv]->addAction(action_D88_ListImage_FD[drv][ii]);
			action_D88_ListImage_FD[drv][ii]->setVisible(false);
		}
	   
	}
# if defined(USE_DISK_WRITE_PROTECT)
	menuFD[drv]->addSeparator();
	menuFD[drv]->addAction(menuWrite_Protection_FD[drv]->menuAction());
	menuWrite_Protection_FD[drv]->addAction(actionProtection_ON_FD[drv]);
	menuWrite_Protection_FD[drv]->addAction(actionProtection_OFF_FD[drv]);
# endif
#endif
}

void Ui_MainWindow::ConfigFloppyMenuSub(int drv)
{
#ifdef USE_FD1
	QString drive_name = QString::number(drv);
  
	actionInsert_FD[drv] = new Action_Control(this);
	actionInsert_FD[drv]->setObjectName(QString::fromUtf8("actionInsert_FD") + drive_name);
	actionInsert_FD[drv]->binds->setDrive(drv);
	actionInsert_FD[drv]->binds->setNumber(0);
  
	actionEject_FD[drv] = new Action_Control(this);
	actionEject_FD[drv]->setObjectName(QString::fromUtf8("actionEject_FD") + drive_name);
	actionEject_FD[drv]->binds->setDrive(drv);
	actionEject_FD[drv]->binds->setNumber(0);
  
	actionEject_FD[drv] = new Action_Control(this);
	actionEject_FD[drv]->setObjectName(QString::fromUtf8("actionEject_FD") + drive_name);
	actionEject_FD[drv]->binds->setDrive(drv);
	actionEject_FD[drv]->binds->setNumber(0);
	
        actionIgnoreCRC[drv] = new Action_Control(this);
	actionIgnoreCRC[drv]->setCheckable(true);
	actionIgnoreCRC[drv]->setVisible(true);
	actionIgnoreCRC[drv]->binds->setDrive(drv);
	actionIgnoreCRC[drv]->binds->setNumber(drv);
	if(config.ignore_crc[drv] == false) {
		actionIgnoreCRC[drv]->setChecked(false);
	} else {
		actionIgnoreCRC[drv]->setChecked(true);
	}
	connect(actionIgnoreCRC[drv], SIGNAL(toggled(bool)),
		actionIgnoreCRC[drv]->binds, SLOT(do_set_ignore_crc_error(bool)));

	actionGroup_Opened_FD[drv] = new QActionGroup(this);
	actionRecent_Opened_FD[drv] = new Action_Control(this);
	actionRecent_Opened_FD[drv]->setObjectName(QString::fromUtf8("actionRecent_Opened_FD") + drive_name);
	actionRecent_Opened_FD[drv]->binds->setDrive(drv);
	actionRecent_Opened_FD[drv]->binds->setNumber(0);
  
	{
		int ii;
		actionGroup_D88_Image_FD[drv] = new QActionGroup(this);
		actionGroup_D88_Image_FD[drv]->setExclusive(true);
  
		actionSelect_D88_Image_FD[drv] = new Action_Control(this);
		actionSelect_D88_Image_FD[drv]->setObjectName(QString::fromUtf8("actionSelect_D88_Image_FD") + drive_name);
		actionSelect_D88_Image_FD[drv]->binds->setDrive(drv);
		actionSelect_D88_Image_FD[drv]->binds->setNumber(0);
		for(ii = 0; ii < MAX_D88_BANKS; ii++) {
			action_D88_ListImage_FD[drv][ii] = new Action_Control(this);
			action_D88_ListImage_FD[drv][ii]->setCheckable(true);
			if(ii == 0) action_D88_ListImage_FD[drv][ii]->setChecked(true);
			action_D88_ListImage_FD[drv][ii]->binds->setDrive(drv);
			action_D88_ListImage_FD[drv][ii]->binds->setNumber(ii);
			actionGroup_D88_Image_FD[drv]->addAction(action_D88_ListImage_FD[drv][ii]);
			connect(action_D88_ListImage_FD[drv][ii], SIGNAL(triggered()),
				action_D88_ListImage_FD[drv][ii]->binds, SLOT(on_d88_slot()));
			connect(action_D88_ListImage_FD[drv][ii]->binds, SIGNAL(set_d88_slot(int, int)),
				this, SLOT(set_d88_slot(int, int)));
		}
	}
	{
		int ii;
		actionGroup_Opened_FD[drv] = new QActionGroup(this);
		actionGroup_Opened_FD[drv]->setExclusive(true);

		actionRecent_Opened_FD[drv] = new Action_Control(this);
		actionRecent_Opened_FD[drv]->setObjectName(QString::fromUtf8("actionSelect_Recent_FD") + drive_name);
		actionRecent_Opened_FD[drv]->binds->setDrive(drv);
		actionRecent_Opened_FD[drv]->binds->setNumber(0);
		for(ii = 0; ii < MAX_HISTORY; ii++) {
			action_Recent_List_FD[drv][ii] = new Action_Control(this);
			action_Recent_List_FD[drv][ii]->binds->setDrive(drv);
			action_Recent_List_FD[drv][ii]->binds->setNumber(ii);
			action_Recent_List_FD[drv][ii]->setText(QString::fromUtf8(config.recent_disk_path[drv][ii]));
			actionGroup_Opened_FD[drv]->addAction(action_Recent_List_FD[drv][ii]);
			connect(action_Recent_List_FD[drv][ii], SIGNAL(triggered()),
				action_Recent_List_FD[drv][ii]->binds, SLOT(on_recent_disk()));
			connect(action_Recent_List_FD[drv][ii]->binds, SIGNAL(set_recent_disk(int, int)),
				this, SLOT(set_recent_disk(int, int)));
		}
	}
# if defined(USE_DISK_WRITE_PROTECT)
	{
		int ii;
		actionGroup_Protect_FD[drv] = new QActionGroup(this);
		actionGroup_Protect_FD[drv]->setExclusive(true);
		actionProtection_ON_FD[drv] = new Action_Control(this);
		actionProtection_ON_FD[drv]->setObjectName(QString::fromUtf8("actionProtection_ON_FD") + drive_name);
		actionProtection_ON_FD[drv]->setCheckable(true);
		actionProtection_ON_FD[drv]->setChecked(true);
		actionProtection_ON_FD[drv]->binds->setDrive(drv);
		actionProtection_ON_FD[drv]->binds->setNumber(0);
		actionProtection_OFF_FD[drv] = new Action_Control(this);
		actionProtection_OFF_FD[drv]->setObjectName(QString::fromUtf8("actionProtection_OFF_FD") + drive_name);
		actionProtection_OFF_FD[drv]->setCheckable(true);
		actionProtection_OFF_FD[drv]->binds->setDrive(drv);
		actionProtection_OFF_FD[drv]->binds->setNumber(0);

		actionGroup_Protect_FD[drv]->addAction(actionProtection_ON_FD[drv]);
		actionGroup_Protect_FD[drv]->addAction(actionProtection_OFF_FD[drv]);
		connect(actionProtection_ON_FD[drv], SIGNAL(triggered()), actionProtection_ON_FD[drv]->binds, SLOT(write_protect_fd()));
		connect(actionProtection_ON_FD[drv]->binds, SIGNAL(sig_write_protect_fd(int, bool)), this, SLOT(write_protect_fd(int, bool)));
		connect(actionProtection_OFF_FD[drv], SIGNAL(triggered()), actionProtection_OFF_FD[drv]->binds, SLOT(no_write_protect_fd()));
		connect(actionProtection_OFF_FD[drv]->binds, SIGNAL(sig_write_protect_fd(int, bool)), this, SLOT(write_protect_fd(int, bool)));

	}
# endif
	connect(actionInsert_FD[drv], SIGNAL(triggered()), actionInsert_FD[drv]->binds, SLOT(insert_fd()));
	connect(actionInsert_FD[drv]->binds, SIGNAL(sig_insert_fd(int)), this, SLOT(open_disk_dialog(int)));
	connect(actionEject_FD[drv], SIGNAL(triggered()), actionEject_FD[drv]->binds, SLOT(eject_fd()));
	connect(actionEject_FD[drv]->binds, SIGNAL(sig_eject_fd(int)), this, SLOT(eject_fd(int)));
	// Translate Menu
#endif
}

void Ui_MainWindow::retranslateFloppyMenu(int drv, int basedrv)
{
#ifdef USE_FD1
	QString drive_name = (QApplication::translate("MainWindow", "Floppy ", 0));
	drive_name += QString::number(basedrv);
  
	if((drv < 0) || (drv >= 8)) return;
	menuFD[drv]->setTitle(QApplication::translate("MainWindow", drive_name.toUtf8().constData() , 0));
	actionInsert_FD[drv]->setText(QApplication::translate("MainWindow", "Insert", 0));
	actionEject_FD[drv]->setText(QApplication::translate("MainWindow", "Eject", 0));
	actionIgnoreCRC[drv]->setText(QApplication::translate("MainWindow", "Ignore CRC Errors", 0));

	menuFD_Recent[drv]->setTitle(QApplication::translate("MainWindow", "Recent Opened", 0));
	menuFD_D88[drv]->setTitle(QApplication::translate("MainWindow", "Select D88 Image", 0));
  
# if defined(USE_DISK_WRITE_PROTECT)
	actionProtection_ON_FD[drv]->setText(QApplication::translate("MainWindow", "Protection ON", 0));
	actionProtection_OFF_FD[drv]->setText(QApplication::translate("MainWindow", "Protection OFF", 0));
	menuWrite_Protection_FD[drv]->setTitle(QApplication::translate("MainWindow", "Write Protection", 0));
# endif
#endif
}

void Ui_MainWindow::ConfigFloppyMenu(void)
{
#if defined(USE_FD1)
	ConfigFloppyMenuSub(0); 
#endif
#if defined(USE_FD2)
	ConfigFloppyMenuSub(1);
#endif
#if defined(USE_FD3)
	ConfigFloppyMenuSub(2);
#endif
#if defined(USE_FD4)
	ConfigFloppyMenuSub(3);
#endif
#if defined(USE_FD5)
	ConfigFloppyMenuSub(4);
#endif
#if defined(USE_FD6)
	ConfigFloppyMenuSub(5);
#endif
#if defined(USE_FD7)
	ConfigFloppyMenuSub(6);
#endif
#if defined(USE_FD8)
	ConfigFloppyMenuSub(7);
#endif
}
QT_END_NAMESPACE
