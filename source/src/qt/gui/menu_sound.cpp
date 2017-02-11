/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Sound Implementations.
 *  History: Jan 14, 2015 : Initial
 */

#include <QImageReader>
#include <QImage>
#include <QMenu>

#include "commonclasses.h"
#include "mainwidget_base.h"
//#include "menuclasses.h"
#include "sound_dialog.h"
#include "menu_flags.h"


// WIP: Will move to another file
const double s_late_table[5] = {0.05, 0.1, 0.2, 0.3, 0.4};
extern USING_FLAGS *using_flags;

void Object_Menu_Control::on_set_freq(void) {
   emit sig_freq(s_num);
}

void Object_Menu_Control::on_set_latency(void) {
   emit sig_latency(s_num);
}


void Ui_MainWindowBase::do_set_sound_strict_rendering(bool f)
{
	if(using_flags != NULL) {
		using_flags->get_config_ptr()->sound_strict_rendering = f;
	}
}

void Ui_MainWindowBase::rise_volume_dialog(void)
{
	Ui_SoundDialog *dlg = new Ui_SoundDialog(using_flags, this);
	QString tmps, s_val;
	float n;
	QIcon  img = QIcon(":/icon_speaker.png");
	
	dlg->setWindowIcon(img);
	this->retranslateVolumeLabels(dlg);

	n = (float)(((using_flags->get_config_ptr()->general_sound_level + 32768) * 1000) / 65535) / 10.0;
	s_val.setNum(n, 'f', 1);
	tmps = QApplication::translate("Ui_SoundDialog", "Set Volume", 0);
	tmps = tmps + QString::fromUtf8(" (") + s_val + QString::fromUtf8("%)");
	dlg->setWindowTitle(tmps);
	dlg->show();
	//dlg->exec();
}

void Ui_MainWindowBase::retranslateVolumeLabels(Ui_SoundDialog *)
{
}

void Ui_MainWindowBase::CreateSoundMenu(void)
{
	if(using_flags->is_without_sound()) return;
	int i;
	//  menuRecord = new QMenu(menuSound);
	//  menuRecord->setObjectName(QString::fromUtf8("menuRecord_Sound"));
  
	menuSound->addAction(actionStart_Record);
	menuSound->addSeparator();
	actionSoundStrictRendering = new Action_Control(this, using_flags);
	actionSoundStrictRendering->setObjectName(QString::fromUtf8("actionSoundStrictRendering"));
	actionSoundStrictRendering->setCheckable(true);
	if(using_flags->get_config_ptr()->sound_strict_rendering) actionSoundStrictRendering->setChecked(true);
	connect(actionSoundStrictRendering, SIGNAL(toggled(bool)),
			this, SLOT(do_set_sound_strict_rendering(bool)));
	menuSound->addAction(actionSoundStrictRendering);
	
	menuOutput_Frequency = new QMenu(menuSound);
	menuOutput_Frequency->setObjectName(QString::fromUtf8("menuOutput_Frequency"));
	menuSound->addAction(menuOutput_Frequency->menuAction());
	menuSound->addSeparator();

	if(using_flags->is_datarec_sound()) {
		actionSoundCMT = new Action_Control(this, using_flags);
		actionSoundCMT->setObjectName(QString::fromUtf8("actionSoundCMT"));
		actionSoundCMT->setCheckable(true);
		if(using_flags->get_config_ptr()->tape_sound != 0) {
			actionSoundCMT->setChecked(true);
		} else {
			actionSoundCMT->setChecked(false);
		}
		connect(actionSoundCMT, SIGNAL(toggled(bool)),
				this, SLOT(set_cmt_sound(bool)));
		menuSound->addAction(actionSoundCMT);
		menuSound->addSeparator();
	}
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
		connect(action_Latency[i], SIGNAL(triggered()),
			action_Latency[i]->binds, SLOT(on_set_latency()));
		connect(action_Latency[i]->binds, SIGNAL(sig_latency(int)),
			this, SLOT(set_latency(int)));
	}
	menuSound->addAction(action_VolumeDialog);
}

void Ui_MainWindowBase::ConfigSoundMenu(void)
{
	if(using_flags->is_without_sound()) return;
	int i;
	QString tmps;
	double dval;
	//int freq = 48000;
	actionGroup_Sound_Freq = new QActionGroup(this);
	actionGroup_Sound_Freq->setExclusive(true);
	for(i = 0; i < 8; i++) {
		action_Freq[i] = new Action_Control(this, using_flags);
		tmps.setNum(using_flags->get_s_freq_table(i));
		tmps = QString::fromUtf8("action") + tmps + QString::fromUtf8("Hz");
		action_Freq[i]->setObjectName(tmps);
		action_Freq[i]->setCheckable(true);
		action_Freq[i]->binds->setNumber(i);
		if(i == using_flags->get_config_ptr()->sound_frequency) {
			action_Freq[i]->setChecked(true);
			//freq = using_flags->get_s_freq_table(i);
		}
		actionGroup_Sound_Freq->addAction(action_Freq[i]);
	}
	actionGroup_Sound_Latency = new QActionGroup(this);
	actionGroup_Sound_Latency->setExclusive(true);

	for(i = 0; i < 5; i++) {
		action_Latency[i] = new Action_Control(this, using_flags);
		dval = s_late_table[i];
		dval = dval * 1000.0;
		tmps.setNum((int)dval);
		tmps = QString::fromUtf8("action") + tmps + QString::fromUtf8("ms");
		action_Latency[i]->setObjectName(tmps);
		action_Latency[i]->setCheckable(true);
		action_Latency[i]->binds->setNumber(i);
		if(i == using_flags->get_config_ptr()->sound_latency) action_Latency[i]->setChecked(true);
		actionGroup_Sound_Latency->addAction(action_Latency[i]);
	}

	actionStart_Record = new Action_Control(this, using_flags);
	actionStart_Record->setObjectName(QString::fromUtf8("actionStart_Record"));
	actionStart_Record->setCheckable(true);
	actionStart_Record->setChecked(false);
	connect(actionStart_Record, SIGNAL(toggled(bool)), this, SLOT(start_record_sound(bool)));

	action_VolumeDialog = new Action_Control(this, using_flags);
	connect(action_VolumeDialog, SIGNAL(triggered()), this, SLOT(rise_volume_dialog()));
	action_VolumeDialog->setObjectName(QString::fromUtf8("actionVolumedialog"));
}

void Ui_MainWindowBase::do_update_volume(int level)
{
	if(using_flags->is_without_sound()) return;
	if(level <= -32768) {
		action_VolumeDialog->setIcon(VolumeMutedIcon);
	} else if(level < -4096) {
		action_VolumeDialog->setIcon(VolumeLowIcon);
	} else if(level < 16384) {
		action_VolumeDialog->setIcon(VolumeMidIcon);
	} else {
		action_VolumeDialog->setIcon(VolumeHighIcon);
	}
}
	
void Ui_MainWindowBase::retranslateSoundMenu(void)
{
	int i;
	QString tmps;
	double dval;
	if(using_flags->is_without_sound()) return;
  
	for(i = 0; i < 8; i++) {
		tmps.setNum(using_flags->get_s_freq_table(i));
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
	actionStart_Record->setIcon(RecordSoundIcon);
	actionStart_Record->setText(QApplication::translate("MainWindow", "Start Recording Sound", 0));
	actionStart_Record->setToolTip(QApplication::translate("MainWindow", "Record sound as WAV file.", 0));
	
	if(using_flags->is_datarec_sound()) {
		actionSoundCMT->setText(QApplication::translate("MainWindow", "Sound CMT", 0));
		actionSoundCMT->setToolTip(QApplication::translate("MainWindow", "Enable sound of CMT TAPE recorder.", 0));
	}
	actionSoundStrictRendering->setText(QApplication::translate("MainWindow", "Strict Rendering", 0));
	actionSoundStrictRendering->setToolTip(QApplication::translate("MainWindow", "Rendering per a sample.Select to slower, but accurate rendering sound.", 0));
	
	menuSound->setTitle(QApplication::translate("MainWindow", "Sound", 0));
	menuOutput_Frequency->setTitle(QApplication::translate("MainWindow", "Output Frequency", 0));
	menuSound_Latency->setTitle(QApplication::translate("MainWindow", "Sound Latency", 0));
	
	action_VolumeDialog->setText(QApplication::translate("MainWindow", "Set Volumes", 0));
	action_VolumeDialog->setToolTip(QApplication::translate("MainWindow", "Open a VOLUME dialog.", 0));

	menuSound->setToolTipsVisible(true);
	do_update_volume(using_flags->get_config_ptr()->general_sound_level);
}
