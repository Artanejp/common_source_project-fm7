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
class EmuThreadClassBase;

#ifndef _SCREEN_MODE_NUM
#define _SCREEN_MODE_NUM 32
#endif

class Ui_MainWindow : public Ui_MainWindowBase
{
	Q_OBJECT
protected:

public:
	Ui_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent = 0);
	~Ui_MainWindow();

	// Belows are able to re-implement.
	//virtual void retranslateUi(void);
	//void retranslateUI_Help(void);
	// EmuThread
	void StopEmuThread(void);
	void LaunchEmuThread(EmuThreadClassBase *m);
	// JoyThread
	void StopJoyThread(void);
	void LaunchJoyThread(void);
	// Screen
	void OnWindowMove(void);
	void OnWindowRedraw(void);
	void OnMainWindowClosed(void);

	QString GetBubbleB77BubbleName(int drv, int num);
	QString get_system_version();
	QString get_build_date();

public slots:
	void do_create_hard_disk(int drv, int sector_size, int sectors, int surfaces, int cylinders, QString name);
	void do_create_d88_media(int drv, quint8 media_type, QString name);
#if defined(USE_DEBUGGER)
	void OnOpenDebugger(void);
	void OnCloseDebugger(void);
#endif
	void on_actionExit_triggered();
	void do_release_emu_resources(void);
	void delete_joy_thread(void);
	void do_set_mouse_enable(bool flag);
	void do_toggle_mouse(void);
	void rise_movie_dialog(void);
signals:
	int sig_movie_set_width(int);
	int sig_movie_set_height(int);

	//virtual void redraw_status_bar(void);

};
QT_END_NAMESPACE

#endif
