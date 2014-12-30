message("* ui-agar/scaler/generic")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -msse -mmmx")
add_library(xm7_scaler-generic
                               scaler_x05.c
			       scaler_x1.c
			       scaler_x125.c
			       scaler_x15.c
			       scaler_x2.c
			       scaler_x225.c
			       scaler_x25.c
			       scaler_x3.c
			       scaler_x4.c
			       scaler_x45.c
			       scaler_x5.c
			       scaler_x6.c
)
