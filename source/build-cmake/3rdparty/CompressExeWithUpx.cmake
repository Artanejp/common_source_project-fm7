# Compress an executable with UPX.
# COMPRESS_EXE_WITH_UPX(EXE_TARGET)
#

FIND_PROGRAM(UPX upx)
IF(NOT UPX)
	MESSAGE(WARNING "upx was not found; executables will not be compressed.")
ENDIF(NOT UPX)

IF(WIN32)
	SET(UPX_OPTIONS "--compress-icons=0")
ENDIF(WIN32)

MACRO(COMPRESS_EXE_WITH_UPX EXE_TARGET)
IF(UPX)
	# NOTE: $<TARGET_FILE:gcbanner> is preferred,
	# but this doesn't seem to work on Ubuntu 10.04.
	# (cmake_2.8.0-5ubuntu1_i386)
	GET_PROPERTY(UPX_EXE_LOCATION TARGET ${EXE_TARGET} PROPERTY LOCATION)

	ADD_CUSTOM_COMMAND(TARGET ${EXE_TARGET} POST_BUILD
		COMMAND ${UPX} --best --lzma ${UPX_OPTIONS}
			${UPX_EXE_LOCATION}
		)

	UNSET(UPX_EXE_LOCATION)
ENDIF(UPX)
ENDMACRO(COMPRESS_EXE_WITH_UPX)
