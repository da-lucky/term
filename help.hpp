#ifndef help_h
#define help_h

#include "commonDefs.hpp"
#include "commands.hpp"

using namespace term_app;

std::string processHelp(const std::string& arg) {

    std::string replyToSend {"Allowable commands:\r\n\t"};
    
    for(auto& e: cmdMap) {        
        replyToSend.append(e.first).append(" ");
    }

    return replyToSend;
};

// TODO: move to separate files
std::string processCmd1(const std::string& arg) {
    std::string replyToSend {"Cmd1 output"};

    return replyToSend;
};

std::string processCmd2(const std::string& arg) {
    std::string replyToSend {"Cmd2 output"};

    return replyToSend;
};

std::string processExit(const std::string& arg) {
    std::string replyToSend {"Exit output"};

    return replyToSend;
};

std::string processQuit(const std::string& arg) {
    std::string replyToSend {"Quit output"};

    return replyToSend;
};

//

#endif /* help_h */
