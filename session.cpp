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

const std::string defaultPrompt(std::string(APP_NAME) + std::string(":"));

thread_local std::queue<std::string> cmdQueue {};
thread_local InputBuffer_T inputBuffer {};
thread_local std::string cmdPart {};
thread_local int session_socket {};

thread_local std::string CTRLcmdPart {};

/*---------------------------- functions definitions -----------------------------------------------------------------*/
std::string copyToStringAndAdvance(const char* &it, std::size_t dist) {
    std::string s{it, dist};
    std::advance(it, dist);
    return s;
}

std::string assembleCmd(const std::string& s) {

    std::string c{};

    if(! cmdPart.empty()) { 
        c.swap(cmdPart);
    }
    return c.append(s);
}

void tmp(const std::string& s) {for(auto c: s) {std::cout<< std::to_string(static_cast<uint8_t>(c)) << " "; }}

void receiveCmd_streambuf() {

    std::array<char, CTRL_CMD_SIZE> tmp_ctrl;
    std::string tmp_inp;
    tmp_inp.reserve(MAX_BUF_SIZE);
    
    while (true) {

        const int recvBytes = recv(session_socket, inputBuffer.data(), inputBuffer.size(), 0);        

        if(-1 != recvBytes) {

            tmp_inp.clear();

            std::stringstream ss;
            ss.rdbuf()->pubsetbuf(inputBuffer.data(), recvBytes);

            while(ss.rdbuf()->in_avail()) {

                if(! CTRLcmdPart.empty()) {
                    
                    auto b = ss.readsome(tmp_ctrl.begin(), CTRL_CMD_SIZE - CTRLcmdPart.size());

                    CTRLcmdPart.append(tmp_ctrl.begin(), b);

                    if(CTRLcmdPart.size() == CTRL_CMD_SIZE) {
                        cmdQueue.push(CTRLcmdPart);
                        CTRLcmdPart.clear();                        
                    }
                    continue;
                }   

                char symbol = ss.get();
                if(IAC == symbol) {

                    tmp_ctrl[0] = symbol;

                    auto b = ss.readsome(&tmp_ctrl[1], CTRL_CMD_SIZE - 1) + 1;

                    if(b < CTRL_CMD_SIZE) {
                        CTRLcmdPart.append(tmp_ctrl.begin(), b);
                    } else {
                        cmdQueue.emplace(tmp_ctrl.begin(), b);
                    }                    
                } else {
                    if('\x0a' == symbol || '\x00' == symbol) {
                        break;
                    } else if('\x0d' == symbol) {
                        cmdQueue.push(cmdPart); 
                        cmdPart.clear(); 
                    } else {
                        cmdPart.append(1, symbol);  
                    }

                    while(! ss.eof()) {
    
                        std::getline(ss, tmp_inp);
                        if(ss.eof()) {
                            cmdPart.append(tmp_inp);
                            tmp_inp.clear();
                        }
                        if(! tmp_inp.empty()) {
                            tmp_inp.pop_back();
                            cmdQueue.push(tmp_inp); 
                            cmdPart.clear();
                        }
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

void receiveCmd_CTRL() {
    while (true) {

        const int recvBytes = recv(session_socket, inputBuffer.data(), inputBuffer.size(), 0);

        if(-1 != recvBytes) {            
            
            auto beginIt = inputBuffer.cbegin();
            auto endIt = beginIt + recvBytes;

            while(beginIt != endIt) {
                // telnet control commands obtaining
                if( ! CTRLcmdPart.empty() ) {

                    auto partLen = CTRLcmdPart.size();
                    std::size_t dist = std::distance(beginIt, endIt);

                    CTRLcmdPart.append(copyToStringAndAdvance(beginIt, std::min((CTRL_CMD_SIZE - partLen), dist)));                    

                    if(CTRLcmdPart.size() == CTRL_CMD_SIZE) {
                        cmdQueue.push(CTRLcmdPart);
                        CTRLcmdPart.clear();
                    }
                    break;
                }
                if( IAC == *beginIt ) {
                    if (std::distance(beginIt, endIt) < CTRL_CMD_SIZE) {

                        CTRLcmdPart.append(copyToStringAndAdvance(beginIt, std::distance(beginIt, endIt)));
                        
                    } else {
                        cmdQueue.push(copyToStringAndAdvance(beginIt, CTRL_CMD_SIZE));                                              
                    }
                } else {
                    // user input obtaining
                    // find end of the input command marked with CR LF
                    auto crlfPos = std::search(beginIt, endIt, CR_LF.cbegin(), CR_LF.cend());

                    if (crlfPos != endIt) {
                    
                        cmdQueue.push(assembleCmd(copyToStringAndAdvance(beginIt, std::distance(beginIt, crlfPos))));
                    
                        std::advance(beginIt, CR_LF.size());

                        continue;
                    } else {
                        cmdPart.append(copyToStringAndAdvance(beginIt, std::distance(beginIt, endIt)));
                        
                        auto crlfPos = cmdPart.find(CR_LF);
                        
                        if(std::string::npos != crlfPos) {
                            cmdQueue.push(cmdPart.substr(0, crlfPos));
                            cmdPart.clear();
                        }
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

    prompt();

    while(isActiveFlag) {
        
        receiveCmd_streambuf();
        
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

