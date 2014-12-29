/*
 * Agar <- Win32 Wrapper
 * 2014-12-30 K.Ohta <Whatisthis.sowhat@gmail.com>
 */

#include <agar/core.h>
#include <agar/gui.h>

extern "C" 
{
void Sleep(int tick)
     {
	AG_Delay(tick);
     }
   
uint32_t timeGetTime(void)
     {
	return AG_GetTicks();
     }
}
