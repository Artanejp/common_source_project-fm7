
#include <QtCore/QVariant>
#include <QtGui>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QPixmap>

#include "commonclasses.h"
#include "display_about.h"
#include "display_text_document.h"
#include "mainwidget.h"
//#include "menuclasses.h"
#include "menu_disk.h"
#include "menu_cmt.h"
#include "menu_cart.h"
#include "menu_quickdisk.h"
#include "menu_binary.h"
#include "menu_compactdisc.h"
#include "menu_bubble.h"

#include "qt_gldraw.h"
#include "emu.h"
#include "qt_main.h"
#include "menu_flags.h"

Ui_MainWindow::Ui_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent) : Ui_MainWindowBase(p, logger, parent)
{
}

Ui_MainWindow::~Ui_MainWindow()
{
}
