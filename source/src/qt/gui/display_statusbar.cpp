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
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

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
   statusbar->addPermanentWidget(dummyStatusArea2, 0);
//   statusbar->addWidget(dummyStatusArea2);
   
   connect(statusUpdateTimer, SIGNAL(timeout()), this, SLOT(redraw_status_bar()));
   statusUpdateTimer->start(50);
}

	
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
	   
	      
	   

	      
