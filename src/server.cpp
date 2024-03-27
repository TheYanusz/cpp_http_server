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
                std::string filename = ROOT_DIR+data.file; 
                std::string response = "HTTP/1.1 200 OK\nContent-Type: text/"+data.filetype+"\nContent-Length: ";
                if (data.file == "/" && data.type == "GET") {
                    response = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
                    html = getTextFile("htdocs/servermainsite.html").value();
                    response += html.size();
                    response += "\n\n";
                    response += html;
                }
                else if (getTextFile(filename.c_str()).has_value() && data.type == "GET") {
                    html = getTextFile(filename.c_str()).value();
                    response += html.size();
                    response += "\n\n";
                    response += html;
                } else if (!getTextFile(filename.c_str()).has_value()) {
                    response = "HTTP/1.1 404\nContent-Type: text/html\nContent-Length: 90\n\n<html><head><title>404</title></head><body><h1>404</h1><p>File not found</p></body></html>";
                } else if (data.type == "POST" && getTextFile(filename.c_str()).has_value()) {
                    response = "HTTP/1.1 201 Created\nContent-Type: text/plain; charset=UTF-8\nContent-Length: ";
                    html = getTextFile(filename.c_str()).value();
                    response += html.size();
                    response += "\n\n";
                    response += html;
                } else if (data.filetype == "dir") { // TODO: Not working
                    response = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
                    html = createHTMLReposponseForDirectory(filename.c_str());
                    response += html.size();
                    response += "\n\n";
                    response += html;
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
        if (data.file.find(".css") != std::string::npos) {
            data.filetype = "css";
        } else if (data.file.find(".html") != std::string::npos) {
            data.filetype = "html";
        } else if (data.file.find(".js") != std::string::npos) {
            data.filetype = "javascript";
        } else {
            data.filetype = "dir";
        }
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

    std::string Server::createHTMLReposponseForDirectory(const char *directory) {
        std::string response = "<!DOCTYPE html><html><head><title>Directory</title></head><body><h1>Directory</h1><ul>";
        for (const std::filesystem::__cxx11::directory_entry entry : std::filesystem::directory_iterator(directory)) {
            response += "<li><a href=\"#\">"+entry.path().filename().string()+"</a></li>";
        }
        response += "</body></html>";
        return response;
    }
}