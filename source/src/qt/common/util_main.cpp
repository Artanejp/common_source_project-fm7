
#include <QtCore/QVariant>
#include <QtGui>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QPixmap>

#include "commonclasses.h"
#include "display_about.h"
#include "display_text_document.h"
#include "mainwidget.h"
//#include "menuclasses.h"
#include "menu_disk.h"
#include "menu_cmt.h"
#include "menu_cart.h"
#include "menu_quickdisk.h"
#include "menu_binary.h"
#include "menu_compactdisc.h"
#include "menu_bubble.h"

#include "qt_gldraw.h"
#include "emu.h"
#include "qt_main.h"
#include "menu_flags.h"
#include "../gui/csp_logger.h"

extern CSP_Logger *csp_logger;
extern EMU* emu;

Ui_MainWindow::Ui_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindowBase(p, logger, parent)
{
}

Ui_MainWindow::~Ui_MainWindow()
{
}

// screen
extern unsigned int desktop_width;
extern unsigned int desktop_height;
//int desktop_bpp;
extern int prev_window_mode;
extern bool now_fullscreen;

void Ui_MainWindow::set_window(int mode)
{
	//	static LONG style = WS_VISIBLE;

	if(mode >= 0 && mode < using_flags->get_screen_mode_num()) {
		if(mode >= screen_mode_count) return;
		// window
		int width = emu->get_window_mode_width(mode);
		int height = emu->get_window_mode_height(mode);
		
		this->resize(width + 10, height + 100); // OK?
		int dest_x = 0;
		int dest_y = 0;
		dest_x = (dest_x < 0) ? 0 : dest_x;
		dest_y = (dest_y < 0) ? 0 : dest_y;
		
		config.window_mode = prev_window_mode = mode;
		
		// set screen size to emu class
		emit sig_emu_set_display_size(width, height, width, height);
		emit sig_resize_osd(width);
		this->resize_statusbar(width, height);
	} else if(!now_fullscreen) {
		// fullscreen
		if(mode >= screen_mode_count) return;
		int width;
		int height;
		if(mode < 0) {
			width = desktop_width;
			height = desktop_height;
		} else {
			double nd = actionScreenSize[mode]->binds->getDoubleValue();
			width = (int)(nd * (double)using_flags->get_screen_width());
			height = (int)(nd * (double)using_flags->get_screen_height());

			if((config.rotate_type == 1) || (config.rotate_type == 3)){
					int tmp_w = width;
					width = height;
					height = tmp_w;
			}
		}
		config.window_mode = mode;
		emit sig_emu_set_display_size(using_flags->get_screen_width(), using_flags->get_screen_height(), width, height);
		emit sig_resize_osd(width);
		this->resize_statusbar(width, height);
	}
}
