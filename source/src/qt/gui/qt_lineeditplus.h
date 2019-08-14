#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QString>
#include <QStringList>

#define QLINEEDITPLUS_MAX_HISTORY 32
QT_BEGIN_NAMESPACE	

class QLineEditPlus : public QLineEdit {
	Q_OBJECT
protected:
	QStringList list;
	int pointer;
	virtual void keyPressEvent(QKeyEvent *event);
public:
	QLineEditPlus(const QString &contents, QWidget *parent = nullptr);
	~QLineEditPlus();
	int historyCount(void);
	int maxCount(void);
	QStringList &getList(void);
public slots:
	void clearHistory(void);
	void redirectEditString2(void);
signals:
	int editingFinished2();
};

QT_END_NAMESPACE	
