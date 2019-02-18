# make all: build executables
all: memManager

memManager:
	gcc memManager.c -o memManager

clean:
	rm memManager
