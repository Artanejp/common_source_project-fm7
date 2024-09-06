set(BUILD_M5 ON CACHE BOOL "Build SORD M5.")
set(BUILD_M23 ON CACHE BOOL "Build SORD M23.")
set(BUILD_M68 ON CACHE BOOL "Build SORD M68.")

if(BUILD_M5)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/m5.qrc)
	ADD_VM(m5 emum5 _M5)
endif()

if(BUILD_M23)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/m23.qrc)
	ADD_VM(m23 emum23 _M23)
endif()

if(BUILD_M68)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/m68.qrc)
	ADD_VM(m23 emum68 _M68)
endif()

