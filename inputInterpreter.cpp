#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <array>
#include <algorithm>
#include <queue>
#include "commonDefs.hpp"
#include "inputInterpreter.hpp"
#include "commands.hpp"


using namespace term_app;

namespace {

using InputBuffer_T = std::array<char,MAX_BUF_SIZE>;

//=============== new =========================
thread_local std::queue<std::string> cmdQueue {};
thread_local InputBuffer_T inputBuffer {};
thread_local std::string cmdPart {};

std::string assembleCmd(const char* s, std::size_t count) {

    std::string c{};

    if(! cmdPart.empty()) { 
        c.swap(cmdPart);
    }
    return c.append(s, count);
}

void receiveCmd(int sock) {
    
    const std::string crlf {CR_LF};
    
    while(true) {

        int recvBytes = recv(sock, inputBuffer.data(), inputBuffer.size(), 0);
        
        if (-1 != recvBytes) {

            auto beginIt = inputBuffer.cbegin();
            auto endIt = beginIt + recvBytes;
            
            auto crlfPos = std::search(beginIt, endIt, crlf.cbegin(), crlf.cend());
            
            while (crlfPos != endIt) {
                auto cmdSize = std::distance(beginIt, crlfPos);                
            
                cmdQueue.push(assembleCmd(beginIt, cmdSize));
            
                std::advance(beginIt, (cmdSize + crlf.size()));
            
                crlfPos = std::search(beginIt, endIt, crlf.begin(), crlf.end());
            }

            if( (beginIt != endIt) ) {
                cmdPart.append(beginIt, std::distance(beginIt, endIt));

                auto crlfPos = cmdPart.find(crlf);
                if(std::string::npos != crlfPos) {
                    cmdQueue.push(cmdPart.substr(0, crlfPos));
                    cmdPart.clear();
                }
            }

            if(! cmdQueue.empty()) {
                break;
            }
        } else {
            std::cerr << formErrnoString("recv returned -1:");
            return; // TODO: check this!!!
        }
    }
}
//=============================================
    
void prompt(int sock) {
    static const std::string defaultPrompt(std::string(APP_NAME) + std::string(":"));
    send(sock, defaultPrompt.c_str(), defaultPrompt.size(),0);
}
    
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
const std::string crlf {CR_LF};


void handler(int sock, bool& isActiveFlag) {

    InputBuffer_T recvBuff;
    
    prompt(sock);

    while(isActiveFlag) {
        
        receiveCmd(sock);
        
        while (! cmdQueue.empty()) {

            std::string cmd = std::move(cmdQueue.front()); cmdQueue.pop();
                    
            std::string responseToSend{std::string("\tinput is: ") + cmd};
            responseToSend.append(crlf);

            if(-1 == send(sock, responseToSend.c_str(), responseToSend.size(), 0)) {
                std::cerr << formErrnoString("send returned -1:");
                break;
            }

            if(! std::strcmp(exit_cmd, cmd.c_str())) {
                isActiveFlag = false;
                break;
            }

            prompt(sock);
        }
    }

    isActiveFlag = false;
    close(sock);
    return;    
}

}

