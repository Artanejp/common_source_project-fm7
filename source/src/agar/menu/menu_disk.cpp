/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.01.09

	[AGAR -> UI -> MENU -> Disk]
*/

#include "menu_common.h"
#include "agar_logger.h"

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

#ifdef USE_FD1
#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)

static volatile void Floppy_DispD88(AG_Event *event)
{
  AG_MenuItem *me = (AG_MenuItem *)AG_SENDER();
  AG_Menu *menu = (AG_Menu *)AG_SELF();
  int drive = AG_INT(1);
  int num = AG_INT(2);
  char str[128];
   
  if((me == NULL) || (menu == NULL)) return;
  if(emu) {
    if(emu->d88_file[drive].bank_num > num) { // Enabled
//      AG_ObjectLock(AGOBJECT(menu));
      me->state = 1;
      strcpy(str, " ");
      if(emu->d88_file[drive].cur_bank == num) strcpy(str, "*");
//      printf("D88: %d: %s\n", num, emu->d88_file[drive].bank[num].name);
      strncat(str, emu->d88_file[drive].bank[num].name, 127);
      AG_MenuSetLabel(me, str);
//      AG_ObjectUnlock(AGOBJECT(menu));
    } else {
//      AG_ObjectLock(AGOBJECT(menu));
      AG_MenuSetLabel(me, " ");
      me->state = 0;
//      AG_ObjectUnlock(AGOBJECT(menu));
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



static void DeleteD88List(int drive) 
{
  int i;
  struct MenuNodes_FDx *p = NULL;
  switch(drive) {
#ifdef USE_FD8
   case 0:
	p = &MenuNode_FD_1;
        break
#endif
#ifdef USE_FD8
   case 1:
	p = &MenuNode_FD_2;
        break
#endif
#ifdef USE_FD8
   case 2:
	p = &MenuNode_FD_3;
        break
#endif
#ifdef USE_FD8
   case 3:
	p = &MenuNode_FD_4;
        break
#endif
#ifdef USE_FD8
   case 4:
	p = &MenuNode_FD_5;
        break
#endif
#ifdef USE_FD8
   case 5:
	p = &MenuNode_FD_6;
        break
#endif
#ifdef USE_FD7
   case 6:
	p = &MenuNode_FD_7;
        break
#endif
#ifdef USE_FD8
   case 7:
	p = &MenuNode_FD_8;
        break
#endif
   default:
        break;
  }
  if(p != NULL) {
     if(emu) {
	AG_ObjectLock(AGOBJECT(p->Node));
	for(i = 0; i < emu->d88_file[drive].bank_num; i++) {
	   if(p->Node_D88Img[i] == NULL) {
	      AG_MenuItemFree(p->Node_D88Img[i]);
	      p->Node_D88Img[i] = NULL;
	   }
	   
	}
	AG_ObjectUnlock(AGOBJECT(p->Node));
     }
   }
}

static void BuildD88List(int drive) 
{
  int i;
  struct MenuNodes_FDx *p = NULL;
   char str[128];
   
  switch(drive) {
#ifdef USE_FD8
   case 0:
	p = &MenuNode_FD_1;
        break
#endif
#ifdef USE_FD8
   case 1:
	p = &MenuNode_FD_2;
        break
#endif
#ifdef USE_FD8
   case 2:
	p = &MenuNode_FD_3;
        break
#endif
#ifdef USE_FD8
   case 3:
	p = &MenuNode_FD_4;
        break
#endif
#ifdef USE_FD8
   case 4:
	p = &MenuNode_FD_5;
        break
#endif
#ifdef USE_FD8
   case 5:
	p = &MenuNode_FD_6;
        break
#endif
#ifdef USE_FD7
   case 6:
	p = &MenuNode_FD_7;
        break
#endif
#ifdef USE_FD8
   case 7:
	p = &MenuNode_FD_8;
        break
#endif
   default:
        break;
  }
  if(p != NULL) {
     if(emu) {
	AG_ObjectLock(AGOBJECT(p->Node));
	for(i = 0; i < emu->d88_file[drive].bank_num; i++) {
	   strcpy(str, "　");
	   if(emu->d88_file[drive].bank[i].name != NULL) {
	      if(emu->d88_file[drive].cur_bank == i) strcpy(str, "■");
	      strncat(str, emu->d88_file[drive].bank[i].name, 127);
	   }
	   //if(p->Node_D88Img[i] != NULL) {
	      p->Node_D88Img[i] = AG_MenuAction(p->Node_D88Img_Root, str, NULL,
						Floppy_SelectD88, "%i,%i", drive, i);
	   //}
	}
	AG_ObjectUnlock(AGOBJECT(p->Node));
     }
   }
}



void OnOpenFD(AG_Event *event)
{
  int drive = AG_INT(1);

  //DeleteD88List(drive);
  if(emu) open_disk_dialog(AGWIDGET(hWindow), drive);
  //BuildD88List(drive);
}

void OnCloseFD(AG_Event *event)
{
  int drive = AG_INT(1);
  if(emu) close_disk(drive);
}
void OnRecentFD(AG_Event *event)
{
  char path[4096];
  int drive = AG_INT(1);
  int menunum = AG_INT(2);
  int i;
   
  if((menunum < 0) || (menunum > 7)) return;
  strcpy(path, config.recent_disk_path[drive][menunum]);
  for(int i = menunum; i > 0; i--) {
    strcpy(config.recent_disk_path[drive][i], config.recent_disk_path[drive][i - 1]);
  }
  strcpy(config.recent_disk_path[drive][0], path);

  //DeleteD88List(drive);
  if(emu) open_disk(drive, path, 0);
  //BuildD88List(drive);

}

void OnSelectD88Bank(AG_Event *event)
{
  int drive = AG_INT(1);
  int no = AG_INT(2);

  if((no < 0) || (no > 63)) return;
  if(emu && emu->d88_file[drive].cur_bank != no) {
    //DeleteD88List(drive);
    emu->open_disk(drive, emu->d88_file[drive].path, emu->d88_file[drive].bank[no].offset);
    emu->d88_file[drive].cur_bank = no;
    //BuildD88List(drive);
  }
}
#endif

static volatile void Floppy_DispRecent(AG_Event *event)
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

static volatile void Floppy_SelectRecent(AG_Event *event)
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
    node->Node_D88Img[i] = NULL;
    node->Node_D88Img[i] = MakeDynamicElement(node->Node_D88Img_Root, "XXXXXXXXXXX", NULL,
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
