# Set configuration for building XM7/SDL.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

include(CheckFunctionExists)
# Use cmake if enabled.
SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)


  FIND_PACKAGE(Qt5Widgets REQUIRED)
  FIND_PACKAGE(Qt5Core REQUIRED)
  FIND_PACKAGE(Qt5Gui REQUIRED)
  FIND_PACKAGE(Qt5OpenGL REQUIRED)
  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  include_directories(${Qt5Core_INCLUDE_DIRS})
  include_directories(${Qt5Gui_INCLUDE_DIRS})
  include_directories(${Qt5OpenGL_INCLUDE_DIRS})
if(USE_SOCKET)
  FIND_PACKAGE(Qt5Network REQUIRED)
  include_directories(${Qt5Network_INCLUDE_DIRS})
endif()
SET(USE_QT_5 ON)
set(USE_QT5_4_APIS OFF CACHE BOOL "Build with Qt5.4 (or later) APIs if you can.")

add_definitions(-D_USE_QT5)
if(USE_QT5_4_APIS)
  add_definitions(-D_USE_QT_5_4)
endif()

SET(CMAKE_AUTOMOC OFF)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-D_USE_QT)
add_definitions(-DUSE_QT)
add_definitions(-DQT_MAJOR_VERSION=${Qt5Widgets_VERSION_MAJOR})
add_definitions(-DQT_MINOR_VERSION=${Qt5Widgets_VERSION_MINOR})
# Build Flags

#find_package(Gettext)
#include_directories(${GETTEXT_INCLUDE_PATH})
#include(compile_gettext_catalogue)
#if(GETTEXT_FOUND)
#   add_definitions(-DUSE_GETTEXT)
#endif()


find_package(Freetype)
include_directories(${FREETYPE_INCLUDE_PATH})

find_package(Iconv)
if(ICONV_FOUND)
  add_definitions(-DUSE_ICONV)
endif()

if(USE_OPENMP)
  find_package(OpenMP)
  include_directories(${OPENMP_INCLUDE_PATH})
endif()

find_package(Threads)
include_directories(${THREADS_INCLUDE_PATH})

##PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
#find_package(SDL)
#include_directories(${SDL_INCLUDE_PATH})


include(FindPkgConfig)

pkg_search_module(SDL2 REQUIRED sdl2)
include_directories(${SDL2_INCLUDE_DIRS})

if(ICONV_FOUND)
 include_directories(${ICONV_INCLUDE_DIRS})
 set(LOCAL_LIBS ${LOCAL_LIBS} ${ICONV_LIBRARIES})
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

find_package(OpenGL)

set(USE_OPENCL OFF CACHE BOOL "Build using OpenCL")
if(USE_OPENCL)
 if(OPENGL_FOUND)
   find_package(OpenCL)
   if(OPENCL_FOUND)
    include_directories(${OPENCL_INCLUDE_DIRS})
    add_definitions(-D_USE_OPENCL -DUSE_OPENCL)
    set(OPENCL_LIBRARY ${OPENCL_LIBRARIES})
    set(USE_OPENGL ON)
   endif()
 endif()
endif()

if(USE_OPENGL)
 if(OPENGL_FOUND)
   include_directories(${OPENGL_INCLUDE_PATH})
   add_definitions(-D_USE_OPENGL -DUSE_OPENGL)
 else()
   set(USE_OPENGL OFF)
   set(USE_OPENCL OFF)
   unset(OPENCL_LIBRARY)
 endif()
endif()

set(SRC_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../src)

if(WITH_DEBUGGER)
   add_definitions(-DUSE_DEBUGGER)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/debugger)
#   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/3rdparty/qtermwidget/lib)
endif()

if(USE_QT_5)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/gui)

#add_subdirectory(../../src/agar/common/scaler/generic agar/common/scaler/generic)
add_subdirectory(../../src/qt/gui qt/gui)

add_subdirectory(../../src common)
add_subdirectory(../../src/vm vm/)

if(WITH_DEBUGGER)
#   add_subdirectory(../../src/qt/3rdparty/qtermwidget/lib qt/qtermwidget)
   add_subdirectory(../../src/qt/debugger qt/debugger)
# if(USE_QT5)
#   set(LOCAL_LIBS ${LOCAL_LIBS} qtermwidget5)
# else()
#   set(LOCAL_LIBS ${LOCAL_LIBS} qtermwidget4)
# endif()
   set(LOCAL_LIBS ${LOCAL_LIBS} qt_debugger)
endif()

include(simd-x86)

set(BUNDLE_LIBS 
                           ${OPENGL_LIBRARY}
			   ${OPENCL_LIBRARY}
			   ${GETTEXT_LIBRARY}
			   ${OPENMP_LIBRARY}
#			   ${SDL_LIBRARY}
                           ${SDL2_LIBRARIES}
)
if(USE_QT_5)
  set(BUNDLE_LIBS ${BUNDLE_LIBS} ${QT_LIBRARIES})
endif()

set(BUNDLE_LIBS ${BUNDLE_LIBS} ${THREADS_LIBRARY})


