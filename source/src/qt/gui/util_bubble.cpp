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
#include "dock_disks.h"

#include "osdcall_types.h"
#include "emu_thread_tmpl.h"


// Common Routine
#ifndef UPDATE_B77_LIST
#define UPDATE_B77_LIST(__d, lst) { \
		QString __tmps;									\
		lst.clear();												\
		int ___bn = hRunEmu->get_b77_file_bank_num(__d);			\
		for(int iii = 0; iii < ((___bn < 16) ? ___bn : 16); iii++) {	\
			__tmps = hRunEmu->get_b77_file_media_name(__d, iii);		\
			lst << __tmps;												\
		}																\
	}

#endif

void Ui_MainWindowBase::CreateBubbleMenu(int drv, int drv_base)
{
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if(up.get() == nullptr) return;
	if(p_config == nullptr) return;
	if(!(up->is_use_bubble())) return;
	if((up->get_max_bubble() <= drv) || (drv < 0)) return;
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
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if(up.get() == nullptr) return;
	if(p_config == nullptr) return;
	if(!(up->is_use_bubble())) return;
	if((up->get_max_bubble() <= drv) || (drv < 0)) return;
	if(menu_bubbles[drv] == nullptr) return;

	QString drive_name = (QApplication::translate("MainWindow", "Bubble ", 0));
	drive_name += QString::number(basedrv);

	menu_bubbles[drv]->setTitle(QApplication::translate("MainWindow", drive_name.toUtf8().constData() , 0));
	menu_bubbles[drv]->retranslateUi();
}

void Ui_MainWindowBase::ConfigBubbleMenu(void)
{
	std::shared_ptr<USING_FLAGS> up = using_flags;
	if(up.get() == nullptr) return;

	for(int i = 0; i < up->get_max_bubble(); i++) {
		ConfigBubbleMenuSub(i);
	}
}

int Ui_MainWindowBase::set_recent_bubble(int drv, int num)
{
	QString s_path;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return -1;
	if(p_config == nullptr) return -1;
	if(!(p->is_use_bubble())) return -1;
	if((p->get_max_bubble() <= drv) || (drv < 0)) return -1;

	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	s_path = QString::fromLocal8Bit(p_config->recent_bubble_casette_path[drv][num]);

	if(!(s_path.isEmpty())) {
		_open_bubble(drv, s_path);
		return 0;
	}
	return -1;
}

void Ui_MainWindowBase::_open_bubble(int drv, const QString fname)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(fname.length() <= 0) return;
	if(!(p->is_use_bubble())) return;
	if((p->get_max_bubble() <= drv) || (drv < 0)) return;

	const _TCHAR *fnamep = (const _TCHAR*)(fname.toLocal8Bit().constData());
	if(fnamep == nullptr) return;

	if(!(FILEIO::IsFileExisting(fnamep))) return; // File not found.

	emit sig_close_bubble_ui(drv);
	emit sig_open_bubble(drv, fname, 0);

}

void Ui_MainWindowBase::do_ui_bubble_insert_history(int drv, QString fname, quint64 bank)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(fname.length() <= 0) return;
	if(p_config == nullptr) return;
	if(!(p->is_use_bubble()) || (p->get_max_bubble() <= drv) || (drv < 0)) return;

	_TCHAR path_shadow[_MAX_PATH] = {0};

	my_strncpy_s(path_shadow, _MAX_PATH, fname.toLocal8Bit().constData(), _TRUNCATE);
	if(!(FILEIO::IsFileExisting(path_shadow))) return;

	if((bank & EMU_MEDIA_TYPE::MULTIPLE_SLOT_DETECT_MASK) == 0) {
		UPDATE_HISTORY(path_shadow, p_config->recent_bubble_casette_path[drv], listBubbles[drv]);
		my_strncpy_s(p_config->initial_bubble_casette_dir,
					 sizeof(p_config->initial_bubble_casette_dir) / sizeof(_TCHAR),
					 get_parent_dir((const _TCHAR *)path_shadow),
					 _TRUNCATE);
	// Update List
		my_strncpy_s(path_shadow,
					_MAX_PATH,
					 fname.toLocal8Bit().constData(),
					 _TRUNCATE);
		menu_bubbles[drv]->do_set_initialize_directory(p_config->initial_bubble_casette_dir);
		do_update_bubble_history(drv, listBubbles[drv]);
	}
	listB77[drv].clear();
	QString int_name = fname;
	if(check_file_extension(path_shadow, ".b77")) {
		UPDATE_B77_LIST(drv, listB77[drv]);
		unsigned int xbank = bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK;
		if(listB77[drv].size() > xbank) {
			int_name = listB77[drv].at(xbank);
		}
	}
	if(menu_bubbles[drv] != nullptr) {
		menu_bubbles[drv]->do_update_inner_media(listB77[drv], bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK);
	}
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_Bubble, drv, int_name);
	}

}
void Ui_MainWindowBase::do_ui_eject_bubble_casette(int drv)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_bubble()) || (p->get_max_bubble() <= drv) || (drv < 0)) return;

	if(menu_bubbles[drv] != nullptr) {
		menu_bubbles[drv]->do_clear_inner_media();
	}
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_Bubble, drv, QString::fromUtf8(""));
	}
}
void Ui_MainWindowBase::do_ui_bubble_write_protect(int drv, quint64 flag)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_bubble()) || (p->get_max_bubble() <= drv) || (drv < 0)) return;
	if(menu_bubbles[drv] == nullptr) return;

	if((flag & EMU_MESSAGE_TYPE::WRITE_PROTECT) != 0) {
		menu_bubbles[drv]->do_set_write_protect(true);
	} else {
		menu_bubbles[drv]->do_set_write_protect(false);
	}
}

void Ui_MainWindowBase::do_update_bubble_history(int drive, QStringList lst)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_bubble()) || (p->get_max_bubble() <= drive) || (drive < 0)) return;

	if(menu_bubbles[drive] != nullptr) {
		menu_bubbles[drive]->do_update_histories(lst);
	}
}
void Ui_MainWindowBase::do_update_b77_list(int drv, int bank)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_bubble()) || (p->get_max_bubble() <= drv) || (drv < 0)) return;

	UPDATE_B77_LIST(drv, listB77[drv]);
	if(menu_bubbles[drv] != nullptr) {
		menu_bubbles[drv]->do_update_inner_media(listD88[drv], bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK);
	}
}
