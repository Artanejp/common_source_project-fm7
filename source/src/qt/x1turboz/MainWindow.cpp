/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QtCore/QVariant>
#include <QtGui>
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE

META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
//   ui->setupUi(this);
//   setupUi();
//   createContextMenu();
}

META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



