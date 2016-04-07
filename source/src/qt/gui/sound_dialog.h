/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Adjust Volume.
 *  History: Jul 28, 2015 : Initial
 */

#ifndef _CSP_QT_SOUND_DIALOG_H
#define _CSP_QT_SOUND_DIALOG_H

#include <QApplication>
#include <QString>
#include <QVariant>
#include <QAction>
#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QSlider>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include "common.h"
#include "config.h"
//#include "emu.h"

#include "qt_main.h"
#include "commonclasses.h"
#include "menu_flags.h"

QT_BEGIN_NAMESPACE

//

class Ui_SndSliderObject : public QSlider
{
	Q_OBJECT
private:
	EMU *p_emu;
	QWidget *parent_widget;
	int bind_num;
public:
	Ui_SndSliderObject(EMU *_emu, Qt::Orientation orientation, QWidget *parent, int num = 0);
	~Ui_SndSliderObject();
public slots:
	void setValue(int volume);
	void setLevelValue(int volume);
	void setBalanceValue(int volume);
	void resetVolumeValue(void);
	void resetBalanceValue(void);
signals:	
	int sig_emu_update_config(void);
	int sig_emu_update_volume_level(int, int);
	int sig_emu_update_volume_label(int, int);
	int sig_emu_update_volume_balance(int, int);
	int sig_update_master_volume(int);
};

class Ui_SoundDialog : public QWidget
{
	Q_OBJECT
private:
	EMU *p_emu;
	QWidget *parent_widget;
	QGridLayout *MasterLayout;
protected:
	Ui_SndSliderObject *sliderMasterVolume;
	QGroupBox *boxMasterVolume;
	QVBoxLayout *VBoxMasterVolume;
	Ui_SndSliderObject *sliderDeviceVolume[32 * 2];
	QGroupBox *boxDeviceVolume[32];
	QGridLayout *LayoutDeviceVolume[32];
	QLabel *LabelVolume[32];
	QLabel *LabelBalance[32];
	QLabel *LabelLevel[32];

	QPushButton *ResetBalance[32];
	QPushButton *closeButton;
public:
	Ui_SoundDialog(EMU *_emu, QWidget *parent = 0);
	~Ui_SoundDialog();
	void setDeviceLabel(int num, QString s);
	void setSliderVisible(int num, bool flag);
public slots:
	void do_update_volume_label(int num, int level);
	signals:
};
QT_END_NAMESPACE
#endif //_CSP_QT_SOUND_DIALOG_H
