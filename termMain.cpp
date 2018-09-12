#include <iostream>
#include <sstream>
#include <cstring>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <signal.h> 
#include "commonDefs.hpp"
#include "SessionManager.hpp"

namespace {

std::string convertIPtoStr(uint32_t ip) {
    char ip_str[16];

    if(! inet_ntop(AF_INET, &ip, ip_str, sizeof(ip_str))) {
        std::cerr << term_app::formErrnoString("failed to convert IP to string::") << "\n";
        ip_str[0] = '\0';
    }
    return std::string {ip_str};    
}

};

namespace term_app {
    
void launchServer() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("failed to create socket");
    }

    int i = 1;
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&i,sizeof(i));

    struct sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(LOCAL_PORT_TO_LISTEN);

    if(bind(sock, reinterpret_cast<struct sockaddr*>(&sin), sizeof(sin)) < 0) {
        throw std::runtime_error(formErrnoString("failed to bind socket::"));
    }

    if(listen(sock, MAX_NUM_SESSIONS_ALLOWED) < 0) {
        throw std::runtime_error(formErrnoString("failed to listen socket::"));
    }
        
    while(true) {
        int new_sock;
        struct sockaddr_in s_in;
        unsigned int addrlen = sizeof(s_in);

        new_sock = accept(sock, reinterpret_cast<struct sockaddr*>(&s_in), &addrlen);

        if(new_sock < 0) {
            std::cerr << formErrnoString("failed to accept connection::") << "\n";
            continue;
        }

        std::cout << "connection established from " << convertIPtoStr(s_in.sin_addr.s_addr) << "\n";

        if( startSession(new_sock) != session::SessionStatus::OK ) {
            close(new_sock);
        }
    }
}

};


int main(int argc, char* argv[]) {

    try {
        //term_app::initSessionMngr();
        term_app::launchServer();        

    } catch (std::exception& e) {
        std::cerr << "exception occured: " << e.what() << "\n";
        return -1;
    }
    return 0;
}
