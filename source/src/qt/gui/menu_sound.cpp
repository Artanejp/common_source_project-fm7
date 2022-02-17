/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Sound Implementations.
 *  History: Jan 14, 2015 : Initial
 */

#include <QImageReader>
#include <QImage>
#include <QMenu>
#include <QActionGroup>

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

void Object_Menu_Control::on_set_host_sound_device(void) {
   emit sig_set_host_sound_device(s_num);
}

void Ui_MainWindowBase::do_set_host_sound_device(int num)
{
	if(p_config != NULL) {
		p_config->sound_device_num = num;
	}
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::do_set_host_sound_name(int num, QString s)
{
	if(num < 0) return;
	if(num >= 16) return;
	
	if(s.isEmpty()) {
		if(action_HostSoundDevice[num] != NULL) {
			action_HostSoundDevice[num]->setVisible(false);
		}
	} else {
		if(action_HostSoundDevice[num] != NULL) {
			action_HostSoundDevice[num]->setVisible(true);
			action_HostSoundDevice[num]->setText(s);
		}
	}
}

void Ui_MainWindowBase::do_set_sound_strict_rendering(bool f)
{
	if(using_flags != NULL) {
		p_config->sound_strict_rendering = f;
	}
}

void Ui_MainWindowBase::do_set_sound_tape_signal(bool f)
{
	if(using_flags != NULL) {
		p_config->sound_tape_signal = f;
	}
}

void Ui_MainWindowBase::do_set_sound_tape_voice(bool f)
{
	if(using_flags != NULL) {
		p_config->sound_tape_voice = f;
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

	n = (float)(((p_config->general_sound_level + 32768) * 1000) / 65535) / 10.0;
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
	SET_ACTION_CHECKABLE_SINGLE_CONNECT(menuSound, actionSoundStrictRendering,
										"actionSoundStrictRendering", p_config->sound_strict_rendering,
										SIGNAL(toggled(bool)), SLOT(do_set_sound_strict_rendering(bool)));
	SET_ACTION_CHECKABLE_SINGLE_CONNECT(menuSound, actionSoundTapeSignal,
										"actionSoundTapeSignal", p_config->sound_tape_signal,
										SIGNAL(toggled(bool)), SLOT(do_set_sound_tape_signal(bool)));
	SET_ACTION_CHECKABLE_SINGLE_CONNECT(menuSound, actionSoundTapeVoice,
										"actionSoundTapeVoice", p_config->sound_tape_voice,
										SIGNAL(toggled(bool)), SLOT(do_set_sound_tape_voice(bool)));
	
	//actionSoundStrictRendering = new Action_Control(this, using_flags);
	//actionSoundStrictRendering->setObjectName(QString::fromUtf8("actionSoundStrictRendering"));
	//actionSoundStrictRendering->setCheckable(true);
	//if(p_config->sound_strict_rendering) actionSoundStrictRendering->setChecked(true);
	//connect(actionSoundStrictRendering, SIGNAL(toggled(bool)),
	//		this, SLOT(do_set_sound_strict_rendering(bool)));
	//menuSound->addAction(actionSoundStrictRendering);

	menuSound_HostDevices = new QMenu(menuSound);
	menuSound_HostDevices->setObjectName(QString::fromUtf8("menuSound_HostDevices"));
	menuSound->addAction(menuSound_HostDevices->menuAction());
	for(i = 0; i < 16; i++) {
		if(action_HostSoundDevice[i] != NULL) {
			menuSound_HostDevices->addAction(action_HostSoundDevice[i]);
			connect(action_HostSoundDevice[i], SIGNAL(triggered()),
					action_HostSoundDevice[i]->binds, SLOT(on_set_host_sound_device()));
			connect(action_HostSoundDevice[i]->binds, SIGNAL(sig_set_host_sound_device(int)),
					this, SLOT(do_set_host_sound_device(int)));
		}
	}
	
	menuSound->addSeparator();
	
	menuOutput_Frequency = new QMenu(menuSound);
	menuOutput_Frequency->setObjectName(QString::fromUtf8("menuOutput_Frequency"));
	menuSound->addAction(menuOutput_Frequency->menuAction());
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

	menuSound->addSeparator();
	if(using_flags->is_use_sound_files_fdd()) {
		SET_ACTION_SINGLE(action_SoundFilesFDD, true, true, (p_config->sound_noise_fdd != 0));
	}
	if(using_flags->is_use_sound_files_relay()) {
		SET_ACTION_SINGLE(action_SoundFilesRelay, true, true, (p_config->sound_noise_cmt != 0));
	}
	if(using_flags->is_use_sound_files_fdd() || using_flags->is_use_sound_files_relay()) {
		if(using_flags->is_use_sound_files_fdd())     menuSound->addAction(action_SoundFilesFDD);
		if(using_flags->is_use_sound_files_relay())   menuSound->addAction(action_SoundFilesRelay);
		menuSound->addSeparator();
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

	actionGroup_Sound_HostDevices = new QActionGroup(this);
	for(i = 0; i < 16; i++) {
		action_HostSoundDevice[i] = new Action_Control(this, using_flags);
		tmps.setNum(i);	
		tmps = QString::fromUtf8("action_HostSoundDevice") + tmps;
	   
		action_HostSoundDevice[i]->setObjectName(tmps);
		action_HostSoundDevice[i]->setCheckable(true);
		action_HostSoundDevice[i]->binds->setNumber(i);
		if(i == p_config->sound_device_num) {
			action_HostSoundDevice[i]->setChecked(true);
			//freq = using_flags->get_s_freq_table(i);
		}
		actionGroup_Sound_HostDevices->addAction(action_HostSoundDevice[i]);
	}
	
	actionGroup_Sound_Freq = new QActionGroup(this);
	actionGroup_Sound_Freq->setExclusive(true);
	for(i = 0; i < 8; i++) {
		action_Freq[i] = new Action_Control(this, using_flags);
		tmps.setNum(using_flags->get_s_freq_table(i));
		tmps = QString::fromUtf8("action") + tmps + QString::fromUtf8("Hz");
		action_Freq[i]->setObjectName(tmps);
		action_Freq[i]->setCheckable(true);
		action_Freq[i]->binds->setNumber(i);
		if(i == p_config->sound_frequency) {
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
		if(i == p_config->sound_latency) action_Latency[i]->setChecked(true);
		actionGroup_Sound_Latency->addAction(action_Latency[i]);
	}

	SET_ACTION_SINGLE(actionStart_Record, true, true, false);
	//actionStart_Record = new Action_Control(this, using_flags);
	//actionStart_Record->setObjectName(QString::fromUtf8("actionStart_Record"));
	//actionStart_Record->setCheckable(true);
	//actionStart_Record->setChecked(false);
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
		tmps = tmps + QApplication::translate("MenuSound", "Hz", 0);
		action_Freq[i]->setText(tmps);
	}
	for(i = 0; i < 5; i++) {
		dval = s_late_table[i];
		dval = dval * 1000.0;
		tmps.setNum((int)dval);
		tmps = tmps + QApplication::translate("MenuSound", "mSec", 0);
		action_Latency[i]->setText(tmps);
	}
	actionStart_Record->setIcon(RecordSoundIcon);
	actionStart_Record->setText(QApplication::translate("MenuSound", "Start Recording Sound", 0));
	actionStart_Record->setToolTip(QApplication::translate("MenuSound", "Record sound as WAV file.", 0));
	
	actionSoundStrictRendering->setText(QApplication::translate("MenuSound", "Strict Rendering", 0));
	actionSoundStrictRendering->setToolTip(QApplication::translate("MenuSound", "Rendering per a sample.Select to slower, but accurate rendering sound.", 0));
	actionSoundTapeSignal->setText(QApplication::translate("MenuSound", "Play CMT Signal", 0));
	actionSoundTapeSignal->setToolTip(QApplication::translate("MenuSound", "Play Signal from CMTs.", 0));

	actionSoundTapeVoice->setText(QApplication::translate("MenuSound", "Play CMT Voice", 0));
	actionSoundTapeVoice->setToolTip(QApplication::translate("MenuSound", "Play Audio/Voice from CMTs.", 0));
	
	if(using_flags->is_tape_binary_only()) {
		actionSoundTapeSignal->setEnabled(false);
		actionSoundTapeVoice->setEnabled(false);
		actionSoundTapeSignal->setVisible(false);
		actionSoundTapeVoice->setVisible(false);
	}

	menuSound_HostDevices->setTitle(QApplication::translate("MenuSound", "Output to:", 0));
	menuSound_HostDevices->setToolTip(QApplication::translate("MenuSound", "Select sound device to output.\nThis effects after re-start this emulator.", 0));
	
	menuSound->setTitle(QApplication::translate("MenuSound", "Sound", 0));
	menuOutput_Frequency->setTitle(QApplication::translate("MenuSound", "Output Frequency", 0));
	menuSound_Latency->setTitle(QApplication::translate("MenuSound", "Sound Latency", 0));
	
	action_VolumeDialog->setText(QApplication::translate("MenuSound", "Set Volumes", 0));
	action_VolumeDialog->setToolTip(QApplication::translate("MenuSound", "Open a VOLUME dialog.", 0));
	if(using_flags->is_use_sound_files_fdd()) {
		action_SoundFilesFDD->setText(QApplication::translate("MenuSound", "Sound FDD Seek", 0));
		action_SoundFilesFDD->setToolTip(QApplication::translate("MenuSound", "Enable FDD HEAD seeking sound.\nNeeds sound file.\nSee HELP->READMEs->Bios and Key assigns", 0));
	}
	if(using_flags->is_use_sound_files_relay()) {
		action_SoundFilesRelay->setText(QApplication::translate("MenuSound", "Sound CMT Relay and Buttons", 0));
		action_SoundFilesRelay->setToolTip(QApplication::translate("MenuSound", "Enable CMT relay's sound and buttons's sounds.\nNeeds sound file.\nSee HELP->READMEs->Bios and Key assigns", 0));
		if(using_flags->is_tape_binary_only()) action_SoundFilesRelay->setEnabled(false);
	}

	menuSound->setToolTipsVisible(true);
	do_update_volume(p_config->general_sound_level);
}
