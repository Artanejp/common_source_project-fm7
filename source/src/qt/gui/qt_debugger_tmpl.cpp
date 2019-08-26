/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.04.09 -
	History: 09 Apr, 2015 : Initial from Takeda.Toshiya's w32_debugger.cpp.
	[ debugger console ]
*/

#include <stdio.h>
#include <string.h>

//#include <unistd.h>
#include <fcntl.h>
#include <QObject>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFontDialog>
#include <QFont>
#include <QApplication>
#include <QResizeEvent>
#include <QSize>

#include "qt_debugger_tmpl.h"
#include "qt_lineeditplus.h"
#include "osd_base.h"

void CSP_Debugger_Tmpl::set_string_attr(QString color, bool is_strong)
{
	string_is_strong = is_strong;
	if(!(color.isEmpty())) {
		text_color = color;
	} else {
		text_color.clear();
	}
}
void CSP_Debugger_Tmpl::put_string(QString s)
{
	QString prefix = text_color;
	QString suffix;
	suffix.clear();
	if(string_is_strong) {
		prefix = prefix + QString::fromUtf8("<B>");
	}
	if(string_is_strong) {
		suffix = QString::fromUtf8("</B>");
	}
	if(!(text_color.isEmpty())) {
		suffix = suffix + QString::fromUtf8("</FONT>");
	}
	//if(!(prefix.isEmpty())) text->insertHtml(prefix);
	text->insertPlainText(s);
	//text->insertHtml(s);
	//if(!(suffix.isEmpty())) text->insertHtml(suffix);
	text->moveCursor(QTextCursor::End);
	
}

void CSP_Debugger_Tmpl::cmd_clear()
{
	text_command->clear();
	text->moveCursor(QTextCursor::End);
}

void CSP_Debugger_Tmpl::stop_polling()
{
	//poll_stop = true;
}

QStringList &CSP_Debugger_Tmpl::get_complete_list()
{
	return complete_list;
}

void CSP_Debugger_Tmpl::add_complete_list(_TCHAR *index)
{
	if(index != NULL) {
		complete_list.append(QString::fromUtf8(index));
	}
}

void CSP_Debugger_Tmpl::add_complete_list(QString index)
{
	complete_list.append(index);
}

void CSP_Debugger_Tmpl::set_complete_list(QStringList list)
{
	complete_list.clear();
	complete_list = list;
}

void CSP_Debugger_Tmpl::set_complete_list(std::list<std::string> *list)
{
	if(list != NULL) {
		if(!(list->empty())) {
			complete_list.clear();
			for(auto n = list->begin(); n != list->end(); ++n) {
				QString tmps;
				tmps.fromStdString(*n);
				complete_list.append(tmps);
			}
		}
	}
}

void CSP_Debugger_Tmpl::clear_complete_list(void)
{
	complete_list.clear();
}

void CSP_Debugger_Tmpl::apply_complete_list(void)
{
	emit sig_apply_complete_list(complete_list);
}

void CSP_Debugger_Tmpl::run(void)
{
	connect(this, SIGNAL(sig_finished()), this, SLOT(close()));
	connect(parent_object, SIGNAL(quit_debugger_thread()), this, SLOT(close()));
}

void CSP_Debugger_Tmpl::set_font(const QFont &font)
{
	text_command->setFont(font);
	text->setFont(font);
	if(!(font.toString().isEmpty())) {
		memset(p_cfg->debugwindow_font, 0x00, sizeof(p_cfg->debugwindow_font));
		snprintf(p_cfg->debugwindow_font, sizeof(p_cfg->debugwindow_font) - 1, "%s", font.toString().toLocal8Bit().constData());
	}
}

void CSP_Debugger_Tmpl::rise_font_dialog(void)
{
	QFontDialog *dlg = new QFontDialog(text->font(), this);
	connect(dlg, SIGNAL(fontSelected(const QFont)), this, SLOT(set_font(const QFont)));
	dlg->show();
}


void CSP_Debugger_Tmpl::resizeEvent(QResizeEvent *event)
{
	QSize s = event->size();
	int width = s.width();
	int height = s.height();
	if(width < 320) width = 320;
	if(height < 200) height = 200;
	p_cfg->debugwindow_height = height;
	p_cfg->debugwindow_width = width;
}

CSP_Debugger_Tmpl::CSP_Debugger_Tmpl(OSD_BASE* p_osd, QWidget *parent) : QWidget(parent, Qt::Window)
{
	widget = this;
	p_cfg = p_osd->get_config_flags()->get_config_ptr();
	parent_object = parent;
	text = new QTextEdit(this);
	text->setReadOnly(true);
	text->setLineWrapMode(QTextEdit::WidgetWidth);
	text->setAcceptRichText(true);

	text_command = new QLineEditPlus(QString::fromUtf8(""), this);
	text_command->setEchoMode(QLineEdit::Normal);
	text_command->setMaxLength(1024);
	text_command->setReadOnly(false);
	text_command->setEnabled(true);
	text_command->clear();
	call_font_dialog = new QPushButton(QApplication::translate("Debugger", "Set Font", 0),this);

	
	if(strlen(p_cfg->debugwindow_font) > 0) {
		QFont font;
		font.fromString(QString::fromLocal8Bit(p_cfg->debugwindow_font));
		text->setFont(font);
		text_command->setFont(font);
	}
	complete_list.clear();

	connect(this, SIGNAL(sig_apply_complete_list(QStringList)), text_command, SLOT(setCompleteList(QStringList)));
	connect(call_font_dialog, SIGNAL(pressed()), this, SLOT(rise_font_dialog()));
	connect(p_osd, SIGNAL(sig_apply_dbg_completion_list()), this, SLOT(apply_complete_list()));
	connect(p_osd, SIGNAL(sig_clear_dbg_completion_list()), this, SLOT(clear_complete_list()));
	connect(p_osd, SIGNAL(sig_add_dbg_completion_list(_TCHAR *)), this, SLOT(add_complete_list(_TCHAR *)));

	VBoxWindow = new QVBoxLayout;
	TailButtons = new QHBoxLayout;
	
	TailButtons->setAlignment(Qt::AlignRight);
	TailButtons->addStretch();
	TailButtons->addWidget(call_font_dialog);
	VBoxWindow->addLayout(TailButtons);
	VBoxWindow->addWidget(text);
	VBoxWindow->addWidget(text_command);
	
	int w = p_cfg->debugwindow_width;
	int h = p_cfg->debugwindow_height;
	if(w < 320) w = 320;
	if(h < 200) h = 200;
	
	this->setLayout(VBoxWindow);
	this->resize(w, h);
}


CSP_Debugger_Tmpl::~CSP_Debugger_Tmpl()
{
	
}
