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

#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"
#include "vm.h"

extern EMU* emu;

void Ui_MainWindow::initStatusBar(void)
{
	int i;
	statusUpdateTimer = new QTimer;
	messagesStatusBar = new QLabel;
	//dummyStatusArea1 = new QWidget;
	QSize size1, size2, size3;
	QString tmpstr;
	//   QHBoxLayout *layout = new QHBoxLayout();
	
	//statusbar->addWidget(layout, 0);
	messagesStatusBar->setFixedWidth(350);
	statusbar->addPermanentWidget(messagesStatusBar, 0);
	dummyStatusArea1 = new QWidget;
	statusbar->addPermanentWidget(dummyStatusArea1, 1);
	//   statusbar->insertStretch(1);
#ifdef USE_FD1
	for(i = 0; i < MAX_FD; i++) { // Will Fix
		fd_StatusBar[i] = new QLabel;
		fd_StatusBar[i]->setFixedWidth(200);
		//      fd_StatusBar[i]->setAlignment(Qt::AlignRight);
		statusbar->addPermanentWidget(fd_StatusBar[i]);
	}
#endif
#ifdef USE_QD1
	for(i = 0; i < MAX_QD; i++) {
		qd_StatusBar[i] = new QLabel;
		qd_StatusBar[i]->setFixedWidth(150);
		//     qd_StatusBar[i]->setAlignment(Qt::AlignRight);
		statusbar->addPermanentWidget(qd_StatusBar[i]);
	}
#endif
#ifdef USE_TAPE
	cmt_StatusBar = new QLabel;
	cmt_StatusBar->setFixedWidth(100);
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

#ifdef SUPPORT_DUMMY_DEVICE_LED
void Ui_MainWindow::redraw_leds(void)
{
		uint32 drawflags;
		QPen pen;
		QBrush rbrush(QColor(Qt::red));
		QBrush bbrush(QColor(Qt::black));
		float bitwidth = (float)dummyStatusArea2->width() / (float)(SUPPORT_DUMMY_DEVICE_LED * 2);
		float start = -(float)dummyStatusArea2->width() + bitwidth * 3.0f;
		int i;
		drawflags = emu->get_led_status();
		
		for(i = 0; i < SUPPORT_DUMMY_DEVICE_LED; i++) {
			flags_led[i] = ((drawflags & (1 << i)) != 0);
			if(flags_led[i] != flags_led_bak[i]) {
				if(flags_led[i]) {
					pen.setColor(Qt::red);
					led_gScene->addEllipse(start,
										   0.0f, bitwidth - 4.0f, bitwidth - 4.0f,
										   pen, rbrush);
				} else {
					pen.setColor(Qt::black);
					led_gScene->addEllipse(start,
										   0.0f, bitwidth - 4.0f, bitwidth - 4.0f,
										   pen, bbrush);
				}
				emit sig_led_update(QRectF(start,
										   0.0f, bitwidth * 2.0f, bitwidth * 2.0f));
			}
			start = start + bitwidth * 1.5f;
			flags_led_bak[i] = flags_led[i];
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
