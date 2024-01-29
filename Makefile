#!/bin/dash
.PHONY: build

system_file_name=./thirdparty/Imagine_System
tool_file_name=./thirdparty/Imagine_Tool

all: clean init prepare build

init:
	python3 init.py
prepare:
ifneq (${wildcard ${system_file_name}},)
	@echo "Imagine_System exists"
else
	@echo -e "\033[;31mImagine_System NOT exist, Please exucete make init to init it\033[0m"
	exit 1
endif
ifneq (${wildcard ${tool_file_name}},)
	@echo "Imagine_Tool exists"
else
	@echo -e "\033[;31mImagine_Tool NOT exist, Please exucete make init to init it\033[0m"
	exit 1
endif
	cd ${system_file_name} && make prepare
	cd ${tool_file_name} && make prepare

build:
	cd build && cmake -DBUILD_MUDUO=OFF .. && make imagine_muduo

muduo:
	cd build && cmake -DBUILD_MUDUO=ON .. && make imagine_muduo

clean:
	cd build && make clean

define check_file
    @echo "Checking file: $1"
    ifeq ($1, ./thirdparty/Imagine_System)
        @echo "File exists"
    else
        @echo "File does not exist"
    endif
endef