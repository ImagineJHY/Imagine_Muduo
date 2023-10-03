cmake_minimum_required(VERSION 3.0)
project(IMAGINE_MUDUO)
include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
add_library(imagine_muduo SHARED ${SRC_LIST})

add_custom_target(
    setup ALL # ALL代表每次构建都会运行这个目标
    COMMAND ./thirdparty/Imagine_System/setup.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} # 在哪个目录下执行命令
)

add_custom_target(
    init ALL # ALL代表每次构建都会运行这个目标
    COMMAND python3 init.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} # 在哪个目录下执行命令
)

target_link_libraries(imagine_muduo pthread)