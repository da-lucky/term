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

using InputBuffer_T = std::array<char,MAX_BUF_SIZE>;
    
void prompt(int sock) {
    static const std::string defaultPrompt(std::string(APP_NAME) + std::string(":"));
    send(sock, defaultPrompt.c_str(), defaultPrompt.size(),0);
}
    
/*
std::string receiveCmd(int sock, InputBuffer_T& buf) {
    
    std::string cmd {};
    auto retCode = 0;
    
    while(recv(sock, buf.data(), buf.size(), 0) != -1) {
        cmd.append(buf.data());
        
        if (cmd.find_first_of(std::string(CR_LF)) != std::string::npos) {
            
        }
    }
}
*/
    
cmdPack defineCmd(InputBuffer_T& input) {

    cmdPack cp {};  
    std::string cmd {};

    cp.args = input.data();
    auto space = cp.args.find_first_of(' ');
    
    cmd = (space != std::string::npos) ? cp.args.substr(0, space) : cp.args;

    auto it = cmdMap.find(cmd);
    
    cp.code = (it != cmdMap.end()) ? it->second : cmdCode::notValid;
    
    return cp;
}


cmdPack handleInput(InputBuffer_T& input, std::size_t readBytes) {
    
    /* skip CR LF to define command correctly later */
    if(readBytes > 1 &&
       std::equal(&input[readBytes-2], &input[readBytes], &CR_LF[0])) {
        input[readBytes-2] = '\0';
    }

    cmdPack cp = defineCmd(input);

    return cp;
}

std::string processCmd(cmdPack& cp) {

    auto mapIt = fCbMap.find(cp.code);

    auto& fDescr = ( mapIt == fCbMap.end() ) ? fCbMap.at(cmdCode::notValid) : mapIt->second;
    
    std::string output = fDescr.cb(cp.args);

    if(! output.empty()) {
        output.append(CR_LF);
    }    

    return output;
}

}

namespace session {

void handler(int sock, bool& isActiveFlag) {

    InputBuffer_T recvBuff;
    
    prompt(sock);

    while(isActiveFlag) {
        
        auto retCode = recv(sock, recvBuff.data(), recvBuff.size(), 0);
        
        if(retCode != -1) {

            auto cmd = handleInput(recvBuff, retCode);

            std::string responseToSend = processCmd(cmd);

            retCode = send(sock, responseToSend.c_str(), responseToSend.size(), 0);
            if(retCode == -1) {
                std::cerr << formErrnoString("send returned -1:");
                break;
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

