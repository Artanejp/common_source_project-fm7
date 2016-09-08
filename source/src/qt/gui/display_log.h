/*
 * Common Source code project
 */

#ifndef _CSP_QT_DISPLAY_LOG_H
#define _CSP_QT_DISPLAY_LOG_H

#include <QString>
#include <QByteArray>
#include <QUrl>
#include <QStringList>
#include <QWidget>

#include "common.h"

QT_BEGIN_NAMESPACE

class QTextBrowser;
class QLabel;
class QFont;
class QVBoxLayout;
class QGridLayout;
class QGroupBox;
class QTimer;
class USING_FLAGS;

class DLL_PREFIX Dlg_LogViewerBind : public QObject
{
	Q_OBJECT
protected:
	uint32_t bind_int;
	QString str;
public:
	Dlg_LogViewerBind(QObject *parent, QString _str, int32_t _bind_int = 0xffffffff);
	~Dlg_LogViewerBind();
	int32_t get_int(void);
	QString get_domain(void);
public slots:
	void do_set_int(int n);
	void do_reset_int(int n);
	void do_set_bind_val(uint32_t n);
	void do_set_domain(QString);
	void do_update(void);
signals:
	int sig_throw_int(int);
	int sig_throw_desc(QString, uint32_t);
};

class DLL_PREFIX Dlg_LogViewer : public QWidget
{
	Q_OBJECT
	
protected:
	QString log_str;
	
	QString domain_name;
	uint32_t level_map;
	int64_t now_end_line;

	QTextBrowser *TextBox;
	QGridLayout *MasterLayout;
	QTimer *UpdateTimer;
	USING_FLAGS *using_flags;
public:
	Dlg_LogViewer(USING_FLAGS *p, QWidget *parent, QString _domain = QString::fromUtf8(""), uint32_t _level = 0xffffffff);
	~Dlg_LogViewer();

public slots:
	void do_search_by_domain(QString _domain_name, uint32_t _level);
	void do_update(void);
	void do_refresh(void);
signals:
	int sig_text_clear();
	int sig_text_update(QString);
	int sig_text_append(QString);
};

QT_END_NAMESPACE
#endif
