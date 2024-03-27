#pragma once

#include "utils.hpp"

namespace http {
    class Server {
        public:
        Server(const char *bindAddress, int bindPort);
        ~Server();

        private:
        int p_socket, p_acceptedSocket, p_addrLen;
        char p_buffer[RECIEVE_BUFFER_SIZE] = {0};
        struct sockaddr_in p_addr;

        void mainLoop(void);
        [[nodiscard]] std::optional<std::string> getTextFile(const char *filename);
        [[nodiscard]] std::optional<ParsedData> parseData(const char *recievedData);
    };
}