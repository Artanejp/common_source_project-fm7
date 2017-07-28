
#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "dock_disks.h"

#define NEW_LABELS(_from,lim,_l,name,_d,_p,mes,zero) {	\
		QString tmps, tmpss;							\
		for(int i = 0; i < lim; i++) {					\
			if(zero) {								\
				tmpss.setNum(i);					\
			} else {								\
				tmpss.setNum(i + 1);				\
			}										\
			tmps = name;							\
			tmps = tmps + tmpss;					\
			tmps = tmps + ": ";				\
			_p[i] = tmps;							\
			_l[i] = new QLabel(tmps, this);			\
			_l[i]->setVisible(false);									\
			_l[i]->setStyleSheet("font: 12pt \"Sans\";");				\
			_l[i]->setMinimumSize(12 * 8, 20);							\
			_l[i]->setMaximumSize(12 * 8, 20);							\
			_d[i] = new QLabel("  ", this);								\
			_d[i]->setVisible(false);									\
			_d[i]->setStyleSheet("font: 12pt \"Sans\";");				\
			_d[i]->setMinimumSize(12 * 2, 20);							\
			_d[i]->setMaximumSize(12 * 2, 20);							\
			_l[i]->setVisible(false);									\
			_d[i]->setVisible(false);									\
		}																\
	}

#define ADD_LABELS(_d,_l,pos) { \
	{																	\
		int i = pos;													\
		QWidget *_sub_hwidget, *_sub_vwidget;							\
		QHBoxLayout *_sub_hlayout;										\
		QVBoxLayout *_sub_vlayout;										\
		_sub_hlayout = new QHBoxLayout(this);							\
		_sub_vlayout = new QVBoxLayout(this);							\
		_sub_hlayout->setAlignment(Qt::AlignLeft);						\
		_sub_vlayout->setAlignment(Qt::AlignTop);						\
		_sub_hlayout->addWidget(_d[i]);									\
		_sub_hlayout->addWidget(_l[i]);									\
		_sub_vlayout->addWidget(_d[i]);									\
		_sub_vlayout->addWidget(_l[i]);									\
		_sub_hwidget = new QWidget(this);								\
		_sub_vwidget = new QWidget(this);								\
		_sub_hwidget->setLayout(_sub_hlayout);							\
		_sub_vwidget->setLayout(_sub_vlayout);							\
		HBox->addWidget(_sub_vwidget);									\
		VBox->addWidget(_sub_hwidget);									\
	}																	\
}

#define RELEASE_LABELS(lim,_l)	{			\
		for(int i = 0; i < lim; i++) {			\
			if(_l[i] != NULL) delete _l[i];		\
			_l[i] = NULL;						\
		}										\
	}
			
CSP_DockDisks::CSP_DockDisks(QWidget *parent) :  QWidget(parent)
{
	QString ns, ms;
	HBox = new QHBoxLayout(this);
	VBox = new QVBoxLayout(this);
	HBox->setAlignment(Qt::AlignRight);
	VBox->setAlignment(Qt::AlignTop);
	ns = QApplication::translate("DockDisks", "BINARY", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(0, 8, lBinary, ns, dBinary, pBinary, ms, false);
	for(int ii = 0; ii < 8; ii++) {
		wBinary = 12 * 6;
		lBinary[ii]->setMinimumSize(wBinary, 20);
		lBinary[ii]->setMaximumSize(wBinary, 20);
	}
	
	ns = QApplication::translate("DockDisks", "FD", 0);
	ms =  QApplication::translate("DockDisks", "", 0);
	NEW_LABELS(8, 8, lFloppyDisk, ns, dFloppyDisk, pFloppyDisk, ms, true);
	for(int ii = 0; ii < 8; ii++) {
		wFloppyDisk = 12 * 14;
		lFloppyDisk[ii]->setMinimumSize(wFloppyDisk, 20);
		lFloppyDisk[ii]->setMaximumSize(wFloppyDisk, 20);
	}

	ns = QApplication::translate("DockDisks", "CMT", 0);
	ms =  QApplication::translate("DockDisks", "     *EJECT*      ", 0);
	NEW_LABELS(16, 2, lCMT, ns, dCMT, pCMT, ms, false);
	for(int ii = 0; ii < 2; ii++) {
		wCMT = 12 * 14;
		lCMT[ii]->setMinimumSize(wCMT, 20);
		lCMT[ii]->setMaximumSize(wCMT, 20);
	}

	ns = QApplication::translate("DockDisks", "BUBBLE", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(18, 8, lBubble, ns, dBubble, pBubble, ms, false);
	for(int ii = 0; ii < 8; ii++) {
		wBubble = 12 * 8;
		lBubble[ii]->setMinimumSize(wBubble, 20);
		lBubble[ii]->setMaximumSize(wBubble, 20);
	}
	
	ns = QApplication::translate("DockDisks", "CART", 0);
	ms =  QApplication::translate("DockDisks", "**", 0);
	NEW_LABELS(26, 8, lCart, ns, dCart, pCart, ms, false);
	for(int ii = 0; ii < 8; ii++) {
		wCart = 12 * 6;
		lCart[ii]->setMinimumSize(wCart, 20);
		lCart[ii]->setMaximumSize(wCart, 20);
	}
	
	ns = QApplication::translate("DockDisks", "QD", 0);
	ms =  QApplication::translate("DockDisks", "*EJECT*", 0);
	NEW_LABELS(34, 8, lQuickDisk, ns, dQuickDisk, pQuickDisk, ms, false);
	for(int ii = 0; ii < 8; ii++) {
		wQuickDisk = 12 * 8;
		lQuickDisk[ii]->setMinimumSize(wQuickDisk, 20);
		lQuickDisk[ii]->setMaximumSize(wQuickDisk, 20);
	}
	ns = QApplication::translate("DockDisks", "CD", 0);
	ms =  QApplication::translate("DockDisks", "*EJECT*", 0);
	NEW_LABELS(42, 2, lCompactDisc, ns, dCompactDisc, pCompactDisc, ms, false);
	for(int ii = 0; ii < 2; ii++) {
		wCompactDisc = 12 * 4;
		lCompactDisc[ii]->setMinimumSize(wCompactDisc, 20);
		lCompactDisc[ii]->setMaximumSize(wCompactDisc, 20);
	}		
	
	ns = QApplication::translate("DockDisks", "HDD", 0);
	ms =  QApplication::translate("DockDisks", "", 0);
	NEW_LABELS(44, 8, lHardDisk, ns, dHardDisk, pHardDisk, ms, true);
	for(int ii = 0; ii < 8; ii++) {
		wHardDisk = 12 * 6;
		lHardDisk[ii]->setMinimumSize(wHardDisk, 20);
		lHardDisk[ii]->setMaximumSize(wHardDisk, 20);
	}		
	
	ns = QApplication::translate("DockDisks", "LD", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(52, 2, lLaserDisc, ns, dLaserDisc, pLaserDisc, ms, false);
	for(int ii = 0; ii < 2; ii++) {
		wLaserDisc = 12 * 4;
		lLaserDisc[ii]->setMinimumSize(wLaserDisc, 20);
		lLaserDisc[ii]->setMaximumSize(wLaserDisc, 20);
	}
	this->setLayout(HBox);
}

			
CSP_DockDisks::~CSP_DockDisks()
{
	RELEASE_LABELS(8,lBinary);
	RELEASE_LABELS(8,lFloppyDisk);
	RELEASE_LABELS(2,lCMT);
	RELEASE_LABELS(8,lBubble);
	RELEASE_LABELS(8,lCart);
	RELEASE_LABELS(8,lQuickDisk);
	RELEASE_LABELS(2,lCompactDisc);
	RELEASE_LABELS(8,lHardDisk);
	RELEASE_LABELS(2,lLaserDisc);
	
	RELEASE_LABELS(8,dBinary);
	RELEASE_LABELS(8,dFloppyDisk);
	RELEASE_LABELS(2,dCMT);
	RELEASE_LABELS(8,dBubble);
	RELEASE_LABELS(8,dCart);
	RELEASE_LABELS(8,dQuickDisk);
	RELEASE_LABELS(2,dCompactDisc);
	RELEASE_LABELS(8,dHardDisk);
	RELEASE_LABELS(2,dLaserDisc);
}

void CSP_DockDisks::updateLabel(int dom, int localnum, QString str)
{
	switch(dom) {
	case CSP_DockDisks_Domain_Binary:
		if((localnum < 8) && (localnum >= 0)) {
			dBinary[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			dBubble[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			dCart[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			dCMT[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			dCompactDisc[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			dFloppyDisk[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			dHardDisk[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			dLaserDisc[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 8) && (localnum >= 0)) {
			dQuickDisk[localnum]->setText(str);
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
			str = pBinary[localnum] + str;
			lBinary[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			str = pBubble[localnum] + str;
			lBubble[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			str = pCart[localnum] + str;
			lCart[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			str = pCMT[localnum] + str;
			lCMT[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			str = pCompactDisc[localnum] + str;
			lCompactDisc[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			str = pFloppyDisk[localnum] + str;
			lFloppyDisk[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			str = pHardDisk[localnum] + str;
			lHardDisk[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			str = pLaserDisc[localnum] + str;
			lLaserDisc[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 8) && (localnum >= 0)) {
			lQuickDisk[localnum]->setText(str);
		}
		break;
	default:
		break;
	}
}

void CSP_DockDisks::setVisibleLabel(int dom, int localnum, bool enabled)
{
	switch(dom) {
	case CSP_DockDisks_Domain_Binary:
		if((localnum < 8) && (localnum >= 0)) {
			lBinary[localnum]->setVisible(enabled);
			//dBinary[localnum]->setVisible(enabled);
			dBinary[localnum]->setVisible(false);
			ADD_LABELS(dBinary, lBinary, localnum);
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			lBubble[localnum]->setVisible(enabled);
			dBubble[localnum]->setVisible(enabled);
			ADD_LABELS(dBubble, lBubble, localnum);
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			lCart[localnum]->setVisible(enabled);
			dCart[localnum]->setVisible(false);
			ADD_LABELS(dCart, lCart, localnum);
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			lCMT[localnum]->setVisible(enabled);
			dCMT[localnum]->setVisible(false);
			ADD_LABELS(dCMT, lCMT, localnum);
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			lCompactDisc[localnum]->setVisible(enabled);
			dCompactDisc[localnum]->setVisible(enabled);
			ADD_LABELS(dCompactDisc, lCompactDisc, localnum);
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			lFloppyDisk[localnum]->setVisible(enabled);
			dFloppyDisk[localnum]->setVisible(enabled);
			ADD_LABELS(dFloppyDisk, lFloppyDisk, localnum);
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			lHardDisk[localnum]->setVisible(enabled);
			dHardDisk[localnum]->setVisible(enabled);
			ADD_LABELS(dHardDisk, lHardDisk, localnum);
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			lLaserDisc[localnum]->setVisible(enabled);
			dLaserDisc[localnum]->setVisible(enabled);
			ADD_LABELS(dLaserDisc, lLaserDisc, localnum);
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 8) && (localnum >= 0)) {
			lQuickDisk[localnum]->setVisible(enabled);
			dQuickDisk[localnum]->setVisible(enabled);
			ADD_LABELS(dQuickDisk, lQuickDisk, localnum);
		}
		break;
	default:
		break;
	}
}

void CSP_DockDisks::setPixmap(int dom, int localnum, const QPixmap &pix)
{
	switch(dom) {
	case CSP_DockDisks_Domain_Binary:
		if((localnum < 8) && (localnum >= 0)) {
			lBinary[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			lBubble[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			lCart[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			lCMT[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			lCompactDisc[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			lFloppyDisk[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			lHardDisk[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			lLaserDisc[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 8) && (localnum >= 0)) {
			lQuickDisk[localnum]->setPixmap(pix);
		}
		break;
	default:
		break;
	}
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
