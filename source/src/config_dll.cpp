/*
  Note: This file is only for DLL.
	Author : Kyuma.Ohta <whatisthis.sowhat@gmail.com>
	Date   : 2020.10.11 -
  */
#if defined(_USE_QT)
//#include <string>
//#include <vector>

#include "csp_logger.h"
#include "qt_main.h"
# if defined(Q_OS_WIN)
# include <windows.h>
# endif
extern CSP_Logger *csp_logger;
#endif

//#include <stdlib.h>
//#include <stdio.h>
#include "common.h"
#include "fileio.h"
#include "config.h"

config_t DLL_PREFIX config;
