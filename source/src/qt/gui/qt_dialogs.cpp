/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.01.07

	[Qt dialogs]
*/

#include <stdio.h>
#include <string>
#include <vector>
#include "common.h"
//#include "emu.h"
#include "qt_main.h"
#include "qt_dialogs.h"

//#include "csp_logger.h"
#include "commonclasses.h"
//#include "menuclasses.h"

void CSP_DiskParams::_open_disk(QString s)
{
	int d = getDrive();
	//int n = d + CSP_LOG_TYPE_VFILE_FLOPPY;
	//csp_logger->debug_log(CSP_LOG_INFO, n, "Try to open media image: %s", s.toLocal8Bit().constData());
	emit do_open_disk(d, s);
}

void CSP_DiskParams::_open_cart(QString s)
{
	int d = getDrive();
	emit sig_open_cart(d, s);
}
void CSP_DiskParams::_open_cmt(QString s)
{
	emit do_open_cmt(play, s);
}

void CSP_DiskParams::_open_binary(QString s) {
	emit sig_open_binary_file(drive, s, play);
}


CSP_CreateDiskDialog::CSP_CreateDiskDialog(bool *masks, QWidget *parent = 0) : QWidget(parent)
{
	__real_media_type = 0x00;
	dlg = new QFileDialog(NULL, Qt::Widget);
	dlg->setParent(this);
	dlg->setOption(QFileDialog::ReadOnly, false);
	dlg->setOption(QFileDialog::DontConfirmOverwrite, false);
	dlg->setOption(QFileDialog::DontUseNativeDialog, true);
	dlg->setAcceptMode(QFileDialog::AcceptSave);
	dlg->setFileMode(QFileDialog::AnyFile);
		
	param = new CSP_FileParams();
	if(masks[0]) media_type.addItem(QString::fromUtf8("2D"),  (const quint8)0x00);
	if(masks[1]) media_type.addItem(QString::fromUtf8("2DD"), (const quint8)0x10);
	if(masks[2]) media_type.addItem(QString::fromUtf8("2HD"), (const quint8)0x20);
	if(masks[3]) media_type.addItem(QString::fromUtf8("2HD/1.44M"), (const quint8)0x30);
					
	type_label.setText(QApplication::translate("MenuMedia", "Virtual FD type:", 0));
	type_label.setToolTip(QApplication::translate("MenuMedia", "Select type of virtual floppy.", 0));
	
	layout.addWidget(&type_label, 1, 0);
	layout.addWidget(&media_type, 1, 1);
	layout.addWidget(dlg, 2, 0, 2, 4);

	this->setLayout(&layout);
	connect(&media_type, SIGNAL(activated(int)), this, SLOT(do_set_type(int)));
	connect(dlg, SIGNAL(fileSelected(QString)), this, SLOT(do_create_disk(QString)));

}

