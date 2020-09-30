#
include(CheckFunctionExists)

# Still not as one shared lib with win32
if(WIN32)
	set(USE_DEVICES_SHARED_LIB OFF)
endif()
if(UNIX)
	include(GNUInstallDirs)
endif()
# Check HOST NAME
cmake_host_system_information(RESULT OSNAME QUERY OS_NAME)
cmake_host_system_information(RESULT OSVERSION QUERY OS_VERSION)
cmake_host_system_information(RESULT OSARCH QUERY OS_PLATFORM)
message("* HOST: OSNAME=" ${OSNAME} " RELEASE=" ${OSVERSION} " ARCH=" ${OSARCH} " OSARCH=" ${CMAKE_LIBRARY_ARCHITECTURE})

set(NEED_REPLACE_LIBDIR OFF)
#if((UNIX) AND (NOT DEFINED LIBCSP_INSTALL_DIR)) 
	# Modify LIBDIR if supports MULTI-ARCH.
	# ToDo: Another OSs i.e)Fedora
	if(EXISTS "/etc/lsb-release")
		file(READ "/etc/lsb-release" TMPDATA)
		string(TOUPPER "${TMPDATA}" U_TMPDATA)
		string(REGEX MATCH "UBUNTU" D_UBUNTU "${U_TMPDATA}")
		string(REGEX MATCH "DEBIAN" D_DEBIAN "${U_TMPDATA}")
#		message("*BUILD: UBUNTU=" ${D_UBUNTU} " DEBIAN=" ${D_DEBIAN})
	endif()
	if(("${D_DEBIAN}" STREQUAL "DEBIAN") OR ("${D_UBUNTU}" STREQUAL "UBUNTU"))
		set(NEED_REPLACE_LIBDIR ON)
#	elseif(("${OSVERSION}" MATCHES "^.*Debian.*$") OR ("${OSVERSION}" MATCHES "^.*Ubuntu.*$"))
#		# Fallback, but this may misdetect OS, by HOST when under DOCKER.
#		# Will not use.
#		set(NEED_REPLACE_LIBDIR ON)
	endif()
#endif()
if(NEED_REPLACE_LIBDIR)
	set(LIBCSP_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}")
	message("* CHANGE LIB_CSP_INSTALL_DIR TO " ${LIBCSP_INSTALL_DIR})
endif()

if(USE_DEVICES_SHARED_LIB)
  add_definitions(-DUSE_SHARED_DLL)
  add_definitions(-DUSE_SHARED_UI_DLL)
  add_definitions(-DUSE_SHARED_DEVICES_DLL)
endif()


#ccache
find_program(USE_CCACHE ccache)
if(USE_CCACHE)
   SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

if(WIN32)
  FIND_PACKAGE(Qt5Core REQUIRED)
else()
  FIND_PACKAGE(Qt5Widgets REQUIRED)
endif()
  FIND_PACKAGE(Qt5Gui REQUIRED)
  FIND_PACKAGE(Qt5OpenGL REQUIRED)
  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  include_directories(${Qt5Core_INCLUDE_DIRS})
  include_directories(${Qt5Gui_INCLUDE_DIRS})
  include_directories(${Qt5OpenGL_INCLUDE_DIRS})
  add_definitions(-D_USE_OPENGL -DUSE_OPENGL)
if(DEFINED QT5_ROOT_PATH)
  SET(CMAKE_FIND_ROOT_PATH  ${QT5_ROOT_PATH} ${CMAKE_FIND_ROOT_PATH})
endif()

#socket
function(APPEND_SOCKET_FEATURE)
  if(USE_SOCKET)
     FIND_PACKAGE(Qt5Network REQUIRED)
     include_directories(${Qt5Network_INCLUDE_DIRS})
   endif()
endfunction(APPEND_SOCKET_FEATURE)

SET(USE_QT_5 ON)
set(USE_QT5_4_APIS ON CACHE BOOL "Build with Qt5.4 (or later) APIs if you can.")
set(USE_GCC_OLD_ABI ON CACHE BOOL "Build with older GCC ABIs if you can.")
set(USE_SDL2 ON CACHE BOOL "Build with libSDL2. DIsable is building with libSDL1.")
set(USE_MOVIE_SAVER ON CACHE BOOL "Save screen/audio as MP4 MOVIE. Needs libav .")
set(USE_MOVIE_LOADER ON CACHE BOOL "Load movie from screen for some VMs. Needs libav .")
set(USE_LTO ON CACHE BOOL "Use link-time-optimization to build.")
set(USE_OPENMP OFF CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

if(USE_LTO)
   set_property(TARGET PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
else()
   set_property(TARGET PROPERTY INTERPROCEDURAL_OPTIMIZATION OFF)
endif()

add_definitions(-D_USE_QT5)

if(USE_QT5_4_APIS)
  add_definitions(-D_USE_QT_5_4)
else()
  #add_definitions(-DQT_NO_VERSION_TAGGING)
endif()

if(USE_GCC_OLD_ABI)
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
else()
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
endif()

SET(CMAKE_AUTOMOC OFF)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-D_USE_QT)
add_definitions(-DUSE_QT)
add_definitions(-DQT_MAJOR_VERSION=${Qt5Widgets_VERSION_MAJOR})
add_definitions(-DQT_MINOR_VERSION=${Qt5Widgets_VERSION_MINOR})

if(USE_OPENMP)
  find_package(OpenMP)
  include_directories(${OPENMP_INCLUDE_PATH})
  if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif()

find_package(Threads)
include_directories(${THREADS_INCLUDE_PATH})

include(FindPkgConfig)

find_package(Git)
if(GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD OUTPUT_VARIABLE __tstr)
	string(FIND ${__tstr} "fatal" __notfound)
	string(REPLACE "\n" "" __tstr2 ${__tstr})
	if(${__notfound} EQUAL -1)
		   add_definitions(-D__GIT_REPO_VERSION=\"${__tstr2}\")
	else()
		   add_definitions(-U__GIT_REPO_VERSION)
	endif()
endif()

string(TIMESTAMP __build_date "%b %d,%Y %H:%M:%S UTC" UTC)
add_definitions(-D__BUILD_DATE=\"${__build_date}\")

include(FindLibAV)
    if(LIBAV_FOUND)
      add_definitions(-DUSE_LIBAV)
      if(USE_MOVIE_SAVER)
        add_definitions(-DUSE_MOVIE_SAVER)
      endif()
      if(USE_MOVIE_LOADER)
        add_definitions(-DUSE_MOVIE_LOADER)
      endif()
      add_definitions(-D__STDC_CONSTANT_MACROS)
      add_definitions(-D__STDC_FORMAT_MACROS)
    else()
      set(USE_MOVIE_SAVER OFF)
      set(USE_MOVIE_LOADER OFF)
      set(LIBAV_LIBRARIES "")
    endif()
    
if(USE_SDL2)
   if(CMAKE_CROSSCOMPILING)
      include_directories(${SDL2_INCLUDE_DIRS})
   else()
      pkg_search_module(SDL2 REQUIRED  sdl2)
      include_directories(${SDL2_INCLUDE_DIRS})
   endif()
   set(SDL_LIBS ${SDL2_LIBRARIES})
   add_definitions(-DUSE_SDL2)
else()
   if(CMAKE_CROSSCOMPILING)
      include_directories(${SDL_INCLUDE_DIRS})
      set(SDL_LIBS ${SDL_LIBRARIES})
   else()
      include(FindSDL)
      #pkg_search_module(SDL REQUIRED sdl)
      #include_directories(${SDL_INCLUDE_DIRS})
      include_directories(${SDL_INCLUDE_DIR})
      set(SDL_LIBS ${SDL_LIBRARY})
   endif()
endif()

include(FindZLIB)
if(ZLIB_FOUND)
  add_definitions(-DUSE_ZLIB)
   include_directories(${ZLIB_INCLUDE_DIRS})
endif()

# GCC Only?
if(CMAKE_COMPILER_IS_GNUCC) 
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flax-vector-conversions")
endif()

if(CMAKE_COMPILER_IS_GNUCXX) 
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -flax-vector-conversions")
endif()


check_function_exists("nanosleep" HAVE_NANOSLEEP)
if(NOT HAVE_NANOSLEEP)
  check_library_exists("rt" "nanosleep" "" LIB_RT_HAS_NANOSLEEP)
endif(NOT HAVE_NANOSLEEP)

if(LIB_RT_HAS_NANOSLEEP)
  add_target_library(${EXEC_TARGET} rt)
endif(LIB_RT_HAS_NANOSLEEP)

if(HAVE_NANOSLEEP OR LIB_RT_HAS_NANOSLEEP)
  add_definitions(-DHAVE_NANOSLEEP)
endif(HAVE_NANOSLEEP OR LIB_RT_HAS_NANOSLEEP)


set(SRC_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../src)

if(USE_QT_5)
	if(NOT WIN32)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	endif()
endif()

#include(simd-x86)

if(LIBAV_FOUND)
	include_directories(${LIBAV_INCLUDE_DIRS})
endif()

# Now make DLLs
include_directories(
				"${PROJECT_SOURCE_DIR}/src"
				"${PROJECT_SOURCE_DIR}/src/vm"
				"${PROJECT_SOURCE_DIR}/src/qt/common"
				"${PROJECT_SOURCE_DIR}/src/qt/gui"
				"${PROJECT_SOURCE_DIR}/src/qt/debugger"
				"${PROJECT_SOURCE_DIR}/src/qt"
)

# Additional flags from toolchain.
string(TOUPPER "${CMAKE_BUILD_TYPE}" U_BUILD_TYPE)
if("${U_BUILD_TYPE}" MATCHES "RELWITHDEBINFO")
#	if("${CSP_ADDTIONAL_FLAGS_COMPILE_RELWITHDEBINFO}")
		add_compile_options(${CSP_ADDTIONAL_FLAGS_COMPILE_RELWITHDEBINFO})
#	endif()
#	if("${CSP_ADDTIONAL_FLAGS_LINK_RELWITHDEBINFO}")
		add_link_options(${CSP_ADDTIONAL_FLAGS_LINK_RELWITHDEBINFO})
#	endif()
elseif("${U_BUILD_TYPE}" MATCHES "RELEASE")
	if("${CSP_ADDTIONAL_FLAGS_COMPILE_RELEASE}")
		add_compile_options(${CSP_ADDTIONAL_FLAGS_COMPILE_RELEASE}")
	endif()
	if("${CSP_ADDTIONAL_FLAGS_LINK_RELEASE}")
		add_link_options(${CSP_ADDTIONAL_FLAGS_LINK_RELEASE})
	endif()
elseif("${U_BUILD_TYPE}" MATCHES "DEBUG")
	if("${CSP_ADDTIONAL_FLAGS_COMPILE_DEBUG}")
		add_compile_options(${CSP_ADDTIONAL_FLAGS_COMPILE_DEBUG}")
	endif()
	if("${CSP_ADDTIONAL_FLAGS_LINK_DEBUG}")
		add_link_options(${CSP_ADDTIONAL_FLAGS_LINK_DEBUG})
	endif()
endif()

add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt" osd)
add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt/avio" qt/avio)
add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt/gui" qt/gui)
add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt/emuutils" emu_utils)
if(USE_DEVICES_SHARED_LIB)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/vm/common_vm" vm/)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/vm/fmgen" vm/fmgen)
else()
	add_subdirectory("${PROJECT_SOURCE_DIR}/src" common)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/vm" vm)
endif()

function(ADD_VM VM_NAME EXE_NAME VMDEF)
	set(COMMON_DIRECTORY ${PROJECT_SOURCE_DIR}/src/qt/common)
	set(s_qt_common_headers
		${COMMON_DIRECTORY}/emu_thread.h
		${COMMON_DIRECTORY}/mainwidget.h
		${COMMON_DIRECTORY}/../osd.h
	)
	if(USE_SOCKET)
		set(s_qt_common_headers ${s_qt_common_headers} ${COMMON_DIRECTORY}/../osd_socket.h)
	endif()
	QT5_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})
	set(QT_COMMON_BASE
		${COMMON_DIRECTORY}/main.cpp
		${COMMON_DIRECTORY}/qt_utils.cpp
		${COMMON_DIRECTORY}/menu_flags.cpp
		${COMMON_DIRECTORY}/emu_thread.cpp
		${COMMON_DIRECTORY}/emu_thread_slots.cpp
		${COMMON_DIRECTORY}/util_bubble2.cpp
		${COMMON_DIRECTORY}/util_main.cpp
		${COMMON_DIRECTORY}/../osd.cpp
		${COMMON_DIRECTORY}/../osd_wrapper.cpp
	)

	if(WIN32)
		add_executable(${EXE_NAME} WIN32
			${PROJECT_SOURCE_DIR}/src/vm/event.cpp
			${VMFILES}
			${PROJECT_SOURCE_DIR}/src/emu.cpp
			${COMMON_DIRECTORY}/../gui/qt_main.cpp
			${QT_COMMON_BASE}
			${s_qt_common_headers_MOC}
		)
	else()
		add_executable(${EXE_NAME} 
			${PROJECT_SOURCE_DIR}/src/vm/event.cpp
			${VMFILES}
			${PROJECT_SOURCE_DIR}/src/emu.cpp
			${QT_COMMON_BASE}
			${s_qt_common_headers_MOC}
		)
    endif()
	if(USE_SOCKET)
		QT5_USE_MODULES(${EXE_NAME} Widgets Core Gui OpenGL Network)
	else()
		QT5_USE_MODULES(${EXE_NAME} Widgets Core Gui OpenGL)
	endif()
	target_include_directories(${EXE_NAME} 
		PRIVATE "${PROJECT_SOURCE_DIR}/src/qt/machines/${VM_NAME}"
		PRIVATE "${PROJECT_SOURCE_DIR}/src/vm/${VM_NAME}"
	)
   if(LIBAV_FOUND)
		target_include_directories(${EXE_NAME} PUBLIC
					"${PROJECT_SOURCE_DIR}/src/qt/avio"
		)
	endif()
#	if(WITH_DEBUGGER)
		set(DEBUG_LIBS qt_debugger_${EXE_NAME})
#		target_include_directories(${EXE_NAME} PUBLIC
#					PUBLIC "${PROJECT_SOURCE_DIR}/src/qt/debugger"
#		)
#	else()
#		set(DEBUG_LIBS)
#	endif()
	if(NOT USE_DEVICES_SHARED_LIB)
		if(USE_FMGEN)
			set(VM_APPEND_LIBS fmgen)
		else()
			set(VM_APPEND_LIBS)
		endif()
	endif()
	if(WIN32)
		set(LOCAL_LIBS     
			common_emu
			qt_${EXE_NAME}
			vm_${EXE_NAME}
			vm_vm
			${VM_APPEND_LIBS}
			${DEBUG_LIBS}
			common_${EXE_NAME}
		)
	else()
		set(LOCAL_LIBS
			qt_${EXE_NAME}
			vm_${EXE_NAME}
			vm_vm
			${VM_APPEND_LIBS}
			${DEBUG_LIBS}
			common_${EXE_NAME}
		)
	endif()
	if(WIN32)
		set(BUNDLE_LIBS
			${OPENGL_LIBRARY}
			${OPENCL_LIBRARY}
			${GETTEXT_LIBRARY}
			${OPENMP_LIBRARY}
			${LIBAV_LIBRARIES}
			${SDL_LIBS}
			${LIBAV_LIBRARIES}
			${ADDITIONAL_LIBRARIES}
			${ZLIB_LIBRARIES}
		)
	else()		
		add_definitions(-D_UNICODE)
		set(BUNDLE_LIBS 
			${OPENMP_LIBRARY}
			${SDL_LIBS}
			${ADDITIONAL_LIBRARIES}
			${BUNDLE_LIBS}
			${QT_LIBRARIES}
			${ZLIB_LIBRARIES}
			${THREADS_LIBRARY}
       )
	endif()
	if(WIN32)
	   set(LOCAL_LIBS     
           qt_${EXE_NAME}
		   vm_${EXE_NAME}
		   vm_vm
		   ${VM_APPEND_LIBS}
		   ${DEBUG_LIBS}
		   common_${EXE_NAME}
		   )
	else()
	   set(LOCAL_LIBS     
           qt_${EXE_NAME}
		   vm_${EXE_NAME}
		   ${VM_APPEND_LIBS}
		   ${DEBUG_LIBS}
		   common_${EXE_NAME}
		   )
	endif()
	if(USE_DEVICES_SHARED_LIB)
		set(BUNDLE_LIBS
			${BUNDLE_LIBS}
			CSPosd
			CSPcommon_vm
			CSPfmgen
			CSPgui
			CSPemu_utils
			CSPavio
		)
	else()
		set(BUNDLE_LIBS
			${BUNDLE_LIBS}
#			-lCSPosd
#			-lCSPgui
#			-lCSPavio
		)
	endif()

	# Subdirectories
	add_subdirectory("${PROJECT_SOURCE_DIR}/src" common/${EXE_NAME} EXCLUDE_FROM_ALL)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/vm/${VM_NAME}" vm/${EXE_NAME} EXCLUDE_FROM_ALL)
	if(NOT USE_DEVICES_SHARED_LIB)
		if(USE_FMGEN)
			add_subdirectory("${PROJECT_SOURCE_DIR}/src/vm/fmgen" vm/fmgen_${EXE_NAME}  EXCLUDE_FROM_ALL)
		endif()
	endif()
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt/machines/${VM_NAME}" qt/${EXE_NAME}  EXCLUDE_FROM_ALL)
#	if(WITH_DEBUGGER)
		add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt/debugger" qt/debugger_${EXE_NAME} EXCLUDE_FROM_ALL)
#	endif()
#	add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt/common" qt/common EXCLUDE_FROM_ALL)
	add_dependencies(${EXE_NAME}
		CSPosd
		CSPcommon_vm
		CSPfmgen
		common_${EXE_NAME}
		CSPemu_utils
		CSPgui
		CSPavio
		qt_debugger_${EXE_NAME}
		qt_${EXE_NAME}
	)
	target_compile_definitions(${EXE_NAME}
		PRIVATE  ${VMDEF}
	)
	target_compile_definitions(vm_${EXE_NAME}
		PRIVATE  ${VMDEF}
	)
	target_compile_definitions(qt_${EXE_NAME}
		PUBLIC ${VMDEF}
	)
	target_compile_definitions(qt_debugger_${EXE_NAME}
		PRIVATE  ${VMDEF}
	)
	target_compile_definitions(common_${EXE_NAME}
		PRIVATE  ${VMDEF}
	)

	if(WIN32)
		target_link_libraries(${EXE_NAME}
			${LOCAL_LIBS}
			${BUNDLE_LIBS}
			-lpthread
		)
	else()	
		target_link_libraries(${EXE_NAME}
			${LOCAL_LIBS}
			${BUNDLE_LIBS}
			-lpthread)
	endif()
	install(TARGETS ${EXE_NAME} 
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION "${CMAKE_LIBRARY_ARCHITECTURE}"
		ARCHIVE DESTINATION "${CMAKE_LIBRARY_ARCHITECTURE}"
	)
endfunction()


