# CMakeList.txt : CMake project for mm_manager
#
cmake_minimum_required (VERSION 3.8)

project ("mm_manager")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

execute_process(COMMAND git describe --dirty --always --tags
                OUTPUT_VARIABLE GIT_REV
                ERROR_QUIET)

if ("${GIT_REV}" STREQUAL "")
    set(GIT_REV "Unknown")
endif()

string(STRIP "${GIT_REV}" GIT_REV)
add_definitions(-DVERSION=\"${GIT_REV}\")

ADD_LIBRARY(mm_util STATIC "mm_util.c")

if(MSVC)
ADD_LIBRARY(mm_serial STATIC "mm_serial_win32.c" "mm_serial.h" "third-party/getopt.c" "third-party/getopt.h")
include_directories("third-party")
ADD_LIBRARY(sqlite3 STATIC "third-party/sqlite3.c" "third-party/sqlite3.h")
else()
ADD_LIBRARY(mm_serial STATIC "mm_serial_posix.c" "mm_serial.h")
endif()

# Built executables.
set(MANAGER_SRC
    "mm_manager.c"
    "mm_manager.h"
    "mm_accounting.c"
    "mm_modem.c"
    "mm_pcap.c"
    "mm_pcap.h"
    "mm_proto.c"
    "mm_serial.c"
    "mm_serial.h"
    "mm_config.c"
    "mm_tables.c"
    "mm_udp.c"
    "mm_udp.h"
    "mm_sqlite3.c"
)

set(DLOG2PCAP_SRC
    "mm_dlog2pcap.c"
    "mm_manager.h"
    "mm_pcap.c"
    "mm_pcap.h"
)

add_executable (mm_manager ${MANAGER_SRC})
#target_compile_options(mm_manager PUBLIC $<$<CONFIG:DEBUG>:-fprofile-instr-generate -fcoverage-mapping>)
if(MSVC)
TARGET_LINK_LIBRARIES(mm_manager mm_serial mm_util sqlite3 wsock32 ws2_32)
else()
TARGET_LINK_LIBRARIES(mm_manager mm_serial mm_util sqlite3)
endif()
add_executable (mm_admess "mm_admess.c" "mm_manager.h")
add_executable (mm_areacode "mm_areacode.c" "mm_manager.h")
add_executable (mm_callscrn "mm_callscrn.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_callscrn mm_util)
add_executable (mm_convert_callscrn_mtr2_to_mtr1 "mm_convert_callscrn_mtr2_to_mtr1.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_convert_callscrn_mtr2_to_mtr1 mm_util)
add_executable (mm_card "mm_card.c" "mm_manager.h" "mm_card.h")
TARGET_LINK_LIBRARIES(mm_card mm_util)
add_executable (mm_card_mtr1 "mm_card_mtr1.c" "mm_manager.h" "mm_card.h")
TARGET_LINK_LIBRARIES(mm_card_mtr1 mm_util)
add_executable (mm_convert_card_mtr2_to_mtr1 "mm_convert_card_mtr2_to_mtr1.c" "mm_manager.h" "mm_card.h")
TARGET_LINK_LIBRARIES(mm_convert_card_mtr2_to_mtr1 mm_util)
add_executable (mm_carrier "mm_carrier.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_carrier mm_util)
add_executable (mm_carrier_mtr1 "mm_carrier_mtr1.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_carrier_mtr1 mm_util)
add_executable (mm_coinvl "mm_coinvl.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_coinvl mm_util)
add_executable (mm_featru "mm_featru.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_featru mm_util)
add_executable (mm_instsv "mm_instsv.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_instsv mm_util)
add_executable (mm_lcd "mm_lcd.c" "mm_manager.h")
add_executable (mm_rate "mm_rate.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_rate mm_util)
add_executable (mm_rateint "mm_rateint.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_rateint mm_util)
add_executable (mm_rdlist "mm_rdlist.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_rdlist mm_util)
add_executable (mm_smcard "mm_smcard.c" "mm_manager.h")
add_executable (mm_table_cutter "mm_table_cutter.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_table_cutter mm_util)
add_executable (mm_dlog2pcap ${DLOG2PCAP_SRC})
TARGET_LINK_LIBRARIES(mm_dlog2pcap mm_util)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  target_link_libraries(mm_carrier wsock32 ws2_32 sqlite3)
  target_link_libraries(mm_carrier_mtr1 wsock32 ws2_32 sqlite3)
endif()

# Install
set(INSTALL_TARGETS
    "mm_manager"
    "mm_admess"
    "mm_areacode"
    "mm_callscrn"
    "mm_card"
    "mm_card_mtr1"
    "mm_convert_card_mtr2_to_mtr1"
    "mm_carrier"
    "mm_carrier_mtr1"
    "mm_coinvl"
    "mm_dlog2pcap"
    "mm_featru"
    "mm_instsv"
    "mm_lcd"
    "mm_rate"
    "mm_rateint"
    "mm_rdlist"
    "mm_smcard"
    "mm_table_cutter"
)

install(TARGETS ${INSTALL_TARGETS} DESTINATION .)
install(DIRECTORY tables/default DESTINATION tables)
install(FILES LICENSE DESTINATION .)
install(FILES README.md DESTINATION .)
install(FILES generate_intl_sbr.py generate_lcd.py generate_lcd_lata.py generate_npa.py DESTINATION .)

set(CPACK_GENERATOR "ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_VERSION ${GIT_REV})
SET(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})
include(CPack)
