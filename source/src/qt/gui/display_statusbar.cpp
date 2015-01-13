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
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

extern EMU* emu;

void Ui_MainWindow::initStatusBar(void)
{
   int i;
   statusUpdateTimer = new QTimer;
   messagesStatusBar = new QLabel;
   dummyStatusArea1 = new QWidget;
   QSize size1, size2, size3;
   
   statusbar->addWidget(messagesStatusBar, 0);
//   size1.setWidth(400);
   //staususBar->setWidth(size1);
   statusbar->addWidget(dummyStatusArea1, 1);
#ifdef USE_FD1
  // size2.setWidth(180);
   for(i = 0; i < MAX_FD; i++) { // Will Fix
      fd_StatusBar[i] = new QLabel;
      fd_StatusBar[i]->setFixedWidth(200);
      statusbar->addWidget(fd_StatusBar[i]);
   }
#endif
//   size3.setWidth(200);
   dummyStatusArea2 = new QWidget;
   statusbar->addWidget(dummyStatusArea2);
   dummyStatusArea2->setFixedWidth(200);
   connect(statusUpdateTimer, SIGNAL(timeout()), this, SLOT(redraw_status_bar()));
   statusUpdateTimer->start(50);
}

	
void Ui_MainWindow::redraw_status_bar(void)
{
   int access_drv;
   QString alamp;
   QString tmpstr;
   QString iname;
   int i;
   if(emu) {
	access_drv = emu->get_access_lamp();
        for(i = 0; i < MAX_FD; i++) {
	   if(emu->disk_inserted(i)) {
	      if(i == (access_drv - 1)) {
		 alamp = QString::fromUtf8("● ");
	      } else {
		 alamp = QString::fromUtf8("○ ");
	      }
	      tmpstr = QString::fromUtf8("Disk");
	      tmpstr = alamp + tmpstr + QString::number(i) + QString::fromUtf8(":");
	      if(emu->d88_file[i].bank_num > 0) {
		  iname = QString::fromUtf8(emu->d88_file[i].bank[emu->d88_file[i].cur_bank].name);
	      } else {
		 iname = QString::fromUtf8("*Inserted*");
	      }
	      tmpstr = tmpstr + iname;
	   } else {
	      tmpstr = QString::fromUtf8("× Disk") + QString::number(i) + QString::fromUtf8(":");
	      tmpstr = tmpstr + QString::fromUtf8(" ");
	   }
	   fd_StatusBar[i]->setText(tmpstr);
	}
//      emit sig_statusbar_updated("");

   }
   
}

	   
	      
	   

	      