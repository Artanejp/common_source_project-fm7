
#ifndef _CSP_QT_EMUEVENTS_H
#define _CSP_QT_EMUEVENTS_H

#include <string>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include "menuclasses.h"
#include "qt_dialogs.h"

#include "simd_types.h"
#include "common.h"
#include "emu.h"
#include "fileio.h"
#include "config.h"

extern EMU *emu;

extern void OnStartRecordScreen(int num);
extern void OnStopRecordScreen(void);

extern void OnScreenCapture(QWidget *parent);
extern void OnFullScreen(QMainWindow *MainWindow, QWidget *drawspace, int mode);
#endif // End
