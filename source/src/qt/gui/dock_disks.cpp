
#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QDockWidget>

#include "dock_disks.h"

#define NEW_LABELS(lim,_l,name,_p,mes,zero) {	\
		QString tmps, tmpss;					\
		for(int i = 0; i < lim; i++) {			\
			if(zero) {								\
				tmpss.setNum(i);					\
			} else {								\
				tmpss.setNum(i + 1);				\
			}										\
			tmps = name;							\
			tmps = tmps + tmpss;					\
			_l[i] = new QLabel(tmps, this);			\
			_p[i] = new QLabel(mes, this);			\
			_l[i]->setVisible(false);				\
			_p[i]->setVisible(false);				\
			VBox->addWidget(_l[i]);					\
			VBox->addWidget(_p[i]);					\
		}											\
	}

#define RELEASE_LABELS(lim,_l,_p)	{			\
		for(int i = 0; i < lim; i++) {			\
			if(_l[i] != NULL) delete _l[i];		\
			if(_p[i] != NULL) delete _p[i];		\
			_l[i] = NULL;						\
			_p[i] = NULL;						\
		}										\
	}
			
CSP_DockDisks::CSP_DockDisks(QWidget *parent, Qt::WindowFlags flags) :  QDockWidget(parent, flags)
{
	QString ns, ms;
	QWidget *Widget = new QWidget(this);
	VBox = new QVBoxLayout(Widget);
	VBox->setAlignment(Qt::AlignTop);
	
	ns = QApplication::translate("DockDisks", "Binary", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(8,lBinary,ns,pBinary,ms,false);
	
	ns = QApplication::translate("DockDisks", "FD", 0);
	ms =  QApplication::translate("DockDisks", "     *EMPTY*      ", 0);
	NEW_LABELS(8,lFloppyDisk,ns,pFloppyDisk,ms,true);
	
	ns = QApplication::translate("DockDisks", "CMT", 0);
	ms =  QApplication::translate("DockDisks", "     *EJECT*      ", 0);
	NEW_LABELS(2,lCMT,ns,pCMT,ms,false);
	
	ns = QApplication::translate("DockDisks", "Bubble", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(8,lBubble,ns,pBubble,ms,false);
	
	ns = QApplication::translate("DockDisks", "Cartridge", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(8,lCart,ns,pCart,ms,false);
	
	ns = QApplication::translate("DockDisks", "Quick Disk", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(8,lQuickDisk,ns,pQuickDisk,ms,false);

	ns = QApplication::translate("DockDisks", "CD", 0);
	ms =  QApplication::translate("DockDisks", "*EJECT*", 0);
	NEW_LABELS(2,lCompactDisc,ns,pCompactDisc,ms,false);
	
	ns = QApplication::translate("DockDisks", "HDD", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(8,lHardDisk,ns,pHardDisk,ms,true);
	
	ns = QApplication::translate("DockDisks", "Laser Disc", 0);
	ms =  QApplication::translate("DockDisks", "*EMPTY*", 0);
	NEW_LABELS(2,lLaserDisc,ns,pLaserDisc,ms,false);
	
	Widget->setLayout(VBox);
	this->setWidget(Widget);
}


			
CSP_DockDisks::~CSP_DockDisks()
{
	RELEASE_LABELS(8,lBinary,pBinary);
	RELEASE_LABELS(8,lFloppyDisk,pFloppyDisk);
	RELEASE_LABELS(2,lCMT,pCMT);
	RELEASE_LABELS(8,lBubble,pBubble);
	RELEASE_LABELS(8,lCart,pCart);
	RELEASE_LABELS(8,lQuickDisk,pQuickDisk);
	RELEASE_LABELS(2,lCompactDisc,pCompactDisc);
	RELEASE_LABELS(8,lHardDisk,pHardDisk);
	RELEASE_LABELS(2,lLaserDisc,pLaserDisc);
}

void CSP_DockDisks::updateLabel(int dom, int localnum, QString str)
{
	switch(dom) {
	case CSP_DockDisks_Domain_Binary:
		if((localnum < 8) && (localnum >= 0)) {
			lBinary[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			lBubble[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			lCart[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			lCMT[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			lCompactDisc[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			lFloppyDisk[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			lHardDisk[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
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

void CSP_DockDisks::updateMessage(int dom, int localnum, QString str)
{
	switch(dom) {
	case CSP_DockDisks_Domain_Binary:
		if((localnum < 8) && (localnum >= 0)) {
			pBinary[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			pBubble[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			pCart[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			pCMT[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			pCompactDisc[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			pFloppyDisk[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			pHardDisk[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			pLaserDisc[localnum]->setText(str);
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 8) && (localnum >= 0)) {
			pQuickDisk[localnum]->setText(str);
		}
		break;
	default:
		break;
	}
}

void CSP_DockDisks::setVisible(int dom, int localnum, bool enabled)
{
	switch(dom) {
	case CSP_DockDisks_Domain_Binary:
		if((localnum < 8) && (localnum >= 0)) {
			lBinary[localnum]->setVisible(enabled);
			pBinary[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			lBubble[localnum]->setVisible(enabled);
			pBubble[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			lCart[localnum]->setVisible(enabled);
			pCart[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			lCMT[localnum]->setVisible(enabled);
			pCMT[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			lCompactDisc[localnum]->setVisible(enabled);
			pCompactDisc[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			lFloppyDisk[localnum]->setVisible(enabled);
			pFloppyDisk[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			lHardDisk[localnum]->setVisible(enabled);
			pHardDisk[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			lLaserDisc[localnum]->setVisible(enabled);
			pLaserDisc[localnum]->setVisible(enabled);
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 8) && (localnum >= 0)) {
			lQuickDisk[localnum]->setVisible(enabled);
			pQuickDisk[localnum]->setVisible(enabled);
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
			pBinary[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_Bubble:
		if((localnum < 8) && (localnum >= 0)) {
			pBubble[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_Cart:
		if((localnum < 8) && (localnum >= 0)) {
			pCart[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_CMT:
		if((localnum < 2) && (localnum >= 0)) {
			pCMT[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_CD:
		if((localnum < 2) && (localnum >= 0)) {
			pCompactDisc[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_FD:
		if((localnum < 8) && (localnum >= 0)) {
			pFloppyDisk[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_HD:
		if((localnum < 8) && (localnum >= 0)) {
			pHardDisk[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_LD:
		if((localnum < 2) && (localnum >= 0)) {
			pLaserDisc[localnum]->setPixmap(pix);
		}
		break;
	case CSP_DockDisks_Domain_QD:
		if((localnum < 8) && (localnum >= 0)) {
			pQuickDisk[localnum]->setPixmap(pix);
		}
		break;
	default:
		break;
	}
}
