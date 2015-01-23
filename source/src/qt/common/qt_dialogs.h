
#ifndef _CSP_QT_DIALOGS_H
#define _CSP_QT_DIALOGS_H
#include <QtGui/QFileDialog>
#include "emu.h"
#include "qt_main.h"

typedef class CSP_DiskParams : public QObject {   
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
   void setRecMode(bool num) {play = num; }
   int getRecMode(void) {
      if(play) return 1;
      return 0;;
   }
   
 signals:
   int do_open_disk(int, QString);
   int do_close_disk(int);
   int sig_open_cart(int, QString);
   int do_close_cart(int);
   int do_open_cmt(bool, QString);
   int do_close_cmt();
#if defined(USE_QD1) || defined(USE_QD2)
   int do_open_quick_disk(int, QString);
#endif   
 public slots:
     void _open_disk(const QString fname);
     void _open_cart(const QString fname);
     void _open_cmt(const QString fname);

#if defined(USE_QD1) || defined(USE_QD2)
     void _open_quick_disk(QString);
#endif
   
 private:
   int drive;
   bool play;
} CSP_FileParams;

typedef class CSP_DiskDialog : public QFileDialog {
 Q_OBJECT
 public:
   CSP_FileParams *param;
   CSP_DiskDialog(QObject *parent = 0) : QFileDialog(parent) {
	param = new CSP_FileParams(parent);
   }
   ~CSP_DiskDialog() {
	delete param;
   }
   
   
} CSP_DiskDialog;

extern "C" {
#ifdef USE_CART1
#endif
#ifdef USE_FD1
#endif
}

#endif //End.
