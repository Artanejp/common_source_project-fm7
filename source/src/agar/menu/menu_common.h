/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.01.07

	[agar main manu]
*/

#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "common.h"
#include "emu.h"
#include "agar_main.h"
#include "agar_emuevents.h"

struct MenuNodes_FDx {
  AG_MenuItem *Node;
  AG_MenuItem *Insert;
  AG_MenuItem *Eject;
  AG_MenuItem *Node_D88Img_Root;
  AG_MenuItem *Node_D88Img[64];
  AG_MenuItem *Node_Recent_Root;
  AG_MenuItem *Node_Recent[8];
};

struct MenuNodes_Tape {
  AG_MenuItem *Node;
  AG_MenuItem *Insert;
  AG_MenuItem *Eject;
  AG_MenuItem *Record;
  AG_MenuItem *Node_Recent_Root;
  AG_MenuItem *Node_Recent[8];
};

struct MenuNodes {
  AG_MenuItem *Node;
  AG_MenuItem *Insert;
  AG_MenuItem *Eject;
  AG_MenuItem *Node_Recent_Root;
  AG_MenuItem *Node_Recent[8];
};

// MayBE to class?
extern AG_MenuItem *MenuNode_Control;


#ifdef USE_FD1
extern struct MenuNodes_FDx MenuNode_FD_1;
#endif
#ifdef USE_FD2
extern struct MenuNodes_FDx MenuNode_FD_2;
#endif
#ifdef USE_FD3
extern struct MenuNodes_FDx MenuNode_FD_3;
#endif
#ifdef USE_FD4
extern struct MenuNodes_FDx MenuNode_FD_4;
#endif
#ifdef USE_FD5
extern struct MenuNodes_FDx MenuNode_FD_5;
#endif
#ifdef USE_FD6
extern struct MenuNodes_FDx MenuNode_FD_6;
#endif
#ifdef USE_FD7
extern struct MenuNodes_FDx MenuNode_FD_7;
#endif
#ifdef USE_FD8
extern struct MenuNodes_FDx MenuNode_FD_8;
#endif

#ifdef USE_QD1
extern struct MenuNodes MenuNode_QD_1;
#endif
#ifdef USE_QD2
extern struct MenuNodes MenuNode_QD_2;
#endif

#ifdef USE_TAPE
extern struct MenuNodes_Tape MenuNode_Tape;
#endif

#ifdef USE_LASER_DISC
extern struct MenuNodes MenuNode_LaserDisc;
#endif

#ifdef USE_BINARY_FILE1
extern struct MenuNodes MenuNode_BINARY_1;
#endif
#ifdef USE_BINARY_FILE2
extern struct MenuNodes MenuNode_BINARY_2;
#endif

extern AG_MenuItem *MakeDynamicElement(AG_MenuItem *parent, const char *text, AG_Surface *icon,
				       void (*UpdateFn)(AG_Event *event),
				       void (*ClickFn)(AG_Event *event),
				       const char *fnArgs, ...);


extern AG_Menu *AGAR_MainMenu(AG_Widget *parent);


extern void FloppyMenu(struct MenuNodes_FDx* node, int drive);
extern void TapeMenu(struct MenuNodes_Tape* node);

extern void Convert_CP932_to_UTF8(char *dst, char *src, int n_limit);


// Support routines
#ifdef USE_CART1
void open_cart_dialog(AG_Widget *hWnd, int drv); // int slot
#endif

#ifdef USE_FD1
extern void open_disk_dialog(AG_Widget *hWnd, int drv); // int drv
extern void open_disk(int drv, _TCHAR* path, int bank);
extern void close_disk(int drv);
#endif

#ifdef USE_QD1
extern void open_quickdisk_dialog(AG_Widget *hWnd, int drv); // int drv
#endif

#ifdef USE_TAPE
extern void open_tape_dialog(AG_Widget *hWnd, bool play); // int play : 1 = play, 0 = rec
#endif

#ifdef USE_LASER_DISC
extern void open_laser_disc_dialog(AG_Widget *hWnd); 
#endif

#ifdef USE_BINARY_FILE1
extern void open_binary_dialog(AG_widget *hWnd, int drv, bool load); // int drv, int load ; 1 = load?
#endif

#ifdef SUPPORT_DRAG_DROP
extern void open_any_file(_TCHAR* path);
#endif

