/*
 * UI->Qt->MainWindow : Bubble Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Mar 24, 2016 : Initial.
 */
#include <QApplication>
#include <QVariant>
#include <QAction>

#include "mainwidget_base.h"
#include "commonclasses.h"
#include "menu_bubble.h"

#include "qt_dialogs.h"
//#include "csp_logger.h"

int Ui_MainWindowBase::do_write_protect_bubble(int drv, bool flag)
{
	if((drv < 0) || (drv >= using_flags->get_max_bubble())) return -1;
	emit sig_write_protect_bubble(drv, flag);
	return 0;
}


// Common Routine

void Ui_MainWindowBase::CreateBubbleMenu(int drv, int drv_base)
{
	{
		QString ext = "*.b77 *.bbl";
		QString desc1 = "Bubble Casette";
		menu_bubbles[drv] = new Menu_BubbleClass(menubar, QString::fromUtf8("Obj_Bubble"), using_flags, this, drv);
		menu_bubbles[drv]->create_pulldown_menu();

		menu_bubbles[drv]->do_clear_inner_media();
		menu_bubbles[drv]->do_add_media_extension(ext, desc1);
		SETUP_HISTORY(p_config->recent_bubble_casette_path[drv], listBubbles[drv]);
		menu_bubbles[drv]->do_update_histories(listBubbles[drv]);
		menu_bubbles[drv]->do_set_initialize_directory(p_config->initial_bubble_casette_dir);
		listB77[drv].clear();

		QString name = QString::fromUtf8("BUBBLE");
		QString tmpv;
		tmpv.setNum(drv_base);
		name.append(tmpv);
		menu_bubbles[drv]->setTitle(name);
	}
}

void Ui_MainWindowBase::CreateBubblePulldownMenu(int drv)
{
}

void Ui_MainWindowBase::ConfigBubbleMenuSub(int drv)
{
}

void Ui_MainWindowBase::retranslateBubbleMenu(int drv, int basedrv)
{
	QString drive_name = (QApplication::translate("MainWindow", "Bubble ", 0));
	drive_name += QString::number(basedrv);

	if((drv < 0) || (drv >= 8)) return;
	menu_bubbles[drv]->setTitle(QApplication::translate("MainWindow", drive_name.toUtf8().constData() , 0));
	menu_bubbles[drv]->retranslateUi();
}

void Ui_MainWindowBase::ConfigBubbleMenu(void)
{
	for(int i = 0; i < using_flags->get_max_bubble(); i++) {
		ConfigBubbleMenuSub(i);
	}
}
