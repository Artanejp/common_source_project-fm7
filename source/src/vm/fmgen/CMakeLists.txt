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
add_library(vm_fmgen
    ${s_vm_fmgen_srcs}
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
