# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i586-mingw32msvc-gcc)
SET(CMAKE_CXX_COMPILER i586-mingw32msvc-g++)
SET(CMAKE_RC_COMPILER i586-mingw32msvc-windres)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/i586-mingw32msvc 
                          /usr/local/i586-mingw-msvc
                          /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32
			  /usr/local/i586-mingw-msvc/5.5/mingw492_32)



# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(SDL2_LIBRARIES
                         /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/lib/libSDL2.dll.a 
			 /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/lib/libSDL2main.a)
set(SDL2_INCLUDE_DIRS /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/include/SDL2)
set(SDLMAIN_LIBRARY "")

set(ADDITIONAL_LIBRARIES libwinmm.a)