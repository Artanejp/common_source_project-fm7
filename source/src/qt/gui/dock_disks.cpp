
#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>
#include "dock_disks.h"

#define NEW_LABELS(lim,_l,name,_p,mes,zero) {		\
		QString tmps, tmpss;							\
		for(int i = 0; i < lim; i++) {					\
			if(zero) {								\
				tmpss.setNum(i);					\
			} else {								\
				tmpss.setNum(i + 1);				\
			}										\
			tmps = name;							\
			tmps = tmps + tmpss;					\
			_p[i] = tmps;							\
			_l[i] = new QLabel(tmps, this);			\
			_l[i]->setVisible(false);				\
			_l[i]->setStyleSheet("font: 12pt \"Sans\";"); \
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
	ns = QApplication::translate("DockDisks", "Binary", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(8, lBinary, ns, pBinary, ms, false);
	
	ns = QApplication::translate("DockDisks", "FD", 0);
	ms =  QApplication::translate("DockDisks", "     *EMPTY*      ", 0);
	NEW_LABELS(8, lFloppyDisk, ns, pFloppyDisk, ms, true);
	
	ns = QApplication::translate("DockDisks", "CMT", 0);
	ms =  QApplication::translate("DockDisks", "     *EJECT*      ", 0);
	NEW_LABELS(2, lCMT, ns, pCMT, ms, false);
	
	ns = QApplication::translate("DockDisks", "Bubble", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(8, lBubble, ns, pBubble, ms, false);
	
	ns = QApplication::translate("DockDisks", "Cartridge", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(8, lCart, ns, pCart, ms, false);
	
	ns = QApplication::translate("DockDisks", "Quick Disk", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(8, lQuickDisk, ns, pQuickDisk, ms, false);

	ns = QApplication::translate("DockDisks", "CD", 0);
	ms =  QApplication::translate("DockDisks", "*EJECT*", 0);
	NEW_LABELS(2, lCompactDisc, ns, pCompactDisc, ms, false);
	
	ns = QApplication::translate("DockDisks", "HDD", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(8, lHardDisk, ns, pHardDisk, ms, true);
	
	ns = QApplication::translate("DockDisks", "Laser Disc", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(2, lLaserDisc, ns, pLaserDisc, ms, false);
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
}

void CSP_DockDisks::updateLabel(int dom, int localnum, QString str)
{
	switch(dom) {
	case CSP_DockDisks_Domain_Binary:
		if((localnum < 8) && (localnum >= 0)) {
			pBinary[localnum] = str;
			lBinary[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			pBubble[localnum] = str;
			lBubble[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			pCart[localnum] = str;
			lCart[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			pCMT[localnum] = str;
			lCMT[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			pCompactDisc[localnum] = str;
			lCompactDisc[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			pFloppyDisk[localnum] = str;
			lFloppyDisk[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			pHardDisk[localnum] = str;
			lHardDisk[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			pLaserDisc[localnum] = str;
			lLaserDisc[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 8) && (localnum >= 0)) {
			pQuickDisk[localnum] = str;
			lQuickDisk[localnum]->setText(str);
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
			str = pQuickDisk[localnum] + str;
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
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			lBubble[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			lCart[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			lCMT[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			lCompactDisc[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			lFloppyDisk[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			lHardDisk[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			lLaserDisc[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 8) && (localnum >= 0)) {
			lQuickDisk[localnum]->setVisible(enabled);
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
