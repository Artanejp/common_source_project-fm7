set(BUILD_MZ80A ON CACHE BOOL "Build for SHARP MZ-80A")
set(BUILD_MZ80K ON CACHE BOOL "Build for SHARP MZ-80K")
set(BUILD_MZ1200 ON CACHE BOOL "Build for SHARP MZ-1200")

set(BUILD_MZ80B ON CACHE BOOL "Build for SHARP MZ-80B")
#set(BUILD_MZ2000 ON CACHE BOOL "Build for SHARP MZ-2000")
set(BUILD_MZ2200 ON CACHE BOOL "Build for SHARP MZ-2200")
set(BUILD_MZ2500 ON CACHE BOOL "Build for SHARP MZ-2500")

set(BUILD_MZ700 ON CACHE BOOL "Build for SHARP MZ-700")
set(BUILD_MZ800 ON CACHE BOOL "Build for SHARP MZ-800")
set(BUILD_MZ1500 ON CACHE BOOL "Build for SHARP MZ-1500")

set(BUILD_MZ2800 ON CACHE BOOL "Build for SHARP MZ-2800")
set(BUILD_MZ3500 ON CACHE BOOL "Build for SHARP MZ-3500")

set(BUILD_MZ5500 ON CACHE BOOL "Build for SHARP MZ-5500")
set(BUILD_MZ6500 ON CACHE BOOL "Build for SHARP MZ-6500")
set(BUILD_MZ6550 ON CACHE BOOL "Build for SHARP MZ-6550")

if(BUILD_MZ80A)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz80a.qrc)
	ADD_VM(mz80k emumz80A _MZ80A)
	# MZ80AIF
endif()
if(BUILD_MZ80K)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz80k.qrc)
	ADD_VM(mz80k emumz80k _MZ80K)
	# MZ80AIF
endif()
if(BUILD_MZ1200)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz1200.qrc)
	ADD_VM(mz80k emumz1200 _MZ1200)
	# MZ80FIO
endif()

if(BUILD_MZ700)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz700.qrc)
	ADD_VM(mz700 emumz700 _MZ700)
endif()
if(BUILD_MZ800)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz800.qrc)
	ADD_VM(mz700 emumz800 _MZ800)
endif()
if(BUILD_MZ1500)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz1500.qrc)
	ADD_VM(mz700 emumz1500 _MZ1500)
endif()

if(BUILD_MZ80B)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz80b.qrc)
	ADD_VM(mz2500 emumz80b _MZ80B)
endif()
if(BUILD_MZ2000)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz2000.qrc)
	ADD_VM(mz2500 emumz2000 _MZ2000)
endif()
if(BUILD_MZ2200)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz2200.qrc)
	ADD_VM(mz2500 emumz2200 _MZ2200)
endif()

if(BUILD_MZ2500)
	set(USE_SOCKET_emumz2500 ON)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz2500.qrc)
	ADD_VM(mz2500 emumz2500 _MZ2500)
	# Set soicket flags
	# ToDo: To be separated.
	target_compile_definitions(common_emumz2500
		PRIVATE  ${USE_SOCKET}
	)
	target_compile_definitions(qt_emumz2500
		PUBLIC ${USE_SOCKET}
	)
	target_compile_definitions(emumz2500
		PUBLIC ${USE_SOCKET}
	)

endif()

if(BUILD_MZ2800)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz2800.qrc)
	ADD_VM(mz2800 emumz2800 _MZ2800)
endif()

if(BUILD_MZ3500)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz3500.qrc)
	ADD_VM(mz3500 emumz3500 _MZ3500)
endif()

if(BUILD_MZ5500)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz5500.qrc)
	ADD_VM(mz5500 emumz5500 _MZ5500)
endif()
if(BUILD_MZ6500)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz6500.qrc)
	ADD_VM(mz5500 emumz6500 _MZ6500)
endif()
if(BUILD_MZ6550)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mz6550.qrc)
	ADD_VM(mz5500 emumz6550 _MZ6550)
endif()

