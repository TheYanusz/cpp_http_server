CC = g++

SRC += $(wildcard ./src/*.cpp)
SRC += $(wildcard ./src/**/*.cpp)

all:
	$(CC) -o http_server.bin $(SRC)