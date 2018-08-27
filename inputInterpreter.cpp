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

const std::string defaultPrompt(std::string(APP_NAME) + std::string(":"));

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
    
    while(true) {

        int recvBytes = recv(sock, inputBuffer.data(), inputBuffer.size(), 0);
        
        if (-1 != recvBytes) {

            auto beginIt = inputBuffer.cbegin();
            auto endIt = beginIt + recvBytes;
            
            auto crlfPos = std::search(beginIt, endIt, CR_LF.cbegin(), CR_LF.cend());
            
            while (crlfPos != endIt) {
                auto cmdSize = std::distance(beginIt, crlfPos);                
            
                cmdQueue.push(assembleCmd(beginIt, cmdSize));
            
                std::advance(beginIt, (cmdSize + CR_LF.size()));
            
                crlfPos = std::search(beginIt, endIt, CR_LF.begin(), CR_LF.end());
            }

            if( (beginIt != endIt) ) {
                cmdPart.append(beginIt, std::distance(beginIt, endIt));

                auto crlfPos = cmdPart.find(CR_LF);
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

void sendResponse(int sock, const std::string& response) {
    if(-1 == send(sock, response.c_str(), response.size(), 0)) {
        std::cerr << formErrnoString("send returned -1:");
    }
}

void prompt(int sock) {    
    send(sock, defaultPrompt.c_str(), defaultPrompt.size(),0);
}

   
cmdPack defineCmd(const std::string& input) {

    cmdPack cp {};
    
    auto cmdStart = input.find_first_not_of(SPACE_TAB); // ignore starting spaces

    if(std::string::npos == cmdStart) {
        cp.code = cmdCode::enter;
        cp.args = "";
    } else {
        auto cmdEnd = input.find_first_of(SPACE_TAB, cmdStart);

        std::size_t count = (std::string::npos == cmdEnd) ? cmdEnd : (cmdEnd - cmdStart) ;
        
        auto it = cmdMap.find(input.substr(cmdStart, count));

        cp.code = (it != cmdMap.end()) ? it->second : cmdCode::notValid;

        cp.args = input.substr(cmdStart);
    }

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

    prompt(sock);

    while(isActiveFlag) {
        
        receiveCmd(sock);
        
        while (! cmdQueue.empty()) {

            auto cmd_pack = defineCmd(std::move(cmdQueue.front()));

            cmdQueue.pop();
                    
            std::string responseToSend = processCmd(cmd_pack);

            sendResponse(sock, std::move(responseToSend));

            if(cmdCode::exit == cmd_pack.code) {
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

