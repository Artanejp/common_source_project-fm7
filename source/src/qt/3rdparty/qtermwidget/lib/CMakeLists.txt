cmake_minimum_required( VERSION 2.8 )
message("* qt/3rdparty/qtermwidget/lib")

#project(qtermwidget)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# just change version for releases
set(QTERMWIDGET_VERSION_MAJOR "0")
set(QTERMWIDGET_VERSION_MINOR "6")
set(QTERMWIDGET_VERSION_PATCH "0")

set(QTERMWIDGET_VERSION "${QTERMWIDGET_VERSION_MAJOR}.${QTERMWIDGET_VERSION_MINOR}.${QTERMWIDGET_VERSION_PATCH}")


#include(CheckFunctionExists)
#include(GNUInstallDirs)

#include_directories(
#    "${CMAKE_SOURCE_DIR}/lib"
#    "${CMAKE_BINARY_DIR}/lib"
#    "${CMAKE_BINARY_DIR}"
#)
add_definitions(-Wall)


if(USE_QT5)
    set(QTERMWIDGET_LIBRARY_NAME qtermwidget5)
    include(../cmake/qtermwidget5_use)
else()
    include(../cmake/qtermwidget4_use)
    set(QTERMWIDGET_LIBRARY_NAME qtermwidget4)
endif()


# main library

set(SRCS
    BlockArray.cpp
    ColorScheme.cpp
    Emulation.cpp
    Filter.cpp
    History.cpp
    HistorySearch.cpp
    KeyboardTranslator.cpp
    konsole_wcwidth.cpp
    kprocess.cpp
    kpty.cpp
    kptydevice.cpp
    kptyprocess.cpp
    Pty.cpp
    qtermwidget.cpp
    Screen.cpp
    ScreenWindow.cpp
    SearchBar.cpp
    Session.cpp
    ShellCommand.cpp
    TerminalCharacterDecoder.cpp
    TerminalDisplay.cpp
    tools.cpp
    Vt102Emulation.cpp
)

# Only the Headers that need to be moc'd go here
set(HDRS
    Emulation.h
    Filter.h
    HistorySearch.h
    kprocess.h
    kptydevice.h
    kptyprocess.h
    Pty.h
    qtermwidget.h
    ScreenWindow.h
    SearchBar.h
    Session.h
    TerminalDisplay.h
    Vt102Emulation.h
)

set(UI
    SearchBar.ui
)

# for distribution
set(HDRS_DISTRIB
    qtermwidget.h
    Filter.h
)

# dirs
set(KB_LAYOUT_DIR "${CMAKE_INSTALL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/kb-layouts/")
message(STATUS "Keyboard layouts will be installed in: ${KB_LAYOUT_DIR}")
add_definitions(-DKB_LAYOUT_DIR="${CMAKE_INSTALL_PREFIX}/${KB_LAYOUT_DIR}")

set(COLORSCHEMES_DIR "${CMAKE_INSTALL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/color-schemes/")
message(STATUS "Color schemes will be installed in: ${COLORSCHEMES_DIR}" )
add_definitions(-DCOLORSCHEMES_DIR="${CMAKE_INSTALL_PREFIX}/${COLORSCHEMES_DIR}")

set(QTERMWIDGET_INCLUDE_DIR
"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/${QTERMWIDGET_LIBRARY_NAME} ")

#| Defines
add_definitions(-DHAVE_POSIX_OPENPT -DHAVE_SYS_TIME_H)
if(APPLE)
    add_definitions(-DHAVE_UTMPX -D_UTMPX_COMPAT)
endif()
CHECK_FUNCTION_EXISTS(updwtmpx HAVE_UPDWTMPX)
if(HAVE_UPDWTMPX)
    add_definitions(-DHAVE_UPDWTMPX)
endif()


if(USE_QT5)
    qt5_wrap_cpp(MOCS ${HDRS})
    qt5_wrap_ui(UI_SRCS ${UI})
    set(PKG_CONFIG_REQ "Qt5Core, Qt5Xml, Qt5Widgets")
else()
    qt4_wrap_cpp(MOCS ${HDRS})
    qt4_wrap_ui(UI_SRCS ${UI})
    set(PKG_CONFIG_REQ "QtCore, QtXml")
endif()


#add_library(${QTERMWIDGET_LIBRARY_NAME} SHARED ${SRCS} ${MOCS} ${UI_SRCS})
add_library(${QTERMWIDGET_LIBRARY_NAME} ${SRCS} ${MOCS} ${UI_SRCS})
include_directories(${PROJECT_BINARY_DIR}/qt/qtermwidget)

#target_link_libraries(${QTERMWIDGET_LIBRARY_NAME} ${QTERMWIDGET_QT_LIBRARIES})
#set_target_properties( ${QTERMWIDGET_LIBRARY_NAME} PROPERTIES
#                       SOVERSION ${QTERMWIDGET_VERSION_MAJOR}
#                       VERSION ${QTERMWIDGET_VERSION}
#                     )
if(APPLE)
    set (CMAKE_SKIP_RPATH 1)
    # this is a must to load the lib correctly
    set_target_properties( ${QTERMWIDGET_LIBRARY_NAME} PROPERTIES INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} )
endif()

#install(TARGETS ${QTERMWIDGET_LIBRARY_NAME} DESTINATION "${CMAKE_INSTALL_LIBDIR}")
#install(FILES ${HDRS_DISTRIB} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${QTERMWIDGET_LIBRARY_NAME}")
# keyboard layouts
install(DIRECTORY lib/kb-layouts/ DESTINATION "${KB_LAYOUT_DIR}" FILES_MATCHING PATTERN "*.keytab" )
# color schemes
install(DIRECTORY lib/color-schemes/ DESTINATION "${COLORSCHEMES_DIR}" FILES_MATCHING PATTERN "*.*schem*")

include(create_pkgconfig_file)

create_pkgconfig_file(${QTERMWIDGET_LIBRARY_NAME}
    "QTermWidget library for Qt ${QTERMWIDGET_VERSION_MAJOR}.x"
    ${PKG_CONFIG_REQ}
    ${QTERMWIDGET_LIBRARY_NAME}
    ${QTERMWIDGET_VERSION}
)

configure_file(
    "../cmake/${QTERMWIDGET_LIBRARY_NAME}-config.cmake.in"
    "${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-config.cmake"
    @ONLY
)
install(FILES
    "../${QTERMWIDGET_LIBRARY_NAME}-config.cmake"
    "${CMAKE_SOURCE_DIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}_use.cmake"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}"
)
# end of main library


# designer plugin
if (BUILD_DESIGNER_PLUGIN)
    if(USE_QT5)
        message(FATAL_ERROR "Building Qt designer plugin is not supported for Qt5 yet. Use -DBUILD_DESIGNER_PLUGIN=0")
    endif()
    message(STATUS "Building Qt designer plugin")

    include_directories(designer "${QT_QTDESIGNER_INCLUDE_DIR}")

    set(DESIGNER_SRC designer/qtermwidgetplugin.cpp)
    qt4_wrap_cpp(DESIGNER_MOC  designer/qtermwidgetplugin.h)
    qt4_add_resources(DESIGNER_QRC designer/qtermwidgetplugin.qrc)

    link_directories(${CMAKE_BINARY_DIR})
    add_library(qtermwidget4plugin SHARED
        ${DESIGNER_MOC}
        ${DESIGNER_QRC}
        ${DESIGNER_SRC}
    )
    add_dependencies(qtermwidget4plugin qtermwidget4)

    target_link_libraries(qtermwidget4plugin
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDESIGNER_LIBRARY}
        ${QT_QTDESIGNERCOMPONENTS_LIBRARY}
        ${QTERMWIDGET_LIBRARY_NAME}
    )

    if(APPLE)
        # this is a must to load the lib correctly
        set_target_properties(qtermwidget4plugin PROPERTIES
            INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/qt4/plugins/designer"
        )
    endif()

    install(TARGETS qtermwidget4plugin DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/qt4/plugins/designer")

endif (BUILD_DESIGNER_PLUGIN)
# end of designer plugin



