#ifndef help_h
#define help_h

#include "commonDefs.hpp"
#include "commands.hpp"

using namespace term_app;

std::string processHelp(const std::string& arg) {

    std::string replyToSend {"Allowable commands:" + ASCII::CR_LF_TAB};
    
    for(auto& e: cmdMap) {        
        replyToSend.append(e.first).append(" ");
    }

    return replyToSend.append(ASCII::CR_LF);
};

std::string processClear(const std::string& arg) {

    return std::string{MOVEMENT_ESCAPE_SEQ::CLEAR_SCR + MOVEMENT_ESCAPE_SEQ::POS_0_0};
}

// TODO: move to separate files
std::string processCmd1(const std::string& arg) {
    std::string replyToSend {"Cmd1 output"};

    return replyToSend.append(ASCII::CR_LF);
};

std::string processCmd2(const std::string& arg) {
    std::string replyToSend {"Cmd2 output"};

    return replyToSend.append(ASCII::CR_LF);
};

std::string processExit(const std::string& arg) {
    std::string replyToSend {"Exit output"};

    return replyToSend.append(ASCII::CR_LF);
};

std::string processQuit(const std::string& arg) {
    std::string replyToSend {"Quit output"};

    return replyToSend.append(ASCII::CR_LF);
};

//

#endif /* help_h */
