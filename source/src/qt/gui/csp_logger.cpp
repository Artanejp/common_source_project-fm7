/*
 * Log functions
 * (C) 2014-06-30 K.Ohta
 * History:
 *  Dec 30, 2014 Move from XM7/SDL, this was Ohta's original code.
 * Licence : GPLv2
 */
#include <QObject>

#if QT_VERSION >= 0x051400
	#include <QRecursiveMutex>
#else
	#include <QMutex>
#endif

#include <QMutexLocker>

#include "csp_logger.h"
#include <string.h>
#include "emu_template.h"
#include "../../vm/vm_template.h"
#include "menu_flags.h"
#include "../osd_base.h"

CSP_LoggerLine::CSP_LoggerLine(int64_t line, int _level, QString _domain, QString time_s, QString s, double us)
{
		mainstr = s;
		linenum = line;
		level = _level;
		domain = _domain;
		timestamp = time_s;
		vm_usec = us;
}

CSP_LoggerLine::~CSP_LoggerLine()
{
}

int64_t CSP_LoggerLine::get_line_num(void)
{
	return linenum;
}

QString CSP_LoggerLine::get_domain(void)
{
	return domain;
}

QString CSP_LoggerLine::get_element_syslog(void)
{
	QString s;
	_TCHAR secstr[64] = {0};
	my_stprintf_s(secstr, 63, _T(" (%6.7fSec) "), vm_usec / 1.0e6);
	if(domain.isEmpty()) {
		s = timestamp + QString::fromUtf8(secstr) + mainstr;
	} else {
		s = timestamp + QString::fromUtf8(secstr) + domain + QString::fromUtf8(" ")  + mainstr;
	}
	return s;
}

QString CSP_LoggerLine::get_element_console(void)
{
	QString s;
	_TCHAR secstr[64] = {0};
	my_stprintf_s(secstr, 63, _T(" (%6.7fSec) "), vm_usec / 1.0e6);
	if(domain.isEmpty()) {
		s = timestamp + QString::fromUtf8(secstr) +  mainstr;
	} else {
		s = timestamp + QString::fromUtf8(secstr) + domain + QString::fromUtf8(" ") +  mainstr;
	}
	return s;
}

bool CSP_LoggerLine::check_level(QString _domain, int _level)
{
	bool f = true;
	if(!_domain.isEmpty()) {
		if(_domain != domain) f = false;
	}
	if(_level >= 0) {
		if(_level != level) {
			f = false;
		}
	}
	return f;
}

bool CSP_LoggerLine::contains(QString s, bool case_sensitive)
{
	if(!(check_domain(s, case_sensitive))) {
		return contains_mainstr(s, case_sensitive);
	}
	return true;
}

bool CSP_LoggerLine::check_domain(QString s, bool case_sensitive)
{
	Qt::CaseSensitivity _n = (case_sensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
	if(domain.contains(s, _n)) return true;
	return false;
}

bool CSP_LoggerLine::contains_mainstr(QString s, bool case_sensitive)
{
	Qt::CaseSensitivity _n = (case_sensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
	if(mainstr.contains(s, _n)) return true;
	return false;
}

CSP_Logger::CSP_Logger(QObject *parent, bool b_syslog, bool cons, const char *devname) : QObject(parent)
{
	p_osd = NULL;
#if QT_VERSION >= 0x051400
	lock_mutex = new QRecursiveMutex();
#else
	lock_mutex = new QMutex(QMutex::Recursive);
#endif
	this->reset();
	this->open(b_syslog, cons, devname);
	level_state_out_record = false;
	level_state_out_syslog = false;
	level_state_out_console = false;

	console_printer = new CSP_Log_ConsoleThread(nullptr);
	if(console_printer != nullptr) {
		console_printer->setObjectName(QString::fromUtf8("Console_Logger_To_Stdout"));
		//console_printer->moveToThread(console_printer);
		connect(this, SIGNAL(sig_console_message(QString, QString)), console_printer, SLOT(do_message(QString, QString)));
		connect(this, SIGNAL(sig_console_quit()), console_printer, SLOT(quit()));
		//connect(console_printer, SIGNAL(finished()), console_printer, SLOT(deleteLater()));
		console_printer->start(QThread::LowPriority);
	}
}


CSP_Logger::~CSP_Logger()
{
	QMutexLocker locker(lock_mutex);
	emit sig_console_quit();
	if(console_printer != nullptr) {
		console_printer->wait();
	}
	delete console_printer;
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
		"SOUND_LOADER",
		"GL_SHADER",
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
	for(int i = 0; i < (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1); i++) {
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
	log_sysname = QString::fromUtf8("CSP(");
	log_sysname = log_sysname + dname + QString::fromUtf8(")");

	if(b_syslog) {
#if !defined(Q_OS_WIN)
		syslog_flag = true;
		if(cons) {
			flags = LOG_CONS;
		}
		//openlog(devname, flags | LOG_PID | LOG_NOWAIT, LOG_USER);
		openlog(devname, flags | LOG_PID , LOG_USER);
#else
		syslog_flag = false;
#endif
	} else {
		syslog_flag = false;
	}
	log_cons = cons;
	log_opened = true;
	log_cons_out = log_cons;
	syslog_flag_out = syslog_flag;

	cons_log_levels = 1 << CSP_LOG_INFO;

#if !defined(Q_OS_WIN)
	sys_log_levels = 1 << CSP_LOG_INFO;
	sys_log_levels |= (1 << CSP_LOG_DEBUG);
	sys_log_levels |= (1 << CSP_LOG_WARN);
#else
	sys_log_levels = 0;
#endif
	linenum = 1;
	line_wrap = 0;

	this->debug_log(CSP_LOG_INFO, "Start logging.");
}

void CSP_Logger::debug_log(int level, const char *fmt, ...)
{
	QMutexLocker locker(lock_mutex);
	char strbuf[4096];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(strbuf, 4095, fmt, ap);
	debug_log(level, 0, strbuf);
	va_end(ap);
}


void CSP_Logger::debug_log(int level, int domain_num, const char *fmt, ...)
{
	QMutexLocker locker(lock_mutex);
	char strbuf[4096];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(strbuf, 4095, fmt, ap);
	debug_log(level, domain_num, strbuf);
	va_end(ap);
}

void CSP_Logger::do_debug_log(int level, int domain_num, QString mes)
{
	debug_log(level, domain_num, (char *)mes.toUtf8().constData());
}

void CSP_Logger::debug_log(int level, int domain_num, char *strbuf)
{
	struct tm *timedat;
	time_t nowtime;
	char strbuf2[256];
	char strbuf3[24];
	struct timeval tv;

	if(!log_opened) return;
	if(strbuf == nullptr) return;
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
	char *p = strbuf;
	char *p_bak;
	const char delim[2] = "\n";

	QMutexLocker locker(lock_mutex);
	{
#ifdef __MINGW32__
		p = strtok(strbuf, delim);
#else
		p = strtok_r(strbuf, delim, &p_bak);
#endif
	}
	if(strbuf != NULL) {
		nowtime = time(NULL);
		gettimeofday(&tv, NULL);
		memset(strbuf2, 0x00, sizeof(strbuf2));
		memset(strbuf3, 0x00, sizeof(strbuf3));
		timedat = localtime(&nowtime);
		strftime(strbuf2, 255, "%Y-%m-%d %H:%M:%S", timedat);
		snprintf(strbuf3, 23, ".%06ld", tv.tv_usec);

		QString time_s = QString::fromLocal8Bit(strbuf2) + QString::fromLocal8Bit(strbuf3);

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
		} else if(domain_num == CSP_LOG_TYPE_VM_STATE) {
			domain_s = QString::fromUtf8("STATE");
			record_flag = level_state_out_record;
			if(!level_state_out_syslog) sys_log_level_n = 0;
			if(!level_state_out_console) cons_log_level_n = 0;
		}
		if(!domain_s.isEmpty()) {
			domain_s = QString::fromUtf8("[") + domain_s + QString::fromUtf8("]");
		}
		if(!log_cons || !log_cons_out) cons_log_level_n = 0;
		if(!syslog_flag || !syslog_flag_out) sys_log_level_n = 0;

		do {
			if(p != NULL) {
				std::shared_ptr<CSP_LoggerLine> tmps =
					std::make_shared<CSP_LoggerLine>(linenum, level, domain_s, time_s, QString::fromUtf8(p), get_vm_clocks_usec());
				//tmps = new CSP_LoggerLine(linenum, level, domain_s, time_s, QString::fromLocal8Bit(p));
				if(tmps.get() != nullptr) {
					if(log_onoff) {
						if(cons_log_level_n != 0) {
							emit sig_console_message(log_sysname, tmps->get_element_console());
						}
#if !defined(Q_OS_WIN)
						if(sys_log_level_n != 0) {
							syslog(level_flag, "%s",
								   tmps->get_element_syslog().toLocal8Bit().constData());
						}
#endif

					}
					{
#ifdef __MINGW32__
						p = strtok(NULL, delim);
#else
						p = strtok_r(NULL, delim, &p_bak);
#endif
					}
					if(!record_flag) {
						tmps.reset();
					} else {
						//squeue.enqueue(tmps);
						squeue.push_back(tmps);
						if(linenum == LLONG_MAX) {
							line_wrap++;
							linenum = 0;
						} else {
							linenum++;
						}
					}
					//if(tmps != NULL) delete tmps;
				}
#if defined(Q_OS_WIN)
				{
					fflush(stdout);
				}
#endif
			}
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
#if !defined(Q_OS_WIN)
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
#endif
}

bool CSP_Logger::get_status(void)
{
	return log_opened;
}


void CSP_Logger::close(void)
{
	QMutexLocker locker(lock_mutex);

	log_opened = false;
#if !defined(Q_OS_WIN)
	if(syslog_flag != 0) {
	     closelog();
	}
#endif
	syslog_flag = false;
	syslog_flag_out = false;

	log_cons = false;
	log_cons_out = false;

	log_onoff = false;

//	while(!squeue.isEmpty()) {
//		CSP_LoggerLine *p = squeue.dequeue();
//		if(p != NULL) delete p;
//	}
	loglist.clear();
	log_sysname.clear();
	clear_log();
}

void CSP_Logger::set_emu_vm_name(const char *devname)
{
	QString dname;
	if(devname != NULL) {
		dname = QString::fromUtf8(devname);
	} else {
		dname = QString::fromUtf8("*Undefined*");
	}
	log_sysname = QString::fromUtf8("CSP(");
	log_sysname = log_sysname + dname + QString::fromUtf8(")");
}

void CSP_Logger::do_set_device_name(int num, QString devname)
{
	if(num < 0) return;
	if(num > (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0)) return;

	QMutexLocker locker(lock_mutex);

	if(devname.isEmpty()) {
		QString s;
		s.setNum(num);
		QString tmps = QString::fromUtf8("Device#");
		tmps = tmps + s;
		device_names.replace(num, tmps);
	} else {
		device_names.replace(num, devname);
	}

	if(max_devices <= num) max_devices = num + 1;
}

void CSP_Logger::set_device_name(int num, char *devname)
{
	QString tmps;
	if(devname == nullptr) {
		do_set_device_name(num, QString::fromUtf8(""));
	} else {
		do_set_device_name(num, QString::fromUtf8(devname));
	}
}

void CSP_Logger::set_cpu_name(int num, char *devname)
{
	if(devname == nullptr) return;
	do_set_cpu_name(num, QString::fromUtf8(devname));
}

void CSP_Logger::do_set_cpu_name(int num, QString devname)
{
	if(num < 0) return;
	if(num > (CSP_LOG_TYPE_VM_CPU7 - CSP_LOG_TYPE_VM_CPU0)) return;

	QMutexLocker locker(lock_mutex);
	device_names.replace(num, devname);
	if(max_cpus <= num) max_cpus = num + 1;
}

void CSP_Logger::set_state_log(int to_output, bool flag)
{
	if(to_output < 0) return;
	if(to_output > 2) return;
	QMutexLocker locker(lock_mutex);
	switch(to_output)
	{
	case 0:
		level_state_out_record = flag;
		break;
	case 1:
		level_state_out_syslog = flag;
		break;
	case 2:
		level_state_out_console = flag;
		break;
	default:
		break;
	}
}
void CSP_Logger::set_device_node_log(int device_id, int to_output, int type, bool flag)
{
	if(type < 0) return;
	if(type >= CSP_LOG_LEVELS) return;
	if(to_output < 0) return;
	if(to_output > 2) return;
	if(device_id > (CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0)) return;
	QMutexLocker locker(lock_mutex);
	if(device_id < 0) { // Flush all device
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
	std::shared_ptr<CSP_LoggerLine> t;
	QString tmps;
	char *pp = buffer;
	bool check_line = ((start >= 0) && (end >= start));
	int ssize;
	int ipos = 0;
	for(auto p = squeue.begin(); p != squeue.end(); ++p) {
		{
			QMutexLocker locker(lock_mutex);
			t = (*p);
		}
		if(t.get() != NULL) {
			int64_t n_line = t->get_line_num();
			if(end_line != NULL) *end_line = n_line;
			if(check_line) {
				if((n_line < start) || (n_line >= end)) {
					if(forget) {
						QMutexLocker locker(lock_mutex);
						squeue.removeAt(ipos);
					}
					ipos++;
					continue;
				}
			}
			ipos++;
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
					if(l != 0) strncpy(tmpbuf, ns.constData(), l);
				}
				if(((int64_t)l + total_size) < buf_size) {
					if(l != 0)strncpy(pp, tmpbuf, l);
					pp += l;
					total_size += (int64_t)l;
				} else {
					if(forget) {
						QMutexLocker locker(lock_mutex);
						squeue.erase(p);
					}
					break;
				}
			}
			if(forget) {
				QMutexLocker locker(lock_mutex);
				squeue.erase(p);
			}
		}
	}
	return total_size;
}

void CSP_Logger::clear_log(void)
{
	QMutexLocker locker(lock_mutex);
//	for(auto p = squeue.begin(); p != squeue.end(); ++p) {
//		(*p).reset();
//	}
	squeue.clear();
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
		if((int64_t)(fio->Fwrite(strbuf, (uint32_t)len, 1)) != len) break;
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

std::shared_ptr<CSP_LoggerLine> CSP_Logger::get_raw_data(bool forget, int64_t start, int64_t *end_line)
{
	QMutexLocker locker(lock_mutex);
	int64_t n = squeue.size();
	std::shared_ptr<CSP_LoggerLine> t;
	t.reset();
	
	if((start >= 0) && (start < n)) {
		t = squeue.at(start);
		if(forget) {
			squeue.removeAt(start);
		}
		if(t.get() != NULL) {
			if(end_line != NULL) *end_line = t->get_line_num();
		}
	}
	return t;
}

uint64_t CSP_Logger::get_vm_clocks()
{
	if(p_osd == NULL) return (uint64_t)0;
	return p_osd->get_vm_current_clock_uint64();
}

double CSP_Logger::get_vm_clocks_usec()
{
	if(p_osd == NULL) return 0.0;
	return p_osd->get_vm_current_usec();
}

#include <QTimer>
#include <QElapsedTimer>

CSP_Log_ConsoleThread::CSP_Log_ConsoleThread(QObject *parent) : QThread(parent)
{
	tick_timer = new QTimer(this);
	conslog.setCapacity(1024);
	if(tick_timer != nullptr) {
		tick_timer->setTimerType(Qt::CoarseTimer);
		tick_timer->setInterval(25); // 25mSec
		connect(this, SIGNAL(started()), tick_timer, SLOT(start()));
		connect(this, SIGNAL(finished()), tick_timer, SLOT(stop()));
		connect(tick_timer, SIGNAL(timeout()), this, SLOT(do_print()));
	}
}

CSP_Log_ConsoleThread::~CSP_Log_ConsoleThread()
{
	conslog.clear();
}

void CSP_Log_ConsoleThread::do_message(QString header, QString message)
{
	QString tmps = header;
	tmps = tmps + QString::fromUtf8(" : ") + message;
	conslog.append(tmps);
}

void CSP_Log_ConsoleThread::do_print()
{
	//int leftline = 40;
	QString tmps;
	QElapsedTimer tim;
	tim.start();
	while(!(conslog.isEmpty())) {
		tmps = conslog.takeFirst();
		fprintf(stdout, "%s\n", tmps.toLocal8Bit().constData());
		if(tim.elapsed() >= 20) break; // 20mSec
	}
}


#if defined(CSP_OS_WINDOWS)
std::shared_ptr<CSP_Logger> DLL_PREFIX csp_logger;
#endif
