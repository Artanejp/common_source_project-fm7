# Set simd X86.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>

# Detect host CPU
if(UNIX)
 # See http://stackoverflow.com/questions/11944060/how-to-detect-target-architecture-using-cmake
 EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
elseif(WIN32)
 set(ARCHITECTURE "ia32")
else()
 # Assume Unix.
 # See http://stackoverflow.com/questions/11944060/how-to-detect-target-architecture-using-cmake
 EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
endif()
