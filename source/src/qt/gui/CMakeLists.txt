message("* qt/gui")

set(THIS_LIB_VERSION 9.0.0)

set(s_qt_gui_headers
	  qt_dialogs.h
	  qt_gldraw.h
	  qt_glpack.h

	  gl/qt_glutil_gl_tmpl.h
	  gl2/qt_glutil_gl2_0.h
	  gl3/qt_glutil_gl3_0.h
	  gl4_5/qt_glutil_gl4_5.h
	  gles2/qt_glutil_gles_2.h

	  commonclasses.h
	  sound_dialog.h
	  mouse_dialog.h

	  virtualfileslist.h

	  mainwidget_base.h
	  menu_metaclass.h
	  menu_disk.h
	  menu_harddisk.h
	  menu_cmt.h
	  menu_cart.h
	  menu_quickdisk.h
	  menu_compactdisc.h
	  menu_laserdisc.h
	  menu_binary.h
	  menu_bubble.h
	  display_about.h
	  display_text_document.h
	  dropdown_keyset.h
	  dropdown_joystick.h
	  dropdown_jsbutton.h
	  dropdown_jspage.h

	  dropdown_joykey.h
	  dropdown_joykeybutton.h
	  dialog_set_key.h
	  dialog_set_key_combo.h
	  dialog_movie.h
	  dialog_memory.h

	  display_log.h

	  tab_movie_general.h
	  tab_movie_h264.h
	  tab_movie_mpeg4.h

	  dock_disks.h

	  draw_thread.h
	  joy_thread.h
	  ../emu_thread/emu_thread_tmpl.h

	  qt_debugger_tmpl.h
	  qt_lineeditplus.h
)


set(s_qt_gui_srcs
	  emuevents_control.cpp
	  qt_gldraw.cpp
	  qt_glutil.cpp
	  qt_glpack.cpp
	  gl/qt_glutil_gl_tmpl.cpp
	  gl2/qt_glutil_gl2_0.cpp
	  gl3/qt_glutil_gl3_0.cpp
	  gl4_5/qt_glutil_gl4_5.cpp
	  gles2/qt_glutil_gles_2.cpp
	  qt_glevents.cpp

	  virtualfileslist.cpp

	  qt_dialogs.cpp
	  qt_lineeditplus.cpp

	  qt_input.cpp
	  qt_drawitem.cpp
	  menu_control.cpp
	  menu_metaclass.cpp
	  menu_disk.cpp
	  menu_harddisk.cpp
	  menu_quickdisk.cpp
	  menu_compactdisc.cpp
	  menu_laserdisc.cpp
	  menu_cmt.cpp
	  menu_cart.cpp
	  menu_main.cpp
	  menu_emulator.cpp
	  menu_machine.cpp
	  menu_sound.cpp
	  menu_binary.cpp
	  menu_bubble.cpp
	  menu_screen.cpp
	  mainwindow_utils.cpp
	  util_fd.cpp
	  util_hdd.cpp
	  util_cd.cpp
	  util_ld.cpp
	  util_cmt.cpp
	  util_qd.cpp
	  util_cart.cpp
	  util_binary.cpp
	  util_bubble.cpp
	  display_statusbar.cpp
	  display_about.cpp
	  display_text_document.cpp
	  sound_dialog.cpp
	  mouse_dialog.cpp
	  dialog_memory.cpp

	  dropdown_keyset.cpp
	  dropdown_joystick.cpp
	  dropdown_jsbutton.cpp
	  dropdown_jspage.cpp

	  dropdown_joykey.cpp
	  dropdown_joykeybutton.cpp

	  dialog_set_key.cpp
	  dialog_set_key_combo.cpp
	  dialog_movie.cpp
	  display_log.cpp

	  tab_movie_general.cpp
	  tab_movie_h264.cpp
	  tab_movie_mpeg4.cpp

	  dock_disks.cpp

	  draw_thread.cpp
	  joy_thread.cpp

	  qt_debugger_tmpl.cpp

	  menu_flags_tmpl.cpp

	  util_fd2.cpp
	  util_hdd2.cpp

	  ../emu_thread/emu_thread_tmpl.cpp
	  ../emu_thread/utils.cpp
	  ../emu_thread/inputs.cpp
	  ../emu_thread/sequences.cpp
	  ../emu_thread/emu_thread_slots.cpp
)
add_definitions(-D__GUI_LIBRARY_NAME=\"libCSPgui.${THIS_LIB_VERSION}\")

MAKE_WRAP_CPP(s_qt_gui_headers_MOC ${s_qt_gui_headers})
if(CHECK_QT_6)
  QT_ADD_RESOURCES(s_qt_gui_RCC ${RESOURCE}
              ${CMAKE_CURRENT_SOURCE_DIR}/../common/qrc/commontexts.qrc
		      ${CMAKE_CURRENT_SOURCE_DIR}/../common/qrc/shaders.qrc
  		      ${CMAKE_CURRENT_SOURCE_DIR}/../common/qrc/i18n_global.qrc
		      ${CMAKE_CURRENT_SOURCE_DIR}/../common/qrc/icons.qrc
		      )
else()
  QT5_ADD_RESOURCES(s_qt_gui_RCC ${RESOURCE}
              ${CMAKE_CURRENT_SOURCE_DIR}/../common/qrc/commontexts.qrc
		      ${CMAKE_CURRENT_SOURCE_DIR}/../common/qrc/shaders.qrc
  		      ${CMAKE_CURRENT_SOURCE_DIR}/../common/qrc/i18n_global.qrc
		      ${CMAKE_CURRENT_SOURCE_DIR}/../common/qrc/icons.qrc
		      )
endif()

if(WIN32)
  include (GenerateExportHeader)
  add_library(CSPgui SHARED
	${s_qt_gui_srcs}
	${s_qt_gui_headers_MOC}
	${s_qt_gui_RCC}
	)

  if(CHECK_QT_6)
	target_link_libraries(CSPgui
	  PRIVATE 	  CSPosd
	  CSPavio
	  CSPemu_utils
	  Qt::Core
	  Qt::Gui
	  Qt::OpenGL
	  Qt::Widgets
	  ${SDL2_LIBRARIES}
	  ${ADDITIONAL_DLL_LIBRARIES}
	  )
  else()
	target_link_libraries(CSPgui
	  PRIVATE 	  CSPosd
	  CSPavio
	  CSPemu_utils
	  Qt5::Core
	  Qt5::Gui
	  Qt5::OpenGL
	  Qt5::Widgets
	  ${SDL2_LIBRARIES}
	  ${ADDITIONAL_DLL_LIBRARIES}
	  )
  endif()
  set_target_properties(CSPgui PROPERTIES
	SOVERSION ${THIS_LIB_VERSION}
	VERSION ${THIS_LIB_VERSION}
	LINK_INTERFACE_LIBRARIES ""
	)
  generate_export_header(CSPgui
    BASE_NAME CSPgui
    EXPORT_MACRO_NAME CSPgui_EXPORT
    EXPORT_FILE_NAME CSPgui_Export.h
    STATIC_DEFINE CSPgui_BUILT_AS_STATIC
	)
  set_std(CSPgui)
else()
  set(s_qt_gui_srcs ${s_qt_gui_srcs} qt_main.cpp)

  add_library(CSPgui SHARED
	${s_qt_gui_srcs}
	${s_qt_gui_headers_MOC}
	${s_qt_gui_RCC}
	)
  #add_precompiled_header(CSPgui pch_list
  #		SOURCE_CXX pch.cpp FORCEINCLUDE
  #)
  if(CHECK_QT_6)
	target_link_libraries(CSPgui PUBLIC
	  Qt::Core
	  Qt::Gui
	  Qt::OpenGL
	  Qt::Widgets
	  ${SDL2_LIBRARIES}
	  ${ADDITIONAL_DLL_LIBRARIES}
	  )
  else()
	target_link_libraries(CSPgui PUBLIC
	  Qt5::Core
	  Qt5::Gui
	  Qt5::OpenGL
	  Qt5::Widgets
	  ${SDL2_LIBRARIES}
	  ${ADDITIONAL_DLL_LIBRARIES}
	  )
  endif()

  set_target_properties(CSPgui PROPERTIES
	SOVERSION ${THIS_LIB_VERSION}
	VERSION ${THIS_LIB_VERSION}
	)
  INSTALL(TARGETS CSPgui DESTINATION ${LIBCSP_INSTALL_DIR})
  set_std(CSPgui)
endif()
