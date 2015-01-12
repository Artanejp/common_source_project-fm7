
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
	record = false;
        drive = 0;
   }
   ~CSP_DiskParams() {}
   void setDrive(int num) {if((num < 0) || (num >= 8)) num = 0; drive = num;}
   int getDrive(void) { return drive;}
   void setRecMode(bool num) {record = num; }
   int getRecMode(void) {
      if(record) return 1;
      return 0;;
   }
   
 signals:
   int do_open_disk(int, QString);
   int do_close_disk(int);
   int do_open_cart(int, QString);
   int do_close_cart(int);
   int do_open_cmt(QString);
   int do_close_cmt();
   
 public slots:
     void _open_disk(const QString fname);
     void _open_cart(const QString fname);
     void _open_cmt(const QString fname);
 private:
   int drive;
   bool record;
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
extern void open_cart_dialog(QWidget *hWnd, int drv);
#endif
#ifdef USE_FD1
#endif
#ifdef USE_TAPE
extern void open_tape_dialog(QWidget *hWnd, bool play);
#endif
}

#endif //End.