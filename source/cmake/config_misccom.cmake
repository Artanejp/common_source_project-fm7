set(BUILD_BMJR ON CACHE BOOL "Build for Hitachi BASIC MASTER Jr.")
set(BUILD_BUBCOM80 ON CACHE BOOL "Build for Systems Formulate BUBCOM-80.")
set(BUILD_AX1 OFF CACHE BOOL "Build CANON AX-1")
set(BUILD_BX1 ON CACHE BOOL "Build CANON BX-1")

set(BUILD_CEFUCOM21 ON CACHE BOOL "Build Hino Electronics CEFUCOM-21.")
set(BUILD_COLECOVISION ON CACHE BOOL "Build for COLECO ColecoVision.")
set(BUILD_FAMILYBASIC ON CACHE BOOL "Build Nintendo Family Basic.")

set(BUILD_JR100 ON CACHE BOOL "Build Matsushita JR-100.")
set(BUILD_JR800 ON CACHE BOOL "Build Matsushita JR-800.")

set(BUILD_JX ON CACHE BOOL "Build IBM JX.")

set(BUILD_M5 ON CACHE BOOL "Build SORD M5.")
set(BUILD_M23 ON CACHE BOOL "Build SORD M23.")
set(BUILD_M68 ON CACHE BOOL "Build SORD M68.")

set(BUILD_MAP1010 ON CACHE BOOL "Build SEIKO MAP-1010.")

set(BUILD_MICOM_MAHJONG ON CACHE BOOL "Build MICOM MAHJONG.")
set(BUILD_MYCOMZ80A ON CACHE BOOL "Build Japan Electronics College MYCOM Z-80A.")

set(BUILD_MULTI8 ON CACHE BOOL "Build Mitsubishi Multi 8.")

set(BUILD_PHC20 ON CACHE BOOL "Build Sanyo PHC-20.")
set(BUILD_PHC25 ON CACHE BOOL "Build Sanyo PHC-25.")

set(BUILD_PYUTA ON CACHE BOOL "Build TOMY PYUTA.")

set(BUILD_RX78 ON CACHE BOOL "Build BANDAI RX-78.")

set(BUILD_SCV ON CACHE BOOL "Build EPOCH Cuper Casette Vision.")

set(BUILD_SMC70 ON CACHE BOOL "Build SONY SMC-70")
set(BUILD_SMC777 ON CACHE BOOL "Build SONY SMC-777")

set(BUILD_TVBOY ON CACHE BOOL "Build GAKKEN TV BOY")

set(BUILD_TRNJR ON CACHE BOOL "Build ESP TRN Junior")

set(BUILD_X07 ON CACHE BOOL "Build CANON X07")

set(BUILD_YALKY ON CACHE BOOL "Build Yuasa Kyouiku System YALKY")

set(BUILD_YIS ON CACHE BOOL "Build YAMAHA YIS")

set(BUILD_Z80TVGAME_I8255 ON CACHE BOOL "Build Homebrew Z80 TV GAME SYSTEM (i8255)")
set(BUILD_Z80TVGAME_Z80PIO ON CACHE BOOL "Build Homebrew Z80 TV GAME SYSTEM (Z80PIO)")

if(BUILD_BMJR)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/bmjr.qrc)
	ADD_VM(bmjr emubmjr _BMJR)
endif()
if(BUILD_BUBCOM80)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/bubcom80.qrc)
	ADD_VM(bubcom80 emububcom80 _BUBCOM80)
endif()
if(BUILD_AX1)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/ax1.qrc)
	ADD_VM(bx1 emuax1 _AX1)
endif()
if(BUILD_BX1)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/bx1.qrc)
	ADD_VM(bx1 emubx1 _BX1)
endif()
if(BUILD_CEFUCOM21)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/cefucom21.qrc)
	ADD_VM(cefucom21 emucefucom21 _CEFUCOM21)
endif()
if(BUILD_COLECOVISION)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/colecovision.qrc)
	ADD_VM(colecovision emucolecovision _COLECOVISION)
endif()
if(BUILD_FAMILYBASIC)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/familybasic.qrc)
	ADD_VM(familybasic emufamilybasic _FAMILYBASIC)
endif()

if(BUILD_JR100)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/jr100.qrc)
	ADD_VM(jr100 emujr100 _JR100)
endif()
if(BUILD_JR800)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/jr800.qrc)
	ADD_VM(jr800 emujr800 _JR800)
endif()

if(BUILD_JX)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/jx.qrc)
	ADD_VM(jx emujx _JX)
endif()

if(BUILD_M5)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/m5.qrc)
	ADD_VM(m5 emum5 _M5)
endif()

if(BUILD_M23)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/m23.qrc)
	ADD_VM(m23 emum23 _M23)
endif()

if(BUILD_M68)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/m68.qrc)
	ADD_VM(m23 emum68 _M68)
endif()

if(BUILD_MAP1010)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/map1010.qrc)
	ADD_VM(phc25 emumap1010 _MAP1010)
endif()
if(BUILD_MICOM_MAHJONG)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/micom_mahjong.qrc)
	ADD_VM(micom_mahjong emumicom_mahjong _MICOM_MAHJONG)
endif()

if(BUILD_MULTI8)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/multi8.qrc)
	ADD_VM(multi8 emumulti8 _MULTI8)
endif()
if(BUILD_MYCOMZ80A)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/mycomz80a.qrc)
	ADD_VM(mycomz80a emumycomz80a _MYCOMZ80A)
endif()


if(BUILD_PHC20)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/phc20.qrc)
	ADD_VM(phc20 emuphc20 _PHC20)
endif()
if(BUILD_PHC25)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/phc25.qrc)
	ADD_VM(phc25 emuphc25 _PHC25)
endif()
if(BUILD_PYUTA)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/pyuta.qrc)
	ADD_VM(pyuta emupyuta _PYUTA)
endif()

if(BUILD_RX78)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/rx78.qrc)
	ADD_VM(rx78 emurx78 _RX78)
endif()
if(BUILD_SCV)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/scv.qrc)
	ADD_VM(scv emuscv _SCV)
endif()
if(BUILD_SMC70)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/smc70.qrc)
	ADD_VM(smc777 emusmc70 _SMC70)
endif()
if(BUILD_SMC777)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/smc777.qrc)
	ADD_VM(smc777 emusmc777 _SMC777)
endif()
if(BUILD_TVBOY)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/tvboy.qrc)
	ADD_VM(tvboy emutvboy _TVBOY)
endif()

if(BUILD_TRNJR)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/trnjr.qrc)
	ADD_VM(trnjr emutrnjr _TRNJR)
endif()

if(BUILD_X07)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/x07.qrc)
	ADD_VM(x07 emux07 _X07)
endif()

if(BUILD_YALKY)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/yalky.qrc)
	ADD_VM(yalky emuyalky _YALKY)
endif()

if(BUILD_YIS)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/yis.qrc)
	ADD_VM(yis emuyis _YIS)
endif()

if(BUILD_Z80TVGAME_I8255)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/z80tvgame_i8255.qrc)
	ADD_VM(z80tvgame emuz80tvgame_i8255 _Z80TVGAME)
	target_compile_definitions(emuz80tvgame_i8255 PRIVATE -D_USE_I8255)
	target_compile_definitions(vm_emuz80tvgame_i8255 PRIVATE -D_USE_I8255)
	target_compile_definitions(qt_emuz80tvgame_i8255 PRIVATE -D_USE_I8255)
	target_compile_definitions(common_emuz80tvgame_i8255 PRIVATE -D_USE_I8255)
endif()
if(BUILD_Z80TVGAME_Z80PIO)
	set(RESOURCE ${PROJECT_SOURCE_DIR}/src/qt/common/qrc/z80tvgame_z80pio.qrc)
	ADD_VM(z80tvgame emuz80tvgame_z80pio _Z80TVGAME)
	target_compile_definitions(emuz80tvgame_z80pio PRIVATE -D_USE_Z80PIO)
	target_compile_definitions(vm_emuz80tvgame_z80pio PRIVATE -D_USE_Z80PIO)
	target_compile_definitions(qt_emuz80tvgame_z80pio PRIVATE -D_USE_Z80PIO)
	target_compile_definitions(common_emuz80tvgame_z80pio PRIVATE -D_USE_Z80PIO)
endif()

