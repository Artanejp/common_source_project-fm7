# Set simd X86.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

if((CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64") OR
   (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86") OR
   (CMAKE_SYSTEM_PROCESSOR STREQUAL "ia32") OR
   (CMAKE_SYSTEM_PROCESSOR STREQUAL "amd64"))
 set(USE_SSE2 ON CACHE BOOL "Using SSE2 SIMD instructions, sometimes faster if enabled.")
# set(USE_MMX  ON CACHE BOOL "Using MMX SIMD instructions, sometimes faster if enabled.")
endif()


if(USE_SSE2)
# set(LOCAL_LIBS ${LOCAL_LIBS} common_scaler-sse2)
 add_definitions(-DUSE_SSE2)
# add_subdirectory(sdl/vram/sse2)
# add_subdirectory(../../src/agar/common/scaler/sse2 agar/common/scaler/sse2)
endif()

#if(USE_SSE)
# add_definitions(-DUSE_SSE)
#endif()

#if(USE_MMX)
# add_definitions(-DUSE_MMX)
# set(LOCAL_LIBS ${LOCAL_LIBS} xm7_soundbuffer-mmx)
# add_subdirectory(sdl/soundbuffer/mmx)
#endif()

