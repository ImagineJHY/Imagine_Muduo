cmake_minimum_required(VERSION 3.0)
project(IMAGINE_MUDUO)

# 定义全局参数 IS_MAKE_PROJECT
# 若已定义，则当前project作为subModule
# 若未定义, 则当前project作为mainModule
# 若当前的任务是编译动态库, 则构建TARGET: init, prepare, build
# 若当前的任务是生成项目, 则不构建上述TARGET
if(NOT DEFINED IS_MAKE_PROJECT)
    message("[Constructing LIB][Imagine Muduo]: Starting Imagine_Muduo lib building, lib name is ${PROJECT_NAME}...")
    set(TARGET_LIB "Imagine_Muduo")
    set(IS_MAKE_PROJECT false)
    set(IMAGINE_SYSTEM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Imagine_System)
    set(IMAGINE_LIB_WORKER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Imagine_System/worker)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

    add_custom_target(
        init
        COMMAND python3 init.py
        echo "[Constructing LIB][Imagine Muduo]: Starting Imagine_Muduo lib target init building..."
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} # 在哪个目录下执行命令
    )
    if(EXISTS ${IMAGINE_SYSTEM_DIR}/CMakeLists.txt)
        add_custom_target(
            prepare
            echo "[Constructing LIB][Imagine Muduo]: thirdparty Imagine_System exists, Starting Imagine_Muduo lib target prepare building..."
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} # 在哪个目录下执行命令
        )
        add_dependencies(prepare system_prepare)
    else()
        message("[Constructing LIB][Imagine Muduo]: thirdparty Imagine_System NOT exists, cin make init to init it!")
    endif()

    add_custom_target(
        build
        echo "[Constructing LIB][Imagine Muduo]: Starting Imagine_Muduo lib target build building..."
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} # 在哪个目录下执行命令
    )
    add_dependencies(build imagine_muduo)
    
    if(EXISTS ${IMAGINE_SYSTEM_DIR}/CMakeLists.txt)
        add_subdirectory(${IMAGINE_SYSTEM_DIR})
        message("[Constructing LIB][Imagine Muduo]: thirdparty Imagine_System exists, add subModule...")
    else()
        message("[Constructing LIB][Imagine Muduo]: thirdparty Imagine_System NOT exists, cin make init to init it!")
    endif()
else()
    message("[Constructing PROJECT][Imagine Muduo]: Starting Imagine_Muduo lib building, lib name is ${PROJECT_NAME}...")
endif()

# 设置动态库的TARGET的头文件, 源文件, 依赖
file(GLOB IMAGINE_MUDUO_SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(imagine_muduo SHARED ${IMAGINE_MUDUO_SRC_LIST})
target_include_directories(imagine_muduo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(imagine_muduo pthread)