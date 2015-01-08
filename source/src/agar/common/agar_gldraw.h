/*
 * agar_gldraw.h
 *
 *  Created on: 2011/01/21
 *      Author: whatisthis
 */

#ifndef AGAR_GLDRAW_H_
#define AGAR_GLDRAW_H_

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>
#include <GL/gl.h>

#ifdef USE_OPENGL
extern void DrawOSDGL(AG_GLView *w);

extern void AGEventScaleGL(AG_Event *event);
extern void AGEventDrawGL(AG_Event *event);

extern void AGEventOverlayGL(AG_Event *event);
extern void AGEventMouseMove_AG_GL(AG_Event *event);
extern void AGEventKeyRelease_AG_GL(AG_Event *event);
extern void AGEventKeyPress_AG_GL(AG_Event *event);

extern void InitGL_AG2(int w, int h);
extern void Detach_AG_GL();
/*
 * agar_gldraw2.cpp
 */
extern void InitGL_AG2(int w, int h);
extern void DetachGL_AG2(void);

extern void AGEventDrawGL2(AG_Event *event);
extern void AGEventKeyUpGL(AG_Event *event);
extern void AGEventKeyDownGL(AG_Event *event);

extern  GLuint uVramTextureID;
extern  GLuint uNullTextureID;
#endif /* USE_OPENGL */
#endif /* AGAR_GLDRAW_H_ */
