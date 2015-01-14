/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Sound Implementations.
 *  History: Jan 14, 2015 : Initial
 */

#include "menuclasses.h"

QT_BEGIN_NAMESPACE

const int s_freq_table[8] = {
		2000, 4000, 8000, 11025, 22050, 44100,
#ifdef OVERRIDE_SOUND_FREQ_48000HZ
		OVERRIDE_SOUND_FREQ_48000HZ,
#else
		48000,
#endif
		96000,
};
const double s_late_table[5] = {0.05, 0.1, 0.2, 0.3, 0.4};

void Object_Menu_Control::on_set_freq(void) {
   emit sig_freq(s_num);
}

void Object_Menu_Control::on_set_latency(void) {
   emit sig_latency(s_num);
}

void Ui_MainWindow::set_latency(int num)
{
   if((num < 0) || (num > 4)) return;
   config.sound_latency = num;
   if(emu) {
      emu->LockVM();
      emu->update_config();
      emu->UnlockVM();
   }
}

void Ui_MainWindow::set_freq(int num)
{
   if((num < 0) || (num > 7)) return;
   config.sound_frequency = num;
   if(emu) {
      emu->LockVM();
      emu->update_config();
      emu->UnlockVM();
   }
}

void Ui_MainWindow::set_sound_device(int num)
{
#ifdef USE_SOUND_DEVICE_TYPE
   if((num < 0) || (num >7)) return;
   config.sound_device_type = num;
   if(emu) {
      emu->LockVM();
      emu->update_config();
      emu->UnlockVM();
   }
#endif
}

   
void Ui_MainWindow::start_record_sound(bool start)
{
   if(emu) {
	if(start) {
	   emu->start_rec_sound();
	} else {
	   emu->stop_rec_sound();
	}
   }
}

      

void Ui_MainWindow::CreateSoundMenu(void)
{
  int i;
//  menuRecord = new QMenu(menuSound);
//  menuRecord->setObjectName(QString::fromUtf8("menuRecord_Sound"));
  
  menuSound->addAction(actionStart_Record);
  menuSound->addSeparator();
  menuOutput_Frequency = new QMenu(menuSound);
  menuOutput_Frequency->setObjectName(QString::fromUtf8("menuOutput_Frequency"));
  menuSound->addAction(menuOutput_Frequency->menuAction());
  menuSound->addSeparator();
  for(i = 0; i < 8; i++) {
    menuOutput_Frequency->addAction(action_Freq[i]);
    connect(action_Freq[i], SIGNAL(triggered()),
	    action_Freq[i]->binds, SLOT(on_set_freq()));
    connect(action_Freq[i]->binds, SIGNAL(sig_freq(int)),
	    this, SLOT(set_freq(int)));

  }
  menuSound_Latency = new QMenu(menuSound);
  menuSound_Latency->setObjectName(QString::fromUtf8("menuSound_Latency"));
  menuSound->addAction(menuSound_Latency->menuAction());
  for(i = 0; i < 5; i++) {
    menuSound_Latency->addAction(action_Latency[i]);
  }
} 

void Ui_MainWindow::ConfigSoundMenu(void)
{
  int i;
  QString tmps;
  double dval;
  int freq = 48000;


  actionGroup_Sound_Freq = new QActionGroup(this);
  actionGroup_Sound_Freq->setExclusive(true);
  
  for(i = 0; i < 8; i++) {
	   
	    action_Freq[i] = new Action_Control(this);
	    tmps.setNum(s_freq_table[i]);
	    tmps = QString::fromUtf8("action") + tmps + QString::fromUtf8("Hz");
	    action_Freq[i]->setObjectName(tmps);
	    action_Freq[i]->setCheckable(true);
	    action_Freq[i]->binds->setNumber(i);
	    if(i == config.sound_frequency) {
	      action_Freq[i]->setChecked(true);
	      freq = s_freq_table[i];
	    }
	    actionGroup_Sound_Freq->addAction(action_Freq[i]);
	}
        actionGroup_Sound_Latency = new QActionGroup(this);
	actionGroup_Sound_Latency->setExclusive(true);

	for(i = 0; i < 5; i++) {
	    action_Latency[i] = new Action_Control(this);
	    dval = s_late_table[i];
	    dval = dval * 1000.0;
	    tmps.setNum((int)dval);
	    tmps = QString::fromUtf8("action") + tmps + QString::fromUtf8("ms");
	    action_Latency[i]->setObjectName(tmps);
	    action_Latency[i]->setCheckable(true);
	    action_Latency[i]->binds->setNumber(i);
	    if(i == config.sound_latency) action_Latency[i]->setChecked(true);
	    actionGroup_Sound_Latency->addAction(action_Latency[i]);
	}
	

        actionStart_Record = new Action_Control(this);
        actionStart_Record->setObjectName(QString::fromUtf8("actionStart_Record"));
        actionStart_Record->setCheckable(true);
	actionStart_Record->setChecked(false);
	connect(actionStart_Record, SIGNAL(toggled(bool)), this, SLOT(start_record_sound(bool)));

}

void Ui_MainWindow::retranslateSoundMenu(void)
{
  int i;
  QString tmps;
  double dval;
  
  for(i = 0; i < 8; i++) {
    tmps.setNum(s_freq_table[i]);
    tmps = tmps + QApplication::translate("MainWindow", "Hz", 0, QApplication::UnicodeUTF8);
    action_Freq[i]->setText(tmps);
  }
  for(i = 0; i < 5; i++) {
    dval = s_late_table[i];
    dval = dval * 1000.0;
    tmps.setNum((int)dval);
    tmps = tmps + QApplication::translate("MainWindow", "mSec", 0, QApplication::UnicodeUTF8);
    action_Latency[i]->setText(tmps);
  }
  actionStart_Record->setText(QApplication::translate("MainWindow", "Start Recording sound", 0, QApplication::UnicodeUTF8));

  menuSound->setTitle(QApplication::translate("MainWindow", "Sound", 0, QApplication::UnicodeUTF8));
  menuOutput_Frequency->setTitle(QApplication::translate("MainWindow", "Output Frequency", 0, QApplication::UnicodeUTF8));
  menuSound_Latency->setTitle(QApplication::translate("MainWindow", "Sound Latency", 0, QApplication::UnicodeUTF8));
}

  
QT_END_NAMESPACE
