message("* sdl/vram/generic")

add_library(xm7_vram-generic api_vram256k.c
                   api_vram4096.c
                   api_vram8.c
                   api_vramvec.c
)