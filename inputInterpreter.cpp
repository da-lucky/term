#include <unistd.h>
#include <iostream>
#include <cstring>
#include <array>
#include "commonDefs.hpp"
#include "inputInterpreter.hpp"

namespace session {


void prompt(int sock) {
    static const std::string defaultPrompt(std::string(term_app::APP_NAME) + std::string(":"));
    write(sock, defaultPrompt.c_str(), defaultPrompt.size());
}


void provider(int sock, bool& isActiveFlag) {
    
    std::array<char,256> buffer;
    buffer.front() = '\0';
    int count = 0;
    const std::string crlf = {'\xd','\xa'};

    prompt(sock);

    while(count != -1) {
        if(count = read(sock, buffer.data(), buffer.size())) {            

            if(count > 1) {
                buffer[count-2] = '\0';
            }
        }    

        std::string response {std::string{"\tinput is:"} + std::string{buffer.data()} + crlf};
        write(sock, response.c_str(), response.size());        

        if(!std::strcmp("quit", buffer.data())) {
            isActiveFlag = false;
            close(sock);
            return;
        }
        
        prompt(sock);
    }
}
}

