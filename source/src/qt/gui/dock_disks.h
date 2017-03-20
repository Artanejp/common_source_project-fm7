#ifndef _CSP_QT_DOCKWIDGET_BASE_H
#define _CSP_QT_DOCKWIDGET_BASE_H

#include <QObject>
#include <QDockWidget>
#include <QIcon>
#include <QString>
#include <QStringList>
#include <QPixmap>

class QLabel;

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
class QVBoxLayout;

class CSP_DockDisks : public QDockWidget {
	Q_OBJECT
protected:
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
	
public:
	QLabel *pBinary[8];
	QLabel *pBubble[8];
	QLabel *pCart[8];
	QLabel *pCMT[2];
	QLabel *pCompactDisc[2];
	QLabel *pFloppyDisk[8];
	QLabel *pHardDisk[8];
	QLabel *pLaserDisc[2];
	QLabel *pQuickDisk[8];
	CSP_DockDisks(QWidget *parent, Qt::WindowFlags flags = 0);
	~CSP_DockDisks();

public slots:
	void updateLabel(int dom, int localnum, QString str);
	void updateMessage(int dom, int localnum, QString str);
	void setVisible(int dom, int localNum, bool enabled);
	void setPixmap(int dom, int localNum, const QPixmap &); 
};
QT_END_NAMESPACE

#endif
	
