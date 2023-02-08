#include "emu_thread_tmpl.h"
#include "mainwidget_base.h"
#include "commonclasses.h"
#include "menu_disk.h"
#include "qt_dialogs.h"

#include "menu_flags.h"

#include "../../fileio.h"

//#if defined(USE_FLOPPY_DISK)

#ifndef UPDATE_D88_LIST
#define UPDATE_D88_LIST(__d, lst) {						\
		QString __tmps;									\
		int ___bn = hRunEmu->get_d88_file_bank_num(__d);			\
		for(int iii = 0; iii < ((___bn < 64) ? ___bn : 64); iii++) {	\
			__tmps = hRunEmu->get_d88_file_disk_name(__d, iii);			\
			lst << __tmps;												\
		}																\
	}

#endif


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

	if(using_flags->get_max_drive() <= drv) return -1;
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
	if(fname.length() <= 0) return;
	if(using_flags->get_max_drive() <= drv) return;

	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, fname.toLocal8Bit().constData(), _MAX_PATH - 1);
	if(!(FILEIO::IsFileExisting(path_shadow))) return;

	if((bank & EMU_MEDIA_TYPE::MULTIPLE_SLOT_DETECT_MASK) == 0) {
		UPDATE_HISTORY(path_shadow, p_config->recent_floppy_disk_path[drv], listFDs[drv]);
		strcpy(p_config->initial_floppy_disk_dir, 	get_parent_dir((const _TCHAR *)path_shadow));
	// Update List
		strncpy(path_shadow, fname.toLocal8Bit().constData(), _MAX_PATH - 1);
		menu_fds[drv]->do_set_initialize_directory(p_config->initial_floppy_disk_dir);
	}
	do_update_floppy_history(drv, listFDs[drv]);

	listD88[drv].clear();
	if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
		UPDATE_D88_LIST(drv, listD88[drv]);
	}
	menu_fds[drv]->do_update_inner_media(listD88[drv], bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK);

}

void Ui_MainWindowBase::_open_disk(int drv, const QString fname)
{
	char path_shadow[PATH_MAX];

	if(fname.length() <= 0) return;
	if(using_flags->get_max_drive() <= drv) return;
	if(!(FILEIO::IsFileExisting(fname.toLocal8Bit().constData()))) return; // File not found.
//	drv = drv & 7;
	strncpy(path_shadow, fname.toLocal8Bit().constData(), PATH_MAX - 1);
	emit sig_close_floppy_disk(drv);
	emit sig_open_floppy_disk(drv, fname, 0);

	if(using_flags->get_max_drive() > (drv + 1)) {
		if(check_file_extension(path_shadow, ".d88") || check_file_extension(path_shadow, ".d77")) {
			if(((drv & 1) == 0) && (drv + 1 < using_flags->get_max_drive()) && (1 < hRunEmu->get_d88_file_bank_num(drv))) {
				int drv2 = drv + 1;
				emit sig_close_floppy_disk(drv2);
				emit sig_open_floppy_disk(drv2, fname, 1 | EMU_MEDIA_TYPE::MULTIPLE_SLOT_DETECT_MASK);
			}
		}
	}
}


void Ui_MainWindowBase::do_update_d88_list(int drv, int bank)
{
//	UPDATE_D88_LIST(drv, listD88[drv]);
	menu_fds[drv]->do_update_inner_media(listD88[drv], bank & EMU_MEDIA_TYPE::EMU_SLOT_MASK);
}
//#endif
