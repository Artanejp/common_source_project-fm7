/*
 * qt_events.cpp
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */


#include "emu.h"

#include <QtGui>
#include <QtOpenGL/QGLWidget>
#include <SDL/SDL.h>
#ifdef _WINDOWS
#include <GL/gl.h>
#include <GL/glext.h>
#else
#include <GL/glx.h>
#include <GL/glxext.h>
#endif
#include <GL/glu.h>
#include "qt_gldraw.h"


void GLDrawClass::mousePressEvent(QMouseEvent *event)
 {
//     lastPos = event->pos();
 }

void GLDrawClass::mouseMoveEvent(QMouseEvent *event)
 {
//     int dx = event->x() - lastPos.x();
//     int dy = event->y() - lastPos.y();

 }
