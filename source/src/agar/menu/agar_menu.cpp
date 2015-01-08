/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.01.07

	[agar main manu]
*/

#include "menu_common.h"
#include "agar_logger.h"

// MayBE to class?
AG_MenuItem *MenuNode_Control = NULL;


#ifdef USE_FD1
struct MenuNodes_FDx MenuNode_FD_1;
#endif
#ifdef USE_FD2
struct MenuNodes_FDx MenuNode_FD_2;
#endif
#ifdef USE_FD3
struct MenuNodes_FDx MenuNode_FD_3;
#endif
#ifdef USE_FD4
struct MenuNodes_FDx MenuNode_FD_4;
#endif
#ifdef USE_FD5
struct MenuNodes_FDx MenuNode_FD_5;
#endif
#ifdef USE_FD6
struct MenuNodes_FDx MenuNode_FD_6;
#endif
#ifdef USE_FD7
struct MenuNodes_FDx MenuNode_FD_7;
#endif
#ifdef USE_FD8
struct MenuNodes_FDx MenuNode_FD_8;
#endif

#ifdef USE_QD1
struct MenuNodes MenuNode_QD_1;
#endif
#ifdef USE_QD2
struct MenuNodes MenuNode_QD_2;
#endif

#ifdef USE_TAPE
struct MenuNodes_Tape MenuNode_Tape;
#endif

#ifdef USE_LASER_DISC
struct MenuNodes MenuNode_LaserDisc;
#endif

#ifdef USE_BINARY_FILE1
struct MenuNodes MenuNode_BINARY_1;
#endif
#ifdef USE_BINARY_FILE2
struct MenuNodes MenuNode_BINARY_2;
#endif

// Copy from Agar Toolkit, gui/menu.c 
static AG_Button *CreateToolbarButton(AG_MenuItem *mi, AG_Surface *icon, const char *text)
{
        AG_Menu *m = mi->pmenu;
        AG_Button *bu;

        if (icon != NULL) {
                bu = AG_ButtonNewS(m->curToolbar->rows[0], 0, NULL);
                AG_ButtonSurface(bu, icon);
        } else {
                bu = AG_ButtonNewS(m->curToolbar->rows[0], 0, text);
        }
        AG_ButtonSetFocusable(bu, 0);
        m->curToolbar->nButtons++;
        mi->tbButton = bu;
        return (bu);
}

#define AG_MENU_NO_BINDING 0


/* Generic constructor for menu items. Menu must be locked. */
static AG_MenuItem *
CreateItem(AG_MenuItem *pitem, const char *text, AG_Surface *icon)
{
	AG_MenuItem *mi;

	if (pitem != NULL) {
		if (pitem->subitems == NULL) {
			pitem->subitems = malloc(sizeof(AG_MenuItem));
		} else {
			pitem->subitems = realloc(pitem->subitems,
					  (pitem->nsubitems+1) * 
					  sizeof(AG_MenuItem));
		}
		mi = &pitem->subitems[pitem->nsubitems++];
		mi->pmenu = pitem->pmenu;
		mi->parent = pitem;
		mi->y = pitem->nsubitems*mi->pmenu->itemh - mi->pmenu->itemh;
		mi->state = mi->pmenu->curState;
	} else {
		mi = malloc(sizeof(AG_MenuItem));
		mi->pmenu = NULL;
		mi->parent = NULL;
		mi->y = 0;
		mi->state = 1;
	}
	mi->subitems = NULL;
	mi->nsubitems = 0;
	mi->view = NULL;
	mi->sel_subitem = NULL;
	mi->key_equiv = 0;
	mi->key_mod = 0;
	mi->clickFn = NULL;
	mi->poll = NULL;
	mi->bind_type = AG_MENU_NO_BINDING;
	mi->bind_flags = 0;
	mi->bind_invert = 0;
	mi->bind_lock = NULL;
	mi->text = Strdup((text != NULL) ? text : "");
	mi->lblMenu[0] = -1;
	mi->lblMenu[1] = -1;
	mi->lblView[0] = -1;
	mi->lblView[1] = -1;
	mi->value = -1;
	mi->flags = 0;
	mi->icon = -1;
	mi->tbButton = NULL;

	if (icon != NULL) {
		if (pitem != NULL) {
			/* Request that the parent allocate space for icons. */
			pitem->flags |= AG_MENU_ITEM_ICONS;
		}
		/* TODO: NODUP */
		mi->iconSrc = AG_SurfaceDup(icon);
	} else {
		mi->iconSrc = NULL;
	}
	if (mi->pmenu != NULL &&
	   (mi->pmenu->style == AG_MENU_GLOBAL) &&
	    agDriverSw != NULL &&
	    agAppMenuWin != NULL) {
		unsigned int wMax, hMax;
		AG_SizeReq rMenu;

		AG_GetDisplaySize(agDriverSw, &wMax, &hMax);
		AG_WidgetSizeReq(mi->pmenu, &rMenu);
		AG_WindowSetGeometry(agAppMenuWin, 0, 0, wMax, rMenu.h);
	}
	if (mi->pmenu != NULL) {
		AG_Redraw(mi->pmenu);
	}
	return (mi);
}
// End copy

AG_MenuItem *MakeDynamicElement(AG_MenuItem *parent, const char *text, AG_Surface *icon,
				       void (*UpdateFn)(AG_Event *event),
				       void (*ClickFn)(AG_Event *event),
				       const char *fnArgs, ...)
{
  AG_MenuItem *me;
  //va_list ls;
  //va_start(ls, fnArgs);
  //me = AG_MenuDynamicItem(parent, text, icon, UpdateFn, fnArgs, ls);
  
  AG_ObjectLock(parent->pmenu);
  me = CreateItem(parent, text, icon);
  
  me->poll = AG_SetEvent(parent->pmenu, NULL, UpdateFn, NULL);
  AG_EVENT_GET_ARGS(me->poll, fnArgs);
  
  me->clickFn = AG_SetEvent(parent->pmenu, NULL, ClickFn, NULL);
  AG_EVENT_GET_ARGS(me->clickFn, fnArgs);
  
  if(parent->pmenu->curToolbar != NULL) {
    AG_Event *bev;
    me->tbButton = CreateToolbarButton(parent, icon, text);
    bev = AG_SetEvent(me->tbButton, "button-pushed", ClickFn, NULL);
    AG_EVENT_GET_ARGS(bev, fnArgs);
  }
  AG_ObjectUnlock(parent->pmenu);
  return me;
}

#ifdef USE_FD1
static void Floppy_DispD88(AG_Event *event)
{
  AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
  AG_Menu *menu = (AG_Menu *)AG_SELF();
  int drive = AG_INT(1);
  int num = AG_INT(2);
  char str[128];
  if(emu) {
    if(emu->d88_file[drive].bank_num > num) { // Enabled
      me->state = 1;
      strncpy(str, emu->d88_file[drive].bank[num].name, 127);
      if(emu->d88_file[drive].cur_bank == num) {
	AG_MenuSetLabel(me, "■%s", str); // Selected
      } else {
	AG_MenuSetLabel(me, "　%s", str);
      }
    } else {
      AG_MenuSetLabel(me, " ");
      me->state = 0;
    }
  }
}

void Floppy_SelectD88(AG_Event *event)
{
  AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
  AG_Menu *menu = (AG_Menu *)AG_SELF();
  int drive = AG_INT(1);
  int num = AG_INT(2);
  OnSelectD88Bank(event);
  AGAR_DebugLog(AGAR_LOG_DEBUG, "Selected D88 %d, %d\n", drive, num);
}

static void Floppy_DispRecent(AG_Event *event)
{
  AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
  AG_Menu *menu = (AG_Menu *)AG_SELF();
  int drive = AG_INT(1);
  int num = AG_INT(2);
  const char *str = config.recent_disk_path[drive][num];
  if(strlen(str) > 0) {
    AG_MenuSetLabel(me, "%s", AG_ShortFilename(str));
  } else {
    AG_MenuSetLabel(me, "%s", "   ");
  }
  me->state = 1;
}

static void Floppy_SelectRecent(AG_Event *event)
{
  AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
  AG_Menu *menu = (AG_Menu *)AG_SELF();
  int drive = AG_INT(1);
  int num = AG_INT(2);

  OnRecentFD(event);
  AGAR_DebugLog(AGAR_LOG_DEBUG, "Selected Recent image Drive = %d, %d\n", drive, num);
}

static void OpenDiskDlgSub(AG_Event *event)
{
  AG_Widget *self = AG_SELF();
  int drv = AG_INT(1);

  open_disk_dialog(self, drv);
}
static void CloseDiskDlgSub(AG_Event *event)
{
  AG_Widget *self = AG_SELF();
  int drv = AG_INT(1);

  close_disk(drv);
}


void FloppyMenu(struct MenuNodes_FDx* node, int drive)
{
  int i;
  
  // Insert
  node->Insert = AG_MenuAction(node->Node, _N("Insert"), NULL, OpenDiskDlgSub, "%i", drive);
  // Eject
  node->Eject  = AG_MenuAction(node->Node, _N("Eject"), NULL, CloseDiskDlgSub, "%i", drive);
  AG_MenuSeparator(node->Node);

  node->Node_D88Img_Root = AG_MenuNode(node->Node, _N("Select Disk Image"), NULL); 
  
  for(i = 0; i < 64; i++) {
    node->Node_D88Img[i] = MakeDynamicElement(node->Node_D88Img_Root, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", NULL,
					      Floppy_DispD88,
					      Floppy_SelectD88,
					      "%i,%i", drive, i);

  }    
  AG_MenuSeparator(node->Node);

  node->Node_Recent_Root = AG_MenuNode(node->Node, _N("Recent Selected Disks"), NULL); 
  for(i = 0; i < 8; i++) {
    node->Node_Recent[i] = MakeDynamicElement(node->Node_Recent_Root, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", NULL,
					      Floppy_DispRecent,
					      Floppy_SelectRecent,
					      "%i,%i", drive, i);
  }
}
#else // Not FD
void FloppyMenu(struct MenuNodes_FDx* node, int drive)
{
}
#endif // FD1

#ifdef USE_TAPE
static void Tape_DispRecent(AG_Event *event)
{
  AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
  AG_Menu *menu = (AG_Menu *)AG_SELF();
  int num = AG_INT(1);
  const char *str = config.recent_tape_path[num];
  if(strlen(str) > 0) {
    AG_MenuSetLabel(me, "%s", AG_ShortFilename(str));
  } else {
    AG_MenuSetLabel(me, "%s", "   ");
  }
  me->state = 1;
}

static void Tape_SelectRecent(AG_Event *event)
{
  AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
  AG_Menu *menu = (AG_Menu *)AG_SELF();
  int num = AG_INT(1);

  OnRecentTAPE(event);
  AGAR_DebugLog(AGAR_LOG_DEBUG, "Selected Recent Tape %d\n", num);
}

static void OnOpenTapeDlgSub(AG_Event *event)
{
  AG_Widget *me = AG_SELF();
  int play = AG_INT(1);
  bool playf = false;
  if(play != 0) playf = true;
  open_tape_dialog(me, playf);
}

void TapeMenu(struct MenuNodes_Tape* node)
{
  int i;
  
  // Insert
  node->Insert = AG_MenuAction(node->Node, _N("Insert Tape"), NULL, OnOpenTapeDlgSub, "%i", 1);
  // Eject
  node->Eject  = AG_MenuAction(node->Node, _N("Eject Tape"), NULL, OnCloseTAPE, NULL);
  AG_MenuSeparator(node->Node);

  // Record
  node->Record = AG_MenuAction(node->Node, _N("Record to Tape"), NULL, OnOpenTapeDlgSub, "%i", 0);

  node->Node_Recent_Root = AG_MenuNode(node->Node, _N("Recent Selected Tapes"), NULL); 
  AG_MenuSeparator(node->Node);
  
  for(i = 0; i < 8; i++) {
    node->Node_Recent[i] = MakeDynamicElement(node->Node_Recent_Root, "   ", NULL,
					      Tape_DispRecent,
					      Tape_SelectRecent,
					      "%i", i);
  }
  AG_MenuSeparator(node->Node);
}
#else
void TapeMenu(struct MenuNodes_Tape* node)
{
}
#endif // USE_TAPE
