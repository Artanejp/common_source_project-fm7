#ifndef _CSP_QT_DOCKWIDGET_BASE_H
#define _CSP_QT_DOCKWIDGET_BASE_H

#include <QObject>
#include <QWidget>
//#include <QToolBar>
//#include <QDockWidget>
#include <QIcon>
#include <QString>
#include <QStringList>
#include <QPixmap>

class QLabel;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;

enum {
	CSP_DockDisks_Domain_Binary = 0,
	CSP_DockDisks_Domain_Bubble,
	CSP_DockDisks_Domain_Cart,
	CSP_DockDisks_Domain_CMT,
	CSP_DockDisks_Domain_CD,
	CSP_DockDisks_Domain_FD,
	CSP_DockDisks_Domain_HD,
	CSP_DockDisks_Domain_LD,
	CSP_DockDisks_Domain_QD,
};
	
QT_BEGIN_NAMESPACE
class CSP_DockDisks : public QWidget {
	Q_OBJECT
private:
	QHBoxLayout *HBox;
	QVBoxLayout *VBox;
	QLabel *lBinary[8];
	QLabel *lBubble[8];
	QLabel *lCart[8];
	QLabel *lCMT[2];
	QLabel *lCompactDisc[2];
	QLabel *lFloppyDisk[8];
	QLabel *lHardDisk[8];
	QLabel *lLaserDisc[2];
	QLabel *lQuickDisk[8];
	
	QLabel *dBinary[8];
	QLabel *dBubble[8];
	QLabel *dCart[8];
	QLabel *dCMT[2];
	QLabel *dCompactDisc[2];
	QLabel *dFloppyDisk[8];
	QLabel *dHardDisk[8];
	QLabel *dLaserDisc[2];
	QLabel *dQuickDisk[8];

	QString pBinary[8];
	QString pBubble[8];
	QString pCart[8];
	QString pCMT[2];
	QString pCompactDisc[2];
	QString pFloppyDisk[8];
	QString pHardDisk[8];
	QString pLaserDisc[2];
	QString pQuickDisk[8];

	int wBinary;
	int wBubble;
	int wCart;
	int wCMT;
	int wCompactDisc;
	int wFloppyDisk;
	int wHardDisk;
	int wLaserDisc;
	int wQuickDisk;
public:
	CSP_DockDisks(QWidget *parent);
	~CSP_DockDisks();

public slots:
	void updateLabel(int dom, int localnum, QString str);
	void updateMessage(int dom, int localnum, QString str);
	void setVisibleLabel(int dom, int localNum, bool enabled);
	void setPixmap(int dom, int localNum, const QPixmap &);
	void setOrientation(int loc);
};
QT_END_NAMESPACE

#endif
	
