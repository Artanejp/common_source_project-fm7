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

#include "mainwidget_base.h"
//#include "emu.h"
#include "qt_main.h"
//#include "vm.h"
#include "menu_flags.h"

//extern EMU* emu;
//extern USING_FLAGS *using_flags;

int Ui_MainWindowBase::Calc_OSD_Wfactor()
{
	float wfactor;
	if(using_flags->is_use_fd() && using_flags->is_use_qd() && using_flags->is_use_tape()) {
		wfactor = (1280.0 - 400.0 - 100.0 - 100.0) / ((float)using_flags->get_max_qd() + (float)using_flags->get_max_drive());
	} else 	if(using_flags->is_use_fd() && using_flags->is_use_bubble() && using_flags->is_use_tape()) {
		wfactor = (1280.0 - 400.0 - 100.0 - 100.0 - 100.0 * (float)using_flags->get_max_bubble()) /
			(float)using_flags->get_max_drive();
	} else if(using_flags->is_use_fd() && using_flags->is_use_tape()) {
		wfactor = (1280.0 - 400.0 - 100.0 - 100.0) / (float)using_flags->get_max_drive();
	} else if(using_flags->is_use_qd() && using_flags->is_use_tape()) {
		wfactor = (1280.0 - 400.0 - 100.0 - 100.0) / (float)using_flags->get_max_qd();
	} else if(using_flags->is_use_fd()) {
		wfactor = (1280.0 - 400.0 - 100.0) / (float)using_flags->get_max_drive();
	} else if(using_flags->is_use_fd()) {	
		wfactor = (1280.0 - 400.0 - 100.0) / (float)using_flags->get_max_qd();
	} else if(using_flags->is_use_fd() && using_flags->is_use_qd()) {
		wfactor = (1280.0 - 400.0 - 100.0) / ((float)using_flags->get_max_qd() + (float)using_flags->get_max_drive());
	} else {
		wfactor = 0.0;
	}
	if(wfactor < 0.0) wfactor = 0.0;
	return (int)wfactor;
}

void Ui_MainWindowBase::initStatusBar(void)
{
	int i;
	int wfactor;
	statusUpdateTimer = new QTimer;
	messagesStatusBar = new QLabel;
	//dummyStatusArea1 = new QWidget;
	QSize size1, size2, size3;
	QString tmpstr;
	QString n_s;
	QString tmps_n;
	//   QHBoxLayout *layout = new QHBoxLayout();
	
	//statusbar->addWidget(layout, 0);
	messagesStatusBar->setFixedWidth(400);
	statusbar->addPermanentWidget(messagesStatusBar, 0);
	messagesStatusBar->setStyleSheet("font: 12pt \"Sans\";");
	dummyStatusArea1 = new QWidget;
	statusbar->addPermanentWidget(dummyStatusArea1, 1);
	
	wfactor = Calc_OSD_Wfactor();
	if(using_flags->is_use_fd()) {
		for(i = 0; i < using_flags->get_max_drive(); i++) osd_str_fd[i].clear();
	}
	if(using_flags->is_use_qd()) {
		for(i = 0; i < using_flags->get_max_qd(); i++) osd_str_qd[i].clear();
	}
	if(using_flags->is_use_tape()) {
		osd_str_cmt.clear();
	}
	if(using_flags->is_use_compact_disc()) {
		osd_str_cdrom.clear();
	}
	if(using_flags->is_use_laser_disc()) {
		osd_str_laserdisc.clear();
	}
	if(using_flags->is_use_bubble()) {
		for(i = 0; i < using_flags->get_max_bubble(); i++) osd_str_bubble[i].clear();
	}
	osd_led_data = 0x00000000;

	tmps_n = QString::fromUtf8("font: ");
	n_s.setNum(12);
	tmps_n = tmps_n + n_s + QString::fromUtf8("pt \"Sans\";");
	if(using_flags->is_use_fd()) {
		for(i = 0; i < using_flags->get_max_drive(); i++) { // Will Fix
			fd_StatusBar[i] = new QLabel;
			fd_StatusBar[i]->setStyleSheet(tmps_n);
			fd_StatusBar[i]->setFixedWidth((wfactor > 200) ? 200 : wfactor);
			//      fd_StatusBar[i]->setAlignment(Qt::AlignRight);
			statusbar->addPermanentWidget(fd_StatusBar[i]);
		}
	}
	if(using_flags->is_use_qd()) {
		for(i = 0; i < using_flags->get_max_qd(); i++) {
			qd_StatusBar[i] = new QLabel;
			qd_StatusBar[i]->setStyleSheet(tmps_n);
			qd_StatusBar[i]->setFixedWidth((wfactor > 150) ? 150 : wfactor);
			//     qd_StatusBar[i]->setAlignment(Qt::AlignRight);
			statusbar->addPermanentWidget(qd_StatusBar[i]);
		}
	}
	if(using_flags->is_use_bubble()) {
		for(i = 0; i < using_flags->get_max_bubble(); i++) {
			bubble_StatusBar[i] = new QLabel;
			bubble_StatusBar[i]->setFixedWidth(100);
			bubble_StatusBar[i]->setStyleSheet(tmps_n);
			statusbar->addPermanentWidget(bubble_StatusBar[i]);
		}
	}
	if(using_flags->is_use_tape()) {
		cmt_StatusBar = new QLabel;
		cmt_StatusBar->setFixedWidth(100);
		cmt_StatusBar->setStyleSheet(tmps_n);;
		statusbar->addPermanentWidget(cmt_StatusBar);
	}
	if(using_flags->is_use_compact_disc()) {
		cdrom_StatusBar = new QLabel;
		cdrom_StatusBar->setFixedWidth(100);
		cdrom_StatusBar->setStyleSheet(tmps_n);
		statusbar->addPermanentWidget(cdrom_StatusBar);
	}
	if(using_flags->is_use_laser_disc()) {
		laserdisc_StatusBar = new QLabel;
		laserdisc_StatusBar->setFixedWidth(100);
		laserdisc_StatusBar->setStyleSheet(tmps_n);
		statusbar->addPermanentWidget(laserdisc_StatusBar);
	}
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
	int wfactor;
	QSize nowSize;
	double height, width;
	double scaleFactor;
	int ww;
	int pt;
	int i;
	int qd_width, fd_width;
	int sfactor = 0;;
	QString n_s;
	QString tmps;

	nowSize = messagesStatusBar->size();
	height = (double)(nowSize.height());
	width  = (double)(nowSize.width());
	scaleFactor = (double)w / 1280.0;
   
	statusbar->setFixedWidth(w);
	pt = (int)(14.0 * scaleFactor);
	if(pt < 4) pt = 4;
	sfactor = (int)(400.0 * scaleFactor);
	messagesStatusBar->setFixedWidth((int)(400.0 * scaleFactor));
	
	tmps = QString::fromUtf8("font: ");
	n_s.setNum(pt);
	tmps = tmps + n_s + QString::fromUtf8("pt \"Sans\";");
	messagesStatusBar->setStyleSheet(tmps);
   
	wfactor = Calc_OSD_Wfactor();
	
	fd_width = wfactor;
	qd_width = wfactor;
	if(fd_width > 200) fd_width = 200;
	if(fd_width < 50) fd_width = 50;
	if(qd_width > 150) qd_width = 150;
	if(qd_width < 50) qd_width = 50;

	if(using_flags->is_use_fd()) {
		ww = (int)(scaleFactor * (double)fd_width);
		for(i = 0; i < using_flags->get_max_drive(); i++) { // Will Fix
			fd_StatusBar[i]->setStyleSheet(tmps);
			fd_StatusBar[i]->setFixedWidth(ww);
			sfactor += ww;
		}
	}
	if(using_flags->is_use_qd()) {
		ww = (int)(scaleFactor * (double)fd_width);
		for(i = 0; i < using_flags->get_max_qd(); i++) { // Will Fix
			qd_StatusBar[i]->setStyleSheet(tmps);
			qd_StatusBar[i]->setFixedWidth(ww);
			sfactor += ww;
		}
	}
	if(using_flags->is_use_tape()) {
		cmt_StatusBar->setFixedWidth((int)(100.0 * scaleFactor));
		cmt_StatusBar->setStyleSheet(tmps);
		sfactor += (int)(100.0 * scaleFactor);
	}
	if(using_flags->is_use_bubble()) {
		ww = (int)(scaleFactor * 100.0);
		for(i = 0; i < using_flags->get_max_bubble(); i++) { // Will Fix
			bubble_StatusBar[i]->setStyleSheet(tmps);
			bubble_StatusBar[i]->setFixedWidth(ww);
			sfactor += ww;
		}
	}
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

void Ui_MainWindowBase::do_change_osd_qd(int drv, QString tmpstr)
{
	if((drv < 0) || (drv > using_flags->get_max_qd())) return;
	osd_str_qd[drv] = tmpstr;
}

void Ui_MainWindowBase::do_change_osd_fd(int drv, QString tmpstr)
{
	if((drv < 0) || (drv >= using_flags->get_max_drive())) return;
	osd_str_fd[drv] = tmpstr;
}
void Ui_MainWindowBase::do_change_osd_cdrom(QString tmpstr)
{
	osd_str_cdrom = tmpstr;
}
void Ui_MainWindowBase::do_change_osd_cmt(QString tmpstr)
{
	osd_str_cmt = tmpstr;
}
void Ui_MainWindowBase::do_change_osd_bubble(int drv, QString tmpstr)
{
	if((drv < 0) || (drv > using_flags->get_max_bubble())) return;
	osd_str_bubble[drv] = tmpstr;
}


void Ui_MainWindowBase::redraw_status_bar(void)
{
	int access_drv;
	int tape_counter;
	int i;

	if(using_flags->is_use_fd()) {
		for(i = 0; i < using_flags->get_max_drive(); i++) {	   
			if(osd_str_fd[i] != fd_StatusBar[i]->text()) fd_StatusBar[i]->setText(osd_str_fd[i]);
		}
	}
	if(using_flags->is_use_qd()) {
		for(i = 0; i < using_flags->get_max_qd(); i++) {	   
			if(osd_str_qd[i] != qd_StatusBar[i]->text()) qd_StatusBar[i]->setText(osd_str_qd[i]);
		}
	}
	if(using_flags->is_use_tape()) {
		if(osd_str_cmt != cmt_StatusBar->text()) cmt_StatusBar->setText(osd_str_cmt);
	}
	if(using_flags->is_use_compact_disc()) {
		if(osd_str_cdrom != cdrom_StatusBar->text()) cdrom_StatusBar->setText(osd_str_cdrom);
	}
	if(using_flags->is_use_laser_disc()) {
		if(osd_str_laserdisc != laserdisc_StatusBar->text()) laserdisc_StatusBar->setText(osd_str_laserdisc);
	}
	if(using_flags->is_use_bubble()) {
		for(i = 0; i < using_flags->get_max_bubble(); i++) {
		if(osd_str_bubble[i] != bubble_StatusBar[i]->text()) bubble_StatusBar[i]->setText(osd_str_bubble[i]);
		}
	}
}


void Ui_MainWindowBase::message_status_bar(QString str)
{
	//QString tmpstr;
	if(messagesStatusBar == NULL) return;
	if(str != messagesStatusBar->text()) messagesStatusBar->setText(str);
}
