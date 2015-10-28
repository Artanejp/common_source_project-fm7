/*
 * Common Source code project
 */

#ifndef _CSP_QT_DISPLAY_ABOUT_H
#define _CSP_QT_DISPLAY_ABOUT_H

#include <QFile>
#include <QString>
#include <QByteArray>
#include <QTextBrowser>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>

#include "emu.h"

QT_BEGIN_NAMESPACE

class Dlg_AboutCSP : public QWidget
{
	Q_OBJECT
protected:
	EMU *p_emu;
	QWidget *parent_widget;

	QTextBrowser *TextBox;
	QLabel *iconarea;
	QGroupBox *gBox;
	QVBoxLayout *VBox;
public:
	Dlg_AboutCSP(QWidget *parent = 0);
	~Dlg_AboutCSP();
	
};
QT_END_NAMESPACE

#endif //_CSP_QT_DISPLAY_ABOUT_H
