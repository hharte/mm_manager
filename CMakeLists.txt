# CMakeList.txt : CMake project for mm_manager
#
cmake_minimum_required (VERSION 3.8)

project ("mm_manager")

execute_process(COMMAND git describe --dirty --always --tags
                OUTPUT_VARIABLE GIT_REV
                ERROR_QUIET)

if ("${GIT_REV}" STREQUAL "")
    set(GIT_REV "Unknown")
endif()

string(STRIP "${GIT_REV}" GIT_REV)
add_definitions(-DVERSION=\"${GIT_REV}\")

# Built executables.
if(NOT MSVC)
add_executable (mm_manager "mm_manager.c" "mm_proto.c" "mm_util.c" "mm_modem.c" "mm_manager.h")
endif()
add_executable (mm_admess "mm_admess.c" "mm_manager.h")
add_executable (mm_areacode "mm_admess.c" "mm_manager.h")
add_executable (mm_callscrn "mm_callscrn.c" "mm_util.c" "mm_manager.h")
add_executable (mm_card "mm_card.c" "mm_util.c" "mm_manager.h" "mm_card.h")
add_executable (mm_carrier "mm_carrier.c" "mm_util.c" "mm_manager.h")
add_executable (mm_coinvl "mm_coinvl.c" "mm_util.c" "mm_manager.h")
add_executable (mm_featru "mm_featru.c" "mm_util.c" "mm_manager.h")
add_executable (mm_instsv "mm_instsv.c" "mm_util.c" "mm_manager.h")
add_executable (mm_lcd "mm_lcd.c" "mm_manager.h")
add_executable (mm_rate "mm_rate.c" "mm_util.c" "mm_manager.h")
add_executable (mm_rateint "mm_rateint.c" "mm_util.c" "mm_manager.h")
add_executable (mm_rdlist "mm_rdlist.c" "mm_util.c" "mm_manager.h")
add_executable (mm_smcard "mm_smcard.c" "mm_manager.h")

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  target_link_libraries(mm_carrier wsock32 ws2_32)
endif()
