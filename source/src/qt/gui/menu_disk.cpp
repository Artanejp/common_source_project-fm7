/*
 * Qt / DIsk Menu, Utilities
 */

#include "menuclasses.h"
#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"

QT_BEGIN_NAMESPACE
void Object_Menu_Control::insert_fd(void) {
   emit sig_insert_fd(drive);
}
void Object_Menu_Control::eject_fd(void) {
   emit sig_eject_fd(drive);
}
void Object_Menu_Control::on_d88_slot(void) {
   emit set_d88_slot(drive, s_num);
}
void Object_Menu_Control::on_recent_disk(void){
   emit set_recent_disk(drive, s_num);
}


#ifdef USE_FD1
int Ui_MainWindow::set_d88_slot(int drive, int num)
{
  if((num < 0) || (num >= MAX_D88_BANKS)) return;
  if(emu && emu->d88_file[drive].cur_bank != num) {
    emu->open_disk(drive, emu->d88_file[drive].path, emu->d88_file[drive].bank[num].offset);
    emu->d88_file[drive].cur_bank = num;
  }
}

int Ui_MainWindow::set_recent_disk(int drive, int num) 
 {
    std::string path;
    int i;
    if((num < 0) || (num >= MAX_HISTORY)) return;
    
    path = config.recent_disk_path[drive][num];
    for(int i = num; i > 0; i--) {
       strcpy(config.recent_disk_path[drive][i], config.recent_disk_path[drive][i - 1]);
    }
    strcpy(config.recent_disk_path[drive][0], path.c_str());
    if(emu) {
       open_disk(drive, path.c_str(), 0);
    }
    for(i = 0; i < MAX_HISTORY; i++) {
       if(action_Recent_List_FD[drive][i] != NULL) { 
	  action_Recent_List_FD[drive][i]->setText(config.recent_disk_path[drive][i]);
	  //emit action_Recent_List_FD[drive][i]->changed();
       }
    }
 }


  

void Ui_MainWindow::open_disk_dialog(int drv)
{
  QString ext = "Disk Images (*.d88,*.d77,*.td0,*.imd,*.dsk,*.fdi,*.hdm,*.tfd,*.xdf,*.2d,*.sf7)";
  QString desc = "Floppy Disk";
  CSP_DiskDialog dlg;
  QString dirname;
  
  if(config.initial_disk_dir != NULL) {
    dirname = config.initial_disk_dir;	        
  } else {
    char app[PATH_MAX];
    QDir df;
    dirname = df.currentPath();
    strncpy(app, dirname.toUtf8().constData(), PATH_MAX);
    dirname = get_parent_dir(app);
  }
  QStringList filter(ext);
  dlg.param->setDrive(drv);
  dlg.setDirectory(dirname);
//  dlg.setNameFilters(filter); 
  QObject::connect(&dlg, SIGNAL(fileSelected(QString)), dlg.param, SLOT(_open_disk(QString))); 
  QObject::connect(dlg.param, SIGNAL(do_open_disk(int, QString)), this, SLOT(_open_disk(int, QString))); 
  dlg.show();
  dlg.exec();
  return;
}

#endif

void Ui_MainWindow::_open_disk(int drv, const QString fname)
{
   char path_shadow[PATH_MAX];
   int i;

#ifdef USE_FD1
   
   if(fname.length() <= 0) return;
   strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
   UPDATE_HISTORY(path_shadow, config.recent_disk_path[drv]);
//   get_parent_dir(path_shadow);
   strcpy(config.initial_disk_dir, path_shadow);
   // Update List
   open_disk(drv, path_shadow, 0);
   if((actionGroup_D88_Image_FD[drv] != NULL) && (emu != NULL)){
      for(i = 0; i < emu->d88_file[drv].bank_num; i++) {
	     if(action_D88_ListImage_FD[drv][i] != NULL) { 
		action_D88_ListImage_FD[drv][i]->setText(emu->d88_file[drv].bank[i].name);
		action_D88_ListImage_FD[drv][i]->setVisible(true);
		//emit action_D88_ListImage_FD[drv][i]->changed();
	     }
      }
      for(; i < MAX_D88_BANKS; i++) {
	     if(action_D88_ListImage_FD[drv][i] != NULL) { 
		//actionSelect_D88_Image_FD[drv][i]->setText(emu->d88_file[drv].bank[i].name);
		action_D88_ListImage_FD[drv][i]->setVisible(false);
		//emit action_D88_ListImage_FD[drv][i]->changed();
	     }
      }
      actionSelect_D88_Image_FD[drv][0].setChecked(true);
   }
   for(i = 0; i < MAX_HISTORY; i++) {
       if(action_Recent_List_FD[drv][i] != NULL) { 
	  action_Recent_List_FD[drv][i]->setText(QString::fromUtf8(config.recent_disk_path[drv][i]));
	  //actiont_Recent_List_FD[drv][i]->changed();
       }
    }

#endif
}

void Ui_MainWindow::_open_cart(int drv, const QString fname)
{
   char path_shadow[PATH_MAX];
#ifdef USE_CART1
   if(fname.length() <= 0) return;
   strncpy(path_shadow, fname.toUtf8().constData(), PATH_MAX);
   UPDATE_HISTORY(path_shadow, config.recent_cart_path[drv]);
   get_parent_dir(path_shadow);
   strcpy(config.initial_cart_dir, path_shadow);

   if(emu) emu->open_cart(drv, path_shadow);
#endif
}

void Ui_MainWindow::_open_cmt(bool mode, const QString path)
{
  char path_shadow[PATH_MAX];
  int play;
   
   play = (mode == false)? 0 : 1;
#ifdef USE_TAPE
  if(path.length() <= 0) return;
  strncpy(path_shadow, path.toUtf8().constData(), PATH_MAX);
  UPDATE_HISTORY(path_shadow, config.recent_tape_path);
  get_parent_dir(path_shadow);
  strcpy(config.initial_tape_dir, path_shadow);
   if(play != 0) {
      emu->play_tape(path_shadow);
  } else {
      emu->rec_tape(path_shadow);
  }
#endif
}

void Ui_MainWindow::eject_fd(int drv) 
{
   close_disk(drv);
}



void Ui_MainWindow::ConfigFloppyMenu(Ui_MainWindow *p)
{
   
#if defined(USE_FD1)
        actionInsert_FD1 = new Action_Control(p);
        actionInsert_FD1->setObjectName(QString::fromUtf8("actionInsert_FD1"));
        actionInsert_FD1->binds->setDrive(0);
        actionInsert_FD1->binds->setNumber(0);
   
        actionEject_FD1 = new Action_Control(p);
        actionEject_FD1->setObjectName(QString::fromUtf8("actionEject_FD1"));
        actionEject_FD1->binds->setDrive(0);
        actionEject_FD1->binds->setNumber(0);
   
        actionGroup_Opened_FD[0] = new QActionGroup(p);
        actionRecent_Opened_FD[0] = new Action_Control(p);
	actionRecent_Opened_FD[0]->setObjectName(QString::fromUtf8("actionRecent_Opened_FD1"));
	actionRecent_Opened_FD[0]->binds->setDrive(0);
	actionRecent_Opened_FD[0]->binds->setNumber(0);
        
        {
	   int ii;
	   actionGroup_D88_Image_FD[0] = new QActionGroup(p);
	   actionGroup_D88_Image_FD[0]->setExclusive(true);
	   
	   actionSelect_D88_Image_FD[0] = new Action_Control(p);
	   actionSelect_D88_Image_FD[0]->setObjectName(QString::fromUtf8("actionSelect_D88_Image_FD1"));
	   actionSelect_D88_Image_FD[0]->binds->setDrive(0);
	   actionSelect_D88_Image_FD[0]->binds->setNumber(0);
	   for(ii = 0; ii < MAX_D88_BANKS; ii++) {
		action_D88_ListImage_FD[0][ii] = new Action_Control(p);
	        action_D88_ListImage_FD[0][ii]->binds->setDrive(0);
	        action_D88_ListImage_FD[0][ii]->binds->setNumber(ii);
	        actionGroup_D88_Image_FD[0]->addAction(action_D88_ListImage_FD[0][ii]);
	        connect(action_D88_ListImage_FD[0][ii], SIGNAL(triggered()),
			action_D88_ListImage_FD[0][ii]->binds, SLOT(on_d88_slot()));
	        connect(action_D88_ListImage_FD[0][ii]->binds, SIGNAL(set_d88_slot(int, int)),
			this, SLOT(set_d88_slot(int, int)));
	   }
	}
        {
	   int ii;
	   actionGroup_Opened_FD[0] = new QActionGroup(p);
	   actionGroup_Opened_FD[0]->setExclusive(true);
	   
	   actionRecent_Opened_FD[0] = new Action_Control(p);
	   actionRecent_Opened_FD[0]->setObjectName(QString::fromUtf8("actionSelect_Recent_FD1"));
	   actionRecent_Opened_FD[0]->binds->setDrive(0);
	   actionRecent_Opened_FD[0]->binds->setNumber(0);
	   for(ii = 0; ii < MAX_HISTORY; ii++) {
		action_Recent_List_FD[0][ii] = new Action_Control(p);
	        action_Recent_List_FD[0][ii]->binds->setDrive(0);
	        action_Recent_List_FD[0][ii]->binds->setNumber(ii);
	        action_Recent_List_FD[0][ii]->setText(QString::fromUtf8(config.recent_disk_path[0][ii]));
	        actionGroup_Opened_FD[0]->addAction(action_Recent_List_FD[0][ii]);
	        connect(action_Recent_List_FD[0][ii], SIGNAL(triggered()),
			action_Recent_List_FD[0][ii]->binds, SLOT(on_recent_disk()));
	        connect(action_Recent_List_FD[0][ii]->binds, SIGNAL(set_recent_disk(int, int)),
			this, SLOT(set_recent_disk(int, int)));
	   }
	}
  
        actionProtection_ON_FD1 = new Action_Control(p);
        actionProtection_ON_FD1->setObjectName(QString::fromUtf8("actionProtection_ON_FD1"));
        actionProtection_ON_FD1->setCheckable(true);
        actionProtection_ON_FD1->setChecked(true);
        actionProtection_OFF_FD1 = new Action_Control(p);
        actionProtection_OFF_FD1->setObjectName(QString::fromUtf8("actionProtection_OFF_FD1"));
        actionProtection_OFF_FD1->setCheckable(true);
   
        connect(actionInsert_FD1, SIGNAL(triggered()), actionInsert_FD1->binds, SLOT(insert_fd()));
        connect(actionInsert_FD1->binds, SIGNAL(sig_insert_fd(int)), this, SLOT(open_disk_dialog(int)));
        connect(actionEject_FD1, SIGNAL(triggered()), actionEject_FD1->binds, SLOT(eject_fd()));
        connect(actionEject_FD1->binds, SIGNAL(sig_eject_fd(int)), this, SLOT(eject_fd(int)));
   
#endif
   
#if defined(USE_FD2)
        actionInsert_FD2 = new Action_Control(p);
        actionInsert_FD2->setObjectName(QString::fromUtf8("actionInsert_FD2"));
        actionInsert_FD2->binds->setDrive(1);
        actionInsert_FD2->binds->setNumber(0);
   
        actionEject_FD2 = new Action_Control(p);
        actionEject_FD2->setObjectName(QString::fromUtf8("actionEject_FD2"));
        actionEject_FD2->binds->setDrive(1);
        actionEject_FD2->binds->setNumber(0);

        actionGroup_Opened_FD[1] = new QActionGroup(p);
        actionRecent_Opened_FD[1] = new Action_Control(p);
	actionRecent_Opened_FD[1]->setObjectName(QString::fromUtf8("actionRecent_Opened_FD2"));
	actionRecent_Opened_FD[1]->binds->setDrive(1);
	actionRecent_Opened_FD[1]->binds->setNumber(0);

        {
	   int ii;
	   actionGroup_D88_Image_FD[1] = new QActionGroup(p);
	   actionGroup_D88_Image_FD[1]->setExclusive(true);
	   
	   actionSelect_D88_Image_FD[1] = new Action_Control(p);
	   actionSelect_D88_Image_FD[1]->setObjectName(QString::fromUtf8("actionSelect_D88_Image_FD2"));
	   actionSelect_D88_Image_FD[1]->binds->setDrive(1);
	   actionSelect_D88_Image_FD[1]->binds->setNumber(0);
	   for(ii = 0; ii < MAX_D88_BANKS; ii++) {
		action_D88_ListImage_FD[1][ii] = new Action_Control(p);
	        action_D88_ListImage_FD[1][ii]->binds->setDrive(1);
	        action_D88_ListImage_FD[1][ii]->binds->setNumber(ii);
	        actionGroup_D88_Image_FD[1]->addAction(action_D88_ListImage_FD[0][ii]);
	        connect(action_D88_ListImage_FD[1][ii], SIGNAL(triggered()),
			action_D88_ListImage_FD[1][ii]->binds, SLOT(on_d88_slot()));
	        connect(action_D88_ListImage_FD[1][ii]->binds, SIGNAL(set_d88_slot(int, int)),
			this, SLOT(set_d88_slot(int, int)));
	   }
	}
  
        {
	   int ii;
	   actionGroup_Opened_FD[0] = new QActionGroup(p);
	   actionGroup_Opened_FD[0]->setExclusive(true);
	   
	   actionRecent_Opened_FD[1] = new Action_Control(p);
	   actionRecent_Opened_FD[1]->setObjectName(QString::fromUtf8("actionSelect_Recent_FD2"));
	   actionRecent_Opened_FD[1]->binds->setDrive(1);
	   actionRecent_Opened_FD[1]->binds->setNumber(0);
	   for(ii = 0; ii < MAX_HISTORY; ii++) {
		action_Recent_List_FD[1][ii] = new Action_Control(p);
	        action_Recent_List_FD[1][ii]->binds->setDrive(1);
	        action_Recent_List_FD[1][ii]->binds->setNumber(ii);
	        action_Recent_List_FD[1][ii]->setText(QString::fromUtf8(config.recent_disk_path[1][ii]));
	        actionGroup_Opened_FD[1]->addAction(action_Recent_List_FD[1][ii]);
	        connect(action_Recent_List_FD[1][ii], SIGNAL(triggered()),
			action_Recent_List_FD[1][ii]->binds, SLOT(on_recent_disk()));
	        connect(action_Recent_List_FD[1][ii]->binds, SIGNAL(set_recent_disk(int, int)),
			this, SLOT(set_recent_disk(int, int)));
	   }
	}

        actionProtection_ON_FD2 = new Action_Control(p);
        actionProtection_ON_FD2->setObjectName(QString::fromUtf8("actionProtection_ON_FD2"));
        actionProtection_ON_FD2->setCheckable(true);
        actionProtection_ON_FD2->setChecked(true);
        actionProtection_ON_FD2->binds->setDrive(1);
        actionProtection_ON_FD2->binds->setNumber(0);

        actionProtection_OFF_FD2 = new Action_Control(p);
        actionProtection_OFF_FD2->setObjectName(QString::fromUtf8("actionProtection_OFF_FD2"));
        actionProtection_OFF_FD2->setCheckable(true);
        actionProtection_OFF_FD2->binds->setDrive(1);
        actionProtection_OFF_FD2->binds->setNumber(0);
   
       // Connect
        connect(actionInsert_FD2, SIGNAL(triggered()), actionInsert_FD2->binds, SLOT(insert_fd()));
        connect(actionInsert_FD2->binds, SIGNAL(sig_insert_fd(int)), this, SLOT(open_disk_dialog(int)));
        connect(actionEject_FD2, SIGNAL(triggered()), actionEject_FD2->binds, SLOT(eject_fd()));
        connect(actionEject_FD2->binds, SIGNAL(sig_eject_fd(int)), this, SLOT(eject_fd(int)));
      
#endif
   
}
QT_END_NAMESPACE
