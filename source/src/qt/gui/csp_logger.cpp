/*
 * Log functions
 * (C) 2014-06-30 K.Ohta
 * History:
 *  Dec 30, 2014 Move from XM7/SDL, this was Ohta's original code.
 * Licence : GPLv2
 */

#include <QMutex>
#include <QMutexLocker>

#include "csp_logger.h"
#include <string.h>
#include "emu.h"
#include "vm/vm.h"
#include "menu_flags.h"
#include "../osd.h"

CSP_Logger::CSP_Logger(bool b_syslog, bool cons, const char *devname)
{
	lock_mutex = new QMutex(QMutex::Recursive);
	this->reset();
	this->open(b_syslog, cons, devname);
}


CSP_Logger::~CSP_Logger()
{
	loglist.clear();
	log_sysname.clear();
	squeue.clear();
	delete lock_mutex;
}

void CSP_Logger::reset(void)
{
	QString tmps;
	const char *p;

	QMutexLocker locker(lock_mutex);
	component_names.clear();
	vfile_names.clear();
	cpu_names.clear();
	device_names.clear();
	max_cpus = 0;
	max_devices = 0;
	
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
		"EVENT",
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
		for(int j = 0; j < CSP_LOG_LEVELS; j++) {
			level_cpu_out_record[i][j] = true;
			level_cpu_out_syslog[i][j] = false;
			level_cpu_out_console[i][j] = true;
		}
	}
	tmps = QString::fromUtf8("DEV"); // Enable to update
	for(int i = 0; i < (256 - (32 + 8)); i++) {
		QString ss;
		ss.setNum(i);
		device_names.append(tmps + ss);
		for(int j = 0; j < CSP_LOG_LEVELS; j++) {
			level_dev_out_record[i][j] = true;
			level_dev_out_syslog[i][j] = false;
			level_dev_out_console[i][j] = true;
		}
	}
}

//extern class USING_FLAGS *using_flags;
void CSP_Logger::open(bool b_syslog, bool cons, const char *devname)
{
	int flags = 0;
	
	QMutexLocker locker(lock_mutex);
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
	{
		QMutexLocker locker(lock_mutex);
#ifdef __MINGW32__
		p = strtok(strbuf, delim);
#else
		p = strtok_r(strbuf, delim, &p_bak); 
#endif
	}
	if(strbuf != NULL) {
		nowtime = time(NULL);
		gettimeofday(&tv, NULL);
		if(log_cons != 0) { // Print only
			timedat = localtime(&nowtime);
			strftime(strbuf2, 255, "%Y-%m-%d %H:%M:%S", timedat);
			snprintf(strbuf3, 23, ".%06ld", tv.tv_usec);
		}
		QString time_s = QString::fromUtf8(strbuf2) + QString::fromUtf8(strbuf3);
		
		int cons_log_level_n = (1 << level) & cons_log_levels;
		int sys_log_level_n = (1 << level) & sys_log_levels;
		QString domain_s;
		bool record_flag = true;
		
		domain_s.clear();
		if((domain_num > 0) && (domain_num < CSP_LOG_TYPE_COMPONENT_END)) {
			domain_s = component_names.at(domain_num - 1);
		} else if((domain_num >= CSP_LOG_TYPE_VM_CPU0) && (domain_num < CSP_LOG_TYPE_VM_DEVICE_0)) {
			domain_s = cpu_names.at(domain_num - CSP_LOG_TYPE_VM_CPU0);
			
			record_flag = level_cpu_out_record[domain_num - CSP_LOG_TYPE_VM_CPU0][level];
			if(!level_cpu_out_syslog[domain_num - CSP_LOG_TYPE_VM_CPU0][level]) sys_log_level_n = 0;
			if(!level_cpu_out_console[domain_num - CSP_LOG_TYPE_VM_CPU0][level]) cons_log_level_n = 0;
		} else if((domain_num >= CSP_LOG_TYPE_VM_DEVICE_0) && (domain_num <= CSP_LOG_TYPE_VM_DEVICE_END)) {
			domain_s = device_names.at(domain_num - CSP_LOG_TYPE_VM_DEVICE_0);
			
			record_flag = level_dev_out_record[domain_num - CSP_LOG_TYPE_VM_DEVICE_0][level];
			if(!level_dev_out_syslog[domain_num - CSP_LOG_TYPE_VM_DEVICE_0][level]) sys_log_level_n = 0;
			if(!level_dev_out_console[domain_num - CSP_LOG_TYPE_VM_DEVICE_0][level]) cons_log_level_n = 0;
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
				tmps = new CSP_LoggerLine(linenum, level, domain_s, time_s, QString::fromUtf8(p));
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
				{
					QMutexLocker locker(lock_mutex);
#ifdef __MINGW32__
					p = strtok(NULL, delim);
#else
					p = strtok_r(NULL, delim, &p_bak);
#endif
				}
				if(!record_flag) {
					delete tmps;
				} else {
					QMutexLocker locker(lock_mutex);
					squeue.enqueue(tmps);
					if(linenum == LLONG_MAX) {
						line_wrap++;
						linenum = 0;
					} else {
						linenum++;
					}
				}
			}
#if defined(Q_OS_WIN)
			{
				QMutexLocker locker(lock_mutex);
				fflush(stdout);
			}
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
	QString tmps;
	if(num < 0) return;
	if(num > (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0)) return;
	if(devname == NULL) {
		QString s;
		s.setNum(num);
		tmps = QString::fromUtf8("Device#");
		tmps = tmps + s;
	} else {
		tmps = QString::fromUtf8(devname);
	}
	device_names.replace(num, tmps);
	if(max_devices <= num) max_devices = num + 1;
}

void CSP_Logger::set_cpu_name(int num, char *devname)
{
	if(num < 0) return;
	if(num > (CSP_LOG_TYPE_VM_CPU7 - CSP_LOG_TYPE_VM_CPU0)) return;
	if(devname == NULL) return;
	QString tmps = QString::fromUtf8(devname);
	device_names.replace(num, tmps);
	if(max_cpus <= num) max_cpus = num + 1;
}

void CSP_Logger::set_device_node_log(int device_id, int to_output, int type, bool flag)
{
	if(type < 0) return;
	if(type >= CSP_LOG_LEVELS) return;
	if(to_output < 0) return;
	if(to_output > 2) return;
	if(device_id >= (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0)) return;
	if(device_id == -1) { // Flush all device
		for(int i = 0; i < (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1); i++) {
			// 0 = record, 1 = syslog, 2 = console;
			switch(to_output)
			{
			case 0:
				level_dev_out_record[i][type] = flag;
				break;
			case 1:
				level_dev_out_syslog[i][type] = flag;
				break;
			case 2:
				level_dev_out_console[i][type] = flag;
				break;
			default:
				break;
			}
		}
	} else {
		// 0 = record, 1 = syslog, 2 = console;
		switch(to_output)
		{
		case 0:
			level_dev_out_record[device_id][type] = flag;
			break;
		case 1:
			level_dev_out_syslog[device_id][type] = flag;
			break;
		case 2:
			level_dev_out_console[device_id][type] = flag;
			break;
		default:
			break;
		}
	}
}

void CSP_Logger::set_device_node_log(int to_output, int type, bool *flags, int start, int devices)
{
	if(flags == NULL) return;
	if(start < 0) return;

	int i;
	for(i = 0; i < devices; i++) {
		set_device_node_log(i + start, to_output, type, flags[i]);
	}
}

void CSP_Logger::set_device_node_log(int to_output, int type, int *flags, int start, int devices)
{
	if(flags == NULL) return;
	if(start < 0) return;
	
	int i;
	bool f;
	for(i = 0; i < devices; i++) {
		f = (flags[i] == 0) ? false : true;
		set_device_node_log(i + start, to_output, type, f);
	}
}

void CSP_Logger::output_event_log(int device_id, int level, const char *fmt, ...)
{
	char strbuf[4500];
	char strbuf2[4096];
	char *p = NULL;
	p = (char *)(device_names.at(device_id).toLocal8Bit().constData());
	
	va_list ap;
	va_start(ap, fmt);	
	vsnprintf(strbuf2, 4095, fmt, ap);
	snprintf(strbuf, 4500, "[%s] %s", p, strbuf2); 
	debug_log(level, CSP_LOG_TYPE_EVENT, strbuf);
	va_end(ap);
}

int64_t CSP_Logger::get_console_list(char *buffer, int64_t buf_size, bool utf8, char *domainname, bool forget, int64_t start, int64_t end, int64_t *end_line)
{
	if((buffer == NULL) || (buf_size <= 0)) return (int64_t)-1;

	QString dom;
	char tmpbuf[8192];
	bool not_match_domain = false;
	if(domainname == NULL) not_match_domain = true;
	if(!not_match_domain && (domainname != NULL)) dom = QString::fromUtf8(domainname);
	if(dom.isEmpty()) not_match_domain = true;

	int64_t total_size = 0;
	CSP_LoggerLine *t;
	QString tmps;
	char *pp = buffer;
	bool check_line = ((start >= 0) && (end >= start));
	int ssize;
	{
		QMutexLocker locker(lock_mutex);
		ssize=squeue.size();
	}
	for(int i = 0; i < ssize; i++) {
		if(forget) {
			QMutexLocker locker(lock_mutex);
			if(squeue.isEmpty()) break;
			t = squeue.dequeue();
		} else {
			QMutexLocker locker(lock_mutex);
			t = squeue.at(i);
		}
		if(t != NULL) {
			int64_t n_line = t->get_line_num();
			if(end_line != NULL) *end_line = n_line;
			if(check_line) {
				if((n_line < start) || (n_line >= end)) {
					if(forget) {
						QMutexLocker locker(lock_mutex);
						delete t;
					}
					continue;
				}
			}

			if(not_match_domain) {
				tmps = t->get_element_console();
			} else {
				if(dom == t->get_domain()) {
					tmps = t->get_element_console();
				} else {
					tmps.clear();
				}
			}
			if(!tmps.isEmpty()) {
				int l = 0;
				QByteArray ns;
				if(utf8) {
					ns = tmps.toUtf8();
				} else {
					ns = tmps.toLocal8Bit();
				}
				l = ns.size();
				if(l > 0) {
					memset(tmpbuf, 0x00, 8192);
					if(l >= 8192) l = 8192 -1;
					strncpy(tmpbuf, ns.constData(), l);
				}
				if(((int64_t)l + total_size) < buf_size) {
					strncpy(pp, tmpbuf, l);
					pp += l;
					total_size += (int64_t)l;
				} else {
					if(forget) {
						QMutexLocker locker(lock_mutex);
						delete t;
					}
					break;
				}
			}
			if(forget) {
				QMutexLocker locker(lock_mutex);
				delete t;
			}
		}
	}
	return total_size;
}

void CSP_Logger::clear_log(void)
{
	QMutexLocker locker(lock_mutex);
	while(!squeue.isEmpty()) {
		CSP_LoggerLine *p = squeue.dequeue();
		if(p != NULL) delete p;
	}
}

int64_t CSP_Logger::write_log(const _TCHAR *name, const char *domain_name, bool utf8, bool forget)
{
	int64_t n_len = (int64_t)-1;
	if(name == NULL) return n_len;
	
	FILEIO *fio = new FILEIO();
	if(fio == NULL) return n_len;
	
	if(!fio->Fopen(name, FILEIO_READ_WRITE_BINARY)) {
		delete fio;
		return n_len;
	}
	if(fio->FileLength() > 0) fio->Fseek(0, FILEIO_SEEK_END);
	n_len = 0;
	char strbuf[0x20000];
	int64_t len = 0;
	do {
		memset(strbuf, 0x00, sizeof(strbuf));
		len = get_console_list(strbuf, 0x20000, utf8, (char *)domain_name, forget);
		if(len > 0x20000) break; // Illegal
		if(len <= 0) break;
		if(fio->Fwrite(strbuf, (uint32_t)len, 1) != len) break;
		n_len += len;
	} while(len > 0);
	fio->Fclose();
	delete fio;
	return n_len;
}

int64_t CSP_Logger::copy_log(char *buffer, int64_t buf_size, int64_t *lines, char *domainname, bool utf8, bool forget, int64_t start, int64_t start_size, int64_t *end_line)
{
	if((buffer == NULL) || (buf_size <= 0)) return (int64_t)-1;
	int64_t line = start;
	int64_t size = 0;
	int64_t buf_left = buf_size;
	int64_t lines_t = 0;
	int64_t nlines = 0;
	int64_t ssize = start_size;
	if(ssize < 0) ssize = 0;
	if(start_size >= buf_size) return -1;
	
	char *p = &(buffer[start_size]);
	
	if(line < 0) line = 0;
	{
		QMutexLocker locker(lock_mutex);
		nlines = squeue.size();
	}
	for(; (buf_left > 0) && (nlines > 0);) {
		size = get_console_list(buffer, buf_left, utf8, domainname, forget, line, line + 1, end_line);
		if(size <= 0) break;
		buf_left -= size;
		ssize += size;
		p = &(p[size]);
		lines_t++;
		nlines--;
	}
	if(lines != NULL) *lines = *lines + lines_t;
	return ssize;
}

void *CSP_Logger::get_raw_data(bool forget, int64_t start, int64_t *end_line)
{
	QMutexLocker locker(lock_mutex);
	CSP_LoggerLine *t;
	int i;
	int64_t n = squeue.size();

	if(start < 0)  return (void *)NULL;
	if(start >= n) return (void *)NULL;
	if(forget) {
		t = squeue.dequeue();
	} else {
		t = squeue.at(start);
	}
	if(t != NULL) {
		if(end_line != NULL) *end_line = t->get_line_num();
		return (void *)t;
	}
	return (void *)NULL;
}
