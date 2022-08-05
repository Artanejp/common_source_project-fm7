/*
 * Common Source code project
 */

#ifndef _CSP_QT_DISPLAY_LOG_H
#define _CSP_QT_DISPLAY_LOG_H

#include <memory>
#include <QString>
#include <QByteArray>
#include <QUrl>
#include <QStringList>
#include <QWidget>
#include <QFont>
#include <memory>

#include "common.h"

QT_BEGIN_NAMESPACE

class QTextBrowser;
class QLabel;
class QFont;
class QVBoxLayout;
class QGridLayout;
class QGroupBox;
class QFontDialog;
class QPushButton;
class QResizeEvent;
class QTimer;

#if QT_VERSION >= 0x051400
class QRecursiveMutex;
#else
class QMutex;
#endif

class USING_FLAGS;
class CSP_Logger;
class DLL_PREFIX Dlg_LogViewerBind : public QObject
{
	Q_OBJECT
protected:
	uint32_t bind_int;
	QString str;
	std::shared_ptr<CSP_Logger> csp_logger;
public:
	Dlg_LogViewerBind(QObject *parent, std::shared_ptr<CSP_Logger> logger, QString _str, int32_t _bind_int = 0xffffffff);
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
	std::shared_ptr<CSP_Logger> csp_logger;
	QString domain_name;
	uint32_t level_map;
	int64_t now_end_line;

	QTextBrowser *TextBox;
	QGridLayout *MasterLayout;
	QTimer *UpdateTimer;
	QPushButton *FontDlgButton;

	std::shared_ptr<USING_FLAGS> using_flags;

#if QT_VERSION >= 0x051400
	QRecursiveMutex *lock_mutex;
#else
	QMutex *lock_mutex;
#endif
public:
	Dlg_LogViewer(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent, QString _domain = QString::fromUtf8(""), uint32_t _level = 0xffffffff);
	~Dlg_LogViewer();
	virtual void resizeEvent(QResizeEvent *event);

public slots:
	void do_search_by_domain(QString _domain_name, uint32_t _level);
	void do_update(void);
	void do_refresh(void);
	void set_font(const QFont &font);
	void rise_font_dialog(void);

signals:
	int sig_text_clear();
	int sig_text_update(QString);
	int sig_text_append(QString);
};

QT_END_NAMESPACE
#endif
