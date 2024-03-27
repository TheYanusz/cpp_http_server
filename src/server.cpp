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

        

        printf("Listening on: %s:%d\nEntering main loop...\n", bindAddress, bindPort);
        mainLoop();
    }

    Server::~Server() {
        close(this->p_socket);
        close(this->p_acceptedSocket);
    }

    void Server::mainLoop(void) {
        while (true) {
            p_acceptedSocket = accept(p_socket, (struct sockaddr*)&p_addr, (socklen_t*)&p_addrLen);
            _ASSERT(p_acceptedSocket >= 0, "Unable to accept request");
            int bytes_recieved = read(p_acceptedSocket, p_buffer, RECIEVE_BUFFER_SIZE);
            _ASSERT(bytes_recieved != -1, "Error recieving data");
            std::cout << bytes_recieved << " bytes recieved" << std::endl;
            std::cout << "DATA:\n" << p_buffer << std::endl;

            struct ParsedData data;
            
            if (parseData(p_buffer).has_value()) {
                data = parseData(p_buffer).value();
                
                printf("PARSED REQUEST:\nREQUEST TYPE: %s\nPROTOCOL: %s\nFILE: %s\nHOST ADDRESS: %s\nHOST PORT: %d\n", data.type.c_str(), data.protocol.c_str(), data.file.c_str() ,data.hostAddr.c_str(), data.hostPort);
                std::cout << "SENDING RESPONSE" << std::endl;
                std::string html;
                std::string filename = ROOT_DIR+data.file; // TODO: Refactor this garbage
                std::string response = "HTTP/1.1 200 OK\nContent-Type: text/"+data.filetype+"\nContent-Length: ";
                if (data.file == "/") {
                    html = "<!DOCTYPE html>\n<html><head><link rel=\"stylesheet\" href=\"style.css\"/><meta charset=\"utf-8\"><title>Siema</title></head><body><h1>SIEMSON</h1></body></html>";
                    response += html.size();
                    response += "\n\n";
                    response += html;
                }
                else if (getTextFile(filename.c_str()).has_value()) {
                    html = getTextFile(filename.c_str()).value();
                    response += html.size();
                    response += "\n\n";
                    response += html;
                } else if (!getTextFile(filename.c_str()).has_value()) {
                    response = "HTTP/1.1 404";
                }
                
                _ASSERT(write(p_acceptedSocket, response.c_str(), response.size()) != -1, "Unable to send response");
                std::cout << response << std::endl;
            } else {
                std::cout << "Recieved empty request" << std::endl;
            }
            close(p_acceptedSocket);

            memset(&p_buffer, '\0', RECIEVE_BUFFER_SIZE);
        }
    }

    [[nodiscard]] std::optional<ParsedData> Server::parseData(const char *recievedData) {
        struct ParsedData data;
        memset(&data, 0, sizeof(data));
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
            else if (i == 1) data.file = trimmed;
            else if (i == 2) data.protocol = trimmed;
            trimmed = strtok(NULL, " ");
        }
        bool flag = false;
        std::string filetype = "";
        // TODO: unable to specify filetype
        for (char c : firstLine) {
            if (std::iswspace(c)) flag = false;
            if (flag) filetype += c;

            if (c == '.') flag = true;
        }
        filetype += '\n';
        data.filetype = filetype;

        return data;
    }

    [[nodiscard]] std::optional<std::string> Server::getTextFile(const char *filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return {};

        std::string fromFile = "", line;
        while (std::getline(file, line)) {
            fromFile += line;
        }

        return fromFile;
    }
}