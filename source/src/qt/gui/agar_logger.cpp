/*
 * Log functions
 * (C) 2014-06-30 K.Ohta
 * History:
 *  Dec 30, 2014 Move from XM7/SDL, this was Ohta's original code.
 * Licence : GPLv2
 */

#include <QMutex>
#include <QMutexLocker>

#include "agar_logger.h"
#include <string.h>
#include "emu.h"
#include "vm/vm.h"
#include "menu_flags.h"


CSP_Logger::CSP_Logger(bool b_syslog, bool cons, const char *devname)
{
	lock_mutex = new QMutex(QMutex::Recursive);
	QString tmps;
	const char *p;

	component_names.clear();
	vfile_names.clear();
	cpu_names.clear();
	device_names.clear();
	
	const char *components[] = {
		"GENERAL",
		"OSD",
		"EMU",
		"VM",
		"GUI",
		"SOUND",
		"VIDEO",
		"KEYBOARD",
		"MOUSE",
		"JOYSTICK",
		"MOVIE_LOADER",
		"MOVIE_SAVER",
		"SCREEN",
		"PRINTER",
		"SOCKET",
		"Undefined",
		NULL
	};
	for(int i = 0; ; i++) {
		p = components[i];
		if(p == NULL) break;
		tmps = QString::fromUtf8(p);
		component_names.append(tmps);
	}
	const char *vfiles[] = {
		"BINARY",
		"BUBBLE",
		"CART",
		"CMT",
		"CD",
		"FD",
		"LD",
		"QD",
		NULL
	};
	for(int i = 0; ; i++) {
		QString ss;
		int lim;
		p = vfiles[i];
		if(p == NULL) break;
		tmps = QString::fromUtf8(p);
		if(i == 5) { // FD
			lim = 16;
		} else if(i == 7) { // QD
			lim = 16;
		} else {
			lim = 8;
		}
		for(int j = 0; j < lim; j++) {
			ss.setNum(j + 1);
			vfile_names.append(tmps + ss);
		}
	}
	
	tmps = QString::fromUtf8("CPU"); // Enable to update
	for(int i = 0; i < 8; i++) {
		QString ss;
		ss.setNum(i);
		cpu_names.append(tmps + ss);
	}
	tmps = QString::fromUtf8("DEV"); // Enable to update
	for(int i = 0; i < (256 - (32 + 8)); i++) {
		QString ss;
		ss.setNum(i);
		device_names.append(tmps + ss);
	}
	
	
	this->open(b_syslog, cons, devname);
}

CSP_Logger::~CSP_Logger()
{
	loglist.clear();
	log_sysname.clear();
	squeue.clear();
	delete lock_mutex;
}

//extern class USING_FLAGS *using_flags;
void CSP_Logger::open(bool b_syslog, bool cons, const char *devname)
{
	int flags = 0;
	
	log_onoff = true;
	
	loglist.clear();
	log_sysname.clear();
	squeue.clear();
	
	QString dname;
	if(devname != NULL) {
		dname = QString::fromUtf8(devname);
	} else {
		dname = QString::fromUtf8("*Undefined*");
	}
	log_sysname = QString::fromUtf8("Common Source Code Project(");
	log_sysname = log_sysname + dname + QString::fromUtf8(")");
	
	if(b_syslog) {
		syslog_flag = true;
#if defined(_SYS_SYSLOG_H) || defined(_SYSLOG_H)
		if(cons) { 
			flags = LOG_CONS;
		}
		openlog(log_sysname.toLocal8Bit().constData(), flags | LOG_PID | LOG_NOWAIT, LOG_USER);
#endif
	} else {
		syslog_flag = false;
	}
	log_cons = cons;
	log_opened = true;
	log_cons_out = log_cons;
	syslog_flag_out = syslog_flag;
	
	cons_log_levels = 1 << CSP_LOG_INFO;
	linenum = 1;
	line_wrap = 0;
	
	this->debug_log(CSP_LOG_INFO, "Start logging.");
}

void CSP_Logger::debug_log(int level, const char *fmt, ...)
{
	char strbuf[4096];
	va_list ap;
	
	va_start(ap, fmt);	
	vsnprintf(strbuf, 4095, fmt, ap);
	debug_log(level, 0, strbuf);
	va_end(ap);
}


void CSP_Logger::debug_log(int level, int domain_num, const char *fmt, ...)
{
	char strbuf[4096];
	va_list ap;
	
	va_start(ap, fmt);	
	vsnprintf(strbuf, 4095, fmt, ap);
	debug_log(level, domain_num, strbuf);
	va_end(ap);
}

void CSP_Logger::debug_log(int level, int domain_num, char *strbuf)
{
	struct tm *timedat;
	time_t nowtime;
	char strbuf2[256];
	char strbuf3[24];
	struct timeval tv;
	
	if(!log_opened) return;
	if(level < 0) level = 0;
	if(level >= CSP_LOG_LEVELS) level = CSP_LOG_LEVELS - 1;
		
#if !defined(Q_OS_WIN)   
	int level_flag = LOG_USER;
	if(level == CSP_LOG_DEBUG) {
	   level_flag |= LOG_DEBUG;
	} else if(level == CSP_LOG_INFO) { 
	   level_flag |= LOG_INFO;
	} else if(level == CSP_LOG_WARN) {
	   level_flag |= LOG_WARNING;
	} else {
	   level_flag |= LOG_DEBUG;
	}
#endif	
	char *p;
	char *p_bak;
	const char delim[2] = "\n";
#ifdef __MINGW32__
	p = strtok(strbuf, delim);
#else
	p = strtok_r(strbuf, delim, &p_bak); 
#endif
	if(strbuf != NULL) {
		nowtime = time(NULL);
		gettimeofday(&tv, NULL);
		if(log_cons != 0) { // Print only
			timedat = localtime(&nowtime);
			strftime(strbuf2, 255, "%Y-%m-%d %H:%M:%S", timedat);
			snprintf(strbuf3, 23, ".%06ld", tv.tv_usec);
		}
		QString time_s = QString::fromUtf8(strbuf2) + QString::fromUtf8(strbuf3);
		QMutexLocker locker(lock_mutex);
		
		int cons_log_level_n = (1 << level) & cons_log_levels;
		int sys_log_level_n = (1 << level) & sys_log_levels;
		QString domain_s;
		domain_s.clear();
		
		if((domain_num > 0) && (domain_num < CSP_LOG_TYPE_COMPONENT_END)) {
			domain_s = component_names.at(domain_num - 1);
		} else if((domain_num >= CSP_LOG_TYPE_VM_CPU0) && (domain_num < CSP_LOG_TYPE_VM_DEVICE_0)) {
			domain_s = cpu_names.at(domain_num - CSP_LOG_TYPE_VM_CPU0);
		} else if((domain_num >= CSP_LOG_TYPE_VM_DEVICE_0) && (domain_num <= CSP_LOG_TYPE_VM_DEVICE_END)) {
			domain_s = device_names.at(domain_num - CSP_LOG_TYPE_VM_DEVICE_0);
		} else if((domain_num >= CSP_LOG_TYPE_VFILE_HEAD) && (domain_num < CSP_LOG_TYPE_VFILE_END)) {
			domain_s = vfile_names.at(domain_num - CSP_LOG_TYPE_VFILE_HEAD);
		}
		if(!domain_s.isEmpty()) {
			domain_s = QString::fromUtf8("[") + domain_s + QString::fromUtf8("]");
		}
		if(!log_cons || !log_cons_out) cons_log_level_n = 0;
		if(!syslog_flag || !syslog_flag_out) sys_log_level_n = 0;
		
		do {
			if(p != NULL) {
				CSP_LoggerLine *tmps;
				if(linenum == LLONG_MAX) line_wrap++;
				tmps = new CSP_LoggerLine(linenum++, level, domain_s, time_s, QString::fromUtf8(p));
				squeue.enqueue(tmps);
				if(log_onoff) {
					if(cons_log_level_n != 0) {
						fprintf(stdout, "%s : %s\n",
								log_sysname.toLocal8Bit().constData(),
								tmps->get_element_console().toLocal8Bit().constData());
					}
#if !defined(Q_OS_WIN)   
					if(sys_log_level_n != 0) {
						syslog(level_flag, "uS=%06ld %s",
							   tv.tv_usec,
							   tmps->get_element_syslog().toLocal8Bit().constData());
					}
#endif
				}
#ifdef __MINGW32__
				p = strtok(NULL, delim);
#else
				p = strtok_r(NULL, delim, &p_bak);
#endif
			}
#if defined(Q_OS_WIN)
			fflush(stdout);
#endif			
		} while(p != NULL);
	}
}

void CSP_Logger::set_log_status(bool sw)
{
	log_onoff = sw;
}
   
void CSP_Logger::set_log_stdout(int level, bool sw)
{
	if((level < 0) || (level >= 32)) {
		log_cons_out = sw;
		return;
	}
	int lv = 1 << level;
	if(sw) {
		cons_log_levels |= lv;
	} else {
		cons_log_levels &= ~lv;
	}
}

void CSP_Logger::set_log_syslog(int level, bool sw)
{
	if((level < 0) || (level >= 32)) {
		syslog_flag_out = sw;
		return;
	}
	int lv = 1 << level;
	if(sw) {
		sys_log_levels |= lv;
	} else {
		sys_log_levels &= ~lv;
	}
}

bool CSP_Logger::get_status(void)
{
	return log_opened;
}
   
	
void CSP_Logger::close(void)
{
	QMutexLocker locker(lock_mutex);

	log_opened = false;
#if !defined(Q_OS_WIN32)   
	if(syslog_flag != 0) {
	     closelog();
	}
#endif
	syslog_flag = false;
	syslog_flag_out = false;
	
	log_cons = false;
	log_cons_out = false;
	
	log_onoff = false;
	
	while(!squeue.isEmpty()) {
		CSP_LoggerLine *p = squeue.dequeue();
		if(p != NULL) delete p;
	}
	loglist.clear();
	log_sysname.clear();
	squeue.clear();
}

void CSP_Logger::set_emu_vm_name(const char *devname)
{
	QString dname;
	if(devname != NULL) {
		dname = QString::fromUtf8(devname);
	} else {
		dname = QString::fromUtf8("*Undefined*");
	}
	log_sysname = QString::fromUtf8("Common Source Code Project(");
	log_sysname = log_sysname + dname + QString::fromUtf8(")");
}

void CSP_Logger::set_device_name(int num, char *devname)
{
	if(num < 0) return;
	if(num > (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0)) return;
	if(devname == NULL) return;
	QString tmps = QString::fromUtf8(devname);
	device_names.replace(num, tmps);
}

void CSP_Logger::set_cpu_name(int num, char *devname)
{
	if(num < 0) return;
	if(num > (CSP_LOG_TYPE_VM_CPU7 - CSP_LOG_TYPE_VM_CPU0)) return;
	if(devname == NULL) return;
	QString tmps = QString::fromUtf8(devname);
	device_names.replace(num, tmps);
}
