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
#include <QVector>
#include <QString>
#include <QQueue>
#include <QThread>
#include <QContiguousCache>

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
	CSP_LOG_TYPE_SOUND_LOADER,
	CSP_LOG_TYPE_GL_SHADER,
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
	CSP_LOG_TYPE_VM_STATE = 512,
	CSP_LOG_TYPE_END = 1023,
};

QT_BEGIN_NAMESPACE

class DLL_PREFIX CSP_LoggerLine {
private:
	int64_t linenum;
    double vm_usec;
	int level;
	QString domain;
	QString mainstr;
	QString timestamp;
public:
	CSP_LoggerLine(int64_t line, int _level, QString _domain, QString time_s, QString s, double us = 0.0);
	~CSP_LoggerLine();
	int64_t get_line_num(void);
	QString get_domain(void);
	QString get_element_syslog(void);
	QString get_element_console(void);
	bool check_level(QString _domain, int _level);
	bool contains(QString s, bool case_sensitive = false);
	bool contains_mainstr(QString s, bool case_sensitive = false);
	bool check_domain(QString s, bool case_sensitive = false);
};

class QMutex;
class OSD_BASE;

class DLL_PREFIX CSP_Log_ConsoleThread: public QThread {
	Q_OBJECT
	QContiguousCache<QString> conslog;
	//QQueue<QString> conslog;
	QMutex *_mutex;
public:
	CSP_Log_ConsoleThread(QObject *parent);
	~CSP_Log_ConsoleThread();
	void run() override;
public slots:
	void do_message(QString header, QString message);
	
};

class DLL_PREFIX CSP_Logger: public QObject {
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

	bool level_state_out_record;
	bool level_state_out_syslog;
	bool level_state_out_console;
	
	QString loglist;
	QString log_sysname;
	QStringList component_names;
	QStringList vfile_names;
	QStringList cpu_names;
	QStringList device_names;
	CSP_Log_ConsoleThread *console_printer;
	
	// Device
	bool level_dev_out_record[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1][CSP_LOG_LEVELS]; // Record to log chain
	bool level_dev_out_syslog[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1][CSP_LOG_LEVELS]; // Syslog chain
	bool level_dev_out_console[CSP_LOG_TYPE_VM_DEVICE_END - CSP_LOG_TYPE_VM_DEVICE_0 + 1][CSP_LOG_LEVELS]; // Console log chain

    // CPU
	bool level_cpu_out_record[8][CSP_LOG_LEVELS]; // Record to log chain
	bool level_cpu_out_syslog[8][CSP_LOG_LEVELS]; // Syslog chain
	bool level_cpu_out_console[8][CSP_LOG_LEVELS]; // Console log chain
	
	QVector<CSP_LoggerLine *> squeue;
	QMutex *lock_mutex;
	OSD_BASE *p_osd;

	int max_devices;
	int max_cpus;
protected:
	uint64_t get_vm_clocks();
	double get_vm_clocks_usec();
public:
	CSP_Logger(QObject *parent, bool b_syslog, bool cons, const char *devname);
	~CSP_Logger();
	void set_osd(OSD_BASE *p) { p_osd = p; }
	void open(bool b_syslog, bool cons, const char *devname);
	void reset(void);
	void debug_log(int level, const char *fmt, ...);
	void debug_log(int level, int domain_num, const char *fmt, ...);
	void debug_log(int level, int domain_num, char *strbuf);
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

	void set_state_log(int to_output, bool flag);

	void output_event_log(int device_id, int level, const char *fmt, ...);
	int64_t get_console_list(char *buffer, int64_t buf_size, bool utf8, char *domainname, bool forget, int64_t start = -1, int64_t end = -1, int64_t *end_line = 0);
	void clear_log(void);
	int64_t write_log(const _TCHAR *name, const char *domain_name = NULL, bool utf8 = true, bool forget = false);
	int64_t copy_log(char *buffer, int64_t buf_size, int64_t *lines = NULL, char *domainname = NULL, bool utf8 = true, bool forget = false, int64_t start = 0, int64_t start_size = 0, int64_t *end_line = 0);
	void *get_raw_data(bool forget = false, int64_t start = 0, int64_t *end_line = NULL);
public slots:
	void do_debug_log(int level, int domain_num, QString mes);
signals:
	int sig_console_message(QString, QString);
	int sig_console_quit();
};
QT_END_NAMESPACE

//extern CSP_Logger DLL_PREFIX *csp_logger;

#endif
