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

void Ui_MainWindowBase::do_ui_bubble_closed(int drive)
{
}

int Ui_MainWindowBase::set_recent_bubble(int drv, int num)
{
	QString s_path;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return -1;
	if(p_config == nullptr) return -1;

	if(p->get_max_bubble() <= drv) return -1;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	s_path = QString::fromLocal8Bit(p_config->recent_bubble_casette_path[drv][num]);
	if(!(s_path.isEmpty())) {
		_open_bubble(drv, s_path);
		return 0;
	}
	return -1;
}


void Ui_MainWindowBase::do_ui_bubble_insert_history(int drv, QString fname, quint64 bank)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(fname.length() <= 0) return;
	if(p->get_max_bubble() <= drv) return;
	if(p_config == nullptr) return;

	_TCHAR path_shadow[_MAX_PATH] = {0};

	my_strncpy_s(path_shadow, _MAX_PATH, fname.toLocal8Bit().constData(), _TRUNCATE);
	if(!(FILEIO::IsFileExisting(path_shadow))) return;

	if((bank & EMU_MEDIA_TYPE::MULTIPLE_SLOT_DETECT_MASK) == 0) {
		UPDATE_HISTORY(path_shadow, p_config->recent_bubble_casette_path[drv], listBubbles[drv]);
		my_strncpy_s(p_config->initial_floppy_bubble_casette_dir,
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
	if(menu_bubbless[drv] != nullptr) {
		menu_bubbless[drv]->do_clear_inner_media();
	}
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_Bubble, drv, QString::fromUtf8(""));
	}
}
void Ui_MainWindowBase::do_ui_bubble_write_protect(int drv, quint64 flag)
{
	if(drive < 0) return;

	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_bubble()) || (p->get_max_bubble() <= drive)) return;
	if(menu_bubbless[drive] != nullptr) return;

	if((flag & EMU_MESSAGE_TYPE::WRITE_PROTECT) != 0) {
		menu_bubbles[drive]->do_set_write_protect(true);
	} else {
		menu_bubbles[drive]->do_set_write_protect(false);
	}
}

void Ui_MainWindowBase::do_update_bubble_history(int drive, QStringList lst)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if((drive < 0) || (drive >= p->get_max_bubble())) return;
	if(menu_bubbless[drive] != nullptr) {
		menu_bubbless[drive]->do_update_histories(lst);
	}
}
