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

#include "display_about.h"
#include "menu_flags.h"
#include "mainwidget_base.h"

//extern USING_FLAGS *using_flags;

Dlg_AboutCSP::Dlg_AboutCSP(USING_FLAGS *p, QWidget *parent) : QWidget(NULL)
{
	QByteArray tmps;
	QFile f_credits(":/credits.html");
	QFile f_rev(":/revision.txt");
	QString credits;
	QString rev;
	QImage image;
	QUrl url;
	QStringList pathes;
	
	parent_widget = parent;
	using_flags = p;
	// Credits
	credits.clear();
	printf("%x\n",parent_widget);
	if(f_credits.open(QIODevice::ReadOnly | QIODevice::Text)) {
		tmps = f_credits.readAll();
		if(!tmps.isEmpty()) {
			QString ss;
			QString bs;
			ss.clear();
			QString ns = QString::fromUtf8(tmps);
			if(parent != NULL) {
				ss = static_cast<Ui_MainWindowBase *>(parent)->get_system_version();
				bs = static_cast<Ui_MainWindowBase *>(parent)->get_build_date();
			}
			QString reps = QString::fromUtf8("@@RevisionString@@");
			int ni = ns.indexOf(reps);
			if(ni >= 0) {
				ns.replace(ni, reps.length(), ss);
			}
			reps = QString::fromUtf8("@@BuildDateAt@@");
			ni = ns.indexOf(reps);
			if(ni >= 0) {
				ns.replace(ni, reps.length(), bs);
			}
			
			credits = ns;
		}
		f_credits.close();
	}
	//credits = QString::fromUtf8("file:::/credits.html");
	//url = QUrl::fromEncoded("file://:/credits.html");
	TextBox = new QTextBrowser();
	TextBox->setStyleSheet("font: 12pt \"Sans\";");
	TextBox->setHtml(credits);
	TextBox->setMinimumSize(640, 420);
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
	name.append(using_flags->get_device_name());
	name.append(QString::fromUtf8("</B></FONT></div>"));
	titlearea->setText(name);
	titlearea->setAlignment(Qt::AlignRight);
	titlearea->setStyleSheet("font: 12pt \"Sans\";");
   
	rev.clear();
	if(f_rev.open(QIODevice::ReadOnly | QIODevice::Text)) {
		tmps = f_rev.readAll();
		if(!tmps.isEmpty()) {
			rev = tmps;
		}
		f_rev.close();
	}
	revarea = new QLabel(parent);
	revarea->setText(rev);
	revarea->setAlignment(Qt::AlignRight);
   
	HBox1 = new QHBoxLayout;
	HBox1->addWidget(iconarea);
	HBox1->addWidget(titlearea);

	//BoxTitle = new QGroupBox();
	BoxTitle = new QWidget(this);
	BoxTitle->setLayout(HBox1);

	VBox = new QVBoxLayout;
	VBox->addWidget(BoxTitle);
	VBox->addWidget(revarea);
	VBox->addWidget(TextBox);
	this->setLayout(VBox);

	QString title;
	title = QString::fromUtf8("About emu");
	title.append(using_flags->get_config_name());
	if(parent == NULL) this->setWindowTitle(title);
}

Dlg_AboutCSP::~Dlg_AboutCSP()
{

}	

