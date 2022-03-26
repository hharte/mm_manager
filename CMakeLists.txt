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
else()
ADD_LIBRARY(mm_serial STATIC "mm_serial_posix.c" "mm_serial.h")
endif()

# Built executables.
set(MANAGER_SRC
    "mm_manager.c"
    "mm_proto.c"
    "mm_modem.c"
    "mm_manager.h"
    "mm_serial.h"
)

add_executable (mm_manager ${MANAGER_SRC})
TARGET_LINK_LIBRARIES(mm_manager mm_serial mm_util)
add_executable (mm_admess "mm_admess.c" "mm_manager.h")
add_executable (mm_areacode "mm_areacode.c" "mm_manager.h")
add_executable (mm_callscrn "mm_callscrn.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_callscrn mm_util)
add_executable (mm_card "mm_card.c" "mm_manager.h" "mm_card.h")
TARGET_LINK_LIBRARIES(mm_card mm_util)
add_executable (mm_carrier "mm_carrier.c" "mm_manager.h")
TARGET_LINK_LIBRARIES(mm_carrier mm_util)
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

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  target_link_libraries(mm_carrier wsock32 ws2_32)
endif()

# Install
if (NOT MSVC)
install(TARGETS mm_manager DESTINATION /usr/local/bin)
endif()
