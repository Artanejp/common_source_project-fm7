/*
 * UI->Qt->MainWindow : Cartridge Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */

#include "mainwidget_base.h"
#include "commonclasses.h"
#include "menu_cart.h"
#include "qt_dialogs.h"

#include "dock_disks.h"


void Ui_MainWindowBase::do_ui_cartridge_insert_history(int drv, QString fname)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_cart()) || (p->get_max_cart() <= drv) || (drv < 0)) return;
	if(fname.length() <= 0) return;

	_TCHAR path_shadow[_MAX_PATH] = {0};

	my_strncpy_s(path_shadow, _MAX_PATH, fname.toLocal8Bit().constData(), _TRUNCATE);
	if(!(FILEIO::IsFileExisting(path_shadow))) return;

	UPDATE_HISTORY(path_shadow, p_config->recent_cart_path[drv], listCARTs[drv]);
	my_strncpy_s(p_config->initial_cart_dir,
				 sizeof(p_config->initial_cart_dir) / sizeof(_TCHAR),
				 get_parent_dir((const _TCHAR *)path_shadow),
				 _TRUNCATE);
	// Update List
	my_strncpy_s(path_shadow,
				 _MAX_PATH,
				 fname.toLocal8Bit().constData(),
				 _TRUNCATE);
	if(menu_Cart[drv] != nullptr) {
		menu_Cart[drv]->do_set_initialize_directory(p_config->initial_cart_dir);
	}
	do_update_cartridge_history(drv, listCARTs[drv]);
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_Cart, drv, fname);
	}
}


void Ui_MainWindowBase::set_recent_cart(int drv, int num)
{
	QString s_path;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(p_config == nullptr) return;

	if((p->get_max_cart() <= drv) || !(p->is_use_cart()) || (drv < 0)) return;
	if((num < 0) || (num >= MAX_HISTORY)) return;

	s_path = QString::fromLocal8Bit(p_config->recent_cart_path[drv][num]);
	if(s_path.isEmpty()) return;
	do_open_cartridge_ui(drv, s_path);
	return;
}

void Ui_MainWindowBase::do_open_cartridge_ui(int drv, QString path)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if((p->get_max_cart() <= drv) || !(p->is_use_cart()) || (drv < 0)) return;
	if(path.length() <= 0) return;

	const _TCHAR *fnamep = (const _TCHAR*)(path.toLocal8Bit().constData());
	if(fnamep == nullptr) return;
	if(!(FILEIO::IsFileExisting(fnamep))) return; // File not found.

	emit sig_eject_cartridge_ui(drv);
	emit sig_open_cartridge(drv, path);

	return;
}
void Ui_MainWindowBase::do_eject_cartridge(int drv)
{
	emit sig_eject_cartridge_ui(drv);
}

void Ui_MainWindowBase::do_ui_eject_cartridge(int drv)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if((p->get_max_cart() <= drv) || !(p->is_use_cart()) || (drv < 0)) return;

	if(menu_Cart[drv] != nullptr) {
		menu_Cart[drv]->do_clear_inner_media();
	}
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_Cart, drv, QString::fromUtf8(""));
	}
}

void Ui_MainWindowBase::CreateCartMenu(int drv, int drv_base)
{
	QString ext;
	QString desc;

	QString drv_base_name = QString::number(drv_base);

	if(using_flags->is_machine_gamegear()) {
		ext = "*.rom *.bin *.gg *.col *.gz";
		desc = "Game Cartridge";
	} else if(using_flags->is_machine_mastersystem()) {
		ext = "*.rom *.bin *.sms *.gz";
		desc = "Game Cartridge";
	} else if(using_flags->is_machine_pc6001_variants()) {
		ext = "*.rom *.bin *.60 *.gz";
		desc = "Game Cartridge";
	} else if(using_flags->is_machine_has_pcengine()) {
		ext = "*.rom *.bin *.pce *.gz";
		desc = "HuCARD";
	} else if(using_flags->is_machine_z80tvgame()) {
		ext = "*.rom *.bin *.hex *.gz";
		desc = "GameData";
	} else if(using_flags->is_machine_sc3000()) {
		ext = "*.rom *.bin *.sms *.sg *.gz";
		desc = "SC-3000/1000 Game Cartridge";
	} else {
		ext = "*.rom *.bin *.gz";
		desc = "Game Cartridge";
	}

	menu_Cart[drv] = new Menu_CartClass(menubar, QString::fromUtf8("Cart"), using_flags, this, drv, drv_base);
	menu_Cart[drv]->create_pulldown_menu();

	menu_Cart[drv]->do_clear_inner_media();
	menu_Cart[drv]->do_add_media_extension(ext, desc);
	SETUP_HISTORY(p_config->recent_cart_path[drv], listCARTs[drv]);
	menu_Cart[drv]->do_update_histories(listCARTs[drv]);
	menu_Cart[drv]->do_set_initialize_directory(p_config->initial_cart_dir);

	QString name = QString::fromUtf8("Cart");
	QString tmpv;
	tmpv.setNum(drv_base);
	name.append(tmpv);
	menu_Cart[drv]->setTitle(name);
}

void Ui_MainWindowBase::CreateCartPulldownMenu(int drv)
{
}

void Ui_MainWindowBase::ConfigCartMenuSub(int drv)
{
}

void Ui_MainWindowBase::retranslateCartMenu(int drv, int basedrv)
{
	menu_Cart[drv]->retranslateUi();
}

void Ui_MainWindowBase::ConfigCartMenu(void)
{
	for(int i = 0; i > using_flags->get_max_cart(); i++) {
		ConfigCartMenuSub(0);
	}
}

void Ui_MainWindowBase::do_update_cartridge_history(int drive, QStringList lst)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((drive < 0) || (drive >= p->get_max_cart()) || !(p->is_use_cart())) return;
	if(menu_Cart[drive] != nullptr) {
		menu_Cart[drive]->do_update_histories(lst);
	}
}
