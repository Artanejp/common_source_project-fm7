/*
 * UI->Qt->MainWindow : Bubble Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Mar 24, 2016 : Initial.
 */

#include "mainwidget_base.h"
#include "commonclasses.h"
#include "menu_bubble.h"

#include "qt_dialogs.h"
#include "agar_logger.h"

extern class EMU *emu;

void Object_Menu_Control::insert_bubble(void) {
	emit sig_insert_bubble(getDrive());
}
void Object_Menu_Control::eject_bubble(void) {
	write_protect = false;
	emit sig_eject_bubble(getDrive());
}
void Object_Menu_Control::on_b77_slot(void) {
	emit set_b77_slot(drive, s_num);
}
void Object_Menu_Control::on_recent_bubble(void){
	emit set_recent_bubble(drive, s_num);
}
void Object_Menu_Control::write_protect_bubble(void) {
	write_protect = true;
	emit sig_write_protect_bubble(drive, write_protect);
}
void Object_Menu_Control::no_write_protect_bubble(void) {
	write_protect = false;
	emit sig_write_protect_bubble(drive, write_protect);
}



int Ui_MainWindowBase::write_protect_bubble(int drv, bool flag)
{
	if((drv < 0) || (drv >= using_flags->get_max_bubble())) return -1;
	emit sig_write_protect_bubble(drv, flag);
	return 0;
}
  
int Ui_MainWindowBase::set_b77_slot(int drive, int num)
{
	return 0;
}

void Ui_MainWindowBase::do_update_recent_bubble(int drv)
{
}


int Ui_MainWindowBase::set_recent_bubble(int drv, int num) 
{
	return 0;
}

void Ui_MainWindowBase::_open_bubble(int drv, const QString fname)
{
}

void Ui_MainWindowBase::eject_bubble(int drv) 
{
}

// Common Routine

void Ui_MainWindowBase::CreateBubbleMenu(int drv, int drv_base)
{
	{
		QString ext = "*.b77 *.bbl";
		QString desc1 = "Bubble Casette";
		menu_bubbles[drv] = new Menu_BubbleClass(emu, menubar, QString::fromUtf8("Obj_Bubble"), using_flags, this, drv);
		menu_bubbles[drv]->create_pulldown_menu();
		
		menu_bubbles[drv]->do_clear_inner_media();
		menu_bubbles[drv]->do_add_media_extension(ext, desc1);
		SETUP_HISTORY(using_flags->get_config_ptr()->recent_bubble_casette_path[drv], listBubbles[drv]);
		menu_bubbles[drv]->do_update_histories(listBubbles[drv]);
		menu_bubbles[drv]->do_set_initialize_directory(using_flags->get_config_ptr()->initial_bubble_casette_dir);
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
