/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Help->About Dialog
 *  History: Oct 28, 2015 : Initial
 */
#ifndef _CSP_QT_DISPLAY_ABOUT_H
#define _CSP_QT_DISPLAY_ABOUT_H

#include <QTextBrowser>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

#include "common.h"

QT_BEGIN_NAMESPACE
class USING_FLAGS;
class EMU;
class Ui_MainWindowBase;
class DLL_PREFIX Dlg_AboutCSP : public QWidget
{
	Q_OBJECT
protected:
	USING_FLAGS *using_flags;
	QWidget *parent_widget;

	QTextBrowser *TextBox;
	QLabel *iconarea;
	QLabel *titlearea;
	QHBoxLayout *HBox1;
	QLabel *revarea;
	QWidget *BoxTitle;
	QVBoxLayout *VBox;
public:
	Dlg_AboutCSP(USING_FLAGS *p, QWidget *parent = 0);
	~Dlg_AboutCSP();
	
};
QT_END_NAMESPACE

#endif //_CSP_QT_DISPLAY_ABOUT_H
