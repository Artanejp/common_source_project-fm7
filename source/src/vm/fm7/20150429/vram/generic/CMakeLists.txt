message("* vm/fm7/vram/generic")

add_library(fm7_vram-generic api_vram256k.c
                   api_vram4096.c
                   api_vram8.c
                   api_vramvec.c
)