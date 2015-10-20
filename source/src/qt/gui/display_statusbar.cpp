/*
 * Common Source code project : GUI
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *     License : GPLv2
 *     History:
 *      Jan 14, 2015 : Initial
 *
 * [qt -> gui -> status bar]
 */

#include <QtCore/QVariant>
#include <QtGui>
#include <QSize>
#include <QHBoxLayout>
#include <QPainter>
#include <QBrush>
#include <QGraphicsView>
#include <QTransform>

#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"
#include "vm.h"

extern EMU* emu;

void Ui_MainWindow::initStatusBar(void)
{
	int i;
	int wfactor;
	statusUpdateTimer = new QTimer;
	messagesStatusBar = new QLabel;
	//dummyStatusArea1 = new QWidget;
	QSize size1, size2, size3;
	QString tmpstr;
	//   QHBoxLayout *layout = new QHBoxLayout();
	
	//statusbar->addWidget(layout, 0);
	messagesStatusBar->setFixedWidth(400);
	statusbar->addPermanentWidget(messagesStatusBar, 0);
	messagesStatusBar->font().setPointSize(12);
	dummyStatusArea1 = new QWidget;
	statusbar->addPermanentWidget(dummyStatusArea1, 1);
	
#if defined(USE_FD1) && defined(USE_QD1) && defined(USE_TAPE)
	wfactor = (1280 - 400 - 100 - 100) / (MAX_FD + MAX_QD);
#elif defined(USE_FD1) && defined(USE_TAPE)
	wfactor = (1280 - 400 - 100 - 100) / MAX_FD;
#elif defined(USE_QD1) && defined(USE_TAPE)
	wfactor = (1280 - 400 - 100 - 100) / MAX_QD;
#elif defined(USE_FD1)
	wfactor = (1280 - 400 - 100) / MAX_FD;
#elif defined(USE_QD1)
	wfactor = (1280 - 400 - 100) / MAX_QD;
#elif defined(USE_QD1) && defined(USE_FD1)
	wfactor = (1280 - 400 - 100) / (MAX_QD + MAX_FD);
#else
	wfactor = 0;
#endif
#ifdef USE_FD1
	for(i = 0; i < MAX_FD; i++) { // Will Fix
		fd_StatusBar[i] = new QLabel;
		fd_StatusBar[i]->font().setPointSize(12);
		fd_StatusBar[i]->setFixedWidth((wfactor > 200) ? 200 : wfactor);
		//      fd_StatusBar[i]->setAlignment(Qt::AlignRight);
		statusbar->addPermanentWidget(fd_StatusBar[i]);
	}
#endif
#ifdef USE_QD1
	for(i = 0; i < MAX_QD; i++) {
		qd_StatusBar[i] = new QLabel;
		qd_StatusBar[i]->font().setPointSize(12);
		qd_StatusBar[i]->setFixedWidth((wfactor > 150) ? 150 : wfactor);
		//     qd_StatusBar[i]->setAlignment(Qt::AlignRight);
		statusbar->addPermanentWidget(qd_StatusBar[i]);
	}
#endif
#ifdef USE_TAPE
	cmt_StatusBar = new QLabel;
	cmt_StatusBar->setFixedWidth(100);
	cmt_StatusBar->font().setPointSize(12);
	statusbar->addPermanentWidget(cmt_StatusBar);
#endif
	dummyStatusArea2 = new QWidget;
	dummyStatusArea2->setFixedWidth(100);
#ifdef SUPPORT_DUMMY_DEVICE_LED
	for(i = 0; i < SUPPORT_DUMMY_DEVICE_LED; i++) {
		flags_led[i] = false;
		flags_led_bak[i] = false;
	}
	led_graphicsView = new QGraphicsView(dummyStatusArea2);
	
	led_gScene = new QGraphicsScene(0.0f, 0.0f, (float)dummyStatusArea2->width(), (float)dummyStatusArea2->height());
	QPen pen;
	QBrush bbrush(QColor(Qt::black));
	led_graphicsView->setBackgroundBrush(bbrush);
	connect(this, SIGNAL(sig_led_update(QRectF)), led_graphicsView, SLOT(updateSceneRect(QRectF)));
	{
		QBrush rbrush(QColor(Qt::red));
		float bitwidth = (float)dummyStatusArea2->width() / ((float)SUPPORT_DUMMY_DEVICE_LED * 2.0);
		float start = -(float)dummyStatusArea2->width()  / 2.0f + bitwidth * 3.0f;

		pen.setColor(Qt::black);
		led_gScene->addRect(0, 0, 
				    -(float)dummyStatusArea2->width(),
				    (float)dummyStatusArea2->height(),
				    pen, bbrush);
		for(i = 0; i < SUPPORT_DUMMY_DEVICE_LED; i++) {
			led_leds[i] = NULL;
			pen.setColor(Qt::red);
			led_leds[i] = led_gScene->addEllipse(start,
				  (float)dummyStatusArea2->height() / 3.0f,
				   bitwidth - 2.0f, bitwidth - 2.0f,
				   pen, rbrush);
			start = start + bitwidth * 1.5f;
		}
	}
#endif
	statusbar->addPermanentWidget(dummyStatusArea2, 0);
	//   statusbar->addWidget(dummyStatusArea2);
	connect(statusUpdateTimer, SIGNAL(timeout()), this, SLOT(redraw_status_bar()));
	statusUpdateTimer->start(33);
#ifdef SUPPORT_DUMMY_DEVICE_LED
	ledUpdateTimer = new QTimer;
	connect(statusUpdateTimer, SIGNAL(timeout()), this, SLOT(redraw_leds()));
	statusUpdateTimer->start(5);
#endif
}

void Ui_MainWindow::resize_statusbar(int w, int h)
{
	int wfactor;
	QSize nowSize;
	double height, width;
	double scaleFactor;
	int ww;
	int pt;
	int i;
	int qd_width, fd_width;
	int sfactor = 0;;

	nowSize = messagesStatusBar->size();
	height = (double)(nowSize.height());
	width  = (double)(nowSize.width());
	scaleFactor = (double)w / 1280.0;
   
	statusbar->setFixedWidth(w);
	pt = (int)(14.0 * scaleFactor);
	if(pt < 4) pt = 4;
	sfactor = (int)(400.0 * scaleFactor);
	messagesStatusBar->setFixedWidth((int)(400.0 * scaleFactor));
	messagesStatusBar->font().setPointSize(pt);
   
#if defined(USE_FD1) && defined(USE_QD1) && defined(USE_TAPE)
	wfactor = (1280 - 400 - 100 - 100) / (MAX_FD + MAX_QD);
#elif defined(USE_FD1) && defined(USE_TAPE)
	wfactor = (1280 - 400 - 100 - 100) / MAX_FD;
#elif defined(USE_QD1) && defined(USE_TAPE)
	wfactor = (1280 - 400 - 100 - 100) / MAX_QD;
#elif defined(USE_FD1)
	wfactor = (1280 - 400 - 100) / MAX_FD;
#elif defined(USE_QD1)
	wfactor = (1280 - 400 - 100) / MAX_QD;
#elif defined(USE_QD1) && defined(USE_FD1)
	wfactor = (1280 - 400 - 100) / (MAX_QD + MAX_FD);
#else
	wfactor = 100;
#endif
	fd_width = wfactor;
	qd_width = wfactor;
	if(fd_width > 200) fd_width = 200;
	if(fd_width < 50) fd_width = 50;
	if(qd_width > 150) qd_width = 150;
	if(qd_width < 50) qd_width = 50;

#ifdef USE_FD1
	ww = (int)(scaleFactor * (double)fd_width);
	for(i = 0; i < MAX_FD; i++) { // Will Fix
		fd_StatusBar[i]->font().setPointSize(pt);
		fd_StatusBar[i]->setFixedWidth(ww);
		sfactor += ww;
	}
#endif
#ifdef USE_QD1
	ww = (int)(scaleFactor * (double)fd_width);
	for(i = 0; i < MAX_QD; i++) { // Will Fix
		qd_StatusBar[i]->font().setPointSize(pt);
		qd_StatusBar[i]->setFixedWidth(ww);
		sfactor += ww;
	}
#endif
#ifdef USE_TAPE
	cmt_StatusBar->setFixedWidth((int)(100.0 * scaleFactor));
	cmt_StatusBar->font().setPointSize(pt);
	sfactor += (int)(100.0 * scaleFactor);
#endif
#ifdef SUPPORT_DUMMY_DEVICE_LED
	led_graphicsView->setFixedWidth((int)(100.0 * scaleFactor)); 
#endif   
	dummyStatusArea2->setFixedWidth((int)(108.0 * scaleFactor));
	sfactor += (int)(100.0 * scaleFactor);
	sfactor = (int)(1280.0 * scaleFactor) - sfactor;
	if(sfactor > 10) {
		dummyStatusArea1->setVisible(true);
	} else {
		dummyStatusArea1->setVisible(false);
		sfactor = 10;
	}
	dummyStatusArea1->setFixedWidth(sfactor);   
#ifdef SUPPORT_DUMMY_DEVICE_LED
	{
		QPen pen;
		QBrush rbrush(QColor(Qt::red));
		QBrush bbrush(QColor(Qt::black));
		float bitwidth = (float)dummyStatusArea2->width() / ((float)SUPPORT_DUMMY_DEVICE_LED * 2.0);
		float start = -(float)dummyStatusArea2->width()  / 2.0f + bitwidth * 3.0f;

		led_gScene->clear();

		pen.setColor(Qt::black);
		led_gScene->addRect(0, 0, 
				    -(float)dummyStatusArea2->width(),
				    (float)dummyStatusArea2->height(),
				    pen, bbrush);
		for(i = 0; i < SUPPORT_DUMMY_DEVICE_LED; i++) {
			led_leds[i] = NULL;
			pen.setColor(Qt::red);
			led_leds[i] = led_gScene->addEllipse(start,
				  (float)dummyStatusArea2->height() / 3.0f,
				   bitwidth - 2.0f, bitwidth - 2.0f,
				   pen, rbrush);
			start = start + bitwidth * 1.5f;
		}
		//redraw_leds();
	}
#endif
}

#ifdef SUPPORT_DUMMY_DEVICE_LED
void Ui_MainWindow::redraw_leds(void)
{
		uint32 drawflags;
		int i;
		float bitwidth = (float)dummyStatusArea2->width() / ((float)SUPPORT_DUMMY_DEVICE_LED * 2.0);
		float start = -(float)dummyStatusArea2->width() + bitwidth * 4.0f;
		if(emu == NULL) return;
		drawflags = emu->get_led_status();
		
		for(i = 0; i < SUPPORT_DUMMY_DEVICE_LED; i++) {
			flags_led[i] = ((drawflags & (1 << i)) != 0);
			if(led_leds[i] != NULL) {
				if(flags_led[i]) {
					led_leds[i]->setVisible(true);
				} else {
					led_leds[i]->setVisible(false);
				}
			}
			emit sig_led_update(QRectF(start,
					   0.0f, bitwidth * 2.0f, bitwidth * 2.0f));
			start = start + bitwidth * 1.5f;
		}
		led_graphicsView->setScene(led_gScene);
}	
#endif	
	
void Ui_MainWindow::redraw_status_bar(void)
{
	int access_drv;
	int tape_counter;
	QString alamp;
	QString tmpstr;
	QString iname;
	int i;
	if(emu) {
		//     emu->LockVM();
#if defined(USE_QD1)
# if defined(USE_ACCESS_LAMP)      
		access_drv = emu->get_access_lamp();
# endif
		for(i = 0; i < MAX_QD ; i++) {
			if(emu->quickdisk_inserted(i)) {
				//	     printf("%d\n", access_drv);
# if defined(USE_ACCESS_LAMP)      
				if(i == (access_drv - 1)) {
					alamp = QString::fromUtf8("● ");
				} else {
					alamp = QString::fromUtf8("○ ");
				}
				tmpstr = QString::fromUtf8("QD");
				tmpstr = alamp + tmpstr + QString::number(i) + QString::fromUtf8(":");
# else
				tmpstr = QString::fromUtf8("QD");
				tmpstr = tmpstr + QString::number(i) + QString::fromUtf8(":");
# endif
				iname = QString::fromUtf8("*Inserted*");
				tmpstr = tmpstr + iname;
			} else {
				tmpstr = QString::fromUtf8("× QD") + QString::number(i) + QString::fromUtf8(":");
				tmpstr = tmpstr + QString::fromUtf8(" ");
			}
			if(tmpstr != qd_StatusBar[i]->text()) qd_StatusBar[i]->setText(tmpstr);
		}
#endif

#if defined(USE_FD1)
# if defined(USE_ACCESS_LAMP)      
		access_drv = emu->get_access_lamp();
# endif
		for(i = 0; i < MAX_FD; i++) {
			if(emu->disk_inserted(i)) {
# if defined(USE_ACCESS_LAMP)      
				if(i == (access_drv - 1)) {
					alamp = QString::fromUtf8("● ");
				} else {
					alamp = QString::fromUtf8("○ ");
				}
				tmpstr = QString::fromUtf8("FD");
				tmpstr = alamp + tmpstr + QString::number(i) + QString::fromUtf8(":");
# else
				tmpstr = QString::fromUtf8("FD");
				tmpstr = tmpstr + QString::number(i) + QString::fromUtf8(":");
# endif
				if(emu->d88_file[i].bank_num > 0) {
					iname = QString::fromUtf8(emu->d88_file[i].disk_name[emu->d88_file[i].cur_bank]);
				} else {
					iname = QString::fromUtf8("*Inserted*");
				}
				tmpstr = tmpstr + iname;
			} else {
				tmpstr = QString::fromUtf8("× FD") + QString::number(i) + QString::fromUtf8(":");
				tmpstr = tmpstr + QString::fromUtf8(" ");
			}
			if(tmpstr != fd_StatusBar[i]->text()) fd_StatusBar[i]->setText(tmpstr);
		}
#endif

#ifdef USE_TAPE
		if(emu->tape_inserted()) {
# if defined(USE_TAPE_PTR)
			tape_counter = emu->get_tape_ptr();
			if(tape_counter >= 0) {
				tmpstr = QString::fromUtf8("CMT:");
				tmpstr = tmpstr + QString::number(tape_counter) + QString::fromUtf8("%");
			} else {
				tmpstr = QString::fromUtf8("CMT:");
				tmpstr = tmpstr + QString::fromUtf8("TOP");
			}
# else
			tmpstr = QString::fromUtf8("CMT:Inserted");
# endif
			//	 cmt_StatusBar->setText(tmpstr);
		} else {
			tmpstr = QString::fromUtf8("CMT:EMPTY");
		}
		if(tmpstr != cmt_StatusBar->text()) cmt_StatusBar->setText(tmpstr);
#endif
		//      emu->UnlockVM();
	}
}

void Ui_MainWindow::message_status_bar(QString str)
{
	//QString tmpstr;
	if(messagesStatusBar == NULL) return;
	if(str != messagesStatusBar->text()) messagesStatusBar->setText(str);
}
