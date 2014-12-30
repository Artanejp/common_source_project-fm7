message("* ui-agar/scaler/sse2")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -msse -mmmx")
add_compile_options(-msse2 -msse -mmmx) 
add_library(xm7_scaler-sse2
			       scaler_x1_sse2.c
			       scaler_x125_sse2.c
			       scaler_x15_sse2.c
			       scaler_x2_sse2.c
			       scaler_x225_sse2.c
			       scaler_x25_sse2.c
			       scaler_x3_sse2.c
			       scaler_x4_sse2.c
			       scaler_x45_sse2.c
			       scaler_x5_sse2.c
			       scaler_x6_sse2.c
)
