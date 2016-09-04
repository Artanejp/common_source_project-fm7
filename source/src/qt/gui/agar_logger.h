/*
 * Log functions
 * (C) 2014-06-30 K.Ohta
 * 
 * History:
 *  Dec 30, 2014 Move from XM7/SDL, this was Ohta's original code.
 * Licence : GPLv2
 */
#ifndef _CSP_LOGGER_H
#define _CSP_LOGGER_H

#include <stdarg.h>
#include <stdio.h>
#include <QObject>
#include <QStringList>
#include <QString>
#include <QQueue>

#if !defined(Q_OS_WIN32)
#  include <syslog.h>
#endif
#include <time.h>
#include <sys/time.h>

#include "simd_types.h"
#include "common.h"

#define CSP_LOG_ON 1
#define CSP_LOG_OFF 0
   
#define CSP_LOG_DEBUG 0
#define CSP_LOG_INFO 1
#define CSP_LOG_WARN 2
#define CSP_LOG_DEBUG2 3

#define CSP_LOG_LEVELS 8

enum {
	CSP_LOG_TYPE_UNDEFINED = 0,
	CSP_LOG_TYPE_GENERAL = 1,
	CSP_LOG_TYPE_OSD,
	CSP_LOG_TYPE_EMU,
	CSP_LOG_TYPE_VM,
	CSP_LOG_TYPE_GUI,
	CSP_LOG_TYPE_SOUND,
	CSP_LOG_TYPE_VIDEO,
	CSP_LOG_TYPE_KEYBOARD,
	CSP_LOG_TYPE_MOUSE,
	CSP_LOG_TYPE_JOYSTICK,
	CSP_LOG_TYPE_MOVIE_LOADER,
	CSP_LOG_TYPE_MOVIE_SAVER,
	CSP_LOG_TYPE_SCREEN,
	CSP_LOG_TYPE_PRINTER,
	CSP_LOG_TYPE_SOCKET,
	CSP_LOG_TYPE_EVENT,
	CSP_LOG_TYPE_COMPONENT_END,
	CSP_LOG_TYPE_VM_CPU0 = 32,
	CSP_LOG_TYPE_VM_CPU1,
	CSP_LOG_TYPE_VM_CPU2,
	CSP_LOG_TYPE_VM_CPU3,
	CSP_LOG_TYPE_VM_CPU4,
	CSP_LOG_TYPE_VM_CPU5,
	CSP_LOG_TYPE_VM_CPU6,
	CSP_LOG_TYPE_VM_CPU7,
	CSP_LOG_TYPE_VM_DEVICE_0,
	CSP_LOG_TYPE_VM_DEVICE_END = 255,
	CSP_LOG_TYPE_VFILE_HEAD = 256,
	CSP_LOG_TYPE_VFILE_BINARY = 256,
	CSP_LOG_TYPE_VFILE_BUBBLE = 264,
	CSP_LOG_TYPE_VFILE_CART = 272,
	CSP_LOG_TYPE_VFILE_CMT = 280,
	CSP_LOG_TYPE_VFILE_COMPACTDISC = 288,
	CSP_LOG_TYPE_VFILE_FLOPPY = 296,
	CSP_LOG_TYPE_VFILE_LASERDISC = 312,
	CSP_LOG_TYPE_VFILE_QUICKDISK = 320,
	CSP_LOG_TYPE_VFILE_END = 336,

};
	
	
	
QT_BEGIN_NAMESPACE

class CSP_LoggerLine {
private:
	int64_t linenum;
	int level;
	QString domain;
	QString mainstr;
	QString timestamp;
public:
	CSP_LoggerLine(int64_t line, int _level, QString _domain, QString time_s, QString s) {
		mainstr = s;
		linenum = line;
		level = _level;
		domain = _domain;
		timestamp = time_s;
	};
	~CSP_LoggerLine() {};
	int64_t get_line_num(void) {
		return linenum;
	}
	QString get_domain(void) {
		return domain;
	}
	QString get_element_syslog(void) {
		QString s;
		if(domain.isEmpty()) {
			s = mainstr;
		} else {
			s = domain + QString::fromUtf8(" ") + mainstr;
		}
		return s;
	};
	QString get_element_console(void) {
		QString s;
		if(domain.isEmpty()) {
			s = timestamp + QString::fromUtf8(" ") + mainstr;
		} else {
			s = timestamp + QString::fromUtf8(" ") + domain + QString::fromUtf8(" ") + mainstr;
		}
		return s;
	};
};

class QMutex;
class OSD;

class CSP_Logger: public QObject {
	Q_OBJECT
private:
	bool syslog_flag;
	bool syslog_flag_out;

	bool log_cons;
	bool log_cons_out;
	bool log_onoff;
	bool log_opened;

	int64_t linenum;
	int64_t line_wrap;
	int cons_log_levels;
	int sys_log_levels;
	
	QString loglist;
	QString log_sysname;
	QStringList component_names;
	QStringList vfile_names;
	QStringList cpu_names;
	QStringList device_names;

	// Device
	bool level_dev_out_record[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1][CSP_LOG_LEVELS]; // Record to log chain
	bool level_dev_out_syslog[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1][CSP_LOG_LEVELS]; // Syslog chain
	bool level_dev_out_console[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1][CSP_LOG_LEVELS]; // Console log chain

    // CPU
	bool level_cpu_out_record[8][CSP_LOG_LEVELS]; // Record to log chain
	bool level_cpu_out_syslog[8][CSP_LOG_LEVELS]; // Syslog chain
	bool level_cpu_out_console[8][CSP_LOG_LEVELS]; // Console log chain
	
	QQueue<CSP_LoggerLine *> squeue;
	QMutex *lock_mutex;
	OSD *p_osd;

	int max_devices;
	int max_cpus;
protected:
	void debug_log(int level, int domain_num, char *strbuf);

public:
	CSP_Logger(bool b_syslog, bool cons, const char *devname);
	~CSP_Logger();
	void set_osd(OSD *p) { p_osd = p; }
	void open(bool b_syslog, bool cons, const char *devname);
	void reset(void);
	void debug_log(int level, const char *fmt, ...);
	void debug_log(int level, int domain_num, const char *fmt, ...);
	void close(void);
	void set_log_status(bool sw);
	void set_log_syslog(int level, bool sw);
	void set_log_stdout(int level, bool sw);
	bool get_status(void);
	
	void set_emu_vm_name(const char *devname);
	void set_device_name(int num, char *devname);
	void set_cpu_name(int num, char *devname);
	void set_device_node_log(int device_id, int to_output, int type, bool flag);
	void set_device_node_log(int to_output, int type, bool* flags, int start, int devices);
	void set_device_node_log(int to_output, int type, int *flags, int start, int devices);
	void output_event_log(int device_id, int level, const char *fmt, ...);
	int64_t get_console_list(char *buffer, int64_t buf_size, bool utf8, char *domainname, bool forget, int64_t start, int64_t end);
	void clear_log(void);
};
QT_END_NAMESPACE

extern CSP_Logger *csp_logger;

#endif
