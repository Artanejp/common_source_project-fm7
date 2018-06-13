## From:
##https://qiita.com/mrk_21/items/264f6135679239ff018a#cotire-で生成したプリコンパイル済みヘッダーを複数のバイナリターゲットで使いまわす
# HACK: shared precompiled header
# use_shared_pch(target origin [origin_source_dir])
function(use_shared_pch target origin)
  # origin source directory
  set(origin_source_dir ${ARGV2})
  if(NOT DEFINED origin_source_dir)
    set(origin_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  # origin prefix header
  get_target_property(origin_prefix_header ${origin} COTIRE_CXX_PREFIX_HEADER)
  if(NOT origin_prefix_header)
    return()
  endif()

  # make target pch flags
  cotire_make_pch_file_path(CXX ${origin_source_dir} ${origin} origin_pch_file)
  cotire_add_prefix_pch_inclusion_flags(
    CXX ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}
    ${origin_prefix_header} ${origin_pch_file}
    target_pch_flags
  )
  set_property(TARGET ${target} APPEND_STRING PROPERTY COMPILE_FLAGS ${target_pch_flags})
endfunction()

foreach(i RANGE 9)
  add_executable(bin_alias${i} main.cpp)
  target_link_libraries(bin_alias${i} ${Boost_LIBRARIES})
  use_shared_pch(bin_alias${i})
endforeach()
