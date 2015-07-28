/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Sound Implementations.
 *  History: Jan 14, 2015 : Initial
 */

#include "menuclasses.h"
#include "sound_dialog.h"

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


#ifdef USE_MULTIPLE_SOUNDCARDS
void Ui_MainWindow::set_multiple_speakers(bool flag) {
	emu->LockVM();
	config.multiple_speakers = flag;
	emu->update_config();
	emu->UnlockVM();
}
#endif

void Ui_MainWindow::rise_volume_dialog(void)
{
	Ui_SoundDialog *dlg = new Ui_SoundDialog(emu);
	dlg->setWindowTitle(QApplication::translate("Ui_SoundDialog", "Set Volume", 0));
	this->retranslateVolumeLabels(dlg);
	dlg->show();
	//dlg->exec();
}

void Ui_MainWindow::retranslateVolumeLabels(Ui_SoundDialog *)
{
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
#ifdef DATAREC_SOUND
	actionSoundCMT = new Action_Control(this);
	actionSoundCMT->setObjectName(QString::fromUtf8("actionSoundCMT"));
	actionSoundCMT->setCheckable(true);
	if(config.tape_sound != 0) {
		actionSoundCMT->setChecked(true);
	} else {
		actionSoundCMT->setChecked(false);
	}
	connect(actionSoundCMT, SIGNAL(toggled(bool)),
		this, SLOT(set_cmt_sound(bool)));
	menuSound->addAction(actionSoundCMT);
	menuSound->addSeparator();
#endif
#ifdef USE_MULTIPLE_SOUNDCARDS
	actionSoundMultipleSpeakers = new Action_Control(this);
	actionSoundMultipleSpeakers->setObjectName(QString::fromUtf8("actionMultipleSpeakers"));
	actionSoundMultipleSpeakers->setCheckable(true);
	if(config.multiple_speakers != 0) {
		actionSoundMultipleSpeakers->setChecked(true);
	} else {
		actionSoundMultipleSpeakers->setChecked(false);
	}
	connect(actionSoundMultipleSpeakers, SIGNAL(toggled(bool)),
		this, SLOT(set_multiple_speakers(bool)));
	menuSound->addAction(actionSoundMultipleSpeakers);
#endif	
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
	menuSound->addAction(action_VolumeDialog);
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

	action_VolumeDialog = new Action_Control(this);
	connect(action_VolumeDialog, SIGNAL(triggered()), this, SLOT(rise_volume_dialog()));
	action_VolumeDialog->setObjectName(QString::fromUtf8("actionVolumedialog"));

}

void Ui_MainWindow::retranslateSoundMenu(void)
{
	int i;
	QString tmps;
	double dval;
  
	for(i = 0; i < 8; i++) {
		tmps.setNum(s_freq_table[i]);
		tmps = tmps + QApplication::translate("MainWindow", "Hz", 0);
		action_Freq[i]->setText(tmps);
	}
	for(i = 0; i < 5; i++) {
		dval = s_late_table[i];
		dval = dval * 1000.0;
		tmps.setNum((int)dval);
		tmps = tmps + QApplication::translate("MainWindow", "mSec", 0);
		action_Latency[i]->setText(tmps);
	}
	actionStart_Record->setText(QApplication::translate("MainWindow", "Start Recording sound", 0));
#ifdef DATAREC_SOUND
	actionSoundCMT->setText(QApplication::translate("MainWindow", "Sound CMT", 0));
#endif
#ifdef USE_MULTIPLE_SOUNDCARDS
	actionSoundMultipleSpeakers->setText(QApplication::translate("MainWindow", "Stereo Speakers", 0));
#endif	
	menuSound->setTitle(QApplication::translate("MainWindow", "Sound", 0));
	menuOutput_Frequency->setTitle(QApplication::translate("MainWindow", "Output Frequency", 0));
	menuSound_Latency->setTitle(QApplication::translate("MainWindow", "Sound Latency", 0));
	action_VolumeDialog->setText(QApplication::translate("MainWindow", "Set Volumes", 0));
}
 
QT_END_NAMESPACE
