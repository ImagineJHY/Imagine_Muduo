.PHONY: build

all: clean init prepare build

init:
	cd build && cmake .. && make init
prepare:
	cd build && cmake .. && make prepare
build:
	cd build && cmake .. && make build
clean:
	cd build && make clean