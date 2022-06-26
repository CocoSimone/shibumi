cmake_minimum_required(VERSION 3.21)
project(shibumi)

find_package(SDL2 REQUIRED)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(src/)
add_executable(shibumi src/main.c)

include_directories(${SDL2_INCLUDE_DIRECTORIES})

if(WIN32)
  set(FLAGS -static SDL2main SDL2 gcc stdc++ winpthread winmm version Imm32 Setupapi)
else()
  set(FLAGS dl SDL2 pthread)
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(OPTIMIZATIONS -g
   # -fsanitize=address -fsanitize=undefined
  )
  #target_link_libraries(shibumi -fsanitize=address -fsanitize=undefined)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
  set(OPTIMIZATIONS -O3)
endif()


target_link_libraries(shibumi core ${FLAGS})
target_compile_options(shibumi PUBLIC ${OPTIMIZATIONS} -mno-ms-bitfields -Wall -Wextra -Werror)
