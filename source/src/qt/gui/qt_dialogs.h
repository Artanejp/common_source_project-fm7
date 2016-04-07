/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.01.07

	[Qt dialogs]
*/

#ifndef _CSP_QT_DIALOGS_H
#define _CSP_QT_DIALOGS_H
#if defined(_USE_QT5)
#include <QFileDialog>
#else
#include <QtGui/QFileDialog>
#endif
//#include "emu.h"
#include "qt_main.h"

typedef class CSP_DiskParams : public QObject
{   
Q_OBJECT
Q_DISABLE_COPY(CSP_DiskParams)
public:
//   explicit CSP_DiskParams(QObject *parent = 0);
	CSP_DiskParams(QObject *parent = 0) : QObject(parent){
		play = true;
		drive = 0;
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
signals:
	int do_open_disk(int, QString);
	int do_close_disk(int);
	int sig_open_cart(int, QString);
	int do_close_cart(int);
	int do_open_cmt(bool, QString);
	int do_close_cmt();
	int sig_open_binary_file(int, QString, bool);
	int do_open_quick_disk(int, QString);
	public slots:
	void _open_disk(const QString fname);
	void _open_cart(const QString fname);
	void _open_cmt(const QString fname);
	void _open_binary(QString);
	void _open_quick_disk(QString);
private:
	int drive;
	bool play;
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
} CSP_DiskDialog;

#endif //End.
