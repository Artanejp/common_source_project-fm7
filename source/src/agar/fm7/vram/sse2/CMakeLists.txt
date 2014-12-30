message("* sdl/vram/sse2")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -msse -mmmx")
add_compile_options(-msse2 -msse -mmmx)
add_library(xm7_vram-sse2 api_vram256k.c
                   api_vram4096.c
                   api_vram8.c
                   api_vramvec.c
)
