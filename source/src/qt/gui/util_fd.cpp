/*
 * UI->Qt->MainWindow : FDD Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */


#include "mainwidget_base.h"
#include "commonclasses.h"
#include "menu_disk.h"

#include "qt_dialogs.h"
#include "csp_logger.h"

#include "menu_flags.h"

//extern USING_FLAGS *using_flags;
//extern class EMU *emu;

void Object_Menu_Control::insert_fd(void) {
	emit sig_insert_fd(getDrive());
}
void Object_Menu_Control::eject_fd(void) {
	write_protect = false;
	emit sig_eject_fd(getDrive());
}
void Object_Menu_Control::on_d88_slot(void) {
	emit set_d88_slot(drive, s_num);
}
void Object_Menu_Control::on_recent_disk(void){
  //   write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit set_recent_disk(drive, s_num);
}
void Object_Menu_Control::write_protect_fd(void) {
	write_protect = true;
	emit sig_write_protect_fd(drive, write_protect);
}
void Object_Menu_Control::no_write_protect_fd(void) {
	write_protect = false;
	emit sig_write_protect_fd(drive, write_protect);
}

void Object_Menu_Control::do_set_ignore_crc_error(bool flag)
{
	using_flags->get_config_ptr()->ignore_disk_crc[drive] = flag;
	emit sig_emu_update_config();
}

void Object_Menu_Control::do_set_correct_disk_timing(bool flag)
{
	using_flags->get_config_ptr()->correct_disk_timing[drive] = flag;
	emit sig_emu_update_config();
}

void Object_Menu_Control::do_set_disk_count_immediate(bool flag)
{
	using_flags->get_config_ptr()->disk_count_immediate[drive] = flag;
	emit sig_emu_update_config();
}

int Ui_MainWindowBase::write_protect_fd(int drv, bool flag)
{
	if((drv < 0) || (drv >= using_flags->get_max_drive())) return -1;
	emit sig_write_protect_disk(drv, flag);
	return 0;
}
  
int Ui_MainWindowBase::set_d88_slot(int drive, int num)
{
	return 0;
}

void Ui_MainWindowBase::do_update_recent_disk(int drv)
{
}


int Ui_MainWindowBase::set_recent_disk(int drv, int num) 
{
	return 0;
}

void Ui_MainWindowBase::_open_disk(int drv, const QString fname)
{
}

void Ui_MainWindowBase::eject_fd(int drv) 
{
	emit sig_close_disk(drv);
	menu_fds[drv]->do_clear_inner_media();
}

// Common Routine

void Ui_MainWindowBase::CreateFloppyMenu(int drv, int drv_base)
{
	{
		QString ext = "*.d88 *.d77 *.1dd *.td0 *.imd *.dsk *.nfd *.fdi *.hdm *.hd5 *.hd4 *.hdb *.dd9 *.dd6 *.tfd *.xdf *.2d *.sf7 *.img *.ima *.vfd";
		QString desc1 = "Floppy Disk";
		menu_fds[drv] = new Menu_FDClass(menubar, QString::fromUtf8("Obj_Floppy"), using_flags, this, drv);
		menu_fds[drv]->create_pulldown_menu();
		
		menu_fds[drv]->do_clear_inner_media();
		menu_fds[drv]->do_add_media_extension(ext, desc1);
		SETUP_HISTORY(using_flags->get_config_ptr()->recent_floppy_disk_path[drv], listFDs[drv]);
		menu_fds[drv]->do_update_histories(listFDs[drv]);
		menu_fds[drv]->do_set_initialize_directory(using_flags->get_config_ptr()->initial_floppy_disk_dir);
		listD88[drv].clear();

		QString name = QString::fromUtf8("FD");
		QString tmpv;
		tmpv.setNum(drv_base);
		name.append(tmpv);
		menu_fds[drv]->setTitle(name);
	}
}

void Ui_MainWindowBase::CreateFloppyPulldownMenu(int drv)
{
}

void Ui_MainWindowBase::ConfigFloppyMenuSub(int drv)
{
}

void Ui_MainWindowBase::retranslateFloppyMenu(int drv, int basedrv)
{
	QString drive_name = (QApplication::translate("MainWindow", "Floppy ", 0));
	drive_name += QString::number(basedrv);
  
	if((drv < 0) || (drv >= using_flags->get_max_drive())) return;
	menu_fds[drv]->setTitle(QApplication::translate("MainWindow", drive_name.toUtf8().constData() , 0));
	menu_fds[drv]->retranslateUi();
}

void Ui_MainWindowBase::ConfigFloppyMenu(void)
{
	for(int i = 0; i < using_flags->get_max_drive(); i++) {
		ConfigFloppyMenuSub(i);
	}
}
