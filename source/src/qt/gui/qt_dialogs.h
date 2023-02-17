/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.01.07

	[Qt dialogs]
*/

#ifndef _CSP_QT_DIALOGS_H
#define _CSP_QT_DIALOGS_H

#include <QFileDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QApplication>
#include <QWindow>
#include <QStringList>

#include "qt_main.h"

QT_BEGIN_NAMESPACE
typedef class DLL_PREFIX CSP_DiskParams : public QObject
{
Q_OBJECT
Q_DISABLE_COPY(CSP_DiskParams)
public:
//   explicit CSP_DiskParams(QObject *parent = 0);
	CSP_DiskParams(QObject *parent = 0) : QObject(parent){
		play = true;
		drive = 0;
		initial_dir = QString::fromLocal8Bit("");
		ext_filter.clear();
	}
	~CSP_DiskParams() {}
	void setDrive(int num) {drive = num & 7;}
	int getDrive(void) { return drive;}
	void setPlay(bool num) {play = num;}
	bool isPlaying(void) { return play;}
	void setRecMode(bool num) {play = num; }
	int getRecMode(void) {
		if(play) return 1;
		return 0;
	}
	void setDirectory(QString _dir) { initial_dir = _dir; }
	QString getDirectory() { return initial_dir; }
	void setNameFilters(QStringList _name) { ext_filter = _name; }
	QStringList getNameFilters() { return ext_filter; }
signals:
	int sig_open_media(int, QString);

	int sig_open_cart(int, QString);
	int sig_close_cart(int);

	int sig_open_binary_file(int, QString, bool);
public slots:
	void _open_media(const QString fname);

	void _open_cart(const QString fname);

	void _open_binary(QString);

private:
	int drive;
	bool play;
	QString initial_dir;
	QStringList ext_filter;
} CSP_FileParams;

typedef class CSP_DiskDialog : public QFileDialog {
	Q_OBJECT
public:
	CSP_FileParams *param;
	CSP_DiskDialog(QWidget *parent = 0) : QFileDialog(parent) {
		param = new CSP_FileParams();
	}
	~CSP_DiskDialog() {
		delete param;
	}
public slots:
	virtual void open() override;
	virtual void do_update_params();
} CSP_DiskDialog;

class CSP_CreateDiskDialog : public QWidget {
	Q_OBJECT
	quint8 __real_media_type;
	QComboBox media_type;
	QLabel type_label;
	QGridLayout layout;
public:
	QFileDialog* dlg;
	CSP_FileParams *param;
	CSP_CreateDiskDialog(bool *masks, QWidget *parent = 0);
	~CSP_CreateDiskDialog() {
		delete param;
		delete dlg;
	}
signals:
	int sig_create_disk(quint8, QString);
public slots:
	void do_set_type(int i) {
		__real_media_type = media_type.itemData(i).toUInt();
	}
	void do_create_disk(QString s) {
		emit sig_create_disk(__real_media_type, s);
	}
};

class CSP_CreateHardDiskDialog : public QWidget {
	Q_OBJECT
	QComboBox _preset_type;
	QComboBox _sector_size;
	QSpinBox _sectors;
	QSpinBox _surfaces;
	QSpinBox _cylinders;

	QLabel _label_preset_type;
	QLabel _label_sector_size;
	QLabel _label_sectors;
	QLabel _label_surfaces;
	QLabel _label_cylinders;
	QLabel type_label;
	QLabel _size_label_label;
	QLabel _size_label;

	QGridLayout layout;

	int media_drv;
public:
	QFileDialog* dlg;
	CSP_CreateHardDiskDialog(int drive, int secsize, int sectors, int surfaces, int cylinders, QWidget *parent = 0);
	~CSP_CreateHardDiskDialog() {
		delete dlg;
	}
signals:
	int sig_update_total_size(uint64_t);
	int sig_create_disk(int, int, int, int, int, QString);
public slots:
	void do_preset(int num);
	void do_update_total_size(uint64_t size);
	void do_create_disk(QString filename);
	void do_update_values(int dummy);
};

QT_END_NAMESPACE

#endif //End.
