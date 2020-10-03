set(VMFILES_LIB
   i386.cpp
   noise.cpp
   pcm1bit.cpp
   i8251.cpp
   i8253.cpp
   i8259.cpp
   io.cpp
   upd71071.cpp
   mb8877.cpp
   
   scsi_dev.cpp
   scsi_hdd.cpp
   scsi_cdrom.cpp
   
   disk.cpp
   prnfile.cpp
   harddisk.cpp
)
set(VMFILES
  ${PROJECT_SOURCE_DIR}/src/vm/scsi_host.cpp
  ${PROJECT_SOURCE_DIR}/src/vm/msm58321.cpp
)

set(BUILD_FMTOWNS_1 ON CACHE BOOL "Build for FM-Towns Model 1")
#set(BUILD_FMTOWNS_2 ON CACHE BOOL "Build for FM-Towns Model 2")
#set(BUILD_FMTOWNS_2F ON CACHE BOOL "Build for FM-Towns 2F")
set(BUILD_FMTOWNS_2H ON CACHE BOOL "Build for FM-Towns 2H")
#set(BUILD_FMTOWNS_20F ON CACHE BOOL "Build for FM-Towns 20F")
#set(BUILD_FMTOWNS_20H ON CACHE BOOL "Build for FM-Towns 20H")
#set(BUILD_FMTOWNS2_UX20 ON CACHE BOOL "Build for FM-Towns2 UX20")
#set(BUILD_FMTOWNS2_UX40 ON CACHE BOOL "Build for FM-Towns2 UX40")
set(BUILD_FMTOWNS2_CX20 ON CACHE BOOL "Build for FM-Towns2 CX20")
set(BUILD_FMTOWNS2_CX40 ON CACHE BOOL "Build for FM-Towns2 CX40")
#set(BUILD_FMTOWNS2_CX100 ON CACHE BOOL "Build for FM-Towns2 CX100")
#set(BUILD_FMTOWNS2_UG10 ON CACHE BOOL "Build for FM-Towns2 UG10")
#set(BUILD_FMTOWNS2_UG20 ON CACHE BOOL "Build for FM-Towns2 UG20")
#set(BUILD_FMTOWNS2_UG40 ON CACHE BOOL "Build for FM-Towns2 UG40")
#set(BUILD_FMTOWNS2_UG80 ON CACHE BOOL "Build for FM-Towns2 UG80")
#set(BUILD_FMTOWNS2_HG20 ON CACHE BOOL "Build for FM-Towns2 HG20")
set(BUILD_FMTOWNS2_HG40 ON CACHE BOOL "Build for FM-Towns2 HG40")
#set(BUILD_FMTOWNS2_HG100 ON CACHE BOOL "Build for FM-Towns2 HG100")
#set(BUILD_FMTOWNS2_HR20 ON CACHE BOOL "Build for FM-Towns2 HR20")
set(BUILD_FMTOWNS2_HR100 ON CACHE BOOL "Build for FM-Towns2 HR100")
#set(BUILD_FMTOWNS2_HR200 ON CACHE BOOL "Build for FM-Towns2 HR200")

if(BUILD_FMTOWNS_1)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns _FMTOWNS_1)
  ## ToDo
endif()

if(BUILD_FMTOWNS_2)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns2 _FMTOWNS_2)
  ## ToDo
endif()
if(BUILD_FMTOWNS_1F)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns1F _FMTOWNS_1F)
  ## ToDo
endif()
if(BUILD_FMTOWNS_2F)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns2F _FMTOWNS_2F)
  ## ToDo
endif()
if(BUILD_FMTOWNS_1H)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns1H _FMTOWNS_1H)
  ## ToDo
endif()
if(BUILD_FMTOWNS_2H)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns2H _FMTOWNS_2H)
  ## ToDo
endif()

if(BUILD_FMTOWNS_10F)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns10F _FMTOWNS_10F)
  ## ToDo
endif()
if(BUILD_FMTOWNS_20F)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns20F _FMTOWNS_20F)
  ## ToDo
endif()
if(BUILD_FMTOWNS_10H)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns10H _FMTOWNS_10H)
  ## ToDo
endif()
if(BUILD_FMTOWNS_20H)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns20H _FMTOWNS_20H)
  ## ToDo
endif()

if(BUILD_FMTOWNS2_UX20)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns2UX20 _FMTOWNS2_UX20)
  ## ToDo
endif()
if(BUILD_FMTOWNS2_UX40)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns2UX40 _FMTOWNS2_UX40)
  ## ToDo
endif()
if(BUILD_FMTOWNS2_CX20)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns2CX20 _FMTOWNS2_CX20)
  ## ToDo
endif()
if(BUILD_FMTOWNS2_CX40)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns2CX40 _FMTOWNS2_CX40)
  ## ToDo
endif()
if(BUILD_FMTOWNS2_CX100)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns2CX100 _FMTOWNS2_CX100)
  ## ToDo
endif()

if(BUILD_FMTOWNS2_HR20)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns2HR20 _FMTOWNS2_HR20)
  ## ToDo
endif()
if(BUILD_FMTOWNS2_HR100)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns2HR100 _FMTOWNS2_HR100)
  ## ToDo
endif()
if(BUILD_FMTOWNS2_HR200)
  set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/fmtowns.qrc)
  ADD_VM(fmtowns emufmtowns2HR200 _FMTOWNS2_HR200)
  ## ToDo
endif()
