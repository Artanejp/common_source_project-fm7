# Set configuration for building XM7/SDL.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

include(CheckFunctionExists)

if(USE_DEVICES_SHARED_LIB)
  add_definitions(-DUSE_DEVICES_SHARED_LIB)
  set(I386_CPPS
	libcpu_newdev/i386.cpp
	libcpu_newdev/libcpu_i386/i386_real.cpp
	libcpu_newdev/libcpu_i386/i386op16_real.cpp
	libcpu_newdev/libcpu_i386/i386dasm.cpp
	)
  set(MC6809_CPPS 
	libcpu_newdev/libcpu_mc6809/mc6809.cpp
  )
else()
  set(I386_CPPS i386.cpp)
  set(MC6809_CPPS mc6809.cpp)
  set(VMFILES ${VMFILES} ${VMFILES_LIB})
endif()

if(FLAG_USE_I86)
  set(VMFILES ${VMFILES} i86.cpp)
endif()
if(FLAG_USE_I286)
  set(VMFILES ${VMFILES} i286.cpp)
endif()
if(FLAG_USE_I386_VARIANTS)
  set(VMFILES ${VMFILES} ${I386_CPPS})
endif()
if(FLAG_USE_Z80)
  set(VMFILES ${VMFILES} z80.cpp)
endif()
if(FLAG_USE_MC6809)
  set(VMFILES ${VMFILES} ${MC6809_CPPS})
endif()

if(USE_DEVICES_SHARED_LIB)
  set(VMFILES ${VMFILES}   libcpu_newdev/device.cpp)
endif()

if(DEFINED QT5_ROOT_PATH)
  SET(CMAKE_FIND_ROOT_PATH  ${QT5_ROOT_PATH} ${CMAKE_FIND_ROOT_PATH})
endif()

# Use cmake if enabled.
  find_program(USE_CCACHE ccache)
  if(USE_CCACHE)
   SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
   SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
  endif()
  FIND_PACKAGE(Qt5Widgets REQUIRED)
  FIND_PACKAGE(Qt5Core REQUIRED)
  FIND_PACKAGE(Qt5Gui REQUIRED)
  FIND_PACKAGE(Qt5OpenGL REQUIRED)
  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  include_directories(${Qt5Core_INCLUDE_DIRS})
  include_directories(${Qt5Gui_INCLUDE_DIRS})
  include_directories(${Qt5OpenGL_INCLUDE_DIRS})
  add_definitions(-D_USE_OPENGL -DUSE_OPENGL)
if(USE_SOCKET)
  FIND_PACKAGE(Qt5Network REQUIRED)
  include_directories(${Qt5Network_INCLUDE_DIRS})
endif()
SET(USE_QT_5 ON)
set(USE_QT5_4_APIS OFF CACHE BOOL "Build with Qt5.4 (or later) APIs if you can.")
set(USE_GCC_OLD_ABI ON CACHE BOOL "Build with older GCC ABIs if you can.")
set(USE_SDL2 ON CACHE BOOL "Build with libSDL2. DIsable is building with libSDL1.")
set(USE_MOVIE_SAVER OFF CACHE BOOL "Save screen/audio as MP4 MOVIE. Needs libav .")
set(USE_MOVIE_LOADER OFF CACHE BOOL "Load movie from screen for some VMs. Needs libav .")
set(USE_LTO ON CACHE BOOL "Use link-time-optimization to build.")
if(USE_LTO)
  # set_property(DIRECTORY PROPERTY INTERPROCEDURAL_OPTIMIZATION true)
else()
  # set_property(DIRECTORY PROPERTY INTERPROCEDURAL_OPTIMIZATION false)
endif()

add_definitions(-D_USE_QT5)

if(USE_QT5_4_APIS)
  add_definitions(-D_USE_QT_5_4)
else()
  #add_definitions(-DQT_NO_VERSION_TAGGING)
endif()

if(USE_GCC_OLD_ABI)
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
else()
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
endif()

SET(CMAKE_AUTOMOC OFF)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)


add_definitions(-D_USE_QT)
add_definitions(-DUSE_QT)
add_definitions(-DQT_MAJOR_VERSION=${Qt5Widgets_VERSION_MAJOR})
add_definitions(-DQT_MINOR_VERSION=${Qt5Widgets_VERSION_MINOR})

if(USE_OPENMP)
  find_package(OpenMP)
  include_directories(${OPENMP_INCLUDE_PATH})
endif()

find_package(Threads)
include_directories(${THREADS_INCLUDE_PATH})

include(FindPkgConfig)

include(FindLibAV)
    if(LIBAV_FOUND)
      add_definitions(-DUSE_LIBAV)
      if(USE_MOVIE_SAVER)
        add_definitions(-DUSE_MOVIE_SAVER)
      endif()
      if(USE_MOVIE_LOADER)
        add_definitions(-DUSE_MOVIE_LOADER)
      endif()
      add_definitions(-D__STDC_CONSTANT_MACROS)
      add_definitions(-D__STDC_FORMAT_MACROS)
    else()
      set(USE_MOVIE_SAVER OFF)
      set(USE_MOVIE_LOADER OFF)
      set(LIBAV_LIBRARIES "")
    endif()
    
if(USE_SDL2)
   if(CMAKE_CROSSCOMPILING)
      include_directories(${SDL2_INCLUDE_DIRS})
   else()
      pkg_search_module(SDL2 REQUIRED  sdl2)
      include_directories(${SDL2_INCLUDE_DIRS})
   endif()
   set(SDL_LIBS ${SDL2_LIBRARIES})
   add_definitions(-DUSE_SDL2)
else()
   if(CMAKE_CROSSCOMPILING)
      include_directories(${SDL_INCLUDE_DIRS})
      set(SDL_LIBS ${SDL_LIBRARIES})
   else()
      include(FindSDL)
      #pkg_search_module(SDL REQUIRED sdl)
      #include_directories(${SDL_INCLUDE_DIRS})
      include_directories(${SDL_INCLUDE_DIR})
      set(SDL_LIBS ${SDL_LIBRARY})
   endif()
endif()

# GCC Only?
if(CMAKE_COMPILER_IS_GNUCC) 
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flax-vector-conversions")
endif()

if(CMAKE_COMPILER_IS_GNUCXX) 
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -flax-vector-conversions")
endif()


check_function_exists("nanosleep" HAVE_NANOSLEEP)
if(NOT HAVE_NANOSLEEP)
  check_library_exists("rt" "nanosleep" "" LIB_RT_HAS_NANOSLEEP)
endif(NOT HAVE_NANOSLEEP)

if(LIB_RT_HAS_NANOSLEEP)
  add_target_library(${EXEC_TARGET} rt)
endif(LIB_RT_HAS_NANOSLEEP)

if(HAVE_NANOSLEEP OR LIB_RT_HAS_NANOSLEEP)
  add_definitions(-DHAVE_NANOSLEEP)
endif(HAVE_NANOSLEEP OR LIB_RT_HAS_NANOSLEEP)


set(SRC_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../src)

if(USE_QT_5)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

if(DEFINED VM_NAME)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/${VM_NAME})
#  if(USE_FMGEN)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
#    if(WIN32)
#      set(FMGEN_LIB vm_fmgen)
#	  set(FMGEN_LIB "-lCSPfmgen")
#	endif()
#  endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/machines/${VM_NAME})
endif()

if(LIBAV_FOUND)
   include_directories(${LIBAV_INCLUDE_DIRS})
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/gui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt)
if(WIN32)
#  add_subdirectory(../../src/qt/gui qt/gui)
endif()  
#add_subdirectory(../../src/qt qt/osd)
add_subdirectory(../../src common)
add_subdirectory(../../src/vm vm/)

if(DEFINED VM_NAME)
# if(WITH_DEBUGGER)
   set(DEBUG_LIBS qt_debugger)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/debugger)
   add_subdirectory(../../src/qt/debugger qt/debugger)
# else()
#   set(DEBUG_LIBS)
# endif()

if(WIN32)
	   set(LOCAL_LIBS 
		   common_emu
           qt_${VM_NAME}
		   vm_${VM_NAME}
		   ${VM_APPEND_LIBS}
		   ${FMGEN_LIB}
		   vm_vm
		   ${DEBUG_LIBS}
		   common_common
		   )
else()
	   set(LOCAL_LIBS     
		   common_emu
           qt_${VM_NAME}
		   vm_${VM_NAME}
		   vm_vm
		   ${VM_APPEND_LIBS}
		   ${DEBUG_LIBS}
		   common_common
		   )
	   endif()
endif()

include(simd-x86)

if(WIN32)
   set(BUNDLE_LIBS 
       ${OPENGL_LIBRARY}
       ${OPENCL_LIBRARY}
       ${GETTEXT_LIBRARY}
       ${OPENMP_LIBRARY}
       ${LIBAV_LIBRARIES}
       ${SDL_LIBS}
       ${LIBAV_LIBRARIES}
       ${ADDITIONAL_LIBRARIES}
       )
       #SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET>  <LINK_FLAGS> <OBJECTS>")
       #SET(CMAKE_C_ARCHIVE_FINISH   true)
       #SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
       #SET(CMAKE_CXX_ARCHIVE_FINISH   true)
else()
   set(BUNDLE_LIBS 
#       ${OPENGL_LIBRARY}
       ${OPENCL_LIBRARY}
#       ${GETTEXT_LIBRARY}
       ${OPENMP_LIBRARY}
       ${SDL_LIBS}
#       ${LIBAV_LIBRARIES}
       ${ADDITIONAL_LIBRARIES}
       )
       set(BUNDLE_LIBS ${BUNDLE_LIBS} -lCSPosd -lCSPfmgen -lCSPcommon_vm -lCSPgui -lCSPemu_utils -lCSPavio)
endif()

if(USE_QT_5)
  set(BUNDLE_LIBS ${BUNDLE_LIBS} ${QT_LIBRARIES})
endif()

set(BUNDLE_LIBS ${BUNDLE_LIBS} ${THREADS_LIBRARY})

if(DEFINED VM_NAME)
	add_subdirectory(../../src/vm/${VM_NAME} vm/${VM_NAME})
#  if(USE_FMGEN)
#	add_subdirectory(../../src/vm/fmgen vm/fmgen)
#  endif()	
	add_subdirectory(../../src/qt/machines/${VM_NAME} qt/${VM_NAME})
	add_subdirectory(../../src/qt/common qt/common)
endif()
