#ifndef AGAR_DRAW_H_INCLUDED
#define AGAR_DRAW_H_INCLUDED

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>
//#include <agar/gui/opengl.h>
//#include "EmuAgarGL.h"
#include "api_vram.h"
//#include "api_draw.h"
//#include "api_scaler.h"

//#include "agar_vramutil.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_sdlview.h"

//#include "DrawAGNonGL.h"

#include <SDL/SDL.h>
#ifdef __cplusplus
extern "C" {
#endif
//    extern AG_Pixmap *DrawArea;
    extern AGAR_SDLView *DrawArea;
    extern AG_Window *MainWindow;
    extern AG_Menu  *MenuBar;
    extern AG_HBox *pStatusBar;
    extern void DrawStatus(void);
    extern AG_Surface *GetDrawSurface(void);
    extern int DrawSurfaceId;
    extern AG_Surface *DrawSurface;
    extern unsigned int nRenderMethod;
    extern BOOL IsUsingCL(void);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern Uint32 nDrawTick1E;

extern void InitGL(int w, int h);
extern void InitNonGL(int w, int h);
extern void ResizeWindow_Agar(int w, int h);
extern void AGDrawTaskMain(void);
#endif
#endif // AGAR_DRAW_H_INCLUDED
