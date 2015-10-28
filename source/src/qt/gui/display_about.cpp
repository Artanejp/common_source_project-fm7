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
#include "display_about.h"


Dlg_AboutCSP::Dlg_AboutCSP(QWidget *parent) : QWidget(parent)
{
	QByteArray tmps;
	QFile f_credits(":/credits.html");
	QString credits;
	QImage image;
	QUrl url;
	QStringList pathes;
	
	parent_widget = parent;
	// Credits
	credits.clear();
	if(f_credits.open(QIODevice::ReadOnly | QIODevice::Text)) {
		tmps = f_credits.readAll();
		if(!tmps.isEmpty()) {
			credits = tmps;
		}
		f_credits.close();
	}
	//credits = QString::fromUtf8("file:::/credits.html");
	//url = QUrl::fromEncoded("file://:/credits.html");
	TextBox = new QTextBrowser();
	TextBox->setHtml(credits);
	TextBox->setMinimumSize(540, 300);
	TextBox->setOpenExternalLinks(true);

	pathes << QString::fromUtf8(":/");
	TextBox->setSearchPaths(pathes);
	
	image.load(":/default.ico");

	iconarea = new QLabel(parent);
	iconarea->setPixmap(QPixmap::fromImage(image));

	
	VBox = new QVBoxLayout;
	VBox->addWidget(iconarea);
	VBox->addWidget(TextBox);
	this->setLayout(VBox);

	QString title;
	title = QString::fromUtf8("About emu");
	title.append(QString::fromUtf8(CONFIG_NAME));
	if(parent == NULL) this->setWindowTitle(title);
}

Dlg_AboutCSP::~Dlg_AboutCSP()
{

}	
