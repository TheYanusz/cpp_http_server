#pragma once

#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <chrono>
#include <thread>
#include <string>
#include <optional>
#include <fstream>
#include <filesystem>

#define _ASSERT(x, ...) if (!(x)) { fprintf(stderr, "ASSERTION ERROR\nON: %s\nLINE: %d\nFILE: %s\nCOMMENT: %s\n", #x, __LINE__, __FILE__, __VA_ARGS__); exit(EXIT_FAILURE); }

#define ADDR "127.0.0.1"
#define PORT 8080

#define RECIEVE_BUFFER_SIZE 1024

#define ROOT_DIR "./htdocs"

struct ParsedData {
    std::string type;
    std::string protocol;
    std::string file;
    std::string filetype;
    std::string hostAddr;
    int hostPort;
};