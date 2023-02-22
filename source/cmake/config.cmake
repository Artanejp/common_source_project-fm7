#
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)
include(CheckCXXCompilerFlag)

# Still not as one shared lib with win32
if(WIN32)
	set(USING_TOOLCHAIN_GCC_DEBIAN OFF CACHE BOOL "Workaround for Debian's mingw-w64 linker.")
endif()
if(UNIX)
	include(GNUInstallDirs)
endif()

check_cxx_compiler_flag("-std=c++20" HAS_STD_CXX20)
if(HAS_STD_CXX20)
	set(CSP_BUILD_WITH_CXX20 OFF CACHE BOOL "Build with C++20 specification compilers.This is a very experimental feature.If not set, will build with C++11 specification.")
else()
	set(CSP_BUILD_WITH_CXX20 OFF)
endif()
# Note: Belows are temporally disabled, not implemented older CMake.
# Check HOST NAME
#cmake_host_system_information(RESULT OSNAME QUERY OS_NAME)
#cmake_host_system_information(RESULT OSVERSION QUERY OS_VERSION)
#cmake_host_system_information(RESULT OSARCH QUERY OS_PLATFORM)
#message("* HOST: OSNAME=" ${OSNAME} " RELEASE=" ${OSVERSION} " ARCH=" ${OSARCH} " OSARCH=" ${CMAKE_LIBRARY_ARCHITECTURE})

set(NEED_REPLACE_LIBDIR OFF)
if((UNIX) AND (NOT DEFINED LIBCSP_INSTALL_DIR))
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
endif()
if(NEED_REPLACE_LIBDIR)
	set(LIBCSP_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}")
	message("* CHANGE LIB_CSP_INSTALL_DIR TO " ${LIBCSP_INSTALL_DIR})
endif()


if(USE_DEVICES_SHARED_LIB)
  add_definitions(-DUSE_SHARED_DLL)
  add_definitions(-DUSE_SHARED_UI_DLL)
  add_definitions(-DUSE_SHARED_DEVICES_DLL)
  add_definitions(-DUSE_FIXED_CONFIG)
elseif(WIN32)
  add_definitions(-DUSE_SHARED_DLL)
  add_definitions(-DUSE_SHARED_UI_DLL)
endif()

set(USE_FMGEN ON)
set(WITH_DEBUGGER ON)
set(WITH_MOUSE ON)
set(WITH_JOYSTICK ON)

include(detect_target_cpu)
# set entry

add_definitions(-D_CONFIGURE_WITH_CMAKE)
#ccache
find_program(USE_CCACHE ccache)
if(USE_CCACHE)
   SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

if(DEFINED QT6_ROOT_PATH)
  SET(CMAKE_FIND_ROOT_PATH  ${QT6_ROOT_PATH} ${CMAKE_FIND_ROOT_PATH})
elseif(DEFINED QT5_ROOT_PATH)
  SET(CMAKE_FIND_ROOT_PATH  ${QT5_ROOT_PATH} ${CMAKE_FIND_ROOT_PATH})
endif()


SET(USE_QT_5 ON CACHE BOOL "Build with Qt5 API.If set USE_QT_6 and Qt6 exists, not effectable.")
SET(USE_QT_6 OFF CACHE BOOL "Build with Qt6.If not available, will fallback to Qt5.")

set(CHECK_QT_6 OFF)
if((CMAKE_VERSION VERSION_GREATER_EQUAL 3.16) AND (USE_QT_6))
	FIND_PACKAGE(Qt6 COMPONENTS Core Widgets Gui OpenGL OpenGLWidgets Network Multimedia Core5Compat)
	if(Qt6_FOUND)
		SET(CHECK_QT_6 ON)
	endif()
endif()

if(NOT CHECK_QT_6)
	FIND_PACKAGE(Qt5 COMPONENTS Core Widgets Gui OpenGL Network Multimedia REQUIRED)
endif()

if(CHECK_QT_6)
	set(WITH_QT_VERSION_MAJOR 6)
	include_directories(${Qt6Widgets_INCLUDE_DIRS})
	include_directories(${Qt6Core_INCLUDE_DIRS})
	include_directories(${Qt6Gui_INCLUDE_DIRS})
	include_directories(${Qt6OpenGL_INCLUDE_DIRS})
	include_directories(${Qt6OpenGLWidgets_INCLUDE_DIRS})
	include_directories(${Qt6Network_INCLUDE_DIRS})
	include_directories(${Qt6Multimedia_INCLUDE_DIRS})
	include_directories(${Qt6Core5Compat_INCLUDE_DIRS})
	add_definitions(-D_USE_QT6)
else()
# Qt5
	set(WITH_QT_VERSION_MAJOR 5)
	include_directories(${Qt5Widgets_INCLUDE_DIRS})
	include_directories(${Qt5Core_INCLUDE_DIRS})
	include_directories(${Qt5Gui_INCLUDE_DIRS})
	include_directories(${Qt5OpenGL_INCLUDE_DIRS})
	include_directories(${Qt5Network_INCLUDE_DIRS})
	include_directories(${Qt5Multimedia_INCLUDE_DIRS})
	add_definitions(-D_USE_QT5)
endif()

#socket
function(APPEND_SOCKET_FEATURE)
  if(${USE_SOCKET_${EXE_NAME}})
	if(CHECK_QT_6)
      FIND_PACKAGE(Qt6Network REQUIRED)
      include_directories(${Qt6Network_INCLUDE_DIRS})
	else()
     FIND_PACKAGE(Qt5Network REQUIRED)
     include_directories(${Qt5Network_INCLUDE_DIRS})
	 endif()
   endif()
endfunction(APPEND_SOCKET_FEATURE)


if(CHECK_QT_6)
  set(USE_QT5_4_APIS ON)
else()
  set(USE_QT5_4_APIS  ON  CACHE BOOL "Build with Qt5.4 (or later) APIs if you can.")
endif()

set(USE_SDL2 ON CACHE BOOL "Build with libSDL2. DIsable is building with libSDL1.")
set(USE_MOVIE_SAVER ON CACHE BOOL "Save screen/audio as MP4 MOVIE. Needs libav .")
set(USE_MOVIE_LOADER ON CACHE BOOL "Load movie from screen for some VMs. Needs libav .")
set(USE_GCC_OLD_ABI OFF CACHE BOOL "Build with older GCC ABIs if you can.")
set(USE_LTO ON CACHE BOOL "Use link-time-optimization to build.")
#set(USE_OPENMP OFF CACHE BOOL "Build using OpenMP")
set(USE_OPENMP OFF)
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

if(USE_OPENGL)
	add_definitions(-D_USE_OPENGL -DUSE_OPENGL)
endif()
#set(IS_ENABLE_LTO FALSE)
#if(CMAKE_VERSION VERSION_GREATER 3.8)
#  if(USE_LTO)
#	include(CheckIPOSupported)
#  endif()
#endif()

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

# 20210914 K.O Start to migrate for Qt6.
# See, https://doc.qt.io/qt-6/portingguide.html
if(CHECK_QT_6)
	#Qt6 or later
    add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060200)
else()
  if(Qt5Widgets_VERSION VERSION_GREATER 5.15)
      add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)
  endif()
endif()

if(CHECK_QT_6)
  add_definitions(-DQT_MAJOR_VERSION=${Qt6Widgets_VERSION_MAJOR})
  add_definitions(-DQT_MINOR_VERSION=${Qt6Widgets_VERSION_MINOR})
else()
  add_definitions(-DQT_MAJOR_VERSION=${Qt5Widgets_VERSION_MAJOR})
  add_definitions(-DQT_MINOR_VERSION=${Qt5Widgets_VERSION_MINOR})
endif()

if(USE_OPENMP)
  find_package(OpenMP)
  include_directories(${OPENMP_INCLUDE_PATH})
  if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif()

#find_package(Threads)
#include_directories(${THREADS_INCLUDE_PATH})
include(FindThreads)
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

if(CHECK_QT_6)
	if(NOT WIN32)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	endif()
else()
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
function(additional_link_options n_target)
	string(TOUPPER "${CMAKE_BUILD_TYPE}" U_BUILD_TYPE)
	if("${U_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO")
		if(DEFINED CSP_ADDTIONAL_FLAGS_LINK_RELWITHDEBINFO)
			target_link_options(${n_target}
				PRIVATE ${CSP_ADDTIONAL_FLAGS_COMPILE_RELWITHDEBINFO}
			)
		endif()
	elseif("${U_BUILD_TYPE}" STREQUAL "RELEASE")
		if(DEFINED CSP_ADDTIONAL_FLAGS_LINK_RELEASE)
			target_link_options(${n_target}
				PRIVATE ${CSP_ADDTIONAL_FLAGS_COMPILE_RELEASE}
			)
		endif()
	elseif("${U_BUILD_TYPE}" STREQUAL "DEBUG")
		if(DEFINED CSP_ADDTIONAL_FLAGS_LINK_DEBUG)
			target_link_options(${n_target}
				PRIVATE ${CSP_ADDTIONAL_FLAGS_COMPILE_DEBUG}
			)
		endif()
	endif()
endfunction(additional_link_options)

function(additional_options n_target)
	string(TOUPPER "${CMAKE_BUILD_TYPE}" U_BUILD_TYPE)
	if("${U_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO")
		if(DEFINED CSP_ADDTIONAL_FLAGS_COMPILE_RELWITHDEBINFO)
			target_compile_options(${n_target}
				PRIVATE ${CSP_ADDTIONAL_FLAGS_COMPILE_RELWITHDEBINFO}
			)
		endif()
	elseif("${U_BUILD_TYPE}" STREQUAL "RELEASE")
		if(DEFINED CSP_ADDTIONAL_FLAGS_COMPILE_RELEASE)
			target_compile_options(${n_target}
				PRIVATE ${CSP_ADDTIONAL_FLAGS_COMPILE_RELEASE}
			)
		endif()
	elseif("${U_BUILD_TYPE}" STREQUAL "DEBUG")
		if(DEFINED CSP_ADDTIONAL_FLAGS_COMPILE_DEBUG)
			target_compile_options(${n_target}
				PRIVATE ${CSP_ADDTIONAL_FLAGS_COMPILE_DEBUG}
			)
		endif()
	endif()
endfunction(additional_options)

#ToDo: MSVC.
#if(CMAKE_VERSION VERSION_LESS "3.1")
if((CSP_BUILD_WITH_CXX20) AND (HAS_STD_CXX20))
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
else()
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
endif()


macro(MAKE_WRAP_CPP)
  if(CHECK_QT_6)
	QT_WRAP_CPP(${ARGV})
  else()
	QT5_WRAP_CPP(${ARGV})
  endif()
endmacro(MAKE_WRAP_CPP)

function(set_std TARGET)
#	if(CMAKE_VERSION VERSION_LESS "3.1")
#	else()
#		set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 11)
#		set_property(TARGET ${TARGET} PROPERTY C_STANDARD 11)
#	endif()
endfunction(set_std)


add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt" osd)
add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt/avio" qt/avio)
add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt/gui" qt/gui)
add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt/emuutils" emu_utils)

if(USE_DEVICES_SHARED_LIB)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/vm/common_vm" vm/)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/vm/fmgen" vm/fmgen)
else()
	add_subdirectory("${PROJECT_SOURCE_DIR}/src" common)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/vm/common_vm" vm/)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/vm/fmgen" vm/fmgen)
endif()


function(ADD_VM VM_NAME EXE_NAME VMDEF)
	set(COMMON_DIRECTORY ${PROJECT_SOURCE_DIR}/src/qt/common)
	set(s_qt_common_headers
		${COMMON_DIRECTORY}/emu_thread.h
		${COMMON_DIRECTORY}/mainwidget.h
		${PROJECT_SOURCE_DIR}/src/qt/osd.h
	)
  if(CHECK_QT_6)
	QT_ADD_RESOURCES(RESOURCE_${EXE_NAME} ${RESOURCE})
	QT_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})
  else()
	QT5_ADD_RESOURCES(RESOURCE_${EXE_NAME} ${RESOURCE})
	QT5_WRAP_CPP(s_qt_common_headers_MOC ${s_qt_common_headers})
  endif()
	set(QT_COMMON_BASE
		${COMMON_DIRECTORY}/main.cpp
		${COMMON_DIRECTORY}/qt_utils.cpp
		${COMMON_DIRECTORY}/menu_flags.cpp
		${COMMON_DIRECTORY}/emu_thread.cpp
		${COMMON_DIRECTORY}/mainwidget.cpp

		${COMMON_DIRECTORY}/../osd.cpp
		${COMMON_DIRECTORY}/../osd_wrapper.cpp
	)
	if(${USE_SOCKET_${EXE_NAME}})
			set(QT_COMMON_BASE
				${QT_COMMON_BASE}
				${s_qt_net_headers_MOC}
			)
	endif()
	if(WIN32)
		add_executable(${EXE_NAME} WIN32
			${PROJECT_SOURCE_DIR}/src/vm/event.cpp
			${VMFILES}
			${PROJECT_SOURCE_DIR}/src/emu.cpp
			${COMMON_DIRECTORY}/../gui/qt_main.cpp
			${s_qt_common_headers_MOC}
			${RESOURCE_${EXE_NAME}}
			${QT_COMMON_BASE}
	)
	else()
		add_executable(${EXE_NAME}
			${PROJECT_SOURCE_DIR}/src/vm/event.cpp
			${PROJECT_SOURCE_DIR}/src/emu.cpp
			${QT_COMMON_BASE}
			${s_qt_common_headers_MOC}
			${RESOURCE_${EXE_NAME}}
		)
    endif()

	if(CHECK_QT_6)
	  set(QT_LIBRARIES ${QT_LIBRARIES}
		Qt6::Widgets Qt6::Core Qt6::Gui Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Multimedia Qt6::Network Qt6::Core5Compat)
    else()
	  set(QT_LIBRARIES ${QT_LIBRARIES}
		Qt5::Widgets Qt5::Core Qt5::Gui Qt5::OpenGL Qt5::Multimedia Qt5::Network)
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
			set(VM_APPEND_LIBS CSPfmgen)
		else()
			set(VM_APPEND_LIBS)
		endif()
	endif()
	if(WIN32)
		set(BUNDLE_LIBS
			${OPENGL_LIBRARY}
			${OPENCL_LIBRARY}
			${GETTEXT_LIBRARY}
			${OPENMP_LIBRARY}
			${LIBAV_LIBRARIES}
			${SDL_LIBS}
			${ADDITIONAL_LIBRARIES}
			${QT_LIBRARIES}
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
#		   vm_common_vm
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
			CSPosd
			CSPcommon_vm
			CSPfmgen
			CSPgui
			CSPemu_utils
			CSPavio
			${BUNDLE_LIBS}
		)
	else()
		set(BUNDLE_LIBS
			${BUNDLE_LIBS}
			CSPosd
#			CSPfmgen
			CSPgui
			CSPemu_utils
			CSPavio
		)
	endif()

	# Subdirectories
	add_subdirectory("${PROJECT_SOURCE_DIR}/src" common/${EXE_NAME} EXCLUDE_FROM_ALL)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/vm/${VM_NAME}" vm/${EXE_NAME} EXCLUDE_FROM_ALL)
	set_std(vm_${EXE_NAME})
	if(NOT USE_DEVICES_SHARED_LIB)
		if(USE_FMGEN)
#			add_subdirectory("${PROJECT_SOURCE_DIR}/src/vm/fmgen" vm/fmgen_${EXE_NAME}  EXCLUDE_FROM_ALL)
		endif()
	endif()
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt/machines/${VM_NAME}" qt/${EXE_NAME}  EXCLUDE_FROM_ALL)
	set_std(qt_${EXE_NAME})

#	if(WITH_DEBUGGER)
		add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt/debugger" qt/debugger_${EXE_NAME} EXCLUDE_FROM_ALL)
#	endif()
#	add_subdirectory("${PROJECT_SOURCE_DIR}/src/qt/common" qt/common EXCLUDE_FROM_ALL)
	set_std(${EXE_NAME})
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

#	additional_options(common_${EXE_NAME})
#	additional_options(vm_${EXE_NAME})
#	additional_options(qt_${EXE_NAME})
#	additional_options(qt_debug_${EXE_NAME})

#	additional_options(${EXE_NAME})
#	additional_link_options(${EXE_NAME})

	if(WIN32)
		# Note: With Debian's foo-mingw-w64-g++, needs below workaround
		# due to problems of linker.
		if(USING_TOOLCHAIN_GCC_DEBIAN)
			set(DUPLICATE_LINK_LIB CSPcommon_vm CSPgui)
		endif()
		target_link_libraries(${EXE_NAME}
			${BUNDLE_LIBS}
			${LOCAL_LIBS}
			${DUPLICATE_LINK_LIB}
#			libpthread.a
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
