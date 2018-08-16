#include <iostream>
#include <sstream>
#include <cstring>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h> 
#include <signal.h> 
#include "commonDefs.hpp"
#include "inputInterpreter.hpp"

namespace term_app {
    session::SessionManager g_SMgr {};

void sigHandle(int sig) {
    std::cout << "Signal received: " << sig << "\n";
}

void registerSigHandler() {
    struct sigaction signalHandler;

    signalHandler.sa_handler = sigHandle;
    sigemptyset(&signalHandler.sa_mask);
    signalHandler.sa_flags = 0;

    sigaction(SIGINT, &signalHandler, NULL);
    sigaction(SIGKILL, &signalHandler, NULL);
}

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
    
    std::cout << "starting listening local port " << LOCAL_PORT_TO_LISTEN << "\n";

    while(true) {
        int new_sock;
        struct sockaddr_in s_in;
        unsigned int addrlen = sizeof(s_in);

        new_sock = accept(sock, reinterpret_cast<struct sockaddr*>(&s_in), &addrlen);

        if(new_sock < 0) {
            std::cerr << formErrnoString("failed to accept connection::") << "\n";
            break;
        }

        std::cout << "connection established from " << s_in.sin_addr.s_addr << "\n";

        if( g_SMgr.enableSession(new_sock) != session::SessionStatus::OK ) {
            close(new_sock);
        }
    }
}

};


int main(int argc, char* argv[]) {

    try {
        term_app::registerSigHandler();       
        
        term_app::launchServer();        

    } catch (std::exception& e) {
        std::cerr << "exception occured: " << e.what() << "\n";
        return -1;
    }
    return 0;
}
