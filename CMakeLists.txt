# CMakeList.txt : CMake project for mm_manager
#
cmake_minimum_required (VERSION 3.8)

project ("mm_manager" LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    # Treat warnings as errors
    add_compile_options(/WX)
else()
    # Set the C standard to C11
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)

    # Enforce additional checking
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

execute_process(COMMAND git describe --dirty --always --tags
                OUTPUT_VARIABLE GIT_REV
                ERROR_QUIET)

if ("${GIT_REV}" STREQUAL "")
    set(GIT_REV "Unknown")
endif()

string(STRIP "${GIT_REV}" GIT_REV)
add_definitions(-DVERSION=\"${GIT_REV}\")

include_directories("third-party" ".")

ADD_LIBRARY(mm_util STATIC "src/mm_util.c")
ADD_LIBRARY(sqlite3 STATIC "third-party/sqlite3.c" "third-party/sqlite3.h")

if(MSVC)
ADD_LIBRARY(mm_serial STATIC "src/mm_serial_win32.c" "src/mm_serial.h" "third-party/getopt.c" "third-party/getopt.h")
else()
ADD_LIBRARY(mm_serial STATIC "src/mm_serial_posix.c" "src/mm_serial.h")
endif()

# Built executables.
set(MANAGER_SRC
    "src/mm_manager.c"
    "src/mm_manager.h"
    "src/mm_accounting.c"
    "src/mm_connection.c"
    "src/mm_modem.c"
    "src/mm_pcap.c"
    "src/mm_pcap.h"
    "src/mm_proto.c"
    "src/mm_serial.c"
    "src/mm_serial.h"
    "src/mm_config.c"
    "src/mm_tables.c"
    "src/mm_udp.c"
    "src/mm_udp.h"
    "src/mm_sqlite3.c"
)

set(DLOG2PCAP_SRC
    "src/mm_dlog2pcap.c"
    "src/mm_manager.h"
    "src/mm_pcap.c"
    "src/mm_pcap.h"
)

add_executable (mm_manager ${MANAGER_SRC})
#target_compile_options(mm_manager PUBLIC $<$<CONFIG:DEBUG>:-fprofile-instr-generate -fcoverage-mapping>)
if(MSVC)
TARGET_LINK_LIBRARIES(mm_manager mm_serial mm_util sqlite3 wsock32 ws2_32)
else()
TARGET_LINK_LIBRARIES(mm_manager mm_serial mm_util sqlite3 pthread dl)
endif()
add_executable (mm_admess "src/mm_admess.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_admess mm_util)
add_executable (mm_areacode "src/mm_areacode.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_areacode mm_util)
add_executable (mm_callscrn "src/mm_callscrn.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_callscrn mm_util)
add_executable (mm_convert_callscrn_mtr2_to_mtr1 "src/mm_convert_callscrn_mtr2_to_mtr1.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_convert_callscrn_mtr2_to_mtr1 mm_util)
add_executable (mm_card "src/mm_card.c" "src/mm_manager.h" "src/mm_card.h")
TARGET_LINK_LIBRARIES(mm_card mm_util)
add_executable (mm_callin "src/mm_callin.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_callin mm_util)
add_executable (mm_callstat "src/mm_callstat.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_callstat mm_util)
add_executable (mm_card_mtr1 "src/mm_card_mtr1.c" "src/mm_manager.h" "src/mm_card.h")
TARGET_LINK_LIBRARIES(mm_card_mtr1 mm_util)
add_executable (mm_convert_card_mtr2_to_mtr1 "src/mm_convert_card_mtr2_to_mtr1.c" "src/mm_manager.h" "src/mm_card.h")
TARGET_LINK_LIBRARIES(mm_convert_card_mtr2_to_mtr1 mm_util)
add_executable (mm_carrier "src/mm_carrier.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_carrier mm_util)
add_executable (mm_carrier_mtr1 "src/mm_carrier_mtr1.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_carrier_mtr1 mm_util)
add_executable (mm_commstat "src/mm_commstat.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_commstat mm_util)
add_executable (mm_coinvl "src/mm_coinvl.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_coinvl mm_util)
add_executable (mm_fconfig "src/mm_fconfig.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_fconfig mm_util)
add_executable (mm_instsv "src/mm_instsv.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_instsv mm_util)
add_executable (mm_lcd "src/mm_lcd.c" "src/mm_manager.h")
add_executable (mm_luhn "src/mm_luhn.c")
add_executable (mm_rate "src/mm_rate.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_rate mm_util)
add_executable (mm_rateint "src/mm_rateint.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_rateint mm_util)
add_executable (mm_rdlist "src/mm_rdlist.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_rdlist mm_util)
add_executable (mm_smcard "src/mm_smcard.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_smcard mm_util)
add_executable (mm_table_cutter "src/mm_table_cutter.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_table_cutter mm_util)
add_executable (mm_userif "src/mm_userif.c" "src/mm_manager.h")
TARGET_LINK_LIBRARIES(mm_userif mm_util)
add_executable (mm_dlog2pcap ${DLOG2PCAP_SRC})
TARGET_LINK_LIBRARIES(mm_dlog2pcap mm_util)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  target_link_libraries(mm_carrier wsock32 ws2_32 sqlite3)
  target_link_libraries(mm_carrier_mtr1 wsock32 ws2_32 sqlite3)
  target_link_libraries(mm_smcard wsock32 ws2_32 sqlite3)
  endif()

# Install
set(INSTALL_TARGETS
    "mm_manager"
    "mm_admess"
    "mm_areacode"
    "mm_callin"
    "mm_callscrn"
    "mm_callstat"
    "mm_card"
    "mm_card_mtr1"
    "mm_carrier"
    "mm_carrier_mtr1"
    "mm_convert_callscrn_mtr2_to_mtr1"
    "mm_convert_card_mtr2_to_mtr1"
    "mm_coinvl"
    "mm_commstat"
    "mm_dlog2pcap"
    "mm_fconfig"
    "mm_instsv"
    "mm_lcd"
    "mm_luhn"
    "mm_rate"
    "mm_rateint"
    "mm_rdlist"
    "mm_smcard"
    "mm_table_cutter"
    "mm_userif"
)

install(TARGETS ${INSTALL_TARGETS} DESTINATION bin)
install(DIRECTORY wireshark DESTINATION .)
install(DIRECTORY tables/default DESTINATION tables)
install(DIRECTORY tables/card_only DESTINATION tables)
install(DIRECTORY tables/coin DESTINATION tables)
install(DIRECTORY tables/desk DESTINATION tables)
install(FILES LICENSE DESTINATION .)
install(FILES README.md DESTINATION .)
install(FILES generate_intl_sbr.py generate_lcd.py generate_lcd_lata.py generate_lcd_lir.py generate_npa.py DESTINATION bin)

set(CPACK_GENERATOR "ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_VERSION ${GIT_REV})
SET(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})
include(CPack)
