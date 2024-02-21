cmake_minimum_required(VERSION 3.0)
project(IMAGINE_MUDUO)

option(BUILD_MUDUO "Build Muduo" OFF)

if(NOT DEFINED IMAGINE_TARGET_LIB)

    set(IMAGINE_TARGET_LIB "Imagine_Muduo")

    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g  -std=c++11 -Wall -Werror")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall -Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g  -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

    string(ASCII 27 Esc)
    set(R "${Esc}[1;31m")
    set(G "${Esc}[0;32m")
    set(E "${Esc}[m")

    set(IS_MAKE_PROJECT false)
    set(IMAGINE_SYSTEM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Imagine_System)
    set(IMAGINE_TOOL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Imagine_Tool)
    set(IMAGINE_LIB_WORKER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Imagine_System/worker)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

    message("${G}[Imagine Muduo]: Starting Imagine_Muduo lib building, lib name is ${PROJECT_NAME}...${E}")

    if(EXISTS ${IMAGINE_TOOL_DIR}/CMakeLists.txt)
        add_subdirectory(${IMAGINE_TOOL_DIR})
        message("${G}[Imagine Muduo]: thirdparty Imagine_Tool exists, add subModule...${E}")
    else()
        message("${R}[Imagine Muduo]: thirdparty Imagine_Tool NOT exists, cin make init to init it!${E}")
    endif()

    if(EXISTS ${IMAGINE_SYSTEM_DIR}/CMakeLists.txt)
        add_subdirectory(${IMAGINE_SYSTEM_DIR})
        message("${G}[Imagine Muduo]: thirdparty Imagine_System exists, add subModule...${E}")
    else()
        message("${R}[Imagine Muduo]: thirdparty Imagine_System NOT exists, cin make init to init it!${E}")
    endif()
endif()


# 设置动态库的TARGET的头文件, 源文件, 依赖
file(GLOB IMAGINE_MUDUO_SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(imagine_muduo SHARED ${IMAGINE_MUDUO_SRC_LIST})
target_include_directories(imagine_muduo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${IMAGINE_TOOL_DIR}/Imagine_Time/include ${IMAGINE_TOOL_DIR}/Imagine_Log/include ${IMAGINE_TOOL_DIR}/thirdparty/yaml-cpp/include)

if (BUILD_MUDUO)
    target_link_libraries(imagine_muduo ${LIBRARY_OUTPUT_PATH})
else()
    # target_include_directories(imagine_muduo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_dependencies(imagine_muduo imagine_tool)
    target_link_libraries(imagine_muduo imagine_tool pthread)
endif()