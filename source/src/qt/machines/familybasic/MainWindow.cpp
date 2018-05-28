/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for JR100 .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QMenu>
#include "emu.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "qt_main.h"


void META_MainWindow::setupUI_Emu(void)
{
	ConfigCPUBootMode(6);
}

void META_MainWindow::retranslateUi(void)
{
	int i;
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("System Reset",  false);
	
	menuBootMode->setTitle(QApplication::translate("Machine", "BOOT Mode", 0));
	menuBootMode->setToolTipsVisible(true);
	actionBootMode[0]->setText(QString::fromUtf8("BASIC V1"));
	actionBootMode[1]->setText(QString::fromUtf8("BASIC V2"));	
	actionBootMode[2]->setText(QString::fromUtf8("Playbox BASIC"));
	actionBootMode[3]->setText(QString::fromUtf8("VRC7 BASIC V2"));
	actionBootMode[4]->setText(QString::fromUtf8("VRC7 BASIC V3"));
	actionBootMode[5]->setText(QString::fromUtf8("MMC5 BASIC V3"));
	// Set Labels
} // retranslateUi



META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



