#include "utils.hpp"

#include "server.hpp"

int main(int argc, char *argv[]) {
    http::Server server("127.0.0.1", 8080);

    return EXIT_SUCCESS;
}