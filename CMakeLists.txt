cmake_minimum_required(VERSION 3.5...3.31)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(leftseat C)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Left Seat")
set(VITA_TITLEID  "SKYH00001")
set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -std=gnu99")

add_executable(${PROJECT_NAME} src/main.c src/pad.c)

target_link_libraries(${PROJECT_NAME}
  vita2d
  SceLibKernel_stub
  SceDisplay_stub
  SceGxm_stub
  SceSysmodule_stub
  SceAppMgr_stub
  SceCtrl_stub
  SceTouch_stub
  ScePgf_stub
  ScePvf_stub
  SceCommonDialog_stub
  SceAudio_stub
  freetype png jpeg z m c
)

# Extra metadata blocks inside a PNG can make the Vita show a plain white icon. Strip them every
# time CMake runs (needs python3; if it is missing the images are left as they are).
find_program(PYTHON3_EXE python3)
if(PYTHON3_EXE)
  foreach(img sce_sys/icon0.png sce_sys/livearea/contents/bg0.png sce_sys/livearea/contents/startup.png)
    execute_process(COMMAND ${PYTHON3_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/strip_png.py
                            ${CMAKE_CURRENT_SOURCE_DIR}/${img} ${CMAKE_CURRENT_SOURCE_DIR}/${img}
                    OUTPUT_QUIET ERROR_QUIET)
  endforeach()
endif()

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg0.png sce_sys/livearea/contents/bg0.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
