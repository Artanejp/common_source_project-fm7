# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)
SET(CSP_CROSS_BUILD 1)

# Choose an appropriate compiler prefix
set(CMAKE_TOOLCHAIN_PREFIX "i686-w64-mingw32")
set(CMAKE_FIND_ROOT_PATH /usr/${CMAKE_TOOLCHAIN_PREFIX} /opt/llvm-mingw-11/${CMAKE_TOOLCHAIN_PREFIX})

set(LIBAV_ROOT_DIR "/usr/local/i586-mingw-msvc/ffmpeg-4.3")


# which compilers to use for C and C++
SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER ${CMAKE_TOOLCHAIN_PREFIX}-clang)
SET(CMAKE_CXX_COMPILER ${CMAKE_TOOLCHAIN_PREFIX}-clang++)
SET(CMAKE_RC_COMPILER ${CMAKE_TOOLCHAIN_PREFIX}-windres)
SET(CMAKE_AR ${CMAKE_TOOLCHAIN_PREFIX}-ar)
SET(CMAKE_C_COMPILER_AR  ${CMAKE_TOOLCHAIN_PREFIX}-ar)
SET(CMAKE_CXX_COMPILER_AR  ${CMAKE_TOOLCHAIN_PREFIX}-ar)
SET(CMAKE_LD  /usr/bin/${CMAKE_TOOLCHAIN_PREFIX}-ld)
SET(CMAKE_NM  ${CMAKE_TOOLCHAIN_PREFIX}-nm)
SET(CMAKE_RANLIB  ${CMAKE_TOOLCHAIN_PREFIX}-ranlib)
SET(CMAKE_C_COMPILER_RANLIB  ${CMAKE_TOOLCHAIN_PREFIX}-ranlib)
SET(CMAKE_CXX_COMPILER_RANLIB  ${CMAKE_TOOLCHAIN_PREFIX}-ranlib)

#SET(CMAKE_LINKER  ${CMAKE_TOOLCHAIN_PREFIX}-ld)
#SET(CMAKE_EXE_LINKER  ${CMAKE_TOOLCHAIN_PREFIX}-ld)
#SET(CMAKE_SHARED_LINKER  ${CMAKE_TOOLCHAIN_PREFIX}-ld)

#find_program(CMAKE_RC_COMPILER  NAMES ${CMAKE_TOOLCHAIN_PREFIX}-windres)
#find_program(CMAKE_C_COMPILER   NAMES ${CMAKE_TOOLCHAIN_PREFIX}-clang)
#find_program(CMAKE_CXX_COMPILER NAMES ${CMAKE_TOOLCHAIN_PREFIX}-clang++)
#find_program(CMAKE_ASM_COMPILER NAMES ${CMAKE_TOOLCHAIN_PREFIX}-as)

set(CMAKE_CXX_FLAGS "-target ${CMAKE_TOOLCHAIN_PREFIX}")
set(CMAKE_C_FLAGS "-target ${CMAKE_TOOLCHAIN_PREFIX}")

set(CMAKE_EXE_LINKER_FLAGS "-L/usr/${CMAKE_TOOLCHAIN_PREFIX}/lib -target ${CMAKE_TOOLCHAIN_PREFIX} ")
set(CMAKE_SHARED_LINKER_FLAGS "-L/usr/${CMAKE_TOOLCHAIN_PREFIX}/lib -target ${CMAKE_TOOLCHAIN_PREFIX} ")

#set(CMAKE_EXE_LINKER_FLAGS "")

# here is the target environment located

set(USE_SDL2 ON)
if(USE_SDL2)
   SET(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH} /usr/${CMAKE_TOOLCHAIN_PREFIX}
                          /usr/local/i586-mingw-msvc
                          /usr/local/i586-mingw-msvc/SDL/${CMAKE_TOOLCHAIN_PREFIX}
			  /usr/local/i586-mingw-msvc/Qt5.15/mingw_82x
			  )
else()
   SET(CMAKE_FIND_ROOT_PATH  ${CMAKE_FIND_ROOT_PATH} /usr/${CMAKE_TOOLCHAIN_PREFIX} 
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
                         /usr/local/i586-mingw-msvc/SDL/${CMAKE_TOOLCHAIN_PREFIX}/lib/libSDL2.dll.a 
			 /usr/local/i586-mingw-msvc/SDL/${CMAKE_TOOLCHAIN_PREFIX}/lib/libSDL2main.a)
set(SDL2_INCLUDE_DIRS /usr/local/i586-mingw-msvc/SDL/${CMAKE_TOOLCHAIN_PREFIX}/include/SDL2)

set(SDL_LIBRARIES
                         /usr/local/i586-mingw-msvc/SDL1/lib/libSDL.dll.a 
			 /usr/local/i586-mingw-msvc/SDL1/lib/libSDLmain.a)
set(SDL_INCLUDE_DIRS /usr/local/i586-mingw-msvc/SDL1/include/SDL)

set(SDLMAIN_LIBRARY "")

set(ADDITIONAL_LIBRARIES libwinmm.a)

