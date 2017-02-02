/*
 * Common Source code Project:
 * Ui->Qt->gui->menu_main for generic.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, moved from qt/x1turboz/MainWindow.cpp .
 */
#include <QMenu>
#include <QMenuBar>

#include "commonclasses.h"
#include "mainwidget_base.h"
//#include "menuclasses.h"
#include "qt_main.h"
#include "qt_gldraw.h"
#include "menu_flags.h"

//extern USING_FLAGS *using_flags;
// WIP: Move another header.
#if (SCREEN_WIDTH > 320)
const static float screen_multiply_table[] = {0.5, 1.0, 1.5, 2.0, 2.25, 2.5, 3.0, 3.5, 4.0, 5.0, 6.0, 8.0, 0.0};
#else
const static float screen_multiply_table[] = {0.5, 1.0, 1.5, 2.0, 3.0, 4.0, 5.0, 6.0, 7.5, 10.0, 12.0, 15.0, 0.0};
#endif
void Object_Menu_Control::set_screen_aspect(void) {
	int num = getValue1();
	emit sig_screen_aspect(num);
}

void Object_Menu_Control::set_screen_size(void) {
	int w, h;
	double nd, ww, hh;
	using_flags->get_config_ptr()->window_mode = getNumber();
	nd = getDoubleValue();
	ww = nd * (double)using_flags->get_screen_width();
	hh = nd * (double)using_flags->get_screen_height();
	if((using_flags->get_screen_height_aspect() != using_flags->get_screen_height()) ||
	   (using_flags->get_screen_width_aspect() != using_flags->get_screen_width())) {
		double par_w = (double)using_flags->get_screen_width_aspect() / (double)using_flags->get_screen_width();
		double par_h = (double)using_flags->get_screen_height_aspect() / (double)using_flags->get_screen_height();
		//double par = par_h / par_w;
		if(using_flags->get_config_ptr()->window_stretch_type == 1) { // refer to X, scale Y.
			hh = hh * par_h;
		} else if(using_flags->get_config_ptr()->window_stretch_type == 2) { // refer to Y, scale X only
			ww = ww / par_h;
		} else if(using_flags->get_config_ptr()->window_stretch_type == 3) { // Scale both X, Y
			ww = ww * par_w;
			hh = hh * par_h;
		}
	}
	w = (int)ww;
	h = (int)hh;
	emit sig_screen_size(w, h);
	emit sig_screen_multiply(nd);
}

void Object_Menu_Control::do_save_as_movie(void)
{
	int fps = using_flags->get_config_ptr()->video_frame_rate;
	emit sig_start_record_movie(fps);
}

void Object_Menu_Control::do_stop_saving_movie(void)
{
	emit sig_stop_record_movie();
}


void Ui_MainWindowBase::do_stop_saving_movie(void)
{
	emit sig_stop_saving_movie();
}

void Ui_MainWindowBase::do_start_saving_movie(void)
{
	emit sig_start_saving_movie();
}

void Ui_MainWindowBase::do_set_render_mode_std(void)
{
	using_flags->get_config_ptr()->rendering_type = CONFIG_RENDER_TYPE_STD;
}

void Ui_MainWindowBase::do_set_render_mode_tv(void)
{
	using_flags->get_config_ptr()->rendering_type = CONFIG_RENDER_TYPE_TV;
}

void Ui_MainWindowBase::do_set_state_saving_movie(bool state)
{
	actionStop_Record_Movie->setVisible(state);
	actionStart_Record_Movie->setVisible(!state);
}	

void Ui_MainWindowBase::set_gl_scan_line_vert(bool f)
{
	using_flags->get_config_ptr()->opengl_scanline_vert = f;
}

void Ui_MainWindowBase::set_gl_scan_line_horiz(bool f)
{
	using_flags->get_config_ptr()->opengl_scanline_horiz = f;
}

void Ui_MainWindowBase::ConfigScreenMenu_List(void)
{
	int w, h;
	QString tmps;
	int i;
	
	actionGroup_ScreenSize = new QActionGroup(this);
	actionGroup_ScreenSize->setExclusive(true);
	screen_mode_count = 0;
	for(i = 0; i < using_flags->get_screen_mode_num(); i++) {
		w = (int)(screen_multiply_table[i] * (double)using_flags->get_screen_width());
		h = (int)(screen_multiply_table[i] * (double)using_flags->get_screen_height());
		if((w <= 0) || (h <= 0)) {
			break;
		}
		screen_mode_count++;
		tmps = QString::number(i);
		actionScreenSize[i] = new Action_Control(this, using_flags);
		actionScreenSize[i]->setObjectName(QString::fromUtf8("actionScreenSize", -1) + tmps);
		actionScreenSize[i]->setCheckable(true);
		actionScreenSize[i]->binds->setNumber(i);

		if(i == using_flags->get_config_ptr()->window_mode)  actionScreenSize[i]->setChecked(true);  // OK?

		actionGroup_ScreenSize->addAction(actionScreenSize[i]);
		actionScreenSize[i]->binds->setDoubleValue(screen_multiply_table[i]);
		
		connect(actionScreenSize[i], SIGNAL(triggered()),
			actionScreenSize[i]->binds, SLOT(set_screen_size()));

		connect(actionScreenSize[i]->binds, SIGNAL(sig_screen_size(int, int)),
			this, SLOT(set_screen_size(int, int)));
	}
	for(; i < using_flags->get_screen_mode_num(); i++) {
		actionScreenSize[i] = NULL;
	}
}
void Ui_MainWindowBase::ConfigScreenMenu(void)
{
	actionZoom = new Action_Control(this, using_flags);
	actionZoom->setObjectName(QString::fromUtf8("actionZoom"));
	actionDisplay_Mode = new Action_Control(this, using_flags);
	actionDisplay_Mode->setObjectName(QString::fromUtf8("actionDisplay_Mode"));
	
	if(using_flags->is_use_scanline()) {
		actionScanLine = new Action_Control(this, using_flags);
		actionScanLine->setObjectName(QString::fromUtf8("actionScanLine"));
		actionScanLine->setCheckable(true);
		if(using_flags->get_config_ptr()->scan_line != 0) {
			actionScanLine->setChecked(true);
		} else {
			actionScanLine->setChecked(false);
		}
		connect(actionScanLine, SIGNAL(toggled(bool)),
			this, SLOT(set_scan_line(bool)));
	}
	if(!using_flags->is_use_one_board_computer() && (using_flags->get_max_button() <= 0)) {
		actionGLScanLineHoriz = new Action_Control(this, using_flags);
		actionGLScanLineHoriz->setObjectName(QString::fromUtf8("actionGLScanLineHoriz"));
		actionGLScanLineHoriz->setCheckable(true);
		if(using_flags->get_config_ptr()->opengl_scanline_horiz != 0) {
			actionGLScanLineHoriz->setChecked(true);
		} else {
			actionGLScanLineHoriz->setChecked(false);
		}
		connect(actionGLScanLineHoriz, SIGNAL(toggled(bool)),
				this, SLOT(set_gl_scan_line_horiz(bool)));
		if(using_flags->is_use_vertical_pixel_lines()) {
			actionGLScanLineVert = new Action_Control(this, using_flags);
			actionGLScanLineVert->setObjectName(QString::fromUtf8("actionGLScanLineVert"));
			actionGLScanLineVert->setCheckable(true);
			if(using_flags->get_config_ptr()->opengl_scanline_vert != 0) {
				actionGLScanLineVert->setChecked(true);
			} else {
				actionGLScanLineVert->setChecked(false);
			}
			connect(actionGLScanLineVert, SIGNAL(toggled(bool)),
					this, SLOT(set_gl_scan_line_vert(bool)));
		}
	}
	if(using_flags->is_use_screen_rotate()) {
		actionRotate = new Action_Control(this, using_flags);
		actionRotate->setObjectName(QString::fromUtf8("actionScanLine"));
		actionRotate->setCheckable(true);
		if(using_flags->get_config_ptr()->rotate_type) {
			actionRotate->setChecked(true);
		} else {
			actionRotate->setChecked(false);
		}
		connect(actionRotate, SIGNAL(toggled(bool)),
				this, SLOT(set_screen_rotate(bool)));
	}
	if(using_flags->is_use_crt_filter()) {
		actionCRT_Filter = new Action_Control(this, using_flags);
		actionCRT_Filter->setObjectName(QString::fromUtf8("actionCRT_Filter"));
		actionCRT_Filter->setEnabled(true);
		actionCRT_Filter->setCheckable(true);
		if(using_flags->get_config_ptr()->crt_filter == 0) actionCRT_Filter->setChecked(true);
		connect(actionCRT_Filter, SIGNAL(toggled(bool)), this, SLOT(set_crt_filter(bool)));
	}
	actionOpenGL_Filter = new Action_Control(this, using_flags);
	actionOpenGL_Filter->setObjectName(QString::fromUtf8("actionOpenGL_Filter"));
	actionOpenGL_Filter->setEnabled(true);
	actionOpenGL_Filter->setCheckable(true);
	if(using_flags->get_config_ptr()->use_opengl_filters) actionOpenGL_Filter->setChecked(true);
	connect(actionOpenGL_Filter, SIGNAL(toggled(bool)), this, SLOT(set_gl_crt_filter(bool)));

	if((using_flags->get_screen_height_aspect() != using_flags->get_screen_height()) ||
	   (using_flags->get_screen_width_aspect() != using_flags->get_screen_width())) {
		actionDot_by_Dot = new Action_Control(this, using_flags);
		actionDot_by_Dot->setObjectName(QString::fromUtf8("actionDot_by_Dot"));
		actionDot_by_Dot->setCheckable(true);
		if(using_flags->get_config_ptr()->window_stretch_type == 0) actionDot_by_Dot->setChecked(true);
		actionDot_by_Dot->binds->setValue1(0);
		
		actionReferToX_Display = new Action_Control(this, using_flags);
		actionReferToX_Display->setObjectName(QString::fromUtf8("actionReferToX_Display"));
		actionReferToX_Display->setCheckable(true);
		actionReferToX_Display->binds->setValue1(1);
		if(using_flags->get_config_ptr()->window_stretch_type == 1) actionReferToX_Display->setChecked(true);
		
		actionReferToY_Display = new Action_Control(this, using_flags);
		actionReferToY_Display->setObjectName(QString::fromUtf8("actionReferToY_Display"));
		actionReferToY_Display->setCheckable(true);
		actionReferToY_Display->binds->setValue1(2);
		if(using_flags->get_config_ptr()->window_stretch_type == 2) actionReferToY_Display->setChecked(true);
	
		actionFill_Display = new Action_Control(this, using_flags);
		actionFill_Display->setObjectName(QString::fromUtf8("actionFill_Display"));
		actionFill_Display->setCheckable(true);
		actionFill_Display->binds->setValue1(3);
		if(using_flags->get_config_ptr()->window_stretch_type == 3) actionFill_Display->setChecked(true);
	
		actionGroup_Stretch = new QActionGroup(this);
		actionGroup_Stretch->setExclusive(true);
		actionGroup_Stretch->addAction(actionDot_by_Dot);
		actionGroup_Stretch->addAction(actionReferToX_Display);
		actionGroup_Stretch->addAction(actionReferToY_Display);
		actionGroup_Stretch->addAction(actionFill_Display);
		connect(actionDot_by_Dot,   SIGNAL(triggered()), actionDot_by_Dot->binds,   SLOT(set_screen_aspect()));
		connect(actionDot_by_Dot->binds,   SIGNAL(sig_screen_aspect(int)), this,    SLOT(set_screen_aspect(int)));
		
		connect(actionReferToX_Display,  SIGNAL(triggered()), actionReferToX_Display->binds,  SLOT(set_screen_aspect()));
		connect(actionReferToX_Display->binds,  SIGNAL(sig_screen_aspect(int)), this,    SLOT(set_screen_aspect(int)));
		
		connect(actionReferToY_Display,  SIGNAL(triggered()), actionReferToY_Display->binds,  SLOT(set_screen_aspect()));
		connect(actionReferToY_Display->binds,  SIGNAL(sig_screen_aspect(int)), this,    SLOT(set_screen_aspect(int)));
		
		connect(actionFill_Display, SIGNAL(triggered()), actionFill_Display->binds, SLOT(set_screen_aspect()));
		connect(actionFill_Display->binds, SIGNAL(sig_screen_aspect(int)), this, SLOT(set_screen_aspect(int)));
	}
	actionCapture_Screen = new Action_Control(this, using_flags);
	actionCapture_Screen->setObjectName(QString::fromUtf8("actionCapture_Screen"));

	actionStart_Record_Movie = new Action_Control(this, using_flags);
	actionStart_Record_Movie->setObjectName(QString::fromUtf8("actionStart_Record_Movie"));
	actionStart_Record_Movie->setCheckable(false);
	
	actionStop_Record_Movie = new Action_Control(this, using_flags);
	actionStop_Record_Movie->setObjectName(QString::fromUtf8("actionStop_Record_Movie"));
	actionStop_Record_Movie->setCheckable(false);

	bool b_support_tv_render = using_flags->is_support_tv_render();
	if(b_support_tv_render) {
		int ii = CONFIG_RENDER_TYPE_END;
		int i;
		
		if((ii >= 8) || (ii < 0)) ii = 8;
		actionGroup_RenderMode = new QActionGroup(this);
		actionGroup_RenderMode->setExclusive(true);
		
		for(i = 0; i < ii; i++) {
			action_SetRenderMode[i] = new Action_Control(this, using_flags);
			action_SetRenderMode[i]->setCheckable(true);
			action_SetRenderMode[i]->setEnabled(false);
			action_SetRenderMode[i]->setVisible(false);
			action_SetRenderMode[i]->binds->setValue1(i);
			
			if(i == using_flags->get_config_ptr()->rendering_type) action_SetRenderMode[i]->setChecked(true);
		
			if(i == CONFIG_RENDER_TYPE_STD) {
				action_SetRenderMode[i]->setEnabled(true);
				action_SetRenderMode[i]->setVisible(true);
				actionGroup_RenderMode->addAction(action_SetRenderMode[i]);
				connect(action_SetRenderMode[i], SIGNAL(triggered()), this, SLOT(do_set_render_mode_std()));
			}
			if(b_support_tv_render && (i == CONFIG_RENDER_TYPE_TV)) {
				action_SetRenderMode[i]->setEnabled(true);
				action_SetRenderMode[i]->setVisible(true);
				actionGroup_RenderMode->addAction(action_SetRenderMode[i]);
				connect(action_SetRenderMode[i], SIGNAL(triggered()), this, SLOT(do_set_render_mode_tv()));
			}
		}
	}				
	ConfigScreenMenu_List();  
}

void Ui_MainWindowBase::CreateScreenMenu(void)
{
	int i;
	menuScreen = new QMenu(menubar);
	menuScreen->setObjectName(QString::fromUtf8("menuScreen"));
	if((using_flags->get_screen_height_aspect() != using_flags->get_screen_height()) ||
	   (using_flags->get_screen_width_aspect() != using_flags->get_screen_width())) {
		menuStretch_Mode = new QMenu(menuScreen);
		menuStretch_Mode->setObjectName(QString::fromUtf8("menuStretch_Mode"));
	}
	bool b_support_tv_render = using_flags->is_support_tv_render();
	if(b_support_tv_render) {
		menuScreen_Render = new QMenu(menuScreen);
		menuScreen_Render->setObjectName(QString::fromUtf8("menuRender_Mode"));
		menuScreen_Render->addAction(action_SetRenderMode[CONFIG_RENDER_TYPE_STD]);
		if(b_support_tv_render) {
			menuScreen_Render->addAction(action_SetRenderMode[CONFIG_RENDER_TYPE_TV]);
		}
		menuScreen->addAction(menuScreen_Render->menuAction());
	}
	menuScreenSize = new QMenu(menuScreen);
	menuScreenSize->setObjectName(QString::fromUtf8("menuScreen_Size"));
	menuScreen->addSeparator();
	menuRecord_as_movie = new QMenu(menuScreen);
	menuRecord_as_movie->setObjectName(QString::fromUtf8("menuRecord_as_movie"));

	menuScreen->addAction(actionZoom);
	menuScreen->addAction(menuScreenSize->menuAction());
	for(i = 0; i < using_flags->get_screen_mode_num(); i++) {
		if(actionScreenSize[i] == NULL) continue;
		menuScreenSize->addAction(actionScreenSize[i]);
		actionScreenSize[i]->setVisible(true);
	}

	if((using_flags->get_screen_height_aspect() != using_flags->get_screen_height()) ||
	   (using_flags->get_screen_width_aspect() != using_flags->get_screen_width())) {
		menuScreen->addSeparator();
		menuScreen->addAction(menuStretch_Mode->menuAction());

		menuStretch_Mode->addAction(actionDot_by_Dot);
		menuStretch_Mode->addAction(actionReferToX_Display);
		menuStretch_Mode->addAction(actionReferToY_Display);
		menuStretch_Mode->addAction(actionFill_Display);
	}
	menuScreen->addSeparator();

	if(using_flags->is_use_scanline()) {
		menuScreen->addAction(actionScanLine);
	}

	if(!using_flags->is_use_one_board_computer() && (using_flags->get_max_button() <= 0)) {
		menuScreen->addAction(actionGLScanLineHoriz);
		if(using_flags->is_use_vertical_pixel_lines()) {
			menuScreen->addAction(actionGLScanLineVert);
		}
	}
	if(using_flags->is_use_screen_rotate()) {
		menuScreen->addAction(actionRotate);
	}
	if(using_flags->is_use_crt_filter()) {
		menuScreen->addAction(actionCRT_Filter);
	}

	menuScreen->addAction(actionOpenGL_Filter);
	menuScreen->addAction(actionCapture_Screen);
	menuScreen->addSeparator();
	menuScreen->addAction(menuRecord_as_movie->menuAction());
	menuRecord_as_movie->addAction(actionStart_Record_Movie);
	menuRecord_as_movie->addAction(actionStop_Record_Movie);
}

void Ui_MainWindowBase::retranslateScreenMenu(void)
{
	int i;
	QString tmps;
	actionZoom->setText(QApplication::translate("MainWindow", "Zoom Screen", 0));
	actionDisplay_Mode->setText(QApplication::translate("MainWindow", "Display Mode", 0));

	if(using_flags->is_use_scanline()) {
		actionScanLine->setText(QApplication::translate("MainWindow", "Software Scan Line", 0));
		actionScanLine->setToolTip(QApplication::translate("MainWindow", "Display scan line by software.", 0));
	}
	if(using_flags->is_use_screen_rotate()) {
		actionRotate->setText(QApplication::translate("MainWindow", "Rotate Screen", 0));
		actionRotate->setToolTip(QApplication::translate("MainWindow", "Rotate screen.", 0));
	}
	if(using_flags->is_use_crt_filter()) {
		actionCRT_Filter->setText(QApplication::translate("MainWindow", "Software Filter", 0));
		actionCRT_Filter->setToolTip(QApplication::translate("MainWindow", "Use display filter by software.", 0));
	}
	if(!using_flags->is_use_one_board_computer() && (using_flags->get_max_button() <= 0)) {
		actionGLScanLineHoriz->setText(QApplication::translate("MainWindow", "OpenGL Scan Line", 0));
		actionGLScanLineHoriz->setToolTip(QApplication::translate("MainWindow", "Display scan line by OpenGL.", 0));
		if(using_flags->is_use_vertical_pixel_lines()) {
			actionGLScanLineVert->setText(QApplication::translate("MainWindow", "OpenGL Pixel Line", 0));
			actionGLScanLineVert->setToolTip(QApplication::translate("MainWindow", "Display pixel line by OpenGL.", 0));
		}
	}
	
	actionOpenGL_Filter->setText(QApplication::translate("MainWindow", "OpenGL Filter", 0));
	actionOpenGL_Filter->setToolTip(QApplication::translate("MainWindow", "Use display filter by OpenGL", 0));

	if((using_flags->get_screen_height_aspect() != using_flags->get_screen_height()) ||
	   (using_flags->get_screen_width_aspect() != using_flags->get_screen_width())) {
		actionDot_by_Dot->setText(QApplication::translate("MainWindow", "Dot by Dot", 0));
		actionReferToX_Display->setText(QApplication::translate("MainWindow", "Keep Aspect: Refer to X", 0));
		actionReferToY_Display->setText(QApplication::translate("MainWindow", "Keep Aspect: Refer to Y", 0));
		actionFill_Display->setText(QApplication::translate("MainWindow", "Keep Aspect: Fill", 0));
		menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0));
	}

	actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0));
	actionCapture_Screen->setToolTip(QApplication::translate("MainWindow", "Capture screen to a PNG file.", 0));

	menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0));
	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Recording Movie", 0));
	actionStart_Record_Movie->setToolTip(QApplication::translate("MainWindow", "Start Recording Movie", 0));
	actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Recording Movie", 0));
	actionStop_Record_Movie->setToolTip(QApplication::translate("MainWindow", "Stop Recording Movie", 0));
	menuScreen->setToolTipsVisible(true);
	
	menuRecord_as_movie->setTitle(QApplication::translate("MainWindow", "Record as Movie", 0));
	menuRecord_as_movie->setToolTipsVisible(true);

	menuScreenSize->setTitle(QApplication::translate("MainWindow", "Screen Size", 0));
	double s_mul;
	for(i = 0; i < using_flags->get_screen_mode_num(); i++) {
		if(actionScreenSize[i] == NULL) continue;
		s_mul = screen_multiply_table[i];
		if(s_mul <= 0) break;
		tmps = QString::number(s_mul);
		tmps = QString::fromUtf8("x", -1) + tmps;
		actionScreenSize[i]->setText(tmps);
	}
	bool b_support_tv_render = using_flags->is_support_tv_render();
	if(b_support_tv_render) {
		menuScreen_Render->setTitle(QApplication::translate("MainWindow", "Render Mode", 0));
		menuScreen_Render->setToolTipsVisible(true);
		action_SetRenderMode[CONFIG_RENDER_TYPE_STD]->setText(QApplication::translate("MainWindow", "Standard", 0));
		action_SetRenderMode[CONFIG_RENDER_TYPE_STD]->setToolTip(QApplication::translate("MainWindow", "Standard render.", 0));
		if(b_support_tv_render) {
			action_SetRenderMode[CONFIG_RENDER_TYPE_TV]->setText(QApplication::translate("MainWindow", "TV", 0));
			action_SetRenderMode[CONFIG_RENDER_TYPE_TV]->setToolTip(QApplication::translate("MainWindow", "Rendering like tubed  television with RF modulator.\nNeeds OpenGL 3.0 or later.Not effect with OpenGL 2.0.", 0));
		}
	}
}

