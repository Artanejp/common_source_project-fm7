/*
 */


#include <QFile>
#include <QTextBrowser>
#include <QLabel>
#include <QIODevice>
#include <QUrl>
#include <QStringList>
#include <QApplication>

#include <QTextBrowser>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFont>
#include <QGridLayout>
#include <QFontDialog>
#include <QPushButton>
#include <QTimer>
#if QT_VERSION >= 0x051400
	#include <QRecursiveMutex>
#else
	#include <QMutex>
#endif
#include <QMutexLocker>
#include <QResizeEvent>
#include <QSize>

#include "mainwidget_base.h"
#include "display_log.h"
#include "menu_flags.h"
#include "csp_logger.h"

Dlg_LogViewerBind::Dlg_LogViewerBind(QObject *parent, CSP_Logger *logger, QString _str, int _bind_int) : QObject(parent)
{
	bind_int =_bind_int;
	str = _str;
	csp_logger = logger;
}

Dlg_LogViewerBind::~Dlg_LogViewerBind()
{
}

int32_t Dlg_LogViewerBind::get_int(void)
{
	return bind_int;
}

QString Dlg_LogViewerBind::get_domain(void)
{
	return str;
}

void Dlg_LogViewerBind::do_set_domain(QString _str)
{
	str = _str;
}

void Dlg_LogViewerBind::do_set_bind_val(uint32_t n)
{
	bind_int = n;
}

void Dlg_LogViewerBind::do_reset_int(int n)
{
	if(n < 0) {
		bind_int = 0x00000000;
		emit sig_throw_desc(str, bind_int);
		return;
	}
	bind_int = bind_int & (~(1 << n));
	emit sig_throw_desc(str, bind_int);
}

void Dlg_LogViewerBind::do_set_int(int n)
{
	if(n < 0) {
		bind_int = 0xffffffff;
		emit sig_throw_desc(str, bind_int);
		return;
	}
	bind_int = bind_int | (1 << n);
	emit sig_throw_desc(str, bind_int);
}

void Dlg_LogViewerBind::do_update(void)
{
	emit sig_throw_desc(str, bind_int);
}

void Dlg_LogViewer::set_font(const QFont &font)
{
	TextBox->setFont(font);
	config_t *p_cfg = using_flags->get_config_ptr();
	if(!(font.toString().isEmpty())) {
		memset(p_cfg->logwindow_font, 0x00, sizeof(p_cfg->logwindow_font));
		snprintf(p_cfg->logwindow_font, sizeof(p_cfg->logwindow_font) - 1, "%s", font.toString().toLocal8Bit().constData());
	}
}

void Dlg_LogViewer::rise_font_dialog(void)
{
	QFontDialog *dlg = new QFontDialog(TextBox->font(), this);
	connect(dlg, SIGNAL(fontSelected(const QFont)), this, SLOT(set_font(const QFont)));
	dlg->show();
}

void Dlg_LogViewer::resizeEvent(QResizeEvent *event)
{
	QSize s = event->size();
	int width = s.width();
	int height = s.height();
	if(width < 320) width = 320;
	if(height < 200) height = 200;
	config_t *p_cfg = using_flags->get_config_ptr();
	
	p_cfg->logwindow_height = height;
	p_cfg->logwindow_width = width;
}


Dlg_LogViewer::Dlg_LogViewer(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent, QString _domain, uint32_t level) : QWidget(parent)
{
#if QT_VERSION >= 0x051400
	lock_mutex = new QRecursiveMutex();
#else
	lock_mutex = new QMutex(QMutex::Recursive);
#endif
	log_str.clear();
	domain_name = _domain;
	level_map = level;
	now_end_line = 0;
	using_flags = p;
	csp_logger = logger;
	TextBox = new QTextBrowser();
//	TextBox->setStyleSheet("font: 12pt \"Sans\";");
	TextBox->setMinimumSize(800, 470);
	TextBox->setOpenExternalLinks(true);
	if(csp_logger != NULL) {
		CSP_LoggerLine *p;
		QString tmpstr;
		QString beforestr;
		bool first = true;
		do {
			{
				QMutexLocker locker(lock_mutex);
				p = (CSP_LoggerLine *)(csp_logger->get_raw_data(false, now_end_line, NULL));
				if(p == NULL) break;
			}
			if(p->check_level(_domain, -1)) {
				tmpstr = p->get_element_console();
				if(!beforestr.endsWith("\n") && !first) log_str.append("\n");
				log_str.append(tmpstr);
				beforestr = tmpstr;
				first = false;
			}
			now_end_line = now_end_line + 1;
		} while(1);
		//if(!tmpstr.endsWith("\n")) log_str.append("\n");
	}
	TextBox->setText(log_str.toUtf8());
	
	QString title;
	title = QString::fromUtf8("Log (emu");
	title.append(using_flags->get_config_name());
	title.append(QString::fromUtf8(")"));
	if(parent == NULL) this->setWindowTitle(title);
	
	connect(this, SIGNAL(sig_text_clear()), TextBox, SLOT(clear()));
	connect(this, SIGNAL(sig_text_update(QString)), TextBox, SLOT(setText(QString)));
	connect(this, SIGNAL(sig_text_append(QString)), TextBox, SLOT(append(QString)));
	MasterLayout = new QGridLayout;
	FontDlgButton = new QPushButton(QApplication::translate("LogWindow", "Set Font", 0),this);

	config_t *p_cfg = using_flags->get_config_ptr();

	if(strlen(p_cfg->logwindow_font) > 0) {
		QFont font;
		font.fromString(QString::fromLocal8Bit(p_cfg->logwindow_font));
		TextBox->setFont(font);
	}
	connect(FontDlgButton, SIGNAL(pressed()), this, SLOT(rise_font_dialog()));
	MasterLayout->addWidget(FontDlgButton, 0, 3, Qt::AlignRight);
	MasterLayout->addWidget(TextBox, 1, 0, 4, 4);

	UpdateTimer = new QTimer(this);
	UpdateTimer->setInterval(500); // 500ms
	UpdateTimer->setSingleShot(false);
	connect(UpdateTimer, SIGNAL(timeout()), this, SLOT(do_update()));
	UpdateTimer->start();
	this->setLayout(MasterLayout);

	int w = p_cfg->logwindow_width;
	int h = p_cfg->logwindow_height;
	if(w < 320) w = 320;
	if(h < 200) h = 200;
	this->resize(w, h);
}

Dlg_LogViewer::~Dlg_LogViewer()
{
}

void Dlg_LogViewer::do_search_by_domain(QString _domain_name, uint32_t _level)
{
	domain_name = _domain_name;
	level_map = _level;
	do_refresh();
}

void Dlg_LogViewer::do_update(void)
{
	//log_str.clear();
	CSP_LoggerLine *p;
	QString strlist;
	int64_t line = now_end_line;
	QString tmpstr;
	QString beforestr;
	bool first = true;
	do {
		{
			QMutexLocker locker(lock_mutex);
			p = (CSP_LoggerLine *)(csp_logger->get_raw_data(false, line, NULL));
			if(p == NULL) break;
		}
		for(int i = 0; i < 8; i++) {
			if((level_map & (1 << i)) != 0) {  
				if(p->check_level(domain_name, i)) {
					if(!beforestr.endsWith("\n") && !first) strlist.append("\n");
					tmpstr = p->get_element_console();
					beforestr = tmpstr;
					strlist.append(tmpstr);
					first = false;
					break;
				}
			}
		}
		line++;
	} while(1);
	if((line >= now_end_line) && !strlist.isEmpty()){
		now_end_line = line;
		log_str.append(strlist);
		if(!strlist.endsWith("\n")) log_str.append("\n");
		emit sig_text_append(strlist);
	}
}

void Dlg_LogViewer::do_refresh(void)
{
	CSP_LoggerLine *p;
	QString tmpstr;
	QString beforestr;
	bool first = true;
	log_str.clear();
	now_end_line = 0;
	do {
		{
			QMutexLocker locker(lock_mutex);
			p = (CSP_LoggerLine *)(csp_logger->get_raw_data(false, now_end_line, NULL));
			if(p == NULL) break;
		}
		for(int i = 0; i < 8; i++) {
			if((level_map & (1 << i)) != 0) {  
				if(p->check_level(domain_name, 1 << i)) {
					tmpstr = p->get_element_console();
					if(!beforestr.endsWith("\n") && !first) log_str.append("\n");
					beforestr = tmpstr;
					log_str.append(tmpstr);
					first = false;
					break;
				}
			}
		}
		now_end_line++;
	} while(1);
	//emit sig_text_clear();
	//if(!tmpstr.endsWith("\n")) log_str.append("\n");
	emit sig_text_update(log_str);
}
