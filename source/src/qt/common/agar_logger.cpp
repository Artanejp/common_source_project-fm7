/*
 * Log functions
 * (C) 2014-06-30 K.Ohta
 * History:
 *  Dec 30, 2014 Move from XM7/SDL, this was Ohta's original code.
 * Licence : GPLv2
 */

#include "agar_logger.h"
#include <string.h>
#include "emu.h"
#include "vm/vm.h"

static int syslog_flag = 0;
static int log_cons = 0;
static int log_onoff = 0;
static int log_opened = FALSE;
extern "C" 
{
   
void AGAR_OpenLog(int syslog, int cons)
{
	int flags = 0;
	
	log_onoff = 1;
	char sysname[128];
	memset(sysname, 0x00, 128);
	strncpy(sysname, "Common Source Project(", 127);
	strncat(sysname, DEVICE_NAME, 127);
	strncat(sysname, ")", 127);
	
	if(syslog != 0) {
	   syslog_flag = -1;
#if defined(_SYS_SYSLOG_H) || defined(_SYSLOG_H)
	   if(cons != 0) { 
	      flags = LOG_CONS;
	   }
	   openlog(sysname, flags | LOG_PID | LOG_NOWAIT, LOG_USER);
#endif
	} else {
	   syslog_flag = 0;
	}
	log_cons = cons;
	log_opened = TRUE;
     }
   
   
void AGAR_DebugLog(int level, const char *fmt, ...)
     {
	va_list ap;
	struct tm *timedat;
	time_t nowtime;
	char strbuf[4096];
	char strbuf2[256];
	char strbuf3[24];
	struct timeval tv;
	int level_flag = LOG_USER;
	
	if(log_onoff == 0) return;
	
	if(level == AGAR_LOG_DEBUG) {
	   level_flag |= LOG_DEBUG;
	} else if(level == AGAR_LOG_INFO) { 
	   level_flag |= LOG_INFO;
	} else if(level == AGAR_LOG_WARN) {
	   level_flag |= LOG_WARNING;
	} else {
	   level_flag |= LOG_DEBUG;
	}
	
	char sysname[128];
	memset(sysname, 0x00, 128);
	strncpy(sysname, "Common Source Project(", 127);
	strncat(sysname, DEVICE_NAME, 127);
	strncat(sysname, ")", 127);
	
	va_start(ap, fmt);	
	vsnprintf(strbuf, 4095, fmt, ap);
	nowtime = time(NULL);
	gettimeofday(&tv, NULL);
	if(log_cons != 0) { // Print only
	   timedat = localtime(&nowtime);
	   strftime(strbuf2, 255, "%Y-%m-%d %H:%M:%S", timedat);
	   snprintf(strbuf3, 23, ".%06ld", tv.tv_usec);
	   fprintf(stdout, "%s : %s%s %s\n", sysname, strbuf2, strbuf3, strbuf);
	} 
	if(syslog_flag != 0) { // SYSLOG
	   syslog(level_flag, "uS=%06ld %s", tv.tv_usec, strbuf);
	}
	va_end(ap);
     }

void AGAR_SetLogStatus(int sw)
     {
	if(sw == 0) {
	   log_onoff = 0;
	} else {
	   log_onoff = 1;
	}
     }
   
void AGAR_SetLogStdOut(int sw)
     {
	if(sw == 0) {
	   log_cons = 0;
	} else {
	   log_cons = 1;
	}
     }

void AGAR_SetLogSysLog(int sw)
     {
	if(sw == 0) {
	   syslog_flag = 0;
	} else {
	   syslog_flag = 1;
	}
     }

bool AGAR_LogGetStatus(void)
     {
	return (bool) log_opened;
     }
   
	
void AGAR_CloseLog(void)
    {
	if(syslog_flag != 0) {
	     closelog();
	}
	syslog_flag = 0;
	log_cons = 0;
        log_onoff = 0;
        log_opened = 0;
     }
}

