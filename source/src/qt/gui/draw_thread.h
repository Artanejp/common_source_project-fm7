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
#include <QScreen>
#include <QWaitCondition>

#include <SDL.h>

#include "qt_gldraw.h"

class Ui_MainWindowBase;
class EMU;
class OSD;

QT_BEGIN_NAMESPACE

class DrawThreadClass : public QThread {
	Q_OBJECT
 private:
	EMU *p_emu;
	OSD *p_osd;
	Ui_MainWindowBase *MainWindow;
	GLDrawClass *glv;

	qreal refresh_rate;
	qreal wait_refresh;
	qreal wait_count;
	int wait_factor;
	int rec_frame_count;
	int rec_frame_width;
	int rec_frame_height;
	
 protected:
	QScreen *screen;
	int draw_frames;
	bool bRunThread;
	bool bDrawReq;
	bitmap_t *draw_screen_buffer;
	
 public:
	DrawThreadClass(EMU *p, OSD *o, QObject *parent = 0);
	~DrawThreadClass();
	void run() { doWork("");}
	void SetEmu(EMU *p);
public slots:
	void doWork(const QString &);
	void doExit(void);
	void doDraw(bool flag);
	void do_change_refresh_rate(qreal rate);
	void do_update_screen(bitmap_t *p);
	void do_req_encueue_video(int count, int width, int height);
signals:
	int sig_draw_frames(int);
	int message_changed(QString);
	int sig_update_screen(bitmap_t *);
//	int sig_draw_timing(bool);
	int sig_push_frames_to_avio(int, int, int);

};

QT_END_NAMESPACE
#endif
