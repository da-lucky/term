#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <array>
#include <algorithm>
#include <queue>
#include "commonDefs.hpp"
#include "session.hpp"
#include "commands.hpp"

#include <sstream>

using namespace term_app;

namespace {
/*---------------------------- types definitions ---------------------------------------------------------------------*/

using InputBuffer_T = std::array<char,MAX_BUF_SIZE>;

/*---------------------------- variables definitions -----------------------------------------------------------------*/

const std::string defaultPrompt(std::string("\n") + std::string(APP_NAME) + std::string(":"));

thread_local std::queue<std::string> cmdQueue {};
thread_local InputBuffer_T inputBuffer {};
thread_local int session_socket {};

thread_local std::string USERcmdPart {};
thread_local std::string CTRLcmdPart {};

/*---------------------------- functions definitions -----------------------------------------------------------------*/
void handleSymbol(char symbol) {
    switch (symbol) {
        case '\x7f' : {
            std::cout << "BS\n";
            if(! USERcmdPart.empty()) {
                USERcmdPart.pop_back();
            }
            break;
        }
        default: {
            USERcmdPart.push_back(symbol);
            break;
        }
    }
}

/* read input in telnet char mode */
void readInput() {

    std::stringstream ss;

    while (true) {
        const int recvBytes = recv(session_socket, inputBuffer.data(), inputBuffer.size(), 0);        

        if(-1 != recvBytes) {
            
            ss.rdbuf()->pubsetbuf(inputBuffer.data(), recvBytes);

            while(ss.rdbuf()->in_avail()) {
              
                if(! CTRLcmdPart.empty()) {
                    CTRLcmdPart.push_back(ss.get());

                    if(CTRL_CMD_SIZE == CTRLcmdPart.size()) {
                        cmdQueue.emplace(CTRLcmdPart);
                        CTRLcmdPart.clear();
                    }
                    continue;
                }

                char symbol = ss.get();                
                if(IAC == symbol) {
                    CTRLcmdPart.push_back(symbol);
                } else {                                        
                    if(CR == symbol) {
                        cmdQueue.emplace(USERcmdPart);
                        USERcmdPart.clear();
                        continue;
                    }
                    if(LF == symbol || '\00' == symbol) { continue; }

                    handleSymbol(symbol);

                    if(MAX_USER_CMD_SIZE == USERcmdPart.size()) {
                        USERcmdPart.clear();
                        std::cerr << "not allowed command length... skip it... MAX allowed " << MAX_USER_CMD_SIZE << "\n";
                        continue;
                    }
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

void sendResponse(const std::string& response) {
    if(-1 == send(session_socket, response.c_str(), response.size(), 0)) {
        std::cerr << formErrnoString("send returned -1:");
    }
}

void prompt() {    
    send(session_socket, defaultPrompt.c_str(), defaultPrompt.size(),0);
}

void setenv() {
    constexpr char e1[] = {'\xff','\xfb','\x03'}; // set char mode
    send(session_socket, e1, sizeof(e1), 0);

    constexpr char e2[] = {'\xff','\xfb','\x01'}; // set char mode
    send(session_socket, e2, sizeof(e2), 0);

//    constexpr char e3[] = {'\xff','\xfd','\x1f'}; // set char mode
//    send(session_socket, e3, sizeof(e3), 0);

}

   
cmdPack defineCmd(const std::string& input) {

    cmdPack cp {};
    
    auto cmdStart = input.find_first_not_of(SPACE_TAB); // ignore starting spaces

    if(std::string::npos == cmdStart) {
        cp.code = cmdCode::empty_input;
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

std::string processCmd(const cmdPack& cp) {

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
    
    session_socket = sock; // init thread local variable for socket
    CTRLcmdPart.reserve(CTRL_CMD_SIZE);
    USERcmdPart.reserve(MAX_USER_CMD_SIZE);

    setenv();
    prompt();

    while(isActiveFlag) {
        
        readInput();
        
        while (! cmdQueue.empty()) {

            auto cmd_pack = defineCmd(std::move(cmdQueue.front()));

            cmdQueue.pop();
                    
            std::string responseToSend = processCmd(cmd_pack);

            sendResponse(std::move(responseToSend));

            if(cmdCode::exit == cmd_pack.code) {
                isActiveFlag = false;
                break;
            }            
        }
        prompt();
    }

    isActiveFlag = false;
    close(session_socket);
    return;    
}

}

