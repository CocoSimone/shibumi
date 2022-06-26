project(core)

file(GLOB_RECURSE SOURCES "shibumi/src/*.c")
file(GLOB_RECURSE HEADERS "shibumi/include/*.h")

add_subdirectory(../external temp)

add_library(core
  ${SOURCES}
  ${HEADERS}
  emu.c
  emu.h
)

target_include_directories(core PUBLIC
  ../external
  .
  shibumi/
  shibumi/include/
  shibumi/include/core/
  shibumi/include/core/cpu/
  shibumi/include/core/cpu/instruction/
  shibumi/include/core/cpu/registers/
  shibumi/include/core/mem/
  shibumi/include/core/mem/mmio/
  shibumi/include/core/rsp/
  shibumi/include/utils/
)

target_link_libraries(core PUBLIC contrib)
