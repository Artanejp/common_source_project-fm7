# This is an example project to show and test the usage of the FindOpenCL
# script.

project( Example )

# This is only required for the script to work in the version control
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/..")

find_package( OpenCL REQUIRED )

include_directories( ${OPENCL_INCLUDE_DIRS} )

if( OPENCL_HAS_CPP_BINDINGS )
	message( "OpenCL has CPP bindings. Full include is: " ${OPENCL_INCLUDE_DIRS} )
else( OPENCL_HAS_CPP_BINDINGS )
	message( "No OpenCL CPP bindings found" )
endif( OPENCL_HAS_CPP_BINDINGS )


add_executable( example example.cpp )
target_link_libraries( example ${OPENCL_LIBRARIES} )
