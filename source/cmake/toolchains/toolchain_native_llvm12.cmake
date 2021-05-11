# the name of the target operating system
#SET(CMAKE_SYSTEM_NAME Windows)
SET(CSP_CROSS_BUILD 0)

# Choose an appropriate compiler prefix
#set(CMAKE_TOOLCHAIN_PREFIX "i686-w64-mingw32")

# which compilers to use for C and C++
#SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER clang-12)
SET(CMAKE_CXX_COMPILER clang++-12)
#SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
SET(CMAKE_AR llvm-ar-12)
SET(CMAKE_C_COMPILER_AR  llvm-ar-12)
SET(CMAKE_CXX_COMPILER_AR  llvm-ar-12)
SET(CMAKE_LD  lld-12)
SET(CMAKE_LINKER  lld-12)
SET(CMAKE_EXE_LINKER lld-12)
SET(CMAKE_SHARED_LINKER  lld-12)
SET(CMAKE_NM  nm)
SET(CMAKE_RANLIB  llvm-ranlib-12)
SET(CMAKE_C_COMPILER_RANLIB  llvm-ranlib-12)
SET(CMAKE_CXX_COMPILER_RANLIB  llvm-ranlib-12)

#SET(CSP_ADDTIONAL_FLAGS_COMPILE_RELWITHDEBINFO -g2 -gz -flto) 
#SET(CSP_ADDTIONAL_FLAGS_LINK_RELWITHDEBINFO -g2 -gz -flto -O2) 
#SET(CSP_ADDTIONAL_FLAGS_COMPILE_RELEASE -flto) 
#SET(CSP_ADDTIONAL_FLAGS_LINK_RELEASE -flto -O3) 
#SET(CMAKE_CXX_FLAGS "-g2 -gz -flto")
#SET(CMAKE_C_FLAGS "-g2 -gz -flto")
#SET(CMAKE_EXE_LINK_FLAGS "-g2 -gz -flto")