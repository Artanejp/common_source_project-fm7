
#ifndef _CSP_QT_EMUEVENTS_H
#define _CSP_QT_EMUEVENTS_H

#include <string>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include "menu_common.h"

#include "simd_types.h"
#include "common.h"
#include "emu.h"
#include "filio.h"
#include "config.h"

extern EMU *emu;

#ifdef USE_DIPSWITCH
void OnToggleDipSw(int dipsw);
void OnChangeDipSw(int dipsw, int flag);
#endif

#ifdef USE_DEVICE_TYPE
void OnSetDeviceType(int devtype);
#endif

#if defined(USE_CART1) || defined(USE_CART2)
void OnOpenCart(QWidget *parent, int drive);
void OnCloseCart(int drive);
void OnRecentCart(int drive, int menunum);
#endif

#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
void OpenRecentFloppy(QWidget *parent, int drv, int num);
void OnCloseFD(int drive);
void OnOpenFD(QWidget *parent,int drive);
void OnSelectD88Bank(int drive, int no);
void Floppy_SelectD88(int drive, int num);
#endif

#if defined(USE_QD1) || defined(USE_QD2)
void OnOpenQD(QWidget *parent, int drive);
void OnCloseQD(int drive);
void OnRecentQD(QWidget *parent, int drive, int menunum);
#endif

#ifdef USE_TAPE
void OnPlayTAPE(QWidget *parent);
void OnRecTAPE(QWidget *parent);
void OnCloseTAPE(void);
void OnUseWaveShaperTAPE(QWidget *wid);
void OnDirectLoadMZT(QWidget *wid);
void OnRecentTAPE(QWidget *parent, int menunum);
#endif

#ifdef USE_TAPE_BUTTON
void OnPushPlayButton(QWidget *parent);
void OnPushStopButton(QWidget *parent);
#endif

// Implement LASER Disc, binary
void OnStartRecordScreen(int num);
void OnStopRecordScreen(void);
void OnScreenCapture(QWidget *parent);
void OnSetScreenMode(QMainWindow *MainWindow, QWidget *drawspace, int mode);
void OnFullScreen(QMainWindow *MainWindow, QWidget *drawspace, int mode);

void OnSetStretchMode(int mode);

#endif // End
