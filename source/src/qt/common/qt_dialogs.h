
#ifndef _CSP_QT_DIALOGS_H
#define _CSP_QT_DIALOGS_H
#include <QtGui/QFileDialog>
#include "emu.h"
#include "qt_main.h"

typedef class CSP_DiskParams : QObject {   
   Q_OBJECT
   Q_DISABLE_COPY(CSP_DiskParams)
 public:
   explicit CSP_DiskParams(QObject *parent = 0);
   
   void setDrive(int num) {if((num < 0) || (num >= 8)) num = 0; drive = num;}
   int getDrive(void) { return drive;}
   void setRecMode(bool num) {
      record = num;
   }
   int getRecMode(void) {
      if(record) return 1;
      return 0;;
   }
   
 signals:
 public slots:
     void open_disk(const QString fname);
     void open_cart(const QString fname);
     void open_cmt(const QString fname);
 private:
   int drive;
   bool record;
} CSP_FileParams;

typedef class CSP_DiskDialog : QFileDialog {
 public:
   CSP_FileParams param;
} CSP_DiskDialog;

#ifdef USE_CART1
extern void open_cart_dialog(QWidget *hWnd, int drv);
#endif
#ifdef USE_FD1
//extern void open_disk(int drv, _TCHAR* path, int bank);
extern void open_disk_dialog(QWidget *hWnd, int drv);
#endif
#ifdef USE_TAPE
void open_tape_dialog(QWidget *hWnd, bool play);
#endif

#endif //End.