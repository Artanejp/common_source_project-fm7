/*
 * Common Source code project
 */

#ifndef _CSP_QT_DISPLAY_TEXT_DOCUMENT_H
#define _CSP_QT_DISPLAY_TEXT_DOCUMENT_H

#include <QFile>
#include <QString>
#include <QByteArray>
#include <QTextBrowser>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFont>

//#include "emu.h"

QT_BEGIN_NAMESPACE

class Dlg_BrowseText : public QWidget
{
	Q_OBJECT
protected:
	QWidget *parent_widget;
	//QFont *TextFont;
	QTextBrowser *TextBox;
	QVBoxLayout *VBox;
public:
	Dlg_BrowseText(QString fname, bool internal = true, QWidget *parent = 0);
	~Dlg_BrowseText();
	
};
QT_END_NAMESPACE

#endif //_CSP_QT_DISPLAY_TEXT_DOCUMENT_H
