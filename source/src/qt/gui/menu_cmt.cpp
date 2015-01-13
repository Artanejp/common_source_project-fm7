/*
 * Qt / Tape Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License : GPLv2
 *   History : 
 *     Jan 13 2015 : Start
 */

#include "menuclasses.h"
#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"

QT_BEGIN_NAMESPACE
void Object_Menu_Control::start_insert_play_cmt(void) {
  printf("%d", play);
   emit sig_insert_play_cmt(play);
}
void Object_Menu_Control::eject_cmt(void) {
   emit sig_eject_cmt();
}
void Object_Menu_Control::on_recent_cmt(){
   emit sig_recent_cmt(s_num);
}
void Object_Menu_Control::do_set_write_protect_cmt(void) {
   write_protect = true;
   emit sig_set_write_protect_cmt(write_protect);
}
void Object_Menu_Control::do_unset_write_protect_cmt(void) {
    write_protect = false;
    emit sig_set_write_protect_cmt(write_protect);
}


// Common Routine

#ifdef USE_TAPE  
void Ui_MainWindow::do_write_protect_cmt(bool flag)
{
   write_protect = flag;
}


int Ui_MainWindow::set_recent_cmt(int num) 
 {
    std::string path;
    int i;
    if((num < 0) || (num >= MAX_HISTORY)) return;
    
    path = config.recent_tape_path[num];
    for(int i = num; i > 0; i--) {
       strcpy(config.recent_tape_path[i], config.recent_tape_path[i - 1]);
    }
    strcpy(config.recent_tape_path[0], path.c_str());
    if(emu) {
      AGAR_DebugLog(AGAR_LOG_DEBUG, "Tape: Open READ");
      emu->play_tape(path.c_str()); // Play Readonly, to safety.
    }
    for(i = 0; i < MAX_HISTORY; i++) {
       if(action_Recent_List_CMT[i] != NULL) { 
	  action_Recent_List_CMT[i]->setText(config.recent_tape_path[i]);
	  //emit action_Recent_List_FD[drive][i]->changed();
       }
    }
 }


void Ui_MainWindow::open_cmt_dialog(bool play)
{
  QString ext;
  QString desc1;
  QString desc2;
  CSP_DiskDialog dlg;
  QString dirname;
  
#if defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
  ext = "*.wav *.p6 *.cas";
#elif defined(_PC8001SR) || defined(_PC8801MA) || defined(_PC98DO)
  ext = play ? "*.cas *.cmt *.n80 *.t88" : "*.cas *.cmt";
#elif defined(_MZ80A) || defined(_MZ80K) || defined(_MZ1200) || defined(_MZ700) || defined(_MZ800) || defined(_MZ1500)
  ext = play ? "*.wav *.cas *.mzt *.m12" :"*.wav *.cas";
#elif defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
  ext = play ? "*.wav *.cas *.mzt *.mti *.mtw *.dat" : "*.wav *.cas";
#elif defined(_X1) || defined(_X1TWIN) || defined(_X1TURBO) || defined(_X1TURBOZ)
  ext = play ? "*.wav *.cas *.tap" : "*.wav *.cas";
#elif defined(_FM7) || defined(_FM77) || defined(_FM77AV) || defined(_FM77AV40)
  ext = "*.wav *.t77";
#elif defined(TAPE_BINARY_ONLY)
  ext = "*.cas *.cmt";
#else
  ext = "*.wav;*.cas";
#endif
  desc1 = play ? "Data Recorder Tape [Play]" : "Data Recorder Tape [Rec]";
  if(play) {
    dlg.setWindowTitle("Open Tape");
  } else {
    dlg.setWindowTitle("Record Tape");
  }
  desc2 = desc1 + " (" + ext.toLower() + ")";
  desc1 = desc1 + " (" + ext.toUpper() + ")";
  if(config.initial_tape_dir != NULL) {
    dirname = config.initial_tape_dir;	        
  } else {
    char app[PATH_MAX];
    QDir df;
    dirname = df.currentPath();
    strncpy(app, dirname.toUtf8().constData(), PATH_MAX);
    dirname = get_parent_dir(app);
  }
  QStringList filter;
  filter << desc1 << desc2;
  dlg.param->setRecMode(play);
  dlg.setNameFilters(filter); 
  QObject::connect(&dlg, SIGNAL(fileSelected(QString)), dlg.param, SLOT(_open_cmt(QString))); 
  QObject::connect(dlg.param, SIGNAL(do_open_cmt(bool, QString)), this, SLOT(_open_cmt(bool, QString))); 
  dlg.show();
  dlg.exec();
  return;
}

#endif

void Ui_MainWindow::set_wave_shaper(bool f)
{
  config.wave_shaper = f;
}
bool Ui_MainWindow::get_wave_shaper(void)
{
  if(config.wave_shaper == 0) return false;
  return true;
}

void Ui_MainWindow::set_direct_load_from_mzt(bool f)
{
  config.direct_load_mzt = f;
}
bool Ui_MainWindow::get_direct_load_mzt(void)
{
  if(config.direct_load_mzt == 0) return false;
  return true;
}

#ifdef USE_TAPE_BUTTON
void Ui_MainWindow::OnPushPlayButton(void)
{
  if(emu) emu->push_play();
}
void Ui_MainWindow::OnPushStopButton(void)
{
  if(emu) emu->push_stop();
}

#endif

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
  // Copy filename again.
  strncpy(path_shadow, path.toUtf8().constData(), PATH_MAX);
  if((play != false) || (write_protect != false)) {
    AGAR_DebugLog(AGAR_LOG_DEBUG, "Tape: Open READ : filename = %s", path_shadow);
      emu->play_tape(path_shadow);
  } else {
    AGAR_DebugLog(AGAR_LOG_DEBUG, "Tape: Open Write : filename = %s", path_shadow);
     emu->rec_tape(path_shadow);
  }
#endif
}

void Ui_MainWindow::eject_cmt(void) 
{
  if(emu) emu->close_tape();
}

void Ui_MainWindow::CreateCMTMenu(Ui_MainWindow *p)
{
  menuCMT = new QMenu(menubar);
  menuCMT->setObjectName(QString::fromUtf8("menuCMT", -1));
  menuWrite_Protection_CMT = new QMenu(menuCMT);
  menuWrite_Protection_CMT->setObjectName(QString::fromUtf8("menuWrite_Protection_CMT", -1));
  //CreateCMTPulldownMenu(p);
}

void Ui_MainWindow::CreateCMTPulldownMenu(Ui_MainWindow *p)
{
  
  menuCMT->addAction(actionInsert_CMT);
  menuCMT->addAction(actionEject_CMT);
  menuCMT->addSeparator();
  menuCMT->addAction(actionPlay_Start);
  menuCMT->addAction(actionPlay_Stop);
  
  menuCMT->addSeparator();
  menuCMT_Recent = new QMenu(menuCMT);
  menuCMT_Recent->setObjectName(QString::fromUtf8("Recent_CMT", -1));
  menuCMT->addAction(menuCMT_Recent->menuAction());
  //        menuCMT->addAction(actionRecent_Opened_FD[0]);
  {
    int ii;
    for(ii = 0; ii < MAX_HISTORY; ii++) {
      menuCMT_Recent->addAction(action_Recent_List_CMT[ii]);
      action_Recent_List_CMT[ii]->setVisible(true);
    }
    
  }
  menuCMT->addSeparator();
  menuCMT->addAction(menuWrite_Protection_CMT->menuAction());
  menuWrite_Protection_CMT->addAction(actionProtection_ON_CMT);
  menuWrite_Protection_CMT->addAction(actionProtection_OFF_CMT);

}

void Ui_MainWindow::ConfigCMTMenuSub(Ui_MainWindow *p)
{
  
  actionInsert_CMT = new Action_Control(p);
  actionInsert_CMT->setObjectName(QString::fromUtf8("actionInsert_CMT"));
  actionInsert_CMT->binds->setPlay(true);
  actionInsert_CMT->binds->setNumber(0);
  
  actionEject_CMT = new Action_Control(p);
  actionEject_CMT->setObjectName(QString::fromUtf8("actionEject_CMT"));
  actionEject_CMT->binds->setPlay(true);

  actionPlay_Start = new Action_Control(p);
  actionPlay_Start->setObjectName(QString::fromUtf8("actionPlay_Start"));
  actionPlay_Start->binds->setPlay(true);

  actionPlay_Stop = new Action_Control(p);
  actionPlay_Stop->setObjectName(QString::fromUtf8("actionPlay_Stop"));
  actionPlay_Stop->binds->setPlay(true);

  actionRecording = new Action_Control(p);
  actionRecording->setObjectName(QString::fromUtf8("actionRecording"));
  actionRecording->binds->setPlay(false);
  actionRecording->binds->setNumber(0);
  
  actionGroup_Opened_CMT = new QActionGroup(p);
  actionRecent_Opened_CMT = new Action_Control(p);
  actionRecent_Opened_CMT->setObjectName(QString::fromUtf8("actionRecent_Opened_CMT"));
  actionRecent_Opened_CMT->binds->setPlay(true);
  
  {
    int ii;
    actionGroup_Opened_CMT = new QActionGroup(p);
    actionGroup_Opened_CMT->setExclusive(true);
    
    actionRecent_Opened_CMT = new Action_Control(p);
    actionRecent_Opened_CMT->setObjectName(QString::fromUtf8("actionSelect_Recent_CMT"));
    actionRecent_Opened_CMT->binds->setPlay(true); // For safety
    for(ii = 0; ii < MAX_HISTORY; ii++) {
      action_Recent_List_CMT[ii] = new Action_Control(p);
      action_Recent_List_CMT[ii]->binds->setPlay(true);
      action_Recent_List_CMT[ii]->binds->setNumber(ii);
      action_Recent_List_CMT[ii]->setText(QString::fromUtf8(config.recent_tape_path[ii]));
      actionGroup_Opened_CMT->addAction(action_Recent_List_CMT[ii]);
      connect(action_Recent_List_CMT[ii], SIGNAL(triggered()),
	      action_Recent_List_CMT[ii]->binds, SLOT(on_recent_cmt()));
      connect(action_Recent_List_CMT[ii]->binds, SIGNAL(sig_recent_cmt(int)),
	      this, SLOT(set_recent_cmt(int)));
    }
  }
  
  {
    int ii;
    actionProtection_ON_CMT = new Action_Control(p);
    actionProtection_ON_CMT->setObjectName(QString::fromUtf8("actionProtection_ON_CMT"));
    actionProtection_ON_CMT->setCheckable(true);
    actionProtection_ON_CMT->setChecked(true);
    actionProtection_OFF_CMT = new Action_Control(p);
    actionProtection_OFF_CMT->setObjectName(QString::fromUtf8("actionProtection_OFF_CMT"));
    actionProtection_OFF_CMT->setCheckable(true);
    actionProtection_OFF_CMT->setChecked(false);
    connect(actionProtection_OFF_CMT, SIGNAL(triggered()), actionProtection_OFF_CMT->binds, SLOT(do_unset_write_protect_cmt()));
    connect(actionProtection_OFF_CMT->binds, SIGNAL(sig_set_write_protect_cmt(bool)), this, SLOT(do_write_protect_cmt(bool)));

    connect(actionProtection_ON_CMT, SIGNAL(triggered()), actionProtection_ON_CMT->binds, SLOT(do_set_write_protect_cmt()));
    connect(actionProtection_ON_CMT->binds, SIGNAL(sig_set_write_protect_cmt(bool)), this, SLOT(do_write_protect_cmt(bool)));

    actionGroup_Protect_CMT = new QActionGroup(p);
    //actionGroup_Protect_CMT->setExclusive(true);
    actionGroup_Protect_CMT->addAction(actionProtection_ON_CMT);
    actionGroup_Protect_CMT->addAction(actionProtection_OFF_CMT);
    actionProtection_ON_CMT->setActionGroup(actionGroup_Protect_CMT);
    actionProtection_OFF_CMT->setActionGroup(actionGroup_Protect_CMT);
    
  }
  
  connect(actionRecording, SIGNAL(triggered()), actionRecording->binds, SLOT(start_insert_play_cmt()));
  connect(actionRecording->binds, SIGNAL(sig_insert_play_cmt(bool)), this, SLOT(open_cmt_dialog(bool)));
  connect(actionInsert_CMT, SIGNAL(triggered()), actionInsert_CMT->binds, SLOT(start_insert_play_cmt()));
  connect(actionInsert_CMT->binds, SIGNAL(sig_insert_play_cmt(bool)), this, SLOT(open_cmt_dialog(bool)));
  connect(actionEject_CMT, SIGNAL(triggered()), actionEject_CMT->binds, SLOT(eject_cmt()));
  // Translate Menu


}

void Ui_MainWindow::retranslateCMTMenu(Ui_MainWindow *p)
{

  actionInsert_CMT->setText(QApplication::translate("MainWindow", "Insert", 0, QApplication::UnicodeUTF8));
  actionEject_CMT->setText(QApplication::translate("MainWindow", "Eject", 0, QApplication::UnicodeUTF8));

  menuCMT_Recent->setTitle(QApplication::translate("MainWindow", "Recent Opened", 0, QApplication::UnicodeUTF8));
  
  actionProtection_ON_CMT->setText(QApplication::translate("MainWindow", "Protection ON", 0, QApplication::UnicodeUTF8));
  actionProtection_OFF_CMT->setText(QApplication::translate("MainWindow", "Protection OFF", 0, QApplication::UnicodeUTF8));

  menuCMT->setTitle(QApplication::translate("MainWindow", "Casette tape" , 0, QApplication::UnicodeUTF8));
  menuWrite_Protection_CMT->setTitle(QApplication::translate("MainWindow", "Write Protection", 0, QApplication::UnicodeUTF8));
}
								 



void Ui_MainWindow::ConfigCMTMenu(Ui_MainWindow *p)
{
  
#if defined(USE_TAPE)
  write_protect = true;
  ConfigCMTMenuSub(p); 
#endif
   
}
QT_END_NAMESPACE
