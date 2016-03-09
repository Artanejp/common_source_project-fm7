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
		QString tmps, s_val;
		float n;
		if(level < -32768) level = -32678;
		if(level > 32767)  level = 32767;
		config.general_sound_level = level;
		
		tmps = QApplication::translate("Ui_SoundDialog", "Set Volume", 0);
		n = (float)(((level + 32768) * 1000) / 65535) / 10.0;
		s_val.setNum(n, 'f', 1);
		tmps = tmps + QString::fromUtf8(" (") + s_val + QString::fromUtf8("%)");
		parent_widget->setWindowTitle(tmps);
		emit sig_update_master_volume(level);
		emit sig_emu_update_config();
	}
	QSlider::setValue(level);
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
			emit sig_emu_update_volume_label(bind_num - 1, level);
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

void Ui_SndSliderObject::resetVolumeValue()
{
	if(bind_num <= 0) {
		return;
	}
	this->setValue(-20);
	this->setLevelValue(-20);
}

void Ui_SndSliderObject::resetBalanceValue()
{
	if(bind_num <= 0) {
		return;
	}
	this->setValue(0);
	this->setLevelValue(0);
}

Ui_SoundDialog::Ui_SoundDialog(EMU *_emu, QWidget *parent) : QWidget(0)
{
	p_emu = _emu;
	if(parent != NULL) {
		parent_widget = parent;
	} else {
		parent_widget = this;
	}
	MasterLayout = new QGridLayout;
	
	this->setMinimumWidth(620);
	sliderMasterVolume = new Ui_SndSliderObject(_emu, Qt::Horizontal, this, 0);
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
	VBoxMasterVolume = new QVBoxLayout;
	VBoxMasterVolume->addWidget(sliderMasterVolume);
	boxMasterVolume->setLayout(VBoxMasterVolume);
	connect(sliderMasterVolume, SIGNAL(sig_update_master_volume(int)), parent_widget, SLOT(do_update_volume(int)));

#ifdef USE_SOUND_VOLUME
	MasterLayout->addWidget(boxMasterVolume, 0, 0, 1, 2);
#else
	MasterLayout->addWidget(boxMasterVolume, 0, 0, 1, 2);
#endif	
#ifdef USE_SOUND_VOLUME
	{
		int ii;
		int ij = 0;
		for(ii = 0; ii < USE_SOUND_VOLUME; ii++) {
			QString lbl = QApplication::translate("Ui_SoundDialog", sound_device_caption[ii], 0);
			int l_val = config.sound_volume_l[ii];
			int r_val = config.sound_volume_r[ii];
			
			int s_lvl;
			int s_balance;
			s_lvl = (l_val + r_val) / 2;
			s_balance = -(l_val - r_val) / 2;
			lbl = lbl;
			
			sliderDeviceVolume[ij] = new Ui_SndSliderObject(_emu, Qt::Horizontal, this, ii + 1);
			sliderDeviceVolume[ij]->setMinimum(-40);
			sliderDeviceVolume[ij]->setMaximum(0);
			sliderDeviceVolume[ij]->setSingleStep(1);
			sliderDeviceVolume[ij]->setPageStep(4);

			sliderDeviceVolume[ij]->setValue(s_lvl);
			sliderDeviceVolume[ij]->connect(sliderDeviceVolume[ij], SIGNAL(valueChanged(int)),
											sliderDeviceVolume[ij], SLOT(setLevelValue(int)));
			connect(sliderDeviceVolume[ij], SIGNAL(sig_emu_update_volume_level(int, int)),
					parent_widget, SLOT(do_emu_update_volume_level(int, int)));
			connect(sliderDeviceVolume[ij], SIGNAL(sig_emu_update_volume_label(int, int)),
					this, SLOT(do_update_volume_label(int, int)));
						
			sliderDeviceVolume[ij + 1] = new Ui_SndSliderObject(_emu, Qt::Horizontal, this, ii + 1);
			sliderDeviceVolume[ij + 1]->setMinimum(-20);
			sliderDeviceVolume[ij + 1]->setMaximum(20);
			sliderDeviceVolume[ij + 1]->setSingleStep(1);
			sliderDeviceVolume[ij + 1]->setPageStep(4);
			
			sliderDeviceVolume[ij + 1]->setValue(s_balance);
			sliderDeviceVolume[ij + 1]->connect(sliderDeviceVolume[ij + 1], SIGNAL(valueChanged(int)),
												sliderDeviceVolume[ij + 1], SLOT(setBalanceValue(int)));
			connect(sliderDeviceVolume[ij + 1], SIGNAL(sig_emu_update_volume_balance(int, int)),
					parent_widget, SLOT(do_emu_update_volume_balance(int, int)));

			ResetBalance[ii] = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogResetButton), "");
			ResetBalance[ii]->setFlat(true);
			ResetBalance[ii]->setToolTip(QApplication::translate("Ui_SoundDialog", "Reset to center.", 0));
											   
			LabelVolume[ii] = new QLabel(QApplication::translate("Ui_SoundDialog", "Volume", 0));
			LabelBalance[ii] = new QLabel(QApplication::translate("Ui_SoundDialog", "Balance", 0));
			LabelLevel[ii] = new QLabel(QString::fromUtf8("  0db"));			
			LabelLevel[ii]->setMinimumWidth(48);
			LabelLevel[ii]->setAlignment(Qt::AlignRight);
			do_update_volume_label(ii, s_lvl);
			connect(ResetBalance[ii], SIGNAL(pressed()), sliderDeviceVolume[ij + 1] ,SLOT(resetBalanceValue()));
			
			boxDeviceVolume[ii] = new QGroupBox(lbl);
			LayoutDeviceVolume[ii] = new QGridLayout;
			
			LayoutDeviceVolume[ii]->addWidget(LabelVolume[ii], 0, 0);
			LayoutDeviceVolume[ii]->addWidget(LabelBalance[ii], 1, 0);
			LayoutDeviceVolume[ii]->addWidget(LabelLevel[ii], 0, 2);
			LayoutDeviceVolume[ii]->addWidget(ResetBalance[ii], 1, 2);
			LayoutDeviceVolume[ii]->addWidget(sliderDeviceVolume[ij], 0, 1);
			LayoutDeviceVolume[ii]->addWidget(sliderDeviceVolume[ij + 1], 1, 1);

			boxDeviceVolume[ii]->setLayout(LayoutDeviceVolume[ii]);
#if (USE_SOUND_VOLUME >= 2)
			MasterLayout->addWidget(boxDeviceVolume[ii], ii / 2 + 1, ii % 2);
#else
			MasterLayout->addWidget(boxDeviceVolume[ii], ii + 1, 0);
#endif			
			ij += 2;
		}
	}
#endif
	this->setLayout(MasterLayout);
}

Ui_SoundDialog::~Ui_SoundDialog()
{
}

void Ui_SoundDialog::do_update_volume_label(int num, int level)
{
	QString tmps;
	if(LabelLevel[num] != NULL) {
		QString s_val;
		s_val.setNum(level);
		tmps = s_val + QString::fromUtf8("db");
		LabelLevel[num]->setText(tmps);
	}
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
