
#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>
#include <QGridLayout>
#include <math.h>
#include "dock_disks.h"
#include "menu_flags.h"
// Test
#include "qt_drawitem.h"
#include <QPixmap>
#include <QColor>
#include <QImageReader>

CSP_LabelVirtualDevice::CSP_LabelVirtualDevice(QWidget *parent,
											   int width, float point,
											   QString baseName, int num)
	: QWidget(parent)
{
	QString ptstr;
	QString cssstr;
	float unwidth;
	
	HBox = new QHBoxLayout;
	HBox->setAlignment(Qt::AlignLeft);
	sDesc.clear();
	sMES.clear();
	sStat.clear();
	sDesc = baseName;
	local_num = num;
	_base_pt = point;
	
	cssstr = QString::fromUtf8("font: ");
	ptstr.setNum(_base_pt);
	cssstr = cssstr + ptstr + QString::fromUtf8("pt \"Sans\"; ");
	
	Indicator = new QLabel(this);
	Indicator->setStyleSheet(cssstr);
	unwidth = _base_pt * 3.0;
	Indicator->setText(QString::fromUtf8("　"));
	_height = (int)(_base_pt * 1.5);
	
	Message = new QLabel(this);
	Message->setStyleSheet(cssstr);
	unwidth = _base_pt * (float)(width + 1);
	_width = (int)unwidth;
	setLabel(baseName);

	Message->setVisible(true);
	Indicator->setVisible(true);

	_now_width = _width;
	_now_height = _height;
	_now_pt = _base_pt;

	Message->setMinimumSize(_now_width, _now_height);
	Message->setMaximumSize(_now_width, _now_height);
	
	Indicator->setMinimumSize((int)(_now_pt * 1.2f), _now_height);
	Indicator->setMaximumSize((int)(_now_pt * 1.2f), _now_height);
	
	HBox->addWidget(Indicator);
	HBox->addWidget(Message);
	HBox->setContentsMargins(0, 0, 0, 0);
	this->setLayout(HBox);

	//this->setGeometry(0, 0, this->width(), 	_height);
}


CSP_LabelVirtualDevice::~CSP_LabelVirtualDevice()
{
}


void CSP_LabelVirtualDevice::setLabel(QString s)
{
	QString tmps;
	sDesc = s;
	tmps.setNum(local_num);
	tmps = sDesc + tmps + QString::fromUtf8(":");
	Message->setText(tmps + sMES);
}

void CSP_LabelVirtualDevice::setMessage(QString s)
{
	QString tmps;
	sMES = s;
	tmps.setNum(local_num);
	tmps = sDesc + tmps + QString::fromUtf8(":");
	Message->setText(tmps +sMES);
}

void CSP_LabelVirtualDevice::setDeviceNum(int n)
{
	local_num = n;
	setLabel(sDesc);
}

void CSP_LabelVirtualDevice::setIndicatorStatus(QString s)
{
	sStat = s;
	Indicator->setText(sStat);
}

void CSP_LabelVirtualDevice::setVisibleIndicator(bool f)
{
	Indicator->setVisible(f);
}

void CSP_LabelVirtualDevice::setVisibleMessage(bool f)
{
	Message->setVisible(f);
}

void CSP_LabelVirtualDevice::setScreenWidth(int width, int basewidth)
{
	if(basewidth <= 0) basewidth = 1280;
	float _bw = (float)basewidth;
	float _w = (float)width;
	float _mul;
	QString cssstr, ptstr;

	_mul = _w / _bw;
	_now_width = (int)((float)_width * _mul);
	_now_height = (int)((float)_height * _mul);
	_now_pt = round((_base_pt * _mul) * 100.0) / 100.0;

	Message->setMinimumSize(_now_width, _now_height);
	Message->setMaximumSize(_now_width, _now_height);
	
	Indicator->setMinimumSize((int)(_now_pt * 1.2f), _now_height);
	Indicator->setMaximumSize((int)(_now_pt * 1.2f), _now_height);
	
	cssstr = QString::fromUtf8("font: ");
	ptstr.setNum(_now_pt);
	cssstr = cssstr + ptstr + QString::fromUtf8("pt \"Sans\"; ");
	
	Message->setStyleSheet(cssstr);
	Indicator->setStyleSheet(cssstr);
}

void CSP_LabelVirtualDevice::setPixmapLabel(QPixmap p)
{
	Message->setPixmap(p);
}

void CSP_LabelVirtualDevice::setPixmapIndicator(QPixmap p)
{
	Indicator->setPixmap(p);
}


CSP_DockDisks::CSP_DockDisks(QWidget *parent, USING_FLAGS *p) :  QWidget(parent)
{
	QString ns, ms;
	const float font_pt = 14.0f;
	//const float font_pt = 20.0f;
	using_flags = p;
	int _x;
	int _wlots;
	int _wmod;
	two_rows = false;
	_x = 0;
	HVBox = new QGridLayout(this);
	HVBox->setAlignment(Qt::AlignRight);

	for(int i = 0; i < 8; i++) {
		pBinary[i] = NULL;
		pBubble[i] = NULL;
		pCart[i] = NULL;
		pFloppyDisk[i] = NULL;
		pHardDisk[i] = NULL;
		pQuickDisk[i] = NULL;
	}
	for(int i = 0; i < 2; i++) {
		pCMT[i] = NULL;
		pCompactDisc[i] = NULL;
		pLaserDisc[i] = NULL;
	}
	if(using_flags->is_use_laser_disc()) {
			pLaserDisc[0] = new CSP_LabelVirtualDevice(this, 4, font_pt, QString::fromUtf8("LD"), 0);
			HVBox->addWidget(pLaserDisc[0], 0, _x);
			pLaserDisc[0]->setVisible(true);
			_x++;
	}
	if(using_flags->is_use_compact_disc()) {
			pCompactDisc[0] = new CSP_LabelVirtualDevice(this, 4, font_pt, QString::fromUtf8("CD"), 0);
			HVBox->addWidget(pCompactDisc[0], 0, _x);
			pCompactDisc[0]->setVisible(true);
			_x++;
	}
	
	if(using_flags->is_use_cart()) {
		if(using_flags->get_max_cart() > 4) {
			_wlots = 4;
			_wmod = using_flags->get_max_cart() - 4;
			two_rows = true;
		} else {
			_wlots = using_flags->get_max_cart();
			_wmod = 0;
		}
		for(int i = 0; i < using_flags->get_max_cart(); i++) {
			pCart[i] = new CSP_LabelVirtualDevice(this, 6, font_pt, QString::fromUtf8("CART"), i);
			pCart[i]->setVisible(true);
		}
		int _xtmp = _x;
		for(int i = 0; i < _wlots; i++) {
			HVBox->addWidget(pCart[i], 0, _x);
			_x++;
		}
		for(int i = 0; i < _wmod; i++) {
			HVBox->addWidget(pCart[i + 4], 1, _xtmp);
			_xtmp++;
		}
	}
	if(using_flags->is_use_binary_file()) {
		if(using_flags->get_max_binary() > 4) {
			_wlots = 4;
			_wmod = using_flags->get_max_binary() - 4;
			two_rows = true;
		} else {
			_wlots = using_flags->get_max_binary();
			_wmod = 0;
		}
		for(int i = 0; i < using_flags->get_max_binary(); i++) {
			pBinary[i] = new CSP_LabelVirtualDevice(this, 6, font_pt, QString::fromUtf8("BIN"), i);
			pBinary[i]->setVisible(true);
		}
		int _xtmp = _x;
		for(int i = 0; i < _wlots; i++) {
			HVBox->addWidget(pBinary[i], 0, _x);
			_x++;
		}
		for(int i = 0; i < _wmod; i++) {
			HVBox->addWidget(pBinary[i + 4], 1, _xtmp);
			_xtmp++;
		}
	}
	if(using_flags->is_use_bubble()) {
		if(using_flags->get_max_bubble() > 4) {
			_wlots = 4;
			_wmod = using_flags->get_max_bubble() - 4;
			two_rows = true;
		} else {
			_wlots = using_flags->get_max_bubble();
			_wmod = 0;
		}
		for(int i = 0; i < using_flags->get_max_bubble(); i++) {
			pBubble[i] = new CSP_LabelVirtualDevice(this, 6, font_pt, QString::fromUtf8("BUB"), i);
			pBubble[i]->setVisible(true);
		}
		int _xtmp = _x;
		for(int i = 0; i < _wlots; i++) {
			HVBox->addWidget(pBubble[i], 0, _x);
			_x++;
		}
		for(int i = 0; i < _wmod; i++) {
			HVBox->addWidget(pBubble[i + 4], 1, _xtmp);
			_xtmp++;
		}
	}
	_wlots = 0;
	_wmod = 0;
	two_rows = false;
	
	if(using_flags->is_use_hdd()) {
		if(using_flags->get_max_hdd() > 4) {
			_wlots = 4;
			_wmod = using_flags->get_max_hdd() - 4;
			two_rows = true;
		} else {
			_wlots = using_flags->get_max_hdd();
			_wmod = 0;
		}
		for(int i = 0; i < using_flags->get_max_hdd(); i++) {
			pHardDisk[i] = new CSP_LabelVirtualDevice(this, 12, font_pt, QString::fromUtf8("HD"), i);
			pHardDisk[i]->setVisible(true);
		}
		int _xtmp = _x;
		for(int i = 0; i < _wlots; i++) {
			HVBox->addWidget(pHardDisk[i], 0, _x);
			_x++;
		}
		for(int i = 0; i < _wmod; i++) {
			HVBox->addWidget(pHardDisk[i + 4], 1, _xtmp);
			_xtmp++;
		}
	}
	_wlots = 0;
	_wmod = 0;
	two_rows = false;
	if(using_flags->is_use_fd()) {
		if(using_flags->get_max_drive() > 4) {
			_wlots = 4;
			_wmod = using_flags->get_max_drive() - 4;
			two_rows = true;
		} else {
			_wlots = using_flags->get_max_drive();
			_wmod = 0;
		}
		for(int i = 0; i < using_flags->get_max_drive(); i++) {
			pFloppyDisk[i] = new CSP_LabelVirtualDevice(this, 12, font_pt, QString::fromUtf8("FD"), i);
			pFloppyDisk[i]->setVisible(true);
		}
		int _xtmp = _x;
		for(int i = 0; i < _wlots; i++) {
			HVBox->addWidget(pFloppyDisk[i], 0, _x);
			_x++;
		}
		for(int i = 0; i < _wmod; i++) {
			HVBox->addWidget(pFloppyDisk[i + 4], 1, _xtmp);
			_xtmp++;
		}
	}
	if(using_flags->is_use_qd()) {
		for(int i = 0; i < using_flags->get_max_qd(); i++) {
			pQuickDisk[i] = new CSP_LabelVirtualDevice(this, 4, font_pt, QString::fromUtf8("QD"), i);
			pQuickDisk[i]->setVisible(true);
			HVBox->addWidget(pQuickDisk[i], 0, _x);
			_x++;
		}
	}
	if(using_flags->is_use_tape()) {
		for(int i = 0; i < using_flags->get_max_tape(); i++) {
			pCMT[i] = new CSP_LabelVirtualDevice(this, 12, font_pt, QString::fromUtf8("CMT"), i);
			//HBox->addWidget(pCMT[i]);
			HVBox->addWidget(pCMT[i], 0, _x);
			pCMT[i]->setVisible(true);
			_x++;
		}
	}
	{
		// ToDo: HDD.
	}
	HVBox->setContentsMargins(0, 0, 0, 0);

	base_width = 1280;
	this->setLayout(HVBox);
	initial_width = this->width();
	initial_height = this->height();
	//initial_height = (two_rows) ? (int)(font_pt * 3.0) : (int)(font_pt * 1.5);
	this->setGeometry(0, 0, initial_width, initial_height);
	setScreenWidth(640); // This is workaround at 04e08d2708a595c518ae0bd92c1713e1854c4310. Will fix.
}

			
CSP_DockDisks::~CSP_DockDisks()
{
	
	for(int i = 0; i < 8; i++) {
		if(pBinary[i] != NULL) delete pBinary[i];
		if(pBubble[i] != NULL) delete pBubble[i];
		if(pCart[i] != NULL) delete pCart[i];
		if(pFloppyDisk[i] != NULL) delete pFloppyDisk[i];
		if(pHardDisk[i] != NULL) delete pHardDisk[i];
		if(pQuickDisk[i] != NULL) delete pQuickDisk[i];
	}
	for(int i = 0; i < 2; i++) {
		if(pCMT[i] != NULL) delete pCMT[i];
		if(pCompactDisc[i] != NULL) delete pCompactDisc[i];
		if(pLaserDisc[i] != NULL) delete pLaserDisc[i];
	}
}

void CSP_DockDisks::updateLabel(int dom, int localnum, QString str)
{
	switch(dom) {
	case CSP_DockDisks_Domain_Binary:
		if((localnum < 8) && (localnum >= 0)) {
			if(pBinary[localnum] != NULL) {
				pBinary[localnum]->setIndicatorStatus(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			if(pBubble[localnum] != NULL) {
				pBubble[localnum]->setIndicatorStatus(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			if(pCart[localnum] != NULL) {
				pCart[localnum]->setIndicatorStatus(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			if(pCMT[localnum] != NULL) {
				pCMT[localnum]->setIndicatorStatus(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			if(pCompactDisc[localnum] != NULL) {
				pCompactDisc[localnum]->setIndicatorStatus(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			if(pFloppyDisk[localnum] != NULL) {
				pFloppyDisk[localnum]->setIndicatorStatus(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			if(pHardDisk[localnum] != NULL) {
				pHardDisk[localnum]->setIndicatorStatus(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			if(pLaserDisc[localnum] != NULL) {
				pLaserDisc[localnum]->setIndicatorStatus(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 2) && (localnum >= 0)) {
			if(pQuickDisk[localnum] != NULL) {
				pQuickDisk[localnum]->setIndicatorStatus(str);
			}
		}
		break;
	default:
		break;
	}
}

void CSP_DockDisks::updateMessage(int dom, int localnum, QString str)
{
	switch(dom) {
	case CSP_DockDisks_Domain_Binary:
		if((localnum < 8) && (localnum >= 0)) {
			if(pBinary[localnum] != NULL) {
				pBinary[localnum]->setMessage(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			if(pBubble[localnum] != NULL) {
				pBubble[localnum]->setMessage(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			if(pCart[localnum] != NULL) {
				pCart[localnum]->setMessage(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			if(pCMT[localnum] != NULL) {
				pCMT[localnum]->setMessage(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			if(pCompactDisc[localnum] != NULL) {
				pCompactDisc[localnum]->setMessage(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			if(pFloppyDisk[localnum] != NULL) {
				pFloppyDisk[localnum]->setMessage(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			if(pHardDisk[localnum] != NULL) {
				pHardDisk[localnum]->setMessage(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			if(pLaserDisc[localnum] != NULL) {
				pLaserDisc[localnum]->setMessage(str);
			}
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 2) && (localnum >= 0)) {
			if(pQuickDisk[localnum] != NULL) {
				pQuickDisk[localnum]->setMessage(str);
			}
		}
		break;
	default:
		break;
	}
}

void CSP_DockDisks::setVisibleLabel(int dom, int localnum, bool enabled)
{
}

void CSP_DockDisks::setPixmap(int dom, int localnum, const QPixmap &pix)
{
#if 0
	switch(dom) {
	case CSP_DockDisks_Domain_Binary:
		if((localnum < 8) && (localnum >= 0)) {
			if(pBinary[localnum] != NULL) pBinary[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			if(pBubble[localnum] != NULL) pBubble[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			if(pCart[localnum] != NULL) pCart[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			if(pCMT[localnum] != NULL) pCMT[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			if(pCompactDisc[localnum] != NULL) pCompactDisc[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			if(pFloppyDisk[localnum] != NULL) pFloppyDisk[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			if(pHardDisk[localnum] != NULL) pHardDisk[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			if(pLaserDisc[localnum] != NULL) pLaserDisc[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 2) && (localnum >= 0)) {
			if(pQuickDisk[localnum] != NULL) pQuickDisk[localnum]->setPixmap(pix);
		}
		break;
	default:
		break;
	}
#endif
}

void CSP_DockDisks::setOrientation(int loc)
{
	return;
#if 0
	QLayout *p = this->layout();
	switch(loc) {
	case 1: // Upper
	case 2: // Lower
		delete p;
		this->setLayout(HBox);
		break;
	case 3:
	case 4:
		delete p;
		this->setLayout(VBox);
		break;
	default:
		break;
	}
#endif
}

void CSP_DockDisks::setScreenWidth(int width)
{
	float _mul = (float)width / (float)base_width;
	int now_width = (int)((float)initial_width * _mul);
	int now_height = (int)((float)initial_height * _mul);
	for(int i = 0; i < 8; i++) {
		if(pBinary[i] != NULL) pBinary[i]->setScreenWidth(width, -1);
	}
	for(int i = 0; i < 8; i++) {
		if(pCart[i] != NULL) pCart[i]->setScreenWidth(width, -1);
	}
	for(int i = 0; i < 8; i++) {
		if(pFloppyDisk[i] != NULL) pFloppyDisk[i]->setScreenWidth(width, -1);
	}
	for(int i = 0; i < 8; i++) {
		if(pBubble[i] != NULL) pBubble[i]->setScreenWidth(width, -1);
	}
	for(int i = 0; i < 2; i++) {
		if(pCMT[i] != NULL) pCMT[i]->setScreenWidth(width, -1);
	}
	for(int i = 0; i < 2; i++) {
		if(pCompactDisc[i] != NULL) pCompactDisc[i]->setScreenWidth(width, -1);
	}
	for(int i = 0; i < 2; i++) {
		if(pLaserDisc[i] != NULL) pLaserDisc[i]->setScreenWidth(width, -1);
	}
	for(int i = 0; i < 2; i++) {
		if(pQuickDisk[i] != NULL) pQuickDisk[i]->setScreenWidth(width, -1);
	}
	this->setGeometry(0, 0, now_width, now_height);
}

