#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <array>
#include <algorithm>
#include <queue>
#include "commonDefs.hpp"
#include "session.hpp"
#include "commandToCallbackMap.hpp"
#include "telnetCtrlCommands.hpp"
#include "sessionHistory.hpp"
#include <sstream>

using namespace term_app;

namespace {
/*---------------------------- types definitions ---------------------------------------------------------------------*/

using InputBuffer_T = std::array<char,MAX_BUF_SIZE>;

struct cmdPack {
    cmdCode code;
    std::string args;
};

/*---------------------------- variables definitions -----------------------------------------------------------------*/

const std::string defaultPrompt(std::string(APP_NAME) + std::string(":"));

thread_local std::queue<std::string> cmdQueue {};
thread_local InputBuffer_T inputBuffer {};
thread_local int session_socket {};
thread_local SessionHistory sessionHistory {};

thread_local std::string USERcmdPart {};
thread_local std::string CTRLcmdPart {};

/*---------------------------- functions definitions -----------------------------------------------------------------*/
bool send(const char* const s, std::size_t length, int flags = 0) {
    if(-1 == ::send(session_socket, s, length, flags)) {
        
        std::stringstream err;
        err << std::hex;
        for (std::size_t i = 0; i < std::min(length, 20lu); ++i) {
            err << std::to_string(static_cast<uint8_t>(s[i])) << " ";
        }
        std::cerr << formErrnoString("sendResponse::send returned -1:") 
            << "\nFirst bytes of not sent message (hex): " << err.str() << "\n";
        return false;
    }
    return true;
} 

void handleMovementSeq(const std::string& mSeq) {
   
    if (TERMINAL_ESCAPE_SEQ::BUTTON_BW == mSeq || TERMINAL_ESCAPE_SEQ::BUTTON_FW == mSeq) {
        return;
    }

    if(TERMINAL_ESCAPE_SEQ::BUTTON_UP == mSeq) {
        USERcmdPart = sessionHistory.getPrev();
    }
    else if(TERMINAL_ESCAPE_SEQ::BUTTON_DOWN == mSeq) {
        USERcmdPart = sessionHistory.getNext();
    }

    std::string output {TERMINAL_ESCAPE_SEQ::BLANK_LINE + defaultPrompt + USERcmdPart};

    send(output.data(), output.size());
}

void handleTab() {
    std::string output;

    std::vector<typename std::map<std::string, cmdCode>::const_pointer> cmdMatch {};

    auto firstNonSpace = USERcmdPart.find_first_not_of(ASCII::SPACE);
    auto cmdStart = (std::string::npos == firstNonSpace) ? USERcmdPart.begin() : USERcmdPart.begin() + firstNonSpace;

    for(auto& e: cmdMap) {
        if(std::equal(cmdStart, USERcmdPart.end(), e.first.begin())) {
            cmdMatch.push_back(&e);
        }
    }

    // check if current command is a top one (not from history list) and update if "yes" later after modification
    bool updateTop = (sessionHistory.getTopCmd() == USERcmdPart) ? true : false;

    if(cmdMatch.size() == 1) {

        output.append(cmdMatch.front()->first.substr( USERcmdPart.size() - firstNonSpace ));

        USERcmdPart.append(output);

    } else if(! cmdMatch.empty()) {

        output.append(ASCII::CR_LF_TAB);

        for(auto& e: cmdMatch) {
            output.append(e->first).append(" ");
        }            
        output.append(ASCII::CR_LF).append(defaultPrompt).append(USERcmdPart);
    }

    if(updateTop) {
        sessionHistory.updateTopCmd(USERcmdPart);
    }

    send(output.data(), output.size());
}

void handleSymbol(char symbol) {

    if(( ! USERcmdPart.empty() ) && ( ASCII::ESC == USERcmdPart.back() ) && ('[' != symbol)) {
            USERcmdPart.pop_back(); // remove escape symbol if it is not a part of MOVING ESCAPE SEQUENCE 
    }

    switch (symbol) {
           
        case ASCII::CR :
            if( ! USERcmdPart.empty() ) {
                sessionHistory.completeCmd(USERcmdPart);
            }
            cmdQueue.emplace(USERcmdPart);
            USERcmdPart.clear();
            send(ASCII::CR_LF.data(), ASCII::CR_LF.size());            
            break;
        case ASCII::ETX : // ^C
        case ASCII::EOT : // ^D
        case ASCII::LF :
        case ASCII::NUL :
            break;
        case ASCII::TAB : 
            handleTab();
            break;
        case ASCII::DEL :
            if(! USERcmdPart.empty()) {
                // check if current command is a top one (not from history list) and update if "yes" later after modification
                bool updateTop = (sessionHistory.getTopCmd() == USERcmdPart) ? true : false;

                USERcmdPart.pop_back();
                
                if(updateTop) {
                    sessionHistory.updateTopCmd(USERcmdPart);
                }
                send(TERMINAL_ESCAPE_SEQ::BW_ERASE.data(), TERMINAL_ESCAPE_SEQ::BW_ERASE.size());
            }
            break;      
        default:

            USERcmdPart.push_back(symbol);

            auto escPos = USERcmdPart.find(ASCII::ESC);

            if(std::string::npos == escPos) {
                std::string echo = std::string(1,symbol) + TERMINAL_ESCAPE_SEQ::BW_FW;
                send(echo.data(), echo.size());
                sessionHistory.updateTopCmd(USERcmdPart);

            } else if ((USERcmdPart.size() - escPos) > 2) {
                auto mSeq = USERcmdPart.substr(escPos, TERMINAL_ESCAPE_SEQ::BUTTON_CODE_SIZE);
                USERcmdPart.erase(escPos, TERMINAL_ESCAPE_SEQ::BUTTON_CODE_SIZE);

                handleMovementSeq(mSeq);
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

                    if(TELNET::CMD_CODE::SB == CTRLcmdPart[1]) {
                        if (TELNET::CMD_CODE::SE == CTRLcmdPart.back()) {
                            cmdQueue.emplace(CTRLcmdPart);
                            CTRLcmdPart.clear();
                        }
                    } else if (CTRL_CMD_SIZE == CTRLcmdPart.size()) {
                        cmdQueue.emplace(CTRLcmdPart);
                        CTRLcmdPart.clear();
                    }
                    continue;
                }

                char symbol = ss.get();

                if(TELNET::CMD_CODE::IAC == symbol) {
                    CTRLcmdPart.push_back(symbol);
                } else {

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
    send(response.c_str(), response.size());
}

void prompt() {
    send(defaultPrompt.c_str(), defaultPrompt.size());

}

void initSession() {
    std::string clearScreen{TERMINAL_ESCAPE_SEQ::CLEAR_SCR + TERMINAL_ESCAPE_SEQ::POS_0_0};
    send(clearScreen.data(), clearScreen.size());

    send(TELNET::WILL_SGA.data(), TELNET::WILL_SGA.size());
}

   
cmdPack defineCmd(const std::string& input) {

    cmdPack cp {};
  
    auto cmdStart = input.find_first_not_of(ASCII::SPACE_TAB); // ignore starting spaces

    if(std::string::npos == cmdStart) {
        cp.code = cmdCode::empty_input;
        cp.args = "";
    } else {
        auto cmdEnd = input.find_first_of(ASCII::SPACE_TAB, cmdStart);

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

    return output;
}

inline bool exitSession(cmdCode cmd) {
    return (cmdCode::exit == cmd ||cmdCode::quit == cmd);
}

inline bool telnetControlCmd(const std::string& cmd) {
    return (TELNET::CMD_CODE::IAC == cmd.front());
}

}

namespace session {

void handler(int sock, bool& isActiveFlag) {
    
    session_socket = sock; // init thread local variable for socket
    
    try {
        CTRLcmdPart.reserve(CTRL_CMD_BUFFER_SIZE);
        USERcmdPart.reserve(MAX_USER_CMD_SIZE);

        initSession();
        prompt();

        while(isActiveFlag) {
            
            readInput();
        
            bool userCmdHandled = true; 

            while (! cmdQueue.empty()) {

                const std::string cmd = std::move(cmdQueue.front());
                cmdQueue.pop();

                std::string response {};

                if(telnetControlCmd(cmd)) {

                    response = processIAC(cmd);

                    userCmdHandled = false;

                } else {
                    auto cmd_pack = defineCmd(cmd);                                       

                    response = processCmd(cmd_pack);

                    userCmdHandled = true;

                    if(exitSession(cmd_pack.code)) {
                        isActiveFlag = false;
                        break;
                    }
                }
                sendResponse(std::move(response));
            }      

            if(isActiveFlag && userCmdHandled) {
                prompt();
            }
        }
    } catch (std::exception& e) {
        std::cerr << "exception in thread: " << e.what() << "\n";
    }

    isActiveFlag = false;
    close(session_socket);
}

}

