# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

include(CheckFunctionExists)
# Use cmake if enabled.
  find_program(USE_CCACHE ccache)
  if(USE_CCACHE)
   SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
   SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
  endif()
  if(NOT CSP_CROSS_BUILD)
    SET(CMAKE_FIND_ROOT_PATH  /opt/Qt5.3.2/5.3/gcc_64 ${CMAKE_FIND_ROOT_PATH})
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
      pkg_search_module(SDL2 REQUIRED sdl2)
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

set(SRC_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../src)

if(USE_QT_5)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

if(LIBAV_FOUND)
  include_directories(${LIBAV_INCLUDE_DIRS})
endif()

add_definitions(-D_USE_QT5)

if(USE_QT5_4_APIS)
  add_definitions(-D_USE_QT_5_4)
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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/gui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src)
