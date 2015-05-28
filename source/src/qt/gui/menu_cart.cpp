/*
 * UI->Qt->MainWindow : Cart Menu.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */

#include "menuclasses.h"
#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"


#ifdef USE_CART1
void Object_Menu_Control::insert_cart(void) {
	emit sig_insert_cart(getDrive());
}
void Object_Menu_Control::eject_cart(void) {
	write_protect = false;
	emit sig_eject_cart(getDrive());
}
void Object_Menu_Control::on_recent_cart(void){
	emit set_recent_cart(drive, s_num);
}
#endif

QT_BEGIN_NAMESPACE
#if defined(USE_CART1) || defined(USE_CART2)

void Ui_MainWindow::open_cart_dialog(int drive)
{
	QString ext;
	QString desc1;
	QString desc2;
	CSP_DiskDialog dlg;
	QString dirname;
#if defined(_GAMEGEAR)
	ext = "*.rom *.bin *.gg *.col";
	desc1 = "Game Cartridge";
#elif defined(_MASTERSYSTEM)
	ext = "*.rom *.bin *.sms";
	desc1 = "Game Cartridge";
#elif defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
	ext = "*.rom *.bin *.60";
	desc1 = "Game Cartridge";
#elif defined(_PCENGINE) || defined(_X1TWIN)
	ext = "*.rom *.bin *.pce";
	desc1 = "HuCARD";
#else
	ext = "*.rom *.bin";
	desc1 = "Game Cartridge";
#endif
	desc2.number(drive + 1);
	desc2 = QString::fromUtf8("Open Cartridge on #") + desc2;
	dlg.setWindowTitle(desc2);
	
	desc2 = desc1 + " (" + ext.toLower() + ")";
	desc1 = desc1 + " (" + ext.toUpper() + ")";
  
	if(config.initial_cart_dir != NULL) {
		dirname = QString::fromUtf8(config.initial_cart_dir);	        
	} else {
		char app[PATH_MAX];
		QDir df;
		dirname = df.currentPath();
		strncpy(app, dirname.toUtf8().constData(), PATH_MAX);
		dirname = get_parent_dir(app);
	}
	QStringList filter;
	filter << desc1 << desc2;
	dlg.param->setDrive(drive);
	dlg.setDirectory(dirname);
	dlg.setNameFilters(filter); 
	QObject::connect(&dlg, SIGNAL(fileSelected(QString)), dlg.param, SLOT(_open_cart(QString))); 
	QObject::connect(dlg.param, SIGNAL(sig_open_cart(int, QString)), this, SLOT(_open_cart(int, QString))); 
	dlg.show();
	dlg.exec();
	return;
}
#endif

void Ui_MainWindow::CreateCartMenu(int drv, int drv_base)
{
#ifdef USE_CART1
	QString drv_base_name = QString::number(drv_base); 
	menuCART[drv] = new QMenu(menubar);
	menuCART[drv]->setObjectName(QString::fromUtf8("menuCART", -1) + drv_base_name);
#endif
}

void Ui_MainWindow::CreateCartPulldownMenu(int drv)
{
#ifdef USE_CART1
	menuCART[drv]->addAction(actionInsert_CART[drv]);
	menuCART[drv]->addAction(actionEject_CART[drv]);
	menuCART[drv]->addSeparator();
	menuCART_Recent[drv] = new QMenu(menuCART[drv]);
	menuCART_Recent[drv]->setObjectName(QString::fromUtf8("Recent_CART", -1) + QString::number(drv));
	menuCART[drv]->addAction(menuCART_Recent[drv]->menuAction());
	{
		int ii;
		for(ii = 0; ii < MAX_HISTORY; ii++) {
			menuCART_Recent[drv]->addAction(action_Recent_List_CART[drv][ii]);
			action_Recent_List_CART[drv][ii]->setVisible(true);
		}
	}
#endif
}

void Ui_MainWindow::ConfigCartMenuSub(int drv)
{
#ifdef USE_CART1
	QString drive_name = QString::number(drv);
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "Create: %d\n", drv);
  
	actionInsert_CART[drv] = new Action_Control(this);
	actionInsert_CART[drv]->setObjectName(QString::fromUtf8("actionInsert_CART") + drive_name);
	actionInsert_CART[drv]->binds->setDrive(drv);
	actionInsert_CART[drv]->binds->setNumber(0);
  
	actionEject_CART[drv] = new Action_Control(this);
	actionEject_CART[drv]->setObjectName(QString::fromUtf8("actionEject_CART") + drive_name);
	actionEject_CART[drv]->binds->setDrive(drv);
	actionEject_CART[drv]->binds->setNumber(0);
  
	actionGroup_Opened_CART[drv] = new QActionGroup(this);
	actionRecent_Opened_CART[drv] = new Action_Control(this);
	actionRecent_Opened_CART[drv]->setObjectName(QString::fromUtf8("actionRecent_Opened_CART") + drive_name);
	actionRecent_Opened_CART[drv]->binds->setDrive(drv);
	actionRecent_Opened_CART[drv]->binds->setNumber(0);
  
	{
		int ii;
		actionGroup_Opened_CART[drv] = new QActionGroup(this);
		actionGroup_Opened_CART[drv]->setExclusive(true);
    
		actionRecent_Opened_CART[drv] = new Action_Control(this);
		actionRecent_Opened_CART[drv]->setObjectName(QString::fromUtf8("actionSelect_Recent_CART") + drive_name);
		actionRecent_Opened_CART[drv]->binds->setDrive(drv);
		actionRecent_Opened_CART[drv]->binds->setNumber(0);
		for(ii = 0; ii < MAX_HISTORY; ii++) {
			action_Recent_List_CART[drv][ii] = new Action_Control(this);
			action_Recent_List_CART[drv][ii]->binds->setDrive(drv);
			action_Recent_List_CART[drv][ii]->binds->setNumber(ii);
			action_Recent_List_CART[drv][ii]->setText(QString::fromUtf8(config.recent_cart_path[drv][ii]));
			actionGroup_Opened_CART[drv]->addAction(action_Recent_List_CART[drv][ii]);
			connect(action_Recent_List_CART[drv][ii], SIGNAL(triggered()),
				action_Recent_List_CART[drv][ii]->binds, SLOT(on_recent_cart()));
			connect(action_Recent_List_CART[drv][ii]->binds, SIGNAL(set_recent_cart(int, int)),
				this, SLOT(set_recent_cart(int, int)));
		}
	}
	connect(actionInsert_CART[drv], SIGNAL(triggered()), actionInsert_CART[drv]->binds, SLOT(insert_cart()));
	connect(actionInsert_CART[drv]->binds, SIGNAL(sig_insert_cart(int)), this, SLOT(open_cart_dialog(int)));
  
	connect(actionEject_CART[drv], SIGNAL(triggered()), actionEject_CART[drv]->binds, SLOT(eject_cart()));
	connect(actionEject_CART[drv]->binds, SIGNAL(sig_eject_cart(int)), this, SLOT(eject_cart(int)));
	// Translate Menu
#endif
}

void Ui_MainWindow::retranslateCartMenu(int drv, int basedrv)
{
#ifdef USE_CART1
	QString drive_name = (QApplication::translate("MainWindow", "Cartridge ", 0, QApplication::UnicodeUTF8));
	drive_name += QString::number(basedrv);
  
	if((drv < 0) || (drv >= 8)) return;
	actionInsert_CART[drv]->setText(QApplication::translate("MainWindow", "Insert", 0, QApplication::UnicodeUTF8));
	actionEject_CART[drv]->setText(QApplication::translate("MainWindow", "Eject", 0, QApplication::UnicodeUTF8));

	menuCART_Recent[drv]->setTitle(QApplication::translate("MainWindow", "Recent Opened", 0, QApplication::UnicodeUTF8));
	menuCART[drv]->setTitle(QApplication::translate("MainWindow", drive_name.toUtf8().constData() , 0, QApplication::UnicodeUTF8));
#endif
}

void Ui_MainWindow::ConfigCartMenu(void)
{
#if defined(USE_CART1)
	ConfigCartMenuSub(0); 
#endif
#if defined(USE_CART2)
	ConfigCartMenuSub(1);
#endif
}

QT_END_NAMESPACE
