##
## Compile Gettext's I18N catalogue (foo.po to out.mo)
##  And install to dest.
##  (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
## requires find_package(Gettext) befor define this.
## License: Apache 2
##
cmake_minimum_required (VERSION 2.6)

function(compile_i18n_po_to_mo in target)
  if(GETTEXT_FOUND)
     add_custom_target(${target} ALL 
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${in}
		     COMMAND  ${GETTEXT_MSGFMT_EXECUTABLE}
		              ${CMAKE_CURRENT_SOURCE_DIR}/${in}
		              -o ${CMAKE_CURRENT_BINARY_DIR}/messages.mo
		     )
   endif()
endfunction(compile_i18n_po_to_mo)


function(install_i18n_catalogue in dest)
    if(GETTEXT_FOUND)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${in}
                    ${CMAKE_CURRENT_BINARY_DIR}/messages.mo
             DESTINATION ${dest}/LC_MESSAGES)
    endif()
endfunction(install_i18n_catalogue)

