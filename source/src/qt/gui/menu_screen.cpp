/*
 * Common Source code Project:
 * Ui->Qt->gui->menu_main for generic.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, moved from qt/x1turboz/MainWindow.cpp .
 */

#include <QtCore/QVariant>
#include <QtGui>
#include "commonclasses.h"
#include "mainwidget.h"
//#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

QT_BEGIN_NAMESPACE

#if (SCREEN_WIDTH > 320)
const static double screen_multiply_table[] = {0.5, 1.0, 1.5, 2.0, 2.25, 2.5, 3.0, 3.5, 4.0, 5.0, 6.0, 8.0, 0.0};
#else
const static double screen_multiply_table[] = {0.5, 1.0, 1.5, 2.0, 3.0, 4.0, 5.0, 6.0, 7.5, 10.0, 12.0, 15.0, 0.0};
#endif
void Object_Menu_Control::set_screen_aspect(void) {
	int num = getValue1();
	emit sig_screen_aspect(num);
}

void Object_Menu_Control::set_screen_size(void) {
	int w, h;
	double nd;
	config.window_mode = getNumber();
	nd = getDoubleValue();
	w = (int)(nd * (double)SCREEN_WIDTH);
	h = (int)(nd * (double)SCREEN_HEIGHT);
#if defined(USE_CRT_MONITOR_4_3)
	if(config.stretch_type == 1) {
		h = (int)((double)h * ((double)SCREEN_WIDTH / (double)SCREEN_HEIGHT * 3.0 / 4.0));
	} else if(config.stretch_type == 2) {
		w = (int)((double)w * (4.0 / (3.0 * (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT)));
	}
#endif
	emit sig_screen_size(w, h);
}

void Ui_MainWindow::set_gl_scan_line_vert(bool f)
{
	config.opengl_scanline_vert = f;
}

void Ui_MainWindow::set_gl_scan_line_horiz(bool f)
{
	config.opengl_scanline_horiz = f;
}

void Ui_MainWindow::ConfigScreenMenu_List(void)
{
	int w, h;
	QString tmps;
	int i;
	
	actionGroup_ScreenSize = new QActionGroup(this);
	actionGroup_ScreenSize->setExclusive(true);
	screen_mode_count = 0;
	for(i = 0; i < _SCREEN_MODE_NUM; i++) {
		w = (int)(screen_multiply_table[i] * (double)SCREEN_WIDTH);
		h = (int)(screen_multiply_table[i] * (double)SCREEN_HEIGHT);
		if((w <= 0) || (h <= 0)) {
			break;
		}
		screen_mode_count++;
		tmps = QString::number(i);
		actionScreenSize[i] = new Action_Control(this);
		actionScreenSize[i]->setObjectName(QString::fromUtf8("actionScreenSize", -1) + tmps);
		actionScreenSize[i]->setCheckable(true);
		actionScreenSize[i]->binds->setNumber(i);

		if(i == config.window_mode)  actionScreenSize[i]->setChecked(true);  // OK?

		actionGroup_ScreenSize->addAction(actionScreenSize[i]);
		actionScreenSize[i]->binds->setDoubleValue(screen_multiply_table[i]);
		
		connect(actionScreenSize[i], SIGNAL(triggered()),
			actionScreenSize[i]->binds, SLOT(set_screen_size()));
		connect(actionScreenSize[i]->binds, SIGNAL(sig_screen_size(int, int)),
			this, SLOT(set_screen_size(int, int)));
	}
	for(; i < _SCREEN_MODE_NUM; i++) {
		actionScreenSize[i] = NULL;
	}
}
  
void Ui_MainWindow::ConfigScreenMenu(void)
{
	actionZoom = new Action_Control(this);
	actionZoom->setObjectName(QString::fromUtf8("actionZoom"));
	actionDisplay_Mode = new Action_Control(this);
	actionDisplay_Mode->setObjectName(QString::fromUtf8("actionDisplay_Mode"));
	
#ifdef USE_SCANLINE
	actionScanLine = new Action_Control(this);
	actionScanLine->setObjectName(QString::fromUtf8("actionScanLine"));
	actionScanLine->setCheckable(true);
	if(config.scan_line != 0) {
		actionScanLine->setChecked(true);
	} else {
		actionScanLine->setChecked(false);
	}
	connect(actionScanLine, SIGNAL(toggled(bool)),
		this, SLOT(set_scan_line(bool)));
#endif
#if !defined(ONE_BOARD_MICRO_COMPUTER) && !defined(MAX_BUTTONS)
	actionGLScanLineHoriz = new Action_Control(this);
	actionGLScanLineHoriz->setObjectName(QString::fromUtf8("actionGLScanLineHoriz"));
	actionGLScanLineHoriz->setCheckable(true);
	if(config.opengl_scanline_horiz != 0) {
		actionGLScanLineHoriz->setChecked(true);
	} else {
		actionGLScanLineHoriz->setChecked(false);
	}
	connect(actionGLScanLineHoriz, SIGNAL(toggled(bool)),
			this, SLOT(set_gl_scan_line_horiz(bool)));
# if defined(USE_VERTICAL_PIXEL_LINES)	
	actionGLScanLineVert = new Action_Control(this);
	actionGLScanLineVert->setObjectName(QString::fromUtf8("actionGLScanLineVert"));
	actionGLScanLineVert->setCheckable(true);
	if(config.opengl_scanline_vert != 0) {
		actionGLScanLineVert->setChecked(true);
	} else {
		actionGLScanLineVert->setChecked(false);
	}
	connect(actionGLScanLineVert, SIGNAL(toggled(bool)),
			this, SLOT(set_gl_scan_line_vert(bool)));
# endif
#endif	
#ifdef USE_SCREEN_ROTATE
	actionRotate = new Action_Control(this);
	actionRotate->setObjectName(QString::fromUtf8("actionScanLine"));
	actionRotate->setCheckable(true);
	if(config.rotate_type) {
		actionRotate->setChecked(true);
	} else {
		actionRotate->setChecked(false);
	}
	connect(actionRotate, SIGNAL(toggled(bool)),
		this, SLOT(set_screen_rotate(bool)));
#endif
	
#ifdef USE_CRT_FILTER
	actionCRT_Filter = new Action_Control(this);
	actionCRT_Filter->setObjectName(QString::fromUtf8("actionCRT_Filter"));
	actionCRT_Filter->setEnabled(true);
	actionCRT_Filter->setCheckable(true);
	if(config.crt_filter == 0) actionCRT_Filter->setChecked(true);
	connect(actionCRT_Filter, SIGNAL(toggled(bool)), this, SLOT(set_crt_filter(bool)));
#endif
	actionOpenGL_Filter = new Action_Control(this);
	actionOpenGL_Filter->setObjectName(QString::fromUtf8("actionOpenGL_Filter"));
	actionOpenGL_Filter->setEnabled(true);
	actionOpenGL_Filter->setCheckable(true);
	if(config.use_opengl_filters) actionOpenGL_Filter->setChecked(true);
	connect(actionOpenGL_Filter, SIGNAL(toggled(bool)), this, SLOT(set_gl_crt_filter(bool)));

#if defined(USE_CRT_MONITOR_4_3)	
	actionDot_by_Dot = new Action_Control(this);
	actionDot_by_Dot->setObjectName(QString::fromUtf8("actionDot_by_Dot"));
	actionDot_by_Dot->setCheckable(true);
	if(config.stretch_type == 0) actionDot_by_Dot->setChecked(true);
	actionDot_by_Dot->binds->setValue1(0);
	
	actionKeep_Aspect = new Action_Control(this);
	actionKeep_Aspect->setObjectName(QString::fromUtf8("actionKeep_Aspect"));
	actionKeep_Aspect->setCheckable(true);
	actionKeep_Aspect->binds->setValue1(1);
	if(config.stretch_type == 1) actionKeep_Aspect->setChecked(true);
	
	actionFill_Display = new Action_Control(this);
	actionFill_Display->setObjectName(QString::fromUtf8("actionFill_Display"));
	actionFill_Display->setCheckable(true);
	actionFill_Display->binds->setValue1(2);
	if(config.stretch_type == 2) actionFill_Display->setChecked(true);

	actionGroup_Stretch = new QActionGroup(this);
	actionGroup_Stretch->setExclusive(true);
	actionGroup_Stretch->addAction(actionDot_by_Dot);
	actionGroup_Stretch->addAction(actionKeep_Aspect);
	actionGroup_Stretch->addAction(actionFill_Display);
	connect(actionDot_by_Dot,   SIGNAL(triggered()), actionDot_by_Dot->binds,   SLOT(set_screen_aspect()));
	connect(actionDot_by_Dot->binds,   SIGNAL(sig_screen_aspect(int)), this,    SLOT(set_screen_aspect(int)));
	
	connect(actionKeep_Aspect,  SIGNAL(triggered()), actionKeep_Aspect->binds,  SLOT(set_screen_aspect()));
	connect(actionKeep_Aspect->binds,  SIGNAL(sig_screen_aspect(int)), this,    SLOT(set_screen_aspect(int)));
	
	connect(actionFill_Display, SIGNAL(triggered()), actionFill_Display->binds, SLOT(set_screen_aspect()));
	connect(actionFill_Display->binds, SIGNAL(sig_screen_aspect(int)), this, SLOT(set_screen_aspect(int)));
#endif	
	actionCapture_Screen = new Action_Control(this);
	actionCapture_Screen->setObjectName(QString::fromUtf8("actionCapture_Screen"));

	actionStart_Record_Movie = new Action_Control(this);
	actionStart_Record_Movie->setObjectName(QString::fromUtf8("actionStart_Record_Movie"));
	actionStart_Record_Movie->setCheckable(true);
	
	actionStop_Record_Movie = new Action_Control(this);
	actionStop_Record_Movie->setObjectName(QString::fromUtf8("actionStop_Record_Movie"));
	actionStop_Record_Movie->setCheckable(false);

	ConfigScreenMenu_List();  
}

void Ui_MainWindow::CreateScreenMenu(void)
{
	int i;
	menuScreen = new QMenu(menubar);
	menuScreen->setObjectName(QString::fromUtf8("menuScreen"));
#if defined(USE_CRT_MONITOR_4_3)	
	menuStretch_Mode = new QMenu(menuScreen);
	menuStretch_Mode->setObjectName(QString::fromUtf8("menuStretch_Mode"));
#endif
	menuScreenSize = new QMenu(menuScreen);
	menuScreenSize->setObjectName(QString::fromUtf8("menuScreen_Size"));
	menuScreen->addSeparator();
	menuRecord_as_movie = new QMenu(menuScreen);
	menuRecord_as_movie->setObjectName(QString::fromUtf8("menuRecord_as_movie"));

	menuScreen->addAction(actionZoom);
	menuScreen->addAction(menuScreenSize->menuAction());
	for(i = 0; i < _SCREEN_MODE_NUM; i++) {
		if(actionScreenSize[i] == NULL) continue;
		menuScreenSize->addAction(actionScreenSize[i]);
		actionScreenSize[i]->setVisible(true);
	}
#if defined(USE_CRT_MONITOR_4_3)	
	menuScreen->addSeparator();
	menuScreen->addAction(menuStretch_Mode->menuAction());

	menuStretch_Mode->addAction(actionDot_by_Dot);
	menuStretch_Mode->addAction(actionKeep_Aspect);
	menuStretch_Mode->addAction(actionFill_Display);
#endif
	menuScreen->addSeparator();
#ifdef USE_SCANLINE
	menuScreen->addAction(actionScanLine);
#endif
#if !defined(ONE_BOARD_MICRO_COMPUTER) && !defined(MAX_BUTTONS)
	menuScreen->addAction(actionGLScanLineHoriz);
# ifdef USE_VERTICAL_PIXEL_LINES	
	menuScreen->addAction(actionGLScanLineVert);
# endif
#endif	
#ifdef USE_SCREEN_ROTATE
	menuScreen->addAction(actionRotate);
#endif
#ifdef USE_CRT_FILTER
	menuScreen->addAction(actionCRT_Filter);
#endif   
	menuScreen->addAction(actionOpenGL_Filter);
	menuScreen->addAction(actionCapture_Screen);
	menuScreen->addSeparator();
	menuScreen->addAction(menuRecord_as_movie->menuAction());
	menuRecord_as_movie->addAction(actionStart_Record_Movie);
	menuRecord_as_movie->addAction(actionStop_Record_Movie);
}

void Ui_MainWindow::retranslateScreenMenu(void)
{
	int i;
	QString tmps;
	int w, h;
	actionZoom->setText(QApplication::translate("MainWindow", "Zoom Screen", 0));
	actionDisplay_Mode->setText(QApplication::translate("MainWindow", "Display Mode", 0));
#ifdef USE_SCANLINE
	actionScanLine->setText(QApplication::translate("MainWindow", "Set ScanLine", 0));
#endif
#ifdef USE_SCREEN_ROTATE
	actionRotate->setText(QApplication::translate("MainWindow", "Rotate Screen", 0));
#endif   
#ifdef USE_CRT_FILTER
	actionCRT_Filter->setText(QApplication::translate("MainWindow", "CRT Filter", 0));
#endif
#if !defined(ONE_BOARD_MICRO_COMPUTER) && !defined(MAX_BUTTONS)
	actionGLScanLineHoriz->setText(QApplication::translate("MainWindow", "OpenGL Scan Line", 0));
# if defined(USE_VERTICAL_PIXEL_LINES)	
	actionGLScanLineVert->setText(QApplication::translate("MainWindow", "OpenGL Pixel Line", 0));
# endif
#endif
	actionOpenGL_Filter->setText(QApplication::translate("MainWindow", "OpenGL Filter", 0));
#if defined(USE_CRT_MONITOR_4_3)	
	actionDot_by_Dot->setText(QApplication::translate("MainWindow", "Dot by Dot", 0));
	actionKeep_Aspect->setText(QApplication::translate("MainWindow", "Keep Aspect: Refer to X", 0));
	actionFill_Display->setText(QApplication::translate("MainWindow", "Keep Aspect: Refer to Y", 0));
#endif  
	actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0));

	menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0));
#if defined(USE_CRT_MONITOR_4_3)	
	menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0));
#endif
	
	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0));
	actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0));

	menuRecord_as_movie->setTitle(QApplication::translate("MainWindow", "Record as movie", 0));


	menuScreenSize->setTitle(QApplication::translate("MainWindow", "Screen size", 0));
	double s_mul;
	for(i = 0; i < _SCREEN_MODE_NUM; i++) {
		if(actionScreenSize[i] == NULL) continue;
		s_mul = screen_multiply_table[i];
		if(s_mul <= 0) break;
		tmps = QString::number(s_mul);
		tmps = QString::fromUtf8("x", -1) + tmps;
		actionScreenSize[i]->setText(tmps);
	}
}

