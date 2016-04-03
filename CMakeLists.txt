cmake_minimum_required(VERSION 2.8)
project(compiler)

set(Boost_USE_STATIC_LIBS ON)

find_package(BISON)
find_package(FLEX)
find_package(Boost COMPONENTS program_options system filesystem REQUIRED)

bison_target(Parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
flex_target(Scanner scanner.lex ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)
add_flex_bison_dependency(Scanner Parser)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
)

set(EXTRA_COMPILE_FLAGS "-g -std=c++0x")
#set(LCOV_FLAGS "--coverage")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_COMPILE_FLAGS} ${LCOV_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LCOV_FLAGS}")

#set(main_srcs
#  main.cpp
#)

#set(compiler_srcs
#  ${BISON_Parser_OUTPUTS}
#  ${FLEX_Scanner_OUTPUTS}
#  compiler.hpp
#  logger.hpp
#  register.hpp
#)

set(main_srcs
  ${BISON_Parser_OUTPUTS}
  ${FLEX_Scanner_OUTPUTS}
  compiler.cpp
  compiler.hpp
  logger.cpp
  logger.hpp
  main.cpp
  register.cpp
  register.hpp
  tables.cpp
  tables.hpp
)

set(CMAKE_INSTALL_RPATH ".")
#add_library(compiler_common STATIC ${compiler_srcs})

#IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#    set_target_properties(compiler_common PROPERTIES MACOSX_RPATH "@loader_path/../lib")
#ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

add_executable(compiler ${main_srcs})

#target_link_libraries(compiler_common ${FLEX_LIBRARIES} ${BISON_LIBRARIES} ${Boost_LIBRARIES})
#target_link_libraries(compiler compiler_common)

target_link_libraries(compiler ${FLEX_LIBRARIES} ${BISON_LIBRARIES} ${Boost_LIBRARIES})
