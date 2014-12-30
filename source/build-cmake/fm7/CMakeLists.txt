# Build XM7/SDL, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.


cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure XM7/SDL,Agar **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emufm7)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(XM7_VERSION 3)
#set(WITH_DEBUGGER ON CACHE BOOL "Build witn XM7 Debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

include(config_commonsource)

set(WITH_AGAR_STATIC ON CACHE BOOL "Link LibAgar statically.")
set(WITH_LIBAGAR_PREFIX "/usr/local" CACHE STRING "Set prefix of LibAgar") 
include_directories(${WITH_LIBAGAR_PREFIX}/include/agar)

if(WITH_AGAR_STATIC)
  set(AGAR_LIBS ${WITH_LIBAGAR_PREFIX}/lib/libag_dev.a ${WITH_LIBAGAR_PREFIX}/lib/libag_gui.a ${WITH_LIBAGAR_PREFIX}/lib/libag_core.a)
else(WITH_AGAR_STATIC)
  set(AGAR_LIBS ag_core ag_dev ag_gui)
  link_directories(${WITH_LIBAGAR_PREFIX}/lib)
endif()

set(AGAR_DEPLIBS m jpeg png z dl uim-scm uim Xinerama)


find_package(Gettext)
include_directories(${GETTEXT_INCLUDE_PATH})
include(compile_gettext_catalogue)

find_package(Freetype)
include_directories(${FREETYPE_INCLUDE_PATH})

find_package(Iconv)


if(USE_OPENMP)
  find_package(OpenMP)
  include_directories(${OPENMP_INCLUDE_PATH})
endif()

find_package(Threads)
include_directories(${THREADS_INCLUDE_PATH})

find_package(SDL)
include_directories(${SDL_INCLUDE_PATH})


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


include(simd-x86)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/generic)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)

add_subdirectory(../../src/vm vm/)
#add_subdirectory(../../src/vm/fm7 vm/fm7)
add_subdirectory(../../src/vm/fm7/vram/generic vm/fm7/vram/generic)
add_subdirectory(../../src/vm/fmgen vm/fmgen)

if(USE_SSE2)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()

if(USE_SSE2)
 include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()
