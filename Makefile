make:
	make clean build
clean:
	rm -rf build
build:
	mkdir -p build
	clang src/main.c -o build/main
run:
	build/main
