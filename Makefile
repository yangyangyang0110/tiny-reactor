configure:
	cmake -B build -S .

build:
	cmake --build build --target all -j


.PHONY: build configure
