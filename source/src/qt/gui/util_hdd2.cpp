#include "../gui/emu_thread_tmpl.h"
#include "mainwidget_base.h"
#include "commonclasses.h"
#include "menu_harddisk.h"
#include "dock_disks.h"

#include "qt_dialogs.h"
#include "emu.h"

#include "menu_flags.h"

//#if defined(USE_HARD_DISK)
int Ui_MainWindowBase::set_recent_hard_disk(int drv, int num)
{
	QString s_path;
	std::shared_ptr<USING_FLAGS>p = using_flags;

	if(drv < 0) return -1;
	if(p_config == nullptr) return -1;
	if(p.get() == nullptr) return -1;

	if(p->get_max_hdd() <= drv) return -1;
	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	s_path = QString::fromLocal8Bit(p_config->recent_hard_disk_path[drv][num]);

	if(!(s_path.isEmpty())) {
		_open_hard_disk(drv, (const QString)s_path);
		return 0;
	}
	return 0;
}

void Ui_MainWindowBase::do_ui_eject_hard_disk(int drv)
{
	if(menu_hdds[drv] != nullptr) {
		menu_hdds[drv]->do_clear_inner_media();
	}
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_HD, drv, QString::fromUtf8(""));
	}

}

void Ui_MainWindowBase::do_ui_hard_disk_insert_history(int drv, QString fname)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;

	if(p.get() == nullptr) return;
	if(p_config == nullptr) return;

	if(!(p->is_use_hdd())) return;
	if(p->get_max_hdd() <= drv) return;
	if(fname.length() <= 0) return;

	_TCHAR path_shadow[_MAX_PATH] = {0};

	my_strncpy_s(path_shadow, _MAX_PATH,
				fname.toLocal8Bit().constData(),
				_TRUNCATE);
	if(!(FILEIO::IsFileExisting(path_shadow))) return;

	UPDATE_HISTORY(path_shadow, p_config->recent_hard_disk_path[drv], listHDDs[drv]);

	memset(p_config->initial_hard_disk_dir, 0x00, sizeof(p_config->initial_hard_disk_dir));
	my_strncpy_s(p_config->initial_hard_disk_dir,
				sizeof(p_config->initial_hard_disk_dir) / sizeof(_TCHAR),
				 get_parent_dir((const _TCHAR *)path_shadow),
				 _TRUNCATE);

	if(menu_hdds[drv] != nullptr) {
		menu_hdds[drv]->do_set_initialize_directory(p_config->initial_hard_disk_dir);
		menu_hdds[drv]->do_update_histories(listHDDs[drv]);
	}
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_HD, drv, fname);
	}
}


void Ui_MainWindowBase::eject_hard_disk(int drv)
{
//	emit sig_close_hard_disk(drv);
}

void Ui_MainWindowBase::_open_hard_disk(int drv, const QString fname)
{
	if(drv < 0) return;

	std::shared_ptr<USING_FLAGS>p = using_flags;

	if(p_config == nullptr) return;
	if(p.get() == nullptr) return;
	if(p->get_max_hdd() <= drv) return;
	if(fname.length() <= 0) return;

//	emit sig_close_hard_disk(drv);
	emit sig_open_hard_disk(drv, fname);
}
//#endif
