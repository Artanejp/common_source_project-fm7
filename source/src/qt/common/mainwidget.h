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
	bool LaunchEmuThread(std::shared_ptr<EmuThreadClassBase> m) override;
	// JoyThread
	void StopJoyThread(void) override;
	void LaunchJoyThread(std::shared_ptr<JoyThreadClass> m) override;
	// Screen
	void OnWindowMove(void) override;
	void OnWindowRedraw(void) override;
	void OnMainWindowClosed(void) override;

	QString GetBubbleB77BubbleName(int drv, int num);
	QString get_system_version() override;
	QString get_build_date() override;

public slots:
	void do_create_hard_disk(int drv, int sector_size, int sectors, int surfaces, int cylinders, QString name) override;
	void do_create_d88_media(int drv, quint8 media_type, QString name) override;
#if defined(USE_DEBUGGER)
	void OnOpenDebugger(void) override;
	void OnCloseDebugger(void) override;
#endif
	void on_actionExit_triggered() override;
	void do_release_emu_resources(void) override;
	void delete_joy_thread(void) override;
	void do_set_mouse_enable(bool flag) override;
	void do_toggle_mouse(void) override;
	void rise_movie_dialog(void) override;
signals:
	int sig_movie_set_width(int);
	int sig_movie_set_height(int);

	//virtual void redraw_status_bar(void);

};
QT_END_NAMESPACE

#endif
