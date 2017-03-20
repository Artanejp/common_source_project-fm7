/*
 * Common Source code project : GUI
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *     License : GPLv2
 *     History:
 *      Jan 14, 2015 : Initial
 *
 * [qt -> gui -> status bar]
 */

#include <QVariant>
#include <QtGui>
#include <QSize>
#include <QHBoxLayout>
#include <QPainter>
#include <QBrush>
#include <QGraphicsView>
#include <QTransform>
#include <QLabel>
#include <QTimer>
#include <QStatusBar>
#include <QGraphicsEllipseItem>

#include "mainwidget_base.h"
//#include "emu.h"
#include "qt_main.h"
//#include "vm.h"
#include "menu_flags.h"

//extern EMU* emu;
//extern USING_FLAGS *using_flags;

int Ui_MainWindowBase::Calc_OSD_Wfactor()
{
	float wfactor = 0.0;
	if(wfactor < 0.0) wfactor = 0.0;
	return (int)wfactor;
}

void Ui_MainWindowBase::initStatusBar(void)
{
	int i;
	statusUpdateTimer = new QTimer;
	messagesStatusBar = new QLabel;
	//dummyStatusArea1 = new QWidget;
	QSize size1, size2, size3;
	QString tmpstr;
	QString n_s;
	QString tmps_n;
	//   QHBoxLayout *layout = new QHBoxLayout();
	
	//statusbar->addWidget(layout, 0);
	messagesStatusBar->setFixedWidth(600);
	statusbar->addPermanentWidget(messagesStatusBar, 0);
	messagesStatusBar->setStyleSheet("font: 12pt \"Sans\";");
	dummyStatusArea1 = new QWidget;
	statusbar->addPermanentWidget(dummyStatusArea1, 1);
	
	//wfactor = Calc_OSD_Wfactor();
	osd_led_data = 0x00000000;

	tmps_n = QString::fromUtf8("font: ");
	n_s.setNum(12);
	tmps_n = tmps_n + n_s + QString::fromUtf8("pt \"Sans\";");
	
	dummyStatusArea2 = new QWidget;
	dummyStatusArea2->setFixedWidth(100);
	if(using_flags->get_use_led_device() > 0) {
		for(i = 0; i < using_flags->get_use_led_device(); i++) {
			flags_led[i] = false;
			flags_led_bak[i] = false;
		}
		led_graphicsView = new QGraphicsView(dummyStatusArea2);
	
		led_gScene = new QGraphicsScene(0.0f, 0.0f, (float)dummyStatusArea2->width(), (float)dummyStatusArea2->height());
		QPen pen;
		QBrush bbrush = QBrush(QColor(Qt::black));
		led_graphicsView->setBackgroundBrush(bbrush);
		connect(this, SIGNAL(sig_led_update(QRectF)), led_graphicsView, SLOT(updateSceneRect(QRectF)));
		{
			QBrush rbrush = QBrush(QColor(Qt::red));
			float bitwidth = (float)dummyStatusArea2->width() / ((float)using_flags->get_use_led_device() * 2.0);
			float start = -(float)dummyStatusArea2->width()  / 2.0f + bitwidth * 3.0f;
			
			pen.setColor(Qt::black);
			led_gScene->addRect(0, 0, 
								-(float)dummyStatusArea2->width(),
								(float)dummyStatusArea2->height(),
								pen, bbrush);
			for(i = 0; i < using_flags->get_use_led_device(); i++) {
				led_leds[i] = NULL;
				pen.setColor(Qt::red);
				led_leds[i] = led_gScene->addEllipse(start,
													 (float)dummyStatusArea2->height() / 3.0f,
													 bitwidth - 2.0f, bitwidth - 2.0f,
													 pen, rbrush);
				start = start + bitwidth * 1.5f;
			}
		}
	}
	statusbar->addPermanentWidget(dummyStatusArea2, 0);
	//   statusbar->addWidget(dummyStatusArea2);
	connect(statusUpdateTimer, SIGNAL(timeout()), this, SLOT(redraw_status_bar()));
	statusUpdateTimer->start(33);
	if(using_flags->get_use_led_device() > 0) {
		ledUpdateTimer = new QTimer;
		connect(statusUpdateTimer, SIGNAL(timeout()), this, SLOT(redraw_leds()));
		statusUpdateTimer->start(5);
	}
}

void Ui_MainWindowBase::resize_statusbar(int w, int h)
{
	//QSize nowSize;
	//double height, width;
	double scaleFactor;
	int ww;
	int pt;
	int i;
	int sfactor = 0;;
	QString n_s;
	QString tmps;

	//nowSize = messagesStatusBar->size();
	//height = (double)(nowSize.height());
	//width  = (double)(nowSize.width());
	scaleFactor = (double)w / 1280.0;
   
	statusbar->setFixedWidth(w);
	pt = (int)(14.0 * scaleFactor);
	if(pt < 4) pt = 4;
	sfactor = (int)(600.0 * scaleFactor);
	messagesStatusBar->setFixedWidth((int)(600.0 * scaleFactor));
	
	tmps = QString::fromUtf8("font: ");
	n_s.setNum(pt);
	tmps = tmps + n_s + QString::fromUtf8("pt \"Sans\";");
	messagesStatusBar->setStyleSheet(tmps);
   
	if(using_flags->get_use_led_device() > 0) {
		led_graphicsView->setFixedWidth((int)(100.0 * scaleFactor));
	}
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
	if(using_flags->get_use_led_device() > 0) {
		QPen pen;
		QBrush rbrush = QBrush(QColor(Qt::red));
		QBrush bbrush = QBrush(QColor(Qt::black));
		float bitwidth = (float)dummyStatusArea2->width() / ((float)using_flags->get_use_led_device() * 2.0);
		float start = -(float)dummyStatusArea2->width()  / 2.0f + bitwidth * 3.0f;

		led_gScene->clear();

		pen.setColor(Qt::black);
		led_gScene->addRect(0, 0, 
				    -(float)dummyStatusArea2->width(),
				    (float)dummyStatusArea2->height(),
				    pen, bbrush);
		for(i = 0; i < using_flags->get_use_led_device(); i++) {
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
}

void Ui_MainWindowBase::do_recv_data_led(quint32 d)
{
	osd_led_data = (uint32_t)d;
}

void Ui_MainWindowBase::redraw_leds(void)
{
		uint32_t drawflags;
		int i;
		float bitwidth = (float)dummyStatusArea2->width() / ((float)using_flags->get_use_led_device() * 2.0);
		float start = -(float)dummyStatusArea2->width() + bitwidth * 4.0f;
		drawflags = osd_led_data;
		
		for(i = 0; i < using_flags->get_use_led_device(); i++) {
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

void Ui_MainWindowBase::redraw_status_bar(void)
{
	int i;
}


void Ui_MainWindowBase::message_status_bar(QString str)
{
	//QString tmpstr;
	if(messagesStatusBar == NULL) return;
	if(str != messagesStatusBar->text()) messagesStatusBar->setText(str);
}
