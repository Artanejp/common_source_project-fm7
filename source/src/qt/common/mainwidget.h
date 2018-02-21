#ifndef _CSP_QT_MAINWIDGET_H
#define _CSP_QT_MAINWIDGET_H

#include "mainwidget_base.h"
#include "simd_types.h"
#include "common.h"
#include "config.h"
#include "emu.h"
#include "vm.h"

#include "qt_main.h"

QT_BEGIN_NAMESPACE

class USING_FLAGS;

#ifndef _SCREEN_MODE_NUM
#define _SCREEN_MODE_NUM 32
#endif
//extern 	USING_FLAGS *using_flags;

class Ui_MainWindow : public Ui_MainWindowBase
{
	Q_OBJECT
protected:
	
public:
	Ui_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~Ui_MainWindow();

	void set_window(int mode);
	// Belows are able to re-implement.
	//virtual void retranslateUi(void);
	//void retranslateUI_Help(void);
	// EmuThread
	void StopEmuThread(void);
	void LaunchEmuThread(void);
	// JoyThread
	void StopJoyThread(void);
	void LaunchJoyThread(void);
	// Screen
	void OnWindowMove(void);
	void OnWindowRedraw(void);
	void OnMainWindowClosed(void);
#if defined(USE_NOTIFY_POWER_OFF)
	bool GetPowerState(void);
#endif	
	int GetBubbleBankNum(int drv);
	int GetBubbleCurrentBankNum(int drv);
	bool GetBubbleCasetteIsProtected(int drv);
	QString GetBubbleB77FileName(int drv);
	QString GetBubbleB77BubbleName(int drv, int num);

public slots:
#if defined(USE_FD1)
	void _open_disk(int drv, const QString fname);
	void do_update_recent_disk(int);
	int set_d88_slot(int drive, int num);
	int set_recent_disk(int, int);
#endif
#if defined(USE_DEBUGGER)
	void OnOpenDebugger(int n);
	void OnCloseDebugger(void);
#endif
	void on_actionExit_triggered();
	void do_release_emu_resources(void);
	void delete_joy_thread(void);
	void do_set_mouse_enable(bool flag);
	void do_toggle_mouse(void);
	void rise_movie_dialog(void);
	void do_update_inner_fd(int drv, QStringList base, class Action_Control **action_select_media_list,
							QStringList lst, int num, bool use_d88_menus);
	void do_update_inner_bubble(int drv, QStringList base, class Action_Control **action_select_media_list,
							QStringList lst, int num, bool use_d88_menus);

signals:
	int sig_movie_set_width(int);
	int sig_movie_set_height(int);

	//virtual void redraw_status_bar(void);
	
};
QT_END_NAMESPACE

#endif


