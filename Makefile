# make all: build executables
all: memManager

memManager:
	gcc memManager.c -g -o memManager

clean:
	rm memManager
