
#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>
#include "dock_disks.h"

#define NEW_LABELS(lim,_l,name,_d,_p,mes,zero) {		\
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
			_l[i]->setVisible(false);				\
			_l[i]->setStyleSheet("font: 12pt \"Sans\";"); \
			_l[i]->setMinimumSize(12 * 8, 20);			  \
			_l[i]->setMaximumSize(12 * 8, 20);			  \
			_d[i] = new QLabel("  ", this);			\
			_d[i]->setVisible(false);				\
			_d[i]->setStyleSheet("font: 12pt \"Sans\";"); \
			_d[i]->setMinimumSize(12 * 1, 20);			  \
			_d[i]->setMaximumSize(12 * 1, 20);			  \
			HBox->addWidget(_d[i]);					\
			HBox->addWidget(_l[i]);					\
		}											\
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
	QHBoxLayout *HBox = new QHBoxLayout(this);
	HBox->setAlignment(Qt::AlignRight);
	ns = QApplication::translate("DockDisks", "BINARY", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(8, lBinary, ns, dBinary, pBinary, ms, false);
	for(int ii = 0; ii < 8; ii++) {
		lBinary[ii]->setMinimumSize(12 * 10, 20);
		lBinary[ii]->setMaximumSize(12 * 10, 20);
	}
	
	ns = QApplication::translate("DockDisks", "FD", 0);
	ms =  QApplication::translate("DockDisks", "", 0);
	NEW_LABELS(8, lFloppyDisk, ns, dFloppyDisk, pFloppyDisk, ms, true);
	for(int ii = 0; ii < 8; ii++) {
		lFloppyDisk[ii]->setMinimumSize(12 * 22, 20);
		lFloppyDisk[ii]->setMaximumSize(12 * 22, 20);
	}

	ns = QApplication::translate("DockDisks", "CMT", 0);
	ms =  QApplication::translate("DockDisks", "     *EJECT*      ", 0);
	NEW_LABELS(2, lCMT, ns, dCMT, pCMT, ms, false);
	for(int ii = 0; ii < 2; ii++) {
		lCMT[ii]->setMinimumSize(12 * 12, 20);
		lCMT[ii]->setMaximumSize(12 * 12, 20);
	}

	ns = QApplication::translate("DockDisks", "BUBBLE", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(8, lBubble, ns, dBubble, pBubble, ms, false);
	for(int ii = 0; ii < 8; ii++) {
		lBubble[ii]->setMinimumSize(12 * 10, 20);
		lBubble[ii]->setMaximumSize(12 * 10, 20);
	}
	
	ns = QApplication::translate("DockDisks", "CARTRIDGE", 0);
	ms =  QApplication::translate("DockDisks", "**", 0);
	NEW_LABELS(8, lCart, ns, dCart, pCart, ms, false);
	for(int ii = 0; ii < 8; ii++) {
		lCart[ii]->setMinimumSize(12 * 12, 20);
		lCart[ii]->setMaximumSize(12 * 12, 20);
	}
	
	ns = QApplication::translate("DockDisks", "QD", 0);
	ms =  QApplication::translate("DockDisks", "*EJECT*", 0);
	NEW_LABELS(8, lQuickDisk, ns, dQuickDisk, pQuickDisk, ms, false);
	for(int ii = 0; ii < 8; ii++) {
		lQuickDisk[ii]->setMinimumSize(12 * 10, 20);
		lQuickDisk[ii]->setMaximumSize(12 * 10, 20);
	}
	ns = QApplication::translate("DockDisks", "CD", 0);
	ms =  QApplication::translate("DockDisks", "*EJECT*", 0);
	NEW_LABELS(2, lCompactDisc, ns, dCompactDisc, pCompactDisc, ms, false);
	for(int ii = 0; ii < 2; ii++) {
		lCompactDisc[ii]->setMinimumSize(12 * 10, 20);
		lCompactDisc[ii]->setMaximumSize(12 * 10, 20);
	}		
	
	ns = QApplication::translate("DockDisks", "HDD", 0);
	ms =  QApplication::translate("DockDisks", "", 0);
	NEW_LABELS(8, lHardDisk, ns, dHardDisk, pHardDisk, ms, true);
	for(int ii = 0; ii < 8; ii++) {
		lHardDisk[ii]->setMinimumSize(12 * 4, 20);
		lHardDisk[ii]->setMaximumSize(12 * 4, 20);
	}		
	
	ns = QApplication::translate("DockDisks", "Laser Disc", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(2, lLaserDisc, ns, dLaserDisc, pLaserDisc, ms, false);
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
			dBinary[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			lBubble[localnum]->setVisible(enabled);
			dBubble[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			lCart[localnum]->setVisible(enabled);
			dCart[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			lCMT[localnum]->setVisible(enabled);
			dCMT[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			lCompactDisc[localnum]->setVisible(enabled);
			dCompactDisc[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			lFloppyDisk[localnum]->setVisible(enabled);
			dFloppyDisk[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			lHardDisk[localnum]->setVisible(enabled);
			dHardDisk[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			lLaserDisc[localnum]->setVisible(enabled);
			dLaserDisc[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 8) && (localnum >= 0)) {
			lQuickDisk[localnum]->setVisible(enabled);
			dQuickDisk[localnum]->setVisible(enabled);
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
