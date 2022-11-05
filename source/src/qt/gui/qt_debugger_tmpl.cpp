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
#include "menu_flags.h"

#include "../../vm/vm_limits.h"

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

void CSP_Debugger_Tmpl::call_debugger(void)
{
	emit sig_set_input_string(text_command->text());
	cmd_clear();
}

void CSP_Debugger_Tmpl::run(void)
{
	if(emu == nullptr) {
		close();
		return;
	}

	text_color = QString::fromUtf8("<FONT COLOR=black>");
	string_is_strong = false;
	if(emu != nullptr) {
		emu->open_debugger(debugger_thread_param.cpu_index);
	} else {
		QString mes = QApplication::translate("Debugger", "Emulator still not start\nPlease wait.", 0);
		put_string(mes);
	}
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

void CSP_Debugger_Tmpl::closeEvent(QCloseEvent *event)
{
	//emit sig_close_debugger();
	//event->ignore();

	debugger_thread_param.request_terminate = true;
	if(emu != nullptr) {
		VM_TEMPLATE* p_vm = debugger_thread_param.vm;
		if(p_vm != nullptr) {
			uint32_t cpu_index = debugger_thread_param.cpu_index;
			if(emu->is_debugger_enabled(cpu_index)) {
				emu->close_debugger();
			}
		}
	}
	if(event != nullptr) {
		event->accept();
	}
	emit sig_finished();
}

void CSP_Debugger_Tmpl::do_destroy_thread(void)
{
	this->close();
}

CSP_Debugger_Tmpl::CSP_Debugger_Tmpl(EMU_TEMPLATE* p_emu, QWidget *parent)
	: QWidget(parent, Qt::Window),
	  p_cfg(nullptr),																		 
	  widget(nullptr),
	  text(nullptr),
	  text_command(nullptr),
	  call_font_dialog(nullptr),
	  VBoxWindow(nullptr),
	  TailButtons(nullptr),
	  emu(p_emu)
{
	OSD_BASE* p_osd;
	// First.
	parent_object = parent;

	memset(&debugger_thread_param, 0x00, sizeof(debugger_thread_param));

	
	if(p_emu == nullptr) {
		return; // OK?
	}

	p_osd = emu->get_osd();
	if(p_osd == nullptr) {
		return; // OK?
	}
	widget = this;
	p_cfg = p_osd->get_config_flags()->get_config_ptr();
	
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

	connect(this, SIGNAL(sig_apply_complete_list(QStringList)), text_command, SLOT(setCompleteList(QStringList)), Qt::QueuedConnection);
	connect(call_font_dialog, SIGNAL(pressed()), this, SLOT(rise_font_dialog()));
	connect(p_osd, SIGNAL(sig_apply_dbg_completion_list()), this, SLOT(apply_complete_list()), Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_clear_dbg_completion_list()), this, SLOT(clear_complete_list()), Qt::QueuedConnection);
	
	connect(p_osd, SIGNAL(sig_add_dbg_completion_list(_TCHAR *)), this, SLOT(add_complete_list(_TCHAR *)), Qt::QueuedConnection);

	connect(this, SIGNAL(sig_set_input_string(QString)), p_osd, SLOT(do_set_input_string(QString)), Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_close_console()), this, SLOT(do_destroy_thread()), Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_put_string_debugger(QString)), this, SLOT(put_string(QString)), Qt::QueuedConnection);
	connect(p_osd, SIGNAL(sig_set_attribute_debugger(QString, bool)), this, SLOT(set_string_attr(QString, bool)), Qt::QueuedConnection);

	connect(this, SIGNAL(sig_finished()), this, SLOT(close()), Qt::QueuedConnection);
	connect(text_command, SIGNAL(editingFinished2()), this, SLOT(call_debugger()), Qt::QueuedConnection);
	connect(parent_object, SIGNAL(quit_debugger_thread()), this, SLOT(close()), Qt::QueuedConnection);
	
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
	if(emu != nullptr) {
		uint32_t cpu_index = debugger_thread_param.cpu_index;
		if(emu->is_debugger_enabled(cpu_index)) {
			emu->close_debugger();
		}
	}
	//emu.reset(); // Deallocate
}
