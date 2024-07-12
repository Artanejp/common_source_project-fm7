#include "emu_thread_tmpl.h"
#include "mainwidget_base.h"
#include "menu_disk.h"
#include "qt_dialogs.h"
#include "dock_disks.h"

#include "menu_flags.h"

#include "../../fileio.h"

//#if defined(USE_FLOPPY_DISK)



//extern DLL_PREFIX_I EMU *emu;

void Ui_MainWindowBase::do_ui_write_protect_floppy_disk(int drive, quint64 flag)
{
	if(drive < 0) return;

	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_fd()) || (p->get_max_drive() <= drive)) return;
	if(menu_fds[drive] != nullptr) return;

	if((flag & EMU_MESSAGE_TYPE::WRITE_PROTECT) != 0) {
		menu_fds[drive]->do_set_write_protect(true);
	} else {
		menu_fds[drive]->do_set_write_protect(false);
	}
}
extern const _TCHAR* DLL_PREFIX_I get_parent_dir(const _TCHAR* file);

int Ui_MainWindowBase::set_recent_disk(int drv, int num)
{
	QString s_path;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return -1;
	if(p_config == nullptr) return -1;
	if(!(p->is_use_fd()) || (p->get_max_drive() <= drv) || (drv < 0)) return -1;

	if((num < 0) || (num >= MAX_HISTORY)) return -1;
	s_path = QString::fromLocal8Bit(p_config->recent_floppy_disk_path[drv][num]);
	if(!(s_path.isEmpty())) {
		_open_disk(drv, s_path);
		return 0;
	}
	return -1;
}

void Ui_MainWindowBase::do_ui_floppy_insert_history(int drv, QString fname, quint64 bank)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_fd()) || (p->get_max_drive() <= drv) || (drv < 0)) return;
	if(fname.length() <= 0) return;

	_TCHAR path_shadow[_MAX_PATH] = {0};

	my_strncpy_s(path_shadow, _MAX_PATH, fname.toLocal8Bit().constData(), _TRUNCATE);
	if(!(FILEIO::IsFileExisting(path_shadow))) return;

	if((bank & EMU_MEDIA_TYPE::MULTIPLE_SLOT_DETECT_MASK) == 0) {
		UPDATE_HISTORY(path_shadow, p_config->recent_floppy_disk_path[drv], listFDs[drv]);
		my_strncpy_s(p_config->initial_floppy_disk_dir,
					 sizeof(p_config->initial_floppy_disk_dir) / sizeof(_TCHAR),
					 get_parent_dir((const _TCHAR *)path_shadow),
					 _TRUNCATE);
	// Update List
		my_strncpy_s(path_shadow,
					_MAX_PATH,
					 fname.toLocal8Bit().constData(),
					 _TRUNCATE);
		menu_fds[drv]->do_set_initialize_directory(p_config->initial_floppy_disk_dir);
		do_update_floppy_history(drv, listFDs[drv]);
	}
	QString int_name = fname;
	if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d8e") ||
		check_file_extension(path_shadow, ".d77") || check_file_extension(path_shadow, ".1dd")) {
		return;
	}
	// ToDO: Replace signal model.
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_FD, drv, int_name);
	}
}

void Ui_MainWindowBase::_open_disk(int drv, const QString fname)
{
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_fd()) || (p->get_max_drive() <= drv) || (drv < 0)) return;
	if(fname.length() <= 0) return;

	const _TCHAR *fnamep = (const _TCHAR*)(fname.toLocal8Bit().constData());
	if(fnamep == nullptr) return;

	if(!(FILEIO::IsFileExisting(fnamep))) return; // File not found.

	//printf("%d %s %s\n", drv, fname.toUtf8().constData(), fnamep);
	//emit sig_close_floppy_disk_ui(drv);
	emit sig_open_floppy_disk(drv, fname, 0);
}

void Ui_MainWindowBase::do_clear_d88_list(int drv)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_fd()) || (p->get_max_drive() <= drv) || (drv < 0)) return;
	listD88[drv].clear();
	if(menu_fds[drv] != nullptr) {
		menu_fds[drv]->do_update_inner_media(listD88[drv], 0);
	}
}

void Ui_MainWindowBase::do_insert_d88_list(int drv, QString name, quint64 _slot)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_fd()) || (p->get_max_drive() <= drv) || (drv < 0)) return;
	if(_slot > 64) return;
	QString _s = name;
	if(_s.isEmpty()) {
		_s = QString::fromUtf8(" ");
	}
	listD88[drv] << _s;
}

void Ui_MainWindowBase::do_finish_d88_list(int drv, quint64 bank)
{
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_fd()) || (p->get_max_drive() <= drv) || (drv < 0)) return;
	if(menu_fds[drv] != nullptr) {
		menu_fds[drv]->do_update_inner_media(listD88[drv], bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK);
	}
	QString int_name = listD88[drv].at(bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK);
	if(driveData != nullptr) {
		driveData->updateMediaFileName(CSP_DockDisks_Domain_FD, drv, int_name);
	}
}
//#endif
