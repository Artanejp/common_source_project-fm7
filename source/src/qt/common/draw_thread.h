/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	History : 2015.11.10 Split from qt_main.h
	[ win32 main ] -> [ Qt main ] -> [Draw]
*/
#ifndef _CSP_QT_DRAW_THREAD_H
#define _CSP_QT_DRAW_THREAD_H

#include <QThread>
#include <SDL.h>

class Ui_MainWindow;
class EMU;

QT_BEGIN_NAMESPACE

class DrawThreadClass : public QThread {
	Q_OBJECT
 private:
	EMU *p_emu;
	Ui_MainWindow *MainWindow;
 protected:
	int draw_frames;
	bool bRunThread;
 public:
	DrawThreadClass(EMU *p, QObject *parent = 0);
	~DrawThreadClass() {};
	void run() { doWork("");}
	void SetEmu(EMU *p) {
		p_emu = p;
	}
  
public slots:
	void doWork(const QString &);
	void doExit(void);
	void doDraw(void);
signals:
	int sig_draw_frames(int);
	int message_changed(QString);
	int sig_update_screen(QImage *);
	int sig_draw_timing(bool);
};

QT_END_NAMESPACE
#endif
