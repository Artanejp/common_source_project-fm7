/*
 */


#include <QFile>
#include <QString>
#include <QByteArray>
#include <QTextBrowser>
#include <QLabel>
#include <QIODevice>
#include <QUrl>
#include <QStringList>

#include "menuclasses.h"
#include "display_text_document.h"


Dlg_BrowseText::Dlg_BrowseText(QString fname, bool internal, QWidget *parent) : QWidget(parent)
{
	QByteArray tmps;
	QFile f_desc;
	QString str_text;
	QStringList pathes;
	QString path;
	
	parent_widget = parent;

	if(internal) {
		path = QString::fromUtf8(":/");
		path.append(fname);
	} else { 
		path = fname;
	}
	// Credits

	f_desc.setFileName(path);
	str_text.clear();
	if(f_desc.open(QIODevice::ReadOnly | QIODevice::Text)) {
		tmps = f_desc.readAll();
		if(!tmps.isEmpty()) {
			str_text = tmps;
		}
		f_desc.close();
	}
	TextBox = new QTextBrowser();
	TextBox->setText(str_text.toUtf8());
	TextBox->setMinimumSize(540, 300);
	TextBox->setOpenExternalLinks(true);

	pathes << QString::fromUtf8(":/");
	TextBox->setSearchPaths(pathes);
	
	VBox = new QVBoxLayout;
	VBox->addWidget(TextBox);
	this->setLayout(VBox);

	QString title;
	title = QString::fromUtf8("emu");
	title.append(QString::fromUtf8(CONFIG_NAME));
	title.append(QString::fromUtf8(" / "));
	title.append(fname);
	if(parent == NULL) this->setWindowTitle(title);
}

Dlg_BrowseText::~Dlg_BrowseText()
{

}	
