project(timer_project)
cmake_minimum_required(VERSION 3.5.0)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

add_compile_options(-std=c++17)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall")

set(SRC_FILES src/timer/timer.cpp
              src/timer/timerdb.cpp
              src/timer/timerlog.cpp
              src/timer/timerclock.cpp
              src/timer/timercmd.cpp

              src/util/cmdline.cpp
              src/util/datetime.cpp

              src/main.cpp)

set (LIBS vsqlitepp
          boost_filesystem
          boost_program_options
          boost_date_time
          jsoncpp)

add_executable(timer ${SRC_FILES})
target_link_libraries(timer ${LIBS})
