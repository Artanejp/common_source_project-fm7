# Set configuration for building XM7/SDL.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

include(CheckFunctionExists)
#include(cotire)

if(USE_DEVICES_SHARED_LIB)
  add_definitions(-DUSE_SHARED_DLL)
  add_definitions(-DUSE_SHARED_UI_DLL)
  add_definitions(-DUSE_SHARED_DEVICES_DLL)
#  set(I386_CPPS
#	libcpu_newdev/i386.cpp
#	libcpu_newdev/libcpu_i386/i386_real.cpp
#	libcpu_newdev/libcpu_i386/i386op16_real.cpp
#	libcpu_newdev/libcpu_i386/i386dasm.cpp
#	)
  set(MC6809_CPPS 
	mc6809.cpp
  )
  set(MCS48_CPPS
	mcs48.cpp
	)
  set(IX86_CPPS
	libcpu_newdev/i86.cpp
	)
  set(Z80_CPPS 
       z80.cpp
  )
else()
  set(I386_CPPS i386.cpp)
  set(MC6809_CPPS mc6809_base.cpp mc6809.cpp)
  set(MCS48_CPPS mcs48_base.cpp mcs48.cpp)
  set(IX86_CPPS i86.cpp)
  set(Z80_CPPS   z80_base.cpp z80.cpp)
  set(VMFILES ${VMFILES} ${VMFILES_LIB})
endif()

if(FLAG_USE_I86)
  set(VMFILES ${VMFILES} ${IX86_CPPS})
endif()
if(FLAG_USE_I286)
  set(VMFILES ${VMFILES} i286.cpp)
endif()
if(FLAG_USE_Z80)
  set(VMFILES ${VMFILES} z80.cpp)
endif()
if(FLAG_USE_MC6809)
  set(VMFILES ${VMFILES} ${MC6809_CPPS})
endif()
if(FLAG_USE_MCS48)
  set(VMFILES ${VMFILES} ${MCS48_CPPS})
endif()
if(FLAG_USE_Z80)
  set(VMFILES ${VMFILES} ${Z80_CPPS})
endif()

if(DEFINED QT5_ROOT_PATH)
  SET(CMAKE_FIND_ROOT_PATH  ${QT5_ROOT_PATH} ${CMAKE_FIND_ROOT_PATH})
endif()

# Use ccache if enabled.
find_program(USE_CCACHE ccache)
if(USE_CCACHE)
   SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
#  SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
  endif()
if(WIN32)
  FIND_PACKAGE(Qt5Core REQUIRED)
else()
  FIND_PACKAGE(Qt5Widgets REQUIRED)
endif()
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
  if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif()

find_package(Threads)
include_directories(${THREADS_INCLUDE_PATH})

include(FindPkgConfig)

find_package(Git)
if(GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD OUTPUT_VARIABLE __tstr)
	string(FIND ${__tstr} "fatal" __notfound)
	string(REPLACE "\n" "" __tstr2 ${__tstr})
	if(${__notfound} EQUAL -1)
		   add_definitions(-D__GIT_REPO_VERSION=\"${__tstr2}\")
	else()
		   add_definitions(-U__GIT_REPO_VERSION)
	endif()
endif()

string(TIMESTAMP __build_date "%b %d,%Y %H:%M:%S UTC" UTC)
add_definitions(-D__BUILD_DATE=\"${__build_date}\")

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

include(FindZLIB)
if(ZLIB_FOUND)
  add_definitions(-DUSE_ZLIB)
   include_directories(${ZLIB_INCLUDE_DIRS})
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
  if(NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif()
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

#add_custom_command(OUTPUT test.txt
#          COMMAND grep ARGS -m 1 THIS_LIB_VERSION ${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen/CMakeLists.txt
#	  COMMAND sed ARGS "-r" "'s/.*THIS_VERSION\ //'" 
#	  COMMAND sed ARGS "-r" "'s/\).*$//'"
#	  )


if(DEFINED VM_NAME)
# if(WITH_DEBUGGER)
   set(DEBUG_LIBS qt_debugger)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/debugger)
   add_subdirectory(../../src/qt/debugger qt/debugger)
# else()
#   set(DEBUG_LIBS)
# endif()
	if(USE_DEVICES_SHARED_LIB)
	else()
	  if(USE_FMGEN)
		set(VM_APPEND_LIBS fmgen ${VM_APPEND_LIBS})

	  else()
		set(VM_APPEND_LIBS ${VM_APPEND_LIBS})
	  endif()
	endif()
	if(WIN32)
	   set(LOCAL_LIBS     
		   common_emu
           qt_${VM_NAME}
		   vm_${VM_NAME}
		   vm_vm
		   ${VM_APPEND_LIBS}
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
       ${ZLIB_LIBRARIES}
       )
       #SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET>  <LINK_FLAGS> <OBJECTS>")
       #SET(CMAKE_C_ARCHIVE_FINISH   true)
       #SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
       #SET(CMAKE_CXX_ARCHIVE_FINISH   true)
else()
   add_definitions(-D_UNICODE)
   set(BUNDLE_LIBS 
#       ${OPENGL_LIBRARY}
       ${OPENCL_LIBRARY}
#       ${GETTEXT_LIBRARY}
       ${OPENMP_LIBRARY}
       ${SDL_LIBS}
#       ${LIBAV_LIBRARIES}
       ${ADDITIONAL_LIBRARIES}
       )
	if(USE_DEVICES_SHARED_LIB)
		set(BUNDLE_LIBS ${BUNDLE_LIBS} -lCSPosd -lCSPcommon_vm -lCSPfmgen -lCSPgui -lCSPemu_utils -lCSPavio)
	else()
		set(BUNDLE_LIBS ${BUNDLE_LIBS} -lCSPosd -lCSPgui -lCSPavio)
	endif()  
endif()

if(USE_QT_5)
  set(BUNDLE_LIBS ${BUNDLE_LIBS} ${QT_LIBRARIES} ${ZLIB_LIBRARIES})
endif()

set(BUNDLE_LIBS ${BUNDLE_LIBS} ${THREADS_LIBRARY})

if(DEFINED VM_NAME)
	if(USE_DEVICES_SHARED_LIB)
		add_subdirectory(../../src/vm/${VM_NAME} vm/${VM_NAME})
		add_subdirectory(../../src/qt/machines/${VM_NAME} qt/${VM_NAME})
		add_subdirectory(../../src/qt/common qt/common)
	  else()
		add_subdirectory(../../src/vm/${VM_NAME} vm/${VM_NAME})
		#add_subdirectory(../../src/vm vm/common)
		#add_subdirectory(../../src common/common)
		if(USE_FMGEN)
			add_subdirectory(../../src/vm/fmgen vm/fmgen)
		endif()	
		add_subdirectory(../../src/qt/machines/${VM_NAME} qt/${VM_NAME})
		add_subdirectory(../../src/qt/common qt/common)
	endif()	
endif()
