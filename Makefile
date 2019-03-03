ifeq ($(OS),Windows_NT)
	filename := csv_to_htaccess.exe
	rm_command := del
	rmdir_command := rmdir
	mkdir_line := if not exist "bin" mkdir bin
	dir := \\

else
	filename := csv_to_htaccess
	rm_command := rm
	rmdir_command := rm -rf
	mkdir_line := if [ ! -d "bin" ]; then mkdir bin; fi
	dir := /
endif

all: main.cpp functions.hpp
	${mkdir_line}
	g++ -std=c++11 main.cpp -o bin${dir}csv_to_htaccess

clean:
	${rm_command} bin${dir}${filename}
	cd ..
	${rmdir_command} bin
