#include "server.hpp"

namespace http {
    Server::Server(const char *bindAddress, int bindPort) {
        memset(&p_addr, '\0', sizeof(this->p_addr));
        p_addr.sin_family = AF_INET;
        p_addr.sin_port = htons(bindPort);
        p_addr.sin_addr.s_addr = inet_addr(bindAddress);
        p_addrLen = sizeof(p_addr);

        p_socket = socket(AF_INET, SOCK_STREAM, 0);
        _ASSERT(p_socket != -1, "Unable to create socket");

        while (bind(p_socket, (sockaddr*)&p_addr, sizeof(p_addr)) == -1) {
            printf("Cannot bind on: %s:%d Port probably occupied\n", bindAddress, bindPort);
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        _ASSERT(listen(p_socket, 10) >= 0, "Error while listening");

        p_acceptedSocket = accept(p_socket, (struct sockaddr*)&p_addr, (socklen_t*)&p_addrLen);
        _ASSERT(p_acceptedSocket >= 0, "Unable to accept request");

        printf("Listening on: %s:%d\nEntering main loop...\n", bindAddress, bindPort);
        mainLoop();
    }

    Server::~Server() {
        close(this->p_socket);
        close(this->p_acceptedSocket);
    }

    void Server::mainLoop(void) {
        while (true) {
            int bytes_recieved = read(p_acceptedSocket, p_buffer, RECIEVE_BUFFER_SIZE);
            _ASSERT(bytes_recieved != -1, "Error recieving data");
            std::cout << bytes_recieved << " bytes recieved" << std::endl;
            std::cout << "DATA:\n" << p_buffer << std::endl;

            struct ParsedData data;
            
            if (parseData(p_buffer).has_value()) {
                data = parseData(p_buffer).value();
                
                printf("PARSED REQUEST:\nREQUEST TYPE: %s\nPROTOCOL: %s\nHOST ADDRESS: %s\nHOST PORT: %d\n", data.type.c_str(), data.protocol.c_str(), data.hostAddr.c_str(), data.hostPort);
            } else {
                std::cout << "Recieved empty request" << std::endl;
            }

            memset(&p_buffer, '\0', RECIEVE_BUFFER_SIZE);
        }
    }

    [[nodiscard]] std::optional<ParsedData> Server::parseData(const char *recievedData) {
        struct ParsedData data;
        data.type = "GET";
        data.protocol = "HTTP/1.1";
        std::string recvData = recievedData;
        std::string firstLine = "";
        for (char c : recvData) {
            if (c == '\n') {
                break;
            } else
                firstLine += c;
        }
        char *trimmed = strtok((char*)firstLine.c_str(), " ");
        for (int i = 0; ;i++) {
            if (trimmed == NULL) break;
            else if (i == 0) data.type = trimmed;
            else if (i == 2) data.protocol = trimmed;
        }

        return data;
    }
}