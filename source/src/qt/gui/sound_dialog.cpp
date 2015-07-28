/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Adjust Volume.
 *  History: Jul 28, 2015 : Initial
 */

#include "menuclasses.h"
#include "sound_dialog.h"

Ui_SndSliderObject::Ui_SndSliderObject(EMU *_emu, Qt::Orientation orientation, QWidget *parent, int num)
	: QSlider(orientation, parent)
{
	p_emu = emu;
	bind_num = num;
	parent_widget = parent;
}

Ui_SndSliderObject::~Ui_SndSliderObject()
{
}

void Ui_SndSliderObject::setValue(int level)
{
	if(level < -32768) level = -32768;
	if(level > 32767)  level = 32767;

	p_emu->LockVM();
	if(bind_num == 0) {
		config.general_sound_level = level;
		p_emu->update_config();
	}
#ifdef USE_MULTIPLE_SOUNDCARDS	
	else {
		if(bind_num <= USE_MULTIPLE_SOUNDCARDS) {
			config.sound_device_level[bind_num - 1] = level;
			p_emu->update_config();
		}
	}
#endif	
	p_emu->UnlockVM();
}		

Ui_SoundDialog::Ui_SoundDialog(EMU *_emu, QWidget *parent) : QWidget(parent)
{
	p_emu = _emu;
	if(parent != NULL) {
		parent_widget = parent;
	} else {
		parent_widget = this;
	}
	VBoxWindow = new QVBoxLayout;
	HBoxWindow = new QHBoxLayout;
	
	sliderMasterVolume = new Ui_SndSliderObject(_emu, Qt::Vertical, this, 0);
	boxMasterVolume = new QGroupBox(QApplication::translate("Ui_SoundDialog", "Main", 0));
	
	sliderMasterVolume->setMinimum(-32768);
	sliderMasterVolume->setMaximum(32768);
	sliderMasterVolume->setSingleStep(128);
	sliderMasterVolume->setPageStep(8192);
	sliderMasterVolume->setValue(config.general_sound_level);
	sliderMasterVolume->connect(sliderMasterVolume, SIGNAL(valueChanged(int)),
								sliderMasterVolume, SLOT(setValue(int)));
	
	HBoxMasterVolume = new QHBoxLayout;
	HBoxMasterVolume->addWidget(sliderMasterVolume);
	boxMasterVolume->setLayout(HBoxMasterVolume);

	HBoxWindow->addWidget(boxMasterVolume);
#ifdef USE_MULTIPLE_SOUNDCARDS
	{
		int ii;
		for(ii = 0; ii < USE_MULTIPLE_SOUNDCARDS; ii++) {
			QString lbl = QApplication::translate("Ui_SoundDialog", "Board", 0);
			QString n_s;
			n_s.setNum(ii + 1);
			lbl = lbl + n_s;
			sliderDeviceVolume[ii] = new Ui_SndSliderObject(_emu, Qt::Vertical, this, ii + 1);
			sliderDeviceVolume[ii]->setMinimum(-32768);
			sliderDeviceVolume[ii]->setMaximum(32768);
			sliderDeviceVolume[ii]->setSingleStep(128);
			sliderDeviceVolume[ii]->setPageStep(8192);
			sliderDeviceVolume[ii]->setValue(config.sound_device_level[ii]);
			sliderDeviceVolume[ii]->connect(sliderDeviceVolume[ii], SIGNAL(valueChanged(int)),
											sliderDeviceVolume[ii], SLOT(setValue(int)));

			boxDeviceVolume[ii] = new QGroupBox(lbl);
			HBoxDeviceVolume[ii] = new QHBoxLayout;
			HBoxDeviceVolume[ii]->addWidget(sliderDeviceVolume[ii]);
			boxDeviceVolume[ii]->setLayout(HBoxDeviceVolume[ii]);
			HBoxWindow->addWidget(boxDeviceVolume[ii]);
		}
	}
#endif
	this->setLayout(HBoxWindow);
}

Ui_SoundDialog::~Ui_SoundDialog()
{
}


void Ui_SoundDialog::setDeviceLabel(int num, QString s)
{
	if(num >= 0) {
		if(num == 0) {
			boxMasterVolume->setTitle(s);
		}
#ifdef USE_MULTIPLE_SOUNDCARDS
		else if(num <= USE_MULTIPLE_SOUNDCARDS){
			boxDeviceVolume[num - 1]->setTitle(s);
		}
#endif
	}
}
