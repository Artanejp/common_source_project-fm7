/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Adjust Volume.
 *  History: Jul 28, 2015 : Initial
 */

#include "menuclasses.h"
#include "sound_dialog.h"
#include "vm.h"

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
	if(bind_num < 0) return;
	if(bind_num == 0) {
		if(level < -32768) level = -32678;
		if(level > 32768)  level = 32768;

		config.general_sound_level = level;
		emit sig_emu_update_config();
		QSlider::setValue(level);
	}

}		

void Ui_SndSliderObject::setLevelValue(int level)
{
	if(bind_num <= 0) {
		return;
	}
#ifdef USE_SOUND_VOLUME	
	else {
		if(level < -60) level = -60;
		if(level > 3)  level = 3;
		if(bind_num <= USE_SOUND_VOLUME) {
			emit sig_emu_update_volume_level(bind_num - 1, level);
		}
	}
#endif	
	QSlider::setValue(level);
}		

void Ui_SndSliderObject::setBalanceValue(int level)
{
	if(bind_num <= 0) {
		return;
	}
#ifdef USE_SOUND_VOLUME	
	else {
		if(level < -20) level = -20;
		if(level > 20)  level = 20;
		if(bind_num <= USE_SOUND_VOLUME) {
			emit sig_emu_update_volume_balance(bind_num - 1, level);
		}
	}
#endif
	QSlider::setValue(level);
}		

Ui_SoundDialog::Ui_SoundDialog(EMU *_emu, QWidget *parent) : QWidget(0)
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

	connect(sliderMasterVolume, SIGNAL(sig_emu_update_config()),
			parent_widget, SLOT(do_emu_update_config()));

	sliderMasterVolume->setMinimum(-32768);
	sliderMasterVolume->setMaximum(32768);
	sliderMasterVolume->setSingleStep(256);
	sliderMasterVolume->setPageStep(4096);
	sliderMasterVolume->setValue(config.general_sound_level);
	sliderMasterVolume->connect(sliderMasterVolume, SIGNAL(valueChanged(int)),
								sliderMasterVolume, SLOT(setValue(int)));
	
	HBoxMasterVolume = new QHBoxLayout;
	HBoxMasterVolume->addWidget(sliderMasterVolume);
	boxMasterVolume->setLayout(HBoxMasterVolume);

	HBoxWindow->addWidget(boxMasterVolume);
#ifdef USE_SOUND_VOLUME
	{
		int ii;
		int ij = 0;
		for(ii = 0; ii < USE_SOUND_VOLUME; ii++) {
			QString lbl = QApplication::translate("Ui_SoundDialog", sound_device_caption[ii], 0);
			//QString n_s;
			//n_s.setNum(ii + 1);
			lbl = lbl;
			
			sliderDeviceVolume[ij] = new Ui_SndSliderObject(_emu, Qt::Vertical, this, ii + 1);
			sliderDeviceVolume[ij]->setMinimum(-60);
			sliderDeviceVolume[ij]->setMaximum(3);
			sliderDeviceVolume[ij]->setSingleStep(1);
			sliderDeviceVolume[ij]->setPageStep(4);

			sliderDeviceVolume[ij]->setValue((config.sound_volume_l[ii] + config.sound_volume_r[ii]) / 2);
			sliderDeviceVolume[ij]->connect(sliderDeviceVolume[ij], SIGNAL(valueChanged(int)),
											sliderDeviceVolume[ij], SLOT(setLevelValue(int)));
			connect(sliderDeviceVolume[ij], SIGNAL(sig_emu_update_volume_level(int, int)),
					parent_widget, SLOT(do_emu_update_volume_level(int, int)));
			
			sliderDeviceVolume[ij + 1] = new Ui_SndSliderObject(_emu, Qt::Vertical, this, ii + 1);
			sliderDeviceVolume[ij + 1]->setMinimum(-20);
			sliderDeviceVolume[ij + 1]->setMaximum(20);
			sliderDeviceVolume[ij + 1]->setSingleStep(1);
			sliderDeviceVolume[ij + 1]->setPageStep(4);
			
			sliderDeviceVolume[ij + 1]->setValue((config.sound_volume_r[ii] - config.sound_volume_l[ii]) / 2);
			sliderDeviceVolume[ij + 1]->connect(sliderDeviceVolume[ij + 1], SIGNAL(valueChanged(int)),
												sliderDeviceVolume[ij + 1], SLOT(setBalanceValue(int)));
			connect(sliderDeviceVolume[ij + 1], SIGNAL(sig_emu_update_volume_balance(int, int)),
					parent_widget, SLOT(do_emu_update_volume_balance(int, int)));
			
			boxDeviceVolume[ii] = new QGroupBox(lbl);
			HBoxDeviceVolume[ii] = new QHBoxLayout;
			HBoxDeviceVolume[ii]->addWidget(sliderDeviceVolume[ij]);
			HBoxDeviceVolume[ii]->addWidget(sliderDeviceVolume[ij + 1]);

			boxDeviceVolume[ii]->setLayout(HBoxDeviceVolume[ii]);
			HBoxWindow->addWidget(boxDeviceVolume[ii]);
			ij += 2;
		}
	}
#endif
	this->setLayout(HBoxWindow);
}

Ui_SoundDialog::~Ui_SoundDialog()
{
}

void Ui_SoundDialog::do_emu_update_config()
{
	if(p_emu != NULL) {
		p_emu->lock_vm();
		p_emu->update_config();
		p_emu->unlock_vm();
	}
}

void Ui_SoundDialog::setDeviceLabel(int num, QString s)
{
	if(num >= 0) {
		if(num == 0) {
			boxMasterVolume->setTitle(s);
		}
	}
}

void Ui_SoundDialog::setSliderVisible(int num, bool visible)
{
	if(num > 0) {
	}
}
