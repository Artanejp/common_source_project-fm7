# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)
SET(CSP_CROSS_BUILD 1)

# Choose an appropriate compiler prefix
set(CMAKE_TOOLCHAIN_PREFIX "i686-w64-mingw32")

# which compilers to use for C and C++
SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER i686-w64-mingw32-clang)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-clang++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
SET(CMAKE_AR i686-w64-mingw32-ar)
SET(CMAKE_C_COMPILER_AR  i686-w64-mingw32-ar)
SET(CMAKE_CXX_COMPILER_AR  i686-w64-mingw32-ar)
SET(CMAKE_LD  /usr/bin/i686-w64-mingw32-ld)
#SET(CMAKE_LINKER  /usr/bin/i686-w64-mingw32-ld)
#SET(CMAKE_EXE_LINKER  /usr/bin/i686-w64-mingw32-ld)
#SET(CMAKE_SHARED_LINKER  /usr/bin/i686-w64-mingw32-ld)
SET(CMAKE_NM  i686-w64-mingw32-nm)
SET(CMAKE_RANLIB  i686-w64-mingw32-ranlib)
SET(CMAKE_C_COMPILER_RANLIB  i686-w64-mingw32-ranlib)
SET(CMAKE_CXX_COMPILER_RANLIB  i686-w64-mingw32-ranlib)

#find_program(CMAKE_RC_COMPILER  NAMES ${CMAKE_TOOLCHAIN_PREFIX}-windres)
#find_program(CMAKE_C_COMPILER   NAMES ${CMAKE_TOOLCHAIN_PREFIX}-clang)
#find_program(CMAKE_CXX_COMPILER NAMES ${CMAKE_TOOLCHAIN_PREFIX}-clang++)
#find_program(CMAKE_ASM_COMPILER NAMES ${CMAKE_TOOLCHAIN_PREFIX}-as)

#set(CMAKE_C_COMPILER   "${CMAKE_TOOLCHAIN_PREFIX}-clang"   "-target ${CMAKE_TOOLCHAIN_PREFIX} -isystem /opt/llvm-mingw/${CMAKE_TOOLCHAIN_PREFIX}/include -isystem /usr/local/${CMAKE_TOOLCHAIN_PREFIX}/include")
#set(CMAKE_CXX_COMPILER "${CMAKE_TOOLCHAIN_PREFIX}-clang++" "-target ${CMAKE_TOOLCHAIN_PREFIX} -isystem /opt/llvm-mingw/${CMAKE_TOOLCHAIN_PREFIX}/include -isystem /usr/local/${CMAKE_TOOLCHAIN_PREFIX}/include")

set(CMAKE_CXX_FLAGS "-target i686-w64-mingw32")
set(CMAKE_C_FLAGS "-target i686-w64-mingw32")
#set(CMAKE_CXX_FLAGS "-target i686-w64-mingw32")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc -nostdinc++")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem /usr/lib/clang/3.9.0/include")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem /usr/i686-w64-mingw32/include/../../../usr/lib/gcc/i686-w64-mingw32/8.2-win32/include/c++")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem /usr/i686-w64-mingw32/include/../../../usr/lib/gcc/i686-w64-mingw32/8.2-win32/include/c++/i686-w64-mingw32")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem /usr/i686-w64-mingw32/include")

set(CMAKE_EXE_LINKER_FLAGS "-L/usr/i686-w64-mingw32/lib -target i686-w64-mingw32 ")
set(CMAKE_SHARED_LINKER_FLAGS "-L/usr/i686-w64-mingw32/lib -target i686-w64-mingw32 ")

#set(CMAKE_EXE_LINKER_FLAGS "")

set(LIBAV_ROOT_DIR "/usr/local/i586-mingw-msvc/ffmpeg-4.2")


# here is the target environment located
# here is the target environment located
set(CMAKE_FIND_ROOT_PATH /usr/${CMAKE_TOOLCHAIN_PREFIX} /opt/llvm-mingw/${CMAKE_TOOLCHAIN_PREFIX})

set(USE_SDL2 ON)
if(USE_SDL2)
   SET(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH} /usr/i686-w64-mingw32 
                          /usr/local/i586-mingw-msvc
                          /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32
			  /usr/local/i586-mingw-msvc/Qt5.15/mingw_82x
			  )
else()
   SET(CMAKE_FIND_ROOT_PATH  ${CMAKE_FIND_ROOT_PATH} /usr/i686-w64-mingw32 
                          /usr/local/i586-mingw-msvc
                          /usr/local/i586-mingw-msvc/SDL1/
			  /usr/local/i586-mingw-msvc/Qt5.15/mingw_82x
			  )
endif()

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#set(CMAKE_CROSS_COMPILING TRUE)


set(SDL2_LIBRARIES
                         /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/lib/libSDL2.dll.a 
			 /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/lib/libSDL2main.a)
set(SDL2_INCLUDE_DIRS /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/include/SDL2)

set(SDL_LIBRARIES
                         /usr/local/i586-mingw-msvc/SDL1/lib/libSDL.dll.a 
			 /usr/local/i586-mingw-msvc/SDL1/lib/libSDLmain.a)
set(SDL_INCLUDE_DIRS /usr/local/i586-mingw-msvc/SDL1/include/SDL)

set(SDLMAIN_LIBRARY "")

set(ADDITIONAL_LIBRARIES libwinmm.a)

