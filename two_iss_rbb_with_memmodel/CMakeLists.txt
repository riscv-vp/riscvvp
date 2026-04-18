cmake_minimum_required(VERSION 3.10)
project(two_iss_rbb_with_memmodel)

#########################
# Set Global parameters #
#########################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Default to Debug build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()
# Optional: richer debug info
#set(CMAKE_CXX_FLAGS_DEBUG "-g3 -Og")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

##############################################
# Create executable add sources and includes #
##############################################
add_executable(two_iss_rbb_with_memmodel)
target_include_directories(two_iss_rbb_with_memmodel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_sources(two_iss_rbb_with_memmodel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/sc_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/rbbspike/rbbspike.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/spike/spike.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/uncore/uncore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/memmodel/memmodel.cpp
)

####################################
# Specify and link SystemC library #
####################################
set(SYSTEMC_HOME $ENV{SYSTEMC_HOME})
if(NOT SYSTEMC_HOME)
    message(FATAL_ERROR "Environment variable SYSTEMC_HOME is not set")
endif()
target_include_directories(two_iss_rbb_with_memmodel PRIVATE ${SYSTEMC_HOME}/include)
add_library(systemc_lib SHARED IMPORTED)
set_target_properties(systemc_lib PROPERTIES IMPORTED_LOCATION ${SYSTEMC_HOME}/lib-linux64/libsystemc.so)
target_link_libraries(two_iss_rbb_with_memmodel PUBLIC systemc_lib dl pthread rt)

########################################
# Specify and link RiscV(Spike) library #
########################################
set(RISCV_HOME $ENV{RISCV_HOME})
if(NOT RISCV_HOME)
    message(FATAL_ERROR "Environment variable RISCV_HOME is not set")
endif()
target_include_directories(two_iss_rbb_with_memmodel PRIVATE ${RISCV_HOME}/include)
add_library(riscv_lib SHARED IMPORTED)
set_target_properties(riscv_lib PROPERTIES IMPORTED_LOCATION ${RISCV_HOME}/lib/libriscv.so)
target_link_libraries(two_iss_rbb_with_memmodel PUBLIC riscv_lib)
