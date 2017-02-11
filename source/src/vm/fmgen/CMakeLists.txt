cmake_minimum_required (VERSION 2.6)

message("* vm/fmgen")

SET(s_vm_fmgen_srcs
	fmgen.cpp
	fmtimer.cpp
	opm.cpp
	opna.cpp
	psg.cpp
	psg_ay_3_891x.cpp
)

if(WIN32)
#add_library(vm_fmgen
#    ${s_vm_fmgen_srcs}
#    )

include (GenerateExportHeader)
add_library(CSPfmgen SHARED
    ${s_vm_fmgen_srcs}
    )
target_link_libraries(CSPfmgen
	${CMAKE_CURRENT_SOURCE_DIR}/../../../build-cmake/bin-win32/libCSPemu_utils.dll.a
	)
set_target_properties(CSPfmgen PROPERTIES
			     SOVERSION 1.0.0
			     VERSION 1.0.0
			     LINK_INTERFACE_LIBRARIES ""
			     )
generate_export_header(CSPfmgen
        BASE_NAME CSPfmgen
        EXPORT_MACRO_NAME CSPfmgen_EXPORT
        EXPORT_FILE_NAME CSPfmgen_Export.h
        STATIC_DEFINE CSPfmgen_BUILT_AS_STATIC
)

else()
  add_library(CSPfmgen SHARED
    ${s_vm_fmgen_srcs}
  )
  set_target_properties(CSPfmgen PROPERTIES 
     SOVERSION 1.0.0
     VERSION 1.0.0
  )
  INSTALL(TARGETS CSPfmgen DESTINATION ${LIBCSP_INSTALL_DIR})
endif()
