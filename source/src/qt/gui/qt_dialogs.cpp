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
	emit sig_open_disk(d, s);
}

void CSP_DiskParams::_open_cart(QString s)
{
	int d = getDrive();
	emit sig_open_cart(d, s);
}
void CSP_DiskParams::_open_cmt(QString s)
{
	emit sig_open_cmt(play, s);
}

void CSP_DiskParams::_open_binary(QString s) {
	emit sig_open_binary_file(drive, s, play);
}

void CSP_DiskParams::_open_quick_disk(QString s)
{
	int d = getDrive();
	//int n = d + CSP_LOG_TYPE_VFILE_FLOPPY;
	//csp_logger->debug_log(CSP_LOG_INFO, n, "Try to open media image: %s", s.toLocal8Bit().constData());
	emit sig_open_quick_disk(d, s);
}


void CSP_DiskDialog::open()
{
	setNameFilters(param->getNameFilters());
	setDirectory(param->getDirectory());
	QFileDialog::open();
	//show();
}

CSP_CreateDiskDialog::CSP_CreateDiskDialog(bool *masks, QWidget *parent) : QWidget(parent)
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

CSP_CreateHardDiskDialog::CSP_CreateHardDiskDialog(int drive, int sector_size, int sectors, int surfaces, int cylinders, QWidget *parent) : QWidget(parent)
{
	dlg = new QFileDialog(NULL, Qt::Widget);
	dlg->setParent(this);
	dlg->setOption(QFileDialog::ReadOnly, false);
	dlg->setOption(QFileDialog::DontConfirmOverwrite, false);
	dlg->setOption(QFileDialog::DontUseNativeDialog, true);
	dlg->setAcceptMode(QFileDialog::AcceptSave);
	dlg->setFileMode(QFileDialog::AnyFile);
		
	if(sector_size < 0) sector_size = 0;
	if(sector_size > 4) sector_size = 4;
	_sector_size.addItem(QString::fromUtf8("256bytes"));
	_sector_size.addItem(QString::fromUtf8("512bytes"));
	_sector_size.addItem(QString::fromUtf8("1024bytes"));
	_sector_size.addItem(QString::fromUtf8("2048bytes"));
	_sector_size.addItem(QString::fromUtf8("4096bytes"));
	_sectors.setRange(15, 33);
	_surfaces.setRange(2, 16);
	_cylinders.setRange(128, 16383);

	_label_sector_size.setText(QString::fromUtf8("Sector Size"));
	_label_sectors.setText(QString::fromUtf8("Sector in cylinder"));
	_label_surfaces.setText(QString::fromUtf8("Heads"));
	_label_cylinders.setText(QString::fromUtf8("Cylinders"));
	_label_preset_type.setText(QString::fromUtf8("Preset size"));
	media_drv = drive;
	
	if(sectors < 15) sectors = 15;
	if(sectors > 33) sectors = 33;
	if(surfaces < 2) surfaces = 2;
	if(surfaces > 16) surfaces = 16;
	if(cylinders < 128) cylinders = 128;
	if(cylinders > 16383) cylinders = 16383;

	// Preset:
	// 10MB
	// 20MB
	// 40MB
	// 80MB
	// 100MB
	// 200MB
	// 320MB
	// 500MB
	// 540MB
	_preset_type.addItem(QString::fromUtf8("10MB"));
	_preset_type.addItem(QString::fromUtf8("20MB"));
	_preset_type.addItem(QString::fromUtf8("40MB"));
	_preset_type.addItem(QString::fromUtf8("80MB"));
	_preset_type.addItem(QString::fromUtf8("100MB"));
	_preset_type.addItem(QString::fromUtf8("200MB"));
	_preset_type.addItem(QString::fromUtf8("320MB"));
	_preset_type.addItem(QString::fromUtf8("500MB"));
	_preset_type.addItem(QString::fromUtf8("540MB"));
	_size_label_label.setText(QString::fromUtf8("Total Size:"));
	
	layout.addWidget(&_label_preset_type, 1, 0);
	layout.addWidget(&_preset_type, 1, 1);
	layout.addWidget(&_label_sector_size, 2, 0);
	layout.addWidget(&_sector_size, 2, 1);
	layout.addWidget(&_label_sectors, 2, 2);
	layout.addWidget(&_sectors, 2, 3);
	
	layout.addWidget(&_label_surfaces, 3, 0);
	layout.addWidget(&_surfaces, 3, 1);
	layout.addWidget(&_label_cylinders, 3, 2);
	layout.addWidget(&_cylinders, 3, 3);
	layout.addWidget(&_size_label_label, 4, 2);
	layout.addWidget(&_size_label, 4, 3);
	layout.addWidget(dlg, 5, 0, 5, 4);

	this->setLayout(&layout);
	connect(&_preset_type, SIGNAL(activated(int)), this, SLOT(do_preset(int)));
	connect(this, SIGNAL(sig_update_total_size(uint64_t)), this, SLOT(do_update_total_size(uint64_t)));
//	connect(&_sector_size, SIGNAL(activated(int)), this, SLOT(do_change_sector_size(int)));
	connect(&_sector_size, SIGNAL(activated(int)), this, SLOT(do_update_values(int)));
	connect(&_sectors, SIGNAL(valueChanged(int)), this, SLOT(do_update_values(int)));
	connect(&_surfaces, SIGNAL(valueChanged(int)), this, SLOT(do_update_values(int)));
	connect(&_cylinders, SIGNAL(valueChanged(int)), this, SLOT(do_update_values(int)));
	connect(dlg, SIGNAL(fileSelected(QString)), this, SLOT(do_create_disk(QString)));

	do_preset(0); // Update label.
}

void CSP_CreateHardDiskDialog::do_create_disk(QString filename)
{
	int secnum = _sector_size.currentIndex();
	if(secnum < 0) secnum = 0;
	if(secnum > 4) secnum = 4;
	static const uint64_t valtbl_sec[] = { 256, 512, 1024, 2048, 4096 };
	int secsize = valtbl_sec[secnum];
	int secs = _sectors.value();
	int heads = _surfaces.value();
	int cyl = _cylinders.value();
//	printf("ToDo: Will create media.Filename = %s SEC_SIZE=%d SECS=%d SURFACES=%d CYL=%d\n",
//		   filename.toLocal8Bit().constData(), secsize, secs, heads, cyl);

	emit sig_create_disk(media_drv, secsize, secs, heads, cyl, filename);
}

void CSP_CreateHardDiskDialog::do_update_values(int dummy)
{
	int secnum = _sector_size.currentIndex();
	int heads = _surfaces.value();
	int secs = _sectors.value();
	int cyl = _cylinders.value();
	static const uint64_t secsize[] = {256, 512, 1024, 2048, 4096};
	if(secnum < 0) secnum = 0;
	if(secnum > 4) secnum = 4;
	uint64_t totalsize = (secsize[secnum] * (uint64_t)secs * (uint64_t)heads * (uint64_t)cyl);
	do_update_total_size(totalsize);	
}
void CSP_CreateHardDiskDialog::do_preset(int num)
{
	if(num >= _preset_type.count()) num = _preset_type.count() - 1;
	if(num < 0) num = 0;

	static const uint64_t valtbl[] = { 10, 20, 40, 80, 100, 200, 320, 500, 540 };
	uint64_t total_size = valtbl[num] * 1024 * 1024;
	uint64_t sectors_in_track = 15;
	uint64_t __heads = 4;
	uint64_t cyl;
	if(total_size > (100 * 1024 * 1024)) {
		sectors_in_track = 33;
		__heads = 8;
	}
	if(total_size > (300 * 1024 * 1024)) {
		__heads = 15;
	}
	cyl = (total_size / (512 * sectors_in_track * __heads)) + 1;
	_sector_size.setCurrentIndex(1); // 512bytes
	_sectors.setValue((int)sectors_in_track);
	_surfaces.setValue((int)__heads);
	_cylinders.setValue((int)cyl);
	emit sig_update_total_size(total_size);
}

void CSP_CreateHardDiskDialog::do_update_total_size(uint64_t size)
{
	uint64_t nsize = size / (1024 * 1024);
	QString label = QString("%1MB (%2bytes)").arg(nsize).arg(size);
	_size_label.setText(label);
}

