# the name of the target operating system
#SET(CMAKE_SYSTEM_NAME Windows)
SET(CSP_CROSS_BUILD 0)

# Choose an appropriate compiler prefix
#set(CMAKE_TOOLCHAIN_PREFIX "i686-w64-mingw32")

# which compilers to use for C and C++
#SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER clang-10)
SET(CMAKE_CXX_COMPILER clang++-10)
#SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
SET(CMAKE_AR ar)
SET(CMAKE_C_COMPILER_AR  ar)
SET(CMAKE_CXX_COMPILER_AR  ar)
SET(CMAKE_LD  lld)
#SET(CMAKE_LINKER  lld-10)
#SET(CMAKE_EXE_LINKER lld-10)
#SET(CMAKE_SHARED_LINKER  lld-10)
SET(CMAKE_NM  nm)
SET(CMAKE_RANLIB  ranlib)
SET(CMAKE_C_COMPILER_RANLIB  ranlib)
SET(CMAKE_CXX_COMPILER_RANLIB  ranlib)


