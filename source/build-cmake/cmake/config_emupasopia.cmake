# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

message("")
message("** Start of configure CommonSourceProject,PASOPIA/7, Qt **")
message("")


set(VMFILES
		   z80.cpp

		   hd46505.cpp
		   i8255.cpp
		   
		   ls393.cpp
		   not.cpp

		   z80ctc.cpp
		   z80pio.cpp
		   upd765a.cpp
		   
		   pcm1bit.cpp
		   datarec.cpp
		   io.cpp
		   event.cpp
		   disk.cpp
)

if(NOT BUILD_PASOPIA)
 set(BUILD_PASOPIA OFF CACHE BOOL "Build for PASOPIA")
endif()

if(NOT BUILD_PASOPIA_LCD)
 set(BUILD_PASOPIA_LCD OFF CACHE BOOL "Build for PASOPIA with LCD")
endif()

if(NOT BUILD_PASOPIA7)
 set(BUILD_PASOPIA7 OFF CACHE BOOL "Build for PASOPIA7")
endif()

if(NOT BUILD_PASOPIA7_LCD)
 set(BUILD_PASOPIA7_LCD OFF CACHE BOOL "Build for PASOPIA7 with LCD")
endif()

set(BUILD_SHARED_LIBS OFF)
set(USE_CMT_SOUND ON CACHE BOOL "Sound with Data Recorder.")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/machines/pasopia)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
  
if(BUILD_PASOPIA)
  set(EXEC_TARGET emupasopia)
  add_definitions(-D_PASOPIA)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pasopia.qrc)

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pasopia)
  set(LOCAL_LIBS   
		   qt_gui
		   qt_osd
		   qt_debugger
		   qt_pasopia
		   vm_pasopia
		   vm_vm
		   common_common
                  )
  
elseif(BUILD_PASOPIA_LCD)
  set(EXEC_TARGET emupasopia_lcd)
  add_definitions(-D_PASOPIA)
  add_definitions(-D_LCD)
  
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pasopia_lcd.qrc)
  set(LOCAL_LIBS ${LOCAL_LIBS} vm_pasopia)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pasopia)
  set(LOCAL_LIBS   
		   qt_gui
		   qt_osd
		   qt_debugger
		   qt_pasopia
		   vm_pasopia
		   vm_vm
		   common_common
                  )

elseif(BUILD_PASOPIA7)
  set(EXEC_TARGET emupasopia7)
  add_definitions(-D_PASOPIA7)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pasopia7.qrc)
  set(LOCAL_LIBS ${LOCAL_LIBS} vm_pasopia7)
  set(VMFILES ${VMFILES} sn76489an.cpp)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pasopia7)
  set(LOCAL_LIBS   
		   qt_gui
		   qt_osd
		   qt_debugger
		   qt_pasopia
		   vm_pasopia7
		   vm_vm
		   common_common
                  )

elseif(BUILD_PASOPIA7_LCD)
  set(EXEC_TARGET emupasopia7_lcd)
  add_definitions(-D_PASOPIA7)
  add_definitions(-D_LCD)
  
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pasopia7_lcd.qrc)
  set(LOCAL_LIBS ${LOCAL_LIBS} vm_pasopia7)
  set(VMFILES ${VMFILES} sn76489an.cpp)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pasopia7)
  set(LOCAL_LIBS   
		   qt_gui
		   qt_osd
		   qt_debugger
		   qt_pasopia
		   vm_pasopia7
		   vm_vm
		   common_common
                  )

endif()
 			   

if(USE_CMT_SOUND)
  add_definitions(-DDATAREC_SOUND)
endif()




