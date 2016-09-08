/*
 */


#include <QFile>
#include <QTextBrowser>
#include <QLabel>
#include <QIODevice>
#include <QUrl>
#include <QStringList>

#include <QTextBrowser>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFont>
#include <QGridLayout>
#include <QTimer>

#include "mainwidget_base.h"
#include "display_log.h"
#include "menu_flags.h"
#include "csp_logger.h"

Dlg_LogViewerBind::Dlg_LogViewerBind(QObject *parent, QString _str, int _bind_int) : QObject(parent)
{
	bind_int =_bind_int;
	str = _str;
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

Dlg_LogViewer::Dlg_LogViewer(USING_FLAGS *p, QWidget *parent, QString _domain, uint32_t level) : QWidget(parent)
{
	log_str.clear();
	domain_name = _domain;
	level_map = level;
	now_end_line = 0;
	using_flags = p;
	
	TextBox = new QTextBrowser();
	TextBox->setStyleSheet("font: 12pt \"Sans\";");
	TextBox->setMinimumSize(800, 470);
	TextBox->setOpenExternalLinks(true);
	if(csp_logger != NULL) {
		CSP_LoggerLine *p;
		QString tmpstr;
		QString beforestr;
		bool first = true;
		do {
			p = (CSP_LoggerLine *)(csp_logger->get_raw_data(false, now_end_line, NULL));
			if(p == NULL) break;
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
	MasterLayout->addWidget(TextBox, 0, 0, 4, 4);

	UpdateTimer = new QTimer(this);
	UpdateTimer->setInterval(500); // 500ms
	UpdateTimer->setSingleShot(false);
	connect(UpdateTimer, SIGNAL(timeout()), this, SLOT(do_update()));
	UpdateTimer->start();
	this->setLayout(MasterLayout);
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
		p = (CSP_LoggerLine *)(csp_logger->get_raw_data(false, line, NULL));
		if(p == NULL) break;
		bool f = false;
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
		p = (CSP_LoggerLine *)(csp_logger->get_raw_data(false, now_end_line, NULL));
		if(p == NULL) break;
		bool f = false;
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
