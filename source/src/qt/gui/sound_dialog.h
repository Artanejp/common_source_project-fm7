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

#include "common.h"
#include "config.h"
#include "emu.h"

#include "qt_main.h"
#include "commonclasses.h"

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
signals:	
	int sig_emu_update_config(void);
};

class Ui_SoundDialog : public QWidget
{
	Q_OBJECT
private:
	EMU *p_emu;
	QWidget *parent_widget;
	QVBoxLayout *VBoxWindow;
	QHBoxLayout *HBoxWindow;
	
protected:
	Ui_SndSliderObject *sliderMasterVolume;
	QGroupBox *boxMasterVolume;
	QHBoxLayout *HBoxMasterVolume;
#ifdef USE_MULTIPLE_SOUNDCARDS
	Ui_SndSliderObject *sliderDeviceVolume[USE_MULTIPLE_SOUNDCARDS];
	QGroupBox *boxDeviceVolume[USE_MULTIPLE_SOUNDCARDS];
	QHBoxLayout *HBoxDeviceVolume[USE_MULTIPLE_SOUNDCARDS];
#endif
	QPushButton *closeButton;
public:
	Ui_SoundDialog(EMU *_emu, QWidget *parent = 0);
	~Ui_SoundDialog();
	void setDeviceLabel(int num, QString s);
	void setSliderVisible(int num, bool flag);
public slots:
	void do_emu_update_config();
signals:
};
QT_END_NAMESPACE
#endif //_CSP_QT_SOUND_DIALOG_H
