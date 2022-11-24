cmake_minimum_required (VERSION 3.0)

message("* vm/fmgen")
SET(THIS_LIB_VERSION 5.4.1)
add_definitions(-D__LIBFMGEN_VERSION=\"libCSPfmgen.${THIS_LIB_VERSION}\")

SET(s_vm_fmgen_srcs
	fmgen.cpp
	fmtimer.cpp
	opm.cpp
	opna.cpp
	psg.cpp
#	psg_ay_3_891x.cpp
)

if(WIN32)
	include (GenerateExportHeader)
	add_library(CSPfmgen SHARED
		${s_vm_fmgen_srcs}
	)
	target_link_libraries(CSPfmgen
		PRIVATE CSPemu_utils
		 ${ADDITIONAL_DLL_LIBRARIES}
	)

	set_target_properties(CSPfmgen PROPERTIES
		SOVERSION ${THIS_LIB_VERSION}
		VERSION ${THIS_LIB_VERSION}
		LINK_INTERFACE_LIBRARIES ""
	)
	generate_export_header(CSPfmgen
        	BASE_NAME CSPfmgen
        	EXPORT_MACRO_NAME CSPfmgen_EXPORT
        	EXPORT_FILE_NAME CSPfmgen_Export.h
        	STATIC_DEFINE CSPfmgen_BUILT_AS_STATIC
	)
	set_std(CSPfmgen)

else()
  if(USE_DEVICES_SHARED_LIB)
      add_library(CSPfmgen SHARED
                 ${s_vm_fmgen_srcs}
		 )
      set_target_properties(CSPfmgen PROPERTIES 
                        SOVERSION ${THIS_LIB_VERSION}
			VERSION ${THIS_LIB_VERSION}
			)
      INSTALL(TARGETS CSPfmgen DESTINATION ${LIBCSP_INSTALL_DIR})
	set_std(CSPfmgen)
  else()
    add_library(fmgen STATIC
      ${s_vm_fmgen_srcs}
	)
	set_std(fmgen)
  endif()
endif()
