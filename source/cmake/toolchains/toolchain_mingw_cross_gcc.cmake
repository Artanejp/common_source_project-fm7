# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

SET(TARGET_ARCH i686-w64-mingw32)
SET(LIBS_PREFIX /usr/local/i586-mingw-msvc)
# which compilers to use for C and C++
SET(CMAKE_C_COMPILER ${TARGET_ARCH}-gcc)
SET(CMAKE_CXX_COMPILER ${TARGET_ARCH}-g++)
SET(CMAKE_RC_COMPILER ${TARGET_ARCH}-windres)
SET(CMAKE_AR  ${TARGET_ARCH}-ar)
SET(CMAKE_LD  ${TARGET_ARCH}-ld.bfd)
SET(USING_TOOLCHAIN_GCC_DEBIAN ON)

#set(LIBAV_ROOT_DIR "${LIBS_PREFIX}/ffmpeg-4.1")

# here is the target environment located
set(USE_SDL2 ON)
if(USE_SDL2)
   SET(CMAKE_FIND_ROOT_PATH  /usr/${TARGET_ARCH} 
                          ${LIBS_PREFIX}
                          ${LIBS_PREFIX}/SDL/${TARGET_ARCH}
			  ${LIBS_PREFIX}/Qt5.15/mingw_82x
			  )
else()
   SET(CMAKE_FIND_ROOT_PATH  /usr/${TARGET_ARCH} 
                          ${LIBS_PREFIX}
                          ${LIBS_PREFIX}/SDL1/
			  ${LIBS_PREFIX}/Qt5.15/mingw_82x
			  )
endif()
SET(CSP_CROSS_BUILD 1)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(SDL2_LIBRARIES
                         ${LIBS_PREFIX}/SDL/${TARGET_ARCH}/lib/libSDL2.dll.a 
			 ${LIBS_PREFIX}/SDL/${TARGET_ARCH}/lib/libSDL2main.a)
set(SDL2_INCLUDE_DIRS ${LIBS_PREFIX}/SDL/${TARGET_ARCH}/include/SDL2)

set(SDL_LIBRARIES
                         ${LIBS_PREFIX}/SDL1/lib/libSDL.dll.a 
			 ${LIBS_PREFIX}/SDL1/lib/libSDLmain.a)
set(SDL_INCLUDE_DIRS ${LIBS_PREFIX}/SDL1/include/SDL)

set(SDLMAIN_LIBRARY "")

set(ADDITIONAL_LIBRARIES libwinmm.a)
