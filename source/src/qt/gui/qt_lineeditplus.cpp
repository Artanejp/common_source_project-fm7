#include <QKeyEvent>
#include <QCompleter>
#include "qt_lineeditplus.h"

QLineEditPlus::QLineEditPlus(const QString &contents, QWidget *parent) : QLineEdit(contents, parent)
{
	list.clear();
	pointer = 0;
	completer = NULL;
	connect(this, SIGNAL(editingFinished()), this, SLOT(redirectEditString2()));
}

QLineEditPlus::~QLineEditPlus()
{
	if(completer != NULL) delete completer;
}

void QLineEditPlus::keyPressEvent(QKeyEvent *event)
{
	int keyCode = event->key();
	QString _text;
	switch(keyCode) {
	case Qt::Key_Up:
		if(pointer < list.count()) {
			_text = list.at(pointer);
			if(!(_text.isEmpty())) {
				setText(_text);
			}
			pointer++;
			if(pointer >= list.count()) pointer = list.count() - 1;
			return;
		}
	case Qt::Key_Down:
		if(pointer > 0) {
			pointer--;
			_text = list.at(pointer);
			if(!(_text.isEmpty())) {
				setText(_text);
			}
			return;
		}
		break;
	}
	QLineEdit::keyPressEvent(event);
}

void QLineEditPlus::clearHistory(void)
{
	list.clear();
	pointer = 0;
}

int QLineEditPlus::historyCount(void)
{
	return list.count();
}

QStringList &QLineEditPlus::getList(void)
{
	return list;
}

int QLineEditPlus::maxCount(void)
{
	return QLINEEDITPLUS_MAX_HISTORY;
}

QCompleter *QLineEditPlus::getCompleter(void)
{
	return completer;
}

void QLineEditPlus::redirectEditString2(void)
{
	if(this->text().isEmpty()) return;
	QString _text = this->text();
	QString _first = QString::fromUtf8("");
	if(!(list.isEmpty())) {
		_first = list.first();
	}
	if(_text != _first) {
		list.prepend(_text);
		if(list.count() >= QLINEEDITPLUS_MAX_HISTORY) {
			list.removeLast();
		}
		pointer = 0; // Pointer must be tail.
	}
	emit editingFinished2();
}

void QLineEditPlus::setCompleteList(QStringList complist)
{
	if(completer != NULL) delete completer;
//	printf("COMP: %d\n", (complist.isEmpty()) ? 0 : 1);
	if(complist.isEmpty()) {
		completer = NULL;
	} else {
		completer = new QCompleter(complist, this);
		//completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
		completer->setCompletionMode(QCompleter::PopupCompletion);
		completer->setCaseSensitivity(Qt::CaseInsensitive);
	}
	setCompleter(completer);
}

