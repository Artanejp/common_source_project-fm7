/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Help->About Dialog
 *  History: Oct 28, 2015 : Initial
 */


#include <QFile>
#include <QString>
#include <QByteArray>
#include <QIODevice>
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
	iconarea->setFixedSize(image.width() + 5, image.height() + 5);

	QString name;
	titlearea = new QLabel(parent);
	name = QString::fromUtf8("<div align=left><B><FONT SIZE=+1>Common Source Code Project</B></div>");
	name.append(QString::fromUtf8("<div align=center>for&nbsp;&nbsp;<FONT SIZE=+1><B>"));
	name.append(QString::fromUtf8(DEVICE_NAME));
	name.append(QString::fromUtf8("</B></FONT></div>"));
	titlearea->setText(name);
	titlearea->setAlignment(Qt::AlignRight);
   
	HBox1 = new QHBoxLayout;
	HBox1->addWidget(iconarea);
	HBox1->addWidget(titlearea);

	//BoxTitle = new QGroupBox();
	BoxTitle = new QWidget(this);
	BoxTitle->setLayout(HBox1);

	VBox = new QVBoxLayout;
	VBox->addWidget(BoxTitle);
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
