#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <array>
#include <algorithm>
#include "commonDefs.hpp"
#include "inputInterpreter.hpp"
#include "commands.hpp"


using namespace term_app;

namespace {

constexpr std::size_t MAX_BUF_SIZE = 256;

void prompt(int sock) {
    static const std::string defaultPrompt(std::string(APP_NAME) + std::string(":"));
    write(sock, defaultPrompt.c_str(), defaultPrompt.size());
}

cmdPack defineCmd(std::array<char,MAX_BUF_SIZE>& input) {

    cmdPack cp {};  
    std::string cmd {};

    cp.args = input.data();
    auto space = cp.args.find_first_of(' ');
    
    cmd = (space != std::string::npos) ? cp.args.substr(0, space) : cp.args;

    auto it = cmdMap.find(cmd);
    
    cp.code = (it != cmdMap.end()) ? it->second : cmdCode::notValid;
    
    return cp;
}


cmdPack handleInput(std::array<char,MAX_BUF_SIZE>& input, std::size_t readBytes) {
           
    if(readBytes > 1 && std::equal(&input[readBytes-2], &input[readBytes], &CR_LF[0])) {
        input[readBytes-2] = '\0';
    }

    cmdPack cp = defineCmd(input);

    return cp;
}

std::string executeCmd(cmdPack& cp) {

    auto cb = fCbMap.find(cp.code);

    std::string output = ( cb == fCbMap.end() ) ? fCbMap.at(cmdCode::notValid).fCb(cp.args) : cb->second.fCb(cp.args);

    if(! output.empty()) {
        output.append(CR_LF);
    }    

    return output;
}

}

namespace session {

void handler(int sock, bool& isActiveFlag) {

    std::array<char,MAX_BUF_SIZE> recvBuff;
    
    prompt(sock);

    while(isActiveFlag) {
        
        auto retCode = recv(sock, recvBuff.data(), recvBuff.size(), 0);
        
        if(retCode != -1) {

            auto cmd = handleInput(recvBuff, retCode);

            std::string responseToSend = executeCmd(cmd);

            retCode = write(sock, responseToSend.c_str(), responseToSend.size());
            if(retCode == -1) {
                std::cerr << formErrnoString("send returned -1:");
            }

            if(cmdCode::exit == cmd.code) {
                break;
            }            
        } else {
            std::cerr << formErrnoString("recv returned -1:");
            break;
        }

        prompt(sock);
    }

    isActiveFlag = false;
    close(sock);
    return;    
}

}

